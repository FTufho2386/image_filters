#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "filters.h"

void print_help() {
    printf("Usage: image_craft <input.bmp> <output.bmp> [filters...]\n");
    printf("\nBasic filters:\n");
    printf("  -crop width height      Crop image\n");
    printf("  -gs                     Convert to grayscale\n");
    printf("  -neg                    Convert to negative\n");
    printf("  -sharp                  Apply sharpening\n");
    printf("  -edge threshold         Edge detection\n");
    printf("  -med window_size        Median filter\n");
    printf("  -blur sigma             Gaussian blur\n");
    printf("\nAdditional filters:\n");
    printf("  -crystallize            Crystallize effect (Voronoi cells)\n");
    printf("  -glass                  Glass distortion effect\n");
    printf("  -sepia                  Apply sepia tone\n");
    printf("  -vignette               Apply vignette effect\n");
    printf("\nExamples:\n");
    printf("  image_craft input.bmp output.bmp -crop 800 600 -gs -blur 0.5\n");
    printf("  image_craft input.bmp output.bmp -neg -vignette\n");
    printf("  image_craft input.bmp output.bmp -crystallize -sepia\n");
}

int main(int argc, char* argv[]) {
    // Проверка аргументов командной строки
    if (argc < 3) {
        print_help();
        return 0;
    }

    const char* input_filename = argv[1];
    const char* output_filename = argv[2];

    printf("Loading image: %s\n", input_filename);

    // Загружаем изображение
    char* error = NULL;
    Image* img = bmp_load(input_filename, &error);
    if (!img) {
        fprintf(stderr, "Error loading image: %s\n", error);
        return 1;
    }

    printf("Image loaded: %dx%d pixels\n", img->width, img->height);

    // Обрабатываем фильтры, если они указаны
    if (argc > 3) {
        for (int i = 3; i < argc; i++) {
            if (argv[i][0] == '-') {
                char* filter_name = argv[i] + 1; // Пропускаем '-'

                // Ищем фильтр в таблице
                Filter* filter = NULL;
                for (int j = 0; j < filter_count; j++) {
                    if (strcmp(available_filters[j].name, filter_name) == 0) {
                        filter = &available_filters[j];
                        break;
                    }
                }

                if (!filter) {
                    fprintf(stderr, "Unknown filter: %s\n", filter_name);
                    image_destroy(img);
                    return 1;
                }

                // Собираем аргументы для фильтра
                int args_count = 0;
                char** filter_args = NULL;

                // Подсчитываем аргументы фильтра
                int j = i + 1;
                while (j < argc && argv[j][0] != '-') {
                    args_count++;
                    j++;
                }

                // Проверяем количество аргументов
                if (args_count < filter->min_args ||
                    (filter->max_args != -1 && args_count > filter->max_args)) {
                    fprintf(stderr, "Invalid number of arguments for filter %s\n", filter_name);
                    image_destroy(img);
                    return 1;
                }

                // Если есть аргументы, копируем их
                if (args_count > 0) {
                    filter_args = &argv[i + 1];
                }

                printf("Applying filter: %s\n", filter_name);

                // Применяем фильтр
                if (!filter->function(img, args_count, filter_args, &error)) {
                    fprintf(stderr, "Error applying filter %s: %s\n", filter_name, error);
                    image_destroy(img);
                    return 1;
                }

                // Пропускаем обработанные аргументы
                i += args_count;
            }
        }
    }

    // Сохраняем результат
    printf("Saving image: %s\n", output_filename);
    if (!bmp_save(output_filename, img, &error)) {
        fprintf(stderr, "Error saving image: %s\n", error);
        image_destroy(img);
        return 1;
    }

    // Освобождаем память
    image_destroy(img);

    printf("Done!\n");
    return 0;
}