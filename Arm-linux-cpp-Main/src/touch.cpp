#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue>
#include <cmath>
#include "touch.h"

#define MAX_VECTOR_SIZE 64

using namespace std;

Touch_Manager::Touch_Manager()
{
    should_stop = false;
}

Touch_Manager::~Touch_Manager()
{
    should_stop = true;
}

void *Touch_Manager::Monitor(void *touch_manager)
{
    // 准备好对象和文件
    Touch_Manager *tm = (Touch_Manager *)touch_manager;
    cout << "Touch_Manager::Monitor() is running. should_stop = " << tm->should_stop << endl;

    int FD = open("/dev/input/event0", O_RDONLY);
    if (FD < 0)
    {
        cout << "Touch_Manager::Monitor() failed to open /dev/input/event0." << endl;
        exit(1);
    }
    tm->events.clear();

    input_event Ievent = {{0, 0}, 0, 0, 0};
    queue<input_event> ie3;
    touch_event Tevent = {0, {0, 0}, {0, 0}};
    time_t base_time_sec = time(NULL);
    bool is_touch = false;
    long protect_time = 0;
    // 开始监听，如果主线程要求停止，则停止监听
    while (!tm->should_stop)
    {
        // 读取输入事件
        int bytes_read = read(FD, &Ievent, sizeof(Ievent));
        if (bytes_read != sizeof(Ievent))
        {
            cout << "Touch_Manager::Monitor() failed to read from /dev/input/event0 (size didn't match)" << endl;
            continue;
        }
        // 打印事件信息(debug用)
        // cout << "type = " << Ievent.type << ", code = " << Ievent.code << ", value = " << Ievent.value << '\t';
        // cout << "time = " << Ievent.time.tv_sec - base_time_sec << "." << Ievent.time.tv_usec << endl;

        // 循环判断：
        // 非触碰状态下，用最大3位的队列存储记录(多的话先出一个再进)
        // 如果遇到一个 {EV_KEY, BTN_TOUCH, 1}，则全部出队组成一个touch_event，清空旧容器，放到容器。
        // 然后进入触碰状态。
        // 触碰状态下，每一个type!=0的事件，都组合成一个touch_event，放到容器。
        // 然后退出触碰状态。

        if (!is_touch)
        {
            if (ie3.size() >= 3)
            {
                ie3.pop();
            }
            ie3.push(Ievent);
            if (Ievent.type == EV_KEY && Ievent.code == BTN_TOUCH && Ievent.value == 1)
            {
                tm->events.clear();
                input_event ieX = ie3.front();
                ie3.pop();
                input_event ieY = ie3.front();
                ie3.pop();
                input_event ieT = ie3.front();
                ie3.pop();
                tm->events.push_back({1, {ieX.value * 800 / 1024, ieY.value * 600 / 800}, {ieT.time.tv_sec - base_time_sec, ieT.time.tv_usec}});
                is_touch = true;
            }
        }
        if (is_touch)
        {
            if (Ievent.type == EV_ABS && Ievent.code == ABS_X)
            {
                tm->events.push_back({2, {Ievent.value * 800 / 1024, tm->events.back().pos.y}, {Ievent.time.tv_sec - base_time_sec, Ievent.time.tv_usec}});
            }
            if (Ievent.type == EV_ABS && Ievent.code == ABS_Y)
            {
                tm->events.push_back({2, {tm->events.back().pos.x, Ievent.value * 800 / 1024}, {Ievent.time.tv_sec - base_time_sec, Ievent.time.tv_usec}});
            }
            if (Ievent.type == EV_KEY && Ievent.code == BTN_TOUCH && Ievent.value == 0)
            {
                tm->events.push_back({0, {tm->events.back().pos.x, tm->events.back().pos.y}, {Ievent.time.tv_sec - base_time_sec, Ievent.time.tv_usec}});
                is_touch = false;
            }

            if (tm->events.size() > MAX_VECTOR_SIZE)
            {
                tm->events.erase(tm->events.begin() + 1, tm->events.begin() + (tm->events.size() - MAX_VECTOR_SIZE) + 1);
            }
        }
    }

    // 结束监听，善后工作
    close(FD);
    cout << "Touch_Manager::Monitor() stopped listening." << endl;
    return nullptr;
}

void Touch_Manager::Start_Listen()
{
    should_stop = false;
    if (pthread_create(&Monitor_Thread, NULL, Monitor, this) != 0)
    {
        cout << "Touch_Manager::Start_Listen() failed to create Monitor_Thread." << endl;
        exit(1);
    }
    cout << "Touch_Manager::Start_Listen() succussed to create Monitor_Thread." << endl;
    return;
}

void Touch_Manager::Stop_Listen()
{
    should_stop = true;
    if (pthread_join(Monitor_Thread, NULL) != 0)
    {
        cout << "Touch_Manager::Stop_Listen() failed to join Monitor_Thread." << endl;
        exit(-1);
    }
    cout << "Touch_Manager::Stop_Listen() succussed to join Monitor_Thread." << endl;
    return;
}

void Touch_Manager::print_events()
{
    cout << "\ni\ttype\tx\ty\ttime" << endl;
    for (int i = 0; i < events.size(); i++)
    {
        cout << i << '\t' << events[i].type << "\t" << events[i].pos.x << "\t" << events[i].pos.y << "\t" << events[i].time.tv_sec << "." << events[i].time.tv_usec << endl;
    }
}

