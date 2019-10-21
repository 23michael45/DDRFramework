#include "DenseLUSolver.h"

#include <math.h>
#include <string.h>
#include <utility>
#define SSE_AVX_ACCELERATION

#ifdef SSE_AVX_ACCELERATION
#include "immintrin.h"
#endif

namespace DDRMathLib {

extern const double OUREPS = 1e-18;

bool DecompSymmLU(int dimN, double *pLU, int *pOrdering)
{
	for (int i = 0; i < dimN; ++i) {
		pOrdering[i] = i;
	}
	int ttii = 0;
	for (int i = 0; i < dimN; ++i, ttii += dimN) {
		int ttjj = ttii + dimN;
		double piv = pLU[ttii + i];
		int piv_rowId = i;
		int j;
		for (j = i + 1; j < dimN; ++j, ttjj += dimN) {
			if (fabs(pLU[ttjj + j]) > fabs(piv)) {
				piv = pLU[ttjj + j];
				piv_rowId = j;
			}
		}
		if (fabs(piv) < OUREPS) {
			return false;
		}
		if (piv_rowId != i)	{
			// row-column exchanges to retain symmetry
			int ttrr = dimN * piv_rowId;
			std::swap(pLU[ttii + i], pLU[ttrr + piv_rowId]);
			ttjj = 0;
			int j;
			for (j = 0; j < i; ++j, ttjj += dimN) {
				std::swap(pLU[ttjj + i], pLU[ttjj + piv_rowId]);
			}
			ttjj += dimN;
			for (j = i + 1; j < piv_rowId; ++j, ttjj += dimN) {
				std::swap(pLU[ttii + j], pLU[ttjj + piv_rowId]);
			}
			j = piv_rowId + 1;
#ifdef SSE_AVX_ACCELERATION
			for (; j + 3 < dimN; j += 4) {
				__m256d _a = _mm256_loadu_pd(pLU + ttii + j);
				__m256d _b = _mm256_loadu_pd(pLU + ttrr + j);
				_mm256_storeu_pd(pLU + ttrr + j, _a);
				_mm256_storeu_pd(pLU + ttii + j, _b);
			}
#endif
			for (; j < dimN; ++j) {
				std::swap(pLU[ttii + j], pLU[ttrr + j]);
			}
			std::swap(pOrdering[i], pOrdering[piv_rowId]);
		}
		double invPiv = 1.0 / piv;
		ttjj = ttii + dimN;
		for (j = i + 1; j < dimN; ++j, ttjj += dimN) {
			double c = pLU[ttii + j] * invPiv;
			int k = j;
#ifdef SSE_AVX_ACCELERATION
			__m256d c_ = _mm256_set1_pd(c);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pLU + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pLU + ttii + k);
				__m256d d0_ = _mm256_mul_pd(b_, c_);
				a_ = _mm256_sub_pd(a_, d0_);
				_mm256_storeu_pd(pLU + ttjj + k, a_);
			}
#endif
			for (; k < dimN; ++k) {
				pLU[ttjj + k] -= c * pLU[ttii + k];
			}
		}
		j = i + 1;
#ifdef SSE_AVX_ACCELERATION
		__m256d inv_ = _mm256_set1_pd(invPiv);
		for (; j + 3 < dimN; j += 4) {
			__m256d a_ = _mm256_loadu_pd(pLU + ttii + j);
			a_ = _mm256_mul_pd(a_, inv_);
			_mm256_storeu_pd(pLU + ttii + j, a_);
		}
#endif
		for (; j < dimN; ++j) {
			pLU[ttii + j] *= invPiv;
		}
	}
	return true;
}

