#include "stdafx.h"
#include "Basis.h"
//#include "glm.h"
#include <opencv/cv.h>

Basis* genOri(int phiDiv,int thetaDiv);
Basis getOri(int index,int phiDiv,int thetaDiv);
//CvMat* genDepthMap(Basis &ori,GLMmodel *model,int length);
IplImage* depthMap_to_sketch(CvMat *depthMap);