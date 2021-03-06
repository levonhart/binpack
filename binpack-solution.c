#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "binpack.h"

static inline void binpack_bin_resize( binpack_bin_t * b, size_t size) {
	size_t * old = b->items;
	b->items = (size_t *) malloc( size * sizeof(size_t) );
	if (old != NULL) {
		for (int i = 0; i < b->size; ++i) {
			b->items[i] = old[i];
		}
		free(old);
	}
	b->_max_size = size;
}

/* The following two functions do not update the load of b */
static inline void binpack_bin_add( binpack_bin_t * b, const size_t i ){
	assert(b != NULL);
	if (b->items == NULL) {
		b->items = (size_t *) malloc(sizeof(size_t));
		b->_max_size = 1;
	}
	if (b->size == b->_max_size) binpack_bin_resize( b, (b->_max_size << 1) );
	b->items[b->size++] = i;
}

static inline void binpack_bin_remove( binpack_bin_t * b, const size_t i ) {
	assert(b != NULL && b->items != NULL);
	size_t j = 0;
	while (j < b->size && b->items[j] != i) j++;
	assert(j < b->size);
	b->items[j] = b->items[--b->size];
	if (b->size > 0 && b->size == b->_max_size/4)
		binpack_bin_resize(b, b->_max_size/2);
}

binpack_solution_t * binpack_solution_create( const binpack_t * bp ){
	binpack_solution_t * s =
		(binpack_solution_t *) malloc( sizeof(binpack_solution_t) );
	s->env = bp;
	s->bins = NULL;
	s->size = 0;
	s->_max_size = 0;

	s->bin_of = (size_t *) malloc( bp->n * sizeof(size_t) );
	for (size_t i = 0; i < bp->n; ++i) s->bin_of[i] = BP_NOTSET;

	return s;
}

void binpack_solution_destroy( binpack_solution_t * s ){
	if (s != NULL) {
		if (s->bins != NULL) {
			for (int i = 0; i < s->size; ++i) {
				if (s->bins[i].items != NULL)
					free(s->bins[i].items);
			}
			free(s->bins);
		}
		if (s->bin_of != NULL) {
			free(s->bin_of);
		}
		free(s);
	}
}

void inline binpack_solution_resize( binpack_solution_t * s, size_t size) {
	assert(size > s->_max_size);
	binpack_bin_t * old = s->bins;
	s->bins = (binpack_bin_t *) malloc( size * sizeof(binpack_bin_t) );
	if (old != NULL) {
		for (int i = 0; i < s->size; ++i) {
			s->bins[i] = old[i];
		}
		free(old);
	}
	s->_max_size = size;
}


size_t binpack_solution_add( binpack_solution_t * s, const size_t i,
													const size_t b ) {
	if (s->bin_of[i] != (BP_NOTSET)) return BP_EINVAL;
	size_t dest = b;

	if (s->bins == NULL) binpack_solution_resize(s, 1);
	if (b >= s->size) {
		if (s->size == s->_max_size)
			binpack_solution_resize(s, s->_max_size << 1);
		dest = s->size++;
		s->bins[dest] = (binpack_bin_t){NULL, 0, 0, 0};
	}

	binpack_bin_add(&s->bins[dest], i);
	s->bins[dest].load += s->env->w[i];
	s->bin_of[i] = dest;
	return dest;
}

size_t binpack_solution_remove( binpack_solution_t * s, const size_t i ) {
 	if (s->bin_of[i] == BP_NOTSET) return BP_EINVAL;
	binpack_bin_t * b = s->bins + s->bin_of[i]; // b= &s->bins[ s->bin_of[i] ];
	size_t k = s->bin_of[i];

	binpack_bin_remove(b, i);
	b->load -= s->env->w[i];
	s->bin_of[i] = BP_NOTSET;

	if (b->size == 0) {
		free(b->items);
		*b = s->bins[ -- s->size ];
		for (size_t j = 0; j < b->size; ++j) {
			s->bin_of[ b->items[j] ] = k;
		}
		k = BP_EINVAL;
	}

	return k;
}

