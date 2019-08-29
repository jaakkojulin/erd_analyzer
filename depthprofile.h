#ifndef _DEPTHPROFILE_H_
#define _DEPTHPROFILE_H_

#include <depthfile.h>
#include <elements.h>

typedef struct depth_scale {
    double low;
    double high;
    double scale;
} depth_scale_t;

double areal_density(depthfile_t *depthfile, double depth_low, double depth_high);
double average_concentration(depthfile_t *depthfile, double depth_low, double depth_high, double *sum_out, int *counts_out);
void find_scaling_factor(depthfile_t *depthfiles, depth_scale_t *depthscale);
void make_results_table(depthfile_t *depthfiles, element_t *elements, char *filename, depth_scale_t *depthscale);
double stat_error(double value, int counts);
#endif
