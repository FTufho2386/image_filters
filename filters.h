#ifndef FILTERS_H
#define FILTERS_H

#include "image.h"

// Тип функции фильтра
typedef bool (*FilterFunction)(Image* img, int argc, char** argv, char** error);

// Структура для описания фильтра
typedef struct {
    const char* name;
    FilterFunction function;
    int min_args;  // Минимальное количество аргументов
    int max_args;  // Максимальное количество аргументов (-1 = без ограничений)
} Filter;

// Базовые фильтры
bool filter_crop(Image* img, int argc, char** argv, char** error);
bool filter_grayscale(Image* img, int argc, char** argv, char** error);
bool filter_negative(Image* img, int argc, char** argv, char** error);
bool filter_sharpening(Image* img, int argc, char** argv, char** error);
bool filter_edge_detection(Image* img, int argc, char** argv, char** error);
bool filter_median(Image* img, int argc, char** argv, char** error);
bool filter_gaussian_blur(Image* img, int argc, char** argv, char** error);

// Дополнительные фильтры
bool filter_crystallize(Image* img, int argc, char** argv, char** error);
bool filter_glass_distortion(Image* img, int argc, char** argv, char** error);
bool filter_sepia(Image* img, int argc, char** argv, char** error);
bool filter_vignette(Image* img, int argc, char** argv, char** error);

// Вспомогательные функции
bool apply_matrix_filter(Image* img, float kernel[3][3], char** error);
Pixel get_pixel_with_padding(Image* img, int x, int y);

// Таблица доступных фильтров
extern Filter available_filters[];
extern int filter_count;

// Объявление функции для использования в других файлах
Pixel get_pixel_with_padding(Image* img, int x, int y);

#endif // FILTERS_H