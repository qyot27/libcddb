#ifndef CDDB_DEBUG_H
#define CDDB_DEBUG_H 1


#ifdef DEBUG

#    define dlog(...) fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
                      fprintf(stderr, __VA_ARGS__); \
                      fprintf(stderr,"\n")

#else

#    define dlog(...)

#endif

#endif /* CDDB_DEBUG_H */
