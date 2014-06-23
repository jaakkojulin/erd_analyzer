#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <depthfile.h>
#include <depthprofile.h>

double stat_error(double value, int counts) {
    if(counts < 2)
        return(1.0);
    return(value/(sqrt(1.0*(counts-1))));
}

double average_concentration(depthfile_t *depthfile, double depth_low, double depth_high, double *sum_out, int *counts_out) {
    /* Integrates between low and high depths, return result is the avg concentration, *sum is the result,of integration and counts are also given as output */ 
    int i;
    int n=0;
    int counts=0;
    double sum=0.0;
    if(!depthfile)
        return 0.0;
    for(i=0; i<depthfile->n_depths; i++) {
        if(depthfile->depths[i] >= depth_low && depthfile->depths[i] < depth_high) {
            sum += depthfile->concentrations[i];
            counts += depthfile->counts[i];
            n++;
        }
    }
    if(counts_out)
        *counts_out=counts;
    if(sum_out)
        *sum_out=sum;
    return(sum/(1.0*n));
}

void find_scaling_factor(depthfile_t *depthfiles, depth_scale_t *depthscale) {
    depthfile_t *this;
    double sum, sum_all=0.0, conc, conc_all=0.0;
    int counts, counts_all=0;
    for(this=depthfiles; this != NULL; this=this->next) {
        conc=average_concentration(this, depthscale->low, depthscale->high, &sum, &counts);
        if(this->use_in_scaling) {
            conc_all += conc;
            sum_all += sum;
            counts_all += counts;
        } else {
            conc=0.0;
            sum=0;
            counts=0;
        }
    }
    depthscale->scale=1.0/conc_all;
}

void print_depthprofile(depthfile_t *depthfile) {
    int i;
    if(depthfile==NULL)
        return;
    for(i=0; i<depthfile->n_depths; i++) {
        fprintf(stdout, "%i %i %g\t%g\n", depthfile->Z, depthfile->A, depthfile->depths[i], depthfile->concentrations[i]);
    }
}

void make_results_table(depthfile_t *depthfiles, element_t *elements, char *filename, depth_scale_t *depthscale) {
    depthfile_t *this;
    double sum_all=0.0, conc_all=0.0, sum, conc;
    int counts_all, counts;
    FILE *out=NULL;
    static const char *result_header="ID\tUse?\tZ\tA\tConc\tErr\tCounts\n---------------------------------------------------\n";
    static const char *result_line="%i\t%s\t%i (%s)\t%i\t%.3f\t%.3f\t%i\n";
    static const char *result_footer="Total:\t\t\t%.3f\t%.3f\t%i\n---------------------------------------------------\n";   
    
    if(filename) {
        out=fopen(filename, "w");
    }
    if(!out) {
        out=stdout;
    }
    
    fprintf(out, "UNSCALED RESULTS\n");
    fputs(result_header, out);
    for(this=depthfiles; this != NULL; this=this->next) {
        conc=average_concentration(this, depthscale->low, depthscale->high, &sum, &counts);
        if(this->use_in_scaling) {
            conc_all += conc;
            sum_all += sum;
            counts_all += counts;
        } else {
            conc=0.0;
            sum=0;
            counts=0;
        }
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, elements[this->Z].name, this->A, conc*100.0, stat_error(conc, counts)*100.0, counts);
    }
    fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, counts_all); 
    
    conc_all=0;
    fprintf(out, "SCALED RESULTS\n");
    fputs(result_header, out);
    for(this=depthfiles; this != NULL; this=this->next) {
        conc=average_concentration(this, depthscale->low, depthscale->high, &sum, &counts)*depthscale->scale;
        if(this->use_in_scaling) {
            conc_all += conc;
        } else {
            conc=0.0;
        }
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, elements[this->Z].name, this->A, conc*100.0, stat_error(conc, counts)*100.0, counts);
    }
    fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, counts_all); 
    if(out != stdout) {
        fclose(out);
    }
}