void SolveSymmLU(int dimN, const double *LU, const int *ordering,
	             int dimM, double *pB, double *pTmpMem)
{
	int ind = 0;
	for (int i = 0; i < dimN; ++i) {
		int ind_ = ordering[i] * dimM;
		for (int k = 0; k < dimM; ++k) {
			pTmpMem[ind++] = pB[ind_++];
		}
	}

	int ttii = 0;
	for (int i = 0; i < dimN; ++i, ttii += dimN) {
		for (int j = i + 1; j < dimN; ++j) {
			double wt = -LU[ttii + j];
			ind = i * dimM;
			int k = 0, ind_j = j * dimM;
#ifdef SSE_AVX_ACCELERATION
			__m256d x_ = _mm256_set1_pd(wt);
			for (; k + 3 < dimM; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ind_j);
				a_ = _mm256_mul_pd(a_, x_);
				b_ = _mm256_add_pd(b_, a_);
				_mm256_storeu_pd(pTmpMem + ind_j, b_);
				ind += 4;
				ind_j += 4;
			}
#endif
			for (; k < dimM; ++k) {
				pTmpMem[ind_j++] += wt * pTmpMem[ind++];
			}
		}
	}

	ind = 0, ttii = 0;
	for (int i = 0; i < dimN; ++i, ttii += dimN) {
		double wt = 1.0 / LU[ttii + i];
		int k = 0;
#ifdef SSE_AVX_ACCELERATION
		__m256d x_ = _mm256_set1_pd(wt);
		for (; k + 3 < dimM; k += 4) {
			__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
			a_ = _mm256_mul_pd(a_, x_);
			_mm256_storeu_pd(pTmpMem + ind, a_);
			ind += 4;
		}
#endif
		for (; k < dimM; ++k) {
			pTmpMem[ind++] *= wt;
		}
	}
	for (int i = dimN - 1; i >= 0; --i) {
		for (int j = i - 1; j >= 0; --j) {
			double wt = -LU[j * dimN + i];
			ind = i * dimM;
			int k = 0, ind_j = j * dimM;
#ifdef SSE_AVX_ACCELERATION
			__m256d x_ = _mm256_set1_pd(wt);
			for (; k + 3 < dimM; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ind_j);
				a_ = _mm256_mul_pd(a_, x_);
				b_ = _mm256_add_pd(b_, a_);
				_mm256_storeu_pd(pTmpMem + ind_j, b_);
				ind += 4;
				ind_j += 4;
			}
#endif
			for (; k < dimM; ++k) {
				pTmpMem[ind_j++] += wt * pTmpMem[ind++];
			}
		}
	}

	ind = 0;
	for (int i = 0; i < dimN; ++i) {
		int ind_ = ordering[i] * dimM;
		for (int k = 0; k < dimM; ++k) {
			pB[ind_++] = pTmpMem[ind++];
		}
	}
}

void InvSymmLU(int dimN, double *LU, const int *ordering, double *pTmpMem)
{
	for (int i = 0; i < dimN * dimN; ++i) {
		pTmpMem[i] = 0.0;
	}
	for (int i = 0; i < dimN; ++i) {
		pTmpMem[i * dimN + ordering[i]] = 1.0;
	}

	int ind = 0;
	for (int i = 0; i < dimN; ++i) {
		for (int j = i + 1; j < dimN; ++j) {
			ind = i * dimN;
			double wt = -LU[ind + j];
			int k = 0, ind_j = j * dimN;
#ifdef SSE_AVX_ACCELERATION
			__m256d x_ = _mm256_set1_pd(wt);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ind_j);
				a_ = _mm256_mul_pd(a_, x_);
				b_ = _mm256_add_pd(b_, a_);
				_mm256_storeu_pd(pTmpMem + ind_j, b_);
				ind += 4;
				ind_j += 4;
			}
#endif
			for (; k < dimN; ++k) {
				pTmpMem[ind_j++] += wt * pTmpMem[ind++];
			}
		}
	}

	ind = 0;
	for (int i = 0; i < dimN; ++i) {
		double wt = 1.0 / LU[ind + i];
		int k = 0;
#ifdef SSE_AVX_ACCELERATION
		__m256d x_ = _mm256_set1_pd(wt);
		for (; k + 3 < dimN; k += 4) {
			__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
			a_ = _mm256_mul_pd(a_, x_);
			_mm256_storeu_pd(pTmpMem + ind, a_);
			ind += 4;
		}
#endif
		for (; k < dimN; ++k) {
			pTmpMem[ind++] *= wt;
		}
	}
	for (int i = dimN - 1; i >= 0; --i) {
		for (int j = i - 1; j >= 0; --j) {
			double wt = -LU[j * dimN + i];
			ind = i * dimN;
			int k = 0, ind_j = j * dimN;
#ifdef SSE_AVX_ACCELERATION
			__m256d x_ = _mm256_set1_pd(wt);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ind);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ind_j);
				a_ = _mm256_mul_pd(a_, x_);
				b_ = _mm256_add_pd(b_, a_);
				_mm256_storeu_pd(pTmpMem + ind_j, b_);
				ind += 4;
				ind_j += 4;
			}
#endif
			for (; k < dimN; ++k) {
				pTmpMem[ind_j++] += wt * pTmpMem[ind++];
			}
		}
	}
	ind = 0;
	for (int i = 0; i < dimN; ++i) {
		int ind_ = ordering[i] * dimN;
		int k = 0;
#ifdef SSE_AVX_ACCELERATION
		for (; k + 3 < dimN; k += 4, ind_ += 4, ind += 4) {
			_mm256_storeu_pd(LU + ind_, _mm256_loadu_pd(pTmpMem + ind));
		}
#endif
		for (; k < dimN; ++k) {
			LU[ind_++] = pTmpMem[ind++];
		}
	}
}

