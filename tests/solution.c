#include <stdio.h>
#include <assert.h>
#include "binpack.h"

int main(int argc, char *argv[]){
	int c = 1000;
	int n = 60;
	int items[] = { 495, 474, 473, 472, 466, 450, 445, 444, 439, 430, 419, 414,
					410, 395, 372, 370, 366, 366, 366, 363, 361, 357, 355, 351,
					350, 350, 347, 320, 315, 307, 303, 299, 298, 298, 292, 288,
					287, 283, 275, 275, 274, 273, 273, 272, 272, 271, 269, 269,
					268, 263, 262, 261, 259, 258, 255, 254, 252, 252, 252, 251 };
	double std2 = 0, avg = 0;
	size_t order[n];
	for (size_t i = 0; i < n; ++i) {
		order[i] = (n-1) - i;
		avg += items[i];
		std2 += items[i]*items[i];
	}
	std2 /= 1.0*n;
	avg /= 1.0*n;
	std2 -= avg*avg;

	binpack_t * bp = binpack_create(c,n,items);

	binpack_solution_t * s = binpack_solution_create(bp);
	if (s == NULL) {
		perror("Create solution failed");
		return 1;
	}
	binpack_solution_trivial(s);

	char * str = binpack_solution_str(s);
	printf("Trivial solution:\n%s", str);
	assert(binpack_solution_feasible(s));
	double eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (err:%.2lf)\n\n", eval, eval-n*std2);

	if (eval - n*std2 < 0.01) {
		fprintf(stderr, "Evaluation function broken\n");
	}



	for (int i = 0; i < n; ++i) {
		printf("%zu  ",s->bin_of[i]);
	}
	printf("\n");

	binpack_solution_swap(s,0,n-1);
	printf("Swap items 0 and n-1:\n%s\n", binpack_solution_str(s));

	binpack_solution_reset(s);

	printf("Empty solution:\n%zu {}\narray capacity: %zu\n", s->size, s->_max_size);
	if (binpack_solution_feasible(s)) {
		fprintf(stderr, "Feasibility test broken");
		return 2;
	}

	binpack_solution_firstfit(s);
	char * str2 = binpack_solution_str(s);
	printf("First fit solution (ascendent):\n%s", str2);
	assert(binpack_solution_feasible(s));
	eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (imp:%.2lf)\n\n",eval, eval-n*std2);

	binpack_solution_firstfit_order(s,order);
	char * str3 = binpack_solution_str(s);
	printf("First fit solution (descendent):\n%s", str3);
	assert(binpack_solution_feasible(s));
	eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (imp:%.2lf)\n\n",eval, eval-n*std2);

	binpack_solution_destroy(s);
	binpack_destroy(bp);
	return 0;
}
