#ifndef _ELEMENTS_H_
#define _ELEMENTS_H_

#define ELEMENTS 116 /* 115+1 */

typedef struct element {
    int Z;
    char name[4];
} element_t;

element_t *load_elements(char *filename);
int find_A(char *element_name);
int find_Z(element_t *elements, char *element_name);

#endif