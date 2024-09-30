#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define BUFSIZE 512
#define MAXWORD 100  
#define MAX_VARIABLES 20
#define MAX_VALUE_LENGTH 256
#define MAX_RECURSION_DEPTH 10

struct Variable {
    char var_name[MAXWORD];
    char value[MAX_VALUE_LENGTH];
};

int numberOfVariables;
struct Variable variables[MAX_VARIABLES];

void my_strncpy(char *dest, const char *src, int n);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, int n);
void my_strcat(char *dest, const char *src);
char *my_strstr(const char *haystack, const char *needle); 
void parse_arguments(int argc, char *argv[]);
void replace_variables(char *line);
void process_define(char *line);
int is_delimiter(char c);
int isalpha(char c);
void safe_strcat(char *dest, const char *src);
void replace_var(char *line, const char *var, const char *value);

void my_strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int my_strncmp(const char *s1, const char *s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        --n;
    }
    return (n == 0) ? 0 : (*(unsigned char *)s1 - *(unsigned char *)s2);
}

void my_strcat(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

char *my_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) return (char *)haystack;

        haystack++;
    }

    return 0;
}

int isalpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

void safe_strcat(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}
void parse_arguments(int argc, char *argv[]) {
    for (int i = 2; i < argc; i++) { 
        if (my_strncmp(argv[i], "-D", 2) == 0) {
            char *equal_sign = my_strstr(argv[i], "=");
            if (equal_sign) {
                
                *equal_sign = '\0'; 
                if (numberOfVariables < MAX_VARIABLES) {
                    my_strncpy(variables[numberOfVariables].var_name, argv[i] + 2, MAXWORD);
                    my_strncpy(variables[numberOfVariables].value, equal_sign + 1, MAX_VALUE_LENGTH);
                    numberOfVariables++;
                } else {
                    printf(2, "Error: Maximum number of variables reached\n");
                }
                *equal_sign = '='; 
            }
        }
    }
}


void replace_var(char *line, const char *var, const char *value) {
    char buffer[BUFSIZE];
    char *pos;

    while ((pos = my_strstr(line, var)) != 0) {
        char before = (pos == line) ? '\0' : *(pos - 1);
        char after = *(pos + strlen(var));

        if (!isalpha(before) && !isalpha(after)) {
            *pos = '\0';  
            buffer[0] = '\0';  
            safe_strcat(buffer, line);  
            safe_strcat(buffer, value); 
            safe_strcat(buffer, pos + strlen(var)); 
            my_strncpy(line, buffer, BUFSIZE);  
        } else {
            line = pos + strlen(var);
        }
    }
}

void replace_variables(char *line) {
    for (int i = 0; i < numberOfVariables; i++) {
        replace_var(line, variables[i].var_name, variables[i].value);
    }
}

void process_define(char *line) {
    char var_name[MAXWORD];
    char value[MAX_VALUE_LENGTH];
    int i = 7; 
    int j = 0;

    while (line[i] == ' ' || line[i] == '\t') i++;
    while (line[i] != ' ' && line[i] != '\t' && line[i] != '\0') {
        var_name[j++] = line[i++];
    }
    var_name[j] = '\0';

    while (line[i] == ' ' || line[i] == '\t') i++;
    j = 0;
    while (line[i] != '\0' && line[i] != '\n') {
        value[j++] = line[i++];
    }
    value[j] = '\0';

    if (numberOfVariables < MAX_VARIABLES) {
        my_strncpy(variables[numberOfVariables].var_name, var_name, MAXWORD);
        my_strncpy(variables[numberOfVariables].value, value, MAX_VALUE_LENGTH);
        numberOfVariables++;
    } else {
        printf(2, "Error: Maximum number of variables reached\n");
    }
}

int is_delimiter(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == ';' || 
            c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']');
}

int main(int argc, char *argv[]) {
    int fd;
    char buf[BUFSIZE];
    char line[BUFSIZE];
    int n, i;

    if (argc < 2) {
        printf(2, "Usage: preprocess filename.txt [-D<var>=<val> ...]\n");
        exit();
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf(2, "preprocess: cannot open %s\n", argv[1]);
        exit();
    }

    numberOfVariables = 0;
    parse_arguments(argc, argv);

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (i = 0; i < n; i++) {
            if (my_strncmp(buf + i, "#define", 7) == 0) {
                char line[BUFSIZE];
                int k = 0;
                while (i < n && buf[i] != '\n') {
                    line[k++] = buf[i++];
                }
                line[k] = '\0';
                process_define(line);
                printf(1, "%s\n", line);  
            } else {
                int j = 0;
                while (i < n && !is_delimiter(buf[i])) {
                    line[j++] = buf[i++];
                }
                line[j] = '\0'; 
                replace_variables(line);
                printf(1, "%s", line);
                if (i < n) {
                    printf(1, "%c", buf[i]); 
                }
            }
        }
    }

    if (n < 0) {
        printf(2, "preprocess: read error\n");
    }

    close(fd);
    exit();
}
