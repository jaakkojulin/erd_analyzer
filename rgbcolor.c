#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rgbcolor.h"

#define MAX_LINE_LEN 100

rgbcolor_t *load_colors(char *filename, rgbcolor_t *first) {
    FILE *in;
    int r,g,b;
    char *name=calloc(31, sizeof(char));
    char *line=calloc(MAX_LINE_LEN, sizeof(char));
    int linenumber=0, ok_colors=0;
    rgbcolor_t *this=NULL, *previous=NULL;
    in=fopen(filename, "r");
    if(!in) {
        fprintf(stderr, "No such file (%s)!\n", filename);
        free(name);
        free(line);
        return first;
    }
    this=first;
    while(this != NULL) { /* Seek the end of the list */
        previous=this;
        this=this->next;
    }
    while(fgets(line, MAX_LINE_LEN, in)) {
        linenumber++;
        if(*line == '#') /* Comment */
            continue;
        if(sscanf(line, "%i %i %i %30s\n", &r, &g, &b, name)==4) {
            ok_colors++;
            this=malloc(sizeof(rgbcolor_t));
            this->name=malloc(sizeof(char)*31);
            strncpy(this->name, name, 31);
            this->r=r;
            this->g=g;
            this->b=b;
            this->a=255;
            this->next=NULL;
            if(previous) 
                previous->next=this;
            else /* Previous element does not exists, so this must be the first element */
                first=this;
            previous=this;
        }
    }
    fclose(in);
    fprintf(stderr, "Read %i lines from %s, got %i colors successfully.\n", linenumber, filename, ok_colors);
    free(name);
    free(line);
    return first;
}

void print_color(rgbcolor_t *color) {
    if(color) {
        fprintf(stderr, "%s: R: %i G: %i B: %i\n", color->name, color->r, color->g, color->b);
    } else {
        fprintf(stderr, "This is not a color.\n");
    }
}

void print_colors(rgbcolor_t *colors) {
    rgbcolor_t *color=colors;
    while(color) {
        print_color(color);
        color=color->next;
    }
}

rgbcolor_t *find_color(rgbcolor_t *colors, char *name) {
    rgbcolor_t *color=colors;
    while(color) {
        if(strncmp(color->name, name, 31)==0)
            return color;
        color=color->next;
    }
    return NULL;
}