bool DecompLU(int dimN, double *pLU, int *pOrdering)
{
	for (int i = 0; i < dimN; ++i) {
		pOrdering[i] = i;
	}

	int ttii = 0;
	for (int i = 0; i < dimN; ++i, ttii += dimN) {
		int ttjj = ttii + dimN;
		double piv = pLU[ttii + i];
		int piv_rowId = i;
		int j;
		for (j = i + 1; j < dimN; ++j, ttjj += dimN) {
			if (abs(pLU[ttjj + i]) > abs(piv)) {
				piv = pLU[ttjj + i];
				piv_rowId = j;
			}
		}
		if (abs(piv) < OUREPS) {
			return false;
		}
		if (piv_rowId != i) {
			// row exchange in L-to-be and U-to-be
			int ttrr = dimN * piv_rowId;
			int j = 0;
#ifdef SSE_AVX_ACCELERATION
			for (; j + 3 < dimN; j += 4) {
				__m256d a0_ = _mm256_loadu_pd(pLU + ttii + j);
				__m256d b0_ = _mm256_loadu_pd(pLU + ttrr + j);
				_mm256_storeu_pd(pLU + ttrr + j, a0_);
				_mm256_storeu_pd(pLU + ttii + j, b0_);
			}
#endif
			for (; j < dimN; ++j) {
				std::swap(pLU[ttii + j], pLU[ttrr + j]);
			}
			std::swap(pOrdering[i], pOrdering[piv_rowId]);
		}
		ttjj = ttii + dimN;
		for (j = i + 1; j < dimN; ++j, ttjj += dimN) {
			double c = pLU[ttjj + i] / piv;
			pLU[ttjj + i] = c;
			int k = i + 1;
#ifdef SSE_AVX_ACCELERATION
			__m256d c_ = _mm256_set1_pd(c);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pLU + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pLU + ttii + k);
				__m256d d0_ = _mm256_mul_pd(b_, c_);
				a_ = _mm256_sub_pd(a_, d0_);
				_mm256_storeu_pd(pLU + ttjj + k, a_);
			}
#endif
			for (; k < dimN; ++k) {
				pLU[ttjj + k] -= c * pLU[ttii + k];
			}
		}
	}
	return true;
}

bool SolveLU(int dimN, const double *LU, const int *ordering,
	         int dimM, double *pB, double *pTmpMem)
{
	int ttii = 0;
	for (int i = 0; i < dimN; ++i, ttii += dimM) {
		int k = 0;
		int tt_oi = ordering[i] * dimM;
#ifdef SSE_AVX_ACCELERATION
		for (; k + 3 < dimM; k += 4) {
			__m256d a_ = _mm256_loadu_pd(pB + tt_oi + k);
			_mm256_storeu_pd(pTmpMem + ttii + k, a_);
		}
#endif
		for (; k < dimM; ++k) {
			pTmpMem[ttii + k] = pB[tt_oi + k];
		}
	}
	memcpy(pB, pTmpMem, sizeof(double) * dimN * dimM);

	ttii = 0;
	for (int i = 0; i < dimN - 1; ++i, ttii += dimM) {
		int ttjj = ttii + dimM, ttjjS = (i + 1) * dimN;
		for (int j = i + 1; j < dimN; ++j, ttjj += dimM, ttjjS += dimN) {
			int k = 0;
			double cc = LU[ttjjS + i];
#ifdef SSE_AVX_ACCELERATION
			__m256d c_ = _mm256_set1_pd(cc);
			for (; k + 3 < dimM; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pB + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pB + ttii + k);
				a_ = _mm256_sub_pd(a_, _mm256_mul_pd(b_, c_));
				_mm256_storeu_pd(pB + ttjj + k, a_);
			}
#endif
			for (; k < dimM; ++k) {
				pB[ttjj + k] -= cc * pB[ttii + k];
			}
		}
	}
	for (int i = dimN - 1; i >= 0; --i, ttii -= dimM) {
		double c0 = 1.0 / LU[i * dimN + i];
		int k = 0;
#ifdef SSE_AVX_ACCELERATION
		__m256d c_ = _mm256_set1_pd(c0);
		for (; k + 3 < dimM; k += 4) {
			__m256d a_ = _mm256_loadu_pd(pB + ttii + k);
			a_ = _mm256_mul_pd(a_, c_);
			_mm256_storeu_pd(pB + ttii + k, a_);
		}
#endif
		for (; k < dimM; ++k) {
			pB[ttii + k] *= c0;
		}
		int ttjj = ttii - dimM, ttjjS = (i-1) * dimN;
		for (int j = i - 1; j >= 0; --j, ttjj -= dimM, ttjjS -= dimN) {
			c0 = LU[ttjjS + i];
			k = 0;
#ifdef SSE_AVX_ACCELERATION
			c_ = _mm256_set1_pd(c0);
			for (; k + 3 < dimM; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pB + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pB + ttii + k);
				a_ = _mm256_sub_pd(a_, _mm256_mul_pd(b_, c_));
				_mm256_storeu_pd(pB + ttjj + k, a_);
			}
#endif
			for (; k < dimM; ++k) {
				pB[ttjj + k] -= c0 * pB[ttii + k];
			}
		}
	}
	return true;
}

