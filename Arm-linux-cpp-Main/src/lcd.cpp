#include "lcd.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iomanip>
#include <regex>
#include <jpeglib.h>
#include <cstring>

using namespace std;

// vector<Image> images;       // 加载图片的容器
// vector<Layer> layers;       // 图层的容器
// pixel per_frame[800 * 480]; // 预备帧
// pixel *mmap_ptr;            // 内存映射指针8

// LCD_Manager();  // 初始化：建立内存映射，加载图片
// ~LCD_Manager(); // 析构：释放内存映射

// void load_image(string path, string name); // 加载图片
// void add_layer(string name);               // 添加图层
// void remove_layer(int index);              // 删除图层
// void clear_layer(int index);                     // 清空图层
// void show_layer(int index);                // 显示图层
// void hide_layer(int index);                // 隐藏图层

// void draw_image(int layer_index, int image_index, int x, int y, int pivot_x, int pivot_y); // 绘制图片到图层

// 初始化：建立内存映射，加载图片
LCD_Manager::LCD_Manager()
{
    cout << "LCD_Manager::LCD_Manager() is called" << endl;
    const char *LCD_file_path = "/dev/fb0";
    lcd_fd = open(LCD_file_path, O_RDWR);
    if (lcd_fd == -1)
    {
        cerr << "Error: cannot open " << LCD_file_path << endl;
        exit(EXIT_FAILURE);
    }
    mmap_ptr = static_cast<pixel *>(mmap(nullptr, 800 * 480 * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0));
    if (mmap_ptr == MAP_FAILED)
    {
        cerr << "Error: cannot mmap " << LCD_file_path << endl;
        close(lcd_fd);
        exit(EXIT_FAILURE);
    }
    font_inversion = false;
}

// 析构：释放内存映射
LCD_Manager::~LCD_Manager()
{
    cout << "LCD_Manager::~LCD_Manager() is called" << endl;
    munmap(mmap_ptr, 800 * 480 * sizeof(pixel));
    close(lcd_fd);
}

int LCD_Manager::get_layer_index(const string &name) const
{
    auto it = layer_map.find(name);
    if (it != layer_map.end())
        return it->second;
    throw invalid_argument("Layer not found: " + name);
}

int LCD_Manager::get_image_index(const string &name) const
{
    auto it = image_map.find(name);
    if (it != image_map.end())
        return it->second;
    throw invalid_argument("Image not found: " + name);
}

void LCD_Manager::load_image_bmp(const char *Path, string name)
{
    // 打开文件
    int FD = open(Path, O_RDONLY);
    if (FD == -1)
    {
        cerr << "Error: cannot open " << Path << endl;
        return;
    }

    // 读取文件头信息
    char FileType[2]{'\0'};
    int FileSize, PixelDataOffset, ImageWidth, ImageHeight, BitsPerPixel = 0;
    read(FD, &FileType, 2);
    read(FD, &FileSize, 4);
    lseek(FD, 10, SEEK_SET);
    read(FD, &PixelDataOffset, 4);
    lseek(FD, 18, SEEK_SET); // 跳过不必要的头信息
    read(FD, &ImageWidth, 4);
    read(FD, &ImageHeight, 4);
    lseek(FD, 28, SEEK_SET); // 跳过不必要的头信息
    read(FD, &BitsPerPixel, 2);
    cout << "\nReading image data ..." << endl;
    cout << "Path: " << Path << endl;
    cout << "File size: " << FileSize << endl;
    cout << "Image width: " << ImageWidth << endl;
    cout << "Image height: " << ImageHeight << endl;
    cout << "Bits per pixel: " << BitsPerPixel << endl;

    // 读取像素数据
    lseek(FD, PixelDataOffset, SEEK_SET);
    pixel *pixel_data = new pixel[ImageWidth * ImageHeight];
    switch (BitsPerPixel)
    {
    case 24:
        for (int i = 0; i < ImageWidth * ImageHeight; i++)
        {
            int color;
            read(FD, &color, 3);
            color |= 0xff000000;
            pixel_data[ImageWidth * (ImageHeight - 1 - (i / ImageWidth)) + i % ImageWidth] = color;
            int bitFill = 4 - (i * 3 + 3) % 4; // 优化比特填充计算
            read(FD, nullptr, bitFill);
        }
        break;
    case 32:
        for (int i = 0; i < ImageWidth * ImageHeight; i++)
        {
            char B, G, R, A;
            read(FD, &B, 1);
            read(FD, &G, 1);
            read(FD, &R, 1);
            read(FD, &A, 1);
            pixel_data[ImageWidth * (ImageHeight - 1 - (i / ImageWidth)) + i % ImageWidth] = (A << 24) | (R << 16) | (G << 8) | B;
        }
        break;
    default:
        cerr << "Error: unsupported bits per pixel: " << BitsPerPixel << endl;
        close(FD);
        delete[] pixel_data;
        return;
    }

    // 保存图片信息
    Image image(name, ImageWidth, ImageHeight, pixel_data);
    images.push_back(image);

    // 建立索引
    image_map[name] = images.size() - 1;

    cout << "Reading image data done.\n"
         << endl;

    // 关闭文件
    close(FD);
}

