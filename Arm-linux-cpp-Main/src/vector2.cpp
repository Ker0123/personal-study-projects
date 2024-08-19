#include "vector2.h"
#include <cmath>

Vector2::Vector2(int _x, int _y)
{
    x = _x;
    y = _y;
}

Vector2 Vector2::operator+(const Vector2 &other) const
{
    return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::operator-(const Vector2 &other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::operator*(int scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::operator/(int scalar) const
{
    return Vector2(x / scalar, y / scalar);
}

bool Vector2::operator==(const Vector2 &other) const
{
    if (x == other.x && y == other.y)
    {
        return true;
    }
    return false;
}

bool Vector2::operator!=(const Vector2 &other) const
{
    if (x != other.x || y != other.y)
    {
        return true;
    }
    return false;
}



int Vector2::length() const
{
    return (int)sqrt(x * x + y * y);
}

Vector2 Vector2::normalized() const
{
    int len = length();
    if (len == 0)
    {
        return Vector2(0, 0);
    }
    return Vector2(x / len, y / len);
}

ostream &operator<<(ostream &os, const Vector2 &v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

