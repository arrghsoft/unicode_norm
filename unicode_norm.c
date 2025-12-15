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
#define LOG_LEVEL LOG_DEBUG
#ifdef _WIN32
  #define PATH_SEP_CHAR '\\'
  #define PATH_SEP_STR "\\"
#else
  #define PATH_SEP_CHAR '/'
  #define PATH_SEP_STR "/"
#endif

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

static int total_cnt = 0;
static int updated_cnt = 0;

#ifdef _WIN32
#include <wchar.h>
#include <stdbool.h>

wchar_t *utf8_to_utf16(const char *s) {
    int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    wchar_t *w = malloc(sizeof(wchar_t) * len);
    MultiByteToWideChar(CP_UTF8, 0, s, -1, w, len);
    return w;
}

char *utf16_to_utf8(const wchar_t *w) {
    if (!w) return NULL;

    int size = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    if (size <= 0) return NULL;

    char *utf8 = (char *) malloc(size);
    if (!utf8) return NULL;

    WideCharToMultiByte(CP_UTF8, 0, w, -1, utf8, size, NULL, NULL);
    return utf8;
}

void traverse_directory_windows(const wchar_t *dir) {
    wchar_t search_path[MAX_PATH];
    wchar_t logbuf[STR_BUFFER];
    WIN32_FIND_DATAW ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    swprintf(search_path, MAX_PATH, L"%ls\\*", dir);
    hFind = FindFirstFileW(search_path, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        swprintf(logbuf, STR_BUFFER, L"FindFirstFile failed: %ls\n", dir);
        char* logbuf_utf8 = utf16_to_utf8(logbuf);
        print_log(logbuf_utf8, LOG_ERROR);
        free(logbuf_utf8);
        return;
    }

    do {
        const wchar_t *name = ffd.cFileName;
        if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0)
            continue;

        wchar_t full_path[MAX_PATH];
        swprintf(full_path, MAX_PATH, L"%ls\\%ls", dir, name);

        bool is_dir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        swprintf(logbuf, STR_BUFFER, L"File entry: %ls (%ls)", full_path, is_dir ? L"dir" : L"file");
        char* logbuf_utf8 = utf16_to_utf8(logbuf);
        print_log(logbuf_utf8, LOG_DEBUG);
        free(logbuf_utf8);

        char *full_path_utf8 = utf16_to_utf8(full_path);
        bool updated = rename_file(full_path_utf8);
        updated_cnt += updated ? 1 : 0;
        total_cnt++;
        free(full_path_utf8);

        if (is_dir) {
            traverse_directory_windows(full_path);
        }
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
}

#endif

static void split_path(const char *path, char* dir_out, size_t dir_size, char* name_out, size_t name_size);

int main(const int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

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

#ifndef _WIN32
        FILE *f = fopen(file_path, "r");
        if (f == NULL) {
            print_log("Error: could not open file.", LOG_ERROR);
            exit(EXIT_FAILURE);
        }
        fclose(f);
#endif

        if (!is_recursive) {
            rename_file(file_path);
        } else {
#ifndef _WIN32
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
#else
            wchar_t *a = utf8_to_utf16(*(argv + optind));
            traverse_directory_windows(a);
            free(a);
#endif

            if (i < argc - 1) {
                continue;
            }

            if (is_dry_run) {
                sprintf(string, "Total files: %d, Files that would be updated: %d", total_cnt, updated_cnt);
                print_log(string, LOG_INFO);
            } else {
                sprintf(string, "Total files: %d, Updated files: %d", total_cnt, updated_cnt);
                print_log(string, LOG_INFO);
            }
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
    char dir_part[PATH_MAX];
    char name_part[PATH_MAX];
    split_path(path, dir_part, sizeof(dir_part), name_part, sizeof(name_part));

    char* new_basename;
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

#ifndef _WIN32
    if (realpath(file_path, abs_path) == NULL) {
        perror("realpath");
        return false;
    }
#else
    if (_fullpath(abs_path, file_path, PATH_MAX) == NULL) {
        perror("_fullpath");
        return false;
    }
#endif

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

static void split_path(const char *path, char* dir_out, size_t dir_size, char* name_out, size_t name_size) {
    const char *last_sep = strrchr(path, PATH_SEP_CHAR);
#ifdef _WIN32
    const char *last_slash = strrchr(path, '/');
    if (!last_sep || (last_slash && last_slash > last_sep)) {
        last_sep = last_slash;
    }
#endif

    if (!last_sep) {
        if (dir_out && dir_size > 0) {
            dir_out[0] = '.';
            dir_out[1] = '\0';
        }
        strncpy(name_out, path, name_size - 1);
        name_out[name_size - 1] = '\0';
        return;
    }

    size_t dir_len = (size_t)(last_sep - path);
    if (dir_out && dir_size > 0) {
        if (dir_len >= dir_size) dir_len = dir_size - 1;
        memcpy(dir_out, path, dir_len);
        dir_out[dir_len] = '\0';
    }

    const char *name_start = last_sep + 1;
    strncpy(name_out, name_start, name_size - 1);
    name_out[name_size - 1] = '\0';
}