void LCD_Manager::load_image_jpg(const char *Path, string name)
{
    FILE *fp = fopen(Path, "rb");
    if (!fp)
    {
        cerr << "Error: cannot open " << Path << endl;
        return;
    }

    cout << "\nOpen and reading jpg image info ..." << endl;

    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);

    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
    {
        cerr << "Error: invalid JPEG file: " << Path << endl;
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return;
    }

    printJPEGDecompressStructMembers(cinfo);
    jpeg_start_decompress(&cinfo);

    int ImageWidth = cinfo.output_width;
    int ImageHeight = cinfo.output_height;
    int row_stride = ImageWidth * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    pixel *pixel_data = new pixel[ImageWidth * ImageHeight];

    cout << "Start reading data ..." << endl;

    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int i = 0; i < ImageWidth; i++)
        {
            pixel_data[cinfo.output_scanline * cinfo.output_width + i] = (0xff << 24) | (buffer[0][i * 3] << 16) | (buffer[0][i * 3 + 1] << 8) | buffer[0][i * 3 + 2];
        }
    }
    Image image(name, ImageWidth, ImageHeight, pixel_data);
    images.push_back(image);

    image_map[name] = images.size() - 1;

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
    cout << "reading data done.\n"
         << endl;
    return;
}

void LCD_Manager::printJPEGDecompressStructMembers(const jpeg_decompress_struct &cinfo)
{
    cout << "jpeg文件属性 : " << endl;
    cout << "图像的宽度(从SOF标记中获取) : " << cinfo.image_width << endl;
    cout << "图像的高度 : " << cinfo.image_height << endl;
    cout << "颜色分量数(例如,灰度图像为1,RGB图像为3) : " << cinfo.num_components << endl;
    cout << "图像的颜色空间 : " << cinfo.jpeg_color_space << endl;
    cout << "输出颜色空间 : " << cinfo.out_color_space << endl;
    cout << "缩放比例的分子 : " << cinfo.scale_num << endl;
    cout << "缩放比例的分母 : " << cinfo.scale_denom << endl;
    cout << "输出伽马值 : " << cinfo.output_gamma << endl;
    cout << "是否以缓冲图像模式解压 : " << cinfo.buffered_image << endl;
    cout << "是否输出原始数据 : " << cinfo.raw_data_out << endl;
    cout << "DCT方法 : " << cinfo.dct_method << endl;
    cout << "是否进行精细上采样 : " << cinfo.do_fancy_upsampling << endl;
    cout << "是否进行块平滑 : " << cinfo.do_block_smoothing << endl;
    cout << "是否量化颜色 : " << cinfo.quantize_colors << endl;
    cout << "抖动模式 : " << cinfo.dither_mode << endl;
    cout << "是否进行两遍量化 : " << cinfo.two_pass_quantize << endl;
    cout << "期望的颜色数量 : " << cinfo.desired_number_of_colors << endl;
    cout << "是否启用单遍量化 : " << cinfo.enable_1pass_quant << endl;
    cout << "是否启用外部量化 : " << cinfo.enable_external_quant << endl;
    cout << "是否启用两遍量化 : " << cinfo.enable_2pass_quant << endl;
}

bool LCD_Manager::is_in_range(int x, int y) const
{
    return x >= 0 && x < 800 && y >= 0 && y < 480;
}

