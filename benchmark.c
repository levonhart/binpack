#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <binpack.h>
#include <tinydir.h>

typedef struct {
	search_t t;
	search_param_t p;
} solver_t;

void benchmark(const char * path, const solver_t solver[], const size_t n);

int main(int argc, char *argv[]){
	if (argc <= 1) {
		printf("usage: benchmark <path>\n\n"
				"   <path>\t\tPath to directory with instances of the bin packing problem\n");
		exit(-1);
	}


	benchmark(argv[1], NULL, 0);
	return 0;
}


void benchmark(const char * path, const solver_t solver[], const size_t n) {
	if (!path) return;

	tinydir_dir dir;
	if (tinydir_open(&dir, path) != 0) {
		perror("Directory Reading Error");
		return;
	}

	while (dir.has_next) {
		tinydir_file file;
		tinydir_readfile(&dir,&file);

		if (file.is_reg) {
			printf("Instance found: %s\n", file.name);
		}

		tinydir_next(&dir);
	}

}
