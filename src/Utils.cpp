/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lrosa-do <lrosa-do@student.42lisboa>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/17 07:10:08 by lrosa-do          #+#    #+#             */
/*   Updated: 2023/04/23 09:30:03 by lrosa-do         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include <raylib.h>

void Log(int severity, const char *fmt, ...)
{
    /* Determine strings for the type and colour */
    const char *type;
    const char *color;
    switch (severity)
    {
    case LOG_INFO:
        type = "info";
        color = CONSOLE_COLOR_GREEN;
        break;
    case LOG_ERROR:
        type = "error";
        color = CONSOLE_COLOR_RED;
        break;
    case LOG_WARNING:
        type = "warning";
        color = CONSOLE_COLOR_PURPLE;
        break;
    default:
        break; /* Unreachable */
    }

    /* Obtain the current date and time */
    time_t rawTime;
    struct tm *timeInfo;
    char timeBuffer[80];

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S]", timeInfo);

    /* Format for printing to the console (with colours) */
    char consoleFormat[1024];
    snprintf(consoleFormat, 1024, "%s%s %s%s%s: %s\n", CONSOLE_COLOR_CYAN,
             timeBuffer, color, type, CONSOLE_COLOR_RESET, fmt);

    va_list argptr;

    /* Print to the console */
    va_start(argptr, fmt);
    vprintf(consoleFormat, argptr);
    va_end(argptr);
}

void RenderQuad(const rQuad *quad)
{

    rlCheckRenderBatchLimit(4); // Make sure there is enough free space on the batch buffer
    rlSetTexture(quad->tex.id);

    rlBegin(RL_QUADS);

    Color a = quad->v[1].col;
    Color b = quad->v[0].col;
    Color c = quad->v[3].col;
    Color d = quad->v[2].col;

    rlNormal3f(0.0f, 0.0f, 1.0f);

    rlColor4ub(a.r, a.g, a.b, a.a);
    rlTexCoord2f(quad->v[1].tx, quad->v[1].ty);
    rlVertex3f(quad->v[1].x, quad->v[1].y, quad->v[1].z);

    rlColor4ub(b.r, b.g, b.b, b.a);
    rlTexCoord2f(quad->v[0].tx, quad->v[0].ty);
    rlVertex3f(quad->v[0].x, quad->v[0].y, quad->v[0].z);

    rlColor4ub(c.r, c.g, c.b, c.a);
    rlTexCoord2f(quad->v[3].tx, quad->v[3].ty);
    rlVertex3f(quad->v[3].x, quad->v[3].y, quad->v[3].z);

    rlColor4ub(d.r, d.g, d.b, d.a);
    rlTexCoord2f(quad->v[2].tx, quad->v[2].ty);
    rlVertex3f(quad->v[2].x, quad->v[2].y, quad->v[2].z);

    rlEnd();
}

void RenderTransform(Texture2D texture, const Matrix2D *matrix, int blend)
{

    rQuad quad;
    quad.tex = texture;
    quad.blend = blend;

    float u = 0.0f;
    float v = 0.0f;
    float u2 = 1.0f;
    float v2 = 1.0f;

    float TempX1 = 0;
    float TempY1 = 0;
    float TempX2 = texture.width;
    float TempY2 = texture.height;

    quad.v[1].x = TempX1;
    quad.v[1].y = TempY1;
    quad.v[1].tx = u;
    quad.v[1].ty = v;

    quad.v[0].x = TempX1;
    quad.v[0].y = TempY2;
    quad.v[0].tx = u;
    quad.v[0].ty = v2;

    quad.v[3].x = TempX2;
    quad.v[3].y = TempY2;
    quad.v[3].tx = u2;
    quad.v[3].ty = v2;

    quad.v[2].x = TempX2;
    quad.v[2].y = TempY1;
    quad.v[2].tx = u2;
    quad.v[2].ty = v;

    for (int i = 0; i < 4; i++)
    {
        float x = quad.v[i].x;
        float y = quad.v[i].y;
        quad.v[i].x = matrix->a * x + matrix->c * y + matrix->tx;
        quad.v[i].y = matrix->d * y + matrix->b * x + matrix->ty;
    }

    quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
    quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = WHITE;

    RenderQuad(&quad);
}

