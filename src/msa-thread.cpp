#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <thread>
#include <omp.h>
#include <iostream>
#include "matrix.h"

using namespace std;

#define _PRINT_INFO 

struct result {
	int top;
	int bottom;
	int left;
	int right;
	int sum;

	result(int t, int b, int l, int r, int s)
	{
		top = t;
		bottom = b;
		left = l;
		right = r;
		sum = s;
	}

};

struct task {
	int startR;
	int endR;
	int startC;
	int endC;

	task(int s1, int s2, int e1, int e2)
	{
		startR = s1;
		startC = s2;
		endR = e1;
		endC = e2;
	}

};

void printMatrix(Matrix*);
long get_usecs (void);
void usage(const char*);
void clear(int*, int);
Matrix* getPrefixSumMatrix(Matrix* m);
void doWork(int, task*, result**); // id, task, result_submission_list

int numthreads;
Matrix* mat;
Matrix* ps;

int main(int argc, char* argv[]) {

    if(argc < 2) {
        usage(argv[0]);
    }

//#Create matrix using filename
    mat = new Matrix(argv[1]); 
	#ifdef _PRINT_INFO 
	    cout << "Input matrix \n";
	    printMatrix(mat);
	#endif

//#Check and set thread num
    if(argc > 2)
    {
	    numthreads = atoi(argv[2]);
    }
    else
    {
    	numthreads = omp_get_num_procs();
    }

//#Check that parameters are valid
    if(numthreads < 1)
	{
		usage(argv[0]);
	}

//#Start timer
    long alg_start = get_usecs();
//#Compute vertical prefix sum
    ps = getPrefixSumMatrix(mat);

	#ifdef _PRINT_INFO
	    cout << "Vertical PrefixMatrix \n";
	    printMatrix(ps);
	#endif
    
//#Start doing some work
    result** results = new result*[numthreads];
	int workercount = numthreads-1;

	if(workercount > 0)	// Start workers if workercount > 0
	{	
		task* tasks[numthreads];
		thread threads[numthreads];

		int chunksize = mat->getRows()/numthreads;
		cout << "chunksize is "<< chunksize << "\n";
	    for(int i = 0; i < workercount; i++)
	    {
	    	int start = i*chunksize;
	    	tasks[i] = new task(start, -1, start+chunksize, -1);
	    	threads[i] = thread(doWork, i, tasks[i], results);
	    }

	    // Main threads task must be altered
	    int start = workercount*chunksize;
	    tasks[workercount] = new task(start, -1, start+chunksize, -1);
	    doWork(workercount, tasks[workercount], results);

	    for(int i = 0; i < workercount; i++)
	    {
			threads[i].join();	    	
			delete tasks[i];
	    }
	}
	else
	{
		cout << "no. available rows is "<< mat->getRows() << "\n";
		task* myTask = new task(0, -1, mat->getRows(), -1); // startRow, startCol, endRow, endCol
		doWork(workercount, myTask, results);
		delete myTask;
	}

	cout << "\nResults in pipeline \n";
	for(int i = 0; i < numthreads; i++)
	{
		cout << "(top, down, left, right) = " << " (" << results[i]->top << ", " << results[i]->bottom << ", " << results[i]->left << ", " << results[i]->right << ") \n";
	}
	cout << "\n";
//#Find result
	result* final_result = results[workercount];
	cout << "Current winning result: (top, down, left, right) = " << " (" << final_result->top << ", " << final_result->bottom << ", " << final_result->left << ", " << final_result->right << ") \n";
	for(int i = 1; i < workercount; i++)
	{
		if(final_result->sum < results[i]->sum)
		{
			final_result = results[i];
			cout << "Current winning result: (top, down, left, right) = " << " (" << final_result->top << ", " << final_result->bottom << ", " << final_result->left << ", " << final_result->right << ") \n";
		}
	}

	int bottom = final_result->bottom;
	int top = final_result->top;
	int right = final_result->right;
	int left = final_result->left;
	int max_sum = final_result->sum;

//#Compose the output matrix
    int outmat_row_dim = bottom - top + 1;
    int outmat_col_dim = right - left + 1;
    Matrix* outmat = new Matrix(outmat_row_dim, outmat_col_dim);

    for(int i=top, k=0; i<=bottom; i++, k++) {
        for(int j=left, l=0; j<=right ; j++, l++) {
            outmat->set(k, l, mat->get(i,j));
        }
    }
    long alg_end = get_usecs();

    // Print output matrix
	#ifdef _PRINT_INFO
	    cout << "Submatrix found" << "\n";
	    printMatrix(outmat);
	#endif


    printf("Sub-matrix [%dX%d] with max sum = %d, top = %d, bottom = %d, left = %d, right = %d\n", outmat_row_dim, outmat_col_dim, max_sum, top, bottom, left, right);
    // Print stats
    printf("%s,arg(%s),%s,%f sec\n", argv[0], argv[1], "CHECK_NOT_PERFORMED", ((double)(alg_end-alg_start))/1000000);

    // Release resources
    delete mat;
    delete ps;
    delete outmat;
    for(int i = 0; i < numthreads; i++)
    {
    	delete results[i];
    }
    delete results;

    return 0;
}

