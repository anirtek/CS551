#include "matrix_mul.h"
#include "errors.h"

//#define DO_TRACE 1
#include "trace.h"

#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/** Return a multi-process matrix multiplier with nWorkers worker
 *  processes.  Set *err to an appropriate error number (documented in
 *  errno(3)) on error.
 *
 *  If trace is not NULL, turn on tracing for calls to mulMatrixMul()
 *  using returned MatrixMul.  Specifically, each dot-product
 *  computation must be logged to trace as a line in the form:
 *
 *  INDEX[PID]: [I]x[J] = Z
 *
 *  where INDEX in [0, nWorkers) is the index of the worker and PID is
 *  its pid, I is the index of the row in the multiplicand, J is the
 *  index of the row in the multiplier and Z is their dot-product
 *  computed by child process PID.  The spacing must be exactly as
 *  shown above and all values must be output in decimal with no
 *  leading zeros or redundant + signs.
 *
 */
struct  MatrixMul {
	int numberOfWorkers;
	int fd1[10][2]; /* used for child --> parent communication */
	int fd2[10][2]; /* used for parent --> child communication */
	long pid[10];  /* used for storing pids of the child processes */

};

enum {MAX_LINE = sizeof(int), DIODE = 2 }; 

MatrixMul *
newMatrixMul(int nWorkers, FILE *trace, int *err)
{
	/**
	 * Define memory for the matrix
	 */
	MatrixMul *matMulImpl = malloc(sizeof(MatrixMul)); 

	/**
	 * Checking if number or workers are less than n
	 */
	if(nWorkers <= 0)
		fprintf(stderr, "Number of workers %d too small\n", nWorkers);
	
	matMulImpl->numberOfWorkers = nWorkers; 
	
	/**
	 * Creating pipes & childs
	 */
	int i;
	for(i = 0; i < nWorkers; i++)
	{
		if(pipe(matMulImpl->fd1[i]) < 0)
		{
			fprintf(stderr, "Cannot create pipe\n");

		}
		if(pipe(matMulImpl->fd2[i]) < 0)
		{			
			fprintf(stderr, "Cannot create pipe\n");
		}
		if((matMulImpl->pid[i] = fork()) == -1)
		{
			fprintf(stderr, "Error in forking\n"); /*If forking fails*/
		}
		else if(matMulImpl->pid[i] == 0) /*child*/
		{
			/**
			 * adding pids into the structure
		 	*/
			matMulImpl->pid[i] = (long)getpid();
		}
	}
	return matMulImpl;
}

/** Free all resources used by matMul.  Specifically, free all memory
 *  and return only after all child processes have been set up to
 *  exit.  Set *err appropriately (as documented in errno(3)) on error.
 */
void
freeMatrixMul(MatrixMul *matMul, int *err)
{
	free(matMul);
}

/** Set matrix c[n1][n3] to a[n1][n2] * b[n2][n3].  It is assumed that
 *  the caller has allocated c[][] appropriately.  Set *err to an
 *  appropriate error number (documented in errno(3)) on error.  If
 *  *err is returned as non-zero, then the matMul object may no longer
 *  be valid and future calls to mulMatrixMul() may have unpredictable
 *  behavior.  It is the responsibility of the caller to call
 *  freeMatrixMul() after an error.
 *
 *  All dot-products of rows from a[][] and columns from b[][] must be
 *  performed using the worker processes which were already created in
 *  newMatrixMul() and all IPC must be handled using anonymous pipes.
 *  The multiplication should be set up in such a way so as to allow
 *  the worker processes to work on different dot-products
 *  concurrently.
 */
void mulMatrixMul (const MatrixMul *matMul, int n1, int n2, int n3,
             CONST MatrixBaseType a[n1][n2],
             CONST MatrixBaseType b[n2][n3],
             MatrixBaseType c[n1][n3], int *err)
{
	
	int product = 0;
	int i, j, k;
	int status;
	int p, q;
	int t;
	int IK, KJ;
	
	for(i = 0; i < n1; i++)
    	{
            		for(j = 0; j < n3; j++)
            		{
				
   	            		for(k = 0; k < n2; k++)
                		{
					IK = a[i][k];
					KJ = b[k][j];
					if (write(matMul->fd1[i][1], &IK, MAX_LINE) == -1 ) 
					{
                				perror("error writing to pipe");
			                	exit(EXIT_FAILURE);
            				}
					if (write(matMul->fd1[i][1], &KJ, MAX_LINE) == -1 ) 
					{
                                        	perror("error writing to pipe");
	                                        exit(EXIT_FAILURE);
                                        }
                		}
			}
    	}
	
	int aa;
	int bb;
	for(p = 0; p < n1; p++)
	{
		close(matMul->fd1[i][1]);
		close(matMul->fd2[i][0]);
		for(int m = 0 ; m < n3; m++) 
		{
			product = 0;
			for(q = 0; q < n2; q++)
			{
        			int n;
				if ((n = read(matMul->fd1[p][0], &aa, sizeof(aa))) < 0) 
				{
			            	perror("error reading from pipe in child");
			            	exit(EXIT_FAILURE);
        			}	
			        else if (n == 0) 
				{
			           	printf("Pipe from parent closed to child");
        			}
        			else 
				{
            				//printf("Child read %d from parent.\n", aa);
				}
				if ((n = read(matMul->fd1[p][0], &bb, sizeof(bb))) < 0) 
				{
                                    	perror("error reading from pipe in child");
                                    	exit(EXIT_FAILURE);
                                }
                                else if (n == 0) 
				{
                                 	printf("Pipe from parent closed to child");
                                }
				product += aa * bb;
			}
			fprintf(stderr, "%d[%ld]: [%d]x[%d] = %d\n", p, (long)matMul->pid[i], p, m, product );

			if ( write(matMul -> fd2[p][1], &product, sizeof(product)) == -1 ) 
			{
                    		perror("error writing to pipe id child");
                    		exit(EXIT_FAILURE);
                	}
		}
	}

	int res = 0;
	for(int i = 0; i < n1; i++)
	{
		close(matMul->fd2[i][1]);
		close(matMul->fd1[i][0]);
		for(int j = 0; j < n3; j++)
		{
			int n;
                        if ((n = read(matMul -> fd2[i][0], &res, sizeof(int))) == -1) 
			{
                        	perror("error reading from pipe");
                                exit(EXIT_FAILURE);
			}
                        else if ( n == 0 ) 
			{
                               	printf("Pipe from child closed.\n");
                        }
                        else 
			{
                               	//printf("Parent read %d from child", res);
                              	c[i][j] = res;
				//	printf("%d[%ld]: [%d]x[%d] = %d\n", i, (long)matMul->pid[i], i, j, res);
                        }
		}
	}

/*	for(int c = 0; c < matMul -> noOfWorkers; c++)
	{
		close(matMul->ctop_fd[c][0]);
		close(matMul->ctop_fd[c][1]);
	 	close(matMul->ptoc_fd[c][1]);
		close(matMul->ptoc_fd[c][0]);
	}
*/

	for(t = 0; t < matMul -> numberOfWorkers; t++) 
	{
		waitpid(matMul-> pid[t], &status, 0);
	}

	//printf("Parent is terminated\n");

//	freeMatrixMul((struct MatrixMul *) matMul, err);
}

