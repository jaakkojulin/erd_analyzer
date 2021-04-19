#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "plot.h"

#define MAX_LINE_LEN 100

void create_plotfile(depthfile_t *depthfiles, element_t *elements, rgbcolor_t *colors, depth_scale_t *depthscale, plot_options_t *plot_options, char *headers_filename, char *filename) {
    depthfile_t *this=depthfiles;
    FILE *in, *out, *outdata;
    char *line;
    int i, n=0;
    char *plotfilename=calloc(strlen(filename)+5, sizeof(char));
    char *plotdatafilename=calloc(strlen(filename)+5, sizeof(char));
    strncpy(plotfilename, filename, strlen(filename));
    strncpy(plotdatafilename, filename, strlen(filename));
    strcat(plotfilename, ".plt");
    strcat(plotdatafilename, ".dat");
    in=fopen(headers_filename, "r");
    out=fopen(plotfilename, "w");
    outdata=fopen(plotdatafilename, "w");
    if(!out) {
        fprintf(stderr, "Could not open file %s for writing!\n", plotfilename);
        free(plotfilename);
        free(plotdatafilename);
        return;
    }
    if(!outdata) {
        fprintf(stderr, "Could not open file %s for writing!\n", plotdatafilename);
        free(plotfilename);
        free(plotdatafilename);
        return;
    }
    if(in) {
        line = malloc(sizeof(char)*(MAX_LINE_LEN));
        while(fgets(line, MAX_LINE_LEN, in)) {
            fputs(line, out);
        }
        fclose(in);
        free(line);
    }
    /* Write a SCALE= value according to "results" */
    /* Count how many depthfiles.*/
    n=number_of_depthfiles(depthfiles);
    if(plot_options->scalinglines) {
        fprintf(out, "set arrow 1 from %g,%g to %g,%g nohead lt 0 lw %g\n", depthscale->low, plot_options->y_autoscale?0.0:plot_options->y_low, depthscale->low, plot_options->y_autoscale?100.0:plot_options->y_high, plot_options->linewidth);
        fprintf(out, "set arrow 2 from %g,%g to %g,%g nohead lt 0 lw %g\n", depthscale->high, plot_options->y_autoscale?0.0:plot_options->y_low, depthscale->high, plot_options->y_autoscale?100.0:plot_options->y_high, plot_options->linewidth);
    }
    i=0;
    if(!plot_options->x_autoscale) {
        fprintf(out, "set xrange [%g:%g]\n", plot_options->x_low, plot_options->x_high);
    }
    if(!plot_options->y_autoscale) {
        fprintf(out, "set yrange [%g:%g]\n", plot_options->y_low, plot_options->y_high);
    }
    fprintf(out, "plot \\\n");
    char *title=NULL;
    for(this=depthfiles; this != NULL; this=this->next) {
        if(!this->plot)
            continue;
        if(this->color == NULL) {
            this->color = colors; /* The first color is the default */ 
        }
        if(this->A) {
            asprintf(&title, "{}^{%i}%s", this->A, elements[this->Z].name);
        } else {
            asprintf(&title, "%s", elements[this->Z].name);
        }
        fprintf(out, "'%s' index %i using 1:2 with lines lt 1 lw %g lc rgbcolor \"#%02x%02x%02x\" title \"%s\"%s", plotdatafilename, i, plot_options->linewidth, this->color->r, this->color->g, this->color->b, title, (i<n-1)?", \\\n":"\n");
        i++;
    }
    free(title);
    for(this=depthfiles; this != NULL;  this=this->next) {
        if(!this->plot)
            continue;
        for(i=0; i<this->n_depths-1; i++) {
            if(plot_options->nanometers) {
                fprintf(outdata, "%g", 10.0*0.5*(this->bins[i].low_ug + this->bins[i+1].low_ug)/(depthscale->density));
            } else {
                fprintf(outdata, "%g", (this->bins[i].low+this->bins[i+1].low)/2.0);
            }
            fprintf(outdata, " %g", 100.0*this->bins[i].conc * depthscale->scale);
            fprintf(outdata, "\n");
        }
        fprintf(outdata, "\n\n");
    }
    fclose(out);
    fclose(outdata);
    free(plotfilename);
    free(plotdatafilename);
}
void show_plotinfo(depthfile_t *depthfiles, element_t *elements) {
    depthfile_t *this;
    fprintf(stderr, "ID\tPlot?\tZ\tA\tColor\n");
    fprintf(stderr, "---------------------------------------------------\n");    
    for(this=depthfiles; this != NULL; this=this->next) {
        fprintf(stderr, "%i\t%s\t%i (%s)\t%i\t%s\n", this->uniq_id, this->plot?"Yes":"No", this->Z, elements[this->Z].name, this->A, this->color->name);
    }
    fprintf(stderr, "---------------------------------------------------\n");
}


