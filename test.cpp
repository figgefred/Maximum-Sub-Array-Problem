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

};

long get_usecs (void);
void usage(const char*);
void clear(int*, int);
result* result_new(int, int, int, int, int);
void result_free(result*);
result* getSubregion(int, int, result*, int, int);

int** ps;
int** mat;

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

result* result_new(int t, int b, int l, int r, int sum)
{
    result* res = new result(t,b,l,r,sum);
    /*res->top = t;
    res->bottom = b;
    res->left = l;
    res->right = r;
    res->sum = sum;*/
    return res;
}

void result_free(result* r)
{
    delete r;
}

result* getSubregion(int arr[], int dim, result* res, int i, int k)
{
    
    //Auxilliary variables 
    int sum[dim];
    int pos[dim];
    int local_max = 0;
    int max_sum = 0;

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

        res->sum = sum[local_max];
        res->top = i;
        res->left = pos[local_max];
        res->bottom = k;
        res->right = local_max;
    }
    return res;
}