bool Touch_Manager::Is_Pressing()
{
    // 无事件或最后一个是松开事件，则代表未按下。其他情况代表按下。
    if (events.empty() || events.back().type == 0)
    {
        return false;
    }
    return true;
}

bool Touch_Manager::Is_Moving(int threshold)
{
    if (events.empty() || events.size() < 2)
    {
        return false;
    }
    // 按照延时往前搜索，如果这段事件存在较大位移动，则认为在移动。
    int base_time = events.front().time.tv_sec * 1000 + events.front().time.tv_usec / 1000;
    int maxD = 10;
    for (int i = events.size() - 1; i >= 0; i--)
    {
        if (events[i].time.tv_sec * 1000 + events[i].time.tv_usec / 1000 - base_time > threshold)
        {
            break;
        }
        if (events[i].type != 2)
        {
            continue;
        }
        int dx = events[i].pos.x - events[i - 1].pos.x;
        int dy = events[i].pos.y - events[i - 1].pos.y;
        if (dx * dx + dy * dy > maxD * maxD)
        {
            return true;
        }
    }
    return false;
}

Vector2 Touch_Manager::Get_Start_Position()
{
    // 如果没有输入，返回{-1,-1}代表无效坐标。其余情况返回第一个事件的坐标。
    if (events.empty())
    {
        return Vector2{-1, -1};
    }
    return events.front().pos;
}

Vector2 Touch_Manager::Get_Current_Position()
{
    // 如果没有输入，返回{-1,-1}代表无效坐标。其余情况返回最后一个事件的坐标。
    if (events.empty())
    {
        return Vector2{-1, -1};
    }
    return events.back().pos;
}

Vector2 Touch_Manager::Get_Direction(int tms, bool consider_last_touch)
{
    // 事件量不够，返回空。
    // 不考虑上一次触控，现在又没在触摸，则返回空。
    int size_now = events.size();
    if (size_now < 2 || (!consider_last_touch && !Is_Pressing()))
    {
        return Vector2{0, 0};
    }

    // 已经至少有两个操作，计算能否直接返回
    Vector2 end_pos = events[size_now - 1].pos;
    int end_time = events[size_now - 1].time.tv_sec * 1000 + events[size_now - 1].time.tv_usec / 1000;
    Vector2 start_pos = events[size_now - 2].pos;
    int start_time = events[size_now - 2].time.tv_sec * 1000 + events[size_now - 2].time.tv_usec / 1000;
    if (size_now == 2 || end_time - start_time > tms)
    {
        return Vector2{end_pos.x - start_pos.x, end_pos.y - start_pos.y};
    }

    // 开始指针继续前移动，时间满足或者到头，计算并返回
    for (int i = size_now - 3; i >= 0; i--)
    {
        start_time = events[i].time.tv_sec * 1000 + events[i].time.tv_usec / 1000;
        start_pos = events[i].pos;
        if (end_time - start_time > tms)
        {
            break;
        }
    }

    return Vector2{end_pos.x - start_pos.x, end_pos.y - start_pos.y};
}

float Touch_Manager::Get_Speed(int tms, bool consider_last_touch)
{
    int size_now = events.size();
    if (size_now < 2 || (!consider_last_touch && !Is_Pressing()))
    {
        return 0.0f;
    }

    // 已经至少有两个操作，计算能否直接返回
    Vector2 end_pos = events[size_now - 1].pos;
    int end_time = events[size_now - 1].time.tv_sec * 1000 + events[size_now - 1].time.tv_usec / 1000;
    Vector2 start_pos = events[size_now - 2].pos;
    int start_time = events[size_now - 2].time.tv_sec * 1000 + events[size_now - 2].time.tv_usec / 1000;
    if (size_now == 2 || end_time - start_time > tms)
    {
        Vector2 dxy = end_pos - start_pos;
        int dxy_len = sqrt(dxy.x * dxy.x + dxy.y * dxy.y);
        return dxy_len * 1000 / (end_time - start_time); // 单位：像素/ms
    }

    // 开始指针继续前移动，时间满足或者到头，计算并返回
    for (int i = size_now - 3; i >= 0; i--)
    {
        start_time = events[i].time.tv_sec * 1000 + events[i].time.tv_usec / 1000;
        start_pos = events[i].pos;
        if (end_time - start_time > tms)
        {
            break;
        }
    }

    Vector2 dxy = end_pos - start_pos;
    int dxy_len = sqrt(dxy.x * dxy.x + dxy.y * dxy.y);
    return dxy_len * 1000 / (end_time - start_time); // 单位：像素/s
}

int Touch_Manager::Get_Duration(bool without_moving)
{
    // 按压持续时间。
    if (events.empty())
    {
        return 0;
    }
    int base_time = events.back().time.tv_sec * 1000 + events.back().time.tv_usec / 1000;
    int end_time = events.front().time.tv_sec * 1000 + events.front().time.tv_usec / 1000;
    if (without_moving)
    {
        for (int i = events.size() - 1; i >= 0; i--)
        {
            if (events[i].type == 2)
            {
                int dx = events[i].pos.x - events.back().pos.x;
                int dy = events[i].pos.y - events.back().pos.y;
                if (dx * dx + dy * dy > 100)
                {
                    end_time = events[i].time.tv_sec * 1000 + events[i].time.tv_usec / 1000;
                    break;
                }
            }
        }
    }
    return base_time - end_time;
}
