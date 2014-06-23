#define VERSION 0.0.1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <depthprofile.h>
#include <depthfile.h>
#include <plot.h>

#define xstr(s) str(s)
#define str(s) #s

#ifndef DATADIR
#define DATADIR .
#endif

#define COLORFILE DATADIR/colors.txt
#define ELEMENTSFILE DATADIR/elements.txt
#define PLOTHEADERSFILE headers.txt
#define MAX_LINE_LEN 100

int main(int argc, char **argv) {
    rgbcolor_t *colors=NULL;
    element_t *elements=NULL;
    depthfile_t *depthfiles=NULL;
    depth_scale_t depthscale;
    plot_options_t plot_options;
    char *user_input=calloc(MAX_LINE_LEN, sizeof(char));
    rgbcolor_t black;
    black.name = "black";
    black.r=0;
    black.g=0;
    black.b=0;
    black.next=load_colors(xstr(COLORFILE), NULL);
    colors=&black; /* black is always there, others are loaded */
    if(!colors) {
        fprintf(stderr, "Could not load colors!\n");
        return -2;
    }
    elements=load_elements(xstr(ELEMENTSFILE));
    switch(argc) {
        case 2:
            depthfiles=load_depthfiles_by_prefix(elements, depthfiles, colors, argv[1]);
            print_depthfiles(depthfiles);
            break;
        case 1:
            break;
        default:
            fprintf(stderr, "ERD analyzer version %s.\nUsage: %s [prefix]\n", xstr(VERSION), argv[0]);
            return -1;
    }
    depthscale.low=-1000000.0;
    depthscale.high=1000000.0;
    depthscale.scale=1.0;
    plot_options.x_autoscale=1;
    plot_options.y_autoscale=1;
    plot_options.x_low=0.0;
    plot_options.y_low=0.0;
    plot_options.x_high=0.0;
    plot_options.y_high=0.0;
    double result;
    int counts;
    int  Z, A;
    depthfile_t *this;
    char *s=calloc(101, sizeof(char));
    char *s2=calloc(101, sizeof(char));
    while(strncmp(user_input, "exit", 4)!=0) {
        if(fgets(user_input, MAX_LINE_LEN, stdin)==NULL)
            break;
        if(sscanf(user_input, "integrate %100s %lf %lf", s, &depthscale.low, &depthscale.high)==3) {
            result=average_concentration(find_depthfile_by_name(depthfiles, elements, s), depthscale.low, depthscale.high, NULL, &counts);
            fprintf(stderr, "result: %g%%\n", result*100);
            fprintf(stderr, "counts: %i\n", counts);
            fprintf(stderr, "relative error (1/sqrt(counts-1)): %e\n", 1/sqrt(1.0*counts-1));
            fprintf(stderr, "error: %g%%\n", 100*result*1/sqrt(1.0*counts-1));
            continue;
        }
        if(sscanf(user_input, "xrange [%lf:%lf]", &plot_options.x_low, &plot_options.x_high)==2) {
            plot_options.x_autoscale=0;
        }
        if(sscanf(user_input, "yrange [%lf:%lf]", &plot_options.y_low, &plot_options.y_high)==2) {
            plot_options.y_autoscale=0;
        }
        if(strcmp(user_input, "autoscale x")==0) {
            plot_options.x_autoscale=1;
        }
        if(strcmp(user_input, "autoscale y")==0) {
            plot_options.y_autoscale=1;
        }
        if(strcmp(user_input, "autoscale xy")==0) {
            plot_options.x_autoscale=1;
            plot_options.y_autoscale=1;
        }




        if(sscanf(user_input, "set low %lf", &depthscale.low)==1){
            find_scaling_factor(depthfiles, &depthscale);
        }
        if(sscanf(user_input, "set high %lf", &depthscale.high)==1) {
            find_scaling_factor(depthfiles, &depthscale);
        }
        if(strncmp(user_input, "results", 7)==0) { 
            find_scaling_factor(depthfiles, &depthscale);
            make_results_table(depthfiles, elements, NULL, &depthscale);
            continue;
        }
        if(sscanf(user_input, "find %100s", s)==1) {
            for(this=depthfiles; this != NULL; this=find_depthfile_by_name(this->next, elements, s)) {
                fprintf(stderr, "Found %i %i %i %s\n", this->uniq_id, this->Z, this->A, this->filename);
            }
            continue;
        }
        if(sscanf(user_input, "previous %100s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                fprintf(stderr, "This is  %i %i %i %s\n", this->uniq_id, this->Z, this->A, this->filename);
            this=previous_depthfile(depthfiles, this);
            if(this)
                fprintf(stderr, "Found previous %i %i %i %s\n", this->uniq_id, this->Z, this->A, this->filename);
            continue;
        }
        if(sscanf(user_input, "swap %s %s", s, s2)==2) {
            depthfiles=depthfile_swap(depthfiles, find_depthfile_by_name(depthfiles, elements, s), find_depthfile_by_name(depthfiles, elements, s2));
            print_depthfile_chain(depthfiles);
        }

        if(sscanf(user_input, "remove %s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            depthfiles=destroy_depthfile(depthfiles, this);
            continue;
        }
        if(sscanf(user_input, "remove %s", s)==1) {
            for(this=depthfiles; this != NULL; this=find_depthfile_by_name(this->next, elements, s)) {
                depthfiles=destroy_depthfile(depthfiles, this);
            }
            continue;
        } 
        if(sscanf(user_input, "load %s %i %i", s, &Z, &A)==3) {
            this=load_depthfile(colors, s, Z, A);
            add_depthfile(depthfiles, this);
            continue;
        }
        if(sscanf(user_input, "load prefix %100s", s)==1) {
            depthfiles=load_depthfiles_by_prefix(elements, depthfiles, colors, s);
            continue;
        }
        if(strncmp(user_input, "clear", 5)==0) {
            while(depthfiles) {
                depthfiles=destroy_depthfile(depthfiles, depthfiles);
            }
            continue;
        }
        if(strncmp(user_input, "number", 6)==0) {
            fprintf(stderr, "%i\n", number_of_depthfiles(depthfiles));
            continue;
        }
        if(sscanf(user_input, "disable scaling %100s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                this->use_in_scaling=0;
            continue;
        }
        if(sscanf(user_input, "enable scaling %100s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                this->use_in_scaling=1;
            continue;
        }
        if(sscanf(user_input, "disable plot %100s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                this->plot=0;
            continue;
        }
        if(sscanf(user_input, "enable plot %100s", s)==1) {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                this->plot=1;
            continue;
        }
        if(sscanf(user_input, "color %100s %100s", s, s2)==2) {
            rgbcolor_t *color;
            color=find_color(colors, s2);
            for(this=find_depthfile_by_name(depthfiles, elements, s); this != NULL; this=find_depthfile_by_name(this->next, elements, s)) {
                this->color=color;
                if(color) {
                    fprintf(stderr, "id %i colored #%02x%02x%02x\n", this->uniq_id, color->r, color->g, color->b);
                }
            }
        }
        if(sscanf(user_input, "join %s %s", s, s2)==2) {
            this=join_depthfiles(find_depthfile_by_name(depthfiles, elements, s), find_depthfile_by_name(depthfiles, elements, s2));
            add_depthfile(depthfiles, this);
            continue;
        }
        if(strncmp(user_input, "sort", 4)==0) {
            depthfiles=sort_depthfiles(depthfiles);
            continue;
        }
        if(strncmp(user_input, "plotinfo", 8)==0) {
            show_plotinfo(depthfiles, elements);
            continue;
        }
        if(sscanf(user_input, "plot %s", s)==1) {
            find_scaling_factor(depthfiles, &depthscale);
            create_plotfile(depthfiles, elements, colors, &depthscale, &plot_options, xstr(PLOTHEADERSFILE), s);
            continue;
        }
    }
    return 0;
}


