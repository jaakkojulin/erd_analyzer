#ifndef _DEPTHFILE_H_
#define _DEPTHFILE_H_

#include <rgbcolor.h>
#include <elements.h>

typedef struct depthfile {
    int uniq_id;
    char *filename;
    int Z;
    int A; /* Zero if not a specific isotope */
    int n_depths;
    int use_in_scaling;
    int plot;
    rgbcolor_t *color;
    double *depths; /* table of n_depths */
    double *concentrations; /* table of n_depths */
    int *counts; /* table of n_depths */
    struct depthfile *next;
} depthfile_t;


int count_lines(const char *filename);
void print_depthfiles(depthfile_t *depthfiles);
void print_depthfile_chain(depthfile_t *depthfiles);
depthfile_t *load_depthfile(rgbcolor_t *colors, char *filename, int Z, int A);
depthfile_t *join_depthfiles(depthfile_t *a, depthfile_t *b);
depthfile_t *previous_depthfile(depthfile_t *depthfiles, depthfile_t *a);
depthfile_t *depthfile_swap(depthfile_t *depthfiles, depthfile_t *a, depthfile_t *b);
depthfile_t *destroy_depthfile(depthfile_t *depthfiles, depthfile_t *depthfile);
depthfile_t *add_depthfile(depthfile_t *depthfiles, depthfile_t *depthfile);
depthfile_t *find_depthfile_by_name(depthfile_t *depthfiles, element_t *elements, char *str);
depthfile_t *sort_depthfiles(depthfile_t *depthfiles);
depthfile_t *load_depthfiles_by_prefix(element_t *elements, depthfile_t *depthfiles, rgbcolor_t *colors, char *prefix);
int number_of_depthfiles(depthfile_t *depthfiles);

#endif