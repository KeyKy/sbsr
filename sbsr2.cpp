// Sketch-Based Shape Retrieval.cpp
// By Weilun Sun

#include "stdafx.h"
#include "Retriever.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	printf("in\n");
	/*Retriever retriever(200,1000,0.075,32,4,4,3,1.7,1.414,4,8);
	retriever.loadAndBuild("e:\\swl\\Sketch-Based Shape Retrieval\\Lib4Sketch");
	retriever.saveData("..\\data.txt");
	retriever.loadData("..\\data.txt");
	IplImage* input = cvLoadImage("E:\\swl\\Sketch-Based Shape Retrieval\\query\\bike.png",CV_LOAD_IMAGE_GRAYSCALE);
	int mid = retriever.searchBest(input);
	GLMmodel *model;
	for(int i=0;i<retriever.modelFilePath.size();i++)
	{
		cout<<"rank"<<i+1<<" : "<<retriever.modelFilePath[retriever.rank[i].mid]<<endl;
		cout<<"score : "<<retriever.rank[i].score*100<<endl;
		model = glmReadOBJ(const_cast<char*>(retriever.modelFilePath[retriever.rank[i].mid].c_str()));
		IplImage *best = depthMap_to_sketch(genDepthMap(retriever.rank[i].viewIndex,model,128));
		cvNamedWindow("Generated Sketch", 1);
		cvShowImage("Generated Sketch",best);
		cvWaitKey(0);  
		cvDestroyWindow("Generated Sketch");
		cout<<endl;
	}*/
	return 0;
}

