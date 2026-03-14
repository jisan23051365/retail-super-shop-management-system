#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/* ── Function prototypes ─────────────────────────────────────────────────── */

/* Read a line from stdin, strip newline; returns buf on success or NULL */
char *utils_read_line(char *buf, int size);

/* Read a trimmed, non-empty string; re-prompts on blank input */
void utils_prompt_string(const char *prompt, char *buf, int size);

/* Read a double; re-prompts on invalid input */
double utils_prompt_double(const char *prompt);

/* Read a non-negative integer; re-prompts on invalid input */
int utils_prompt_int(const char *prompt);

/* Read a positive integer (>0); re-prompts on invalid/zero input */
int utils_prompt_positive_int(const char *prompt);

/* Print a separator line */
void utils_print_separator(char ch, int width);

/* Print a centred title between separator lines */
void utils_print_title(const char *title, int width);

/* Get current datetime string "YYYY-MM-DD HH:MM:SS" */
void utils_current_datetime(char *buf, int size);

/* Get current date string "YYYY-MM-DD" */
void utils_current_date(char *buf, int size);

/* Return 1 if date string 'dt' falls within [from, to] (YYYY-MM-DD prefix) */
int utils_date_in_range(const char *dt, const char *from, const char *to);

/* Ensure the data directory exists */
void utils_ensure_data_dir(void);

/* Securely read a password without echoing (falls back to normal read) */
void utils_read_password(const char *prompt, char *buf, int size);

#endif /* UTILS_H */
