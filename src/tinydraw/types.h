#pragma once

typedef struct int2
{
    int x;
    int y;
} int2;

typedef struct float2
{
    float x;
    float y;
} float2;

typedef struct float3
{
    float x;
    float y;
    float z;
} float3;

typedef struct matrix4x4
{
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} matrix4x4;

typedef struct Color
{
    float r, g, b, a;
} Color;

typedef struct Vertex
{
    float x, y, z;
    float u, v;
    float r, g, b, a;
} Vertex;
