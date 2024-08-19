#pragma once

#define true 1
#define false 0

typedef char bool;
typedef int pixel;
typedef int wchar;

typedef struct Vector2
{
    int x;
    int y;
} Vector2;

typedef struct Font
{
    Vector2 image_size;
    Vector2 word_size;
    pixel *data;
} Font;

pixel *mmap_ptr;
Vector2 font_map[0xEFFFFF];
Font font;
bool font_inversion;

void LCD_printf_init();
bool is_in_range(Vector2 pos);
bool is_han(wchar c);
int get_utf8_len_from_first_byte(char first_byte);

void Load_font(const char *font_file, const char *index_file, Vector2 cut);
void LCD_print_char(Vector2 scr_pos, wchar c, float scale, pixel color);
void LCD_print_string(Vector2 start_pos, char *text, pixel color);
void LCD_print_string_options(Vector2 start_pos, Vector2 end_pos, char *text, pixel color, float scale, Vector2 LC_adjust);