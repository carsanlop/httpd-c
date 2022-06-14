#ifndef FILEC_H
#define FILEC_H

#include <stdio.h>
#include <sys/stat.h>

#include "file.h"

/**
 * content should be freed
 * @param content The content from the file (if any) will be placed in here. This will be dynamically allocated and should be freed by the caller function.
 */
void read_resource(char *resource, char **content);

#endif