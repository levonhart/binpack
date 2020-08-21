#include <stdlib.h>
#include <assert.h>
#include "binpack.h"

static long int hc_run( binpack_t * bp );
static long int vnd_run( binpack_t * bp );
static long int ils_run( binpack_t * bp );
static long int rms_run( binpack_t * bp );
static long int vns_run( binpack_t * bp );

static long int (* const search[])( binpack_t * ) = {
			hc_run, vnd_run, ils_run, rms_run, vns_run };

long int binpack_search( binpack_t * bp, search_t method ) {
	if (bp == NULL) return (long int) BP_EINVAL;
	if (bp->best == NULL) bp->best = binpack_firstfit(bp);
	else if (!binpack_is_feasible(bp->best)) return (long int) BP_EINVAL;
	return search[method](bp);
}

inline double std_diff( double mean, int before, int after ) {
	return (after - mean)*(after - mean) - (before - mean)*(before - mean);
}

static _Bool first_imp_nh1( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t * bin_of = s->bin_of;
	size_t i, j, bi;
	int * w = s->prob->w;
	double mean = (double) s->prob->sum / s->size;
	double imp;

	for (j = 0; j < s->size; j++) {
		for (i = 0; i < s->prob->n; ++i) {
			bi = bin_of[i];
			if (bi != j && bin[j].load + w[i] <= s->prob->c) {
				imp = std_diff(mean,	bin[bi].load /*before*/,
										bin[bi].load - w[i] /*after*/ );
				imp += std_diff(mean,	bin[j].load /*before*/,
										bin[j].load + w[i] /*after*/ );
				if (imp > BP_MINIMP * mean*mean) {
					binpack_solution_remove(s, i);
					binpack_solution_add(s, i, j);
					assert(binpack_is_feasible(s));
					return 1;
				}
			}
		}
	}
	return 0;
}

static _Bool first_imp_nh2( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t bi, bj, i, j, ii, ij;
	int * w = s->prob->w;
	double mean = (double) s->prob->sum / s->size;
	double imp;

	for (bi = 0; bi < s->size ; bi++) {
		for (bj = bi+1; bj < s->size; bj++) {
			for (ii = 0; ii < bin[bi].size ; ii++) {
				i = bin[bi].items[ii];
				for (ij = 0; ij < bin[bj].size; ij++) {
					j = bin[bj].items[ij];
					if (bin[bi].load - w[i] + w[j] <= s->prob->c &&
						bin[bj].load + w[i] - w[j] <= s->prob->c) {
						imp = std_diff(mean,  bin[bi].load /*before*/,
											  bin[bi].load - w[i] + w[j]);
						imp += std_diff(mean, bin[bj].load /*before*/,
											  bin[bj].load + w[i] - w[j]);
						if (imp > BP_MINIMP * mean*mean) {
							binpack_solution_swap(s, i, j);
							assert(binpack_is_feasible(s));
							return 1;
						}
					}
				}
			}
		}
	}
	return 0;
}

static _Bool first_imp_nh3( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t * bin_of = s->bin_of;
	size_t i, j, k, ij, ik, b, bi;
	int * w = s->prob->w;
	double mean = (double) s->prob->sum / s->size;
	double imp;

	for (i = 0; i < s->prob->n; ++i) {
		bi = bin_of[i];
		for (b = 0; b < s->size ; ++b) {
			if (b != bi) {
				for (ij = 0; ij < bin[b].size; ++ij) {
					j = bin[b].items[ij];
					for (ik = ij+1; ik < bin[b].size; ++ik) {
						k = bin[b].items[ik];
						if (bin[bi].load - w[i]+w[j]+w[k] <= s->prob->c &&
								bin[b].load + w[i]-w[j]+w[k] <= s->prob->c) {
							imp = std_diff(mean,  bin[bi].load /*before*/,
												  bin[bi].load -w[i]+w[j]+w[k]);
							imp += std_diff(mean, bin[b].load /*before*/,
												  bin[b].load +w[i]-w[j]-w[k]);
							if (imp > BP_MINIMP * mean*mean) {
								binpack_solution_remove(s,k);
								binpack_solution_swap(s, i, j);
								binpack_solution_add(s, k, bi);
								assert(binpack_is_feasible(s));
								return 1;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

static inline void shuffle( size_t n, size_t v[] ) {
	size_t k; size_t temp;
	for (size_t i = 1; i < n; ++i) {
		k = rand() % i;
		temp = v[i];
		v[i] = v[k];
		v[k] = v[i];
	}
}

/* return number of iteration -1 ( 0 if no improvement were found ) */
static long int hc_run( binpack_t * bp ) {
	_Bool imp = 0; long int i = 0;
	do {
		i += imp = first_imp_nh1(bp->best);
	} while (imp);
	return i;
}

/* return number of iteration -1 ( 0 if no improvement were found ) */
static long int vnd_run( binpack_t * bp ) {
	_Bool imp = 0; long int i = 0;
	do {
		i += imp = first_imp_nh1(bp->best);
		if (!imp) i += imp = first_imp_nh2(bp->best);
		if (!imp) i += imp = first_imp_nh3(bp->best);
	} while (imp);
	return i;
}

static long int ils_run( binpack_t * bp ) {
	return 0;
}

static long int rms_run( binpack_t * bp ) {
	return 0;
}

static long int vns_run( binpack_t * bp ) {
	return 0;
}
