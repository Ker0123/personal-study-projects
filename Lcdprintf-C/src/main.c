#include <stdio.h>
#include "lcdprintf.h"

int main()
{
    char text[] = "生成已完成,但出现错误.\n*  终端进程启动失败(退出代码: -1).\n*  终端将被任务重用,按任意键关闭.\n*  正在执行任务:arm-linux-gcc 生成活动文件";
    LCD_printf_init();
    Load_font("./font image han.bmp", "./font image han index.txt", (Vector2){32, 250});
    LCD_print_string((Vector2){0, 0}, "Hello World!\n你好,世界!", 0xff708090);
    LCD_print_string_options(
        (Vector2){0, 200},   // 起点坐标
        (Vector2){500, 400}, // 终点坐标
        text,                // 字符串
        0xffffa0a0,          // 颜色
        0.5,                 // 缩放比例
        (Vector2){-5, 0}     // 字距行距调整
    );
}