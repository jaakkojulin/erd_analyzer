#include "round.h"

#ifndef HAVE_EXP10
inline double exp10(double val) {
    return pow(10.0, val);
}
#define HAVE_EXP10 1
#endif

int fprint_value_full(FILE *stream, value_err_t val) {
    return fprintf(stream, "%0.*f +- %0.*f", val.err_places, val.value, val.err_places, val.err);
}

int snprint_value_full(char *str, size_t size, value_err_t val) {
    return snprintf(str, size, "%0.*f +- %0.*f", val.err_places, val.value, val.err_places, val.err);
}

value_err_t value_from_numbers(double value, double err) {
    double err_mag=floor(log10(err));
    double err_norm=err*exp10(-err_mag);
    value_err_t rounded;
    if(err_norm <= 1.5) { /* Uncertainty of +- 1.31 is rounded to 1.4, uncertainty of 1.6 is rounded to 2 */
        rounded.err=(ceil(err_norm*10.0)/10.0)*exp10(err_mag); /* Two significant digits */
        err_mag -= 1.0;
    } else {
        rounded.err=ceil(err_norm)*exp10(err_mag);
    }
    rounded.err_places = (int)(-err_mag);
    rounded.value=round(exp10(-err_mag)*value)*exp10(err_mag);
#ifdef DEBUG
    fprintf(stderr, "error: %g, after scaling: %g, rounded: %g (magnitude: %g)\n", err, err_norm, rounded.err, err_mag);
    fprintf(stderr, "value: %g, after rounding %g\n", value, rounded.value);
#endif
    return rounded;
}
