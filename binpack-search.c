#include <stdlib.h>
#include <assert.h>
#include "binpack.h"

static binpack_solution_t * hc_run( binpack_t * bp );
static binpack_solution_t * vnd_run( binpack_t * bp );
static binpack_solution_t * ils_run( binpack_t * bp );
static binpack_solution_t * rms_run( binpack_t * bp );
static binpack_solution_t * vns_run( binpack_t * bp );

static binpack_solution_t * (* const search[])( binpack_t * ) = {
			hc_run, vnd_run, ils_run, rms_run, vns_run };

binpack_solution_t * binpack_search( binpack_t * bp ) {
	if (bp == NULL) return NULL;
	if (bp->best == NULL) bp->best = binpack_firstfit(bp);
	else if (!binpack_is_feasible(bp->best)) return NULL;
	return search[bp->method](bp);
}

static binpack_solution_t * hc_run( binpack_t * bp ) {}
static binpack_solution_t * vnd_run( binpack_t * bp ) {}
static binpack_solution_t * ils_run( binpack_t * bp ) {}
static binpack_solution_t * rms_run( binpack_t * bp ) {}
static binpack_solution_t * vns_run( binpack_t * bp ) {}

