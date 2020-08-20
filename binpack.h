#ifndef BINPACK_H_DJA8I3W6
#define BINPACK_H_DJA8I3W6

#include <stdint.h>
#define BP_EINVAL ~((size_t)0x00)
#define BP_NOTSET ~((size_t)0x01)
#define BP_BUFSIZ 512

typedef enum _search {
	HC,
	VND,
	RMS,
	ILS
} search_t;

typedef struct _binpack {
	int c;
	int * w;
	size_t n;
	long int sum;
	search_t method;
} binpack_t;

binpack_t * binpack_create( const size_t c, const size_t n, const int w[] );

binpack_t * binpack_read( const char * path );

void binpack_destroy( binpack_t * bp );

inline binpack_t * binpack_copy( const binpack_t * restrict bp ){
	binpack_t * copy = binpack_create( bp->c, bp->n, bp->w );
	return copy;
}


int binpack_lowerbound( const binpack_t * restrict bp );

char * binpack_str( const binpack_t * restrict bp );


typedef struct _binpack_bin {
	size_t * items;
	int load;
	size_t size;
	size_t _max_size;
} binpack_bin_t;

typedef struct _binpack_solution {
	const binpack_t * prob;
	binpack_bin_t * bins;
	size_t * bin_of;
	size_t size;
	size_t _max_size;
} binpack_solution_t;

binpack_solution_t * binpack_solution_create( const binpack_t * restrict bp );

void binpack_solution_destroy( binpack_solution_t * s );

/* return bin index on success and BP_EINVAL on failure */
size_t binpack_solution_add( binpack_solution_t * restrict s,
												const size_t i,
												const size_t b );

/* return index of bin where it was found on success and EP_EINVAL on failure */
size_t binpack_solution_remove( binpack_solution_t * restrict s,
												const size_t i );

void binpack_solution_reset( binpack_solution_t * s );

binpack_solution_t * binpack_solution_copy( binpack_solution_t * dest,
											const binpack_solution_t * restrict s );

/* swap bins of items a and b */
void binpack_solution_swap( binpack_solution_t * restrict s,
														size_t a,
														size_t b );

/* return the standard deviation of the bins load */
double binpack_solution_eval( const binpack_solution_t * restrict s );

_Bool binpack_solution_feasible( const binpack_solution_t * restrict s );

char * binpack_solution_str( const binpack_solution_t * restrict s );

void binpack_solution_trivial( binpack_solution_t * restrict s );

void binpack_firstfit( binpack_solution_t * restrict s );

void binpack_firstfit_order( binpack_solution_t * restrict s, size_t order[] );

#endif /* end of include guard: BINPACK_H_DJA8I3W6 */
