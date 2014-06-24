#ifndef _PLOT_H_
#define _PLOT_H_


#include <rgbcolor.h>
#include <elements.h>
#include <depthfile.h>
#include <depthprofile.h>

typedef struct plot_options {
    int x_autoscale; /* Let gnuplot handle x scales */
    int y_autoscale; /* and y */
    int scalinglines; /* "arrows" plotted to indicate integration range */
    double linewidth;
    double x_low;
    double x_high;
    double y_low;
    double y_high;
} plot_options_t;

void create_plotfile(depthfile_t *depthfiles, element_t *elements, rgbcolor_t *colors, depth_scale_t *depthscale, plot_options_t *plot_options, char *headers_filename, char *filename);
void show_plotinfo(depthfile_t *depthfiles, element_t *elements);

#endif
