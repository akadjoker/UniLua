/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrosa-do <lrosa-do@student.42lisboa>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 07:10:15 by lrosa-do          #+#    #+#             */
/*   Updated: 2023/04/23 13:17:56 by lrosa-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <raylib.h>
#include <rlgl.h>
#include <math.h>
#include <string>
#include <random>


#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_GREEN "\033[1;32m"
#define CONSOLE_COLOR_RED "\033[1;31m"
#define CONSOLE_COLOR_PURPLE "\033[1;35m"
#define CONSOLE_COLOR_CYAN "\033[0;36m"

const float RECIPROCAL_PI = 1.0f / PI;
const float HALF_PI = PI / 2.0f;
const float DEGTORAD = PI / 180.0f;
const float RADTODEG = 180.0f / PI;

#define PI_TIMES_TWO 6.28318530718f
#define PI2 PI * 2
#define DEG -180 / PI
#define RAD PI / -180
#define COLLIDE_MAX(a, b) ((a > b) ? a : b)
#define COLLIDE_MIN(a, b) ((a < b) ? a : b)

#define BLEND_ALPHAMULT 6
#define BLEND_ALPHABLEND 2
#define BLEND_ALPHAADD 0

#define BLEND_ZWRITE 4
#define BLEND_NOZWRITE 0

#define BLEND_DEFAULT (BLEND_ALPHABLEND | BLEND_NOZWRITE)
#define BLEND_DEFAULT_Z (BLEND_ALPHABLEND | BLEND_ZWRITE)

const bool FIX_ARTIFACTS_BY_STRECHING_TEXEL = true;

typedef struct rVertex
{
    float x, y, z;
    Color col;
    float tx, ty;

} rVertex;

typedef struct rQuad
{
    rVertex v[4];
    Texture2D tex;
    int blend;
} rQuad;

void Log(int severity, const char *fmt, ...);

//*********************************************************************************************************************
//**                         Vec2                                                                              **
//*********************************************************************************************************************

struct Vec2
{
    float x;
    float y;

    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float xy) : x(xy), y(xy) {}
    Vec2(float x, float y) : x(x), y(y) {}

    Vec2 &add(const Vec2 &other);
    Vec2 &subtract(const Vec2 &other);
    Vec2 &multiply(const Vec2 &other);
    Vec2 &divide(const Vec2 &other);

    Vec2 &add(float value);
    Vec2 &subtract(float value);
    Vec2 &multiply(float value);
    Vec2 &divide(float value);

    friend Vec2 operator+(Vec2 left, const Vec2 &right);
    friend Vec2 operator-(Vec2 left, const Vec2 &right);
    friend Vec2 operator*(Vec2 left, const Vec2 &right);
    friend Vec2 operator/(Vec2 left, const Vec2 &right);

    friend Vec2 operator+(Vec2 left, float value);
    friend Vec2 operator-(Vec2 left, float value);
    friend Vec2 operator*(Vec2 left, float value);
    friend Vec2 operator/(Vec2 left, float value);

    bool operator==(const Vec2 &other) const;
    bool operator!=(const Vec2 &other) const;

    Vec2 &operator+=(const Vec2 &other);
    Vec2 &operator-=(const Vec2 &other);
    Vec2 &operator*=(const Vec2 &other);
    Vec2 &operator/=(const Vec2 &other);

    Vec2 &operator+=(float value);
    Vec2 &operator-=(float value);
    Vec2 &operator*=(float value);
    Vec2 &operator/=(float value);

    bool operator<(const Vec2 &other) const;
    bool operator<=(const Vec2 &other) const;
    bool operator>(const Vec2 &other) const;
    bool operator>=(const Vec2 &other) const;

    float magnitude() const;
    Vec2 normalised() const;
    float distance(const Vec2 &other) const;
    float dot(const Vec2 &other) const;
};

class Matrix2D
{
public:
    Matrix2D();
    virtual ~Matrix2D();
    void Identity();
    void Set(float a, float b, float c, float d, float tx, float ty);
    void Concat(const Matrix2D &m);
    Vec2 TransformCoords(Vec2 point);
    Vec2 TransformCoords(float x, float y);
    Vec2 TransformCoords();
    Matrix2D Mult(const Matrix2D &m);
    void Rotate(float angle);
    void Scale(float x, float y);
    void Translate(float x, float y);
    void Skew(float skewX, float skewY);

public:
    float a;
    float b;
    float c;
    float d;
    float tx;
    float ty;
};
class AABB
{
public:
    AABB(float x, float y, float w, float h)
        : m_x(x), m_y(y), m_w(w), m_h(h)
    {
    }

    float GetX() const { return m_x; }
    float GetY() const { return m_y; }
    float GetWidth() const { return m_w; }
    float GetHeight() const { return m_h; }

    bool contains(const AABB &other) const
    {
        float r1 = m_x + m_w;
        float r2 = other.m_x + other.m_w;
        float b1 = m_y + m_h;
        float b2 = other.m_y + other.m_h;

        if (m_x < r2 && r1 > other.m_x && m_y < b2 && b1 > other.m_y)
        {
            return true;
        }
        return false;
    }

    bool contains(const Vec2 &point) const
    {
        if (point.x >= m_x && point.x <= m_x + m_w && point.y >= m_y && point.y <= m_y + m_h)
        {
            return true;
        }
        return false;
    }

    bool intersects(const AABB &other) const
    {
        float r1 = m_x + m_w;
        float r2 = other.m_x + other.m_w;
        float b1 = m_y + m_h;
        float b2 = other.m_y + other.m_h;

        if (m_x < r2 && r1 > other.m_x && m_y < b2 && b1 > other.m_y)
        {
            return true;
        }
        return false;
    }

    static bool IntersectsCircle(const AABB &aabb, const Vec2 &circleCenter, float radius)
    {
        float deltaX = circleCenter.x - std::max(aabb.m_x, std::min(circleCenter.x, aabb.m_x + aabb.m_w));
        float deltaY = circleCenter.y - std::max(aabb.m_y, std::min(circleCenter.y, aabb.m_y + aabb.m_h));

        return (deltaX * deltaX + deltaY * deltaY) <= (radius * radius);
    }

    float m_x, m_y;
    float m_w, m_h;
};



void RenderTransform(Texture2D texture, const Matrix2D *matrix, int blend);
void RenderTransformFlip(Texture2D texture, Rectangle clip, bool flipX, bool flipY, Color color, const Matrix2D *matrix, int blend);
void RenderTransformFlipClip(Texture2D texture, int width, int height, Rectangle clip, bool flipX, bool flipY, Color color, const Matrix2D *matrix, int blend);

void RenderQuad(const rQuad *quad);
void RenderNormal(Texture2D texture, float x, float y, int blend);
void RenderTile(Texture2D texture, float x, float y, float width, float height, Rectangle clip, bool flipx, bool flipy, int blend);

void Random_Seed(const int seed);
int Random_Int(const int min, const int max);
float Random_Float(const float min, const float max);

int sign(float value);
float distance(float x1, float y1, float x2, float y2);
float distanceRects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
Rectangle getMatrixBounds(Matrix2D matrix, Rectangle rectangle);
float lerpAngleRadians(float a, float b, float lerpFactor);
float lerpAngleDegrees(float a, float b, float lerpFactor);
float SmoothStep(float value1, float value2, float amount);
Vector2 normalize(Vector2 v, float thickness);
float getAngle(float x1, float y1, float x2, float y2);
float floatLerp(float value1, float value2, float amount);
float Hermite(float value1, float tangent1, float value2, float tangent2, float amount);

double fget_angle(double x1, double y1, double x2, double y2);
double get_distx(double angle, double speed);
double get_disty(double angle, double speed);

float Clamp(float value, float min, float max);
float Lerp(float start, float end, float amount);

Vector2 rotatePointAroundOrigin(float x, float y, float rotation);
float memoryInMB(size_t bytes);
float memoryInKB(size_t bytes);
float memoryIn(size_t bytes);

std::string base64_decode(const std::string &base64_string) ;

std::string GetPath(const std::string &path);
bool FileInPath(const std::string &path);