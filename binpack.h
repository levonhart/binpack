#ifndef BINPACK_H_DJA8I3W6
#define BINPACK_H_DJA8I3W6

#include <stdint.h>
#define BP_EINVAL -0x01ul // 0xffffffffffffffff for uint64_t, -1 for signed int
#define BP_NOTSET -0x02ul // 0xfffffffffffffffe for uint64_t, -2 for signed int
#define BP_BUFSIZ 512
#define BP_DMETHD VNS
#define BP_MINIMP 0.0002
#define BP_MAXITR 4
#define BP_DRDSWP 10 // Default random swaps for VNS

typedef enum _search {
	HC,
	VND,
	RMS,
	ILS,
	VNS
} search_t;

typedef struct _search_param {
		unsigned short max_iter;
		union {
			struct { // ILS
				int k;
			};
		};
} search_param_t;

typedef struct _binpack {
	int c;
	int * w;
	size_t n;
	long int sum;
	struct _binpack_solution * best;

	search_t method;
	search_param_t parameters; /* search parameters */
	// size_t * index;
} binpack_t;

binpack_t * binpack_create( const size_t c, const size_t n, const int w[] );

binpack_t * binpack_read( const char * path );

void binpack_destroy( binpack_t * bp );

inline binpack_t * binpack_copy( const binpack_t * bp ){
	binpack_t * copy = binpack_create( bp->c, bp->n, bp->w );
	return copy;
}

inline int binpack_lowerbound(const binpack_t * bp){
	return (bp->sum % bp->c) == 0 ?
			bp->sum / bp->c :
			bp->sum / bp->c + 1;
}

char * binpack_str( const binpack_t * bp );


typedef struct _binpack_bin {
	size_t * items;
	int load;
	size_t size;
	size_t _max_size;
} binpack_bin_t;

typedef struct _binpack_solution {
	const binpack_t * env;
	binpack_bin_t * bins;
	size_t * bin_of;
	size_t size;
	size_t _max_size;
} binpack_solution_t;

binpack_solution_t * binpack_solution_create( const binpack_t * bp );

void binpack_solution_destroy( binpack_solution_t * s );

/* return bin index on success and BP_EINVAL on failure */
size_t binpack_solution_add( binpack_solution_t * s,
									const size_t i,
									const size_t b );

/* return index of bin where it was found on success and EP_EINVAL on failure */
size_t binpack_solution_remove( binpack_solution_t * s, const size_t i );

void binpack_solution_reset( binpack_solution_t * s );

binpack_solution_t * binpack_solution_copy( binpack_solution_t * restrict dest,
											const binpack_solution_t * restrict s );

/* swap bins of items a and b */
void binpack_solution_swap( binpack_solution_t * s, size_t a, size_t b );

/* return the standard deviation of the bins load */
double binpack_solution_eval( const binpack_solution_t * s );

_Bool binpack_is_feasible( const binpack_solution_t * s );

char * binpack_solution_str( const binpack_solution_t * s );

inline void binpack_reset( binpack_t * bp ) {
	if (bp != NULL && bp->best != NULL) binpack_solution_destroy( bp->best );
	bp->best = NULL;
}

size_t binpack_set_start( binpack_t * restrict bp, binpack_solution_t * restrict s);

binpack_solution_t * binpack_get_best( binpack_t * bp );

inline void binpack_copy_best( const binpack_t * bp, binpack_solution_t * dest) {
	if (bp != NULL && dest != NULL)
		binpack_solution_copy(dest,bp->best);
}

binpack_solution_t * binpack_trivial( const binpack_t* bp );

binpack_solution_t * binpack_firstfit( const binpack_t * bp );

binpack_solution_t * binpack_firstfit_order( const binpack_t * restrict bp, size_t order[] );

long int binpack_search( binpack_t * bp, search_t method );

inline long int binpack_solve( binpack_t * bp ) {
	return binpack_search( bp, bp->method );
}

#endif /* end of include guard: BINPACK_H_DJA8I3W6 */