void RenderTransformFlip(Texture2D texture, Rectangle clip, bool flipX, bool flipY, Color color, const Matrix2D *matrix, int blend)
{

    rQuad quad;
    quad.tex = texture;
    quad.blend = blend;

    int widthTex = texture.width;
    int heightTex = texture.height;

    float left;
    float right;
    float top;
    float bottom;

    if (FIX_ARTIFACTS_BY_STRECHING_TEXEL)
    {
        left = (2 * clip.x + 1) / (2 * widthTex);
        right = left + (clip.width * 2 - 2) / (2 * widthTex);
        top = (2 * clip.y + 1) / (2 * heightTex);
        bottom = top + (clip.height * 2 - 2) / (2 * heightTex);
    }
    else
    {
        left = clip.x / widthTex;
        right = (clip.x + clip.width) / widthTex;
        top = clip.y / heightTex;
        bottom = (clip.y + clip.height) / heightTex;
    }

    if (flipX)
    {
        float tmp = left;
        left = right;
        right = tmp;
    }

    if (flipY)
    {
        float tmp = top;
        top = bottom;
        bottom = tmp;
    }

    float TempX1 = 0;
    float TempY1 = 0;
    float TempX2 = texture.width;
    float TempY2 = texture.height;

    quad.v[1].x = TempX1;
    quad.v[1].y = TempY1;
    quad.v[1].tx = left;
    quad.v[1].ty = top;

    quad.v[0].x = TempX1;
    quad.v[0].y = TempY2;
    quad.v[0].tx = left;
    quad.v[0].ty = bottom;

    quad.v[3].x = TempX2;
    quad.v[3].y = TempY2;
    quad.v[3].tx = right;
    quad.v[3].ty = bottom;

    quad.v[2].x = TempX2;
    quad.v[2].y = TempY1;
    quad.v[2].tx = right;
    quad.v[2].ty = top;

    for (int i = 0; i < 4; i++)
    {
        float x = quad.v[i].x;
        float y = quad.v[i].y;
        quad.v[i].x = matrix->a * x + matrix->c * y + matrix->tx;
        quad.v[i].y = matrix->d * y + matrix->b * x + matrix->ty;
    }

    quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
    quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = color;

    RenderQuad(&quad);
}

