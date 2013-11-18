#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <thread>
#include <omp.h>
#include <iostream>
#include "matrix.h"
#include <vector>

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
	vector<int> rows;
	task(int tasksize)
	{
		rows = vector<int>(tasksize, tasksize + 5);
	}
	~task()
	{
		//delete rows;
	}

};

void printMatrix(Matrix*);
long get_usecs (void);
void usage(const char*);
void clear(int*, int);
void setPrefixSumMatrix();
void sumColumn(int, int, int);
void sumAllColumns();
void doWork(int, task*, result* result); // id, task, result_submission_list
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
    result** results = new result*[numthreads];
	int workercount = numthreads-1;

	if(workercount > 0)	// Start workers if workercount > 0
	{	
		thread threads[numthreads];

		task** tasks = getTasks();
		
	// Distribute work
	    for(int i = 0; i < workercount; i++)
	    {
	    	results[i] = new result(0,0,0,0,0);
	    	threads[i] = thread(doWork, i, tasks[i], results[i]);
	    }

	    results[workercount] = new result(0,0,0,0,0);
	    doWork(workercount, tasks[workercount], results[workercount]);

	    for(int i = 0; i < workercount; i++)
	    {
			threads[i].join();	    	
			delete tasks[i];
	    }
	}
	else
	{
		cout << "no. available rows is "<< mat->getRows() << "\n";
		task* myTask = new task(mat->getRows());
		for(int i = 0; i < mat->getRows(); i++)
		{
			myTask->rows[i] = i;
			//cout << "Handed " << i << " to main and ONLY worker. \n";
		} // startRow, startCol, endRow, endCol
		results[workercount] = new result(0,0,0,0,0);
		doWork(workercount, myTask, results[workercount]);
		delete myTask;
	}

	#ifdef _PRINT_INFO
	cout << "\nResults in pipeline \n";
	for(int i = 0; i < numthreads; i++)
	{
		cout << "(top, down, left, right) sum = " << " (" << results[i]->top << ", " << results[i]->bottom << ", " << results[i]->left << ", " << results[i]->right << ") " << results[i]->sum << "\n";
	}
	cout << "\n";
	#endif
//#Find result
	result* final_result = results[0];
	for(int i = 1; i < numthreads; i++)
	{
		if(final_result->sum < results[i]->sum)
		{
			final_result = results[i];
			//cout << "Current winning result: (top, down, left, right) = " << " (" << final_result->top << ", " << final_result->bottom << ", " << final_result->left << ", " << final_result->right << ") \n";
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

task** getTasks()
{
	task** tasks = new task*[numthreads];
		

	// Init tasks
	int rBegin = 0;
	int rEnd = mat->getRows();
	int rowsPerThread = rEnd/numthreads;
	for(int i = 0; i < numthreads; i++)
	{
		tasks[i] = new task(rowsPerThread);	
		bool tmp = true;
		for(int j = 0; j < rowsPerThread; j++)
		{
			if(tmp)
			{
				tasks[i]->rows[j] = rBegin++;	
				tmp = false;
			}
			else
			{
				tasks[i]->rows[j] = rEnd--;		
				tmp = true;
			}
		}
	}
	
	for(int i = 0; i < numthreads; i++)
	{
		cout << "Thread-" << i << ": Retrieved work patch: " << tasks[i]->rows.size() << "tasks(";
		for(int j = 0; j < tasks[i]->rows.size(); j++)
		{
			cout << tasks[i]->rows[j] << " ";
		}
		cout << "\n";
	}
	cout << "\n\n";

	// Excess work
	int r = (rEnd%numthreads);
	for( int i = 0; r > 0; i++ )
	{
		if(i == numthreads)
			i = 0;
		tasks[i]->rows.push_back(rBegin++);	
		r--;
	}

	for(int i = 0; i < numthreads; i++)
	{
		cout << "Thread-" << i << ": Retrieved work patch: " << tasks[i]->rows.size() << "tasks(";
		for(int j = 0; j < tasks[i]->rows.size(); j++)
		{
			cout << tasks[i]->rows[j] << " ";
		}
		cout << "\n";
	}
	cout << "\n\n";


	return tasks;
}

// Task that finds largest region restricted to the a certain Task (submatrix)
void doWork(int id, task* t, result* res)
{
 //   int max_sum = mat->get(0,0);
 //   int top = 0, left = 0, bottom = 0, right = 0; 

	res->top = 0;
	res->bottom = 0;
	res->right = 0;
	res->left = 0;
	res->sum = mat->get(0,0);

    int dimR = mat->getRows();
    int dimC = mat->getCols();

    //Auxilliary variables 
    int sum[dimR];
    int pos[dimR];
    int local_max;


    int i = 0;
    for (int tIndex = 0; tIndex < t->rows.size(); tIndex++) {
    	i = t->rows[tIndex];
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
}

void sumColumn(int id, int start, int end)
{
	for(int j = start; j < end; j++)
	{
		ps->set(0, j, mat->get(0, j));
		for(int i = 1; i < mat->getRows(); i++)
		{
			int val = ps->get(i-1,j) + mat->get(i,j);
			ps->set(i,j, val);
		}
	}
}

void sumAllColumns()
{
    sumColumn(0, 0, mat->getRows());
}

// Algorithm based on information obtained here:
// http://stackoverflow.com/questions/2643908/getting-the-submatrix-with-maximum-sum
void setPrefixSumMatrix()
{
    if(true)
    {	
    	sumAllColumns();
    	return;
    }
    int cols = mat->getCols();
    int chunks = cols/numthreads;
    
    int w = numthreads-1;
    thread threads[w];
    for(int i = 0; i < w; i++)
    {
    	int start = i*chunks;
    	threads[i] = thread(sumColumn, i, start, start+chunks);
    }
    sumColumn(w, w*chunks, cols);
    for(int i = 0; i < w; i++)
    {
    	threads[i].join();
    }
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