#ifndef IMAGE_H
#define IMAGE_H

#include "color.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;              // Сигнатура "BM"
    uint32_t size;              // Размер файла в байтах
    uint16_t reserved1;         // Зарезервировано
    uint16_t reserved2;         // Зарезервировано
    uint32_t offset;            // Смещение до данных пикселей
} BMPFileHeader;

typedef struct {
    uint32_t size;              // Размер структуры (40 байт)
    int32_t width;              // Ширина изображения в пикселях
    int32_t height;             // Высота изображения в пикселях
    uint16_t planes;            // Число плоскостей (должно быть 1)
    uint16_t bits_per_pixel;    // Бит на пиксель (должно быть 24)
    uint32_t compression;       // Тип сжатия (0 - без сжатия)
    uint32_t image_size;        // Размер данных изображения
    int32_t x_pixels_per_meter; // Горизонтальное разрешение
    int32_t y_pixels_per_meter; // Вертикальное разрешение
    uint32_t colors_used;       // Число используемых цветов
    uint32_t colors_important;  // Число важных цветов
} BMPInfoHeader;
#pragma pack(pop)

typedef struct {
    int width;
    int height;
    Pixel** data;  // Двумерный массив пикселей [height][width]
} Image;

// Функции для работы с изображением
Image* image_create(int width, int height);
void image_destroy(Image* img);
Pixel* image_get_pixel(Image* img, int x, int y);
void image_set_pixel(Image* img, int x, int y, Pixel pixel);

// Функции для работы с BMP
Image* bmp_load(const char* filename, char** error);
bool bmp_save(const char* filename, Image* img, char** error);
void print_bmp_info(BMPFileHeader* file_header, BMPInfoHeader* info_header);

#endif // IMAGE_H