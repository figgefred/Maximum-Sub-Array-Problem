/*

This is a sequential implementation of a solver for the Maximum Sub Array Problem.

For further details see http://en.wikipedia.org/wiki/Maximum_subarray_problem

-------------------------------------------------------------------------------
The algorithm implemented is Kadane's algorithm.


Author: Frederick Ceder

*/
#include <stdio.h> 
#include <stdlib.h> 
#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include <omp.h>
#define BUFFER_SIZE 32

typedef struct {
	int start;
	int end;
	int sum;

} bound;

vector<int> numbers(0, 50);

bound* getMaximumSubarray();
void readInput();
void printUsage();
int parse(int, char**);

int main (int argc, char *argv[])
{

  if(parse(argc, argv) == 0)
  {
    return 0;
  }
  readInput();

  for(int i = 1; i < argc; i++)
  {
  	cout << numbers.at(i) << " ";	
  }
  bound* subarray = getMaximumSubarray();

  cout << "Subarray {" << subarray->start << ", " << subarray->end << "} \n";
  for(int i = subarray->start; i <= subarray->end; i++)
  {
  	cout << numbers[i] << " ";
  }
  cout << "\n";

}

bound* getMaximumSubarray()
{

 	int maxend = numbers[0];
	bound* subarray = new bound();
	subarray->start = 0;
	subarray->end = 0;
	subarray->sum = numbers[0];
	int tmp = 0;

	for(int i = 1; i < numbers.size(); i++)
	{
		if(maxend  < 0)
		{
			maxend = numbers[i];
			tmp = i;
		}
		else 
		{
			maxend += numbers[i];
		}
		if(maxend >= subarray->sum)
		{
			subarray->sum = maxend;
			subarray->start = tmp;
			subarray->end = i;
		}
	}
	return subarray;
}

void readInput()
{
	string line;
	int i = 0;
	while(getline(cin, line))
	{
		numbers.push_back(atoi( line.c_str()));		
	}

}

int parse(int argc, char *argv[])
{

  return 1;
}

void printUsage()
{
  cout << "\nUsage: pi <thread_count> <num_steps> \n";
}