
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cddb/cddb_ni.h"


static const char *CDDB_COMMANDS[CMD_LAST] = {
    "cddb hello %s %s %s %s",
    "quit",
    "cddb read %s %08x",
    "cddb query %08x %d %s %d",
    "cddb write %s %08x",
    "proto %d",
};

#define WRITE_BUF_SIZE 4096
#define QUERY_RESULT_SET_INC 10

#define USE_CACHED_ENTRY(c) (c->cache_fp != NULL)

/*
 * Small memory cache for querying local database.
 */
#define QUERY_CACHE_SIZE 256
static struct query_cache_entry {
    unsigned int discid;
    cddb_cat_t category;
} query_cache[QUERY_CACHE_SIZE];


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
int cddb_cache_open(cddb_conn_t *c, cddb_disc_t *disc, const char* mode);

/**
 */
void cddb_cache_close(cddb_conn_t *c);

/**
 */
int cddb_cache_read(cddb_conn_t *c, cddb_disc_t *disc);

/**
 */
int cddb_cache_query(cddb_conn_t *c, cddb_disc_t *disc);

/**
 */
int cddb_cache_query_disc(cddb_conn_t *c, cddb_disc_t *disc);

/**
 * Initialize the local query cache.
 */
void cddb_cache_query_init(void);

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

int cddb_cache_open(cddb_conn_t *c, cddb_disc_t *disc, const char* mode)
{
    char fn[LINE_SIZE];

    dlog("cddb_cache_open()");
    /* close previous entry */
    cddb_cache_close(c);
    /* open new entry */
    snprintf(fn, sizeof(fn), "%s/%s/%08x", c->cache_dir, 
             CDDB_CATEGORY[disc->category], disc->discid);
    c->cache_fp = fopen(fn, mode);
    return (c->cache_fp != NULL);
}

void cddb_cache_close(cddb_conn_t *c)
{
    dlog("cddb_cache_close()");
    if (c->cache_fp != NULL) {
        fclose(c->cache_fp);
        c->cache_fp = NULL;
    }
}

int cddb_cache_read(cddb_conn_t *c, cddb_disc_t *disc)
{
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
    if (!cddb_cache_open(c, disc, "r")) {
        /* cached version not readable */
        dlog("\tcached version not readable");
        return FALSE;
    }

    /* parse CDDB record */
    dlog("\tcached version found");
    rv = cddb_parse_record(c, disc);

    /* close cache entry */
    cddb_cache_close(c);

    return rv;
}

inline void cddb_cache_query_init(void)
{
    static int query_cache_init = FALSE;
    int i;

    if (!query_cache_init) {
        for (i = 0; i < sizeof(QUERY_CACHE_SIZE); i++) {
            query_cache[i].category = CDDB_CAT_INVALID;
        }
        query_cache_init = TRUE;
    }
}

inline int cddb_cache_query_hash(cddb_disc_t *disc)
{
    /* use upper 8 bits of disc ID as hash */
    return disc->discid >> 24;
}

int cddb_cache_query(cddb_conn_t *c, cddb_disc_t *disc)
{
    int hash;

    dlog("cddb_cache_query()");
    if (!c->use_cache) {
        /* don't use cache */
        return FALSE;
    }

    /* initialize memory cache */
    cddb_cache_query_init();

    /* calculate disc hash */
    hash = cddb_cache_query_hash(disc);

    /* data already in memory? */
    if (query_cache[hash].discid == disc->discid) {
        dlog("\tentry found in memory");
        disc->category = query_cache[hash].category;
        c->errnum = CDDB_ERR_OK;
        return TRUE;
    }

    /* search local database on disc */
    return cddb_cache_query_disc(c, disc);
}

