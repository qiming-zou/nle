
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nledl.h"

void nledl_init(nledl, obs) nle_ctx_t *nledl;
nle_obs *obs;
{
    nledl->dlhandle = dlopen(nledl->dlpath, RTLD_LAZY);

    if (!nledl->dlhandle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror(); /* Clear any existing error */

    void *(*start)(void *, nle_obs *);
    start = dlsym(nledl->dlhandle, "nle_start");
    nledl->nle_ctx = start(nledl->outfile, obs);

    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    nledl->step = dlsym(nledl->dlhandle, "nle_step");

    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
}

void nledl_close(nledl) nle_ctx_t *nledl;
{
    void (*end)(void *);

    end = dlsym(nledl->dlhandle, "nle_end");
    end(nledl->nle_ctx);

    if (dlclose(nledl->dlhandle)) {
        fprintf(stderr, "Error in dlclose: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();
}

nle_ctx_t *nle_start(dlpath, obs) const char *dlpath;
nle_obs *obs;
{
    struct nledl_ctx *nledl = malloc(sizeof(struct nledl_ctx));
    nledl->outfile = fopen("nle.ttyrec", "a");
    strncpy(nledl->dlpath, dlpath, sizeof(nledl->dlpath));

    nledl_init(nledl, obs);
    return nledl;
};

nle_ctx_t *nle_step(nledl, obs) nle_ctx_t *nledl;
nle_obs *obs;
{
    if (!nledl || !nledl->dlhandle || !nledl->nle_ctx) {
        fprintf(stderr, "Illegal nledl_ctx\n");
        exit(EXIT_FAILURE);
    }

    nledl->step(nledl->nle_ctx, obs);

    return nledl;
}

/* TODO: This doesn't close nethack cleanly at all. */
void nle_reset(nledl, obs) nle_ctx_t *nledl;
nle_obs *obs;
{
    nledl_close(nledl);
    nledl_init(nledl, obs);
}

void nle_end(nledl) nle_ctx_t *nledl;
{
    nledl_close(nledl);
    fclose(nledl->outfile);
    free(nledl);
}