#include "filters.h"
#include "custom_filters.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

// Таблица доступных фильтров
Filter available_filters[] = {
    {"crop", filter_crop, 2, 2},
    {"gs", filter_grayscale, 0, 0},
    {"neg", filter_negative, 0, 0},
    {"sharp", filter_sharpening, 0, 0},
    {"edge", filter_edge_detection, 1, 1},
    {"med", filter_median, 1, 1},
    {"blur", filter_gaussian_blur, 1, 1},
    {"crystallize", filter_crystallize, 0, 0},
    {"glass", filter_glass_distortion, 0, 0},
    {"sepia", filter_sepia, 0, 0},
    {"vignette", filter_vignette, 0, 0}
};

int filter_count = sizeof(available_filters) / sizeof(available_filters[0]);

// Вспомогательная функция для получения пикселя с учетом границ
Pixel get_pixel_with_padding(Image* img, int x, int y) {
    // Ограничиваем координаты границами изображения
    int clamped_x = x;
    if (clamped_x < 0) clamped_x = 0;
    if (clamped_x >= img->width) clamped_x = img->width - 1;

    int clamped_y = y;
    if (clamped_y < 0) clamped_y = 0;
    if (clamped_y >= img->height) clamped_y = img->height - 1;

    return img->data[clamped_y][clamped_x];
}

// Функция для применения матричного фильтра 3x3
bool apply_matrix_filter(Image* img, float kernel[3][3], char** error) {
    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Создаем временное изображение для результата
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Применяем матрицу к каждому пикселю
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel sum = { 0, 0, 0 };

            // Свертка с ядром 3x3
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    Pixel pixel = get_pixel_with_padding(img, x + kx, y + ky);
                    float weight = kernel[ky + 1][kx + 1];

                    sum.r += pixel.r * weight;
                    sum.g += pixel.g * weight;
                    sum.b += pixel.b * weight;
                }
            }

            // Ограничиваем значения
            sum.r = sum.r < 0.0f ? 0.0f : (sum.r > 1.0f ? 1.0f : sum.r);
            sum.g = sum.g < 0.0f ? 0.0f : (sum.g > 1.0f ? 1.0f : sum.g);
            sum.b = sum.b < 0.0f ? 0.0f : (sum.b > 1.0f ? 1.0f : sum.b);

            image_set_pixel(temp, x, y, sum);
        }
    }

    // Копируем результат обратно в исходное изображение
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

// Функция для сравнения значений float (для сортировки каналов)
int compare_floats(const void* a, const void* b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;

    if (fa < fb) return -1;
    if (fa > fb) return 1;
    return 0;
}

// Реализация фильтра Crop
bool filter_crop(Image* img, int argc, char** argv, char** error) {
    if (argc < 2) {
        if (error) *error = "Crop filter requires width and height parameters";
        return false;
    }

    int new_width = atoi(argv[0]);
    int new_height = atoi(argv[1]);

    if (new_width <= 0 || new_height <= 0) {
        if (error) *error = "Invalid crop dimensions";
        return false;
    }

    // Ограничиваем запрошенные размеры реальными размерами изображения
    if (new_width > img->width) new_width = img->width;
    if (new_height > img->height) new_height = img->height;

    // Создаем новое изображение нужного размера
    Image* cropped = image_create(new_width, new_height);
    if (!cropped) {
        if (error) *error = "Cannot create cropped image";
        return false;
    }

    // Копируем пиксели из верхнего левого угла
    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {
            Pixel* src_pixel = image_get_pixel(img, x, y);
            if (src_pixel) {
                image_set_pixel(cropped, x, y, *src_pixel);
            }
        }
    }

    // Заменяем исходное изображение обрезанным
    // Для этого нам нужно скопировать данные из cropped в img
    // Сначала освобождаем старые данные
    if (img->data) {
        if (img->data[0]) {
            free(img->data[0]);
        }
        free(img->data);
    }

    // Копируем размеры
    img->width = cropped->width;
    img->height = cropped->height;

    // Выделяем новую память
    img->data = (Pixel**)malloc(cropped->height * sizeof(Pixel*));
    if (!img->data) {
        image_destroy(cropped);
        if (error) *error = "Memory allocation failed";
        return false;
    }

    Pixel* pixels = (Pixel*)malloc(cropped->width * cropped->height * sizeof(Pixel));
    if (!pixels) {
        free(img->data);
        image_destroy(cropped);
        if (error) *error = "Memory allocation failed";
        return false;
    }

    // Копируем данные
    memcpy(pixels, cropped->data[0], cropped->width * cropped->height * sizeof(Pixel));

    // Настраиваем указатели
    for (int y = 0; y < cropped->height; y++) {
        img->data[y] = &pixels[y * cropped->width];
    }

    // Освобождаем временное изображение
    cropped->data[0] = NULL; // Чтобы не освобождать скопированные данные
    cropped->data = NULL;
    image_destroy(cropped);

    return true;
}

