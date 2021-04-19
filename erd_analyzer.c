/*
    Copyright (C) 2014-2020 Jaakko Julin <jaakko.julin@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    See file COPYING for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_STRING "erd_analyzer ?.?.?"
#endif
#include "depthprofile.h"
#include "depthfile.h"
#include "plot.h"
#include "csv.h"

#define xstr(s) str(s)
#define str(s) #s

#ifndef DATADIR
#define DATADIR .
#endif

#define COLORFILE DATADIR/colors.txt
#define ELEMENTSFILE DATADIR/elements.txt
#define PLOTHEADERSFILE headers.txt
#define MAX_LINE_LEN 100

#define HELP_TEXT "Usage: %s prefix\n\n"
#define  LICENCE_TEXT "This program is free software; you can redistribute it and/or modify\nit under the terms of the GNU Genera    l Public License as published by\nthe Free Software Foundation; either version 2 of the License, or\n(at your option) any later v    ersion.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied wa    rranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n"

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
            fprintf(stderr, "%s\n", PACKAGE_STRING);
            fprintf(stderr, HELP_TEXT, argv[0]);
            fprintf(stderr, LICENCE_TEXT);
            return 0;
    }
    depthscale.low=-1000000.0;
    depthscale.high=1000000.0;
    depthscale.scale=1.0;
    depthscale.density=1.0;
    plot_options.x_autoscale=1;
    plot_options.y_autoscale=1;
    plot_options.scalinglines=1;
    plot_options.linewidth=1.0;
    plot_options.x_low=0.0;
    plot_options.y_low=0.0;
    plot_options.x_high=0.0;
    plot_options.y_high=0.0;
    plot_options.nanometers=0;
    int Z, A;
    depthfile_t *this;
    char *s=calloc(101, sizeof(char));
    char *s2=calloc(101, sizeof(char));
    while(strncmp(user_input, "exit", 4)!=0) {
        if(fgets(user_input, MAX_LINE_LEN, stdin)==NULL)
            break;
        if(sscanf(user_input, "integrate %100s", s)==1) {
            integration_result_t r=integrate_depthfile(find_depthfile_by_name(depthfiles, elements, s), depthscale.low, depthscale.high);
            fprintf(stdout, "areal density: %g\n", r.adensity);
            fprintf(stdout, "result: %g%%\n", r.conc*100);
            fprintf(stdout, "counts: %i\n", r.counts);
            fprintf(stdout, "relative error (1/sqrt(counts-1)): %e\n", 1/sqrt(1.0*r.counts-1));
            fprintf(stdout, "error: %g%%\n", 100*r.conc*1/sqrt(1.0*r.counts-1));
            continue;
        }
        if(sscanf(user_input, "xrange [%lf:%lf]", &plot_options.x_low, &plot_options.x_high)==2) {
            plot_options.x_autoscale=0;
        }
        if(sscanf(user_input, "yrange [%lf:%lf]", &plot_options.y_low, &plot_options.y_high)==2) {
            plot_options.y_autoscale=0;
        }
        sscanf(user_input, "set linewidth %lf", &plot_options.linewidth);
        if(strncmp(user_input, "set scalinglines", 16)==0) {
            plot_options.scalinglines=1;
            continue;
        }
        if(strncmp(user_input, "unset scalinglines", 18)==0) {
            plot_options.scalinglines=0;
            fprintf(stderr, "Trying to do it...\n");
            continue;
        }
        
        if(strncmp(user_input, "autoscale x", 11)==0) {
            plot_options.x_autoscale=1;
            continue;
        }
        if(strncmp(user_input, "autoscale y", 11)==0) {
            plot_options.y_autoscale=1;
            continue;
        }
        if(strncmp(user_input, "autoscale xy", 12)==0) {
            plot_options.x_autoscale=1;
            plot_options.y_autoscale=1;
            continue;
        }

        if(sscanf(user_input, "set low %lf", &depthscale.low)==1){
            find_scaling_factor(depthfiles, &depthscale);
            fprintf(stderr, "Low set to %g, scaling factor now: %g\n", depthscale.low, depthscale.scale);
            continue;
        }
        if(sscanf(user_input, "set high %lf", &depthscale.high)==1) {
            find_scaling_factor(depthfiles, &depthscale);
            fprintf(stderr, "High set to %g, scaling factor now: %g\n", depthscale.high, depthscale.scale);
            continue;
        }
        if(sscanf(user_input, "set density %lf", &depthscale.density)==1) {
            fprintf(stderr, "Density set to %g g/cm3\n", depthscale.density);
            continue;
        }

        if(strncmp(user_input, "results", 7)==0) { 
            make_results_table(depthfiles, elements, NULL, &depthscale);
            continue;
        }
        if(sscanf(user_input, "find %100s", s)==1) {
            for(this=depthfiles; this != NULL; this=find_depthfile_by_name(this->next, elements, s)) {
                fprintf(stderr, "Found %i %i %i %s\n", this->uniq_id, this->Z, this->A, this->filename);
            }
            continue;
        }
        if(sscanf(user_input, "ratio %100s %100s", s, s2)==2) {
            element_ratio(find_depthfile_by_name(depthfiles, elements, s), find_depthfile_by_name(depthfiles, elements, s2), &depthscale);
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
        if(sscanf(user_input, "swap %100s %100s", s, s2)==2) {
            depthfiles=depthfile_swap(depthfiles, find_depthfile_by_name(depthfiles, elements, s), find_depthfile_by_name(depthfiles, elements, s2));
            print_depthfile_chain(depthfiles);
        }

       if(sscanf(user_input, "remove %100s", s)==1) {
            for(this=depthfiles; this != NULL; this=find_depthfile_by_name(this->next, elements, s)) {
                depthfiles=destroy_depthfile(depthfiles, this);
            }
            continue;
        } 
        if(sscanf(user_input, "load %i %i %100s", &Z, &A, s)==3) {
            this=load_depthfile(colors, s, Z, A);
            add_depthfile(depthfiles, this);
            continue;
        }
        if(sscanf(user_input, "load prefix %100s", s)==1) {
            fprintf(stderr, "Attempting to read depth files with prefix %s\n", s);
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
            if(strncmp(s, "all", 3) == 0) {
                for(this=depthfiles; this != NULL; this=this->next) {
                    this->use_in_scaling=0;
                }
            } else {
            this=find_depthfile_by_name(depthfiles, elements, s);
            if(this)
                this->use_in_scaling=0;
            }
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
            if(!this) {
                fprintf(stderr, "Could not join.\n");
            } else {
                add_depthfile(depthfiles, this);
            }
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
            fprintf(stderr, "Creating plot with scaling factor: %g\n", depthscale.scale);
            create_plotfile(depthfiles, elements, colors, &depthscale, &plot_options, xstr(PLOTHEADERSFILE), s);
            continue;
        }
        if(sscanf(user_input, "csv %s", s)==1) {
            fprintf(stderr, "Creating CSV output with scaling factor: %g\n", depthscale.scale);
            create_csv(depthfiles, elements, &depthscale, s);
        }
        if(strncmp(user_input, "scale", 5)==0) {
            find_scaling_factor(depthfiles, &depthscale);
            continue;
        }
        if(sscanf(user_input, "set scale %lf", &depthscale.scale)==1) {
            fprintf(stderr, "Manual scaling factor now: %g\n", depthscale.scale);
            continue;
        }
        if(strncmp(user_input, "set nanometers",14) == 0) {
            fprintf(stderr, "Output X-axis in nm.\n");
            plot_options.nanometers = 1;
            continue;
        }
        if(strncmp(user_input, "unset nanometers",16) == 0) {
            plot_options.nanometers = 0;
            continue;
        }
    }
    return 0;
}


