#include <stdio.h>
#include <assert.h>
#include "binpack.h"

int main(int argc, char *argv[]){
	if (argc<1) {
		fprintf(stderr, "Not enough arguments.\n"
						"usage: %s <path>", argv[0]);
		return 1;
	}

	printf("reading file: %s\n", argv[1]);
	binpack_t * bp = binpack_read(argv[1]);
	if (bp == NULL) {
		perror("Read file failed");
		return 2;
	}

	char * str = binpack_str(bp);
	if (str == NULL) {
		perror("Dump instance failed");
		return 3;
	}
	printf("bp:\n%s\n", str);

	binpack_t * bp2 = binpack_copy(bp);
	if (bp == NULL) {
		perror("Create context failed");
		return 2;
	}

	for (int i = 0; i < bp->n/2; ++i) {
		assert(bp->w[i] == bp2->w[i]);
	}

	char * str2 = binpack_str(bp2);
	if (str2 == NULL) {
		perror("Dump instance failed");
		return 3;
	}

	printf("A copy of bp:\n%s\n", str2);

	binpack_destroy(bp); binpack_destroy(bp2);
	return 0;
}
