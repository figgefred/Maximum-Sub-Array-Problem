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
	int matrRows;
	int matrCols;
	int top;
	int bottom;
	int left;
	int right;
	int sum;
	double time;
	string filename;

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
void solve(string, result*);

int numthreads;
Matrix* mat;
Matrix* ps;

int main(int argc, char* argv[]) {

     if(argc < 3) {
        usage(argv[0]);
    }

//#Set thread num
    numthreads = atoi(argv[1]);
    if(numthreads < 1)
	{
		numthreads = omp_get_num_procs();
		cout << "\n\n WARNING! Setting default thread count: " << numthreads << "\n\n";
	}

	int resultCount = argc-2;
	result* all_results[resultCount];
	int resIndex = 0;

	cout << resultCount << " data files are expected for this program. \n\n";

// Loop over all files
	for(int c = 2; c < argc; c++)
	{

		cout << "\n\n" << "Solving for " << argv[c] << ": \n\n";
		all_results[resIndex] = new result(0,0,0,0,0);
		solve(argv[c], all_results[resIndex]);
    	resIndex++;
	}

// Stats
	for(int i = 0; i < resultCount; i++)
	{
		cout << all_results[i]->filename << ": \n";
		cout << "Sub-matrix [" << all_results[i]->matrRows << "X" << all_results[i]->matrCols << "] ";
		cout << "with max sum = " << all_results[i]->sum << ", ";
		cout << "top = " << all_results[i]->top << ", ";
		cout << "bottom = " << all_results[i]->bottom << ", ";
		cout << "left = " << all_results[i]->left << ", ";
		cout << "right = " << all_results[i]->right << "\n ";
		cout << "Executed for " << all_results[i]->time << " sec (with default " << numthreads << " threads).\n\n";
	}

// Release resources
    for(int i = 0; i < resultCount; i++)
    {
    	delete all_results[i];
    }
    return 0;
}

void solve(string file, result* res)
{
	//#Create matrix using filename
		mat = new Matrix(file); 	
		#ifdef _PRINT_INFO 
	    	cout << "Input matrix \n";
	    	printMatrix(mat);
		#endif

	//#Check that parameters are valid
		if(numthreads > mat->getRows())
		{
			omp_set_num_threads(mat->getRows());
			cout << "\n WARNING! More threads set then rows in matrice of "<< file << ". Threads set to ROW COUNT: " << mat->getRows() << "\n\n";
		}
		else
		{
			omp_set_num_threads(numthreads);
			cout << "\n Numthreads is set to: " << numthreads << "\n\n";	
		}

	//#Start timer
    	long alg_start = get_usecs();

	//#Compute vertical prefix sum
	    ps = new Matrix(mat->getRows(), mat->getCols());
	    setPrefixSumMatrix();
		#ifdef _PRINT_INFO
		    cout << "Vertical PrefixMatrix \n";
		    printMatrix(ps);
		#endif

	//#EXPLICIT THREADING 
	    result** part_results = new result*[numthreads];
		int workercount = numthreads-1;

		if(workercount > 0)	// Start workers if workercount > 0
		{	
			thread threads[workercount];
			task** tasks = getTasks();
		// FORK-JOIN
		    for(int i = 0; i < workercount; i++)
		    {
		    	part_results[i] = new result(0,0,0,0,0);
		    	threads[i] = thread(doWork, i, tasks[i], part_results[i]);
		    }
		    part_results[workercount] = new result(0,0,0,0,0);
		    doWork(workercount, tasks[workercount], part_results[workercount]);
			delete tasks[workercount];
		    for(int i = 0; i < workercount; i++)
		    {
				threads[i].join();	    	
				delete tasks[i];
		    }
		    delete tasks;
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
			part_results[workercount] = new result(0,0,0,0,0);
			doWork(workercount, myTask, part_results[workercount]);
			delete myTask;
		}

		#ifdef _PRINT_INFO
			cout << "\nResults in pipeline \n";
			for(int i = 0; i < numthreads; i++)
			{
				cout << "(top, down, left, right) sum = " << " (" << part_results[i]->top << ", " << part_results[i]->bottom << ", " << part_results[i]->left << ", " << part_results[i]->right << ") " << part_results[i]->sum << "\n";
			}
			cout << "\n";
		#endif

	//#Find result
		if(workercount == 0)
		{
			res->sum = part_results[0]->sum;
			res->top = part_results[0]->top;
			res->bottom = part_results[0]->bottom;
			res->left = part_results[0]->left;
			res->right = part_results[0]->right;
		}
		else
		{
			for(int i = 0; i < workercount; i++) // Exclude the last as this is the main threads partial result.
			{
				if(res != part_results[i] && res->sum < part_results[i]->sum)
				{
					res->sum = part_results[i]->sum;
					res->top = part_results[i]->top;
					res->bottom = part_results[i]->bottom;
					res->left = part_results[i]->left;
					res->right = part_results[i]->right;
				}
			}
		}
		res->filename = file;
	//#END OF EXPLICIT THREADING

		int bottom = res->bottom;
		int top = res->top;
		int right = res->right;
		int left = res->left;
		int max_sum = res->sum;


	//#Compose the output matrix
	    int outmat_row_dim = bottom - top + 1;
	    int outmat_col_dim = right - left + 1;
	    Matrix* outmat = new Matrix(outmat_row_dim, outmat_col_dim);

	    for(int i=top, k=0; i<=bottom; i++, k++) {
	        for(int j=left, l=0; j<=right ; j++, l++) {
	            outmat->set(k, l, mat->get(i,j));
	        }
	    }

		res->matrRows = outmat_row_dim;
		res->matrCols = outmat_col_dim;

	    long alg_end = get_usecs();
	    res->time = ((double)(alg_end-alg_start))/1000000;

	    // Print output matrix
		#ifdef _PRINT_INFO
		    cout << "Submatrix found" << "\n";
		    printMatrix(outmat);
		#endif

	    // Release resources
        delete mat;
    	delete ps;
    	delete outmat;
    	for(int i = 0; i < numthreads; i++)
    	{
			delete part_results[i];
    	}
    	delete part_results;
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

	// Excess work
	int r = (rEnd%numthreads);
	for( int i = 0; r > 0; i++ )
	{
		if(i == numthreads)
			i = 0;
		tasks[i]->rows.push_back(rBegin++);	
		r--;
	}

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
    //printf("Thread-%i: Results found are: top->%i, down->%i, left->%i, right->%i, sum->%i \n", id, res->top, res->bottom, res->left, res->right, res->sum);
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
    printf("Argument error! Usage: %s <num_threads> <list_input_files> \n", app_name);
    exit(0);
}

void clear(int* a, int len) {
    for (int index=0; index<len; index++) {
        *(a+index) = 0;
    }
}