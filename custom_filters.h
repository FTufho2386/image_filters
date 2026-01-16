#ifndef CUSTOM_FILTERS_H
#define CUSTOM_FILTERS_H

#include "image.h"

// Дополнительные фильтры
bool filter_crystallize(Image* img, int argc, char** argv, char** error);
bool filter_glass_distortion(Image* img, int argc, char** argv, char** error);
bool filter_sepia(Image* img, int argc, char** argv, char** error);
bool filter_vignette(Image* img, int argc, char** argv, char** error);

#endif // CUSTOM_FILTERS_H