// Реализация фильтра Grayscale
bool filter_grayscale(Image* img, int argc, char** argv, char** error) {
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

            float luminance = pixel_luminance(*pixel);
            *pixel = pixel_create(luminance, luminance, luminance);
        }
    }

    return true;
}

// Реализация фильтра Negative
bool filter_negative(Image* img, int argc, char** argv, char** error) {
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

            pixel->r = 1.0f - pixel->r;
            pixel->g = 1.0f - pixel->g;
            pixel->b = 1.0f - pixel->b;
        }
    }

    return true;
}

// Реализация фильтра Sharpening
bool filter_sharpening(Image* img, int argc, char** argv, char** error) {
    (void)argc;
    (void)argv;

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Матрица для повышения резкости
    float kernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    return apply_matrix_filter(img, kernel, error);
}

// Реализация фильтра Edge Detection
bool filter_edge_detection(Image* img, int argc, char** argv, char** error) {
    if (argc < 1) {
        if (error) *error = "Edge detection requires threshold parameter";
        return false;
    }

    float threshold = (float)atof(argv[0]);
    if (threshold < 0.0f || threshold > 1.0f) {
        if (error) *error = "Threshold must be between 0 and 1";
        return false;
    }

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Сначала преобразуем в оттенки серого
    filter_grayscale(img, 0, NULL, error);

    // Матрицы Собеля для вычисления градиентов
    float sobel_x[3][3] = {
        { 1,  0, -1},
        { 2,  0, -2},
        { 1,  0, -1}
    };

    float sobel_y[3][3] = {
        { 1,  2,  1},
        { 0,  0,  0},
        {-1, -2, -1}
    };

    // Создаем временное изображение для результата
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Вычисляем градиенты для каждого пикселя
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            float grad_x = 0.0f;
            float grad_y = 0.0f;

            // Свертка с матрицами Собеля
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    Pixel pixel = get_pixel_with_padding(img, x + kx, y + ky);
                    float intensity = pixel.r; // Все каналы одинаковые после grayscale

                    grad_x += intensity * sobel_x[ky + 1][kx + 1];
                    grad_y += intensity * sobel_y[ky + 1][kx + 1];
                }
            }

            // Вычисляем величину градиента
            float magnitude = sqrtf(grad_x * grad_x + grad_y * grad_y);

            // Применяем порог
            Pixel result;
            if (magnitude > threshold) {
                result = pixel_create(1.0f, 1.0f, 1.0f); // Белый
            }
            else {
                result = pixel_create(0.0f, 0.0f, 0.0f); // Черный
            }

            image_set_pixel(temp, x, y, result);
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