bool LCD_Manager::is_in_range(Vector2 pos) const
{
    return pos.x >= 0 && pos.x < 800 && pos.y >= 0 && pos.y < 480;
}

bool LCD_Manager::is_han(wchar_t c) const
{
    return 0xE00000 <= c && c <= 0xEFFFFF;
}

int LCD_Manager::get_utf8_len_from_first_byte(unsigned char first_byte) const
{
    if (first_byte <= 0x7F) // 1byte编码形式
    {
        return 1;
    }
    else if (0xC0 <= first_byte && first_byte <= 0xDF) // 2byte编码形式
    {
        return 2;
    }
    else if (0xE0 <= first_byte && first_byte <= 0xEF) // 3byte编码形式
    {
        return 3;
    }
    else if (0xF0 <= first_byte && first_byte <= 0xF7) // 4byte编码形式
    {
        return 4;
    }
    else
    {
        printf("Invalid UTF-8 first byte: %02X\n", first_byte);
        throw invalid_argument("Invalid UTF-8 first byte");
    }
}

void LCD_Manager::Load_FontImage(const char *path_to_font_image)
{
    int FD = open(path_to_font_image, O_RDONLY);
    if (FD == -1)
    {
        cerr << "Error: cannot open " << path_to_font_image << endl;
        return;
    }

    // 读取文件头信息
    char FileType[2]{'\0'};
    int FileSize, PixelDataOffset, ImageWidth, ImageHeight, BitsPerPixel = 0;
    read(FD, &FileType, 2);
    read(FD, &FileSize, 4);
    lseek(FD, 10, SEEK_SET);
    read(FD, &PixelDataOffset, 4);
    lseek(FD, 18, SEEK_SET); // 跳过不必要的头信息
    read(FD, &ImageWidth, 4);
    read(FD, &ImageHeight, 4);
    lseek(FD, 28, SEEK_SET); // 跳过不必要的头信息
    read(FD, &BitsPerPixel, 2);
    cout << "\nReading font image data ..." << endl;
    cout << "Path: " << path_to_font_image << endl;
    cout << "File size: " << FileSize << endl;
    cout << "Image width: " << ImageWidth << endl;
    cout << "Image height: " << ImageHeight << endl;
    cout << "Bits per pixel: " << BitsPerPixel << endl;

    // 处理图片像素数据，写到font.data中
    font.image_size = {ImageWidth, ImageHeight};
    font.word_size = {ImageWidth / 16, ImageHeight / 6};
    font.bits_per_pixel = BitsPerPixel;

    lseek(FD, PixelDataOffset, SEEK_SET);
    font.data = new pixel[font.image_size.x * font.image_size.y];

    pixel bit_32;
    pixel bit_32_reverse;
    for (int l = font.image_size.y - 1; l >= 0; l--)
    {
        for (int c = 0; c < font.image_size.x; c += 32)
        {
            read(FD, &bit_32, 4);
            bit_32_reverse = ((bit_32 & 0x000000FF) << 24) | ((bit_32 & 0x0000FF00) << 8) | ((bit_32 & 0x00FF0000) >> 8) | ((bit_32 & 0xFF000000) >> 24);
            for (int i = 0; i < 32 && c + i < font.image_size.x; i++)
            {
                font.data[l * font.image_size.x + c + i] = (bit_32_reverse >> (31 - i)) & 0x01;
            }
        }
    }

    // 建立索引表
    cout << "Building font index table..." << endl;
    char ch = ' ';
    for (int l = 0; l < 6; l++)
    {
        for (int c = 0; c < 16; c++)
        {
            font_map[ch++] = {c * font.word_size.x, l * font.word_size.y};
        }
    }
    cout << "Reading font image data done.\n"
         << endl;

    return;
}

