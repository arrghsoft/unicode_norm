#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fts.h>
#include <getopt.h>

#include "utf8proc.h"
#include "version.h"

#define STR_BUFFER 2048

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

enum LogLevel {
    ERROR,
    WARNING,
    INFO,
    VERBOSE,
    DEBUG
};

static const char *log_level_names[] = {
    "ERROR",
    "WARNING",
    "INFO",
    "VERBOSE",
    "DEBUG"
};

static const struct option long_opts[] = {
    { "help", 0, NULL, 'h'},
    { "recursive", 0, NULL, 'r'},
    { "verbose", 0, NULL, 'v'},
    { "form", 1, NULL, 'f'},
};

int form = NFC;
int global_log_level = INFO;
bool is_recursive = false;

void print_help();
void print_log(char *str, enum LogLevel level);
void rename_file(const char *file_path);
void parse_form();

int main(const int argc, char *argv[]) {
    char string[STR_BUFFER];
    if (argc <= 1) {
        print_help();
        exit(0);
    }

    int c;
    while ((c = getopt_long(argc, argv, "hrvf:", long_opts, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'r':
                is_recursive = true;
                break;
            case 'v':
                global_log_level = VERBOSE;
                break;
            case 'f':
                parse_form();
                break;
            default:
                print_log("Error: unknown option.", ERROR);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        char *file_path = argv[i];
        sprintf(string, "File path: %s", file_path);
        print_log(string, DEBUG);

        FILE *f = fopen(file_path, "r");
        if (f == NULL) {
            print_log("Error: could not open file.", ERROR);
            exit(EXIT_FAILURE);
        }

        if (!is_recursive) {
            rename_file(file_path);
        } else {
            FTS *file_hierarchy = NULL;
            FTSENT *node = NULL;

            file_hierarchy = fts_open(argv + argc - 1, FTS_NOCHDIR | FTS_COMFOLLOW, 0);
            if (!file_hierarchy) {
                perror("fts_open");
                exit(EXIT_FAILURE);
            }

            while ((node = fts_read(file_hierarchy))) {
                if (node->fts_info == FTS_D) {
                    continue;
                }
                sprintf(string, "File entry: %s (%s)", node->fts_path, S_ISDIR(node->fts_statp->st_mode) ? "dir" : "file");
                print_log(string, DEBUG);
                rename_file(node->fts_path);
            }

            fts_close(file_hierarchy);
        }
    }

    return 0;
}

void print_help() {
    printf(
        "unicode_norm - Convert file name from some unicode normalization form to another form (NFC, NFD, NFKC, NFKD)\n"
        "Version: %s (Compiled on %s at %s)"
        "\n\n"
        "Usage: unicode_norm [-options] filepath\n"
        "-r, --recursive    Convert the directory recursively\n"
        "-v, --verbose      Verbose mode\n"
        "-h, --help         Display the help information\n"
        "-f, --form         (defaults to \"NFC\")\n", APP_VERSION, __DATE__, __TIME__);
}

void print_log(char *str, enum LogLevel level) {
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

void rename_file(const char *file_path) {
    char string[STR_BUFFER];

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
            print_log("Error: invalid form.", ERROR);
            exit(EXIT_FAILURE);
    }
    rename(abs_path, new_path);
    sprintf(string, "Successfully renamed %s to %s form", file_path, form_names[form]);
    print_log(string, VERBOSE);
    free(new_path);
}

void parse_form() {
    if (strcmp(optarg, "NFC") == 0) {
        form = NFC;
    } else if (strcmp(optarg, "NFD") == 0) {
        form = NFD;
    } else if (strcmp(optarg, "NFKC") == 0) {
        form = NFKC;
    } else if (strcmp(optarg, "NFKD") == 0) {
        form = NFKD;
    } else {
        print_log("Error: invalid normalization form. "
                  "Form(--form, -f) should be one of 'NFC', 'NFD', 'NFKC', or 'NFKD'.", ERROR);
        exit(EXIT_FAILURE);
    }
}
