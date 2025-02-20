#include "child_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void process_create_handler(int read_fd, int write_fd) {
    int count = 0;
    char line[MAX_LINE_LENGTH];
    FILE *read_stream = fdopen(read_fd, "r");
    if (!read_stream) {
        perror("fdopen in process_create_handler");
        exit(1);
    }
    while (fgets(line, MAX_LINE_LENGTH, read_stream) != NULL) {
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        if (strstr(line, "PROCESS_CREATE") != NULL) {
            count++;
        }
    }
    write(write_fd, &count, sizeof(int));
    fflush(stdout);
    fclose(read_stream);
    close(write_fd);
    exit(0);
}

void memory_alloc_handler(int read_fd, int write_fd) {
    long long total_memory = 0;
    char line[MAX_LINE_LENGTH];
    FILE *read_stream = fdopen(read_fd, "r");
    if (!read_stream) {
        perror("fdopen in memory_alloc_handler");
        exit(1);
    }
    while (fgets(line, MAX_LINE_LENGTH, read_stream) != NULL) {
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        long long memory;
        if (sscanf(line, "%*s MEMORY_ALLOC PID:%*d SIZE:%lld", &memory) == 1) {
            total_memory += memory;
        }
    }
    write(write_fd, &total_memory, sizeof(long long));
    fflush(stdout);
    fclose(read_stream);
    close(write_fd);
    exit(0);
}

void file_open_handler(int read_fd, int write_fd) {
    char files[MAX_EVENTS][MAX_LINE_LENGTH];
    int file_counts[MAX_EVENTS] = {0};
    int num_files = 0;
    char line[MAX_LINE_LENGTH];
    FILE *read_stream = fdopen(read_fd, "r");
    if (!read_stream) {
        perror("fdopen in file_open_handler");
        exit(1);
    }
    while (fgets(line, MAX_LINE_LENGTH, read_stream) != NULL) {
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        if (strstr(line, "FILE_OPEN") != NULL) {
            char *path_start = strstr(line, "PATH:\"");
            if (path_start != NULL) {
                path_start += 6;
                char *path_end = strchr(path_start, '"');
                if (path_end != NULL) {
                    *path_end = '\0';
                    int i;
                    for (i = 0; i < num_files; i++) {
                        if (strcmp(files[i], path_start) == 0) {
                            file_counts[i]++;
                            break;
                        }
                    }
                    if (i == num_files) {
                        strcpy(files[num_files], path_start);
                        file_counts[num_files] = 1;
                        num_files++;
                    }
                }
            }
        }
    }
    int max_count = 0;
    char most_accessed_file[MAX_LINE_LENGTH] = "";
    for (int i = 0; i < num_files; i++) {
        if (file_counts[i] > max_count) {
            max_count = file_counts[i];
            strcpy(most_accessed_file, files[i]);
        }
    }
    write(write_fd, most_accessed_file, MAX_LINE_LENGTH);
    write(write_fd, &max_count, sizeof(int));
    fflush(stdout);
    fclose(read_stream);
    close(write_fd);
    exit(0);
}

void user_login_handler(int read_fd, int write_fd) {
    char users[MAX_EVENTS][MAX_LINE_LENGTH];
    int num_users = 0;
    char line[MAX_LINE_LENGTH];
    FILE *read_stream = fdopen(read_fd, "r");
    if (!read_stream) {
        perror("fdopen in user_login_handler");
        exit(1);
    }
    while (fgets(line, MAX_LINE_LENGTH, read_stream) != NULL) {
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        char *user_start = strstr(line, "USER:\"");
        if (user_start) {
            user_start += 6;
            char *user_end = strchr(user_start, '"');
            if (user_end) {
                *user_end = '\0';
                int i;
                for (i = 0; i < num_users; i++) {
                    if (strcmp(users[i], user_start) == 0)
                        break;
                }
                if (i == num_users) {
                    strcpy(users[num_users], user_start);
                    num_users++;
                }
            }
        }
    }
    write(write_fd, &num_users, sizeof(int));
    fflush(stdout);
    fclose(read_stream);
    close(write_fd);
    exit(0);
}

void system_boot_handler(int read_fd, int write_fd) {
    int count = 0;
    char line[MAX_LINE_LENGTH];
    FILE *read_stream = fdopen(read_fd, "r");
    if (!read_stream) {
        perror("fdopen in system_boot_handler");
        exit(1);
    }
    while (fgets(line, MAX_LINE_LENGTH, read_stream) != NULL) {
        if (line[0] == '\n' || line[0] == '\0')
            continue;
        if (strstr(line, "SYSTEM_BOOT") != NULL) {
            count++;
        }
    }
    write(write_fd, &count, sizeof(int));
    fflush(stdout);
    fclose(read_stream);
    close(write_fd);
    exit(0);
}

void ensure_trailing_newline(const char *filename) {
    FILE *f = fopen(filename, "rb+");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    if (fseek(f, -1, SEEK_END) != 0) {
        fclose(f);
        return;
    }
    int ch = fgetc(f);
    if (ch != '\n') {
        fseek(f, 0, SEEK_END);
        fputc('\n', f);
    }
    fclose(f);
}