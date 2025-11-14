#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include "utf8proc.h"

#define STR_BUFFER 2048

enum LogLevel {
    ERROR,
    WARNING,
    INFO,
    VERBOSE,
    DEBUG,
};


enum NormalizationForm {
    NFC,
    NFD,
    NFKC,
    NFKD
};

static const char *form_names[] = {
    "NFC",
    "NFD",
    "NFKC",
    "NFKD"
};

int form = NFC;
int global_log_level = INFO;

void LOG(char *str, enum LogLevel level);

void print_help();

int main(const int argc, char *argv[]) {
    char string[STR_BUFFER];

    if (argc <= 1) {
        print_help();
        exit(0);
    }

    int c;
    while ((c = getopt(argc, argv, "hrvf:")) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'r':
                LOG("Recursive mode not yet supported!", ERROR);
                exit(EXIT_FAILURE);
            case 'v':
                global_log_level = VERBOSE;
                break;
            case 'f':
                if (strcmp(optarg, "NFC") == 0) {
                    form = NFC;
                } else if (strcmp(optarg, "NFD") == 0) {
                    form = NFD;
                } else if (strcmp(optarg, "NFKC") == 0) {
                    form = NFKC;
                } else if (strcmp(optarg, "NFKD") == 0) {
                    form = NFKD;
                } else {
                    LOG("Error: invalid normalization form. "
                        "Form(--form, -f) should be one of 'NFC', 'NFD', 'NFKC', or 'NFKD'.", ERROR);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                LOG("Error: unknown option.", ERROR);
                exit(EXIT_FAILURE);
        }
    }
    char *file_path = argv[argc - 1];
    sprintf(string, "File path: %s", file_path);
    LOG(string, VERBOSE);

    FILE *f = fopen(file_path, "r");
    if (f == NULL) {
        LOG("Error: could not open file.", ERROR);
        exit(EXIT_FAILURE);
    } else {
    }

    char abs_path[PATH_MAX];
    if (realpath(file_path, abs_path) == NULL) {
        perror("realpath");
    }

    char *new_path;
    switch (form) {
        case NFC:
            new_path = (char *) utf8proc_NFC((uint8_t *) abs_path);
            break;
        case NFD:
            new_path = (char *) utf8proc_NFD((uint8_t *) abs_path);
            break;
        case NFKC:
            new_path = (char *) utf8proc_NFKC((uint8_t *) abs_path);
            break;
        case NFKD:
            new_path = (char *) utf8proc_NFKD((uint8_t *) abs_path);
            break;
        default:
            LOG("Error: invalid form.", ERROR);
            exit(EXIT_FAILURE);
    }
    rename(abs_path, new_path);

    sprintf(string, "Successfully renamed to %s form", form_names[form]);
    LOG(string, INFO);

    free(new_path);

    return 0;
}

void print_help() {
    printf(
        "unicode_norm - Convert file name from some unicode normalization form to another form (NFC, NFD, NFKC, NFKD)\n\n"
        "Usage: unicode_norm [-options] filepath\n"
        "-r, --recursive    Convert the directory recursively\n"
        "-v, --verbose      Verbose mode\n"
        "-h, --help         Display the help information\n"
        "-f, --form         (defaults to \"NFC\")\n");
}

void LOG(char *str, enum LogLevel level) {
    if (level > global_log_level) {
        return;
    }

    switch (level) {
        case ERROR:
            printf("\x1B[31m%s\x1B[0m\n", str);
            break;
        case WARNING:
            printf("\x1B[33m%s\x1B[0m\n", str);
            break;
        case INFO:
            printf("%s\n", str);
            break;
        case VERBOSE:
            printf("\x1B[35m%s\x1B[0m\n", str);
            break;
        case DEBUG:
            printf("\x1B[32m%s\x1B[0m\n", str);
            break;
    }
}
