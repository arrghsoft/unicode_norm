#include <stdio.h>
#include <string.h>

#ifndef _WIN32
    #include <unistd.h>
    #include <libgen.h>
    #include <sys/stat.h>
    #include <fts.h>
#else
    #include <windows.h>
    #include <direct.h>
    #include <sys/stat.h>
#endif

#include <getopt.h>
#include <utf8proc.h>
#include "version.h"

#define STR_BUFFER 2048
#define LOG_LEVEL LOG_INFO
#define PATH_SEP_STR "/"

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
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_VERBOSE,
    LOG_DEBUG
};

static const struct option long_opts[] = {
    { "help", 0, NULL, 'h'},
    { "recursive", 0, NULL, 'r'},
    { "verbose", 0, NULL, 'v'},
    { "form", 1, NULL, 'f'},
    { "check", 0, NULL, 'c'},
    { "dry-run", 0, NULL, 'd'},
};

int form = NFC;
int global_log_level = LOG_LEVEL;
bool is_recursive = false;
bool is_dry_run = false;

void print_help();
void print_log(char *str, enum LogLevel level);
bool rename_file(const char *file_path);
void parse_form();
void convert_to_form(char *path, enum NormalizationForm form, char *buffer);

int main(const int argc, char *argv[]) {
    char string[STR_BUFFER];
    if (argc <= 1) {
        print_help();
        exit(0);
    }

    int c;
    while ((c = getopt_long(argc, argv, "hrvf:cd", long_opts, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0;
            case 'r':
                is_recursive = true;
                break;
            case 'v':
                global_log_level = LOG_VERBOSE;
                break;
            case 'f':
                parse_form();
                break;
            case 'd':
            case 'c':
                is_dry_run = true;
                break;
            default:
                print_log("Error: unknown option.", LOG_ERROR);
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        char *file_path = argv[i];
        sprintf(string, "File path: %s", file_path);
        print_log(string, LOG_DEBUG);

        FILE *f = fopen(file_path, "r");
        if (f == NULL) {
            print_log("Error: could not open file.", LOG_ERROR);
            exit(EXIT_FAILURE);
        }

        if (!is_recursive) {
            rename_file(file_path);
        } else {
            FTS *file_hierarchy = NULL;
            FTSENT *node = NULL;
            int total_cnt = 0;
            int updated_cnt = 0;

            file_hierarchy = fts_open(argv + optind, FTS_NOCHDIR | FTS_COMFOLLOW, 0);
            if (!file_hierarchy) {
                perror("fts_open");
                exit(EXIT_FAILURE);
            }

            while ((node = fts_read(file_hierarchy))) {
                if (node->fts_info == FTS_D) {
                    continue;
                }
                sprintf(string, "File entry: %s (%s)", node->fts_path, S_ISDIR(node->fts_statp->st_mode) ? "dir" : "file");
                print_log(string, LOG_DEBUG);
                bool updated = rename_file(node->fts_path);
                updated_cnt += updated ? 1 : 0;
                total_cnt++;
            }

            fts_close(file_hierarchy);
            if (is_dry_run) {
                sprintf(string, "Total files: %d, Files that would be updated: %d", total_cnt, updated_cnt);
                print_log(string, LOG_INFO);
            } else {
                sprintf(string, "Total files: %d, Updated files: %d", total_cnt, updated_cnt);
                print_log(string, LOG_INFO);
            }
            break;
        }
    }

    return 0;
}

void print_help() {
    printf(
        "unicode_norm - Normalize file names to a specified Unicode normalization form (NFC, NFD, NFKC, NFKD)\n"
        "Version: %s (Compiled on %s at %s)"
        "\n\n"
        "Usage: unicode_norm [-options] FILE...\n"
        "-f, --form         (defaults to \"NFC\")\n"
        "-r, --recursive    Convert the directory recursively\n"
        "-d, --dry-run      Show the changes that would be made, but do not modify files.\n"
        "-c, --check        Synonym of dry-run\n"
        "-v, --verbose      Verbose mode\n"
        "-h, --help         Display the help information\n",
        APP_VERSION, __DATE__, __TIME__);
}

void print_log(char *str, enum LogLevel level) {
    if (level > global_log_level) {
        return;
    }

    switch (level) {
        case LOG_ERROR:
            fprintf(stderr, "%s\n", str);
            break;
        case LOG_WARNING:
            fprintf(stderr, "%s\n", str);
            break;
        case LOG_INFO:
            printf("%s\n", str);
            break;
        case LOG_VERBOSE:
            printf("%s\n", str);
            break;
        case LOG_DEBUG:
            printf("%s\n", str);
            break;
    }
}

void convert_to_form(char *path, enum NormalizationForm form, char *buffer) {
    char* new_basename;

    char* dir_part = dirname(path);
    char* name_part = basename(path);
    switch (form) {
        case NFC:
            new_basename = (char *) utf8proc_NFC((uint8_t *) name_part);
            break;
        case NFD:
            new_basename = (char *) utf8proc_NFD((uint8_t *) name_part);
            break;
        case NFKC:
            new_basename = (char *) utf8proc_NFKC((uint8_t *) name_part);
            break;
        case NFKD:
            new_basename = (char *) utf8proc_NFKD((uint8_t *) name_part);
            break;
        default:
            print_log("Error: invalid form.", LOG_ERROR);
            exit(EXIT_FAILURE);
    }
    buffer[0] = '\0';
    strcat(buffer, dir_part);
    strcat(buffer, PATH_SEP_STR);
    strcat(buffer, new_basename);

    free(new_basename);
}

bool rename_file(const char *file_path) {
    char string[STR_BUFFER];
    char path_buffer[STR_BUFFER];

    char abs_path[PATH_MAX];
    if (realpath(file_path, abs_path) == NULL) {
        perror("realpath");
        return false;
    }

    char form_string[50];
    form_string[0] = '\0';
    if (is_dry_run) {
        bool is_first = true;
        for (int i = NFC; i < NFKD + 1; i++) {
            convert_to_form(abs_path, i, path_buffer);
            if (strcmp(abs_path, path_buffer) == 0) {
                if (!is_first) {
                    strcat(form_string, ", ");
                } else {
                    is_first = false;
                }
                strcat(form_string, form_names[i]);
            }
        }
        if (is_first) {
            strcpy(form_string, "UNKNOWN");
        }
        sprintf(string, "File is in form of %s: %s", form_string, file_path);
        print_log(string, LOG_DEBUG);
    }
    convert_to_form(abs_path, form, path_buffer);

    if (is_dry_run) {
        if (strcmp(path_buffer, abs_path) == 0) {
            sprintf(string, "[SKIP] file name already in %s form: %s", form_names[form], file_path);
            print_log(string, LOG_INFO);
            return false;
        }
        sprintf(string, "[TARGET] This file will be renamed from %s form to %s form: %s", form_string, form_names[form], file_path);
        print_log(string, LOG_INFO);
        return true;
    }

    if (strcmp(path_buffer, abs_path) == 0) {
        sprintf(string, "[SKIP] file name already in %s form: %s", form_names[form], file_path);
        print_log(string, LOG_INFO);
        return false;
    }
    rename(abs_path, path_buffer);
    sprintf(string, "[SUCCESS] Successfully renamed from %s form to %s form: %s", form_string, form_names[form], file_path);
    print_log(string, LOG_INFO);
    return true;
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
                  "Form(--form, -f) should be one of 'NFC', 'NFD', 'NFKC', or 'NFKD'.", LOG_ERROR);
        exit(EXIT_FAILURE);
    }
}