// Реализация медианного фильтра (по каналам)
bool filter_median(Image* img, int argc, char** argv, char** error) {
    if (argc < 1) {
        if (error) *error = "Median filter requires window size parameter";
        return false;
    }

    int window_size = atoi(argv[0]);
    if (window_size <= 0 || window_size % 2 == 0) {
        if (error) *error = "Window size must be positive odd number";
        return false;
    }

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    int radius = window_size / 2;
    int window_area = window_size * window_size;

    // Выделяем память для временных массивов
    float* r_values = (float*)malloc(window_area * sizeof(float));
    float* g_values = (float*)malloc(window_area * sizeof(float));
    float* b_values = (float*)malloc(window_area * sizeof(float));

    if (!r_values || !g_values || !b_values) {
        free(r_values);
        free(g_values);
        free(b_values);
        if (error) *error = "Memory allocation failed";
        return false;
    }

    // Создаем временное изображение для результата
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        free(r_values);
        free(g_values);
        free(b_values);
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Применяем медианный фильтр
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int count = 0;

            // Собираем значения из окна
            for (int wy = -radius; wy <= radius; wy++) {
                for (int wx = -radius; wx <= radius; wx++) {
                    Pixel pixel = get_pixel_with_padding(img, x + wx, y + wy);

                    r_values[count] = pixel.r;
                    g_values[count] = pixel.g;
                    b_values[count] = pixel.b;
                    count++;
                }
            }

            // Сортируем значения каждого канала
            qsort(r_values, count, sizeof(float), compare_floats);
            qsort(g_values, count, sizeof(float), compare_floats);
            qsort(b_values, count, sizeof(float), compare_floats);

            // Берем медианное значение
            int median_index = count / 2;
            Pixel median_pixel = {
                r_values[median_index],
                g_values[median_index],
                b_values[median_index]
            };

            image_set_pixel(temp, x, y, median_pixel);
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
    free(r_values);
    free(g_values);
    free(b_values);
    image_destroy(temp);

    return true;
}

// Реализация Гауссова размытия
bool filter_gaussian_blur(Image* img, int argc, char** argv, char** error) {
    if (argc < 1) {
        if (error) *error = "Gaussian blur requires sigma parameter";
        return false;
    }

    float sigma = (float)atof(argv[0]);
    if (sigma <= 0.0f) {
        if (error) *error = "Sigma must be positive";
        return false;
    }

    if (!img) {
        if (error) *error = "No image provided";
        return false;
    }

    // Вычисляем размер ядра (правило 3?)
    int radius = (int)ceilf(3 * sigma);
    int kernel_size = 2 * radius + 1;

    // Создаем и заполняем ядро Гаусса
    float* kernel = (float*)malloc(kernel_size * sizeof(float));
    if (!kernel) {
        if (error) *error = "Memory allocation failed";
        return false;
    }

    float sum = 0.0f;
    for (int i = 0; i < kernel_size; i++) {
        int x = i - radius;
        kernel[i] = expf(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }

    // Нормализуем ядро
    for (int i = 0; i < kernel_size; i++) {
        kernel[i] /= sum;
    }

    // Создаем временное изображение для промежуточных результатов
    Image* temp = image_create(img->width, img->height);
    if (!temp) {
        free(kernel);
        if (error) *error = "Cannot create temporary image";
        return false;
    }

    // Применяем размытие по горизонтали
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel sum = { 0, 0, 0 };

            for (int kx = -radius; kx <= radius; kx++) {
                Pixel pixel = get_pixel_with_padding(img, x + kx, y);
                float weight = kernel[kx + radius];

                sum.r += pixel.r * weight;
                sum.g += pixel.g * weight;
                sum.b += pixel.b * weight;
            }

            image_set_pixel(temp, x, y, sum);
        }
    }

    // Применяем размытие по вертикали к результату горизонтального размытия
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            Pixel sum = { 0, 0, 0 };

            for (int ky = -radius; ky <= radius; ky++) {
                Pixel pixel = get_pixel_with_padding(temp, x, y + ky);
                float weight = kernel[ky + radius];

                sum.r += pixel.r * weight;
                sum.g += pixel.g * weight;
                sum.b += pixel.b * weight;
            }

            // Ограничиваем значения
            sum.r = sum.r < 0.0f ? 0.0f : (sum.r > 1.0f ? 1.0f : sum.r);
            sum.g = sum.g < 0.0f ? 0.0f : (sum.g > 1.0f ? 1.0f : sum.g);
            sum.b = sum.b < 0.0f ? 0.0f : (sum.b > 1.0f ? 1.0f : sum.b);

            image_set_pixel(img, x, y, sum);
        }
    }

    // Освобождаем память
    free(kernel);
    image_destroy(temp);

    return true;
}