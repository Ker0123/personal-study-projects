#pragma once

#include <vector>
#include <string>
#include <jpeglib.h>
#include <map>
#include <stdexcept>

#include "vector2.h"

using namespace std;

typedef int pixel;

struct Vector2_f
{
    float x;
    float y;
};

class Image
{
public:
    string name;
    int width;
    int height;
    pixel *data; // aarrggbb aa代表透明度，目前00是隐藏，其他都是显示
    Image(string name, int width, int height, pixel *data)
        : name(name), width(width), height(height), data(data) {}
};

class Layer
{
public:
    string name; // 名字只用于做标识，打印调试信息。不用于做索引。
    bool visible;
    pixel *pixels;
    Layer(string name, bool visible = true)
        : name(name), visible(visible)
    {
        pixels = new pixel[800 * 480]{0x00000000};
    }
};

class LCD_Manager
{
private:
    int get_layer_index(const string &name) const; // 获取图层索引
    int get_image_index(const string &name) const; // 获取图片索引
    int lcd_fd;                                    // 屏幕设备文件
    pixel *mmap_ptr;                               // 屏幕内存映射指针
    pixel per_frame[800 * 480];                    // 预备帧
    struct Font
    {
        Vector2 image_size;
        Vector2 word_size;
        int bits_per_pixel;
        pixel *data;
    } font;                                                                     // 字库图像：属性和数据指针
    vector<Image> images;                                                       // 加载图片的容器
    vector<Layer> layers;                                                       // 图层的容器
    map<wchar_t, Vector2> font_map;                                             // 字符到字母位置的映射
    map<string, int> image_map;                                                 // 图片名到索引的映射
    map<string, int> layer_map;                                                 // 图层名到索引的映射
    void load_image_bmp(const char *Path, string name);                         // 加载bmp图片
    void load_image_jpg(const char *Path, string name);                         // 加载jpg图片
    void printJPEGDecompressStructMembers(const jpeg_decompress_struct &cinfo); // 打印jpeg解码器信息
    bool is_in_range(int x, int y) const;                                       // 判断坐标是否在屏幕范围内
    bool is_in_range(Vector2 pos) const;                                        // 判断坐标是否在屏幕范围内

    bool is_han(wchar_t c) const;                                     // 判断是否是汉字
    int get_utf8_len_from_first_byte(unsigned char first_byte) const; // 获取utf8编码的字节数

public:
    LCD_Manager();  // 初始化：建立内存映射
    ~LCD_Manager(); // 析构：释放内存映射

    bool font_inversion; // 是否反色显示

    void Load_FontImage(const char *path_to_font_image);                                   // 加载字库图像
    void Load_FontImage(const char *font_image, const char *font_index_text, Vector2 cut); // 加载字库图像
    void load_image(const char *Path, string name);                                        // 加载图片

    void add_layer(const string &name);    // 添加图层
    void remove_layer(int index);          // 删除图层
    void remove_layer(const string &name); // 删除图层
    void clear_layer(int index);           // 清空图层
    void clear_layer(const string &name);  // 清空图层
    void show_layer(int index);            // 显示图层
    void show_layer(const string &name);   // 显示图层
    void hide_layer(int index);            // 隐藏图层
    void hide_layer(const string &name);   // 隐藏图层

    void Print_Char(int layer_index, Vector2 scr_pos, wchar_t c, float scale, pixel color); // 打印字符到图层
    void Print_Text(int layer_index, Vector2 start_pos, string text,
                    float scale = 1.0, pixel color = 0xFF000000, Vector2 LC_adjust = Vector2(0, 0)); // 打印文本到图层
    void Print_Text(int layer_index, Vector2 start_pos, Vector2 end_pos, string text,
                    float scale = 1.0, pixel color = 0xFF000000, Vector2 LC_adjust = Vector2(0, 0)); // 打印文本到图层
    void Print_Text(const string &layer_name, Vector2 start_pos, const string &text,
                    float scale = 1.0, pixel color = 0xFF000000, Vector2 LC_adjust = Vector2(0, 0)); // 打印文本到图层
    void Print_Text(const string &layer_name, Vector2 start_pos, Vector2 end_pos, const string &text,
                    float scale = 1.0, pixel color = 0xFF000000, Vector2 LC_adjust = Vector2(0, 0)); // 打印文本到图层

    void draw_image(int layer_index, Vector2 loc, int image_index, Vector2_f pivot);                   // 绘制图片到图层
    void draw_image(const string &layer_name, Vector2 loc, const string &image_name, Vector2_f pivot); // 绘制图片到图层
    void draw_image(const string &layer_name, Vector2 loc, int image_index, Vector2_f pivot);          // 绘制图片到图层
    void draw_pixel(int layer_index, Vector2 loc, pixel color);                                        // 绘制像素到图层
    void draw_line(int layer_index, Vector2 start, Vector2 end, pixel color);                          // 绘制线条到图层
    void draw_line(int layer_index, Vector2 start, Vector2 end, int thickness, pixel color);           // 绘制有粗度的线条到图层
    void draw_rect(int layer_index, Vector2 top_left, Vector2 bottom_right, pixel color);              // 绘制矩形到图层
    void draw_circle(int layer_index, Vector2 center, int radius, pixel color);                        // 绘制圆形到图层

    void LayersInfo(); // 打印图层信息
    void update();     // 更新显示
};