void RenderTransformFlipClip(Texture2D texture, int width, int height, Rectangle clip, bool flipX, bool flipY, Color color, const Matrix2D *matrix, int blend)
{

    rQuad quad;
    quad.tex = texture;
    quad.blend = blend;

    int widthTex = texture.width;
    int heightTex = texture.height;

    float left;
    float right;
    float top;
    float bottom;

    if (FIX_ARTIFACTS_BY_STRECHING_TEXEL)
    {
        left = (2 * clip.x + 1) / (2 * widthTex);
        right = left + (clip.width * 2 - 2) / (2 * widthTex);
        top = (2 * clip.y + 1) / (2 * heightTex);
        bottom = top + (clip.height * 2 - 2) / (2 * heightTex);
    }
    else
    {
        left = clip.x / widthTex;
        right = (clip.x + clip.width) / widthTex;
        top = clip.y / heightTex;
        bottom = (clip.y + clip.height) / heightTex;
    }

    if (flipX)
    {
        float tmp = left;
        left = right;
        right = tmp;
    }

    if (flipY)
    {
        float tmp = top;
        top = bottom;
        bottom = tmp;
    }

    float TempX1 = 0;
    float TempY1 = 0;
    float TempX2 = width;
    float TempY2 = height;

    quad.v[1].x = TempX1;
    quad.v[1].y = TempY1;
    quad.v[1].tx = left;
    quad.v[1].ty = top;

    quad.v[0].x = TempX1;
    quad.v[0].y = TempY2;
    quad.v[0].tx = left;
    quad.v[0].ty = bottom;

    quad.v[3].x = TempX2;
    quad.v[3].y = TempY2;
    quad.v[3].tx = right;
    quad.v[3].ty = bottom;

    quad.v[2].x = TempX2;
    quad.v[2].y = TempY1;
    quad.v[2].tx = right;
    quad.v[2].ty = top;

    for (int i = 0; i < 4; i++)
    {
        float x = quad.v[i].x;
        float y = quad.v[i].y;
        quad.v[i].x = matrix->a * x + matrix->c * y + matrix->tx;
        quad.v[i].y = matrix->d * y + matrix->b * x + matrix->ty;
    }

    quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
    quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = color;

    RenderQuad(&quad);
}
void RenderNormal(Texture2D texture, float x, float y, int blend)
{

    rQuad quad;
    quad.tex = texture;
    quad.blend = blend;

    float u = 0.0f;
    float v = 0.0f;
    float u2 = 1.0f;
    float v2 = 1.0f;

    float fx2 = x + texture.width;
    float fy2 = y + texture.height;

    quad.v[1].x = x;
    quad.v[1].y = y;
    quad.v[1].tx = u;
    quad.v[1].ty = v;

    quad.v[0].x = x;
    quad.v[0].y = fy2;
    quad.v[0].tx = u;
    quad.v[0].ty = v2;

    quad.v[3].x = fx2;
    quad.v[3].y = fy2;
    quad.v[3].tx = u2;
    quad.v[3].ty = v2;

    quad.v[2].x = fx2;
    quad.v[2].y = y;
    quad.v[2].tx = u2;
    quad.v[2].ty = v;

    quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
    quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = WHITE;

    RenderQuad(&quad);
}
void RenderTile(Texture2D texture, float x, float y, float width, float height, Rectangle clip, bool flipx, bool flipy, int blend)
{

    float fx2 = x + width;
    float fy2 = y + height;
    rQuad quad;
    quad.tex = texture;
    quad.blend = blend;

    int widthTex = texture.width;
    int heightTex = texture.height;

    float left, right, top, bottom;

    if (FIX_ARTIFACTS_BY_STRECHING_TEXEL)
    {

        left = (2 * clip.x + 1) / (2 * widthTex);
        right = left + (clip.width * 2 - 2) / (2 * widthTex);
        top = (2 * clip.y + 1) / (2 * heightTex);
        bottom = top + (clip.height * 2 - 2) / (2 * heightTex);
    }
    else
    {
        left = clip.x / widthTex;
        right = (clip.x + clip.width) / widthTex;
        top = clip.y / heightTex;
        bottom = (clip.y + clip.height) / heightTex;
    }

    if (flipx)
    {
        float tmp = left;
        left = right;
        right = tmp;
    }

    if (flipy)
    {
        float tmp = top;
        top = bottom;
        bottom = tmp;
    }

    quad.v[1].tx = left;
    quad.v[1].ty = top;
    quad.v[1].x = x;
    quad.v[1].y = y;

    quad.v[0].x = x;
    quad.v[0].y = fy2;
    quad.v[0].tx = left;
    quad.v[0].ty = bottom;

    quad.v[3].x = fx2;
    quad.v[3].y = fy2;
    quad.v[3].tx = right;
    quad.v[3].ty = bottom;

    quad.v[2].x = fx2;
    quad.v[2].y = y;
    quad.v[2].tx = right;
    quad.v[2].ty = top;

    quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
    quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = WHITE;

    RenderQuad(&quad);
}

float Clamp(float value, float min, float max)
{
    float result = (value < min) ? min : value;

    if (result > max)
        result = max;

    return result;
}

// Calculate linear interpolation between two floats
float Lerp(float start, float end, float amount)
{
    float result = start + amount * (end - start);

    return result;
}

float normalizeAngle(float angle)
{
    // move into range [-180 deg, +180 deg]
    while (angle < -PI)
        angle += PI * 2.0f;
    while (angle > PI)
        angle -= PI * 2.0f;
    return angle;
}

