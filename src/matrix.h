#include <string>

#ifndef _MATRIX
class Matrix
{

	private:
		int** matrix;
		int rows;
		int cols;

		void initFromFile(std::string);
		void initMatrix(int, int);
	
	public:
		Matrix(std::string);
		Matrix(int n);
		Matrix(int n, int m);
		~Matrix();

		void print();
		int getRows();
		int getCols();
		int get(int, int);
		void set(int, int, int);

};
#define _MATRIX
#endif