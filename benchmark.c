#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <binpack.h>
#include <tinydir.h>
#include <list.h>

typedef struct {
	search_t t;
	search_param_t p;
} solver_t;

typedef struct _binpack_list {
	binpack_t data;
	struct list_head list;
} binpack_list_t;

const char * search_dict[] = {
		"HC", "VND", "RMS", "ILS", "VNS", "ERROR"
	};
const unsigned sdict_length = 5;

void benchmark(const char * path, const solver_t solver[], const size_t n);

int main(int argc, char *argv[]){
	if (argc <= 1) {
		printf("usage: benchmark <path>\n\n"
				"   <path>        Path to directory with instances of "
				"the bin packing problem\n");
		exit(-1);
	}


	size_t n_solvers = 3;
	solver_t solvers[] = {
			{ .t = HC, .p = { 0 }},
			{ .t = VND, .p = { 0 }},
			{ .t = VNS, .p = { .max_iter = 10000 }}
		};

	srand(4321);
	benchmark(argv[1], solvers, n_solvers);
	return 0;
}


void benchmark(const char * path, const solver_t solver[], const size_t n) {
	if (!path) return;

	tinydir_dir dir;
	if (tinydir_open(&dir, path) != 0) {
		perror("Directory Reading Error");
		return;
	}

	/* Read files and add instances found to list */
	LIST_HEAD(instances);
	size_t n_instances = 0;
	while (dir.has_next) {
		tinydir_file file;
		binpack_t * env;

		tinydir_readfile(&dir,&file);

		if (file.is_reg){
			env = binpack_read(file.path);
			if (env != NULL) {
				binpack_list_t * node =
					(binpack_list_t *) malloc(sizeof(binpack_list_t));

				printf("Instance found: %s\n", file.name);
				node->data = *env;
				free(env);
				list_add_tail(&node->list, &instances);
				n_instances++;
			}
		}
		tinydir_next(&dir);
	}
	tinydir_close(&dir);

	if (n_instances == 0) {
		printf("No Instance was found in:\n   %s\n",path);
		return;
	} else { printf("%zu Instances found\n", n_instances); }

	/* Solve instances */
	struct list_head * iter, * temp;
	for (size_t i = 0; i < n; ++i) {
		size_t sum_bins;
		clock_t tick;

		sum_bins = 0;
		tick = clock();

		size_t j = 0;
		list_for_each(iter, &instances) {
			binpack_t * env;
			env = (binpack_t *) list_entry(iter, binpack_list_t, list);
			binpack_reset(env);
			j++;

			env->parameters = solver[i].p;
			binpack_search(env, solver[i].t);

			printf("\rSolver %zu: Solving... %3zu%% [%3zu/%3zu] completed",
					i, j*100/n_instances, j, n_instances);
			fflush(stdout);
			sum_bins += env->best->size;
		}
		printf("\rSolver %zu (method: %-3s) - mean: %lf   "
				"execution time: % .5lf secs\n", i,
				search_dict[solver[i].t],
				((double) sum_bins) / n_instances,
				((double) (clock() - tick)) / CLOCKS_PER_SEC);
	}

	/* Destroy list of instances */
	list_for_each_safe(iter, temp, &instances) {
		binpack_destroy((binpack_t*)list_entry(iter,binpack_list_t,list));
	}
}
