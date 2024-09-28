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

void my_strncpy(char *dest, const char *src, int n) {
    if (n == 0) return;  // No characters to copy

    int i;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // Always null-terminate the destination
    dest[i] = '\0';
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void my_strcat(char *dest, const char *src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

int my_strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void parse_arguments(int argc, char *argv[]) {
    int i = 2;  // Start from the third argument (after program name and filename)
    while (i < argc && numberOfVariables < MAX_VARIABLES) {
        if (argv[i][0] == '-' && argv[i][1] == 'D') {
            char *var_start = argv[i] + 2;
            char *equals_pos = var_start;
            while (*equals_pos != '=' && *equals_pos != '\0') {
                equals_pos++;
            }
            
            if (*equals_pos == '=') {
                int name_length = equals_pos - var_start;
                my_strncpy(variables[numberOfVariables].var_name, var_start, name_length + 1);
                variables[numberOfVariables].var_name[name_length] = '\0';
                
                // Collect the entire value, including spaces
                char *value_start = equals_pos + 1;
                int value_length = my_strlen(value_start);
                my_strncpy(variables[numberOfVariables].value, value_start, value_length + 1);
                
                // Continue collecting value from next arguments if they don't start with '-'
                i++;
                while (i < argc && argv[i][0] != '-') {
                    my_strcat(variables[numberOfVariables].value, " ");
                    my_strcat(variables[numberOfVariables].value, argv[i]);
                    i++;
                }
                i--; // Adjust i because the outer loop will increment it
                
                numberOfVariables++;
            } else {
                printf(2, "Invalid argument format: %s\n", argv[i]);
            }
        } else {
            printf(2, "Unexpected argument: %s\n", argv[i]);
        }
        i++;
    }
}

int is_special_character(char c) {
    const char special_chars[] = "!@#$%^&*()_+[]{}|;:',.<>/?`~\"\\-=";
    int i;
    for (i = 0; special_chars[i] != '\0'; i++) {
        if (c == special_chars[i]) {
            return 1;
        }
    }
    return 0;
}

void replace_variables(char *word, char *result, int depth) {
    if (depth > MAX_RECURSION_DEPTH) {
        printf(2, "Error: Maximum recursion depth reached. Possible circular reference.\n");
        my_strncpy(result, word, MAX_VALUE_LENGTH);
        return;
    }

    int length = my_strlen(word);
    char modified_word[MAXWORD];
    my_strncpy(modified_word, word, MAXWORD);
    
    int has_special = (length > 0 && is_special_character(word[length - 1]));

    if (has_special) {
        modified_word[length - 1] = '\0';
    }

    int replaced = 0;

    for (int i = 0; i < numberOfVariables; i++) {
        if (my_strcmp(word, variables[i].var_name) == 0) {
            replace_variables(variables[i].value, result, depth + 1);
            replaced = 1;
            break;
        }
    }

    if (!replaced) {
        for (int i = 0; i < numberOfVariables; i++) {
            if (my_strcmp(modified_word, variables[i].var_name) == 0) {
                replace_variables(variables[i].value, result, depth + 1);
                if (has_special) {
                    my_strcat(result, &word[length - 1]);
                }
                replaced = 1;
                break;
            }
        }
    }

    if (!replaced) {
        my_strncpy(result, word, MAX_VALUE_LENGTH);
    }
}


void process_word(char *word) {
    char result[MAX_VALUE_LENGTH];
    replace_variables(word, result, 0);
    printf(1, "%s", result);
}

int main(int argc, char *argv[]) {
    int fd;
    char buf[BUFSIZE];
    char word[MAXWORD];
    int n, i, j = 0;

    if (argc < 2) {
        printf(2, "Usage: define-demo filename.txt [-D<var>=<val> ...]\n");
        exit();
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf(2, "define-demo: cannot open %s\n", argv[1]);
        exit();
    }

    numberOfVariables = 0;
    parse_arguments(argc, argv);

    for (i = 0; i < numberOfVariables; i++) {
    }

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (i = 0; i < n; i++) {
            if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t') {
                if (j > 0) {
                    word[j] = '\0';
                    process_word(word);
                    j = 0;
                }
                printf(1, "%c", buf[i]);
            } else if (j < MAXWORD - 1) {
                word[j++] = buf[i];
            }
        }
    }

    if (j > 0) {
        word[j] = '\0';
        process_word(word);
    }

    if (n < 0) {
        printf(2, "define-demo: read error\n");
    }

    close(fd);
    exit();
}