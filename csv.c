#include <stdio.h>
#include "csv.h"

void create_csv(depthfile_t *depthfiles, element_t *elements, depth_scale_t *depthscale, char *filename) {
    int i;
    if(!depthfiles || !depthscale || !filename)
        return;
    depthfile_t *first=depthfiles;
    depthfile_t *this;
    FILE *out;

    while(!first->plot) {first = first->next;}
    if(!first)
        return;
    for(this=depthfiles; this != NULL;  this=this->next) {
        if(!this->plot)
            continue;
        for(i=0; i<this->n_depths; i++) {  /* Check depth binning, compare all files to the first one .*/
            if(this->bins[i].low != first->bins[i].low || this->bins[i].high != first->bins[i].high) {
                fprintf(stderr, "Cannot create a CSV file, depthscales differ.\n");
            }
        }
    }
    out=fopen(filename, "w");
    if(!out) {
        fprintf(stderr, "Could not open file %s for writing!\n", filename);
        return;
    } 

#ifndef NO_CSV_HEADER
    fprintf(out, "\"low\",\"high\",\"center\"");
    for(this=depthfiles; this != NULL;  this=this->next) {
        if(!this->plot)
             continue;
        if(this->A) {
            fprintf(out, ",\"%i%s\"",this->A, elements[this->Z].name);
        } else {
            fprintf(out, ",\"%s\"", elements[this->Z].name);
        }
     }
     fprintf(out, "\n");
#endif

    for(i=0; i<first->n_depths; i++) { 
        fprintf(out, "%g,%g,%g", first->bins[i].low, first->bins[i].high, (first->bins[i].low+first->bins[i].high)/2.0);
        for(this=depthfiles; this != NULL;  this=this->next) {
            if(!this->plot)
                continue;
            fprintf(out, ",%.3lf", 100.0*this->bins[i].conc * depthscale->scale);
        }
        fprintf(out, "\n");
    }
    fclose(out);
}
