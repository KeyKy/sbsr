#ifndef RETRIEVER_H
#define RETRIEVER_H


#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <direct.h>
#include <string>
#include<io.h>
#include <vector>
#include "Basis.h"
#include <algorithm>

using namespace std;

struct BestView
{
	int mid;
	int viewIndex;
	//Basis viewBasis;
	double score;
};

class Retriever{
private:
	string sketchPath;
	int viewWidth;
	int centerNum;
	int rowCount;
	int midIndex;
	int sampleNum;
	int featNum;
	float patchAreaRatio;
	int patchDiv;
	int tileNum;
	int oriNum;
	int iNu;
	float dSigma;
	float dF;
	int phiDiv;
	int thetaDiv;
	CvMat *sampleMat,*label,*featCenter,*vocabulary,*modelID;
	//void loadPass(string &path,int layer,int pass);//pass=0 : get centers   pass=1 : build histogram lib 
	void loadPassSketch(string &path,int layer,int pass);
	void decomposeToHistogram(int *arr,CvMat *featMat);
	string libPath;
	vector<string> oldModelFilePath;
	vector<string> modelFilePath;
	BestView* modelRank;
public:
	int getLibSize();
	string getLibPath();
	string getPath(int rank);
	Basis getView(int rank);
	double getScore(int rank);
	int getViewIndex(int rank);
	Retriever(string dataPath,string libPath,string sketchPath);
	Retriever(int centerNum,int sampleNum,int viewWidth,
		float patchAreaRatio,int patchDiv,int tileNum,int oriNum,
		int iNu,float dSigma,float dF,int phiDiv,int thetaDiv);
	void rankAll(IplImage *inputImage);
	//void loadAndBuild(string path);
	void loadAndBuildSketch(string path);
	void saveData(string path);
	void loadData(string path,string libPath="");
	int getViewWidth();
	void showBestView(int rank);
	void saveBestView(string root,int rank);
	void rankAllAndSave(IplImage* input,string root,int count);
	void reEvaluateScore(IplImage* input,int count,float sigma);
	~Retriever();

	void addAndBuild();
};

#endif
