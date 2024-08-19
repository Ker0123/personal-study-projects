#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <jpeglib.h>
#include <cmath>
#include <algorithm>
#include "vector2.h"
#include "lcd.h"
#include "touch.h"
#include "game2048.h"
#include <fcntl.h>

using namespace std;

void LayersTest()
{
    LCD_Manager lcd;

    lcd.load_image("./res/simple sence image/background.bmp", "0-背景");
    lcd.load_image("./res/simple sence image/sun.bmp", "1-太阳");
    lcd.load_image("./res/simple sence image/ground.bmp", "2-地面");
    lcd.load_image("./res/simple sence image/butterfly.bmp", "3-蝴蝶");
    lcd.load_image("./res/simple sence image/sign.bmp", "4-标志");

    lcd.add_layer("0-背景图层");
    lcd.add_layer("1-太阳图层");
    lcd.add_layer("2-地面图层");
    lcd.add_layer("3-蝴蝶图层");
    lcd.add_layer("4-标志图层");

    // 绘制静态图层
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.draw_image(2, {0, 0}, 2, {0, 0});
    lcd.draw_image(4, {0, 0}, 4, {0, 0});

    // 绘制动态图层,之后让他们动起来
    lcd.draw_image(1, {400, 240}, 1, {0.5, 0.5});
    lcd.draw_image(3, {500, 350}, 3, {0.5, 0.5});

    // 乱动
    int x = 500;
    int y = 350;
    int sun_y = 240;
    int xspeed, yspeed;
    int dt;
    srand(time(NULL));
    while (true)
    {
        dt = 15 + rand() % 30;     // 运动时间(frames)
        xspeed = rand() % 20 - 10; // x方向速度
        yspeed = rand() % 20 - 10; // y方向速度

        while (dt--)
        {
            // 大方向确定下小方向微变
            xspeed += rand() % 3 - 1;
            yspeed += rand() % 3 - 1;

            int new_x = x + xspeed;
            int new_y = y + yspeed;
            if (new_x < 200 || new_x > 880)
            {
                xspeed = -xspeed;
            }
            if (new_y < 200 || new_y > 450)
            {
                yspeed = -yspeed;
            }
            x += xspeed;
            y += yspeed;

            if (sun_y-- > 80)
            {
                lcd.clear_layer(1);
                lcd.draw_image(1, {400, sun_y}, 1, {0.5, 0.5});
            }
            lcd.clear_layer(3);
            lcd.draw_image(3, {x, y}, 3, {0.5, 0.5});

            lcd.update();
        }
    }
}

void TouchTest()
{
    Touch_Manager touch;
    touch.Start_Listen();

    int cnt = 0;
    while (cnt < 10)
    {
        cout << "TouchTest() is running... cnt = " << cnt++ << endl;
        usleep(700000);
    }

    touch.Stop_Listen();
}

void TouchTest_Message()
{
    LCD_Manager lcd;
    Touch_Manager touch;
    touch.Start_Listen();
    lcd.add_layer("background");
    lcd.load_image("./res/background.bmp", "background");
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();
    int color = 0xff808080;
    int add = 0x00040201;
    touch_event one;
    Vector2 lastPos = {0, 0};
    while (true)
    {
        // usleep(1000000);
        // lcd.update();
        // cout << "------------------" << endl;
        // cout << "当前输入事件队列: " << endl;
        // cout << "|type\t|x\t|y\t|time\t|" << endl;
        // for (touch_event one : touch.events)
        // {
        //     cout << one.type << "\t" << one.pos.x << "\t" << one.pos.y << "\t" << one.time.tv_sec << '.' << one.time.tv_usec << endl;
        // }
        // cout << "------------------" << endl;
        // if (touch.events.empty())
        // {
        //     continue;
        // }
        // one = touch.events.back();
        // if (one.pos.x == lastPos.x && one.pos.y == lastPos.y)
        // {
        //     continue;
        // }
        // lastPos = one.pos;
        // if (0xff000000 > color + add || color + add > 0xffffffff)
        // {
        //     add *= -1;
        // }
        // color += add;
        // switch (one.type)
        // {
        // case 1:
        //     lcd.draw_dot(one.pos.x, one.pos.y, 10, color);
        //     break;
        // case 2:
        //     lcd.draw_dot(one.pos.x, one.pos.y, 10, color);
        //     break;
        // case 0:
        //     lcd.draw_dot(one.pos.x, one.pos.y, 10, color);
        //     break;
        // default:
        //     break;
        // }
    }
}

