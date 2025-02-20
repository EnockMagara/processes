#include "child_handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_CHILDREN 5

int main() {
    ensure_trailing_newline("events_log.txt");  // Ensure file ends with newline

    int pipes[NUM_CHILDREN][2][2]; // [child][0]: parent-to-child, [child][1]: child-to-parent
    pid_t pids[NUM_CHILDREN];

    // Create pipes for each child.
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(pipes[i][0]) == -1 || pipe(pipes[i][1]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    // Spawn child processes.
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            exit(1);
        } else if (pids[i] == 0) {
            // In each child, close all pipe FDs not used by this child.
            for (int j = 0; j < NUM_CHILDREN; j++) {
                if (j != i) {
                    close(pipes[j][0][0]);
                    close(pipes[j][0][1]);
                    close(pipes[j][1][0]);
                    close(pipes[j][1][1]);
                }
            }
            // For our own pipe, close unused ends.
            close(pipes[i][0][1]); // Child reads from parent's pipe.
            close(pipes[i][1][0]); // Child writes to parent's pipe.
            switch (i) {
                case 0:
                    process_create_handler(pipes[i][0][0], pipes[i][1][1]);
                    break;
                case 1:
                    memory_alloc_handler(pipes[i][0][0], pipes[i][1][1]);
                    break;
                case 2:
                    file_open_handler(pipes[i][0][0], pipes[i][1][1]);
                    break;
                case 3:
                    user_login_handler(pipes[i][0][0], pipes[i][1][1]);
                    break;
                case 4:
                    system_boot_handler(pipes[i][0][0], pipes[i][1][1]);
                    break;
                default:
                    exit(1);
            }
        }
    }

    // In the parent, close unused ends.
    for (int i = 0; i < NUM_CHILDREN; i++) {
        close(pipes[i][0][0]); // Parent writes to child.
        close(pipes[i][1][1]); // Parent reads from child.
    }

    // Open the events file and send events to children.
    FILE *file = fopen("events_log.txt", "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }
    char line[MAX_LINE_LENGTH];
    int total_events = 0;
    int process_create_events = 0;
    int memory_alloc_events = 0;
    int file_open_events = 0;
    int user_login_events = 0;
    int system_boot_events = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        total_events++;
        if (strstr(line, "PROCESS_CREATE") != NULL) {
            process_create_events++;
            write(pipes[0][0][1], line, strlen(line) + 1);
            write(pipes[0][0][1], "\n", 1);
        } else if (strstr(line, "MEMORY_ALLOC") != NULL) {
            memory_alloc_events++;
            write(pipes[1][0][1], line, strlen(line) + 1);
            write(pipes[1][0][1], "\n", 1);
        } else if (strstr(line, "FILE_OPEN") != NULL) {
            file_open_events++;
            write(pipes[2][0][1], line, strlen(line) + 1);
            write(pipes[2][0][1], "\n", 1);
        } else if (strstr(line, "USER_LOGIN") != NULL) {
            user_login_events++;
            write(pipes[3][0][1], line, strlen(line) + 1);
            write(pipes[3][0][1], "\n", 1);
        } else if (strstr(line, "SYSTEM_BOOT") != NULL) {
            system_boot_events++;
            write(pipes[4][0][1], line, strlen(line) + 1);
            write(pipes[4][0][1], "\n", 1);
        }
    }
    fclose(file);
    for (int i = 0; i < NUM_CHILDREN; i++) {
        close(pipes[i][0][1]);
    }

    int process_create_count;
    long long total_memory;
    char most_accessed_file[MAX_LINE_LENGTH];
    int max_count;
    int num_unique_users;
    int system_boot_count;
    
    read(pipes[0][1][0], &process_create_count, sizeof(int));
    read(pipes[1][1][0], &total_memory, sizeof(long long));
    read(pipes[2][1][0], most_accessed_file, MAX_LINE_LENGTH);
    read(pipes[2][1][0], &max_count, sizeof(int));
    read(pipes[3][1][0], &num_unique_users, sizeof(int));
    read(pipes[4][1][0], &system_boot_count, sizeof(int));
    
    for (int i = 0; i < NUM_CHILDREN; i++) {
        close(pipes[i][1][0]);
    }
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    // Final output (must match the sample exactly)
    printf("PROCESS_CREATE events: %d\n", process_create_count);
    printf("MEMORY_ALLOC events: Total memory allocated: %lld bytes\n", total_memory);
    printf("FILE_OPEN events: Most accessed file: \"%s\" (%d times)\n", most_accessed_file, max_count);
    printf("USER_LOGIN events: Number of unique users: %d\n", num_unique_users);
    printf("SYSTEM_BOOT events: Number of system boots: %d\n", system_boot_count);
    printf("Overall Statistics:\n");
    printf("Total events processed: %d\n", total_events);
    
    char *most_common_event = "PROCESS_CREATE";
    int most_common_count = process_create_events;
    if (file_open_events > most_common_count) {
        most_common_event = "FILE_OPEN";
        most_common_count = file_open_events;
    }
    if (user_login_events > most_common_count) {
        most_common_event = "USER_LOGIN";
        most_common_count = user_login_events;
    }
    if (system_boot_events > most_common_count) {
        most_common_event = "SYSTEM_BOOT";
        most_common_count = system_boot_events;
    }
    if (memory_alloc_events > most_common_count) {
        most_common_event = "MEMORY_ALLOC";
        most_common_count = memory_alloc_events;
    }
    printf("Most common event type: %s (%d occurrences)\n", most_common_event, most_common_count);
    
    return 0;
}