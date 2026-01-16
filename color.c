#include "color.h"
#include <math.h>

Pixel pixel_create(float r, float g, float b) {
    Pixel p = { r, g, b };
    return p;
}

Pixel pixel_from_bytes(uint8_t r, uint8_t g, uint8_t b) {
    Pixel p = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f
    };
    return p;
}

void pixel_to_bytes(Pixel p, uint8_t* r, uint8_t* g, uint8_t* b) {
    // Ограничиваем значения в диапазоне [0, 1]
    float red = p.r < 0.0f ? 0.0f : (p.r > 1.0f ? 1.0f : p.r);
    float green = p.g < 0.0f ? 0.0f : (p.g > 1.0f ? 1.0f : p.g);
    float blue = p.b < 0.0f ? 0.0f : (p.b > 1.0f ? 1.0f : p.b);

    *r = (uint8_t)(red * 255.0f + 0.5f);
    *g = (uint8_t)(green * 255.0f + 0.5f);
    *b = (uint8_t)(blue * 255.0f + 0.5f);
}

Pixel pixel_add(Pixel a, Pixel b) {
    Pixel result = {
        a.r + b.r,
        a.g + b.g,
        a.b + b.b
    };
    return result;
}

Pixel pixel_multiply(Pixel p, float scalar) {
    Pixel result = {
        p.r * scalar,
        p.g * scalar,
        p.b * scalar
    };
    return result;
}

Pixel pixel_multiply_pixel(Pixel a, Pixel b) {
    Pixel result = {
        a.r * b.r,
        a.g * b.g,
        a.b * b.b
    };
    return result;
}

float pixel_luminance(Pixel p) {
    // Формула для преобразования в оттенки серого
    return 0.299f * p.r + 0.587f * p.g + 0.114f * p.b;
}