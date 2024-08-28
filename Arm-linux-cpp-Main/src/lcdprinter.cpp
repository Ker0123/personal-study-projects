#include "lcdprinter.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <unistd.h>

using namespace std;

void LcdPrinter::Print_Char(char c, Vector2 scr_pos, float scale, pixel color)
{
    Vector2 img_pos;
    if (font_map.find(c) == font_map.end())
    {
        img_pos = font_map['~' + 1];
    }
    else
    {
        img_pos = font_map[c];
    }
    if (abs(scale - 1.0) < 0.001) // 不缩放
    {
        for (int l = 0; l < font.word_size.y; l++)
        {
            for (int c = 0; c < font.word_size.x; c++)
            {
                if (font.data[(img_pos.y + l) * font.image_size.x + img_pos.x + c])
                {
                    lcd_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                }
            }
        }
        return;
    }

    // 遍历缩小后的屏幕区域，等比放大到正常大小，在图片上找对应像素
    Vector2 scr_size_new = {int(font.word_size.x * scale), int(font.word_size.y * scale)};
    Vector2 fit_to_img;
    for (int l = 0; l < scr_size_new.y; l++)
    {
        for (int c = 0; c < scr_size_new.x; c++)
        {
            Vector2 fit_to_img = {img_pos.x + int(c * 1.0 / scale), img_pos.y + int(l * 1.0 / scale)};
            if (font.data[fit_to_img.y * font.image_size.x + fit_to_img.x])
            {
                lcd_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
            }
        }
    }
}

LcdPrinter::LcdPrinter(const char *path_to_font_image)
{
    cout << "Initializing LCD printer..." << endl;
    // 打开LCD设备文件
    char LCD_file_path[] = "/dev/fb0";
    lcd_fd = open(LCD_file_path, O_RDWR);
    if (lcd_fd == -1)
    {
        cout << "Error: cannot open " << LCD_file_path << endl;
        exit(1);
    }
    lcd_ptr = (pixel *)mmap(0, 800 * 480 * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (lcd_ptr == MAP_FAILED)
    {
        cout << "Error: cannot mmap " << LCD_file_path << endl;
        exit(1);
    }

    // 如果有，加载字体图像
    if (path_to_font_image != NULL)
    {
        Load_FontImage(path_to_font_image);
    }
}

LcdPrinter::~LcdPrinter()
{
    // 关闭LCD设备文件
    munmap(lcd_ptr, 800 * 480 * sizeof(int));
    close(lcd_fd);
}

void LcdPrinter::Load_FontImage(const char *path_to_font_image)
{
    int FD = open(path_to_font_image, O_RDONLY);
    if (FD == -1)
    {
        cout << "Error: cannot open " << path_to_font_image << endl;
        exit(1);
    }

    // 读取文件头信息
    char FileType[2]{'\0'};
    int FileSize;
    int PixelDataOffset;
    int HeaderSize;
    int ImageWidth;
    int ImageHeight;
    int Planes = 0;
    int BitsPerPixel = 0;
    int Compression;
    int ImageSize;
    int XpixelsPerMeter;
    int YpixelsPerMeter;
    int TotalColors;
    int ImportantColors;
    read(FD, &FileType, 2);
    read(FD, &FileSize, 4);
    lseek(FD, 10, SEEK_SET);
    read(FD, &PixelDataOffset, 4);
    read(FD, &HeaderSize, 4);
    read(FD, &ImageWidth, 4);
    read(FD, &ImageHeight, 4);
    read(FD, &Planes, 2);
    read(FD, &BitsPerPixel, 2);
    read(FD, &Compression, 4);
    read(FD, &ImageSize, 4);
    read(FD, &XpixelsPerMeter, 4);
    read(FD, &YpixelsPerMeter, 4);
    read(FD, &TotalColors, 4);
    read(FD, &ImportantColors, 4);
    cout << "Path: " << path_to_font_image << endl;
    cout << "File type: " << FileType[0] << FileType[1] << endl;
    cout << "File size: " << FileSize << endl;
    cout << "Pixel data offset: " << PixelDataOffset << endl;
    cout << "Header size: " << HeaderSize << endl;
    cout << "Image width: " << ImageWidth << endl;
    cout << "Image height: " << ImageHeight << endl;
    cout << "Planes: " << Planes << endl;
    cout << "Bits per pixel: " << BitsPerPixel << endl;
    cout << "Compression: " << Compression << endl;
    cout << "Image size: " << ImageSize << endl;
    cout << "X pixels per meter: " << XpixelsPerMeter << endl;
    cout << "Y pixels per meter: " << YpixelsPerMeter << endl;
    cout << "Total colors: " << TotalColors << endl;
    cout << "Important colors: " << ImportantColors << endl;
    cout << "Reading data ..." << endl;

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
    char ch = ' ';
    for (int l = 0; l < 6; l++)
    {
        for (int c = 0; c < 16; c++)
        {
            // cout << ch << " -> " << (Vector2){c * font.word_size.x, l * font.word_size.y} << endl;
            font_map[ch++] = {c * font.word_size.x, l * font.word_size.y};
        }
    }

    return;
    // 根据索引表，尝试在终端打印
    for (char c = ' '; c < 'z'; c++)
    {
        Vector2 pos = font_map[c];
        cout << "Printing " << c << "..." << pos << endl;
        for (int l = 0; l < font.word_size.y; l++)
        {
            for (int c = 0; c < font.word_size.x; c++)
            {
                if (font.data[(pos.y + l) * font.image_size.x + pos.x + c])
                {
                    cout << "# ";
                }
                else
                {
                    cout << "_ ";
                }
            }
            cout << endl;
        }
        usleep(600000);
    }
}

void LcdPrinter::Print_Text(string text, Vector2 start_pos, float scale, pixel color)
{
    int distance = int(font.word_size.x * scale);
    int word_cnt = 0;
    for (char ch : text)
    {
        cout << "Printing " << ch << "..." << endl;
        Print_Char(ch, start_pos + Vector2{word_cnt * distance, 0}, scale, color);
        word_cnt++;
    }
}
