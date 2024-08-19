#pragma once

#include "vector2.h"

class Game2048
{
private:
    int score;
    int target;
    bool game_over;

    void Move_up();
    void Move_down();
    void Move_left();
    void Move_right();
    void Spawn_new_tile();
public:
    int board[4][4];
    Game2048();
    ~Game2048();

    void Move(Vector2 direction);
    void Move(char direction);

    void Update_Display();

};