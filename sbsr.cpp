// Sketch-Based Shape Retrieval.cpp
// By Weilun Sun


#include "stdafx.h"
#include "Retriever.h"
#include <iostream>

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	/*Retriever retriever(200,10000,128,0.075,32,4,4,3,1.7,1.414,4,8);
	retriever.loadAndBuildSketch("E:\\swl\\NewModels\\objs");
	retriever.saveData("e:\\swl\\newdata.bin");
	retriever.loadData("e:\\swl\\lib_data");*/
	Retriever retriever(200,10000,128,0.075,32,4,4,3,1.7,1.414,4,8);
	retriever.loadData("E:\\swl\\model\\new\\data.bin","E:\\swl\\model\\new\\objs");
	retriever.addAndBuild();
	retriever.saveData("E:\\swl\\model\\new\\added.bin");
	/*IplImage* input = cvLoadImage("E:\\swl\\Sketch-Based Shape Retrieval\\query\\vase.png",CV_LOAD_IMAGE_GRAYSCALE);
	retriever.rankAll(input);
	GLMmodel *model;
	
	for(int i=0;i<retriever.getLibSize();i++)
	{
		cout<<"rank"<<i+1<<" : "<<retriever.getPath(i)<<endl;
		cout<<"score : "<<retriever.getScore(i)*100<<endl;
		model = glmReadOBJ(const_cast<char*>(retriever.getPath(i).c_str()));
		IplImage *best = depthMap_to_sketch(genDepthMap(retriever.getView(i),model,128));
		cvNamedWindow("Generated Sketch", 1);
		cvShowImage("Generated Sketch",best);
		cvWaitKey(0);
		cvDestroyWindow("Generated Sketch");
		cvReleaseImage(&best);
		cout<<endl;/*
	}*/
	//glmDelete(model);
	return 0;
}