void binpack_solution_reset( binpack_solution_t * s ) {
	if (s != NULL) {
		size_t i;
		if (s->bins != NULL) {
			for (i = 0; i < s->size; ++i) {
				if (s->bins[i].items != NULL)
					free(s->bins[i].items);
			}
			free(s->bins);
			s->bins = NULL;
		}

		for (i = 0; i < s->env->n; ++i) {
			s->bin_of[i] = BP_NOTSET;
		}
		s->size = 0; s->_max_size = 0;
	}
}

binpack_solution_t * binpack_solution_copy( binpack_solution_t * restrict dest,
									const binpack_solution_t * restrict s ) {
	if (s == NULL || dest == NULL) return NULL;
	binpack_solution_reset( dest );
	if (dest->env != s->env) return NULL;

	dest->bins = (binpack_bin_t *) calloc (s->size, sizeof(binpack_bin_t));
	dest->_max_size = s->size;

	for (size_t i = 0; i < s->size; ++i) {
		for (size_t j = 0; j < s->bins[i].size; ++j) {
			binpack_solution_add( dest, s->bins[i].items[j], i );
		}
	}

	return dest;
}

void binpack_solution_swap( binpack_solution_t * s, size_t a, size_t b ) {
	binpack_bin_t * ba = s->bins + s->bin_of[a];
	binpack_bin_t * bb = s->bins + s->bin_of[b];
	binpack_bin_remove(ba, a);
	binpack_bin_remove(bb, b);
	binpack_bin_add(ba, b);
	ba->load += s->env->w[b] - s->env->w[a];
	binpack_bin_add(bb, a);
	bb->load += s->env->w[a] - s->env->w[b];
	size_t temp = s->bin_of[a];
	s->bin_of[a] = s->bin_of[b];
	s->bin_of[b] = temp;
}

double binpack_solution_eval( const binpack_solution_t * s ) {
	double avg = (double) s->env->sum / s->size;
	double std = 0;
	for (size_t i = 0; i < s->size; ++i) {
		/* std += s->bins[i].load * s->bins[i].load; */
		double x = s->bins[i].load - avg;
		std =+ x * x;
	}
	/* std /= s->size; */
	/* std -= avg*avg; */
	/* std -= s->size * avg*avg; */
	return std;
}

bool binpack_is_feasible( const binpack_solution_t * s ) {
	binpack_bin_t * b;
	size_t count = false;
	for (size_t i = 0; i < s->size; ++i) {
		b = &s->bins[i];
		if (b->load > s->env->c) return false;
		for (size_t j = 0; j < b->size; ++j) {
			if (s->bin_of[ b->items[j] ] != i) return false;
			count++;
		}
	}
	if (count < s->env->n) return false;
	return true;
}


/* void binpack_solution_trivial( binpack_solution_t * restrict s ) { */
/*     binpack_solution_reset(s); */
/*     for (size_t i = 0; i < s->env->n; ++i) { */
/*         binpack_solution_add(s,i,i); */
/*     } */
/* } */
/*  */
/* void binpack_firstfit( binpack_solution_t * restrict s ) { */
/*     binpack_solution_reset(s); */
/*     for (size_t i = 0; i < s->env->n; ++i) { */
/*         size_t j = 0; */
/*         while (j < s->size && s->env->w[i] + s->bins[j].load > s->env->c) { */
/*                 j++; */
/*         } */
/*         binpack_solution_add(s,i,j); */
/*     } */
/* } */
/*  */
/* void binpack_firstfit_order( binpack_solution_t * restrict s, */
/*                                         size_t order[] ) { */
/*     binpack_solution_reset(s); */
/*     for (size_t k = 0; k < s->env->n; ++k) { */
/*         size_t i = order[k]; */
/*         size_t j = 0; */
/*         while (j < s->size && s->env->w[i] + s->bins[j].load > s->env->c) { */
/*                 j++; */
/*         } */
/*         binpack_solution_add(s,i,j); */
/*     } */
/* } */