void LCD_Manager::Load_FontImage(const char *font_image, const char *font_index_text, Vector2 cut)
{
    int image_fd = open(font_image, O_RDONLY);
    if (image_fd == -1)
    {
        cout << "Error: cannot open " << font_image << endl;
        exit(1);
    }

    // 读取文件头信息
    char FileType[2]{'\0'};
    int FileSize, PixelDataOffset, ImageWidth, ImageHeight, BitsPerPixel = 0;
    read(image_fd, &FileType, 2);
    read(image_fd, &FileSize, 4);
    lseek(image_fd, 10, SEEK_SET);
    read(image_fd, &PixelDataOffset, 4);
    lseek(image_fd, 18, SEEK_SET); // 跳过不必要的头信息
    read(image_fd, &ImageWidth, 4);
    read(image_fd, &ImageHeight, 4);
    lseek(image_fd, 28, SEEK_SET); // 跳过不必要的头信息
    read(image_fd, &BitsPerPixel, 2);
    cout << "\nReading font image data ..." << endl;
    cout << "Path: " << font_image << endl;
    cout << "File size: " << FileSize << endl;
    cout << "Image width: " << ImageWidth << endl;
    cout << "Image height: " << ImageHeight << endl;
    cout << "Bits per pixel: " << BitsPerPixel << endl;

    // 处理图片像素数据，写到font.data中
    int word_size_x = ImageWidth / cut.x;
    int word_size_y = ImageHeight / cut.y;
    font.image_size = {ImageWidth, ImageHeight};
    font.word_size = {word_size_x, word_size_y};
    font.data = new pixel[font.image_size.x * font.image_size.y];
    cout << "Word size: " << word_size_x << "x" << word_size_y << endl;

    lseek(image_fd, PixelDataOffset, SEEK_SET);
    pixel bit_32;
    pixel bit_32_reverse;
    for (int l = ImageHeight - 1; l >= 0; l--)
    {
        for (int c = 0; c < ImageWidth; c += 32)
        {
            read(image_fd, &bit_32, 4);
            bit_32_reverse = ((bit_32 & 0x000000FF) << 24) | ((bit_32 & 0x0000FF00) << 8) | ((bit_32 & 0x00FF0000) >> 8) | ((bit_32 & 0xFF000000) >> 24);
            for (int i = 0; i < 32 && c + i < font.image_size.x; i++)
            {
                font.data[l * font.image_size.x + c + i] = (bit_32_reverse >> (31 - i)) & 0x01;
            }
        }
    }

    cout << "Reading font image data done." << endl;
    close(image_fd);

    // 建立索引表
    cout << "Building font index table..." << endl;
    int index_fd = open(font_index_text, O_RDONLY);
    if (index_fd == -1)
    {
        cerr << "Error: cannot open " << font_index_text << endl;
        delete[] font.data;
        return;
    }

    int img_pos_x = 0;
    int img_pos_y = 0;
    wchar_t wch = '\0';
    int ch = true;
    while (ch)
    {
        // 读取字符
        ch = '\0';
        read(index_fd, &ch, 1);
        wch = ch;
        int utf8_len = get_utf8_len_from_first_byte(ch);
        for (int i = 1; i < utf8_len; i++)
        {
            read(index_fd, &ch, 1);
            wch = (wch << 8) | ch;
        }

        // 联系起来
        font_map[wch] = {img_pos_x, img_pos_y};
        img_pos_x = is_han(wch) ? img_pos_x + 2 * word_size_x : img_pos_x + word_size_x;
        if (img_pos_x >= font.image_size.x)
        {
            img_pos_x = 0;
            img_pos_y += word_size_y;
        }
    }
    cout << "Done.\n"
         << endl;
    close(index_fd);

    return;
}

void LCD_Manager::load_image(const char *Path, string name)
{
    string pathStr(Path);
    if (pathStr.size() >= 4 && pathStr.substr(pathStr.size() - 4) == ".bmp")
    {
        load_image_bmp(Path, name);
    }
    else if (pathStr.size() >= 4 && (pathStr.substr(pathStr.size() - 4) == ".jpg" || pathStr.substr(pathStr.size() - 5) == ".jpeg"))
    {
        load_image_jpg(Path, name);
    }
    else
    {
        throw invalid_argument("Error: unsupported image format: " + pathStr);
    }
}

void LCD_Manager::add_layer(const string &name)
{
    // 检查图层名称是否已存在
    if (layer_map.find(name) != layer_map.end())
    {
        throw invalid_argument("Layer with name '" + name + "' already exists.");
    }

    Layer layer(name);
    layers.push_back(layer);
    layer_map[name] = layers.size() - 1;
    return;
}

