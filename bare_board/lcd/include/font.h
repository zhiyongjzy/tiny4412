#pragma once

struct font_desc {
    const char *name;
    int width, height;
    const void *data;
};

// #define FONT_8x16
#define FONT_12x22