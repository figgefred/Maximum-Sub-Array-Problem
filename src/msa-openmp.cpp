#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>
#include <iostream>
#include "matrix.h"

using namespace std;

//#define _PRINT_INFO 

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
	int* rows;
	int count;
	task(int tasksize)
	{
		rows = new int[tasksize];
		count = tasksize;
	}
	~task()
	{
		delete rows;
	}
};

void printMatrix(Matrix*);
long get_usecs (void);
void usage(const char*);
void clear(int*, int);
void setPrefixSumMatrix();
void sumColumn(int, int);
void sumAllColumns();
void doWork(int, result* result); // id, task, result_submission_list
task** getTasks();

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

	if(numthreads > mat->getRows())
	{
		numthreads = mat->getRows();
		cout << "\n\n WARNING! More threads set then rows in matrice. Threads set to ROW COUNT: " << mat->getRows() << "\n\n";
	}
	else
	{
		cout << "\n\n Numthreads is set to: " << numthreads << "\n\n";	
	}


//#Start timer
    long alg_start = get_usecs();
//#Compute vertical prefix sum
    ps = new Matrix(mat->getRows(), mat->getCols());
    //int c = numthreads;
    setPrefixSumMatrix();
	#ifdef _PRINT_INFO
	    cout << "Vertical PrefixMatrix \n";
	    printMatrix(ps);
	#endif
	//numthreads = c;

//#Start doing some work
    omp_set_num_threads(numthreads);
    result* final_result = new result(0,0,0,0,0);
    doWork(0, final_result);

	#ifdef _PRINT_INFO
		cout << "(top, down, left, right) sum = " << " (" << final_result->top << ", " << final_result->bottom << ", " << final_result>left << ", " << final_result->right << ") " << final_result->sum << "\n";	
	#endif

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
    delete final_result;

    return 0;
}

task** getTasks()
{
	task** tasks = new task*[numthreads];
		

	// Init tasks
	int rBegin = 0;
	int rEnd = mat->getRows();
	int rowsPerThread = rEnd/numthreads;
	int w = numthreads-1;
	for(int i = 0; i < w; i++)
	{
		tasks[i] = new task(rowsPerThread);	
		for(int j = 0; j < rowsPerThread; j++)
		{
			if(j % 2 == 0)
			{
				tasks[i]->rows[j] = rBegin++;	
			}
			else
			{
				tasks[i]->rows[j] = rEnd--;		
			}
		}
	}
	{
		int r = rowsPerThread + (rEnd%numthreads);
		tasks[w] = new task(r);	// rowcount + rest
	}
	for(int i = 0; i < tasks[w]->count; i++)
	{
		tasks[w]->rows[i] = rEnd--;
	}
	return tasks;
}

// Task that finds largest region restricted to the a certain Task (submatrix)
// final_res will contain the result.
void doWork(int id, result* final_res)
{
 //   int max_sum = mat->get(0,0);
 //   int top = 0, left = 0, bottom = 0, right = 0; 

	final_res->top = 0;
	final_res->bottom = 0;
	final_res->right = 0;
	final_res->left = 0;
	final_res->sum = mat->get(0,0);

    int dimR = mat->getRows();
    int dimC = mat->getCols();

	#pragma omp parallel
	{
		//Auxilliary variables 
		int id = omp_get_thread_num();
	    int sum[dimR];
	    int pos[dimR];
    	int local_max;
		result* res = new result(0,0,0,0,0);

	#pragma omp for schedule(dynamic)
	    for (int i = 0; i < dimR; i++) {
	    	//printf("Thread-%i: Searching from row: %i \n", id, i);
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
	                //printf("Thread-%i: Found new values: top-%i, down-%i, left-%i, right-%i, sum-%i\n", id, i, k, pos[local_max], local_max, sum[local_max]);
	            }
	        }
	    }
	    printf("Thread-%i: Results found are: top->%i, down->%i, left->%i, right->%i, sum->%i \n", id, res->top, res->bottom, res->left, res->right, res->sum);
    // Lets evaluate the total largest area
	    if(final_res->sum < res->sum)
	    {
	    	#pragma omp critical
		    {
		    	if(final_res->sum < res->sum)
		    	{
		    		final_res->sum = res->sum;
	                final_res->top = res->top;
	                final_res->left = res->left;
	                final_res->bottom = res->bottom;
	                final_res->right = res->right;
		    	}
		    }	
	    }
	}
	printf("MainThread: Final result found is: top->%i, down->%i, left->%i, right->%i, sum->%i \n", id, final_res->top, final_res->bottom, final_res->left, final_res->right, final_res->sum);
}

void sumColumn(int id, int col)
{
	ps->set(0, col, mat->get(0, col));
	for(int i = 1; i < mat->getRows(); i++)
	{
		int val = ps->get(i-1,col) + mat->get(i,col);
		ps->set(i,col, val);
	}
}

// Algorithm based on information obtained here:
// http://stackoverflow.com/questions/2643908/getting-the-submatrix-with-maximum-sum
void setPrefixSumMatrix()
{
//    #pragma omp parallel 
 //   {
    	int id = omp_get_thread_num();
 //   	#pragma omp for 				// Static will do fine
	    	for(int i = 0; i < mat->getCols(); i++)
	    	{
	    		sumColumn(id, i);	
	    	}
 //   }

    return;
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