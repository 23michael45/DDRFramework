#include "CommonGeometry.h"

#include <math.h>

namespace DDRGeometry {

extern const double PI = 3.141592653589793238463;
extern const float PI_f = 3.14159265359f;
extern const double PI2 = PI + PI;
extern const float PI2_f = PI_f + PI_f;
extern const double HALFPI = PI * 0.5;
extern const float HALFPI_f = PI_f * 0.5f;
extern const double RAD2DEG = 180.0 / PI;
extern const float RAD2DEG_f = 180.0f / PI_f;
extern const double DEG2RAD = PI / 180.0;
extern const float DEG2RAD_f = PI_f / 180.0f;

float pi2pif(float angInRad)
{
	int k = (int)floor(angInRad / PI2_f);
	angInRad -= k * PI2_f;
	if (angInRad > PI_f) {
		return (angInRad - PI2_f);
	} else {
		return angInRad;
	}
}

float WeightedAngleAddition_f(float ang1, float ang2, float alpha)
{
	// designed to properly add two angles
	// avoid +pi,-pi cross
	ang1 = pi2pif(ang1);
	ang2 = pi2pif(ang2);
	if (ang1 < ang2) {
		// make sure ang1 >= ang2
		float tt = ang2;
		ang2 = ang1;
		ang1 = tt;
		alpha = 1 - alpha;
	}
	if (ang1 - ang2 > PI_f) {
		// danger of +pi,-pi crossing
		ang2 += PI2_f;
	}
	float r = alpha * ang1 + (1 - alpha) * ang2;
	return pi2pif(r);
}

double pi2pi(double angInRad)
{
	int k = (int)floor(angInRad / PI2);
	angInRad -= k * PI2;
	if (angInRad > PI) {
		return (angInRad - PI2);
	} else {
		return angInRad;
	}
}

double WeightedAngleAddition(double ang1, double ang2, double alpha)
{
	// designed to properly add two angles
	// avoid +pi,-pi cross
	ang1 = pi2pi(ang1);
	ang2 = pi2pi(ang2);
	if (ang1 < ang2) {
		// make sure ang1 >= ang2
		double tt = ang2;
		ang2 = ang1;
		ang1 = tt;
		alpha = 1 - alpha;
	}
	if (ang1 - ang2 > PI) {
		// danger of +pi,-pi crossing
		ang2 += PI2;
	}
	double r = alpha * ang1 + (1 - alpha) * ang2;
	return pi2pi(r);
}

void GetRelative2DPose_f(float x0, float y0, float th0,
	                     float x1, float y1, float th1,
	                     float &x1_0, float &y1_0, float &th1_0)
{
	float cc = cosf(th0);
	float ss = sinf(th0);
	x1_0 = (x1 - x0) * cc + (y1 - y0) * ss;
	y1_0 = -(x1 - x0) * ss + (y1 - y0) * cc;
	th1_0 = pi2pif(th1 - th0);
}

void GetRelative2DPose(double x0, double y0, double th0,
	                   double x1, double y1, double th1,
	                   double &x1_0, double &y1_0, double &th1_0)
{
	double cc = cos(th0);
	double ss = sin(th0);
	x1_0 = (x1 - x0) * cc + (y1 - y0) * ss;
	y1_0 = -(x1 - x0) * ss + (y1 - y0) * cc;
	th1_0 = pi2pi(th1 - th0);
}

void Get2DPoseFromBaseAndRelative(double x0, double y0, double th0,
	                              double x1_0, double y1_0, double th1_0,
	                              double &x1, double &y1, double &th1)
{
	double cc = cos(th0);
	double ss = sin(th0);
	x1 = x0 + cc * x1_0 - ss * y1_0;
	y1 = y0 + ss * x1_0 + cc * y1_0;
	th1 = pi2pi(th0 + th1_0);
}

void Get2DPoseFromBaseAndRelative_f(float x0, float y0, float th0,
	                                float x1_0, float y1_0, float th1_0,
	                                float &x1, float &y1, float &th1)
{
	float cc = cosf(th0);
	float ss = sinf(th0);
	x1 = x0 + cc * x1_0 - ss * y1_0;
	y1 = y0 + ss * x1_0 + cc * y1_0;
	th1 = pi2pif(th0 + th1_0);
}

bool InterpolatePose(double x0, double y0, double th0,
	                 double lv0, double av0, unsigned long long ts0,
	                 double x1, double y1, double th1,
	                 double lv1, double av1, unsigned long long ts1,
	                 unsigned long long curTs,
	                 double *pX, double *pY, double *pTh,
	                 double *pLV, double *pAV)
{
	if (ts0 <= curTs && curTs < ts1 && fabs(pi2pi(th1 - th0)) <= HALFPI) {
		double rr = (ts1 - curTs + 0.0) / (ts1 - ts0);
		if (pX || pY) {
			//double d0 = (curTs - ts0) * lv0 * 0.001;
			//double d1 = (ts1 - curTs) * lv1 * 0.001;
			if (pX) {
				//*pX = rr * (x0 + d0 * cos(th0)) + (1.0 - rr) * (x1 - d1 * cos(th1));
				*pX = rr * x0 + (1.0 - rr) * x1;
			}
			if (pY) {
				//*pY = rr * (y0 + d0 * sin(th0)) + (1.0 - rr) * (y1 - d1 * sin(th1));
				*pY = rr * y0 + (1.0 - rr) * y1;
			}
		}
		if (pLV) {
			*pLV = rr * lv0 + (1.0 - rr) * lv1;
		}
		if (pTh) {
			//double a0 = (curTs - ts0) * av0 * 0.001;
			//double a1 = (ts1 - curTs) * av1 * 0.001;
			th1 = pi2pi(th1 - th0);
			th1 *= 1.0 - rr;
			//th1 = rr * a0 + (1.0 - rr) * (th1 - a1);
			*pTh = pi2pi(th0 + th1);
		}
		if (pAV) {
			*pAV = rr * av0 + (1.0 - rr) * av1;
		}
		return true;
	}
	return false;
}

bool ExtrapolatePose(double x0, double y0, double th0,
	                 double lv0, double av0, unsigned long long ts0,
	                 unsigned long long curTs,
	                 double *pX, double *pY, double *pTh,
	                 double *pLV, double *pAV)
{
	double td;
	if (curTs > ts0) {
		td = (curTs - ts0) * 0.001;
	} else {
		td = (ts0 - curTs) * (-0.001);
	}
	double _th = pi2pi(th0 + td * av0);
	if (pTh) {
		*pTh = _th;
	}
	td *= lv0;
	if (pX) {
		*pX = x0 + td * (cos(th0) + cos(_th)) * 0.5;
	}
	if (pY) {
		*pY = y0 + td * (sin(th0) + sin(_th)) * 0.5;
	}
	if (pLV) {
		*pLV = lv0;
	}
	if (pAV) {
		*pAV = av0;
	}
	return true;
}

bool InterpolatePose_f(float x0, float y0, float th0,
	                   float lv0, float av0, unsigned long long ts0,
	                   float x1, float y1, float th1,
	                   float lv1, float av1, unsigned long long ts1,
	                   unsigned long long curTs,
	                   float *pX, float *pY, float *pTh,
	                   float *pLV, float *pAV)
{
	if (ts0 <= curTs && curTs < ts1 && fabsf(pi2pif(th1 - th0)) <= HALFPI_f) {
		float rr = (ts1 - curTs + 0.0f) / (ts1 - ts0);
		if (pX || pY) {
			//float d0 = (curTs - ts0) * lv0 * 0.001f;
			//float d1 = (ts1 - curTs) * lv1 * 0.001f;
			if (pX) {
				//*pX = rr * (x0 + d0 * cosf(th0)) + (1.0f - rr) * (x1 - d1 * cosf(th1));
				*pX = rr * x0 + (1.0f - rr) * x1;
			}
			if (pY) {
				//*pY = rr * (y0 + d0 * sinf(th0)) + (1.0f - rr) * (y1 - d1 * sinf(th1));
				*pY = rr * y0 + (1.0f - rr) * y1;
			}
		}
		if (pLV) {
			*pLV = rr * lv0 + (1.0f - rr) * lv1;
		}
		if (pTh) {
			//float a0 = (curTs - ts0) * av0 * 0.001f;
			//float a1 = (ts1 - curTs) * av1 * 0.001f;
			th1 = pi2pif(th1 - th0);
			th1 *= 1.0f - rr;
			//th1 = rr * a0 + (1.0f - rr) * (th1 - a1);
			*pTh = pi2pif(th0 + th1);
		}
		if (pAV) {
			*pAV = rr * av0 + (1.0f - rr) * av1;
		}
		return true;
	}
	return false;
}

bool ExtrapolatePose_f(float x0, float y0, float th0,
	                   float lv0, float av0, unsigned long long ts0,
	                   unsigned long long curTs,
	                   float *pX, float *pY, float *pTh,
	                   float *pLV, float *pAV)
{
	float td;
	if (curTs > ts0) {
		td = (curTs - ts0) * 0.001f;
	} else {
		td = (ts0 - curTs) * (-0.001f);
	}
	float _th = pi2pif(th0 + td * av0);
	if (pTh) {
		*pTh = _th;
	}
	td *= lv0;
	if (pX) {
		*pX = x0 + td * (cosf(th0) + cosf(_th)) * 0.5f;
	}
	if (pY) {
		*pY = y0 + td * (sinf(th0) + sinf(_th)) * 0.5f;
	}
	if (pLV) {
		*pLV = lv0;
	}
	if (pAV) {
		*pAV = av0;
	}
	return true;
}

void _eigSymmPDDecomp2x2(const double *A, double *pBiggerEigVal, double *pEigAng)
{
	double eigV = 0.5 * (A[0] + A[3] + sqrt((A[0] - A[3]) * (A[0] - A[3]) + 4 * A[1] * A[1]));
	*pBiggerEigVal = eigV;
	*pEigAng = pi2pi(atan2(eigV - A[0], A[1]));
}

bool _solveSymmAb(int nSz, double *pMatA, double *pb)
{
	for (int i = 1; i < nSz; ++i) {
		for (int j = 0; j < i; ++j) {
			pMatA[i * nSz + j] = pMatA[j * nSz + i];
		}
	}
	// stage I, low triangluar elimination
	for (int i = 0; i < nSz; ++i) {
		int pivRowInd0 = i * nSz;
		double piv = pMatA[pivRowInd0 + i];
		if (fabs(piv) < 1e-12) {
			return false;
		}
		piv = 1.0 / piv;
		int rowInd0 = pivRowInd0 + nSz;
		for (int j = i + 1; j < nSz; ++j, rowInd0 += nSz) {
			for (int k = i + 1; k < nSz; ++k) {
				// v[j, k] -= v[j,i]*piv * v[i,k] (=v[i,j]*v[i,k]*piv)
				pMatA[rowInd0 + k] -= pMatA[pivRowInd0 + j] * pMatA[pivRowInd0 + k] * piv;
			}
			pb[j] -= pb[i] * pMatA[pivRowInd0 + j] * piv;
		}
		for (int j = i + 1; j < nSz; ++j) {
			pMatA[pivRowInd0 + j] *= piv;
		}
		pb[i] *= piv;
#if 0
		for (int i = 0; i < nSz; ++i) {
			for (int j = 0; j < nSz; ++j) {
				std::cout << pMatA[i * nSz + j] << "\t";
			}
			//std::cout << "\t\t" << pb[i] << std::endl;
		}
		std::cout << std::endl;
#endif
	}
	// stage II, off-diagonal elimination
	int rowPivInd = (nSz - 2) * (nSz + 1);
	for (int i = nSz - 2; i >= 0; --i) {
		for (int j = 1; j < nSz - i; ++j) {
			pb[i] -= pMatA[rowPivInd + j] * pb[i + j];
		}
		rowPivInd -= nSz + 1;
	}
	return true;
}

bool CalcTrans_2D(int N, const double *pt_ori, const double *pt_tar,
	              double *pRot_Rad, double *pScale,
	              double *pTransX, double *pTransY)
{
	if (N < 2 || !pt_ori || !pt_tar) {
		return false;
	}
	double X0 = 0.0, Y0 = 0.0, X1 = 0.0, Y1 = 0.0;
	for (int i = 0; i < N; ++i) {
		X0 += pt_ori[i * 2];
		Y0 += pt_ori[i * 2 + 1];
		X1 += pt_tar[i * 2];
		Y1 += pt_tar[i * 2 + 1];
	}
	X0 /= N, Y0 /= N, X1 /= N, Y1 /= N;
	double A[4] = { 0.0, 0.0, 0.0, 0.0 };
	double B[4] = { 0.0, 0.0, 0.0, 0.0 };
	for (int i = 0; i < N; ++i) {
		double dx = pt_ori[i * 2] - X0;
		double dy = pt_ori[i * 2 + 1] - Y0;
		A[0] += dx * dx;
		A[1] += dx * dy;
		A[3] += dy * dy;
		dx = pt_tar[i * 2] - X1;
		dy = pt_tar[i * 2 + 1] - Y1;
		B[0] += dx * dx;
		B[1] += dx * dy;
		B[3] += dy * dy;
	}
	double ang0, ang1, v0, v1;
	_eigSymmPDDecomp2x2(A, &v0, &ang0);
	_eigSymmPDDecomp2x2(B, &v1, &ang1);
	ang1 = pi2pi(ang1 - ang0);
	double cc = cos(ang1), ss = sin(ang1);

	double A_[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	double b_[3] = { 0.0, 0.0, 0.0 };
	for (int i = 0; i < N; ++i) {
		double xi = cc * pt_ori[i * 2] - ss * pt_ori[i * 2 + 1];
		double yi = ss * pt_ori[i * 2] + cc * pt_ori[i * 2 + 1];
		A_[0] += xi * xi + yi * yi;
		A_[1] -= xi;
		A_[2] -= yi;
		b_[0] += xi * pt_tar[i * 2] + yi * pt_tar[i * 2 + 1];
		b_[1] -= pt_tar[i * 2];
		b_[2] -= pt_tar[i * 2 + 1];
	}
	A_[3] = A_[1];
	A_[4] = N + 0.0;
	A_[6] = A_[2];
	A_[8] = N + 0.0;
	if (!_solveSymmAb(3, A_, b_)) {
		return false;
	}

	if (b_[0] < 0.0) {
		b_[0] = -b_[0];
		b_[1] = -b_[1];
		b_[1] = -b_[1];
		ang1 = pi2pi(ang1 + PI);
	}
	if (pRot_Rad) {
		*pRot_Rad = ang1;
	}
	if (pScale) {
		*pScale = b_[0];
	}
	if (pTransX) {
		*pTransX = -b_[1];
	}
	if (pTransY) {
		*pTransY = -b_[2];
	}
	return true;
}

bool MovePC(float x0, float y0, float th0,
	        float x1, float y1, float th1,
	        int nPts,
	        float *pAngles, int nAngInterval_bytes, float angVal2Rad,
	        float *pDist, int nDistInterval_bytes, float distVal2Meter)
{
	if (nPts <= 0 || !pAngles || !pDist ||
		nAngInterval_bytes < (int)sizeof(float) ||
		nDistInterval_bytes < (int)sizeof(float)) {
		return false;
	}
	x0 -= x1;
	y0 -= y1;
	for (int i = 0; i < nPts; ++i,
		pAngles = (float*)((char*)pAngles + nAngInterval_bytes),
		pDist = (float*)((char*)pDist + nDistInterval_bytes)) {
		(*pAngles) = angVal2Rad * (*pAngles) + th0;
		float cc = cosf(*pAngles), ss = sinf(*pAngles);
		float dd = (*pDist) * distVal2Meter;
		float x_ = x0 + dd * cc;
		float y_ = y0 + dd * ss;
		*pDist = sqrtf(x_ * x_ + y_ * y_) / distVal2Meter;
		*pAngles = pi2pif(atan2f(y_, x_) - th1) / angVal2Rad;
	}
	return true;
}

}
