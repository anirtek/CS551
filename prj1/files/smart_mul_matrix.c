#include "dense_matrix.h"
#include "smart_mul_matrix.h"
#include "abstract_matrix.h"

#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

//TODO: Add types, data and functions as required.
/**
 * Smart Matrix Structure
 */
typedef struct {
    SmartMulMatrix;
    int rows;
    int cols;
    int elements[]; //array of integer elements part of struct
} SmartMatrixImpl;

static const char *getKlass(const Matrix *this, int *err) 
{
	return "smart_Matrix"; //just returning the classname
}

static int getNRows(const Matrix * this, int *err) 
{
	const SmartMatrixImpl *matrix = (const SmartMatrixImpl *) this;
	return matrix->rows;
}

static int getNCols(const Matrix *this, int *err)
{
	const SmartMatrixImpl *matrix = (const SmartMatrixImpl *) this;
	return matrix->cols;
}

static MatrixBaseType getElement(const Matrix *this, int rowIndex, int colIndex, int *err)
{
	const SmartMatrixImpl *matrix = (const SmartMatrixImpl *) this;
	int ncols = this->fns->getNCols(this, err);
	return matrix->elements[rowIndex * ncols + colIndex];
}

static void setElement(Matrix *this, int rowIndex, int colIndex, MatrixBaseType data, int *err)
{
	SmartMatrixImpl *matrix = (SmartMatrixImpl *) this;
	int ncols = this->fns->getNCols(this, err);
	matrix->elements[rowIndex * ncols + colIndex] = data;
}

static void mul(const Matrix *this, const Matrix *multiplier, Matrix *product, int *err)
{
 	int srcRows = this->fns->getNRows(this, err);
    	int srcCols = this->fns->getNCols(this, err);
    	int mltprNRows = multiplier->fns->getNRows(multiplier, err);
    	int mltprNCols = multiplier->fns->getNCols(multiplier, err);

    	if(srcCols == mltprNRows)
    	{
		int mltprTranspose[mltprNCols][mltprNRows];
		for(int i = 0; i < mltprNRows; i++)
		{
	    		for(int j = 0; j < mltprNCols; j++)
	    		{
				int localElement = this->fns->getElement(multiplier, i, j, err);
				mltprTranspose[j][i] = localElement;	
	    		}		
		}
			
			int result = 0;

			for(int i = 0; i < srcRows; i++)
			{
	    			for(int j = 0; j < mltprNCols; j++)
	    			{
	        			for(int k = 0; k < mltprNRows; k++)
					{
		  				result += this->fns->getElement(this, i, k, err) * mltprTranspose[j][k];
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

/** Return a newly allocated matrix with all entries in consecutive
 *  memory locations (row-major layout).  All entries in the newly
 *  created matrix are initialized to 0.  The return'd matrix uses
 *  a smart multiplication algorithm to avoid caching issues;
 *  specifically, transpose the multiplier and use a modified
 *  multiplication algorithm with the transposed multiplier.
 *
 *  Set *err to EINVAL if nRows or nCols <= 0, to ENOMEM if not enough
 *  memory.
 */

static _Bool isInit = false;
static SmartMulMatrixFns smartMatrixFns = {
	.getKlass = getKlass,
	.getElement = getElement,
	.setElement = setElement,
	.getNCols = getNCols,
	.getNRows = getNRows,
	.mul = mul
};

SmartMulMatrix *newSmartMulMatrix(int nRows, int nCols, int *err)
{
	SmartMatrixImpl *matrix = malloc(sizeof(SmartMatrixImpl)+nRows*nCols*sizeof(int));
	if(!matrix)
	{
	  *err = ENOMEM;
	}
	
  	if (!isInit) 
  	{	
        	const MatrixFns *fns = getAbstractMatrixFns(); //getting abstract object  into dense matrix class
        	smartMatrixFns.transpose = fns->transpose;
        	smartMatrixFns.free = fns->free;
        	isInit = true;
  	}
    
    	matrix->fns = (MatrixFns *) &smartMatrixFns;
    	matrix->rows = nRows;
    	matrix->cols = nCols;
    
    	for(int i = 0; i < nRows*nCols; i++)
    	{
        	matrix->elements[i] = i;
   	}
    
  	return (SmartMulMatrix *)matrix;
}

/** Return implementation of functions for a smart multiplication
 *  matrix; these functions can be used by sub-classes to inherit
 *  behavior from this class.
 */
const SmartMulMatrixFns *getSmartMulMatrixFns(void)
{
  return &smartMatrixFns;
}