void LCD_Manager::remove_layer(int index)
{
    if (index < 0 || index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }
    layers.erase(layers.begin() + index);
}

void LCD_Manager::remove_layer(const string &name)
{
    auto it = layer_map.find(name);
    if (it != layer_map.end())
    {
        layers.erase(layers.begin() + it->second);
        layer_map.erase(it); // 同时从layer_map中移除该图层的记录
    }
    else
    {
        throw invalid_argument("Layer with name '" + name + "' does not exist.");
    }
}

void LCD_Manager::clear_layer(int index)
{
    if (index < 0 || index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }
    Layer &layer = layers[index];
    memset(layer.pixels, 0, 800 * 480 * sizeof(pixel));
}

void LCD_Manager::clear_layer(const string &name)
{
    auto it = layer_map.find(name);
    if (it == layer_map.end())
    {
        throw invalid_argument("Layer with name '" + name + "' does not exist.");
    }
    Layer &layer = layers[it->second];
    memset(layer.pixels, 0, 800 * 480 * sizeof(pixel));
}

void LCD_Manager::show_layer(int index)
{
    if (index < 0 || index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }
    layers[index].visible = true;
}

void LCD_Manager::hide_layer(int index)
{
    if (index < 0 || index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }
    layers[index].visible = false;
}