// Task that finds largest region restricted to the a certain Task (submatrix)
void doWork(int id, task* t, result** result_submission)
{
	int start = t->startR;

    int max_sum = mat->get(0,0);
    int top = 0, left = 0, bottom = 0, right = 0; 

    int dimR = mat->getRows();
    int dimC = mat->getCols();

    //Auxilliary variables 
    int sum[dimR];
    int pos[dimR];
    int local_max;

    result* res = new result(0,0,0,0,0);
    printf("Thread-%i: Searching from rows: %i \n", id, start);
    for (int i=start; i < dimR; i++) {
        for (int k=i; k < dimR; k++) {
            // Kandane over all columns with the i..k rows
            clear(sum, dimR);
            clear(pos, dimR);
            local_max = 0;

            // We keep track of the position of the max value over each Kandane's execution
            // Notice that we do not keep track of the max value, but only its position
            sum[0] = ps->get(k,0) - (i==0 ? 0 : ps->get(i-1,0));
            for (int j=1; j<dimC; j++) {
                if (sum[j-1] > 0){
                    sum[j] = sum[j-1] + ps->get(k,j) - (i==0 ? 0 : ps->get(i-1, j));
                    pos[j] = pos[j-1];
                } 
                else {
                    sum[j] = ps->get(k,j) - (i==0 ? 0 : ps->get(i-1, j));
                    pos[j] = j;
                }
                if (sum[j] > sum[local_max]){
                    local_max = j;
                }
            } //Kandane ends here

            if (sum[local_max] > res->sum) {
                // sum[local_max] is the new max value
                // the corresponding submatrix goes from rows i..k.
                // and from columns pos[local_max]..local_max

                res->sum = sum[local_max];
                res->top = i;
                res->left = pos[local_max];
                res->bottom = k;
                res->right = local_max;
                printf("Thread-%i: Found new values: top-%i, down-%i, left-%i, right-%i, sum-%i\n", id, i, k, pos[local_max], local_max, sum[local_max]);
                printf("Thread-%i: Found new values: top-%i, down-%i, left-%i, right-%i, sum-%i\n", id, res->top, res->bottom, res->left, res->right, res->sum);
            }
        }
    }
    printf("Thread-%i: Results found are: top->%i, down->%i, left->%i, right->%i, sum->%i \n", id, res->top, res->bottom, res->left, res->right, res->sum);
    result_submission[id] = res;
}

// Algorithm based on information obtained here:
// http://stackoverflow.com/questions/2643908/getting-the-submatrix-with-maximum-sum
Matrix* getPrefixSumMatrix(Matrix* m)
{
    Matrix* p = new Matrix(m->getRows(), m->getCols());

    for (int j=0; j<p->getCols(); j++) {
        p->set(0,j, m->get(0,j));
        for (int i=1; i<p->getRows(); i++) {
            p->set(i,j, (p->get(i-1,j) + m->get(i,j)));
        }
    }
    return p;
}

void printMatrix(Matrix* matrix)
{
	int r = matrix->getRows();
	int c = matrix->getCols();
	for(int i = 0; i < r; i++)
	{
		for(int j = 0; j < c; j++)
		{
			cout << matrix->get(i, j) << "\t";
		}
		cout << "\n";
	}
	cout << "\n";
}

long get_usecs (void)
{
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec*1000000+t.tv_usec;
}

void usage(const char* app_name) {
    printf("Argument error! Usage: %s <input_file> <num_threads> \n", app_name);
    exit(0);
}

void clear(int* a, int len) {
    for (int index=0; index<len; index++) {
        *(a+index) = 0;
    }
}