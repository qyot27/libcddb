
#include <cddb/cddb.h>


cddb_disc_t *do_read(cddb_conn_t *conn, const char *category, int discid);

void do_display(cddb_disc_t *disc);