int sign(float value)
{
    return value < 0 ? -1 : (value > 0 ? 1 : 0);
}

float distance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float distanceRects(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2)
{
    if (x1 < x2 + w2 && x2 < x1 + w1)
    {
        if (y1 < y2 + h2 && y2 < y1 + h1)
            return 0;
        if (y1 > y2)
            return y1 - (y2 + h2);
        return y2 - (y1 + h1);
    }
    if (y1 < y2 + h2 && y2 < y1 + h1)
    {
        if (x1 > x2)
            return x1 - (x2 + w2);
        return x2 - (x1 + w1);
    }
    if (x1 > x2)
    {
        if (y1 > y2)
            return distance(x1, y1, (x2 + w2), (y2 + h2));
        return distance(x1, y1 + h1, x2 + w2, y2);
    }
    if (y1 > y2)
        return distance(x1 + w1, y1, x2, y2 + h2);
    return distance(x1 + w1, y1 + h1, x2, y2);
}

float distanceRectPoint(float px, float py, float rx, float ry, float rw, float rh)
{
    if (px >= rx && px <= rx + rw)
    {
        if (py >= ry && py <= ry + rh)
            return 0;
        if (py > ry)
            return py - (ry + rh);
        return ry - py;
    }
    if (py >= ry && py <= ry + rh)
    {
        if (px > rx)
            return px - (rx + rw);
        return rx - px;
    }
    if (px > rx)
    {
        if (py > ry)
            return distance(px, py, rx + rw, ry + rh);
        return distance(px, py, rx + rw, ry);
    }
    if (py > ry)
        return distance(px, py, rx, ry + rh);
    return distance(px, py, rx, ry);
}

double fget_angle(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double angle;

    if (dx == 0)
        return dy > 0 ? 270 : 90;

    angle = (int)(atan(dy / dx) * 180.0 / M_PI);

    return dx > 0 ? -angle : -angle + 180;
}

float getAngle(float x1, float y1, float x2, float y2)
{
    float a = atan2(y2 - y1, x2 - x1) * DEG;
    return a < 0 ? a + 360 : a;
}

float floatLerp(float value1, float value2, float amount)
{
    return value1 + (value2 - value1) * amount;
}
float Hermite(float value1, float tangent1, float value2, float tangent2, float amount)
{
    float v1 = value1;
    float v2 = value2;
    float t1 = tangent1;
    float t2 = tangent2;
    float s = amount;
    float result;
    float sCubed = s * s * s;
    float sSquared = s * s;

    if (amount == 0)
        result = value1;
    else if (amount == 1)
        result = value2;
    else
        result = (2 * v1 - 2 * v2 + t2 + t1) * sCubed +
                 (3 * v2 - 3 * v1 - 2 * t1 - t2) * sSquared +
                 t1 * s +
                 v1;
    return result;
}
float clamp(float value, float min, float max)
{
    return value < min ? min : (value > max ? max : value);
}
float SmoothStep(float value1, float value2, float amount)
{

    float result = clamp(amount, 0, 1);
    result = Hermite(value1, 0, value2, 0, result);

    return result;
}

double get_distx(double angle, double speed)
{
    double a = angle * M_PI / 180.0;
    return (double)(speed * cos(a));
}

double get_disty(double angle, double speed)
{
    double a = angle * M_PI / 180.0;
    return (double)(speed * -sin(a));
}

float lerpAngleDegrees(float a, float b, float lerpFactor) // Lerps from angle a to b (both between 0.f and 360.f), taking the shortest path
{
    float result;
    float diff = b - a;
    if (diff < -180.f)
    {
        // lerp upwards past 360
        b += 360.f;
        result = Lerp(a, b, lerpFactor);
        if (result >= 360.f)
        {
            result -= 360.f;
        }
    }
    else if (diff > 180.f)
    {
        // lerp downwards past 0
        b -= 360.f;
        result = Lerp(a, b, lerpFactor);
        if (result < 0.f)
        {
            result += 360.f;
        }
    }
    else
    {
        // straight lerp
        result = Lerp(a, b, lerpFactor);
    }

    return result;
}

