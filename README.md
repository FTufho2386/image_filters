# Image Craft - BMP Image Processor

Лабораторная работа по программированию на языке C. Консольное приложение для обработки BMP изображений с применением различных фильтров.

## Описание
Программа позволяет применять различные фильтры к 24-битным BMP изображениям через командную строку. Поддерживается последовательное применение нескольких фильтров за один запуск.

## Возможности
- Загрузка и сохранение 24-битных BMP изображений
- 7 базовых фильтров (Crop, Grayscale, Negative, Sharpening, Edge Detection, Median Filter, Gaussian Blur)
- 4 дополнительных фильтра (Crystallize, Glass Distortion, Sepia, Vignette)
- Пакетная обработка изображений

## Сборка

### На Linux/Mac:
```bash
gcc color.c image.c filters.c custom_filters.c image_craft.c -o image_craft -lm
