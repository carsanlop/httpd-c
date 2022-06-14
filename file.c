#include <stdio.h>
#include <sys/stat.h>

/**
 * content should be freed
 * @param content The content from the file (if any) will be placed in here. This will be dynamically allocated and should be freed by the caller function.
 */
void read_resource(char *resource, char **content)
{
    // Retrieve a file pointer by converting the resource to an actual filesystem address.
    char *path;
    asprintf(&path, "%s%s", ".", resource);
    FILE *file = fopen(path, "r");

    if (file != NULL)
    {
        int file_fd = fileno(file);
        struct stat stats;
        fstat(file_fd, &stats);

        if (S_ISREG(stats.st_mode) == 1)
        {
            *content = malloc(stats.st_size + 1); // +1 for final \n
            read(file_fd, *content, stats.st_size);
        }

        fclose(file);
    }

    free(path);
}