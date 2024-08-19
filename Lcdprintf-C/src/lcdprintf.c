#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "lcdprintf.h"

void LCD_printf_init()
{
    printf("LCD_printf_init() function called\n");
    char lcd_file_path[] = "/dev/fb0";
    int lcd_fd = open(lcd_file_path, O_RDWR);
    if (lcd_fd < 0)
    {
        printf("Error: LCD file cannot be opened : %s\n", lcd_file_path);
        exit(1);
    }
    mmap_ptr = (pixel *)mmap(0, 800 * 480 * sizeof(pixel), PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (mmap_ptr == MAP_FAILED)
    {
        printf("Error: mmap failed\n");
        exit(1);
    }

    // 初始化字符索引，以及默认反色
    for (int i = 0; i < 0xEFFFFF; i++)
    {
        font_map[i] = (Vector2){-1, -1};
    }
    font_inversion = true;
}

bool is_in_range(Vector2 pos)
{
    if (pos.x < 0 || pos.x >= 800 || pos.y < 0 || pos.y >= 480)
        return false;
    return true;
}

bool is_han(wchar c)
{
    if (0xE00000 <= c && c <= 0xEFFFFF)
    {
        return true;
    }
    return false;
}

int get_utf8_len_from_first_byte(char first_byte)
{
    if (0x00 <= first_byte && first_byte <= 0x7F) // 1byte编码形式
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
        return 0;
    }

    return -1;
}

void Load_font(const char *font_file, const char *index_file, Vector2 cut)
{
    int image_fd = open(font_file, O_RDONLY);
    if (image_fd == -1)
    {
        printf("Error: cannot open %s\n", font_file);
        exit(1);
    }
    // 读取必要的bmp文件信息
    int PixelDataOffset;
    int ImageWidth;
    int ImageHeight;
    lseek(image_fd, 10, SEEK_SET);
    read(image_fd, &PixelDataOffset, 4);
    lseek(image_fd, 18, SEEK_SET);
    read(image_fd, &ImageWidth, 4);
    read(image_fd, &ImageHeight, 4);
    lseek(image_fd, PixelDataOffset, SEEK_SET);

    // 读取bmp文件数据，按方便的方式存到font.data中
    int word_size_x = ImageWidth / cut.x;
    int word_size_y = ImageHeight / cut.y;
    font.image_size = (Vector2){ImageWidth, ImageHeight};
    font.word_size = (Vector2){word_size_x, word_size_y};
    font.data = (pixel *)malloc(font.image_size.x * font.image_size.y * sizeof(pixel));
    printf("\nReading font image data ...\n");
    printf("Image width: %d\n", ImageWidth);
    printf("Image height: %d\n", ImageHeight);
    printf("Word size: %dx%d\n", word_size_x, word_size_y);
    pixel bit_32;
    pixel bit_32_reverse;
    for (int l = font.image_size.y - 1; l >= 0; l--)
    {
        for (int c = 0; c < font.image_size.x; c += 32)
        {
            read(image_fd, &bit_32, 4);
            bit_32_reverse = ((bit_32 & 0x000000FF) << 24) | ((bit_32 & 0x0000FF00) << 8) | ((bit_32 & 0x00FF0000) >> 8) | ((bit_32 & 0xFF000000) >> 24);
            for (int i = 0; i < 32 && c + i < font.image_size.x; i++)
            {
                font.data[l * font.image_size.x + c + i] = (bit_32_reverse >> (31 - i)) & 0x01;
            }
        }
    }
/*
    data:  0x12 0x23 0x34 0x45 0x56 0x67

    file:  0x45 0x34 0x23 0x12 0x00 0x00 0x67 0x56

    step1: 0x45 0x34 0x23 0x12

    step2: 0x12 0x23 0x34 0x45

    step3: 0x00 0x00 0x67 0x56

    step4: 0x56 0x67 0x00 0x00

    step6: 0x12 0x23 0x34 0x45 0x56 0x67 0x00 0x00

    step7: 0x12 0x23 0x34 0x45 0x56 0x67 0x00 0x00
*/


/*
    <[3]> <[4]> <[6]> <[12]>

    img[0] = <[3]>
    img[1] = <[4]>
    img[2] = <[6]>
    img[3] = <[12]>

    find(num_img_6)
*/

    // 建立索引表
    printf("Building font index table...\n");
    int index_fd = open("./res/font image/font image han index.txt", O_RDONLY);
    int img_pos_x = 0;
    int img_pos_y = 0;
    wchar wch = '\0';
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
        font_map[wch] = (Vector2){img_pos_x, img_pos_y};
        // printf("Conecting %x to (%d,%d)\n", wch, img_pos_x, img_pos_y);

        // 图片上虚拟指针右移一图片宽度，可能换行。
        img_pos_x = is_han(wch) ? img_pos_x + 2 * word_size_x : img_pos_x + word_size_x;
        if (img_pos_x >= font.image_size.x)
        {
            img_pos_x = 0;
            img_pos_y += word_size_y;
        }
    }
    close(image_fd);
    close(index_fd);
    printf("Done.\n\n");
    return;
}

