
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cddb/cddb_ni.h"


#define CHR_CR         '\r'
#define CHR_LF         '\n'
#define CHR_EOS        '\0'
#define CHR_SPACE      ' '
#define CHR_DOT        '.'

static const char *CDDB_COMMANDS[5] = {
    "cddb hello %s %s %s %s",
    "quit",
    "cddb read %s %08x",
    "cddb query %08x %d %s %d",
    "cddb write %s %08x",
};

#define WRITE_BUF_SIZE 4096
#define QUERY_RESULT_SET_INC 10


/* --- prototypes --- */


/**
 * @return the line read or NULL if something goes wrong
 */
char *cddb_read_line(cddb_conn_t *c);

/**
 * @returns The amount of data written into the buffer.
 */
int cddb_write_data(char *buf, int size, cddb_disc_t *disc);

/**
 */
int cddb_http_parse_response(cddb_conn_t *c);

/**
 */
void cddb_http_parse_headers(cddb_conn_t *c);

/**
 */
int cddb_http_send_cmd(cddb_conn_t *c, int cmd, va_list args);

/**
 */
int cddb_parse_record(cddb_conn_t *c, cddb_disc_t *disc);

/**
 */
int cddb_parse_query_data(cddb_conn_t *c, cddb_disc_t *disc, const char *line);

/**
 */
int cddb_cache_exists(cddb_conn_t *c, cddb_disc_t *disc);

/**
 */
FILE *cddb_cache_open(cddb_conn_t *c, cddb_disc_t *disc, const char* mode);

/**
 */
int cddb_cache_read(cddb_conn_t *c, cddb_disc_t *disc);

/**
 */
int cddb_cache_mkdir(cddb_conn_t *c, cddb_disc_t *disc);


/* --- CDDB slave routines --- */


int cddb_cache_exists(cddb_conn_t *c, cddb_disc_t *disc)
{
    char fn[LINE_SIZE];
    struct stat buf;

    dlog("cddb_cache_exists()");

    /* try to stat cache file */
    snprintf(fn, sizeof(fn), "%s/%s/%08x", c->cache_dir, 
             CDDB_CATEGORY[disc->category], disc->discid);
    if ((stat(fn, &buf) == -1) || !S_ISREG(buf.st_mode)) {
        dlog("\tnot in cache");
        return FALSE;
    }
    dlog("\tin cache");
    return TRUE;
}

FILE *cddb_cache_open(cddb_conn_t *c, cddb_disc_t *disc, const char* mode)
{
    char fn[LINE_SIZE];

    dlog("cddb_cache_open()");
    snprintf(fn, sizeof(fn), "%s/%s/%08x", c->cache_dir, 
             CDDB_CATEGORY[disc->category], disc->discid);
    return fopen(fn, mode);
}

int cddb_cache_read(cddb_conn_t *c, cddb_disc_t *disc)
{
    FILE *fp, *old_fp;
    int rv;

    dlog("cddb_cache_read()");
    if (!c->use_cache) {
        /* don't use cache */
        return FALSE;
    }

    /* check whether cached version exists */
    if (!cddb_cache_exists(c, disc)) {
        /* no cached version available */
        dlog("\tno cached version found");
        return FALSE;
    }

    /* try to open cache file */
    fp = cddb_cache_open(c, disc, "r");
    if (fp == NULL) {
        /* cached version not readable */
        dlog("\tcached version not readable");
        return FALSE;
    }

    /* parse CDDB record */
    dlog("\tcached version found");
    old_fp = c->fp;
    c->fp = fp;
    rv = cddb_parse_record(c, disc);
    c->fp = old_fp;

    return rv;
}

int cddb_cache_mkdir(cddb_conn_t *c, cddb_disc_t *disc)
{
    char fn[LINE_SIZE];

    dlog("cddb_cache_mkdir()");
    /* create CDDB slave dir */
    if ((mkdir(c->cache_dir, 0755) == -1) && (errno != EEXIST)) {
        return FALSE;
    }

    /* create category dir */
    snprintf(fn, sizeof(fn), "%s/%s", c->cache_dir, CDDB_CATEGORY[disc->category]);
    if ((mkdir(fn, 0755) == -1) && (errno != EEXIST)) {
        return FALSE;
    }

    return TRUE;
}


