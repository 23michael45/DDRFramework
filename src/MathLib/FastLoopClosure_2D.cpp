#include "FastLoopClosure_2D.h"

#include <string.h>
#include <math.h>
#include <vector>
#include "CommonGeometry.h"

namespace DDRGeometry {

static float _DetSym3x3(const float *pMat3x3)
{
	return (pMat3x3[0] * (pMat3x3[4] * pMat3x3[8] - pMat3x3[5] * pMat3x3[5]) -
		    pMat3x3[1] * (pMat3x3[1] * pMat3x3[8] - pMat3x3[5] * pMat3x3[2]) +
		    pMat3x3[2] * (pMat3x3[1] * pMat3x3[5] - pMat3x3[4] * pMat3x3[2]));
}

static void _calcNextCov_inc(const float *pCurCov, const float *pDeltaCov,
	                         float x0, float y0, float th0,
	                         const float *pDPose,
	                         float *pNextCov)
{
	float ss = sinf(th0), cc = cosf(th0);
	float i0 = -pDPose[0] * ss - pDPose[1] * cc;
	float i1 = pDPose[0] * cc - pDPose[1] * ss;
	pNextCov[0] = pCurCov[0] + i0 * i0 * pCurCov[8];
	pNextCov[1] = pCurCov[1] + i0 * i1 * pCurCov[8];
	pNextCov[2] = pCurCov[2];
	pNextCov[4] = pCurCov[4] + i1 * i1 * pCurCov[8];
	pNextCov[5] = pCurCov[5];
	pNextCov[8] = pCurCov[8]; // = A¡® * Sig * A

	pNextCov[0] += cc * cc * pDeltaCov[0] - 2.0f * ss * cc * pDeltaCov[1] + ss * ss * pDeltaCov[4];
	pNextCov[1] += ss * cc * (pDeltaCov[0] - pDeltaCov[4]) + (cc * cc - ss * ss) * pDeltaCov[1];
	pNextCov[2] += cc * pDeltaCov[2] - ss * pDeltaCov[5];
	pNextCov[4] += cc * cc * pDeltaCov[4] + 2.0f * ss * cc * pDeltaCov[1] + ss * ss * pDeltaCov[0];
	pNextCov[5] += ss * pDeltaCov[2] + cc * pDeltaCov[5];
	pNextCov[8] += pDeltaCov[8]; // += B' * dSig * B

	pNextCov[3] = pNextCov[1];
	pNextCov[6] = pNextCov[2];
	pNextCov[7] = pNextCov[5];
}

static void _calcNextCov(const float *pCurCov, const float *pDeltaCov,
	                     float x0, float y0, float th0,
	                     float x1, float y1, float th1,
	                     float *pNextCov)
{
	float dpose[3];
	GetRelative2DPose_f(x0, y0, th0, x1, y1, th1, dpose[0], dpose[1], dpose[2]);
	_calcNextCov_inc(pCurCov, pDeltaCov, x0, y0, th0, dpose, pNextCov);
}

static void _fuseTwoPoses(const float *pPose0, const float *pCov0,
	                      const float *pPose1, const float *pCov1,
	                      float *pNewPose)
{
	float aa = pCov0[0] + pCov1[0];
	float bb = pCov0[1] + pCov1[1];
	float cc = pCov0[4] + pCov1[4];
	float idet = 1.0f / (aa * cc - bb * bb);
	float ia = idet * (pCov0[0] * cc - pCov0[1] * bb);
	float ib = idet * (-pCov0[0] * bb + pCov0[1] * aa);
	float ic = idet * (pCov0[1] * cc - pCov0[4] * bb);
	float id = idet * (-pCov0[1] * bb + pCov0[4] * aa);
	pNewPose[0] = pPose0[0] + ia * (pPose1[0] - pPose0[0]) + ib * (pPose1[1] - pPose0[1]);
	pNewPose[1] = pPose0[1] + ic * (pPose1[0] - pPose0[0]) + id * (pPose1[1] - pPose0[1]);
	pNewPose[2] = WeightedAngleAddition_f(pPose0[2], pPose1[2], pCov1[8] / (pCov0[8] + pCov1[8]));
}

bool FastLoopClosure_2D(int NPoses,
	                    float *pX, int XOff_Byte,
	                    float *pY, int YOff_Byte,
	                    float *pTh, int ThOff_Byte,
	                    float *pCov, int CovOff_Byte,
	                    int lInd0, int lInd1,
	                    const float *plPose10,
	                    const float *plCov10)
{
	if (NPoses <= 1 || !pX || !pY || !pTh || !pCov || !plPose10 || !plCov10 ||
		XOff_Byte < sizeof(float) || YOff_Byte < sizeof(float) ||
		ThOff_Byte < sizeof(float) || CovOff_Byte < sizeof(float) * 9) {
		return false;
	}
	if (lInd0 < 0 || lInd1 <= lInd0 || lInd1 >= NPoses) {
		return false;
	}
	float *pV0 = pCov, *pVX0, *pVY0, *pVTh0;
	for (int i = 0; i < NPoses; ++i) {
		if (pV0[0] <= 0.0f || pV0[0] * pV0[4] - pV0[1] * pV0[1] <= 0.0f ||
			_DetSym3x3(pV0) <= 0.0f) {
			return false;
		}
		pV0 = (float*)((char*)pV0 + CovOff_Byte);
	}

	// forward covariance accumulation
	std::vector<float> accCov_forward(9 * (lInd1 - lInd0));
	float tmpCov[9];
	for (int i = 0; i < 9; ++i) {
		tmpCov[i] = 0.0f;
	}
	pV0 = (float*)((char*)pCov + lInd0 * CovOff_Byte);
	pVX0 = (float*)((char*)pX + lInd0 * XOff_Byte);
	pVY0 = (float*)((char*)pY + lInd0 * YOff_Byte);
	pVTh0 = (float*)((char*)pTh + lInd0 * ThOff_Byte);
	for (int i = lInd0; i < lInd1; ++i) {
		float *pNextCov = (float*)((char*)pV0 + CovOff_Byte);
		float *pNextX = (float*)((char*)pVX0 + XOff_Byte);
		float *pNextY = (float*)((char*)pVY0 + YOff_Byte);
		float *pNextTh = (float*)((char*)pVTh0 + ThOff_Byte);
		_calcNextCov(tmpCov, pV0, *pVX0, *pVY0, *pVTh0,
			         *pNextX, *pNextY, *pNextTh,
			         &accCov_forward[9 * (i - lInd0)]);
		memcpy(tmpCov, &accCov_forward[9 * (i - lInd0)], sizeof(float) * 9);
		pV0 = pNextCov;
		pVX0 = pNextX;
		pVY0 = pNextY;
		pVTh0 = pNextTh;
	}

	// backward (ind1->ind0) covariance accumulation and backward poses
	std::vector<float> accCov_backward(9 * (lInd1 - lInd0));
	std::vector<float> pose_backward(3 * (lInd1 - lInd0));
	for (int i = 0; i < 9; ++i) {
		tmpCov[i] = 0.0f;
	}
	pV0 = (float*)((char*)pCov + lInd1 * CovOff_Byte);
	pVX0 = (float*)((char*)pX + lInd1 * XOff_Byte);
	pVY0 = (float*)((char*)pY + lInd1 * YOff_Byte);
	pVTh0 = (float*)((char*)pTh + lInd1 * ThOff_Byte);
	for (int i = lInd1; i > lInd0; --i) {
		if (i != lInd1) {
			float *pNextCov = (float*)((char*)pV0 - CovOff_Byte);
			float *pNextX = (float*)((char*)pVX0 - XOff_Byte);
			float *pNextY = (float*)((char*)pVY0 - YOff_Byte);
			float *pNextTh = (float*)((char*)pVTh0 - ThOff_Byte);
			float dpose[3];
			GetRelative2DPose_f(*pVX0, *pVY0, *pVTh0, *pNextX, *pNextY, *pNextTh,
				                dpose[0], dpose[1], dpose[2]);
			Get2DPoseFromBaseAndRelative_f(pose_backward[3 * (i - lInd0)],
				                           pose_backward[3 * (i - lInd0) + 1],
				                           pose_backward[3 * (i - lInd0) + 2],
										   dpose[0], dpose[1], dpose[2],
				                           pose_backward[3 * (i - lInd0) - 3],
				                           pose_backward[3 * (i - lInd0) - 2],
				                           pose_backward[3 * (i - lInd0) - 1]);
			_calcNextCov_inc(&accCov_backward[9 * (i - lInd0)], pV0, *pVX0, *pVY0, *pVTh0,
				             dpose, &accCov_backward[9 * (i - lInd0 - 1)]);
			pV0 = pNextCov;
			pVX0 = pNextX;
			pVY0 = pNextY;
			pVTh0 = pNextTh;
		} else {
			Get2DPoseFromBaseAndRelative_f(*((float*)((char*)pX + lInd0 * XOff_Byte)),
				                           *((float*)((char*)pY + lInd0 * YOff_Byte)),
				                           *((float*)((char*)pTh + lInd0 * ThOff_Byte)),
				                           plPose10[0], plPose10[1], plPose10[2],
				                           pose_backward[3 * (i - lInd0) - 3],
				                           pose_backward[3 * (i - lInd0) - 2],
				                           pose_backward[3 * (i - lInd0) - 1]);
			_calcNextCov_inc(tmpCov, plCov10, *((float*)((char*)pX + lInd0 * XOff_Byte)),
				             *((float*)((char*)pY + lInd0 * YOff_Byte)),
				             *((float*)((char*)pTh + lInd0 * ThOff_Byte)),
						     plPose10, &accCov_backward[9 * (i - lInd0 - 1)]);
		}
	}

	pVX0 = (float*)((char*)pX + (lInd0 + 1) * XOff_Byte);
	pVY0 = (float*)((char*)pY + (lInd0 + 1) * YOff_Byte);
	pVTh0 = (float*)((char*)pTh + (lInd0 + 1) * ThOff_Byte);
	for (int i = lInd0 + 1; i <= lInd1; ++i) {
		if (i == lInd1) {
			tmpCov[0] = *pVX0;
			tmpCov[1] = *pVY0;
			tmpCov[2] = *pVTh0;
		}

		float poseBefore[3] = { *pVX0, *pVY0, *pVTh0 }, fusedPose[3];
		_fuseTwoPoses(poseBefore, &accCov_forward[9 * (i - lInd0 - 1)],
			          &pose_backward[3 * (i - lInd0 - 1)], &accCov_backward[9 * (i - lInd0 - 1)],
			          fusedPose);
		*pVX0 = fusedPose[0];
		*pVY0 = fusedPose[1];
		*pVTh0 = fusedPose[2];

		if (i == lInd1) {
			float *px_ = (float*)((char*)pVX0 + XOff_Byte);
			float *py_ = (float*)((char*)pVY0 + YOff_Byte);
			float *pth_ = (float*)((char*)pVTh0 + ThOff_Byte);
			for (int j = lInd1 + 1; j < NPoses; ++j) {
				float dx, dy, dth;
				GetRelative2DPose_f(tmpCov[0], tmpCov[1], tmpCov[2],
					                *px_, *py_, *pth_, dx, dy, dth);
				Get2DPoseFromBaseAndRelative_f(*pVX0, *pVY0, *pVTh0,
					                           dx, dy, dth, *px_, *py_, *pth_);
				px_ = (float*)((char*)px_ + XOff_Byte);
				py_ = (float*)((char*)py_ + YOff_Byte);
				pth_ = (float*)((char*)pth_ + ThOff_Byte);
			}
		} else {
			pVX0 = (float*)((char*)pVX0 + XOff_Byte);
			pVY0 = (float*)((char*)pVY0 + YOff_Byte);
			pVTh0 = (float*)((char*)pVTh0 + ThOff_Byte);
		}
	}

	return true;
}


}