#ifndef __DDRMATHLIB_FAST_COV_BASED_LOOP_CLOSURE_H_INCLUDED__
#define __DDRMATHLIB_FAST_COV_BASED_LOOP_CLOSURE_H_INCLUDED__

namespace DDRGeometry {

/*! Fast 2D loop closure based on relative poses and covariances. 2D poses have 
	positions (X and Y in meter) and yaws (Th in radian).
*/
/*!
  \param NPoses number of poses in this sequence, with indices 0, 1, ..., NPose-1
  \param pX pointer to X of the first pose.
  \param XOff_Byte pointer offset of every next one to current X (in bytes).
  \param pY pointer to Y of the first pose.
  \param YOff_Byte pointer offset of every next one to current Y (in bytes).
  \param pTh pointer to Th of the first pose.
  \param ThOff_Byte pointer offset of every next one to current Th (in bytes).
  \param pCov pointer to Th of the first covariance matrix. Each matrix must be
         3x3 float, and it represents covariance of next to current pose.
  \param CovOff_Byte pointer offset of every next one to current Th (in bytes).
  \param lInd0 index of the first pose of the detected loop.
  \param lInd1 index of the second pose of the detected loop.
  \param plPose10 relative pose of the detected loop (three floats X, Y, Th)
  \param plCov10 covariance of the detected loop (3x3 float).
  \return whether loop is successfully closed
*/
bool FastLoopClosure_2D(int NPoses,
	                    float *pX, int XOff_Byte,
	                    float *pY, int YOff_Byte,
	                    float *pTh, int ThOff_Byte,
                        float *pCov, int CovOff_Byte,
	                    int lInd0, int lInd1,
	                    const float *plPose10,
	                    const float *plCov10);

}

#endif // __DDRMATHLIB_FAST_COV_BASED_LOOP_CLOSURE_H_INCLUDED__
