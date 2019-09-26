#ifndef __DDR_COMMON_GEOMETRIC_FUNCTIONS_H_INCLUDED__
#define __DDR_COMMON_GEOMETRIC_FUNCTIONS_H_INCLUDED__

namespace DDRGeometry {

extern const double PI;
extern const double PI2;
extern const double HALFPI;
extern const float PI_f;
extern const float PI2_f;
extern const float HALFPI_f;
extern const double RAD2DEG;
extern const double DEG2RAD;
extern const float RAD2DEG_f;
extern const float DEG2RAD_f;

float pi2pif(float angInRad);
float WeightedAngleAddition_f(float ang1, float ang2, float alpha);
double pi2pi(double angInRad);
double WeightedAngleAddition(double ang1, double ang2, double alpha);
void GetRelative2DPose_f(float x0, float y0, float th0,
                         float x1, float y1, float th1,
						 float &x1_0, float &y1_0, float &th1_0);
void GetRelative2DPose(double x0, double y0, double th0,
	                   double x1, double y1, double th1,
					   double &x1_0, double &y1_0, double &th1_0);
void Get2DPoseFromBaseAndRelative(double x0, double y0, double th0,
	                              double x1_0, double y1_0, double th1_0,
								  double &x1, double &y1, double &th1);
void Get2DPoseFromBaseAndRelative_f(float x0, float y0, float th0,
	                                float x1_0, float y1_0, float th1_0,
									float &x1, float &y1, float &th1);

bool InterpolatePose(double x0, double y0, double th0,
	                 double lv0, double av0, unsigned long long ts0,
	                 double x1, double y1, double th1,
	                 double lv1, double av1, unsigned long long ts1,
	                 unsigned long long curTs,
	                 double *pX, double *pY, double *pTh,
	                 double *pLV, double *pAV);
bool ExtrapolatePose(double x0, double y0, double th0,
	                 double lv0, double av0, unsigned long long ts0,
	                 unsigned long long curTs,
	                 double *pX, double *pY, double *pTh,
					 double *pLV, double *pAV);
bool InterpolatePose_f(float x0, float y0, float th0,
	                   float lv0, float av0, unsigned long long ts0,
	                   float x1, float y1, float th1,
	                   float lv1, float av1, unsigned long long ts1,
	                   unsigned long long curTs,
	                   float *pX, float *pY, float *pTh,
	                   float *pLV, float *pAV);
bool ExtrapolatePose_f(float x0, float y0, float th0,
	                   float lv0, float av0, unsigned long long ts0,
	                   unsigned long long curTs,
	                   float *pX, float *pY, float *pTh,
	                   float *pLV, float *pAV);

// from ori to tar, rotate, scale, and then translate
bool CalcTrans_2D(int N, const double *pt_ori, const double *pt_tar,
	              double *pRot_Rad, double *pScale,
	              double *pTransX, double *pTransY);

// for a point cloud (PC) scanned at (x0, y0, th0),
// compute new points' if scanned at (x1, y1, th1).
bool MovePC(float x0, float y0, float th0,
	        float x1, float y1, float th1,
	        int nPts,
	        float *pAngles, int nAngInterval_bytes, float angVal2Rad,
	        float *pDist, int nDistInterval_bytes, float distVal2Meter);

}

#endif // __DDR_COMMON_GEOMETRIC_FUNCTIONS_H_INCLUDED__
