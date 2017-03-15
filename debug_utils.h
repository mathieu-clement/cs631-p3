#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#ifndef DEBUG
#define DEBUG 0
#endif

// Based on http://stackoverflow.com/a/1644898/753136
#define debug(format, ...) \
                do { if (DEBUG) printf(" %s:%d  %s  " format "\n", \
                               __FILE__, __LINE__, __func__, __VA_ARGS__); } \
                while (0)

#endif