void TouchAPITest()
{
    // 初始化类、清屏
    Touch_Manager tm;
    LCD_Manager lcd;
    lcd.add_layer("0 - 背景");
    lcd.add_layer("1 - 画布");
    lcd.load_image("./res/background.bmp", "0 - 背景图片");
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();

    Vector2 touch_pos;
    bool clear_flag = false;
    // 启用触摸监听，主程序绘制触摸反馈和输出接口获取的信息
    tm.Start_Listen();
    while (true)
    {
        touch_pos = tm.Get_Current_Position();
        // 未开始触摸
        if (touch_pos.x == -1 || touch_pos.y == -1)
        {
            continue;
        }
        // 触摸过程中，不断在当前触摸点绘制图形，松开后会打印一次各接口信息，再次按下会先清除之前绘制的图形。
        if (tm.Is_Pressing())
        {
            if (clear_flag)
            {
                lcd.clear_layer(1);
                clear_flag = false;
            }
            lcd.draw_circle(1, touch_pos, 5, 0xfff0f0f0);
            lcd.update();
        }
        else
        {
            if (!clear_flag)
            {
                clear_flag = true;
            }
        }
    }
}

void JPEGTest()
{
    LCD_Manager lcd;
    lcd.add_layer("0 - 测试用单图层");
    lcd.load_image("./res/jpegTest.jpeg", "0 - 测试用jpeg图片");
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();
}

void drawHint(LCD_Manager &lcd, int layerIndex, int x, int length)
{
    lcd.draw_rect(layerIndex, {x + 2, 474}, {x + length - 2, 480}, 0xff808080);
    lcd.draw_rect(layerIndex, {x + 1, 475}, {x + length - 1, 479}, 0xff909090);
    lcd.draw_rect(layerIndex, {x, 476}, {x + length, 478}, 0xffa0a0a0);
}

void PictureTest()
{
    LCD_Manager lcd;
    Touch_Manager tm;

    lcd.add_layer("背景");
    lcd.add_layer("当前图片");
    lcd.add_layer("新图片");
    lcd.add_layer("UI");
    lcd.load_image("./res/background.bmp", "背景图片"); // 0
    lcd.load_image("./res/WQ.bmp", "万能青年旅店");     // 1
    lcd.load_image("./res/head.bmp", "头像");
    lcd.load_image("./res/h2-export.jpeg", "头像2");
    lcd.load_image("./res/83828890_p0.jpeg", "雷电");
    lcd.load_image("./res/84214638_p0_1.jpeg", "夕阳");
    lcd.load_image("./res/87420887_p0.jpeg", "天台"); // 6

    lcd.draw_image("背景", {0, 0}, "头像", {0, 0});

    tm.Start_Listen();

    vector<int> img_indexs = {1, 3, 4, 5, 6};
    int img_before = img_indexs.size() - 1;
    int img_forward = 0;
    int img_after = 1 % img_indexs.size();
    bool was_pressing = false;
    int dx = 0;

    lcd.draw_image("当前图片", {400, 240}, img_indexs[img_forward], {0.5, 0.5});
    lcd.update();

    int hint_time;
    float speed;

    while (true)
    {
        bool is_pressing = tm.Is_Pressing();
        // 按下时，根据起始位置和当前位置改变前中后3张图片的位置，并刷新显示。
        if (is_pressing)
        {
            was_pressing = true;
            dx = tm.Get_Current_Position().x - tm.Get_Start_Position().x;
            if (abs(dx) > 10)
            {
                lcd.clear_layer("当前图片");
                lcd.draw_image("当前图片", {400 + dx, 240}, img_indexs[img_forward], {0.5, 0.5});
                int new_img_pos = dx > 0 ? 400 - 800 + dx : 400 + 800 + dx;
                int new_img = dx > 0 ? img_before : img_after;
                lcd.clear_layer("新图片");
                lcd.draw_image("新图片", {new_img_pos, 240}, img_indexs[new_img], {0.5, 0.5});
                lcd.update();
            }
        }
        // 按下到松开时，根据滑动方向更新当前、中、后3张图片，并刷新显示。
        if (!is_pressing && was_pressing)
        {
            was_pressing = false;
            cout << "speed : " << tm.Get_Speed(200, true) << " pixel/s" << endl;
            if (abs(dx) > 200 || tm.Get_Speed(200, true) > 999)
            {
                if (dx > 0)
                {
                    img_after = img_forward;
                    img_forward = img_before;
                    img_before = (img_before + img_indexs.size() - 1) % img_indexs.size();
                }
                if (dx < 0)
                {
                    img_before = img_forward;
                    img_forward = img_after;
                    img_after = (img_after + 1) % img_indexs.size();
                }
            }
            lcd.clear_layer("当前图片");
            lcd.clear_layer("新图片");
            lcd.clear_layer("UI");
            lcd.draw_image("当前图片", {400, 240}, img_indexs[img_forward], {0.5, 0.5});

            // 刷新UI层的位置指示器
            drawHint(lcd, 3, 800 * img_forward / img_indexs.size(), 800 / img_indexs.size());
            hint_time = 1000000;
            lcd.update();
        }
        if (!is_pressing && !was_pressing)
        {
            if (hint_time-- == 0)
            {
                lcd.clear_layer("UI");
                lcd.update();
            }
        }
    }
}