/* --- miscellaneous --- */


void cddb_query_clear(cddb_conn_t *c)
{
    int i;

    dlog("cddb_query_clear()");
    if (c->query_data != NULL) {
        for (i = 0; i < c->query_cnt; i++) {
            cddb_disc_destroy(c->query_data[i]);
        }
        free(c->query_data);
        c->query_data = NULL;
        c->query_idx = 0;
        c->query_cnt = 0;
    }
}


/* --- server request / response handling --- */


int cddb_get_response_code(cddb_conn_t *c, const char **msg)
{
    const char *line, *space;
    int code, rv;

    dlog("cddb_get_response_code()");
    line = cddb_read_line(c);
    if (!line) {
        if (c->errnum != CDDB_ERR_OK) {
            c->errnum = CDDB_ERR_UNEXPECTED_EOF;
        }
        return -1;
    }

    rv = sscanf(line, "%d", &code);
    if (rv != 1) {
        c->errnum = CDDB_ERR_INVALID_RESPONSE;
        return -1;
    }

    space = strchr(line, CHR_SPACE);
    if (space == NULL) {
        c->errnum = CDDB_ERR_INVALID_RESPONSE;
        return -1;
    }
    *msg = space + 1;           /* message starts after space */

    c->errnum = CDDB_ERR_OK;
    dlog("\tcode = %d (%s)", code, *msg);
    return code;
}

char *cddb_read_line(cddb_conn_t *c)
{
    char *s;

    dlog("cddb_read_line()");
    /* read line, possibly returning NULL */
    s = fgets(c->line, LINE_SIZE, c->fp);

    /* strip off any line terminating characters */
    if (s) {
        s = s + strlen(s) - 1;
        while ((s >= c->line) && 
               ((*s == CHR_CR) || (*s == CHR_LF))) {
            *s = CHR_EOS;
            s--;
        }
    } else {
        return NULL;
    }

    c->errnum = CDDB_ERR_OK;
    dlog("\tline = '%s'", c->line);
    return c->line;
}

void url_encode(char *s)
{
    while (*s) {
        switch (*s) {
        case ' ': *s = '+'; break;
        }
        s++;
    }
}

