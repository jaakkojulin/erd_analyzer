#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <ctype.h>

#include "depthfile.h"

#define MAX_LINE_LEN 100

int count_lines(const char *filename) { /* Counts the number of non-comment lines. Opens and closes the file. */
    int lineno=0;
    int valid_data=0;
    FILE *in=fopen(filename, "r");
    if(!in) {
        return 0;
    }
    char *line = malloc(sizeof(char)*(MAX_LINE_LEN));
    while(fgets(line, MAX_LINE_LEN, in)) {
        lineno++;
        if(*line == '#') /* Comment */
            continue;
        valid_data++;
    }
    free(line);
    fclose(in);
    return valid_data;
}

depthfile_t *load_depthfile(rgbcolor_t *colors, char *filename, int Z, int A) {
    FILE *in;
    depthfile_t *depthfile;
    char *line;
    int linenumber=0;
    int n_depths=count_lines(filename);
    if(n_depths==0) {
        return NULL;
    }
    in=fopen(filename, "r");
    double depth, b, c, conc, e, f;
    int counts;
    int depth_i=0;
    if(!in) {
        fprintf(stderr, "No such file (%s)!\n", filename);
        return NULL;
    }
    depthfile=malloc(sizeof(depthfile_t));
    depthfile->filename=calloc(strlen(filename)+1, sizeof(char));
    strncat(depthfile->filename, filename, strlen(filename));
    depthfile->Z=Z;
    depthfile->A=A;
    depthfile->n_depths=n_depths;
    depthfile->use_in_scaling=1;
    depthfile->plot=1;
    depthfile->depths=calloc(n_depths, sizeof(double));
    depthfile->concentrations=calloc(n_depths, sizeof(double));
    depthfile->counts=calloc(n_depths, sizeof(int));
    depthfile->color=colors;
    
    line=calloc(MAX_LINE_LEN, sizeof(char));
    while(fgets(line, MAX_LINE_LEN, in)) {
        linenumber++;
        if(*line == '#')
            continue;
        if(sscanf(line, "%lf %lf %lf %lf %lf %lf %i\n", &depth, &b, &c, &conc, &e, &f, &counts)==7) {
            depthfile->depths[depth_i]=depth;
            depthfile->concentrations[depth_i]=conc;
            depthfile->counts[depth_i]=counts;
            depth_i++;
        }
        if(depth_i > n_depths) {
            fprintf(stderr, "Something odd at the depthfile, too many depths (was expecting %i)!\n", n_depths);
            break;
        }
    }
    fclose(in);
    free(line);
    return depthfile;
}


depthfile_t *join_depthfiles(depthfile_t *a, depthfile_t *b) { /* PROBLEM: filename? */
    int n_depths;
    int i;
    depthfile_t *result;
    if(a==NULL || b==NULL) {
        fprintf(stderr, "One of the depthfiles is nonexistent (NULL).\n");
        return NULL;
    }
    if(a==b) {
        fprintf(stderr, "Attempted to join a depthfile to the same depthfile. I don't get the point.\n");
        return NULL;
    }
    
    if(a->n_depths != b->n_depths) {
        fprintf(stderr, "Attempted to join two depthfiles with different depth bins. This is not accepted right now.\n");
        return NULL;
    }
    n_depths=a->n_depths;
    for(i=0; i<n_depths; i++) {
        if(a->depths[i] != b->depths[i]) {
            fprintf(stderr, "Attempted to join two depthfiles with different depth bins. This is not accepted right now.\n");
            return NULL;
        }
    }
    if(a-> Z != b-> Z) {
        fprintf(stderr, "Attempted to join two depthfile that have different Z. I don't understand you.\n");
        return NULL;
    }
    result = malloc(sizeof(depthfile_t));
    result->Z = a->Z;
    result->A = 0;
    result->n_depths = n_depths;
    result->use_in_scaling = 1;
    result->plot = 1;
    /*result->filename = calloc(strlen(filename)+1, sizeof(char));
     strncpy(result->filename, filename, strlen(filename)); */
    result->color = a->color; /* Inherit color */
    if(a->use_in_scaling && b->use_in_scaling) { /* Use the join file in scaling only if both original files would have been used */
        result->use_in_scaling=1;
    }
    a->use_in_scaling = 0;
    b->use_in_scaling = 0;
    if(a->plot && b->plot) { /* Same as for scaling */
        result->plot=1;
    }
    a->plot = 0;
    b->plot = 0;
    result->depths=calloc(n_depths, sizeof(double));
    result->concentrations=calloc(n_depths, sizeof(double));
    result->counts=calloc(n_depths, sizeof(int));
    for(i=0; i<n_depths; i++) {
        result->depths[i]=a->depths[i];
        result->concentrations[i]=a->concentrations[i]+b->concentrations[i];
        result->counts[i]=a->counts[i]+b->counts[i];
    }
    return result;
}

depthfile_t *previous_depthfile(depthfile_t *depthfiles, depthfile_t *a) {
    depthfile_t *this=depthfiles;
    while(this && this->next != a) {
        this=this->next;
    }
    if(this && this->next==a)
        return this;
    else
        return NULL;
}

depthfile_t *depthfile_swap(depthfile_t *depthfiles, depthfile_t *a, depthfile_t *b) {
    depthfile_t *a_previous, *b_previous, *temp;
    if(a==NULL || b==NULL || depthfiles==NULL)
        return depthfiles;
    a_previous=previous_depthfile(depthfiles, a);
    b_previous=previous_depthfile(depthfiles, b);
    if(a_previous)
        a_previous->next=b;
    if(b_previous)
        b_previous->next=a;
    temp=a->next;
    a->next=b->next;
    b->next=temp;
    if(depthfiles==a) {
        return b;
    }
    if(depthfiles==b) {
        return a;
    }
    return depthfiles;
}