void DrawBoardTest()
{
    LCD_Manager lcd;
    lcd.add_layer("0 - 背景");
    lcd.add_layer("1 - 画布");
    lcd.add_layer("2 - 箭头");
    lcd.load_image("./res/background.bmp", "0 - 背景图片");
    lcd.load_image("./res/arrow_up.bmp", "1 - 上箭头");
    lcd.load_image("./res/arrow_down.bmp", "2 - 下箭头");
    lcd.load_image("./res/arrow_left.bmp", "3 - 左箭头");
    lcd.load_image("./res/arrow_right.bmp", "4 - 右箭头");
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();

    Touch_Manager tm;
    tm.Start_Listen();

    int color;
    short speedColor[3] = {0x00, 0x80, 0x00};
    Vector2 lastPos = {-1, -1};
    Vector2 touchPos = {-1, -1};

    while (true)
    {
        // 按下时绘制起始点，按下途中绘制当前触摸位置到上一帧位置的线段。线段的颜色由两位置间的距离决定。
        // 按下时还同时根据当前滑动方向绘制箭头。
        if (tm.Is_Pressing())
        {
            touchPos = tm.Get_Current_Position();
            if (lastPos.x == -1)
            {
                lcd.draw_circle(1, touchPos, 4, 0xff5b6ee1); // 开始位置点
                lcd.update();
            }
            if (touchPos.x != -1 && lastPos.x != -1)
            {
                int speed = (int)sqrt(pow(touchPos.x - lastPos.x, 2) + pow(touchPos.y - lastPos.y, 2)); // 0~10~800~对角线长度
                // 将速度限制到0～400,理想的速度区间
                if (speed > 400)
                {
                    speed = 400;
                }
                // 将速度映射到0x00~0xff，现在速度可视作一个灰度颜色值
                speed = (int)(speed * 255 / 400);
                // 利用该灰度值提升原色亮度
                short R = 0x5b;
                short G = 0x6e;
                short B = 0xe1;
                R = 0xff < R + speed ? 0xff : R + speed;
                G = 0xff < G + speed ? 0xff : G + speed;
                B = 0xff < B + speed ? 0xff : B + speed;
                color = 0xff000000 | R << 16 | G << 8 | B;
                lcd.draw_line(1, lastPos, touchPos, 2, color); // 路径: 速度越快颜色越浅
                lcd.update();
            }
            lastPos = touchPos;

            // 根据滑动方向绘制箭头
            Vector2 slideDir = tm.Get_Direction();
            int length = (int)sqrt(slideDir.x * slideDir.x + slideDir.y * slideDir.y);
            if (length > 14)
            {
                int img_index = 0;
                // 根据向量计算角度
                double angle = atan2(slideDir.y, slideDir.x) * 180 / M_PI;
                // cout << "angle = " << angle << endl;
                if (-135 <= angle && angle < -45)
                {
                    img_index = 1;
                }
                if (-45 <= angle && angle < 45)
                {
                    img_index = 4;
                }
                if (45 <= angle && angle < 135)
                {
                    img_index = 2;
                }
                if (135 <= angle || angle < -135)
                {
                    img_index = 3;
                }
                lcd.clear_layer(2);
                lcd.draw_image(2, {160, 96}, img_index, {0.5, 0.5});
                lcd.update();
            }
        }
        // 松开时绘制结束点并刷新显示，然后清除画布(数据上，不刷新显示)。
        else
        {
            if (touchPos.x != -1)
            {
                lcd.draw_circle(1, touchPos, 3, 0xffcbdbfc); // 结束位置: 蓝点
                lcd.update();
            }
            lastPos = {-1, -1};
            touchPos = {-1, -1};
            lcd.clear_layer(1);
        }
    }
}

