#ifndef CDDB_NET_H
#define CDDB_NET_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdarg.h>


/* --- socket-based work-alikes --- */

char *sock_fgets(char *s, int size, int sock);

size_t sock_fwrite(const void *ptr, size_t size, size_t nmemb, int sock);

int sock_fprintf(int sock, const char *format, ...);

int sock_vfprintf(int sock, const char *format, va_list ap);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_NET_H */