float lerpAngleRadians(float a, float b, float lerpFactor)
// Lerps from angle a to b (both between 0.f and PI_TIMES_TWO), taking the shortest path
{
    float result;
    float diff = b - a;
    if (diff < -PI)
    {
        // lerp upwards past PI_TIMES_TWO
        b += PI_TIMES_TWO;
        result = Lerp(a, b, lerpFactor);
        if (result >= PI_TIMES_TWO)
        {
            result -= PI_TIMES_TWO;
        }
    }
    else if (diff > PI)
    {
        // lerp downwards past 0
        b -= PI_TIMES_TWO;
        result = Lerp(a, b, lerpFactor);
        if (result < 0.f)
        {
            result += PI_TIMES_TWO;
        }
    }
    else
    {
        // straight lerp
        result = Lerp(a, b, lerpFactor);
    }

    return result;
}

//*********************************************************************************************************************
//**                         Vec2                                                                             **
//*********************************************************************************************************************

Vec2 &Vec2::add(const Vec2 &other)
{
    x += other.x;
    y += other.y;

    return *this;
}

Vec2 &Vec2::subtract(const Vec2 &other)
{
    x -= other.x;
    y -= other.y;

    return *this;
}

Vec2 &Vec2::multiply(const Vec2 &other)
{
    x *= other.x;
    y *= other.y;

    return *this;
}

Vec2 &Vec2::divide(const Vec2 &other)
{
    x /= other.x;
    y /= other.y;

    return *this;
}

Vec2 &Vec2::add(float value)
{
    x += value;
    y += value;

    return *this;
}

Vec2 &Vec2::subtract(float value)
{
    x -= value;
    y -= value;

    return *this;
}

Vec2 &Vec2::multiply(float value)
{
    x *= value;
    y *= value;

    return *this;
}

Vec2 &Vec2::divide(float value)
{
    x /= value;
    y /= value;

    return *this;
}

Vec2 operator+(Vec2 left, const Vec2 &right)
{
    return left.add(right);
}

Vec2 operator-(Vec2 left, const Vec2 &right)
{
    return left.subtract(right);
}

Vec2 operator*(Vec2 left, const Vec2 &right)
{
    return left.multiply(right);
}

Vec2 operator/(Vec2 left, const Vec2 &right)
{
    return left.divide(right);
}

Vec2 operator+(Vec2 left, float value)
{
    return Vec2(left.x + value, left.y + value);
}

Vec2 operator-(Vec2 left, float value)
{
    return Vec2(left.x - value, left.y - value);
}

Vec2 operator*(Vec2 left, float value)
{
    return Vec2(left.x * value, left.y * value);
}

Vec2 operator/(Vec2 left, float value)
{
    return Vec2(left.x / value, left.y / value);
}

Vec2 &Vec2::operator+=(const Vec2 &other)
{
    return add(other);
}

Vec2 &Vec2::operator-=(const Vec2 &other)
{
    return subtract(other);
}

Vec2 &Vec2::operator*=(const Vec2 &other)
{
    return multiply(other);
}

Vec2 &Vec2::operator/=(const Vec2 &other)
{
    return divide(other);
}

Vec2 &Vec2::operator+=(float value)
{
    return add(value);
}

Vec2 &Vec2::operator-=(float value)
{
    return subtract(value);
}

Vec2 &Vec2::operator*=(float value)
{
    return multiply(value);
}

Vec2 &Vec2::operator/=(float value)
{
    return divide(value);
}

bool Vec2::operator==(const Vec2 &other) const
{
    return x == other.x && y == other.y;
}

bool Vec2::operator!=(const Vec2 &other) const
{
    return !(*this == other);
}

bool Vec2::operator<(const Vec2 &other) const
{
    return x < other.x && y < other.y;
}

bool Vec2::operator<=(const Vec2 &other) const
{
    return x <= other.x && y <= other.y;
}

