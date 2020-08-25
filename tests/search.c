#include <stdio.h>
#include <stdlib.h>
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
	bp->method = HC;

	int niter = binpack_solve(bp);
	assert(binpack_is_feasible(bp->best));
	binpack_solution_t * s = binpack_get_best(bp);
	if (s == NULL) {
		perror("Hillclimb failed");
		return 1;
	}

	assert(binpack_is_feasible(s));
	char * str = binpack_solution_str(s);
	printf("HC[%d] solution:\n%s", niter, str);
	double eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (err:%.2lf)\n\n", eval, eval-std2);

	binpack_solution_t * trivial = binpack_trivial(bp);

	binpack_reset(bp);
	binpack_set_start(bp, trivial);
	binpack_solution_destroy(trivial);

	niter = binpack_solve(bp);
	assert(binpack_is_feasible(bp->best));

	binpack_solution_destroy(s);
	s = binpack_get_best(bp);
	if (s == NULL) {
		perror("set_start failed");
		return 1;
	}
	assert(binpack_is_feasible(s));
	free(str); str = binpack_solution_str(s);
	printf("HC[%d] solution starting from trivial solution:\n%s", niter, str);
	eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (err:%.2lf)\n\n", eval, eval-std2);

	if (binpack_search(NULL,ILS) > 0) {
		perror("Wrong Value for BP_INVAL");
	}


	binpack_solution_t * ff = binpack_firstfit(bp);

	binpack_reset(bp);
	binpack_set_start(bp, ff);
	binpack_solution_destroy(ff);

	niter = binpack_search(bp, VND);
	assert(binpack_is_feasible(bp->best));

	binpack_solution_destroy(s);
	s = binpack_get_best(bp);
	assert(binpack_is_feasible(s));
	free(str); str = binpack_solution_str(s);
	printf("VND[%d] solution starting from trivial solution:\n%s", niter, str);
	eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (err:%.2lf)\n\n", eval, eval-std2);

	if (binpack_search(NULL,ILS) > 0) {
		perror("Wrong Value for BP_INVAL");
		return -1;
	}

	binpack_reset(bp);

	niter = binpack_search(bp, VNS);
	assert(binpack_is_feasible(bp->best));

	binpack_solution_destroy(s);
	s = binpack_get_best(bp);
	if(binpack_is_feasible(s) == 0){
		perror("VNS failed");
		return 5;
	}
	free(str); str = binpack_solution_str(s);
	printf("VNS[%d] solution:\n%s", niter, str);
	eval = binpack_solution_eval(s);
	printf("Evaluation: %.2lf (err:%.2lf)\n\n", eval, eval-std2);

	binpack_solution_destroy(s);
	binpack_destroy(bp);
	return 0;
}
