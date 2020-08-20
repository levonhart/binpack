#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "binpack.h"

#ifdef _WIN32
	#define snprintf_s snprintf
#endif

static inline size_t resize_str( char ** str, size_t size, size_t length ) {
	char * temp = *str;
	*str = (char *) malloc(size * sizeof(char));
	strncpy_s( *str, size, temp, length);
	free(temp);
	return size;
}

binpack_t * binpack_create( const size_t c,
							const size_t n,
							const int    w[] ) {
	if (w == NULL) return NULL;
	binpack_t * bp = (binpack_t *) malloc(sizeof(binpack_t));
	bp->c = c;
	bp->n = n;
	bp->sum = 0;
	bp->method = ILS;
	bp->w = (int *) malloc(n * sizeof(int));
	for (size_t i = 0; i < n; ++i) {
		bp->w[i] = w[i];
		bp->sum += w[i];
	}
	return bp;
}

binpack_t * binpack_read(const char * path){
	FILE * file;
	fopen_s(&file, path, "r");
	if (file == NULL) {
		perror("Unable to read file");
		return NULL;
	}

	binpack_t * bp = (binpack_t *) malloc(sizeof(binpack_t));
	if (fscanf_s(file, "%zu %zu", &bp->n, &bp->c) != 2){
		perror("Wrong file format"); binpack_destroy(bp);
		return NULL;
	}

	if (bp->n == 0 || bp->c == 0) {
		fprintf(stderr, "Capacity and number of items must be positive integers\n");
		binpack_destroy(bp); return NULL;
	}

	bp->method = ILS;
	bp->sum = 0;
	bp->w = (int *) malloc(bp->n * sizeof(int));

	for (size_t i = 0; i < bp->n; ++i) {
		if (fscanf_s(file, "%d", &(bp->w)[i]) == EOF) {
			perror("Inconsistent number of items");
			binpack_destroy(bp); return NULL;
		}
		bp->sum += (bp->w)[i];
	}
	return bp;
}

void binpack_destroy(binpack_t * bp){
	if (bp != NULL) {
		if (bp->w != NULL)
			free(bp->w);
		free(bp);
	}
}

inline int binpack_lowerbound(const binpack_t * restrict bp){
	return (bp->sum % bp->c) == 0 ?
			bp->sum / bp->c :
			bp->sum / bp->c + 1;
}


/* ======================================================================== */
/* ======================== convert to string routines ==================== */
/* ======================================================================== */

char * binpack_str(const binpack_t * restrict bp){
	size_t size = BP_BUFSIZ;
	size_t length = 0;
	char * str = (char *) malloc( size*sizeof(char) );

	length += snprintf_s( str, size,"c   = %d\n"
									"sum = %ld\n"
									"w[%zu]= { ", bp->c, bp->sum, bp->n );
	for (size_t i = 0; i < bp->n-1; ++i) {
		if ( 4 * length > 3 * size ) size = resize_str(&str, 3 * size, length);
		length += snprintf_s( str + length, size - length, "%d, ", bp->w[i]);
	}
	snprintf_s( str + length, size - length,
							"%d }\n"
							"LB = %d\n",
							bp->w[bp->n-1], binpack_lowerbound(bp));
	return str;
}

static char * binpack_bin_str( const binpack_bin_t * restrict bin ){
	size_t size = BP_BUFSIZ;
	size_t length = 0;
	char * str = (char *) malloc( size*sizeof(char) );

	length += snprintf_s( str, size, "%d:[", bin->load );
	for (size_t i = 0; i < bin->size-1; ++i) {
		if ( 4 * length > 3 * size ) size = resize_str(&str, 3 * size, length);
		length += snprintf_s( str + length, size - length, "%zu ", bin->items[i]);
	}
	snprintf_s( str + length, size - length, "%zu]",
							bin->items[bin->size-1]);
	return str;
}

char * binpack_solution_str( const binpack_solution_t * restrict s ){
	size_t size = BP_BUFSIZ;
	size_t length = 0;
	char * str = (char *) malloc( size*sizeof(char) );
	char * temp;
	size_t templength;

	length += snprintf_s( str, size, "%zu {", s->size );
	for (int i = 0; i < s->size-1; ++i) {
		temp = binpack_bin_str( s->bins +i ); templength = strlen(temp);
		if (length + templength + 3 > size-1) /* 3 := strlen(",  ") */
			size = resize_str(&str, 2 * (length+templength),length);
		if ( 4 * length > 3 * size ) size = resize_str(&str,3*size,length);
		/* strcat_s( str + length, size - length, temp ); */
		/* length += templength; */
		length += snprintf_s( str + length, size - length, "%s,  ", temp );
	}
	temp = binpack_bin_str( s->bins + (s->size-1) ); templength = strlen(temp);
	if (length + templength + 3 > size-1) /* 3 := strlen(" }\n") */
		size = resize_str(&str, (length+templength+3),length);
	snprintf_s( str + length, size - length, "%s }\n", temp);
	return str;
}
