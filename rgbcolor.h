#ifndef _RGBCOLOR_H_
#define _RGBCOLOR_H_

typedef struct rgbcolor {
    char *name;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a; /* Unused */
    struct rgbcolor *next;
} rgbcolor_t;

rgbcolor_t *find_color(rgbcolor_t *colors, char *name);
void print_colors(rgbcolor_t *colors);
void print_color(rgbcolor_t *color);
rgbcolor_t *load_colors(char *filename, rgbcolor_t *first);

#endif
