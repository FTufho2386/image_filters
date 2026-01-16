#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>
#include <stdbool.h>

// Структура для представления цвета пикселя
typedef struct {
    float r;  // Красный компонент (0.0 - 1.0)
    float g;  // Зеленый компонент (0.0 - 1.0)
    float b;  // Синий компонент (0.0 - 1.0)
} Pixel;

// Функции для работы с цветом
Pixel pixel_create(float r, float g, float b);
Pixel pixel_from_bytes(uint8_t r, uint8_t g, uint8_t b);
void pixel_to_bytes(Pixel p, uint8_t* r, uint8_t* g, uint8_t* b);
Pixel pixel_add(Pixel a, Pixel b);
Pixel pixel_multiply(Pixel p, float scalar);
Pixel pixel_multiply_pixel(Pixel a, Pixel b);
float pixel_luminance(Pixel p);

#endif // COLOR_H