depthfile_t *destroy_depthfile(depthfile_t *depthfiles, depthfile_t *depthfile) { /* Removes a depthfile from the list and frees memory. Can also be used with depthfiles==NULL */
    depthfile_t *this=depthfiles;
    depthfile_t *previous=NULL;
    depthfile_t *out=NULL;
    if(depthfile==NULL)
        return NULL;
    if(depthfile==depthfiles) { /* The file we seek is the first one. Return the pointer to the new first element. */
        out=depthfiles->next;
    }
    while(this && this != depthfile) { /* Find it */
        previous=this;
        this=this->next;
    }
    if(this==depthfile && previous) { /* Found it, and there is a previous element as well */
        previous->next=this->next; /* Removed an element between these */
        out=depthfiles;
    }
    if(depthfile->filename)
        free(depthfile->filename);
    if(depthfile->depths)
        free(depthfile->depths);
    if(depthfile->concentrations)
        free(depthfile->concentrations);
    if(depthfile->counts)
        free(depthfile->counts);
    free(depthfile); /* Freeing memory */
    return out;
}

depthfile_t *add_depthfile(depthfile_t *depthfiles, depthfile_t *depthfile) {
    depthfile_t *this=depthfiles;
    depthfile->next=NULL; /* Linked list. We try to add it to the end. So the pointer to next should always be null. Always. */
    if(depthfile==NULL) /* No file, no change */
        return depthfiles;
    if(depthfiles==NULL) {/* This is the first one */
        depthfile->uniq_id=1;
        return depthfile;
    }
    while(this->next != NULL) { /* Seeking the end */
        this=this->next;
    }
    this->next=depthfile;
    depthfile->uniq_id=this->uniq_id+1;
    return depthfiles;
}

void print_depthfiles(depthfile_t *depthfiles) {
    depthfile_t *this=depthfiles;
    while(this != NULL) {
        fprintf(stderr, "DEPTHFILE Z=%i A=%i, filename=%s\n", this->Z, this->A, this->filename);
        this=this->next;
    }
}

depthfile_t *find_depthfile_by_name(depthfile_t *depthfiles, element_t *elements, char *str) {
    int number=0, i, l_remaining, id, A, Z;
    depthfile_t *this=NULL;
    if(str==NULL)
        return NULL;
    if(*str == '\0')
        return NULL;
    for(;*str != '\0'; str++) { /* Parses an initial number (selection number, isotope A) */
        if(isdigit(*str)) {
            number *= 10;
            number += *str-'0';
        } else {
            break;
        }
    } 
    l_remaining=strlen(str);
    if(l_remaining == 0) {
        id=number;
        for(this=depthfiles; this != NULL; this=this->next) {
            if(this->uniq_id == id) {
                return this;
            }
        }
        return NULL;
    }
    
    if(*str=='-') {
        str++;
        l_remaining=strlen(str);
    }
    
    A=number;
    if(l_remaining) {
        for(i=0; i<ELEMENTS; i++) {
            if(strcmp(elements[i].name, str)==0) { /* no strncmp because of complications. Which name is longer? */
                Z=i;
                break;
            }
        }
        if(!Z)
            return NULL;
        
        for(this=depthfiles; this != NULL; this=this->next)
            if(this->Z==Z && this->A==A) {
                return this;
            }
        return NULL;
    } 
    return NULL;
}

depthfile_t *sort_depthfiles(depthfile_t *depthfiles) {
    depthfile_t *first,*smallest,*this;
    for(first=depthfiles; first != NULL; first=first->next) { /* Ye olde selection sort */
#ifdef DEBUG
        print_depthfile_chain(depthfiles);
#endif
        smallest=first;
        for(this=first; this != NULL; this=this->next) {
            if(this->Z < smallest->Z || (this->Z == smallest->Z && this->A < smallest->A)) {
                smallest=this;
            }
            if(first != smallest) {
                depthfiles=depthfile_swap(depthfiles, first, smallest);
                first=smallest;
            }
        }
    }
    return depthfiles;
}

void print_depthfile_chain(depthfile_t *depthfiles) {
    depthfile_t *this=depthfiles;
    while(this != NULL) {
        fprintf(stderr, "%i->", this->uniq_id);
        this=this->next;
    }
    fprintf(stderr, "NULL\n");
}

depthfile_t *load_depthfiles_by_prefix(element_t *elements, depthfile_t *depthfiles, rgbcolor_t *colors, char *prefix) {
    char *pattern = calloc(strlen(prefix)+2+1, sizeof(char));
    char *depthfile_name;
    glob_t globbuf;
    int prefix_length=strlen(prefix)+1; /* including the ".", e.g. prefix=="depth" => "depth." is 6 characters. */
    strncpy(pattern, prefix, strlen(prefix));
    strncat(pattern, ".*", 2);
    glob(pattern, 0, NULL, &globbuf);
    int Z,A,i;
    depthfile_t *depthfile;
    for(i=0; i<globbuf.gl_pathc; i++) {
        depthfile_name=globbuf.gl_pathv[i];
        A=find_A(depthfile_name+prefix_length);
        Z=find_Z(elements, depthfile_name+prefix_length);
        if(Z) {
            fprintf(stderr, "Loaded %s (Z=%i, A=%i)\n", depthfile_name, Z, A);
            depthfile=load_depthfile(colors, depthfile_name, Z, A);
            depthfiles=add_depthfile(depthfiles, depthfile);
        }
    }
    free(pattern);
    return depthfiles;
}

int number_of_depthfiles(depthfile_t *depthfiles) {
    depthfile_t *this;
    int i=0;
    for(this=depthfiles; this != NULL; this=this->next) {
        i++;
    }
    return i;
}

