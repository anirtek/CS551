#include "abstract_matrix.h"
#include "dense_matrix.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

//TODO: Add types, data and functions as required.
/**
 * Structure of the Dense Matrix
 */
typedef struct {
    DenseMatrix;
    int rows;
    int cols;
    int elements[]; //flexy array of integer elements part of struct
} MatrixImpl;

static const char *getKlass(const Matrix *this, int *err) 
{
    return "dense_Matrix"; //just returning the classname
}

static int getNRows(const Matrix *this, int *err) 
{
    const MatrixImpl *matrix = (const MatrixImpl *) this;
    return matrix->rows;
}

static int getNCols(const Matrix *this, int *err) 
{
    const MatrixImpl *matrix = (const MatrixImpl *) this;
    return matrix->cols;
}

static MatrixBaseType getElement(const Matrix *this, int rowIndex, int colIndex, int *err) 
{
    const MatrixImpl *matrix = (const MatrixImpl *) this;
    int ncols = this->fns->getNCols(this, err); //you can write just getNCols
    return matrix->elements[rowIndex * ncols + colIndex]; // how to access a value in an array here?
}

static void setElement(Matrix *this, int rowIndex, int colIndex, MatrixBaseType data, int *err) 
{
    MatrixImpl *matrix = (MatrixImpl *) this;
    int ncols = this->fns->getNCols(this, err);
    matrix->elements[rowIndex * ncols + colIndex] = data;
}

/** Return a newly allocated matrix with all entries in consecutive
 *  memory locations (row-major layout).  All entries in the newly
 *  created matrix are initialized to 0.  Set *err to EINVAL if nRows
 *  or nCols <= 0, to ENOMEM if not enough memory.
 */

static _Bool isInit = false;
static DenseMatrixFns matrixFns = {
    .getKlass = getKlass,
    .getElement = getElement,
    .setElement = setElement,
    .getNRows = getNRows,
    .getNCols = getNCols
};

DenseMatrix *newDenseMatrix(int nRows, int nCols, int *err) 
{
    MatrixImpl *matrix = malloc(sizeof(MatrixImpl)+nRows*nCols*sizeof(int));
    if (!matrix) 
    {
	    *err = ENOMEM;
    }

    if (!isInit) {
        const MatrixFns *fns = getAbstractMatrixFns(); //getting abstract object  into dense matrix class
        matrixFns.transpose = fns->transpose;
        matrixFns.free = fns->free;
        matrixFns.mul = fns->mul;
        isInit = true;
    }
    
    matrix->fns = (MatrixFns *)&matrixFns;
    matrix->rows = nRows;
    matrix->cols = nCols;
    
    for(int i = 0; i < nRows*nCols; i++)
    {
        matrix->elements[i] = i;
    }
    
    return (DenseMatrix *)matrix;
}

/** Return implementation of functions for a dense matrix; these functions
 *  can be used by sub-classes to inherit behavior from this class.
 */
const DenseMatrixFns *getDenseMatrixFns(void) 
{
    return &matrixFns;
}

