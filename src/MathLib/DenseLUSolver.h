#ifndef __DDRMATHLIB_DENSE_LU_SOLVER_H_INCLUDED__
#define __DDRMATHLIB_DENSE_LU_SOLVER_H_INCLUDED__

namespace DDRMathLib {
;
// In-place LU decomposition for SYMMETRIC matrix A (equivalent to LDLt)
// ONLY UPPER TRIANGLE of A will be taken care of
// pLU stores ONLY the normalized UPPER TRIANGULAR matrix (U)
// (off diagonal elements already normalized by diagonal elements)
// L (under diagonal) can be recovered by U transpose
// Note that the decomposition is executed with diagonally pivoting
// Example,           0  4  2
// Input,		A = [ 4  6  8 ]
//                    2  8  10	
//
//                   10   0.8   0.2               1    0   0          10    8    2
// Output,    pLU = [0   -0.4   -6 ]   =>  L = [ 0.8   1   0 ], U = [ 0   -0.4  2.4]
//                   0     0    14               0.2  -6   1          0     0    14 
//
//            ordering = [2 1 0]
// Equivalent equations:
// Original:           4 x1 +   2 x2 = y0
//            4 x0 +   6 x1 +   8 x2 = y1
//            2 x0 +   8 x1 +  10 x2 = y2
// After LU:    z0                   = y2
//          0.8 z0 +     z1          = y1
//          0.2 z0 -   6 z1 +     z2 = y0
//         10 (x2 + 0.8 x1 + 0.2 x0) = z0
//                 -0.4(x1 -   6 x0) = z1
//                           14 (x0) = z2
bool DecompSymmLU(int dimN, double *A, int *pOrdering);

// Linear solver for AX = B (B in-place)
// with symmetric A (NxN) decomposed as A = LU
// with permutation in ordering, and B a NxM matrix
// Results are stored in pB
// pTmpMem points to a temporary memory block enough to hold B
void SolveSymmLU(int dimN, const double *LU, const int *ordering,
	             int dimM, double *pB, double *pTmpMem);

// For a symmetric A (NxN) decomposed as A = LU
// with permutation in ordering, get its inverse (in-place)
// pTmpMem points to a temporary memory enough to hold two NxN matrices
void InvSymmLU(int dimN, double *LU, const int *ordering, double *pTmpMem);

// Decomposition for a general square matrix A = LU (A : NxN)
// pLU stores U in the upper triangle (including diagonal elements)
// and L's lower triangle part (excluding diagonal elements)
// L's diagonal elements are all unities
// Permutation is stored in pOrdering
bool DecompLU(int dimN, double *A, int *pOrdering);

// Linear solver for AX = B (B in-place)
// with general square matrix A (NxN) decomposed as A = LU
// with permutation in ordering, and B a NxM matrix
// Results are stored in pB
// pTmpMem points to a temporary memory block enough to hold B
bool SolveLU(int dimN, const double *LU, const int *ordering,
	         int dimM, double *pB, double *pTmpMem);

// For a general square A (NxN) decomposed as A = LU
// with permutation in ordering, get its inverse (in-place)
// pTmpMem points to a temporary memory enough to hold two NxN matrices
void InvLU(int dimN, double *LU, const int *ordering, double *pTmpMem);

// Cholesky decomposition for symmetric positive definite matrix
// ONLY LOWER TRIANGLE of A will be taken care of
// pCholMat stores the result in LOWER TRIANGLE so that:
// pCholMat(LowerTriangle) * (pCholMat(LowerTriangle))' = A
static bool DecompChol(int dimN, // N
	                   const double *A, // N x N symmetric and PD matrix
	                   double *pCholMat // N x N result (can be equal to A for in-place computation)
                       );



}

#endif // __DDRMATHLIB_DENSE_LU_SOLVER_H_INCLUDED__
