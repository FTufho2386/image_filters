#include "image.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Image* image_create(int width, int height) {
    if (width <= 0 || height <= 0) {
        return NULL;
    }

    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) {
        return NULL;
    }

    img->width = width;
    img->height = height;

    // Выделяем память для строк
    img->data = (Pixel**)malloc(height * sizeof(Pixel*));
    if (!img->data) {
        free(img);
        return NULL;
    }

    // Выделяем память для всех пикселей одним блоком
    Pixel* pixels = (Pixel*)malloc(width * height * sizeof(Pixel));
    if (!pixels) {
        free(img->data);
        free(img);
        return NULL;
    }

    // Настраиваем указатели на строки
    for (int y = 0; y < height; y++) {
        img->data[y] = &pixels[y * width];
    }

    // Инициализируем все пиксели черным цветом
    memset(pixels, 0, width * height * sizeof(Pixel));

    return img;
}

void image_destroy(Image* img) {
    if (img) {
        if (img->data) {
            if (img->data[0]) {
                free(img->data[0]);
            }
            free(img->data);
        }
        free(img);
    }
}

Pixel* image_get_pixel(Image* img, int x, int y) {
    if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height) {
        return NULL;
    }
    return &img->data[y][x];
}

void image_set_pixel(Image* img, int x, int y, Pixel pixel) {
    Pixel* p = image_get_pixel(img, x, y);
    if (p) {
        *p = pixel;
    }
}

Image* bmp_load(const char* filename, char** error) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        if (error) *error = "Cannot open file";
        return NULL;
    }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    // Читаем заголовки
    if (fread(&file_header, sizeof(BMPFileHeader), 1, file) != 1) {
        fclose(file);
        if (error) *error = "Cannot read BMP file header";
        return NULL;
    }

    if (fread(&info_header, sizeof(BMPInfoHeader), 1, file) != 1) {
        fclose(file);
        if (error) *error = "Cannot read BMP info header";
        return NULL;
    }

    // Проверяем сигнатуру
    if (file_header.type != 0x4D42) {  // "BM"
        fclose(file);
        if (error) *error = "Not a BMP file";
        return NULL;
    }

    // Проверяем поддерживаемый формат
    if (info_header.bits_per_pixel != 24) {
        fclose(file);
        if (error) *error = "Only 24-bit BMP supported";
        return NULL;
    }

    if (info_header.compression != 0) {
        fclose(file);
        if (error) *error = "Compressed BMP not supported";
        return NULL;
    }

    // Переходим к данным пикселей
    fseek(file, file_header.offset, SEEK_SET);

    // Создаем изображение
    Image* img = image_create(info_header.width, abs(info_header.height));
    if (!img) {
        fclose(file);
        if (error) *error = "Cannot create image";
        return NULL;
    }

    // Вычисляем размер строки с выравниванием
    int row_size = ((info_header.width * 3 + 3) / 4) * 4;
    uint8_t* row_buffer = (uint8_t*)malloc(row_size);
    if (!row_buffer) {
        image_destroy(img);
        fclose(file);
        if (error) *error = "Memory allocation failed";
        return NULL;
    }

    // Читаем данные пикселей
    int height = abs(info_header.height);
    for (int y = 0; y < height; y++) {
        if (fread(row_buffer, 1, row_size, file) != row_size) {
            free(row_buffer);
            image_destroy(img);
            fclose(file);
            if (error) *error = "Cannot read pixel data";
            return NULL;
        }

        // Преобразуем байты в пиксели
        for (int x = 0; x < info_header.width; x++) {
            // В BMP порядок BGR
            uint8_t b = row_buffer[x * 3];
            uint8_t g = row_buffer[x * 3 + 1];
            uint8_t r = row_buffer[x * 3 + 2];

            // Если высота отрицательная, изображение хранится сверху вниз
            int target_y = info_header.height > 0 ? height - 1 - y : y;
            image_set_pixel(img, x, target_y, pixel_from_bytes(r, g, b));
        }
    }

    free(row_buffer);
    fclose(file);

    return img;
}

bool bmp_save(const char* filename, Image* img, char** error) {
    if (!img) {
        if (error) *error = "No image to save";
        return false;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        if (error) *error = "Cannot create file";
        return false;
    }

    // Вычисляем размер строки с выравниванием
    int row_size = ((img->width * 3 + 3) / 4) * 4;
    int image_size = row_size * img->height;

    // Заполняем заголовки
    BMPFileHeader file_header = { 0 };
    BMPInfoHeader info_header = { 0 };

    file_header.type = 0x4D42;  // "BM"
    file_header.size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size;
    file_header.offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = img->width;
    info_header.height = img->height;
    info_header.planes = 1;
    info_header.bits_per_pixel = 24;
    info_header.compression = 0;
    info_header.image_size = image_size;
    info_header.x_pixels_per_meter = 2835;  // Примерно 72 DPI
    info_header.y_pixels_per_meter = 2835;
    info_header.colors_used = 0;
    info_header.colors_important = 0;

    // Записываем заголовки
    if (fwrite(&file_header, sizeof(BMPFileHeader), 1, file) != 1 ||
        fwrite(&info_header, sizeof(BMPInfoHeader), 1, file) != 1) {
        fclose(file);
        if (error) *error = "Cannot write headers";
        return false;
    }

    // Готовим буфер для строки
    uint8_t* row_buffer = (uint8_t*)calloc(1, row_size);
    if (!row_buffer) {
        fclose(file);
        if (error) *error = "Memory allocation failed";
        return false;
    }

    // Записываем данные пикселей (сверху вниз для BMP)
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            Pixel* pixel = image_get_pixel(img, x, y);
            uint8_t r, g, b;
            pixel_to_bytes(*pixel, &r, &g, &b);

            // В BMP порядок BGR
            row_buffer[x * 3] = b;
            row_buffer[x * 3 + 1] = g;
            row_buffer[x * 3 + 2] = r;
        }

        if (fwrite(row_buffer, 1, row_size, file) != row_size) {
            free(row_buffer);
            fclose(file);
            if (error) *error = "Cannot write pixel data";
            return false;
        }
    }

    free(row_buffer);
    fclose(file);
    return true;
}

void print_bmp_info(BMPFileHeader* file_header, BMPInfoHeader* info_header) {
    printf("BMP File Info:\n");
    printf("  Signature: %c%c\n", file_header->type & 0xFF, file_header->type >> 8);
    printf("  File size: %u bytes\n", file_header->size);
    printf("  Data offset: %u\n", file_header->offset);
    printf("  Image size: %dx%d\n", info_header->width, info_header->height);
    printf("  Bits per pixel: %u\n", info_header->bits_per_pixel);
    printf("  Compression: %u\n", info_header->compression);
}