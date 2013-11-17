
#include "matrix.h"
#include <string>
#include <iostream>
#include <stdio.h>

Matrix::Matrix(std::string file_name)
{
	initFromFile(file_name);
}

Matrix::Matrix(int n)
{
	initMatrix(n,n);
}

Matrix::Matrix(int n, int m)
{
	initMatrix(n, m);
}

Matrix::~Matrix()
{
	for(int i = 0; i < rows; i++)
	{
		delete matrix[i];
	}
	delete matrix;
}

// Private

// This will not go well if T is not an int :))
void Matrix::initFromFile(std::string file_name)
{
	FILE* input_file = fopen(file_name.c_str(), "r");
    // Read the matrix
    int dim = 0;
    fscanf(input_file, "%u\n", &dim);

    rows = dim;
    cols = dim;

    matrix = new int*[dim];
    int element = 0;
    for(int i=0; i<dim; i++) {
        matrix[i] = new int[dim];
        for(int j=0; j<dim; j++) {
            if (j != (dim-1)) 
                fscanf(input_file, "%d\t", &element);
            else 
                fscanf(input_file, "%d\n",&element);
            matrix[i][j] = element;
        }
    }
    fclose(input_file);
}

void Matrix::initMatrix(int n, int m)
{
	rows = n;
	cols = m;
	matrix = new int*[n];
	for(int i = 0; i < n; i++)
	{
		matrix[i] = new int[m];
	}
}

// Public
int Matrix::getRows()
{
	return rows;
}

int Matrix::getCols()
{
	return cols;
}

int Matrix::get(int r, int c)
{
	return matrix[r][c];
}

void Matrix::set(int r, int c, int val)
{
	matrix[r][c] = val;
}

/*

#include "matrix.h"
#include <string>
#include <iostream>
#include <stdio.h>

Matrix::Matrix(std::string file_name)
{
	initFromFile(file_name);
}

Matrix::Matrix(int n)
{
	initMatrix(n,n);
}

Matrix::Matrix(int n, int m)
{
	initMatrix(n, m);
}

Matrix::~Matrix()
{
	for(int c = 0; c < cols; c++)
	{
		delete matrix[c];
	}
	delete matrix;
}

// Private

// This will not go well if T is not an int :))
void Matrix::initFromFile(std::string file_name)
{
	FILE* input_file = fopen(file_name.c_str(), "r");
    // Read the matrix
    int dim = 0;
    fscanf(input_file, "%u\n", &dim);

    rows = dim;
    cols = dim;

    matrix = new int*[cols];
    int element = 0;

    for(int j = 0; j < cols; j++)
	{
		matrix[j] = new int[rows];
	}

    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {

            if (j != (cols-1)) 
                fscanf(input_file, "%d\t", &element);
            else 
                fscanf(input_file, "%d\n",&element);
            matrix[j][i] = element;
        }
    }
    fclose(input_file);
}

void Matrix::initMatrix(int n, int m)
{
	rows = n;
	cols = m;
	matrix = new int*[cols];
	for(int c = 0; c < cols; c++)
	{
		matrix[c] = new int[rows];
	}
}

// Public
int Matrix::getRows()
{
	return rows;
}

int Matrix::getCols()
{
	return cols;
}

int Matrix::get(int r, int c)
{
	return matrix[c][r];
}

void Matrix::set(int r, int c, int val)
{
	matrix[c][r] = val;
}

*/