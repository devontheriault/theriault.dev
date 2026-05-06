#include "storage/db.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    if (db_init() != 0) {
        fprintf(stderr, "[main] Failed to initialise database. Aborting.\n");
        return EXIT_FAILURE;
    }

    server_run(8080);

    db_close();
    return EXIT_SUCCESS;
}
