

#include <stdlib.h>
#include <string.h>
#include "cddb/cddb_ni.h"


/**
 */
static int initialized = 0;

regex_t *REGEX_TRACK_FRAME_OFFSETS;
regex_t *REGEX_TRACK_FRAME_OFFSET;
regex_t *REGEX_DISC_LENGTH;
regex_t *REGEX_DISC_TITLE;
regex_t *REGEX_DISC_YEAR;
regex_t *REGEX_DISC_GENRE;
regex_t *REGEX_TRACK_TITLE;
regex_t *REGEX_QUERY_MATCH;


/**
 */
int cddb_regex_init_1(regex_t **p, const char *regex)
{
    if ((*p = (regex_t*)malloc(sizeof(regex_t))) == NULL) {
        // XXX: check memory alloc
        return -1;
    }
    return regcomp(*p, regex, REG_EXTENDED);
}


void cddb_regex_init()
{
    int rv;

    if (!initialized) {

        rv = cddb_regex_init_1(&REGEX_TRACK_FRAME_OFFSETS,
                               "^#[[:blank:]]*Track frame offsets:[[:blank:]]*$");
        rv = cddb_regex_init_1(&REGEX_TRACK_FRAME_OFFSET,
                               "^#[[:blank:]]*([0-9]+)[[:blank:]]*$");
        rv = cddb_regex_init_1(&REGEX_DISC_LENGTH,
                               "^#[[:blank:]]*Disc length: ([0-9]+) seconds[[:blank:]]*$");
        rv = cddb_regex_init_1(&REGEX_DISC_TITLE,
                               "^DTITLE=((.*) / (.*)|(.*))$");
        rv = cddb_regex_init_1(&REGEX_DISC_YEAR,
                               "^DYEAR=([0-9]*)$");
        rv = cddb_regex_init_1(&REGEX_DISC_GENRE,
                               "^DGENRE=(.*)$");
        rv = cddb_regex_init_1(&REGEX_TRACK_TITLE,
                               "^TTITLE([0-9]+)=((.*) / (.*)|(.*))$");
        rv = cddb_regex_init_1(&REGEX_QUERY_MATCH,
                               "^([[:alpha:]]+)[[:blank:]]([[:xdigit:]]+)[[:blank:]]((.*) / (.*)|(.*))$");

        initialized = 1;
    }
}

int cddb_regex_get_int(char *s, regmatch_t matches[], int idx)
{
    char c;
    int start, end, i;

    start = matches[idx].rm_so;
    end = matches[idx].rm_eo;
    c = s[end];
    s[end] = '\0';
    i = atoi(s+start);
    s[end] = c;
    return i;
}

int cddb_regex_get_hex(char *s, regmatch_t matches[], int idx)
{
    char c;
    int start, end, i;

    start = matches[idx].rm_so;
    end = matches[idx].rm_eo;
    c = s[end];
    s[end] = '\0';
    i = strtol(s+start, NULL, 16);
    s[end] = c;
    return i;
}

char *cddb_regex_get_string(const char *s, regmatch_t matches[], int idx)
{
    char *result;
    int start, end, len;

    start = matches[idx].rm_so;
    end = matches[idx].rm_eo;
    len = end - start;
    result = malloc(len + 1);
    strncpy(result, s+start, len);
    result[len] = '\0';
    return result;
}
