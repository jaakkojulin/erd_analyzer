#ifndef _DEPTHPROFILE_H_
#define _DEPTHPROFILE_H_

#include "depthfile.h"
#include "elements.h"

typedef struct depth_scale {
    double low;
    double high;
    double scale;
    double density;
} depth_scale_t;

typedef struct integration_result {
    int uniq_id; /* Just in case. Should correspond to a uniq_id of a depth profile */
    double conc;
    double adensity;
    int counts;
    double low;
    double high;
    depthfile_t *depthfile;
} integration_result_t;

integration_result_t integrate_depthfile(depthfile_t *depthfile, double low, double high);
double element_ratio(depthfile_t *dividend, depthfile_t *divisor, depth_scale_t *depthscale);
double average_concentration(depthfile_t *depthfile, double depth_low, double depth_high, double *sum_out, int *counts_out);
void find_scaling_factor(depthfile_t *depthfiles, depth_scale_t *depthscale);
void print_scaled_concs(const integration_result_t *results, int n_files, const element_t *elements, const depth_scale_t *depthscale, FILE *out);
void make_results_table(depthfile_t *depthfiles, element_t *elements, char *filename, depth_scale_t *depthscale);
double stat_error(double value, int counts);
double stat_error_normalized(const integration_result_t *results, int n, int i_this, const depth_scale_t *depthscale);
#endif