int cddb_cache_query_disc(cddb_conn_t *c, cddb_disc_t *disc)
{
    int cat, hash;

    dlog("cddb_cache_query_disc()");
    for (cat = CDDB_CAT_DATA; cat < CDDB_CAT_INVALID; cat++) {
        disc->category = cat;
        if (cddb_cache_exists(c, disc)) {
            /* update memory cache */
            hash = cddb_cache_query_hash(disc);
            query_cache[hash].discid = disc->discid;
            query_cache[hash].category = disc->category;
            dlog("\tentry found in local db");
            c->errnum = CDDB_ERR_OK;
            return TRUE;
        }
    }
    disc->category = CDDB_CAT_INVALID;
    dlog("\tentry not found in local db");
    return FALSE;
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


int cddb_get_response_code(cddb_conn_t *c, char **msg)
{
    char *line, *space;
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
    if (USE_CACHED_ENTRY(c)) {
        s = fgets(c->line, LINE_SIZE, cddb_cache_file(c));
    } else {
        s = sock_fgets(c->line, LINE_SIZE, c->socket);
    }

    /* strip off any line-terminating characters */
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
                sock_fprintf(c->socket, "POST http://%s:%d%s HTTP/1.0\r\n", 
                             c->server_name, c->server_port, c->http_path_submit);
                sock_fprintf(c->socket, "Host: %s:%d\r\n", c->server_name, c->server_port);
            } else {
                /* direct connection */
                sock_fprintf(c->socket, "POST %s HTTP/1.0\r\n", c->http_path_submit);
            }

            sock_fprintf(c->socket, "Category: %s\r\n", category);
            sock_fprintf(c->socket, "Discid: %08x\r\n", discid);
            sock_fprintf(c->socket, "User-Email: %s@%s\r\n", c->user, c->hostname);
            sock_fprintf(c->socket, "Submit-Mode: submit\r\n");
            sock_fprintf(c->socket, "Content-Length: %d\r\n", size);
            sock_fprintf(c->socket, "\r\n");
        }
        break;
    default:
        /* anything else */
        {
            char buf[LINE_SIZE];

            if (c->is_http_proxy_enabled) {
                /* use an HTTP proxy */
                sock_fprintf(c->socket, "GET http://%s:%d%s", c->server_name, 
                             c->server_port, c->http_path_query);
            } else {
                /* direct connection */
                sock_fprintf(c->socket, "GET %s", c->http_path_query);
            }

            vsnprintf(buf, sizeof(buf), CDDB_COMMANDS[cmd], args);
            url_encode(buf);
            sock_fprintf(c->socket, "?cmd=%s&", buf);

            sock_fprintf(c->socket, "hello=%s+%s+%s+%s&", 
                         c->user, c->hostname, CLIENT_NAME, CLIENT_VERSION);
            sock_fprintf(c->socket, "proto=%d", DEFAULT_PROTOCOL_VERSION);
            sock_fprintf(c->socket, " HTTP/1.0\r\n");

            if (c->is_http_proxy_enabled) {
                /* insert host header */
                sock_fprintf(c->socket, "Host: %s:%d\r\n", c->server_name, c->server_port);
            }
            sock_fprintf(c->socket, "\r\n");

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
        sock_vfprintf(c->socket, CDDB_COMMANDS[cmd], args);
        sock_fprintf(c->socket, "\n");
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

#define MULTI_NONE          0
#define MULTI_ARTIST        1
#define MULTI_TITLE         2

int cddb_parse_record(cddb_conn_t *c, cddb_disc_t *disc)
{
    char *line, *buf;
    int state, multi_line = MULTI_NONE;
    regmatch_t matches[6];
    cddb_track_t *track;
    int cache_content;
    int track_no = 0, old_no = -1;

    dlog("cddb_parse_record()");
    /* do we need to cache the processed content ? */
    cache_content = c->use_cache && !cddb_cache_exists(c, disc);
    if (cache_content) {
        /* create cache directory structure */
        cache_content = cddb_cache_mkdir(c, disc);
        cache_content &= cddb_cache_open(c, disc, "w");
    }
    dlog("\tcache_content: %s", (cache_content ? "yes" : "no"));

    state = STATE_START;
    while (NEXT_LINE(c, line)) {

        if (cache_content) {
            fprintf(cddb_cache_file(c), "%s\n", line);
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
            if (regexec(REGEX_DISC_TITLE, line, 5, matches, 0) == 0) {
                /* XXX: more error detection possible! */
                if (matches[2].rm_so != -1) {
                    /* both artist and title of disc are specified */
                    buf = cddb_regex_get_string(line, matches, 2);
                    cddb_disc_append_artist(disc, buf);
                    free(buf);
                    buf = cddb_regex_get_string(line, matches, 3);
                    cddb_disc_append_title(disc, buf);
                    free(buf);
                    /* we should only get title continuations now */
                    multi_line = MULTI_TITLE;
                } else {
                    /* only title or artist of disc on this line */
                    if (multi_line != MULTI_TITLE) {
                        /* this line is part of the artist name */
                        buf = cddb_regex_get_string(line, matches, 4);
                        cddb_disc_append_artist(disc, buf);
                        free(buf);
                        /* next line might be continuation of artist name */
                        multi_line = MULTI_ARTIST;
                    } else {
                        /* this line is part of the title */
                        buf = cddb_regex_get_string(line, matches, 4);
                        cddb_disc_append_title(disc, buf);
                        free(buf);
                    }
                }
                break;
            }
            if (multi_line == MULTI_NONE) {
                /* not yet parsing multi-line DTITLE */
                /* might be comment line, just skip it */
                break;
            }
            multi_line = MULTI_NONE;
            /* fall through to end multi-line disc title */
        case STATE_DISC_YEAR:
            dlog("\tstate: DISC YEAR");
            if (regexec(REGEX_DISC_YEAR, line, 2, matches, 0) == 0) {
                disc->year = cddb_regex_get_int(line, matches, 1);
                /* expect disc genre now */
                state = STATE_DISC_GENRE;
                break;
            }
            /* fall through because disc year is optional */
        case STATE_DISC_GENRE:
            dlog("\tstate: DISC GENRE");
            if (regexec(REGEX_DISC_GENRE, line, 2, matches, 0) == 0) {
                disc->genre = cddb_regex_get_string(line, matches, 1);
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
                if (track_no != old_no) {
                    /* reset multi-line flag, expect artist first */
                    multi_line = MULTI_ARTIST;
                }
                if (matches[3].rm_so == -1) {
                    /* only title or artist of track on this line */
                    if (multi_line != MULTI_TITLE) {
                        /* this line might be part of the artist,
                           but if we don't encounter a ' / ' it's the title,
                           so we use the title space for now and fix it later
                           if needed (see below) */
                        buf = cddb_regex_get_string(line, matches, 5);
                        cddb_track_append_title(track, buf);
                        free(buf);
                    } else {
                        /* this line is part of the title */
                        buf = cddb_regex_get_string(line, matches, 5);
                        cddb_disc_append_title(disc, buf);
                        free(buf);
                    }
                } else {
                    /* we might have put the artist in the title space,
                       fix this now (see artist) */
                    track->artist = track->title;
                    track->title = NULL;
                    /* both artist and title of track are specified */
                    buf = cddb_regex_get_string(line, matches, 3);
                    cddb_track_append_artist(track, buf);
                    free(buf);
                    buf = cddb_regex_get_string(line, matches, 4);
                    cddb_track_append_title(track, buf);
                    free(buf);
                    /* we should only get title continuations now */
                    multi_line = MULTI_TITLE;
                }
            } else {
                /* we're done parsing */
                state = STATE_STOP;
            }
            break;
        }
    }

    if (cache_content) {
        cddb_cache_close(c);
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
    char *msg;
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
    char *msg, *line;
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

    if (cddb_cache_query(c, disc)) {
        /* cached version found */
        return TRUE;
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
    case 210:                   /* found exact matches, list follows */
    case 211:                   /* found inexact matches, list follows */
        dlog("\t(in)exact matches");
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

/* Appends some data to the buffer.  The first parameter is the
   number of bytes that will be added.  The other parameters are a
   format string and its arguments as in printf. */
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
    if (disc->year != 0) {
        CDDB_WRITE_APPEND(7+4, "DYEAR=%d\n", disc->year);
    } else {
        CDDB_WRITE_APPEND(7, "DYEAR=\n");
    }
    CDDB_WRITE_APPEND(8+strlen(CDDB_CATEGORY[disc->category]),
                      "DGENRE=%s\n", 
                      (disc->genre ? disc->genre : CDDB_CATEGORY[disc->category]));
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
    char *msg;
    int code, size;
    cddb_track_t *track;
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
            cddb_cache_open(c, disc, "w");
            fwrite(buf, sizeof(char), size, cddb_cache_file(c));
            cddb_cache_close(c);
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
    sock_fwrite(buf, sizeof(char), size, c->socket);
    if (c->is_http_enabled) {
        /* skip HTTP response headers */
        cddb_http_parse_headers(c);
    } else {
        /* send terminating marker */
        sock_fprintf(c->socket, ".\n");
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