bool Vec2::operator>(const Vec2 &other) const
{
    return x > other.x && y > other.y;
}

bool Vec2::operator>=(const Vec2 &other) const
{
    return x >= other.x && y >= other.y;
}

float Vec2::distance(const Vec2 &other) const
{
    float a = x - other.x;
    float b = y - other.y;
    return sqrt(a * a + b * b);
}

float Vec2::dot(const Vec2 &other) const
{
    return x * other.x + y * other.y;
}

float Vec2::magnitude() const
{
    return sqrt(x * x + y * y);
}

Vec2 Vec2::normalised() const
{
    float length = magnitude();
    return Vec2(x / length, y / length);
}

class rRect
{
public:
    float x1, y1, x2, y2;

    rRect(const float _x1, const float _y1, const float _x2, const float _y2)
    {
        x1 = _x1;
        y1 = _y1;
        x2 = _x2;
        y2 = _y2;
        is_clean_ = false;
    }

    rRect()
    {
        is_clean_ = true;
    }

    Rectangle getRactangle()
    {

        float w = x2 - x1;
        float h = x2 - x1;

        return {x1, y1, w, h};
    }
    void Clear()
    {
        is_clean_ = true;
    }

    bool IsClean() const
    {
        return is_clean_;
    }

    void Set(const float _x1, const float _y1, const float _x2, const float _y2)
    {
        x1 = _x1;
        x2 = _x2;
        y1 = _y1;
        y2 = _y2;
        is_clean_ = false;
    }

    void SetRadius(const float x, const float y, const float r)
    {
        x1 = x - r;
        x2 = x + r;
        y1 = y - r;
        y2 = y + r;
        is_clean_ = false;
    }

    void Encapsulate(const float x, const float y)
    {
        if (is_clean_)
        {
            x1 = x2 = x;
            y1 = y2 = y;
            is_clean_ = false;
        }
        else
        {
            if (x < x1)
            {
                x1 = x;
            }
            if (x > x2)
            {
                x2 = x;
            }
            if (y < y1)
            {
                y1 = y;
            }
            if (y > y2)
            {
                y2 = y;
            }
        }
    }

    bool TestPoint(const float x, const float y) const
    {
        if (x >= x1 && x < x2 && y >= y1 && y < y2)
        {
            return true;
        }

        return false;
    }

    bool Intersect(const rRect *rect) const
    {
        if (fabs(x1 + x2 - rect->x1 - rect->x2) < (x2 - x1 + rect->x2 - rect->x1))
            if (fabs(y1 + y2 - rect->y1 - rect->y2) < (y2 - y1 + rect->y2 - rect->y1))
            {
                return true;
            }

        return false;
    }

private:
    bool is_clean_;
};
//*********************************************************************************************************************
//**                         Matrix2D                                                                              **
//*********************************************************************************************************************

Matrix2D::Matrix2D()
{
    a = 1;
    b = 0;
    c = 0;
    d = 1;
    tx = 0;
    ty = 0;
}

Matrix2D::~Matrix2D()
{
}

void Matrix2D::Identity()
{
    a = 1;
    b = 0;
    c = 0;
    d = 1;
    tx = 0;
    ty = 0;
}

void Matrix2D::Set(float a, float b, float c, float d, float tx, float ty)
{

    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
    this->tx = tx;
    this->ty = ty;
}

void Matrix2D::Concat(const Matrix2D &m)
{
    float a1 = this->a * m.a + this->b * m.c;
    this->b = this->a * m.b + this->b * m.d;
    this->a = a1;

    float c1 = this->c * m.a + this->d * m.c;
    this->d = this->c * m.b + this->d * m.d;

    this->c = c1;

    float tx1 = this->tx * m.a + this->ty * m.c + m.tx;
    this->ty = this->tx * m.b + this->ty * m.d + m.ty;
    this->tx = tx1;
}

