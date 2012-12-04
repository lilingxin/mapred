#ifndef _UTIL_H__
#define _UTIL_H__

#define log(format, ...) fprintf(stderr, format, ##__VA_ARGS__)

#define error(exitno, errno, format, ...) \
    error_at_line(exitno, errno,  __FILE__, __LINE__, format, ##__VA_ARGS__)

#define check_error(x, format, ...) \
    do { \
        if (!(x)) { \
            error(EXIT_FAILURE, errno, format, ##__VA_ARGS__); \
        } \
    } while (0)

#define safe_malloc(x, T, size) \
    do { \
            (x) = (typeof(T) *)malloc(sizeof(typeof(T)) * (size)); \
            if (!(x)) { \
                error(EXIT_FAILURE, errno, "not enough memory!"); \
            } \
    } while (0)

#define safe_free(x)    \
    do {                \
        if ((x)) {      \
            free((x));  \
            (x) = NULL; \
        }               \
    } while (0)

#endif /* _UTIL_H__ */
