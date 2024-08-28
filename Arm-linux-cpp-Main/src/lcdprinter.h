#pragma once

#include "vector2.h"
#include <map>
#include <string>

using namespace std;

typedef int pixel;

class LcdPrinter
{
private:
    struct Font
    {
        Vector2 image_size;
        Vector2 word_size;
        int bits_per_pixel;
        pixel *data;
    } font; // 字库图像：属性和数据指针
    int lcd_fd;
    pixel *lcd_ptr;              // 屏幕文件指针
    map<char, Vector2> font_map; // 索引：字符 -> 它在图像中的起始位置


public:

    // 初始化，打开屏幕文件并初始化字库图像
    LcdPrinter(const char *path_to_font_image = NULL);
    ~LcdPrinter();

    // 重载字库图像，建立索引
    void Load_FontImage(const char *path_to_font_image);

    // 打印文字到屏幕
    void Print_Char(char c, Vector2 scr_pos, float scale, pixel color);
    void Print_Text(string text, Vector2 start_pos, float scale = 1.0, pixel color = 0xFF000000);
};
