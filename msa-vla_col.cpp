#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
//#include <string.h>
#include <omp.h>
#include <iostream>
#include <string>

//#define _PRINT_INFO 


using namespace std;

typedef struct result {
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

} ;

long get_usecs (void)
{
    struct timeval t;
    gettimeofday(&t,NULL);
    return t.tv_sec*1000000+t.tv_usec;
}

void usage(const char* app_name) {
    printf("Argument error! Usage: %s <input_file> \n", app_name);
    exit(0);
}

void clear(int* a, int len) {
    for (int index=0; index<len; index++) {
        *(a+index) = 0;
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        usage(argv[0]);
    }

    // Open files
    FILE* input_file = fopen(argv[1], "r");
    if(input_file == NULL) {
        usage(argv[0]);
    }

    int numthreads   = omp_get_num_procs();
    //int numthreads   = 4;
    omp_set_num_threads(numthreads);
    cout << "Running with " << numthreads << " threads.... \n\n";

    // Read the matrix
    int dim = 0;
    fscanf(input_file, "%u\n", &dim);
    int mat[dim][dim];
    int element = 0;
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            if (j != (dim-1)) 
                fscanf(input_file, "%d\t", &element);
            else 
                fscanf(input_file, "%d\n",&element);

            // This is vital. Column points at an array of row elements (locality reasons for parallization)
            mat[j][i] = element;
        }
    }

#ifdef _PRINT_INFO
    // Print the matrix
    printf("Input matrix [%d]\n", dim);
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            printf("%d ", mat[j][i]);
            printf("(%d)\t", &mat[j][i]);
        }
        printf("\n");
    }
#endif

    // Algorithm based on information obtained here:
    // http://stackoverflow.com/questions/2643908/getting-the-submatrix-with-maximum-sum
    long alg_start = get_usecs();
    // Compute vertical prefix sum
    int ps[dim][dim];
    

// Divide work columnwise and for every iteration do column summation
#pragma omp parallel 
    {
        printf("thread-%i: Reporting for work! .. Will sum columns \n", omp_get_thread_num());
        #pragma omp parallel for
        for (int j=0; j<dim; j++) {             // for each col
            ps[j][0] = mat[j][0];               
            for (int i=1; i<dim; i++) {         // for each row
                ps[j][i] = ps[j][i-1] + mat[j][i];
            }
        }
    }

#ifdef _PRINT_INFO
    // Print the matrix
    printf("Vertical prefix sum matrix [%d]\n", dim);
    for(int i=0; i<dim; i++) {
        for(int j=0; j<dim; j++) {
            printf("%d\t", ps[j][i]);
        }
        printf("\n");
    }
#endif

    

    result* final_result = new result(0, 0, 0, 0, mat[0][0]);

// Divide work in terms of rows to check and do Kandane over
#pragma omp parallel 
    {
        int max_sum = mat[0][0];
        int top = 0, left = 0, bottom = 0, right = 0; 

        //Auxilliary variables 
        int sum[dim];
        int pos[dim];
        int local_max;

        printf("thread-%i: Reporting for work! .. Will do kadane stuff \n", omp_get_thread_num());
        #pragma omp parallel for schedule(dynamic, 2)
        for (int i=0; i<dim; i++) {
            for (int k=i; k<dim; k++) {
                // Kandane over all columns with the i..k rows
                clear(sum, dim);
                clear(pos, dim);
                local_max = 0;

                // We keep track of the position of the max value over each Kandane's execution
                // Notice that we do not keep track of the max value, but only its position
                sum[0] = ps[0][k] - (i==0 ? 0 : ps[0][i-1]);
                for (int j=1; j<dim; j++) {
                    if (sum[j-1] > 0){
                        sum[j] = sum[j-1] + ps[j][k] - (i==0 ? 0 : ps[j][i-1]);
                        pos[j] = pos[j-1];
                    } 
                    else {
                        sum[j] = ps[j][k] - (i==0 ? 0 : ps[j][i-1]);
                        pos[j] = j;
                    }
                    if (sum[j] > sum[local_max]){
                        local_max = j;
                    }
                } //Kandane ends here

                if (sum[local_max] > max_sum) {
                    // sum[local_max] is the new max value
                    // the corresponding submatrix goes from rows i..k.
                    // and from columns pos[local_max]..local_max

                    max_sum = sum[local_max];
                    top = i;
                    left = pos[local_max];
                    bottom = k;
                    right = local_max;
                }
            }
        }

        if(max_sum > final_result->sum)
         {
            #pragma omp critical
            {
                if(max_sum > final_result->sum)
                {
                    final_result->top = top;
                    final_result->bottom = bottom;
                    final_result->left = left;
                    final_result->right = right;
                    final_result->sum = max_sum;
                }
            }
         }
    }

    int top = final_result->top;
    int bottom = final_result->bottom;
    int left = final_result->left;
    int right = final_result->right;
    int max_sum = final_result->sum;


    // Compose the output matrix
    int outmat_row_dim = bottom - top + 1;
    int outmat_col_dim = right - left + 1;
    int outmat[outmat_row_dim][outmat_col_dim];
    for(int i=top, k=0; i<=bottom; i++, k++) {
        for(int j=left, l=0; j<=right ; j++, l++) {
            outmat[l][k] = mat[j][i];
        }
    }
    long alg_end = get_usecs();

    // Print output matrix
    printf("Sub-matrix [%dX%d] with max sum = %d, top = %d, bottom = %d, left = %d, right = %d\n", outmat_row_dim, outmat_col_dim, max_sum, top, bottom, left, right);
#ifdef _PRINT_INFO
    for(int i=0; i<outmat_row_dim; i++) {
        for(int j=0; j<outmat_col_dim; j++) {
            printf("%d\t", outmat[j][i]);
        }
        printf("\n");
    }
#endif

    // Release resources
    fclose(input_file);

    // Print stats
    printf("%s,arg(%s),%s,%f sec\n", argv[0], argv[1], "CHECK_NOT_PERFORMED", ((double)(alg_end-alg_start))/1000000);
    return 0;
}
