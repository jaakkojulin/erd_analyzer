#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "elements.h"


#define MAX_LINE_LEN 100

element_t *load_elements(char *filename) {
    FILE *in;
    int Z;
    element_t *elements;
    char *line;
    char *name;
    int linenumber=0, ok_elements=0;
    in=fopen(filename, "r");
    if(!in) {
        fprintf(stderr, "No such file (%s)!\n", filename);
        return NULL;
    }
    elements=malloc(sizeof(element_t)*ELEMENTS);
    line=calloc(MAX_LINE_LEN, sizeof(char));
    name=calloc(4, sizeof(char));
    while(fgets(line, MAX_LINE_LEN, in)) {
        linenumber++;
        if(*line == '#') /* Comment */
            continue;
        if(sscanf(line, "%i %3s\n", &Z, name)==2) {
            ok_elements++;
            if(Z>0 && Z<ELEMENTS) {
                elements[Z].Z=Z;
                strncpy(elements[Z].name, name, 4);
            }
        }
    }
    fclose(in);
    return elements;
}

int find_A(char *element_name) {
    int A=0;
    while(isdigit(*element_name)) { /* Element name may be prefixed with an isotope number */
        A=A*10;
        A += *element_name-'0';
        element_name++;
    }
    return A; 
}

int find_Z(element_t *elements, char *element_name) {
    int i;
    while(isdigit(*element_name)) { /* Element name may be prefixed with some number (isotope), skip that */
        element_name++;
    }
    for(i=0; i<ELEMENTS; i++) {
        if(strncmp(elements[i].name, element_name, 3)==0) {
            return i;
        }
    }
    return 0;
}
