#pragma once
#include <iostream>

using namespace std;

class Vector2
{
public:
    int x, y;

    Vector2(int x = 0, int y = 0);

    Vector2 operator+(const Vector2& other) const;

    Vector2 operator-(const Vector2& other) const;

    Vector2 operator*(int scalar) const;

    Vector2 operator/(int scalar) const;

    bool operator==(const Vector2& other) const;

    bool operator!=(const Vector2& other) const;

    friend ostream& operator<<(ostream& os, const Vector2& v);

    int length() const;

    Vector2 normalized() const;
};
