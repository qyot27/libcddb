#ifndef CDDB_DEBUG_H
#define CDDB_DEBUG_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#ifdef DEBUG

#    define dlog(...) fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
                      fprintf(stderr, __VA_ARGS__); \
                      fprintf(stderr,"\n")

#else

#    define dlog(...)

#endif


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_DEBUG_H */
