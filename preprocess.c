#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUFSIZE 1024

char *my_strncpy(char *dest, const char *src, int n) {
    char *d = dest;
    while (n-- && (*d++ = *src++));
    return dest;
}

int my_strncmp(const char *s1, const char *s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    return n == 0 ? 0 : *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *my_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    for (const char *p = haystack; *p; p++) {
        if ((*p == *needle) && !my_strncmp(p, needle, strlen(needle)))
            return (char*)p;
    }
    return 0;
}

void safe_strcat(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

void replace_var(char *line, const char *var, const char *value) {
    char buffer[BUFSIZE];
    char *pos;

    while ((pos = my_strstr(line, var)) != 0) {
        *pos = '\0';  // End the string at the variable
        buffer[0] = '\0';  // Initialize buffer
        safe_strcat(buffer, line);  // Copy the first part of the line
        safe_strcat(buffer, value); // Append the variable's value
        safe_strcat(buffer, pos + strlen(var)); // Append the rest of the line after the variable
        my_strncpy(line, buffer, BUFSIZE);  // Copy the modified string back to line
    }
}

int main(int argc, char *argv[]) {
    printf(1, "Preprocess program started\n");
    if (argc < 3) {
        printf(1, "Usage: preprocess <input_file> -Dvar1=val1 -Dvar2=val2 ...\n");
        exit();
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf(1, "Error: Cannot open file %s\n", argv[1]);
        exit();
    }

    char line[BUFSIZE];
    int n;

    // Read the input file line by line
    while ((n = read(fd, line, sizeof(line) - 1)) > 0) {
        line[n] = '\0';  // Null-terminate the string

        for (int i = 2; i < argc; i++) {
            if (my_strncmp(argv[i], "-D", 2) == 0) {
                char *equal_sign = strchr(argv[i], '=');
                if (equal_sign) {
                    char var[128], value[128];
                    my_strncpy(var, argv[i] + 2, equal_sign - (argv[i] + 2));
                    var[equal_sign - (argv[i] + 2)] = '\0';
                    my_strncpy(value, equal_sign + 1, 128);
                    replace_var(line, var, value);
                }
            }
        }
        printf(1, "%s", line);  // Print the processed line to stdout
    }

    close(fd);
    exit();
}