void display2048(LCD_Manager *lcd, int layer_index, int images_index[11], int board[4][4], Vector2 loc)
{
    // 算出16个格子的位置
    Vector2 block_pos[4][4];
    for (int l = 0; l < 4; l++)
    {
        for (int c = 0; c < 4; c++)
        {
            block_pos[l][c] = {loc.x + c * 80, loc.y + l * 80};
        }
    }

    // 绘制
    lcd->clear_layer(layer_index);
    for (int l = 0; l < 4; l++)
    {
        for (int c = 0; c < 4; c++)
        {
            int num = board[l][c];
            if (num == 0)
            {
                continue;
            }
            int img_index = log2(num) - 1;
            lcd->draw_image(layer_index, block_pos[l][c], images_index[img_index], {0, 0});
        }
    }
}

void game2048Test()
{
    Game2048 game;
    LCD_Manager lcd;
    lcd.add_layer("0 - 背景");
    lcd.add_layer("1 - 画布");
    lcd.add_layer("2 - 箭头");
    lcd.add_layer("3 - 2048");
    lcd.load_image("./res/back_of_2048.bmp", "0 - 背景图片");
    lcd.load_image("./res/arrow_up.bmp", "1 - 上箭头");
    lcd.load_image("./res/arrow_down.bmp", "2 - 下箭头");
    lcd.load_image("./res/arrow_left.bmp", "3 - 左箭头");
    lcd.load_image("./res/arrow_right.bmp", "4 - 右箭头");
    lcd.load_image("./res/2048_2.bmp", "5 - 数字2");
    lcd.load_image("./res/2048_4.bmp", "6 - 数字4");
    lcd.load_image("./res/2048_8.bmp", "7 - 数字8");
    lcd.load_image("./res/2048_16.bmp", "8 - 数字16");
    lcd.load_image("./res/2048_32.bmp", "9 - 数字32");
    lcd.load_image("./res/2048_64.bmp", "10 - 数字64");
    lcd.load_image("./res/2048_128.bmp", "11 - 数字128");
    lcd.load_image("./res/2048_256.bmp", "12 - 数字256");
    lcd.load_image("./res/2048_512.bmp", "13 - 数字512");
    lcd.load_image("./res/2048_1024.bmp", "14 - 数字1024");
    lcd.load_image("./res/2048_2048.bmp", "15 - 数字2048");
    lcd.load_image("./res/back.bmp", "15 - 数字背景");
    int imgIndex[11] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();

    Touch_Manager tm;
    tm.Start_Listen();

    int color;
    short speedColor[3] = {0x00, 0x80, 0x00};
    Vector2 lastPos = {-1, -1};
    Vector2 touchPos = {-1, -1};
    Vector2 slideDir;
    int img_index;

    while (true)
    {
        // 按下时绘制起始点，按下途中绘制当前触摸位置到上一帧位置的线段。线段的颜色由两位置间的距离决定。
        // 按下时还同时根据当前滑动方向绘制箭头。
        if (tm.Is_Pressing())
        {
            touchPos = tm.Get_Current_Position();
            if (lastPos.x == -1)
            {
                lcd.draw_circle(1, touchPos, 4, 0xff5b6ee1); // 开始位置点
                lcd.update();
            }
            if (touchPos.x != -1 && lastPos.x != -1)
            {
                int speed = (int)sqrt(pow(touchPos.x - lastPos.x, 2) + pow(touchPos.y - lastPos.y, 2)); // 0~10~800~对角线长度
                // 将速度限制到0～400,理想的速度区间
                if (speed > 400)
                {
                    speed = 400;
                }
                // 将速度映射到0x00~0xff，现在速度可视作一个灰度颜色值
                speed = (int)(speed * 255 / 400);
                // 利用该灰度值提升原色亮度
                short R = 0x5b;
                short G = 0x6e;
                short B = 0xe1;
                R = 0xff < R + speed ? 0xff : R + speed;
                G = 0xff < G + speed ? 0xff : G + speed;
                B = 0xff < B + speed ? 0xff : B + speed;
                color = 0xff000000 | R << 16 | G << 8 | B;
                lcd.draw_line(1, lastPos, touchPos, 2, color); // 路径: 速度越快颜色越浅
                lcd.update();
            }
            lastPos = touchPos;

            // 根据滑动方向绘制箭头
            slideDir = tm.Get_Direction();
            int length = (int)sqrt(slideDir.x * slideDir.x + slideDir.y * slideDir.y);
            if (length > 14)
            {
                img_index = 0;
                // 根据向量计算角度
                double angle = atan2(slideDir.y, slideDir.x) * 180 / M_PI;
                // cout << "angle = " << angle << endl;
                if (-135 <= angle && angle < -45)
                {
                    img_index = 1;
                }
                if (-45 <= angle && angle < 45)
                {
                    img_index = 4;
                }
                if (45 <= angle && angle < 135)
                {
                    img_index = 2;
                }
                if (135 <= angle || angle < -135)
                {
                    img_index = 3;
                }
                lcd.clear_layer(2);
                lcd.draw_image(2, {220, 240}, img_index, {0.5, 0.5});
                lcd.update();
            }
        }
        // 松开时绘制结束点并刷新显示，然后清除画布(数据上，不刷新显示)。
        else
        {
            if (touchPos.x != -1)
            {
                lcd.draw_circle(1, touchPos, 3, 0xffcbdbfc); // 结束位置: 蓝点
                switch (img_index)
                {
                case 1:
                    game.Move('u');
                    game.Update_Display();
                    display2048(&lcd, 3, imgIndex, game.board, {440, 60});
                    break;
                case 2:
                    game.Move('d');
                    game.Update_Display();
                    display2048(&lcd, 3, imgIndex, game.board, {440, 60});
                    break;
                case 3:
                    game.Move('l');
                    game.Update_Display();
                    display2048(&lcd, 3, imgIndex, game.board, {440, 60});
                    break;
                case 4:
                    game.Move('r');
                    game.Update_Display();
                    display2048(&lcd, 3, imgIndex, game.board, {440, 60});
                    break;

                default:
                    break;
                }
                lcd.update();
            }
            lastPos = {-1, -1};
            touchPos = {-1, -1};
            lcd.clear_layer(1);
        }
    }
}

