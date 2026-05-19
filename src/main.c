#include "storage/db.h"
#include "utils/secret.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    if (db_init() != 0) {
        fprintf(stderr, "[main] Failed to initialise database. Aborting.\n");
        return EXIT_FAILURE;
    }

    if (secret_init() != 0) {
        fprintf(stderr, "[main] Failed to initialise visitor salt. Aborting.\n");
        return EXIT_FAILURE;
    }

    server_run(8080);

    db_close();
    return EXIT_SUCCESS;
}
