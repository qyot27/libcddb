#ifndef CDDB_REGEX_H
#define CDDB_REGEX_H 1

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdlib.h>
#include <regex.h>


/**
 */
extern regex_t *REGEX_TRACK_FRAME_OFFSETS;
extern regex_t *REGEX_TRACK_FRAME_OFFSET;
extern regex_t *REGEX_DISC_LENGTH;
extern regex_t *REGEX_DISC_TITLE;
extern regex_t *REGEX_DISC_YEAR;
extern regex_t *REGEX_DISC_GENRE;
extern regex_t *REGEX_TRACK_TITLE;
extern regex_t *REGEX_QUERY_MATCH;


/**
 */
void cddb_regex_init(void);

/**
 */
int cddb_regex_get_int(const char *s, regmatch_t matches[], int idx);

/**
 */
char *cddb_regex_get_string(const char *s, regmatch_t matches[], int idx);


#ifdef __cplusplus
    }
#endif

#endif /* CDDB_REGEX_H */