void newDrawTest()
{
    LCD_Manager lcd;
    lcd.add_layer("背景");
    lcd.add_layer("画布");
    lcd.load_image("./res/background.bmp", "背景图片");
    lcd.load_image("./res/testImage03.bmp", "测试图片");
    lcd.draw_image(0, {0, 0}, 0, {0, 0});
    lcd.update();

    int x = 0;
    int y = 0;
    while (true)
    {
        lcd.clear_layer(1);
        lcd.draw_image(1, {x, y}, 1, {0, 0});
        lcd.update();
        cout << "\n当前图像位置: " << "(" << x << "," << y << ")" << endl;
        cout << "输入新图像位置: ";
        cin >> x >> y;
    }
}

void TouchEventOutput()
{
    Touch_Manager tm;
    tm.Start_Listen();

    bool was_pressing;
    bool is_pressing;
    int events_size = 10969;
    touch_event event;
    while (true)
    {
        is_pressing = tm.Is_Pressing();
        if (!was_pressing && is_pressing)
        {
            event = tm.events.back();
            cout << "\n================================================" << endl;
            cout << "按下: " << event.pos << "  时间: " << event.time.tv_sec << "." << event.time.tv_usec << endl;
            was_pressing = true;
        }

        if (was_pressing && is_pressing)
        {
            if (events_size != tm.events.size())
            {
                touch_event event = tm.events.back();
                cout << "移动: " << event.pos << "  时间: " << event.time.tv_sec << "." << event.time.tv_usec << endl;
                events_size = tm.events.size();
            }
        }

        if (was_pressing && !is_pressing)
        {
            touch_event event = tm.events.back();
            cout << "松开: " << event.pos << "  时间: " << event.time.tv_sec << "." << event.time.tv_usec << endl;
            cout << "------------------------------------------------" << endl;
            cout << "方向: " << tm.Get_Direction(100, true) << "  速度: " << tm.Get_Speed(100, true) << " (最后100ms)" << endl;
            cout << "================================================" << endl;
            was_pressing = false;
        }
    }
}

