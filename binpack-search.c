#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "binpack.h"

#define BP_DRDSWP 10 // Default random swaps for VNS

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

static bool first_imp_nh1( binpack_solution_t * s );
static bool first_imp_nh2( binpack_solution_t * s );
static bool first_imp_nh3( binpack_solution_t * s );

static inline void shuffle( size_t n, size_t v[] );

static void open_bin( binpack_solution_t * s );
static void random_swaps( binpack_solution_t * s, unsigned int p );

/* return number of iteration -1 ( 0 if no improvement were found ) */
static long int hc_run( binpack_t * bp ) {
	bool imp = false; long int i = 0;
	do {
		i += imp = first_imp_nh1(bp->best);
	} while (imp);
	return i;
}

/* return number of iteration -1 ( 0 if no improvement were found ) */
static long int vnd_run( binpack_t * bp ) {
	bool imp = false; long int i = 0;
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

/* return number of iterations of the descent method and 0 if the solution was already optimal */
static long int vns_run( binpack_t * bp ) {
	long int i = 0;
	binpack_solution_t * best = binpack_get_best(bp),
					   * curr = bp->best;
	double eval, best_eval = binpack_solution_eval(best),
		   mean = (double) bp->sum / best->size;
	unsigned short nbhd = 1,
				   count = 0,
				   nc = bp->parameters.max_iter/4;
	size_t lb = binpack_lowerbound(bp);
	if (lb < best->size) {
		for (i = 0; i < bp->parameters.max_iter; ++i) {
			if (nbhd == 1) {
				open_bin(curr);
			} else if (nbhd == 2) {
				random_swaps(curr, BP_DRDSWP);
			} else if (nbhd == 3) {
				random_swaps(curr, 2*BP_DRDSWP);
			} else // if (nbhd == 4)
				random_swaps(curr, 3*BP_DRDSWP);

			i += vnd_run(bp) +1;
			eval = binpack_solution_eval(curr);
			if (curr->size < best->size ||
					(curr->size == best->size &&
					 eval - best_eval > BP_MINIMP /* * mean*mean */) ) {
				binpack_solution_copy(best, curr);
				nbhd = 1; count = 0;
				best_eval = eval;
				if (lb == best->size) break;
			} else {
				binpack_solution_copy(curr, best);
				count++;
			}

			if (count > nc) { nbhd++; count = 0; }
		}
		binpack_solution_copy(curr, best);
	}
	binpack_solution_destroy(best);
	return i;
}

static inline double std_diff( double mean, int before, int after ) {
	return (after - mean)*(after - mean) - (before - mean)*(before - mean);
}

static bool first_imp_nh1( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t * bin_of = s->bin_of;
	size_t i, j, bi;
	int * w = s->env->w;
	double mean = (double) s->env->sum / s->size;
	double imp;

	for (j = 0; j < s->size; j++) {
		for (i = 0; i < s->env->n; ++i) {
			bi = bin_of[i];
			if (bi != j && bin[j].load + w[i] <= s->env->c) {
				imp = std_diff(mean,	bin[bi].load /*before*/,
										bin[bi].load - w[i] /*after*/ );
				imp += std_diff(mean,	bin[j].load /*before*/,
										bin[j].load + w[i] /*after*/ );
				if (imp > BP_MINIMP /* * mean*mean */) {
					binpack_solution_remove(s, i);
					binpack_solution_add(s, i, j);
					assert(binpack_is_feasible(s));
					return true;
				}
			}
		}
	}
	return false;
}

static bool first_imp_nh2( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t bi, bj, i, j, ii, ij;
	int * w = s->env->w;
	double mean = (double) s->env->sum / s->size;
	double imp;

	for (bi = 0; bi < s->size ; bi++) {
		for (bj = bi+1; bj < s->size; bj++) {
			for (ii = 0; ii < bin[bi].size ; ii++) {
				i = bin[bi].items[ii];
				for (ij = 0; ij < bin[bj].size; ij++) {
					j = bin[bj].items[ij];
					if (bin[bi].load - w[i] + w[j] <= s->env->c &&
						bin[bj].load + w[i] - w[j] <= s->env->c) {
						imp = std_diff(mean,  bin[bi].load /*before*/,
											  bin[bi].load - w[i] + w[j]);
						imp += std_diff(mean, bin[bj].load /*before*/,
											  bin[bj].load + w[i] - w[j]);
						if (imp > BP_MINIMP /* * mean*mean */) {
							binpack_solution_swap(s, i, j);
							assert(binpack_is_feasible(s));
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

static bool first_imp_nh3( binpack_solution_t * s ) {
	binpack_bin_t * bin = s->bins;
	size_t * bin_of = s->bin_of;
	size_t i, j, k, ij, ik, b, bi;
	int * w = s->env->w;
	double mean = (double) s->env->sum / s->size;
	double imp;

	for (i = 0; i < s->env->n; ++i) {
		bi = bin_of[i];
		for (b = 0; b < s->size ; ++b) {
			if (b != bi) {
				for (ij = 0; ij < bin[b].size; ++ij) {
					j = bin[b].items[ij];
					for (ik = ij+1; ik < bin[b].size; ++ik) {
						k = bin[b].items[ik];
						if (bin[bi].load - w[i]+w[j]+w[k] <= s->env->c &&
								bin[b].load + w[i]-w[j]+w[k] <= s->env->c) {
							imp = std_diff(mean,  bin[bi].load /*before*/,
												  bin[bi].load -w[i]+w[j]+w[k]);
							imp += std_diff(mean, bin[b].load /*before*/,
												  bin[b].load +w[i]-w[j]-w[k]);
							if (imp > BP_MINIMP /* * mean*mean */) {
								binpack_solution_remove(s,k);
								binpack_solution_swap(s, i, j);
								binpack_solution_add(s, k, bi);
								assert(binpack_is_feasible(s));
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

static inline void shuffle( size_t n, size_t v[] ) {
	size_t k; size_t temp;
	for (size_t i = 1; i < n; ++i) {
		k = rand() % (i+1);
		temp = v[i];
		v[i] = v[k];
		v[k] = temp;
	}
}

static void open_bin( binpack_solution_t * s ) {
	size_t n = s->env->n, c = s->env->c;
	size_t index[n];
	int * w = s->env->w;
	for (int i = 0; i < n; ++i) index[i] = i;
	shuffle(n, index);

	size_t new = s->size;
	binpack_solution_remove(s,index[0]);
	binpack_solution_add(s,index[0],new);
	for (int i = 1; i < n; ++i) {
		if (s->bins[new].load + w[index[i]] <= c) {
			binpack_solution_remove(s,index[i]);
			binpack_solution_add(s,index[i],new);
		}
	}
	assert(binpack_is_feasible(s));
}

static void random_swaps( binpack_solution_t * s, unsigned int p ) {
	size_t a, b, ba, bb;
	int * w = s->env->w, c = s->env->c, n = s->env->n;
	for (unsigned i = 0; i < p; ++i) {
		do {
			a = rand() % n;
			b = rand() % n;
			ba = s->bin_of[a];
			bb = s->bin_of[b];
		} while (ba == bb ||
				s->bins[ba].load - w[a] + w[b] > c ||
				s->bins[bb].load + w[a] - w[b] > c );
		binpack_solution_swap(s, a, b);
	}
	assert(binpack_is_feasible(s));
}

