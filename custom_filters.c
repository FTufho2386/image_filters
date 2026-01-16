#ifndef CUSTOM_FILTERS_C
#define CUSTOM_FILTERS_C

#include "custom_filters.h"
#include "color.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>  // Добавляем для FLT_MAX

// Объявляем функцию, которая определена в filters.c
Pixel get_pixel_with_padding(Image* img, int x, int y);

// Фильтр "Кристаллизация" - разбивает изображение на ячейки Вороного
bool filter_crystallize(Image* img, int argc, char** argv, char** error) {
    (void)argc;
    (void)argv;

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Количество ячеек (центров)
    int num_cells = 50;

    // Генерируем случайные центры
    srand((unsigned int)time(NULL));
    int* centers_x = (int*)malloc(num_cells * sizeof(int));
    int* centers_y = (int*)malloc(num_cells * sizeof(int));
    Pixel* center_colors = (Pixel*)malloc(num_cells * sizeof(Pixel));

    if (!centers_x || !centers_y || !center_colors) {
        free(centers_x);
        free(centers_y);
        free(center_colors);
        if (error) *error = "Memory allocation failed";
        return false;
    }

    // Инициализируем центры случайными позициями и цветами
    for (int i = 0; i < num_cells; i++) {
        centers_x[i] = rand() % img->width;
        centers_y[i] = rand() % img->height;

        // Получаем пиксель и копируем его значение
        Pixel* pixel_ptr = image_get_pixel(img, centers_x[i], centers_y[i]);
        if (pixel_ptr) {
            center_colors[i] = *pixel_ptr;  // Копируем значение пикселя
        }
        else {
            center_colors[i] = pixel_create(0, 0, 0);  // Черный цвет по умолчанию
        }
    }

    // Создаем временное изображение для результата
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        free(centers_x);
        free(centers_y);
        free(center_colors);
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Для каждого пикселя находим ближайший центр и используем его цвет
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int closest_center = 0;
            float min_distance = FLT_MAX;

            // Ищем ближайший центр
            for (int i = 0; i < num_cells; i++) {
                int dx = x - centers_x[i];
                int dy = y - centers_y[i];
                float distance = sqrtf((float)(dx * dx + dy * dy));

                if (distance < min_distance) {
                    min_distance = distance;
                    closest_center = i;
                }
            }

            // Используем цвет ближайшего центра
            image_set_pixel(temp, x, y, center_colors[closest_center]);
        }
    }

    // Копируем результат обратно
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel* src_pixel = image_get_pixel(temp, x, y);
            if (src_pixel) {
                image_set_pixel(img, x, y, *src_pixel);
            }
        }
    }

    // Освобождаем память
    free(centers_x);
    free(centers_y);
    free(center_colors);
    image_destroy(temp);

    return true;
}

// Фильтр "Стеклянная дисторсия" - искажение как через стекло
bool filter_glass_distortion(Image* img, int argc, char** argv, char** error) {
    (void)argc;
    (void)argv;

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Параметры эффекта
    float scale = 0.05f;  // Масштаб искажения
    int distortion = 10;   // Сила искажения

    srand((unsigned int)time(NULL));

    // Создаем временное изображение для результата
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Применяем эффект стеклянной дисторсии
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            // Создаем случайное смещение
            float offset_x = sinf((float)x * scale) * (float)distortion;
            float offset_y = cosf((float)y * scale) * (float)distortion;

            // Добавляем немного случайности
            offset_x += (float)(rand() % 5 - 2);
            offset_y += (float)(rand() % 5 - 2);

            // Вычисляем новые координаты
            int new_x = x + (int)offset_x;
            int new_y = y + (int)offset_y;

            // Ограничиваем координаты
            if (new_x < 0) new_x = 0;
            if (new_x >= img->width) new_x = img->width - 1;
            if (new_y < 0) new_y = 0;
            if (new_y >= img->height) new_y = img->height - 1;

            // Берем пиксель из смещенной позиции
            Pixel pixel = get_pixel_with_padding(img, new_x, new_y);
            image_set_pixel(temp, x, y, pixel);
        }
    }

    // Копируем результат обратно
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel* src_pixel = image_get_pixel(temp, x, y);
            if (src_pixel) {
                image_set_pixel(img, x, y, *src_pixel);
            }
        }
    }

    image_destroy(temp);
    return true;
}

// Фильтр "Сепия" - коричневый оттенок как на старых фотографиях
bool filter_sepia(Image* img, int argc, char** argv, char** error) {
    (void)argc;
    (void)argv;

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel* pixel = image_get_pixel(img, x, y);
            if (!pixel) continue;

            // Формула для сепии
            float r = pixel->r;
            float g = pixel->g;
            float b = pixel->b;

            float new_r = r * 0.393f + g * 0.769f + b * 0.189f;
            float new_g = r * 0.349f + g * 0.686f + b * 0.168f;
            float new_b = r * 0.272f + g * 0.534f + b * 0.131f;

            // Ограничиваем значения
            new_r = new_r > 1.0f ? 1.0f : new_r;
            new_g = new_g > 1.0f ? 1.0f : new_g;
            new_b = new_b > 1.0f ? 1.0f : new_b;

            pixel->r = new_r;
            pixel->g = new_g;
            pixel->b = new_b;
        }
    }

    return true;
}

// Фильтр "Виньетка" - затемнение краев изображения
bool filter_vignette(Image* img, int argc, char** argv, char** error) {
    (void)argc;
    (void)argv;

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Центр виньетки
    float center_x = (float)img->width / 2.0f;
    float center_y = (float)img->height / 2.0f;

    // Максимальное расстояние от центра до угла
    float max_distance = sqrtf(center_x * center_x + center_y * center_y);

    // Сила виньетки
    float strength = 0.7f;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel* pixel = image_get_pixel(img, x, y);
            if (!pixel) continue;

            // Вычисляем расстояние от центра
            float dx = (float)x - center_x;
            float dy = (float)y - center_y;
            float distance = sqrtf(dx * dx + dy * dy);

            // Вычисляем коэффициент затемнения (1.0 в центре, меньше на краях)
            float factor = 1.0f - (distance / max_distance) * strength;
            if (factor < 0.3f) factor = 0.3f; // Минимальная яркость

            // Применяем виньетку
            pixel->r *= factor;
            pixel->g *= factor;
            pixel->b *= factor;

            // Ограничиваем значения
            if (pixel->r > 1.0f) pixel->r = 1.0f;
            if (pixel->g > 1.0f) pixel->g = 1.0f;
            if (pixel->b > 1.0f) pixel->b = 1.0f;
        }
    }

    return true;
}

#endif // CUSTOM_FILTERS_C