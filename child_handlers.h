#ifndef CHILD_HANDLERS_H
#define CHILD_HANDLERS_H

#define MAX_EVENTS 1000
#define MAX_LINE_LENGTH 256

void process_create_handler(int read_fd, int write_fd);
void memory_alloc_handler(int read_fd, int write_fd);
void file_open_handler(int read_fd, int write_fd);
void user_login_handler(int read_fd, int write_fd);
void system_boot_handler(int read_fd, int write_fd);
void ensure_trailing_newline(const char *filename);

#endif // CHILD_HANDLERS_H