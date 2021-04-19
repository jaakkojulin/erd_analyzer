#ifndef _ROUND_H_
#define _ROUND_H_
#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>

typedef struct value_err {
    double value;
    double err;
    int err_places; /* Number of decimal places */
} value_err_t;

int fprint_value_full(FILE *stream, value_err_t val);
int snprint_value_full(char *str, size_t size, value_err_t val);
value_err_t value_from_numbers(double value, double err, int min_places);
#endif // _ROUND_H_
