#include "../include/utils.h"
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

/* ── Internal helpers ────────────────────────────────────────────────────── */

static void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

/* ── Public functions ────────────────────────────────────────────────────── */

char *utils_read_line(char *buf, int size) {
    if (!fgets(buf, size, stdin)) return NULL;
    trim_newline(buf);
    return buf;
}

void utils_prompt_string(const char *prompt, char *buf, int size) {
    do {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, size, stdin)) {
            /* EOF or read error - return empty string */
            buf[0] = '\0';
            return;
        }
        trim_newline(buf);
        /* strip leading spaces */
        char *p = buf;
        while (*p == ' ') p++;
        if (p != buf) memmove(buf, p, strlen(p) + 1);
    } while (buf[0] == '\0');
}

double utils_prompt_double(const char *prompt) {
    char buf[64];
    double val;
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) return 0.0;  /* EOF */
        trim_newline(buf);
        char *end;
        val = strtod(buf, &end);
        if (end != buf && *end == '\0' && val >= 0.0) return val;
        printf("  Invalid input. Please enter a valid non-negative number.\n");
    }
}

int utils_prompt_int(const char *prompt) {
    char buf[32];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;  /* EOF */
        trim_newline(buf);
        char *end;
        long val = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && val >= 0) return (int)val;
        printf("  Invalid input. Please enter a valid non-negative integer.\n");
    }
}

int utils_prompt_positive_int(const char *prompt) {
    char buf[32];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;  /* EOF */
        trim_newline(buf);
        char *end;
        long val = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && val > 0) return (int)val;
        printf("  Invalid input. Please enter a positive integer (> 0).\n");
    }
}

void utils_print_separator(char ch, int width) {
    for (int i = 0; i < width; i++) putchar(ch);
    putchar('\n');
}

void utils_print_title(const char *title, int width) {
    utils_print_separator('=', width);
    int len  = (int)strlen(title);
    int pad  = (width - len) / 2;
    if (pad < 0) pad = 0;
    printf("%*s%s\n", pad, "", title);
    utils_print_separator('=', width);
}

void utils_current_datetime(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void utils_current_date(char *buf, int size) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, size, "%Y-%m-%d", tm_info);
}

int utils_date_in_range(const char *dt, const char *from, const char *to) {
    /* Compare the first 10 chars (YYYY-MM-DD) lexicographically */
    char d[11], f[11], t2[11];
    strncpy(d,  dt,   10); d[10]  = '\0';
    strncpy(f,  from, 10); f[10]  = '\0';
    strncpy(t2, to,   10); t2[10] = '\0';
    return (strcmp(d, f) >= 0 && strcmp(d, t2) <= 0);
}

void utils_ensure_data_dir(void) {
    struct stat st;
    if (stat("data", &st) != 0) {
        mkdir("data", 0755);
    }
}

void utils_read_password(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    fflush(stdout);

    struct termios oldt, newt;
    int tty = isatty(STDIN_FILENO);
    if (tty) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(tcflag_t)ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    if (!fgets(buf, size, stdin)) {
        buf[0] = '\0';
    } else {
        trim_newline(buf);
    }

    if (tty) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        putchar('\n');
    }
}
