#pragma once

#include <linux/input.h>
#include <pthread.h>
#include <queue>

#pragma once
#include "vector2.h"

using namespace std;

/* Touch_Manager Hint
 * 应当有一个开始运行的函数，负责新开线程，监听触摸事件。
 * 该函数在按下时开始将每帧的触摸事件存入队列，并在松开时结束。(为防止队列溢出，事件过多时不再持续入队，只在松开时入队)
 * 另有获取各种操作类型的函数，如是否按下、按下位置、是否划动，划动方向、甚至是划动的方向和速度。
 * 以上提到的数据，可以在每次存入触摸事件的时候，结合以有数据进行计算，实时更新到类的成员变量中。
 * 获取函数简单地返回上述成员变量即可。
 */

struct touch_event
{
    __u16 type; // 1:按下 0:松开 2:移动
    Vector2 pos;
    timeval time;
};

class Touch_Manager
{
public:
    Touch_Manager();
    ~Touch_Manager();

    pthread_t Monitor_Thread;   // 监听线程
    vector<touch_event> events; // 触摸事件容器

    bool should_stop; // 停止监听标志位
    static void *Monitor(void *touch_manager);
    void Start_Listen();
    void Stop_Listen();

    void print_events();

    bool Is_Pressing();
    bool Is_Moving(int threshold);
    Vector2 Get_Start_Position();
    Vector2 Get_Current_Position();
    Vector2 Get_Direction(int tms = 100,bool consider_last_touch = false);
    float Get_Speed(int tms = 100,bool consider_last_touch = false);
    int Get_Duration(bool without_moving = true);
};