pixel color_trans(pixel color, int one_step)
{
    // 找到3色值中最小、最大、中间的。
    int rgb[3] = {(color & 0xff0000) >> 16, (color & 0x00ff00) >> 8, (color & 0x0000ff)};
    // cout << "recived rgb = " << rgb[0] << " " << rgb[1] << " " << rgb[2] << endl;
    int min = 0, mid = 0, max = 0;
    for (int i = 0; i < 3; i++)
    {
        if (rgb[i] < rgb[min])
            min = i;
        if (rgb[i] > rgb[max])
            max = i;
    }
    for (int i = 0; i < 3; i++)
    {
        if (i != min && i != max)
        {
            mid = i;
        }
    }
    // 0. 有2个最大值时，max指向最小值左侧；有0个或者3个最大值时，不做操作
    // 1. 如果max指针左侧的值不是最小值，max左侧的值减小，结束。
    // 2. 如果max指针右侧的值不是最大值，max右侧的值增大，结束。
    if (rgb[min] == rgb[mid] && rgb[mid] == rgb[max])
    {
        // cout << "有0个或者3个最大值时, 不做操作" << endl;
        return color;
    }
    if (rgb[max] == rgb[mid] && (max + 1) % 3 != min)
    {
        // cout << "有2个最大值时, max指向最小值左侧" << endl;
        int temp = max;
        max = mid;
        mid = temp;
    }
    if (rgb[(max - 1 + 3) % 3] > rgb[min])
    {
        // cout << "max指针左侧的值不是最小值" << endl;
        int max_left = (max - 1 + 3) % 3;
        rgb[max_left] = rgb[max_left] - one_step > rgb[min] ? rgb[max_left] - one_step : rgb[min];
    }
    else if (rgb[(max + 1) % 3] < rgb[max])
    {
        // cout << "max指针右侧的值不是最大值" << endl;
        int max_right = (max + 1) % 3;
        rgb[max_right] = rgb[max_right] + one_step < rgb[max] ? rgb[max_right] + one_step : rgb[max];
    }

    // cout << "new rgb = " << rgb[0] << " " << rgb[1] << " " << rgb[2] << endl;
    return (pixel)(0xff000000 | rgb[0] << 16 | rgb[1] << 8 | rgb[2]);
}

pixel color_trans_light(pixel color, int one_step)
{
    int rgb[3] = {(color & 0xff0000) >> 16, (color & 0x00ff00) >> 8, (color & 0x0000ff)};
    rgb[0] = rgb[0] + one_step;
    rgb[1] = rgb[1] + one_step;
    rgb[2] = rgb[2] + one_step;
    if (rgb[0] < 0)
    {
        rgb[0] = 0;
    }
    if (rgb[1] < 0)
    {
        rgb[1] = 0;
    }
    if (rgb[2] < 0)
    {
        rgb[2] = 0;
    }
    if (rgb[0] > 255)
    {
        rgb[0] = 255;
    }
    if (rgb[1] > 255)
    {
        rgb[1] = 255;
    }
    if (rgb[2] > 255)
    {
        rgb[2] = 255;
    }

    return (pixel)(0xff000000 | rgb[0] << 16 | rgb[1] << 8 | rgb[2]);
}

void FontTest()
{
    LCD_Manager lcd;
    lcd.add_layer("背景");
    lcd.add_layer("画布");
    lcd.load_image("./res/head 800x480.bmp", "背景图片");
    lcd.Load_FontImage("./res/font image/fangsung_font.bmp");
    lcd.draw_image("背景", {0, 0}, "背景图片", {0, 0});

    pixel color1 = 0xffff0000;
    pixel color2 = 0xff00ff00;
    pixel color3 = 0xff404040;
    Vector2 start(300, 240);
    Vector2 end(750, 420);
    int move_dir = -2;
    float scale = 1.0;
    float scale_change = 0.01;
    int light_step = 4;
    int font_change_cnt = 100;
    int font_type = 0;
    while (true)
    {
        // 字体切换
        if (font_change_cnt == 0)
        {
            if (font_type == 0)
            {
                lcd.Load_FontImage("./res/font image/fangsung_font.bmp");
                font_type = 1;
            }
            else
            {
                lcd.Load_FontImage("./res/font image/DejaVu Sans Mono 16x6 - 32x64.bmp");
                font_type = 0;
            }
            font_change_cnt = 100;
        }
        else
        {
            font_change_cnt--;
        }
        // 颜色渐变
        color1 = color_trans(color1, 10);
        color2 = color_trans(color2, 10);

        // 位置渐变
        start.x += move_dir;
        if (start.x > 500 || start.x < 100)
        {
            move_dir *= -1;
        }

        // 大小渐变
        scale += scale_change;
        if (scale > 1.2 || scale < 0.5)
        {
            scale_change *= -1;
        }

        // 亮度渐变
        color3 = color_trans_light(color3, light_step);
        if (color3 <= 0xff202020 || color3 >= 0xffc0c0c0)
        {
            light_step *= -1;
        }

        lcd.clear_layer("画布");
        lcd.Print_Text("画布", {10, 10}, "Can you hear me?", 1.0, color3);
        lcd.Print_Text("画布", {10, 80}, "I'm trying to hear you.", 1.0, color1);
        lcd.Print_Text("画布", {10, 160}, "Slince strikes like a hurricane.", scale, 0xff5b6ee1);
        lcd.draw_line(1, {start.x - 10, 240}, {start.x - 10, 450}, 5, 0xff309090);
        lcd.draw_line(1, {end.x + 20, 240}, {end.x + 20, 450}, 5, 0xff309090);
        lcd.Print_Text("画布", start, end, "I'm singing for you. You are screaming on me.\nIt's hard to see your tears, in the pouring rain.", 0.5, 0xff309090);
        lcd.update();
    }
}