void InvLU(int dimN, double *LU, const int *ordering, double *pTmpMem)
{
	for (int i = 0; i < dimN * dimN; ++i) {
		pTmpMem[i] = 0.0;
	}
	for (int i = 0; i < dimN; ++i) {
		pTmpMem[i * dimN + ordering[i]] = 1.0;
	}
	int ttii = 0;
	for (int i = 0; i < dimN - 1; ++i, ttii += dimN) {
		int ttjj = ttii + dimN, ttjjS = (i + 1) * dimN;
		for (int j = i + 1; j < dimN; ++j, ttjj += dimN, ttjjS += dimN) {
			int k = 0;
			double cc = LU[ttjjS + i];
#ifdef SSE_AVX_ACCELERATION
			__m256d c_ = _mm256_set1_pd(cc);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ttii + k);
				a_ = _mm256_sub_pd(a_, _mm256_mul_pd(b_, c_));
				_mm256_storeu_pd(pTmpMem + ttjj + k, a_);
			}
#endif
			for (; k < dimN; ++k) {
				pTmpMem[ttjj + k] -= cc * pTmpMem[ttii + k];
			}
		}
	}
	for (int i = dimN - 1; i >= 0; --i, ttii -= dimN) {
		double c0 = 1.0 / LU[i * dimN + i];
		int k = 0;
#ifdef SSE_AVX_ACCELERATION
		__m256d c_ = _mm256_set1_pd(c0);
		for (; k + 3 < dimN; k += 4) {
			__m256d a_ = _mm256_loadu_pd(pTmpMem + ttii + k);
			a_ = _mm256_mul_pd(a_, c_);
			_mm256_storeu_pd(pTmpMem + ttii + k, a_);
		}
#endif
		for (; k < dimN; ++k) {
			pTmpMem[ttii + k] *= c0;
		}
		int ttjj = ttii - dimN, ttjjS = (i - 1) * dimN;
		for (int j = i - 1; j >= 0; --j, ttjj -= dimN, ttjjS -= dimN) {
			c0 = LU[ttjjS + i];
			k = 0;
#ifdef SSE_AVX_ACCELERATION
			c_ = _mm256_set1_pd(c0);
			for (; k + 3 < dimN; k += 4) {
				__m256d a_ = _mm256_loadu_pd(pTmpMem + ttjj + k);
				__m256d b_ = _mm256_loadu_pd(pTmpMem + ttii + k);
				a_ = _mm256_sub_pd(a_, _mm256_mul_pd(b_, c_));
				_mm256_storeu_pd(pTmpMem + ttjj + k, a_);
			}
#endif
			for (; k < dimN; ++k) {
				pTmpMem[ttjj + k] -= LU[ttjjS + i] * pTmpMem[ttii + k];
			}
		}
	}
	memcpy(LU, pTmpMem, sizeof(double) * dimN * dimN);
}

bool DecompChol(int dimN, const double *A, double *pCholMat)
{
	if (pCholMat != A) {
		memcpy((void*)pCholMat, (const void*)A, sizeof(double) * dimN * dimN);
	}
	int IdimN = 0;
	for (int i = 0; i < dimN; ++i, IdimN += dimN) {
		double ssq = 0.0;
		int JdimN = 0;
		for (int j = 0; j < i; ++j, JdimN += dimN) {
			double x = A[IdimN + j];
			for (int k = 0; k < j; ++k) {
				x -= pCholMat[JdimN + k] * pCholMat[IdimN + k];
			}
			x /= pCholMat[JdimN + j];
			pCholMat[IdimN + j] = x;
			ssq += x * x;
		}
		ssq = A[IdimN + i] - ssq;
		if (ssq < OUREPS) {
			return false;
		}
		pCholMat[IdimN + i] = sqrt(ssq);
	}
	return true;
}

}