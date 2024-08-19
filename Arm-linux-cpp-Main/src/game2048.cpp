#include "game2048.h"
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <iomanip>

using namespace std;

void Game2048::Move_up()
{
    cout << "Move up" << endl;
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 3; i++)
            for (int p1 = 3, p2 = 2; p2 >= 0; p1--, p2--)
            {
                if ((board[p1][c] != 0 && board[p2][c] == 0) || (board[p1][c] == board[p2][c]))
                {
                    board[p2][c] += board[p1][c];
                    board[p1][c] = 0;
                }
            }
}

void Game2048::Move_down()
{
    cout << "Move down" << endl;
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 3; i++)
            for (int p1 = 0, p2 = 1; p2 <= 3; p1++, p2++)
            {
                if ((board[p1][c] != 0 && board[p2][c] == 0) || (board[p1][c] == board[p2][c]))
                {
                    board[p2][c] += board[p1][c];
                    board[p1][c] = 0;
                }
            }
}

void Game2048::Move_left()
{
    cout << "Move left" << endl;
    for (int l = 0; l < 4; l++)
        for (int i = 0; i < 3; i++)
            for (int p1 = 3, p2 = 2; p2 >= 0; p1--, p2--)
            {
                if ((board[l][p1] != 0 && board[l][p2] == 0) || (board[l][p1] == board[l][p2]))
                {
                    board[l][p2] += board[l][p1];
                    board[l][p1] = 0;
                }
            }
}

void Game2048::Move_right()
{
    cout << "Move right" << endl;
    for (int l = 0; l < 4; l++)
        for (int i = 0; i < 3; i++)
            for (int p1 = 0, p2 = 1; p2 <= 3; p1++, p2++)
            {
                // 每行/列最多要遍历3次
                // 12：什么都不做继续遍历
                // 00：什么都不做继续遍历
                // 01：什么都不做继续遍历
                // 10：前数加到后位
                // 11：前数加到后位

                if ((board[l][p1] != 0 && board[l][p2] == 0) || (board[l][p1] == board[l][p2]))
                {
                    board[l][p2] += board[l][p1];
                    board[l][p1] = 0;
                }
            }
}

void Game2048::Spawn_new_tile()
{
    // 遍历一遍，看有几个空位
    // 0个空位宣告游戏结束
    // 生成一个0~空位数之间的随机数
    // 再遍历，到第n个空位，插入随机数
    int empty_count = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (board[i][j] == 0)
            {
                empty_count++;
            }
        }
    }
    if (empty_count == 0)
    {
        cout << "Game over!" << endl;
        game_over = true;
        return;
    }
    int rand_num = rand() % (empty_count + 1);
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (board[i][j] == 0)
            {
                if (count == rand_num)
                {
                    board[i][j] = (rand() % 2 == 0) ? 4 : 2;
                    return;
                }
                count++;
            }
        }
    }
}

Game2048::Game2048()
{
    game_over = false;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            board[i][j] = 0;
        }
    }
    Spawn_new_tile();
    Spawn_new_tile();
}

Game2048::~Game2048()
{
}


void Game2048::Move(Vector2 direction)
{
    // 将向量转化为角度，再判断方向
    float angle = atan2(direction.y, direction.x) * 180 / M_PI;
    char dir;
    if (-135 <= angle && angle < -45)
    {
        dir = 'u';
    }
    if (-45 <= angle && angle < 45)
    {
        dir = 'r';
    }
    if (45 <= angle && angle < 135)
    {
        dir = 'd';
    }
    if (135 <= angle || angle < -135)
    {
        dir = 'l';
    }

    // 合并相同数字
    switch (dir)
    {
    case 'u':
        Move_up();
        break;
    case 'd':
        Move_down();
        break;
    case 'l':
        Move_left();
        break;
    case 'r':
        Move_right();
        break;
    default:
        break;
    }

    // 生成新的数字
    Spawn_new_tile();
}

void Game2048::Move(char direction)
{
    // 合并相同数字
    switch (direction)
    {
    case 'u':
        Move_up();
        break;
    case 'd':
        Move_down();
        break;
    case 'l':
        Move_left();
        break;
    case 'r':
        Move_right();
        break;
    default:
        break;
    }

    // 生成新的数字
    Spawn_new_tile();
}

void Game2048::Update_Display()
{
    // 在终端显示
    cout << "---- ---- ---- ----\n";
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (board[i][j] == 0)
            {
                cout << setw(5) << left << '_';
            }
            else
            {
                cout << setw(5) << left << board[i][j];
            }
        }
        cout << endl
             << endl;
    }
    cout << "---- ---- ---- ----\n\n\n";

    // 在lcd显示

}