void clearScreen()
{
    LCD_Manager lcd;
    lcd.add_layer("背景");
    lcd.load_image("./res/head 800x480.bmp", "背景图片");
    lcd.draw_image("背景", {0, 0}, "背景图片", {0, 0});
    lcd.update();
}

void txtReadTest()
{
    int fd = open("./res/font image/font image han index.txt", O_RDONLY);
    int wch = '\0';
    int ch = true;
    while (ch)
    {
        wch = '\0';
        ch = '\0';
        read(fd, &ch, 1);
        if (0x00 <= ch && ch <= 0x7f) // 1byte编码形式
        {
            wch = ch;
        }
        else if (0xC0 <= ch && ch <= 0xDF) // 2byte编码形式
        {
            wch = ch;
            read(fd, &ch, 1);
            wch = (wch << 8) | ch;
        }
        else if (0xE0 <= ch && ch <= 0xEF) // 3byte编码形式
        {
            wch = ch;
            read(fd, &ch, 2);
            wch = (wch << 16) | ch;
        }
        else if (0xF0 <= ch && ch <= 0xF7) // 4byte编码形式
        {
            wch = ch;
            read(fd, &ch, 3);
            wch = (wch << 24) | ch;
        }
    }
}

int main()
{
    cout << "---------------------------------" << endl;
    cout << "Version : " << "V0.4.1" << endl;
    cout << "Last Complie Time : " << __TIME__ << " " << __DATE__ << endl;
    cout << "Log:" << endl;
    cout << "v0.1.2: 并发" << endl;
    cout << "v0.1.3: 触摸输入信息监听、读取、存储" << endl;
    cout << "v0.1.4: 画板功能初步" << endl;
    cout << "v0.1.5: 画板功能完成" << endl;
    cout << "v0.1.6: JPEG图片解码" << endl;
    cout << "v0.1.7: 触控类实用接口实现" << endl;
    cout << "v0.1.8: 优化后的画板" << endl;
    cout << "v0.2.0: 滑动动作丰富的相册" << endl;
    cout << "v0.2.1: 触控信息终端显示测试" << endl;
    cout << "v0.3.0: 完成2048游戏(但是规则有点错误)" << endl;
    cout << "v0.4.0: 加入文字显示功能" << endl;
    cout << "v0.4.1: 基本完成文字功能, 并与lcd.h合并" << endl;
    cout << "---------------------------------" << endl;

    string text = "堆栈异常常见于Linux系统中的应用程序运行过程中, 主要是由于栈溢出, 栈帧损坏, 函数调用错误等原因导致的. \n\
要解决堆栈异常问题, 可以尝试以下步骤: \n\
检查代码逻辑: 首先检查应用程序的代码逻辑,特别是涉及到栈操作的地方.确保函数调用,变量声明等操作正确无误.\n\
检查栈溢出: 堆栈溢出是导致堆栈异常的常见原因.可以通过调整栈大小来解决该问题.可以在编译时使用编译器选项指定栈大小,如 -Wl,--stack,大小.同时,也可以通过优化递归算法,减少局部变量使用等方式来减少栈的使用.\n\
检查函数调用错误: 在函数调用过程中,如果参数传递错误或者函数返回值使用错误,都可能导致堆栈异常.需要仔细检查函数调用的过程,确保参数传递和返回值使用正确.\n\
使用调试工具: 可以使用调试工具来定位堆栈异常的具体位置.常用的调试工具有GDB,Valgrind等.通过调试工具可以查看堆栈状态,变量值等信息,帮助定位问题.\n\
参考系统日志: 如果堆栈异常是由于系统资源不足或者其他系统问题导致的,可以参考系统日志来查找问题.系统日志中会有相应的错误日志,可以帮助定位问题.\n\
升级软件版本: 如果堆栈异常是由于软件版本问题导致的,可以尝试升级到最新版本,看是否能够解决问题.\n\
如果上述方法无法解决堆栈异常问题,可以向相关社区或者开发者论坛等地寻求帮助,通常会有更专业的人员提供指导和解答.";

    LCD_Manager lcd = LCD_Manager();
    lcd.font_inversion = true;
    lcd.Load_FontImage("./res/font image/font image han.bmp", "./res/font image/font image han index.txt", {32, 250});
    lcd.add_layer("背景");
    lcd.add_layer("画布");
    lcd.add_layer("UI");
    lcd.load_image("./res/head 800x480.bmp", "背景图片");
    lcd.draw_image("背景", {0, 0}, "背景图片", {0, 0});
    lcd.Print_Text("画布", {100, 0}, {700, 480}, text, 0.5, 0xffa0a0a0, {-3, 2});

    lcd.font_inversion = false;
    lcd.Print_Text("UI", {0, 0}, {80, 60}, "字号\n +  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {0, 100}, {80, 160}, "字号\n -  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {0, 200}, {80, 260}, "页宽\n +  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {0, 300}, {80, 360}, "页宽\n -  ", 0.5, 0xff404080, {0, 0});

    lcd.Print_Text("UI", {730, 0}, {800, 60}, "字距\n +  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {730, 100}, {800, 160}, "字距\n -  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {730, 200}, {800, 260}, "行距\n +  ", 0.5, 0xff404080, {0, 0});
    lcd.Print_Text("UI", {730, 300}, {800, 360}, "行距\n -  ", 0.5, 0xff404080, {0, 0});

    lcd.Print_Text("UI", {0, 400}, {80, 460}, "变色\n .. ", 0.5, 0xff404080, {0, 0});

    lcd.font_inversion = true;

    int font_size_modi = 0;
    int boarder_modi = 0;
    int wide_modi = 0;
    int line_space_modi = 0;
    pixel color_modi = 0xff404080;

    lcd.update();

    Touch_Manager tm;
    tm.Start_Listen();
    int old_text_pos_y = 0;
    int new_text_pos_y = 0;
    while (true)
    {
        if (tm.Is_Pressing())
        {
            Vector2 start_pos = tm.Get_Start_Position();
            Vector2 current_pos = tm.Get_Current_Position();
            if (0 < current_pos.x && current_pos.x < 80 && 0 < current_pos.y && current_pos.y < 60)
                font_size_modi++;
            else if (0 < current_pos.x && current_pos.x < 80 && 100 < current_pos.y && current_pos.y < 160)
                font_size_modi--;
            else if (0 < current_pos.x && current_pos.x < 80 && 200 < current_pos.y && current_pos.y < 260)
                boarder_modi++;
            else if (0 < current_pos.x && current_pos.x < 80 && 300 < current_pos.y && current_pos.y < 360)
                boarder_modi--;
            else if (0 < current_pos.x && current_pos.x < 80 && 400 < current_pos.y && current_pos.y < 460)
            {
                color_modi = color_trans(color_modi, 10);
                lcd.font_inversion = false;
                lcd.Print_Text("UI", {0, 400}, {80, 460}, "变色\n .. ", 0.5, color_modi, {0, 0});
                lcd.font_inversion = true;
            }
            else if (730 < current_pos.x && current_pos.x < 800 && 0 < current_pos.y && current_pos.y < 60)
                wide_modi++;
            else if (730 < current_pos.x && current_pos.x < 800 && 100 < current_pos.y && current_pos.y < 160)
                wide_modi--;
            else if (730 < current_pos.x && current_pos.x < 800 && 200 < current_pos.y && current_pos.y < 260)
                line_space_modi++;
            else if (730 < current_pos.x && current_pos.x < 800 && 300 < current_pos.y && current_pos.y < 360)
                line_space_modi--;
            else if (100 < current_pos.x && current_pos.x < 700)
            {
                Vector2 dir = current_pos - start_pos;
                new_text_pos_y = old_text_pos_y + dir.y;
            }
            lcd.clear_layer("画布");
            lcd.Print_Text("画布", {100 - boarder_modi, new_text_pos_y}, {700 + boarder_modi, 480}, text, 0.5 + font_size_modi * 0.01, color_modi, {-3 + wide_modi, 2 + line_space_modi});
            lcd.update();
            printf("font_size_modi = %d, boarder_modi = %d, wide_modi = %d, line_space_modi = %d\n", font_size_modi, boarder_modi, wide_modi, line_space_modi);
        }
        if (!tm.Is_Pressing())
        {
            old_text_pos_y = new_text_pos_y;
        }
    }

    return 0;
}