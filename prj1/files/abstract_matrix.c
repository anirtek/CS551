#include "abstract_matrix.h"
#include <errno.h>
#include <stdlib.h>
//#include "dense_matrix.c"

//TODO: Add types, data and functions as required.

/** Return implementation of functions for an abstract matrix; these are
 *  functions which can be implemented using only other matrix functions,
 *  independent of the actual implementation of the matrix.
 */

static void transpose(const Matrix *this, Matrix *result, int *err) 
{
    int src_matrix_rows = this->fns->getNRows(this, err);
    int src_matrix_cols = this->fns->getNCols(this, err);

    /**
     * LOGIC AND LOOPING FOR TRANSPOSE
     */
    for (int i = 0; i < src_matrix_rows; i++) 
    {
        for (int j = 0; j < src_matrix_cols; j++) 
	{
            int data = this->fns->getElement(this, i, j, err);
            result->fns->setElement(result, j, i, data, err);
        }
    }
}

/**
 * Following function is used for multiplication of the matrix :
 */
static void mul(const Matrix *this, const Matrix *multiplier, Matrix *product, int *err)
{
    int srcNRows = this->fns->getNRows(this, err);
    int srcNCols = this->fns->getNCols(this, err);
    int mltprNRows = multiplier->fns->getNRows(multiplier, err);
    int mltprNCols = multiplier->fns->getNCols(multiplier, err);
    
    if(srcNCols == mltprNRows) 
    {
	  int result = 0;
        for(int i = 0; i < srcNRows; i++)
	{
            for(int j = 0; j < mltprNCols; j++)
	    {
		    
                for(int k = 0; k < mltprNRows; k++)
		{
                    result += this->fns->getElement(this, i, k, err) * multiplier->fns->getElement(multiplier, k, j, err);
                }
		product->fns->setElement(product, i, j, result, err);
		result = 0;
            }
        }
    }
    else 
    {
        *err = EDOM;
    }
}

/**
 * Following function is used to free the matrix resources : 
 */
static void free_matrix(Matrix *this, int *err) {
    free(this);
}


static MatrixFns abstractMatrixFns = {
    .transpose = transpose,
    .free = free_matrix,
    .mul = mul
};

const MatrixFns *getAbstractMatrixFns(void) 
{
    return &abstractMatrixFns;
}