void LCD_print_char(Vector2 scr_pos, wchar c, float scale, pixel color)
{
    // 找到字符在图片中的位置，找不到就指向"错误"字符的位置。
    Vector2 img_pos;
    if (font_map[c].x == -1)
    {
        img_pos = font_map['~' + 1];
    }
    else
    {
        img_pos = font_map[c];
    }

    // 缩放
    int font_width = font.word_size.x * (is_han(c) ? 2 : 1);
    int font_height = font.word_size.y;

    if ((scale - 1.0)*(scale - 1.0) < 0.00001) // 不缩放
    {
        for (int l = 0; l < font_height; l++)
        {
            for (int c = 0; c < font_width; c++)
            {
                if (!is_in_range((Vector2){scr_pos.x + c, scr_pos.y + l}))
                {
                    continue;
                }
                if (font_inversion && !font.data[(img_pos.y + l) * font.image_size.x + (img_pos.x + c)])
                {
                    mmap_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                    continue;
                }
                if (!font_inversion && font.data[(img_pos.y + l) * font.image_size.x + (img_pos.x + c)])
                {
                    mmap_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                    continue;
                }
            }
        }
        return;
    }
/*
    lcd     |      img

        scale = 1
    01      |       01
    23      |       23
        scale = 2

    0011    |       01
    0011    |       23
    2233    |
    2233    |
*/
    // 遍历缩小后的屏幕区域，等比放大到正常大小，在图片上找对应像素
    Vector2 scr_size_new = (Vector2){font_width * scale, font_height * scale};
    Vector2 fit_to_img;
    for (int l = 0; l < scr_size_new.y; l++)
    {
        for (int c = 0; c < scr_size_new.x; c++)
        {
            Vector2 fit_to_img = (Vector2){img_pos.x + c * 1.0 / scale, img_pos.y + l * 1.0 / scale};
            if (!is_in_range((Vector2){scr_pos.x + c, scr_pos.y + l}))
            {
                continue;
            }
            if (font_inversion && !font.data[fit_to_img.y * font.image_size.x + fit_to_img.x])
            {
                mmap_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                continue;
            }
            if (!font_inversion && font.data[fit_to_img.y * font.image_size.x + fit_to_img.x])
            {
                mmap_ptr[(scr_pos.y + l) * 800 + scr_pos.x + c] = color;
                continue;
            }
        }
    }
}

void LCD_print_string(Vector2 start_pos, char *text, pixel color)
{
    Vector2 word_size = font.word_size;
    Vector2 write_pos = start_pos;
    char ch;
    wchar wch;

    // 遍历整个字符串
    for (int i = 0; text[i] != '\0'; i++)
    {
        ch = text[i];

        // 根据字符byte长度，额外读取字符组成宽字符wchar
        wch = ch;
        int utf8_len = get_utf8_len_from_first_byte(text[i]);
        for (int j = 1; j < utf8_len; j++)
        {
            i++;
            ch = text[i];
            wch = (wch << 8) | ch;
        }
        // 如果是换行符，换行
        if (wch == '\n')
        {
            write_pos = (Vector2){start_pos.x, write_pos.y + word_size.y};
            continue;
        }
        // 打印字符
        LCD_print_char(write_pos, wch, 1.0, color);
        // 写位置右移，因为无边界，所以不用判断是否换行
        write_pos.x += (is_han(wch) ? 2 : 1) * word_size.x;
    }
}

void LCD_print_string_options(Vector2 start_pos, Vector2 end_pos, char *text, pixel color, float scale, Vector2 LC_adjust)
{
    Vector2 word_size = (Vector2){font.word_size.x * scale + LC_adjust.x, font.word_size.y * scale + LC_adjust.y}; // 计算缩放后的字体大小，用以移动打印位置
    Vector2 write_pos = start_pos;
    char ch;
    wchar_t wch;

    // 遍历整个字符串
    for (int i = 0; text[i] != '\0'; i++)
    {
        ch = text[i];

        // 根据字符byte长度，额外读取字符组成宽字符wchar_t
        wch = ch;
        int utf8_len = get_utf8_len_from_first_byte(text[i]);
        for (int j = 1; j < utf8_len; j++)
        {
            i++;
            ch = text[i];
            wch = (wch << 8) | ch;
        }
        // 如果是换行符，换行
        if (wch == '\n')
        {
            write_pos = (Vector2){start_pos.x, write_pos.y + word_size.y};
            continue;
        }
        // 打印字符
        LCD_print_char(write_pos, wch, scale, color);
        // 写位置右移，可能换行
        write_pos.x += (is_han(wch) ? 2 : 1) * word_size.x;
        if (write_pos.x > end_pos.x)
        {
            write_pos.x = start_pos.x;
            write_pos.y += word_size.y;
        }
    }
}