int cddb_http_parse_response(cddb_conn_t *c)
{
    char *line;
    int code;

    if ((line = cddb_read_line(c)) == NULL) {
        /* no HTTP response line */
        c->errnum = CDDB_ERR_UNEXPECTED_EOF;
        return FALSE;
    }

    if (sscanf(line, "%*s %d %*s", &code) != 1) {
        /* invalid */
        c->errnum = CDDB_ERR_INVALID_RESPONSE;
        return FALSE;
    }

    dlog("\tHTTP response code = %d", code);
    switch (code) {
    case 200:
        /* HTTP OK */
        break;
    default:
        /* anythign else = error */
        c->errnum = CDDB_ERR_SERVER_ERROR;
        return FALSE;
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

void cddb_http_parse_headers(cddb_conn_t *c)
{
    char *line;

    dlog("cddb_http_parse_headers()");
    while (((line = cddb_read_line(c)) != NULL) &&
           (*line != CHR_EOS)) {
        /* no-op */
    }
}

int cddb_http_send_cmd(cddb_conn_t *c, int cmd, va_list args)
{
    dlog("cddb_http_send_cmd()");
    switch (cmd) {
    case CMD_WRITE:
        /* entry submission (POST method) */
        {
            char *category;
            int discid, size;

            category = va_arg(args, char *);
            discid = va_arg(args, int);
            size = va_arg(args, int);

            if (c->is_http_proxy_enabled) {
                /* use an HTTP proxy */
                fprintf(c->fp, "POST http://%s:%d%s HTTP/1.0\r\n", c->server_name, 
                        c->server_port, c->http_path_submit);
                fprintf(c->fp, "Host: %s:%d\r\n", c->server_name, c->server_port);
            } else {
                /* direct connection */
                fprintf(c->fp, "POST %s HTTP/1.0\r\n", c->http_path_submit);
            }

            fprintf(c->fp, "Category: %s\r\n", category);
            fprintf(c->fp, "Discid: %08x\r\n", discid);
            fprintf(c->fp, "User-Email: %s@%s\r\n", c->user, c->hostname);
            fprintf(c->fp, "Submit-Mode: submit\r\n");
            fprintf(c->fp, "Content-Length: %d\r\n", size);
            fprintf(c->fp, "\r\n");
        }
        break;
    default:
        /* anything else */
        {
            char buf[LINE_SIZE];

            if (c->is_http_proxy_enabled) {
                /* use an HTTP proxy */
                fprintf(c->fp, "GET http://%s:%d%s", c->server_name, 
                        c->server_port, c->http_path_query);
            } else {
                /* direct connection */
                fprintf(c->fp, "GET %s", c->http_path_query);
            }

            vsnprintf(buf, sizeof(buf), CDDB_COMMANDS[cmd], args);
            url_encode(buf);
            fprintf(c->fp, "?cmd=%s&", buf);

            fprintf(c->fp, "hello=%s+%s+%s+%s&", 
                    c->user, c->hostname, CLIENT_NAME, CLIENT_VERSION);
            fprintf(c->fp, "proto=5 HTTP/1.0\r\n");

            if (c->is_http_proxy_enabled) {
                /* insert host header */
                fprintf(c->fp, "Host: %s:%d\r\n", c->server_name, c->server_port);
            }
            fprintf(c->fp, "\r\n");

            /* parse HTTP response line */
            if (!cddb_http_parse_response(c)) {
                return FALSE;
            }

            /* skip HTTP response headers */
            cddb_http_parse_headers(c);
        }
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

int cddb_send_cmd(cddb_conn_t *c, int cmd, ...)
{
    va_list args;
    
    dlog("cddb_send_cmd()");
    if (!CONNECTION_OK(c)) {
        c->errnum = CDDB_ERR_NOT_CONNECTED;
        return FALSE;
    }
    
    va_start(args, cmd);
    if (c->is_http_enabled) {
        /* HTTP */
        if (!cddb_http_send_cmd(c, cmd, args)) {
            int errnum;

            errnum = c->errnum; /* save error number */
            cddb_disconnect(c);
            c->errnum = errnum; /* restore error number */
            return FALSE;
        }
    } else {
        /* CDDBP */
        vfprintf(c->fp, CDDB_COMMANDS[cmd], args);
        fprintf(c->fp, "\n");
    }
    va_end(args);

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

#define NEXT_LINE(c,l) (((l = cddb_read_line(c)) != NULL) && (*l != CHR_DOT))

#define STATE_START         0
#define STATE_TRACK_OFFSETS 1
#define STATE_DISC_LENGTH   2
#define STATE_DISC_TITLE    3
#define STATE_DISC_YEAR     4
#define STATE_DISC_GENRE    5
#define STATE_TRACK_TITLE   6
#define STATE_STOP          7

int cddb_parse_record(cddb_conn_t *c, cddb_disc_t *disc)
{
    char *line;
    int state;
    regmatch_t matches[6];
    cddb_track_t *track;
    int cache_content;
    int track_no = 0;
    FILE *cache = NULL;

    dlog("cddb_parse_record()");
    /* do we need to cache the processed content ? */
    cache_content = c->use_cache && !cddb_cache_exists(c, disc);
    if (cache_content) {
        /* create cache directory structure */
        cache_content = cddb_cache_mkdir(c, disc);
        if (cache_content) {
            cache = cddb_cache_open(c, disc, "w");
            cache_content = (cache != NULL);
        }
    }
    dlog("\tcache_content: %s", (cache_content ? "yes" : "no"));

    state = STATE_START;
    while (NEXT_LINE(c, line)) {

        if (cache_content) {
            fprintf(cache, "%s\n", line);
        }

        switch (state) {
        case STATE_START:
            dlog("\tstate: START");
            if (regexec(REGEX_TRACK_FRAME_OFFSETS, line, 0, NULL, 0) == 0) {
                /* expect a list of track frame offsets now */
                state = STATE_TRACK_OFFSETS;
            }
            break;
        case STATE_TRACK_OFFSETS:
            dlog("\tstate: TRACK OFFSETS");
            if (regexec(REGEX_TRACK_FRAME_OFFSET, line, 2, matches, 0) == 0) {
                track = cddb_disc_get_track(disc, track_no);
                if (!track) {
                    /* no such track present in disc structure yet */
                    track = cddb_track_new();
                    /* XXX: insert at track_no pos?? */
                    cddb_disc_add_track(disc, track);
                }
                track->frame_offset = cddb_regex_get_int(line, matches, 1);
                track_no++;
            } else {
                /* expect disc length now */
                state = STATE_DISC_LENGTH;
            }
            break;
        case STATE_DISC_LENGTH:
            dlog("\tstate: DISC LENGTH");
            if (regexec(REGEX_DISC_LENGTH, line, 2, matches, 0) == 0) {
                disc->length = cddb_regex_get_int(line, matches, 1);
                /* expect disc title now */
                state = STATE_DISC_TITLE;
            }            
            break;
        case STATE_DISC_TITLE:
            dlog("\tstate: DISC TITLE");
            if (regexec(REGEX_DISC_TITLE, line, 3, matches, 0) == 0) {
                disc->artist = cddb_regex_get_string(line, matches, 1);
                disc->title = cddb_regex_get_string(line, matches, 2);
                /* expect disc year now */
                state = STATE_DISC_YEAR;
            }            
            break;
        case STATE_DISC_YEAR:
            dlog("\tstate: DISC YEAR");
            if (regexec(REGEX_DISC_YEAR, line, 2, matches, 0) == 0) {
                /* expect disc genre now */
                state = STATE_DISC_GENRE;
                break;
            }
            /* fall through because disc year is optional */
        case STATE_DISC_GENRE:
            dlog("\tstate: DISC GENRE");
            if (regexec(REGEX_DISC_GENRE, line, 2, matches, 0) == 0) {
                /* expect track title now */
                state = STATE_TRACK_TITLE;
                break;
            }
            /* fall through because disc genre is optional */
        case STATE_TRACK_TITLE:
            dlog("\tstate: TRACK TITLE");
            if (regexec(REGEX_TRACK_TITLE, line, 6, matches, 0) == 0) {
                state = STATE_TRACK_TITLE;
                track_no = cddb_regex_get_int(line, matches, 1);
                track = cddb_disc_get_track(disc, track_no);
                if (track == NULL) {
                    c->errnum = CDDB_ERR_TRACK_NOT_FOUND;
                    return FALSE;
                }
                if (matches[3].rm_so != -1) {
                    /* both artist and title of track are specified */
                    track->artist = cddb_regex_get_string(line, matches, 3);
                    track->title = cddb_regex_get_string(line, matches, 4);
                } else {
                    /* only title of track is specified */
                    track->title = cddb_regex_get_string(line, matches, 5);
                }
            } else {
                /* we're done parsing */
                state = STATE_STOP;
            }
            break;
        }
    }

    if (cache_content) {
        fclose(cache);
    }

    if (state != STATE_STOP) {
        c->errnum = CDDB_ERR_INVALID_RESPONSE;
        return FALSE;
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}


/* --- server commands --- */


int cddb_read(cddb_conn_t *c, cddb_disc_t *disc)
{
    const char *msg;
    int code, rc;

    dlog("cddb_read()");
    /* check whether we have enough info to execute the command */
    if ((disc->category == CDDB_CAT_INVALID) || (disc->discid == 0)) {
        c->errnum = CDDB_ERR_DATA_MISSING;
        return FALSE;
    }

    if (cddb_cache_read(c, disc)) {
        /* cached version found */
        return TRUE;
    }

    if (!cddb_connect(c)) {
        /* connection not OK */
        return FALSE;
    }

    /* send read command and check response */
    if (!cddb_send_cmd(c, CMD_READ, CDDB_CATEGORY[disc->category], disc->discid)) {
        return FALSE;
    }
    switch (code = cddb_get_response_code(c, &msg)) {
    case  -1:
        return FALSE;
    case 210:                   /* OK, CDDB database entry follows */
        break;
    case 401:                   /* specified CDDB entry not found */
        c->errnum = CDDB_ERR_DISC_NOT_FOUND;
        return FALSE;
    case 402:                   /* server error */
    case 403:                   /* database entry is corrupt */
        c->errnum = CDDB_ERR_SERVER_ERROR;
        return FALSE;
    case 409:                   /* no handshake */
    case 530:                   /* server error, server timeout */
        cddb_disconnect(c);
        c->errnum = CDDB_ERR_NOT_CONNECTED;
        return FALSE;
    default:
        c->errnum = CDDB_ERR_UNKNOWN;
        return FALSE;
    }

    /* parse CDDB record */
    rc = cddb_parse_record(c, disc);

    /* close connection if using HTTP */
    if (c->is_http_enabled) {
        cddb_disconnect(c);
    }

    return rc;
}

int cddb_parse_query_data(cddb_conn_t *c, cddb_disc_t *disc, const char *line)
{
    char *aux;
    regmatch_t matches[7];

    if (regexec(REGEX_QUERY_MATCH, line, 7, matches, 0) == REG_NOMATCH) {
        /* invalid repsponse */
        c->errnum = CDDB_ERR_INVALID_RESPONSE;
        return FALSE;
    }
    /* extract category */
    aux = cddb_regex_get_string(line, matches, 1);
    cddb_disc_set_category(disc, aux);
    /* extract disc ID */
    aux = cddb_regex_get_string(line, matches, 2);
    disc->discid = strtoll(aux, NULL, 16);
    /* extract artist and title */
    if (matches[4].rm_so != -1) {
        /* both artist and title of disc are specified */
        disc->artist = cddb_regex_get_string(line, matches, 4);
        disc->title = cddb_regex_get_string(line, matches, 5);
    } else {
        /* only title of disc is specified */
        disc->title = cddb_regex_get_string(line, matches, 6);
    }        

    free(aux);
    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

int cddb_query(cddb_conn_t *c, cddb_disc_t *disc)
{
    const char *msg;
    char *line;
    int code, count;
    char buf[LINE_SIZE], offset[32];
    cddb_track_t *track;

    dlog("cddb_query()");
    /* clear previous query result set */
    cddb_query_clear(c);
    
    /* check whether we have enough info to execute the command */
    dlog("\tdisc->discid    = %8x", disc->discid);
    dlog("\tdisc->length    = %d", disc->length);
    dlog("\tdisc->track_cnt = %d", disc->track_cnt);
    if ((disc->discid == 0) || (disc->length == 0) || (disc->track_cnt == 0)) {
        c->errnum = CDDB_ERR_DATA_MISSING;
        return -1;
    }

    /* check track offsets and generate offset list */
    buf[0] = CHR_EOS;
    for (track = cddb_disc_get_track_first(disc); 
         track != NULL; 
         track = cddb_disc_get_track_next(disc)) {
        if (track->frame_offset == -1) {
            c->errnum = CDDB_ERR_DATA_MISSING;
            return -1;
        }
        // XXX: buffer overflow checking
        snprintf(offset, sizeof(offset), "%d ", track->frame_offset);
        strcat(buf, offset);
    }

    if (!cddb_connect(c)) {
        /* connection not OK */
        return -1;
    }

    /* send query command and check response */
    if (!cddb_send_cmd(c, CMD_QUERY, disc->discid, disc->track_cnt, buf, disc->length)) {
        return -1;
    }
    switch (code = cddb_get_response_code(c, &msg)) {
    case  -1:
        return -1;
    case 200:                   /* found exact match */
        dlog("\texact match");
        if (!cddb_parse_query_data(c, disc, msg)) {
            return -1;
        }
        count = 1;
        break;
    case 211:                   /* found inexact match, list follows */
        dlog("\tinexact match");
        {
            int query_max = 0;
            while (NEXT_LINE(c, line)) {
                /* check whether there is enough space in query result set */
                if (c->query_cnt >= query_max) {
                    /* realloc */
                    query_max += QUERY_RESULT_SET_INC;
                    c->query_data = realloc(c->query_data, query_max*sizeof(cddb_disc_t*));
                }
                /* clone disc and fill in the blanks */
                c->query_data[c->query_cnt] = cddb_disc_clone(disc);
                if (!cddb_parse_query_data(c, c->query_data[c->query_cnt], line)) {
                    return -1;
                }
                c->query_cnt++;
            }
            if (c->query_cnt == 0) {
                /* empty result set */
                c->errnum = CDDB_ERR_INVALID_RESPONSE;
                return -1;
            }
            /* return first disc in result set */
            cddb_disc_copy(disc, c->query_data[c->query_idx++]);
        }
        count = c->query_cnt;
        break;
    case 202:                   /* no match found */
        dlog("\tno match");
        count = 0;
        break;
    case 403:                   /* database entry is corrupt */
        c->errnum = CDDB_ERR_SERVER_ERROR;
        return -1;
    case 409:                   /* no handshake */
    case 530:                   /* server error, server timeout */
        cddb_disconnect(c);
        c->errnum = CDDB_ERR_NOT_CONNECTED;
        return -1;
    default:
        c->errnum = CDDB_ERR_UNKNOWN;
        return -1;
    }

    /* close connection if using HTTP */
    if (c->is_http_enabled) {
        cddb_disconnect(c);
    }

    dlog("\tnumber of matches: %d", count);
    c->errnum = CDDB_ERR_OK;
    return count;
}

int cddb_query_next(cddb_conn_t *c, cddb_disc_t *disc)
{
    dlog("cddb_query_next()");
    if ((c->query_cnt == 0) || (c->query_idx >= c->query_cnt)) {
        /* no more discs */
        c->errnum = CDDB_ERR_DISC_NOT_FOUND;
        return FALSE;
    }
    /* return next disc in result set */
    cddb_disc_copy(disc, c->query_data[c->query_idx++]);

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}

int cddb_write_data(char *buf, int size, cddb_disc_t *disc)
{
    int i, remaining;
    cddb_track_t *track;

#define CDDB_WRITE_APPEND(l, ...) \
            snprintf(buf, remaining, __VA_ARGS__); remaining -= l; buf += l;

    remaining = size;
    CDDB_WRITE_APPEND(9, "# xmcd\n#\n");
    /* track offsets */
    CDDB_WRITE_APPEND(23, "# Track frame offsets:\n");
    for (track = cddb_disc_get_track_first(disc); 
         track != NULL; 
         track = cddb_disc_get_track_next(disc)) {
        CDDB_WRITE_APPEND(6+8, "#    %8d\n", track->frame_offset);
    }
    /* disc length */
    CDDB_WRITE_APPEND(26+6, "#\n# Disc length: %6d seconds\n", disc->length);
    /* submission info */
    CDDB_WRITE_APPEND(16, "#\n# Revision: 0\n");
    CDDB_WRITE_APPEND(21+strlen(CLIENT_NAME)+strlen(CLIENT_VERSION),
                      "# Submitted via: %s %s\n#\n", CLIENT_NAME, CLIENT_VERSION);
    /* disc data */
    CDDB_WRITE_APPEND(8+8, "DISCID=%08x\n", disc->discid);
    CDDB_WRITE_APPEND(11+strlen(disc->artist)+strlen(disc->title),
                      "DTITLE=%s / %s\n", disc->artist, disc->title);
    //fprintf(fp, "DYEAR=%s\n",);
    CDDB_WRITE_APPEND(8+strlen(CDDB_CATEGORY[disc->category]),
                      "DGENRE=%s\n", CDDB_CATEGORY[disc->category]);
    /* track data */
    for (track = cddb_disc_get_track_first(disc), i=0; 
         track != NULL; 
         track = cddb_disc_get_track_next(disc), i++) {
        if (track->artist != NULL) {
            CDDB_WRITE_APPEND(11+(i/10+1)+strlen(track->artist)+strlen(track->title),
                              "TTITLE%d=%s / %s\n", i, track->artist, track->title);
        } else {
            CDDB_WRITE_APPEND(8+(i/10+1)+strlen(track->title),
                              "TTITLE%d=%s\n", i, track->title);
        }
    }
    /* extended data */
    CDDB_WRITE_APPEND(6, "EXTD=\n");
    for (i=0; i<disc->track_cnt; i++) {
        CDDB_WRITE_APPEND(6+(i/10+1), "EXTT%d=\n", i);
    }
    /* play order */
    CDDB_WRITE_APPEND(11, "PLAYORDER=\n");

    return (size - remaining);
}

int cddb_write(cddb_conn_t *c, cddb_disc_t *disc)
{
    const char *msg;
    int code, size;
    cddb_track_t *track;
    FILE *cache;
    char buf[WRITE_BUF_SIZE];

    dlog("cddb_write()");
    /* check whether we have enough disc data to execute the command */
    if ((disc->discid == 0) || (disc->category == CDDB_CAT_INVALID) || 
        (disc->length == 0) || (disc->track_cnt == 0) ||
        (disc->artist == NULL) || (disc->title == NULL)) {
        c->errnum = CDDB_ERR_DATA_MISSING;
        return FALSE;
    }

    /* check whether we have enough track data to execute the command */
    for (track = cddb_disc_get_track_first(disc); 
         track != NULL; 
         track = cddb_disc_get_track_next(disc)) {
        if ((track->frame_offset == -1) || (track->title == NULL)) {
            c->errnum = CDDB_ERR_DATA_MISSING;
            return FALSE;
        }
    }

    /* create CDDB entry */
    size = cddb_write_data(buf, sizeof(buf), disc);
    
    /* cache data if needed */
    if (c->use_cache) {
        /* create cache directory structure */
        if (cddb_cache_mkdir(c, disc)) {
            /* open file, possibly overwriting it */
            dlog("\tcaching data");
            cache = cddb_cache_open(c, disc, "w");
            fwrite(buf, sizeof(char), size, cache);
            fclose(cache);
        }
    }

    if (!cddb_connect(c)) {
        /* connection not OK */
        return FALSE;
    }

    /* send query command and check response */
    if (!cddb_send_cmd(c, CMD_WRITE, CDDB_CATEGORY[disc->category], disc->discid, size)) {
        return FALSE;
    }
    if (!c->is_http_enabled) {
        switch (code = cddb_get_response_code(c, &msg)) {
        case  -1:
            return FALSE;
        case 320:                   /* OK, input CDDB data */
            break;
        case 401:                   /* permission denied */
        case 402:                   /* server file system full/file access failed */
        case 501:                   /* entry rejected */
            dlog("\tpermission denied");
            c->errnum = CDDB_ERR_PERMISSION_DENIED;
            return FALSE;
        case 409:                   /* no handshake */
        case 530:                   /* server error, server timeout */
            cddb_disconnect(c);
            c->errnum = CDDB_ERR_NOT_CONNECTED;
            return FALSE;
        default:
            c->errnum = CDDB_ERR_UNKNOWN;
            return FALSE;
        }
    }

    /* ready to send data */
    dlog("\tsending data");
    fwrite(buf, sizeof(char), size, c->fp);
    if (c->is_http_enabled) {
        /* skip HTTP response headers */
        cddb_http_parse_headers(c);
    } else {
        /* send terminating marker */
        fprintf(c->fp, ".\n");
    }

    /* check response */
    switch (code = cddb_get_response_code(c, &msg)) {
    case  -1:
        return FALSE;
    case 200:                   /* CDDB entry accepted */
        dlog("\tentry accepted");
        break;
    case 401:                   /* CDDB entry rejected */
    case 500:                   /* (HTTP) Missing required header information */
    case 501:                   /* (HTTP) Invalid header information */
        dlog("\tentry not accepted");
        c->errnum = CDDB_ERR_REJECTED;
        return FALSE;
    case 530:                   /* server error, server timeout */
        cddb_disconnect(c);
        c->errnum = CDDB_ERR_NOT_CONNECTED;
        return FALSE;
    default:
        c->errnum = CDDB_ERR_UNKNOWN;
        return FALSE;
    }

    /* close connection if using HTTP */
    if (c->is_http_enabled) {
        cddb_disconnect(c);
    }

    c->errnum = CDDB_ERR_OK;
    return TRUE;
}
