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

int low_index_for_depth(depthfile_t *depthfile, double depth) { /* -1 ... n */
    int index=depthfile->n_depths; /* Case: depth > highest */
    int i;
    for(i=0; i<depthfile->n_depths; i++) {
        if(depthfile->depths[i] >= depth) {
            index=i-1;
        }
    }
    fprintf(stderr, "Low index for %g is %i (depth %g)\n", depth, index, depthfile->depths[index>=0?index:0]);
    return index;
}

double areal_density(depthfile_t *depthfile, double depth_low, double depth_high) {
    int i;
    double t;
    double sum=0.0;

    for(i=1; i<depthfile->n_depths; i++) {
        if(depthfile->depths[i-1] < depth_low && depthfile->depths[i] < depth_low) /* too low, continue */
            continue;
        if(depthfile->depths[i-1] > depth_high && depthfile->depths[i] > depth_high) /* too high, stop */
            break;
        t=depthfile->depths[i]-depthfile->depths[i-1]; /* typical case */
        if(depthfile->depths[i-1] < depth_low) {  /* lowest bin, partial */
            t=depthfile->depths[i]-depth_low;
//            fprintf(stderr, "Partial bin low i=%i t=%g\n", i, t);
            if(i==1) {
                fprintf(stderr, "Ignoring!\n");
                t=0.0;
            }
        }
        if(depthfile->depths[i] > depth_high) { /* highest bin, partial */
            t=depth_high-depthfile->depths[i-1];
//            fprintf(stderr, "Partial bin high i=%i t=%g\n", i, t);
            if(i==depthfile->n_depths-1) {
                fprintf(stderr, "IGnoring!\n");
                t=0.0;
            }
        }
        sum += depthfile->concentrations[i]*t;
    }
    return sum;
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
    double sum_all=0.0, conc_all=0.0, sum, conc, ad, ad_all=0.0;
    int counts_all, counts;
    FILE *out=NULL;
    static const char *result_header=" ID Use?   Z         A   Conc   Err  Areal d.  Counts\n------------------------------------------------------\n";
    /*                               "666 Yes  zzz (Zz) aaa  66.66  66.66   xxxxxx  666666  */
    static const char *result_line="%3i  %3s %3i (%*s) %3i  %5.2f  %5.2f   %6.1f  %6i\n";
    static const char *result_footer="Total:                %7.2f %6.2f   %6.1f %7i\n------------------------------------------------------\n\n";   
    
    if(filename) {
        out=fopen(filename, "w");
    }
    if(!out) {
        out=stdout;
    }
    
    fprintf(out, "UNSCALED RESULTS FROM %g to %g\n", depthscale->low, depthscale->high);
    fputs(result_header, out);
    for(this=depthfiles; this != NULL; this=this->next) {
        conc=average_concentration(this, depthscale->low, depthscale->high, &sum, &counts);
        ad=areal_density(this, depthscale->low, depthscale->high);
        if(this->use_in_scaling) {
            conc_all += conc;
            ad_all += ad;
            sum_all += sum;
            counts_all += counts;
        } else {
            conc=0.0;
            sum=0;
            counts=0;
            ad=0.0;
        }
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, 3, elements[this->Z].name, this->A, conc*100.0, stat_error(conc, counts)*100.0, ad, counts);
    }
    fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, ad_all, counts_all); 
    
    conc_all=0;
    fprintf(out, "SCALED RESULTS FROM %g to %g\n", depthscale->low, depthscale->high);
    fputs(result_header, out);
    for(this=depthfiles; this != NULL; this=this->next) {
        conc=average_concentration(this, depthscale->low, depthscale->high, &sum, &counts)*depthscale->scale;
        ad=areal_density(this, depthscale->low, depthscale->high)*depthscale->scale;
        if(this->use_in_scaling) {
            conc_all += conc;
            ad_all += ad;
        } else {
            conc=0.0;
        }
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, 3, elements[this->Z].name, this->A, conc*100.0, stat_error(conc, counts)*100.0, ad, counts);
    }
    fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, ad_all, counts_all); 
    if(out != stdout) {
        fclose(out);
    }
}