Vec2 Matrix2D::TransformCoords(Vec2 point)
{

    Vec2 v;

    v.x = this->a * point.x + this->c * point.y + this->tx;
    v.y = this->d * point.y + this->b * point.x + this->ty;

    return v;
}
Vec2 Matrix2D::TransformCoords(float x, float y)
{
    Vec2 v;

    v.x = this->a * x + this->c * y + this->tx;
    v.y = this->d * y + this->b * x + this->ty;

    return v;
}

Vec2 Matrix2D::TransformCoords()
{

    Vec2 v;

    v.x = this->a * 0 + this->c * 0 + this->tx;
    v.y = this->d * 0 + this->b * 0 + this->ty;

    return v;
}
Matrix2D Matrix2D::Mult(const Matrix2D &m)
{
    Matrix2D result;

    result.a = this->a * m.a + this->b * m.c;
    result.b = this->a * m.b + this->b * m.d;
    result.c = this->c * m.a + this->d * m.c;
    result.d = this->c * m.b + this->d * m.d;

    result.tx = this->tx * m.a + this->ty * m.c + this->tx;
    result.ty = this->tx * m.b + this->ty * m.d + this->ty;

    return result;
}

void Matrix2D::Rotate(float angle)
{
    float acos = cos(angle);
    float asin = sin(angle);

    float a1 = this->a * acos - this->b * asin;
    this->b = this->a * asin + this->b * acos;
    this->a = a1;

    float c1 = this->c * acos - this->d * asin;
    this->d = this->c * asin + this->d * acos;
    this->c = c1;

    float tx1 = this->tx * acos - this->ty * asin;
    this->ty = this->tx * asin + this->ty * acos;
    this->tx = tx1;
}

void Matrix2D::Scale(float x, float y)
{
    this->a *= x;
    this->b *= y;

    this->c *= x;
    this->d *= y;

    this->tx *= x;
    this->ty *= y;
}

void Matrix2D::Translate(float x, float y)
{
    this->tx += x;
    this->ty += y;
}

void Matrix2D::Skew(float skewX, float skewY)
{
    float sinX = sin(skewX);
    float cosX = cos(skewX);
    float sinY = sin(skewY);
    float cosY = cos(skewY);

    Set(
        this->a * cosY - this->b * sinX,
        this->a * sinY + this->b * cosX,
        this->c * cosY - this->d * sinX,
        this->c * sinY + this->d * cosX,
        this->tx * cosY - this->ty * sinX,
        this->tx * sinY + this->ty * cosX);
}

float toRadians(float degrees)
{
    const float pi = 3.14159265f;
    return degrees * (pi / 180.0f);
}

// Função para rotacionar um ponto (x, y) em torno da origem (0, 0) com um ângulo 'rotation' em graus
Vector2 rotatePointAroundOrigin(float x, float y, float rotation)
{
    float theta = toRadians(rotation);

    float x_rotacionado = std::cos(theta) * x - std::sin(theta) * y;
    float y_rotacionado = std::sin(theta) * x + std::cos(theta) * y;

    return Vector2{x_rotacionado, y_rotacionado};
}


#define COLLIDE_MAX(a,b)	((a > b) ? a : b)
#define COLLIDE_MIN(a,b)	((a < b) ? a : b)


bool getTransparentPixel(Color *pixels,int width,int height, int x , int y)
{
    if (x < 0  ) return false;
    if (y < 0 ) return false;
    if (x > width ) return false;
    if (y > height ) return false;


    Color r = pixels[y*width + x];

    return r.a!=0;

}

int CollidePixel(Color* as , Image a, int ax , int ay ,    Color* bs ,Image b, int bx , int by, int skip)
{
	/*a - bottom right co-ordinates in world space*/
	int ax1 = ax + a.width - 1;
	int ay1 = ay + a.height - 1;

	/*b - bottom right co-ordinates in world space*/
	int bx1 = bx + b.width - 1;
	int by1 = by + b.height- 1;





	/*check if bounding boxes intersect*/
	if((bx1 < ax) || (ax1 < bx))
		return 0;
	if((by1 < ay) || (ay1 < by))
		return 0;


	int xstart = COLLIDE_MAX(ax,bx);
	int xend = COLLIDE_MIN(ax1,bx1);

	int ystart = COLLIDE_MAX(ay,by);
	int yend = COLLIDE_MIN(ay1,by1);





	for(int y = ystart ; y <= yend ; y += skip)
	{
		for(int x = xstart ; x <= xend ; x += skip)
		{
			bool ca = getTransparentPixel(as, a.width,a.height,x-ax , y-ay);
			bool cb = getTransparentPixel(bs, b.width,b.height,x-bx , y-by);
			if (ca && cb) return 1;
		}
	}
	return 0;
}


