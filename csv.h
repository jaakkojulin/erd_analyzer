#ifndef _CSV_H_
#define _CSV_H_

#include "elements.h"
#include "depthfile.h"
#include "depthprofile.h"

void create_csv(depthfile_t *depthfiles, element_t *elements, depth_scale_t *depthscale, char *filename);

#endif