void LCD_Manager::Print_Char(int layer_index, Vector2 scr_pos, wchar_t c, float scale, pixel color)
{
    if (layer_index < 0 || layer_index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }
    Layer &layer = layers[layer_index];

    // 找到字库中对应的字体，找不到就用错误字符代替(~的后一个)
    auto it = font_map.find(c);
    Vector2 img_pos = (it == font_map.end()) ? font_map['~' + 1] : it->second;

    // 缩放
    int font_width = font.word_size.x * (is_han(c) ? 2 : 1);
    int font_height = font.word_size.y;

    if (abs(scale - 1.0) < 0.001) // 不缩放
    {
        for (int l = 0; l < font_height; l++)
        {
            for (int c = 0; c < font_width; c++)
            {
                if (!is_in_range(scr_pos + Vector2{c, l}))
                {
                    continue;
                }
                int index = (img_pos.y + l) * font.image_size.x + (img_pos.x + c);
                if (font_inversion != font.data[index]) // 反相+无色/不反相+有色 -> 显示
                {
                    layer.pixels[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                }
            }
        }
        return;
    }

    // 遍历缩小后的屏幕区域，等比放大到正常大小，在图片上找对应像素
    Vector2 scr_size_new = {int(font_width * scale), int(font_height * scale)};
    Vector2 fit_to_img;
    for (int l = 0; l < scr_size_new.y; l++)
    {
        for (int c = 0; c < scr_size_new.x; c++)
        {
            fit_to_img = {img_pos.x + int(c * 1.0 / scale), img_pos.y + int(l * 1.0 / scale)};
            if (!is_in_range(scr_pos + Vector2{c, l}))
            {
                continue;
            }
            int index = fit_to_img.y * font.image_size.x + fit_to_img.x;
            if (font_inversion != font.data[index])
            {
                layer.pixels[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
            }
        }
    }
}

void LCD_Manager::Print_Text(int layer_index, Vector2 start_pos, string text, float scale, pixel color, Vector2 LC_adjust)
{
    if (layer_index < 0 || layer_index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }

    Vector2 word_size = {int(font.word_size.x * scale + LC_adjust.x), int(font.word_size.y * scale + LC_adjust.y)}; // 计算缩放后的字体大小，用以移动打印位置
    Vector2 write_pos = start_pos;
    int text_length = text.length();

    // 遍历整个字符串
    for (int i = 0; i < text_length;)
    {
        unsigned char ch = text[i];
        wchar_t wch = 0;
        int utf8_len = get_utf8_len_from_first_byte(ch);

        // 根据字符byte长度，额外读取字符组成宽字符wchar_t
        wch = ch;
        for (int j = 1; j < utf8_len; j++)
        {
            wch = (wch << 8) | text[i + j];
        }

        i += utf8_len;

        // 如果是换行符，换行
        if (wch == '\n')
        {
            write_pos = {start_pos.x, write_pos.y + word_size.y};
            continue;
        }
        // 打印字符
        Print_Char(layer_index, write_pos, wch, scale, color);
        // 写位置右移，因为无边界，所以不用判断是否换行
        write_pos.x += (is_han(wch) ? 2 : 1) * word_size.x;
    }
}

void LCD_Manager::Print_Text(int layer_index, Vector2 start_pos, Vector2 end_pos, string text, float scale, pixel color, Vector2 LC_adjust)
{
    if (layer_index < 0 || layer_index >= layers.size())
    {
        throw out_of_range("Layer index out of range");
    }

    Vector2 word_size = {int(font.word_size.x * scale + LC_adjust.x), int(font.word_size.y * scale + LC_adjust.y)}; // 计算缩放后的字体大小，用以移动打印位置
    Vector2 write_pos = start_pos;
    int text_length = text.length();

    // 遍历整个字符串
    for (int i = 0; i < text.length();)
    {
        unsigned char ch = text[i];
        wchar_t wch = 0;
        int utf8_len = get_utf8_len_from_first_byte(text[i]);

        // 根据字符byte长度，额外读取字符组成宽字符wchar_t
        wch = ch;
        for (int j = 1; j < utf8_len; j++)
        {
            wch = (wch << 8) | text[i + j];
        }

        i += utf8_len;

        // 如果是换行符，换行
        if (wch == '\n')
        {
            write_pos = {start_pos.x, write_pos.y + word_size.y};
            continue;
        }
        // 打印字符
        Print_Char(layer_index, write_pos, wch, scale, color);
        // 写位置右移，可能换行
        write_pos.x += (is_han(wch) ? 2 : 1) * word_size.x;
        if (write_pos.x > end_pos.x)
        {
            write_pos.x = start_pos.x;
            write_pos.y += word_size.y;
        }
    }
}

void LCD_Manager::Print_Text(const string &layer_name, Vector2 start_pos, const string &text, float scale, pixel color, Vector2 LC_adjust)
{
    auto it = layer_map.find(layer_name);
    if (it == layer_map.end())
    {
        throw invalid_argument("Layer with name '" + layer_name + "' does not exist.");
    }
    int layer_index = it->second;
    Print_Text(layer_index, start_pos, text, scale, color, LC_adjust);
}

void LCD_Manager::Print_Text(const string &layer_name, Vector2 start_pos, Vector2 end_pos, const string &text, float scale, pixel color, Vector2 LC_adjust)
{
    auto it = layer_map.find(layer_name);
    if (it == layer_map.end())
    {
        throw invalid_argument("Layer with name '" + layer_name + "' does not exist.");
    }
    int layer_index = it->second;
    Print_Text(layer_index, start_pos, end_pos, text, scale, color, LC_adjust);
}

void LCD_Manager::draw_image(int layer_index, Vector2 loc, int image_index, Vector2_f pivot)
{
    // 获取图层和图像
    if (layer_index < 0 || layer_index >= layers.size() || image_index < 0 || image_index >= images.size())
    {
        throw out_of_range("Layer or image index out of range");
    }

    Layer &layer = layers[layer_index];
    Image &image = images[image_index];

    // 以画布坐标系为准

    // 计算图片的起末坐标(世界坐标系)
    loc.x -= pivot.x * image.width;
    loc.y -= pivot.y * image.height;
    Vector2 beg(loc.x, loc.y);
    Vector2 end(beg.x + image.width, beg.y + image.height);
    if (beg.x > 800 || beg.y > 480 || end.x < 0 || end.y < 0)
    {
        return;
    }
    beg.x = beg.x < 0 ? 0 : beg.x;
    beg.y = beg.y < 0 ? 0 : beg.y;
    end.x = end.x > 800 ? 800 : end.x;
    end.y = end.y > 480 ? 480 : end.y;

    // 世界坐标系->图片本地坐标系
    Vector2 local_beg(beg.x - loc.x, beg.y - loc.y);
    Vector2 local_end(end.x - loc.x, end.y - loc.y);

    // 以图层为主体，将图片的像素点一一对应，在图层上绘制。
    Vector2 layer_pos(beg.x, beg.y);
    Vector2 image_pos(local_beg.x, local_beg.y);
    for (; layer_pos.y < end.y; layer_pos.y++, image_pos.y++)
    {
        for (layer_pos.x = beg.x, image_pos.x = local_beg.x; layer_pos.x < end.x; layer_pos.x++, image_pos.x++)
        {
            layer.pixels[layer_pos.y * 800 + layer_pos.x] = image.data[image_pos.y * image.width + image_pos.x];
        }
    }
}

void LCD_Manager::draw_image(const string &layer_name, Vector2 loc, const string &image_name, Vector2_f pivot)
{
    int layer_index = layer_map[layer_name];
    int image_index = image_map[image_name];
    draw_image(layer_index, loc, image_index, pivot);
}

void LCD_Manager::draw_image(const string &layer_name, Vector2 loc, int image_index, Vector2_f pivot)
{
    if(layer_map.find(layer_name) == layer_map.end())
    {
        cout << "Layer with name '" << layer_name << "' does not exist." << endl;
        return;
    }
    int layer_index = layer_map[layer_name];
    draw_image(layer_index, loc, image_index, pivot);
}

void LCD_Manager::draw_pixel(int layer_index, Vector2 loc, pixel color)
{
    if (loc.x < 0 || loc.x > 800 || loc.y < 0 || loc.y > 480)
    {
        return;
    }
    layers[layer_index].pixels[loc.y * 800 + loc.x] = color;
}

void LCD_Manager::draw_line(int layer_index, Vector2 start, Vector2 end, pixel color)
{
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int err = dx - dy;
    while (true)
    {
        draw_pixel(layer_index, start, color);
        if (start.x == end.x && start.y == end.y)
        {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err = err - dy;
            start.x += sx;
        }
        if (e2 < dx)
        {
            err = err + dx;
            start.y += sy;
        }
    }
}

void LCD_Manager::draw_line(int layer_index, Vector2 start, Vector2 end, int thickness, pixel color)
{
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int err = dx - dy;
    int half_thickness = thickness / 2;
    while (true)
    {
        for (int i = -half_thickness; i <= half_thickness; i++)
        {
            for (int j = -half_thickness; j <= half_thickness; j++)
            {
                draw_pixel(layer_index, Vector2{start.x + i, start.y + j}, color);
            }
        }
        if (start.x == end.x && start.y == end.y)
        {
            break;
        }
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err = err - dy;
            start.x += sx;
        }
        if (e2 < dx)
        {
            err = err + dx;
            start.y += sy;
        }
    }
}

void LCD_Manager::draw_rect(int layer_index, Vector2 top_left, Vector2 bottom_right, pixel color)
{
    for (int x = top_left.x; x < bottom_right.x; x++)
    {
        for (int y = top_left.y; y < bottom_right.y; y++)
        {
            draw_pixel(layer_index, Vector2{x, y}, color);
        }
    }
}

void LCD_Manager::draw_circle(int layer_index, Vector2 center, int radius, pixel color)
{
    int dx, dy;
    for (int x = center.x - radius; x < center.x + radius; x++)
    {
        for (int y = center.y - radius; y < center.y + radius; y++)
        {
            dx = x - center.x;
            dy = y - center.y;
            if (dx * dx + dy * dy <= radius * radius)
            {
                draw_pixel(layer_index, Vector2{x, y}, color);
            }
        }
    }
}

void LCD_Manager::LayersInfo()
{
    // cout << "LCD_Manager::LayersInfo() is called" << endl;
    cout << "Layers Info:" << endl;
    for (int i = 0; i < layers.size(); i++)
    {
        cout << "Layer " << i << ": " << layers[i].name << " (" << layers[i].visible << ")" << endl;
    }
}

void LCD_Manager::update()
{
    // cout << "LCD_Manager::update() is called" << endl;
    for (int i = 0; i < layers.size(); i++)
    {
        // 跳过不可见图层
        if (!layers[i].visible)
        {
            continue;
        }
        for (int p = 0; p < 800 * 480; p++)
        {
            // 跳过透明像素
            if (!(layers[i].pixels[p] & 0xff000000))
            {
                continue;
            }
            per_frame[p] = layers[i].pixels[p];
        }
    }
    // 缓存帧一次性写入LCD
    for (int i = 0; i < 800 * 480; i++)
    {
        mmap_ptr[i] = per_frame[i];
    }
}