float memoryInMB(size_t bytes)
{
    return static_cast<float>(bytes) / (1024.0f * 1024.0f);
}
float memoryInKB(size_t bytes)
{
    return static_cast<float>(bytes) / 1024.0f;
}

float memoryIn(size_t bytes)
{

    if (bytes >= 1.0e6)
    {
        return memoryInMB(bytes);
    }
    else if (bytes >= 1.0e3)
    {
        return memoryInKB(bytes);
    }
    else
    {
        return static_cast<float>(bytes);
    }
}

std::string base64_decode(const std::string &base64_string) 
{
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int i = 0;
    int j = 0;
    int in_len = base64_string.size();
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (base64_string[i] != '=') && (isalnum(base64_string[i]) || (base64_string[i] == '+') || (base64_string[i] == '/'))) {
        char_array_4[j++] = base64_chars.find(base64_string[i++]);
        if (j == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; j < 3; j++) {
                ret += char_array_3[j];
            }
            j = 0;
        }
    }

    if (j) {
        for (int i = j; i < 4; i++) {
            char_array_4[i] = 0;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int i = 0; i < j - 1; i++) {
            ret += char_array_3[i];
        }
    }

    return ret;
}

std::string GetPath(const std::string &path)
{
    if (FileExists(TextFormat("assets/%s", path.c_str())))
    {
        return TextFormat("assets/%s", path.c_str());
    }else if (FileExists(TextFormat("../assets/%s", path.c_str())))
    {
        return TextFormat("../assets/%s", path.c_str());
    } else if (FileExists(TextFormat("assets/images/%s", path.c_str())))
    {
        return TextFormat("assets/images/%s", path.c_str());
    } else if (FileExists(TextFormat("../assets/images/%s", path.c_str())))
    {
        return TextFormat("../assets/images/%s", path.c_str());
    } else if (FileExists(TextFormat("assets/textures/%s", path.c_str())))
    {
        return TextFormat("assets/textures/%s", path.c_str());
    }
    else if (FileExists(TextFormat("../assets/textures/%s", path.c_str())))
    {
        return TextFormat("../assets/textures/%s", path.c_str());
    }
    else if (FileExists(TextFormat("assets/sounds/%s", path.c_str())))
    {
        return TextFormat("assets/sounds/%s", path.c_str());
    }else if (FileExists(TextFormat("../assets/sounds/%s", path.c_str())))
    {
        return TextFormat("../assets/sounds/%s", path.c_str());
    }
    return path;
}
bool FileInPath(const std::string &path)
{
    
    if (FileExists(path.c_str()))
    {
        return true;
    }
    else if (FileExists(TextFormat("assets/%s", path.c_str())))
    {
        return true;
    }else if (FileExists(TextFormat("../assets/%s", path.c_str())))
    {
        return true;
    } else if (FileExists(TextFormat("assets/images/%s", path.c_str())))
    {
        return true;
    } else if (FileExists(TextFormat("../assets/images/%s", path.c_str())))
    {
        return true;
    } else if (FileExists(TextFormat("assets/textures/%s", path.c_str())))
    {
        return true;
    }
    else if (FileExists(TextFormat("../assets/textures/%s", path.c_str())))
    {
        return true;
    }
    else if (FileExists(TextFormat("assets/sounds/%s", path.c_str())))
    {
        return true;
    }else if (FileExists(TextFormat("../assets/sounds/%s", path.c_str())))
    {
        return true;
    }
    
    
    return false;
}