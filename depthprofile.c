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


integration_result_t integrate_depthfile(depthfile_t *depthfile, double low, double high) { 
    integration_result_t r;
    int i;
    r.low=depthfile->bins[0].low;
    r.high=depthfile->bins[depthfile->n_depths-1].high;
    r.counts=0;
    r.adensity=0.0;
    r.uniq_id=-1;
    int n=0;
#ifdef DEBUG
    fprintf(stderr, "Integrating depthfile %i from %g to %g\n", depthfile->uniq_id, low, high);
#endif
    for(i=0; i<depthfile->n_depths; i++) {
        depthbin_t *bin=&depthfile->bins[i];
        if(bin->low < low)
            continue;
        if(bin->high > high) {
            r.high=bin->low; /* This bin is (partially) too high, but the low limit should have been included in the previous bin */
            break;
        }
        if(n==0) {
            r.low=bin->low;
        }
        r.adensity += (bin->high-bin->low)*bin->conc;
        r.counts += bin->counts;
        n++;
    }
    r.conc = r.adensity/(r.high - r.low);
    r.uniq_id=depthfile->uniq_id;
#ifdef DEBUG
    fprintf(stderr, "%i out of %i bins inside. Actual range from %g to %g. Conc %g, areal density %g\n", n, depthfile->n_depths, r.low, r.high, r.conc, r.adensity);
#endif
    return r;
}

double areal_density(depthfile_t *depthfile, double depth_low, double depth_high) {
    double sum=0.0;
    return sum;
}

void find_scaling_factor(depthfile_t *depthfiles, depth_scale_t *depthscale) {
    depthfile_t *this;
    double adensity=0.0, conc=0.0;
    int counts=0;
    for(this=depthfiles; this != NULL; this=this->next) {
        if(!this->use_in_scaling)
            continue;
        integration_result_t r=integrate_depthfile(this, depthscale->low, depthscale->high);
        conc += r.conc;
        adensity += r.adensity;
        counts += r.counts;
    }
    /* TODO: depthscale->low or high may be different from actual lows or highs in depthfiles(binning) */
    depthscale->scale=1.0/conc;
}

void print_depthprofile(depthfile_t *depthfile) {
    int i;
    if(depthfile==NULL)
        return;
    for(i=0; i<depthfile->n_depths; i++) {
        fprintf(stdout, "%i %i %5g %5g %7g\n", depthfile->Z, depthfile->A, depthfile->bins[i].low, depthfile->bins[i].high, depthfile->bins[i].conc);
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
        integration_result_t r=integrate_depthfile(this, depthscale->low, depthscale->high);    
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, 3, elements[this->Z].name, this->A, r.conc*100.0, stat_error(r.conc, r.counts)*100.0, r.adensity, r.counts);
    }
  //  fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, ad_all, counts_all); 
    
    find_scaling_factor(depthfiles, depthscale);
    fprintf(out, "Scaling factor: %g\n", depthscale->scale);
    fprintf(out, "SCALED RESULTS FROM %g to %g\n", depthscale->low, depthscale->high);
    fputs(result_header, out);
    for(this=depthfiles; this != NULL; this=this->next) {
        integration_result_t r=integrate_depthfile(this, depthscale->low, depthscale->high);    
        if(!this->use_in_scaling)
            continue;
        fprintf(out, result_line, this->uniq_id, this->use_in_scaling?"Yes":"No", this->Z, 3, elements[this->Z].name, this->A, r.conc*100.0*depthscale->scale, stat_error(r.conc*depthscale->scale, r.counts)*100.0, r.adensity*depthscale->scale, r.counts);

    }
    //fprintf(out, result_footer, conc_all*100.0, stat_error(conc_all, counts_all)*100.0, ad_all, counts_all); 
    if(out != stdout) {
        fclose(out);
    }
}
