#include "stdafx.h"

#include "Retriever.h"
#include "cvgabor.h"
#include "sketch_gen.h"
#include "Image.h"
//#include "glm.h"
#include "Basis.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <omp.h>

void long_fread(void *buffer,int size,int len,FILE *&file)
{
	int leftLen = len;
	void *pointer = buffer;
	while(leftLen>0)
	{
		leftLen -= fread(pointer,size,leftLen,file);
		pointer = (void*)(int(pointer)+(len-leftLen)*size);
	}
}

void long_fwrite(void *buffer,int size,int len,FILE *&file)
{
	int leftLen = len;
	void *pointer = buffer;
	while(leftLen>0)
	{
		leftLen -= fwrite(pointer,size,len,file);
		pointer = (void*)(int(pointer)+(len-leftLen)*size);
	}
}

bool cmp(BestView a,BestView b)//for sorting best view of each model by score
{
    if(a.score > b.score)
    {
        return true;
    }
    return false;
}

void saveMat(CvMat *mat,FILE *&writer,int type)//save a CvMat to file
{
	if(writer == NULL)
		return;
	fprintf(writer,"%d %d\n",mat->rows,mat->cols);
	for(int i=0;i<mat->cols*mat->rows;i++)
	{
		if(type == CV_32FC1)
			fprintf(writer,"%f ",mat->data.fl[i]);
		else
			fprintf(writer,"%d ",mat->data.i[i]);
	}
	fprintf(writer,"\n");
}

void loadMat(CvMat *&mat,FILE *&reader,int type)//load a CvMat from file
{
	if(reader == NULL)
		return;
	int rows,cols;
	fscanf(reader,"%d %d ",&rows,&cols);
	mat = cvCreateMat(rows,cols,type);
	for(int i=0;i<mat->cols*mat->rows;i++)
	{
		if(type == CV_32FC1)
			fscanf(reader,"%f ",&mat->data.fl[i]);
		else
			fscanf(reader,"%d ",&mat->data.i[i]);
	}
}

void saveStr_bin(string &str,FILE *&writer)
{
	if(writer == NULL)
		return;
	int len = str.length();
	long_fwrite(&len,sizeof(int),1,writer);
	long_fwrite(&str[0],sizeof(char),str.length(),writer);
}

void loadStr_bin(string &str,FILE *&reader)
{
	if(reader == NULL)
		return;
	int len;
	long_fread(&len,sizeof(int),1,reader);
	char *cstr = new char[len+1];
	long_fread(cstr,sizeof(char),len,reader);
	cstr[len] = 0;
	str.assign(cstr);
	delete []cstr;
}

void saveMat_bin(CvMat *mat,FILE *&writer)//save in binary
{
	if(writer == NULL)
		return;
	long_fwrite(&mat->rows,sizeof(mat->rows),1,writer);
	long_fwrite(&mat->cols,sizeof(mat->cols),1,writer);
	long_fwrite(mat->data.i,sizeof(int),mat->cols*mat->rows,writer);
}

void loadMat_bin(CvMat *&mat,FILE *&reader,int type)//load in binary
{
	if(reader == NULL)
		return;
	int rows,cols;
	long_fread(&rows,sizeof(rows),1,reader);
	long_fread(&cols,sizeof(cols),1,reader);
	mat = cvCreateMat(rows,cols,type);
	long_fread(mat->data.i,sizeof(int),mat->cols*mat->rows,reader);
}

Retriever::Retriever(string dataPath,string libPath,string sketchPath)
{
	label = NULL;
	featCenter = NULL;
	vocabulary = NULL;
	modelID = NULL;
	sampleMat = NULL;
	loadData(dataPath,libPath);

	this->sketchPath = sketchPath;
}

Retriever::Retriever(int centerNum,int sampleNum,int viewWidth,float patchAreaRatio,int patchDiv,int tileNum,int oriNum,int iNu,float dSigma,float dF,int phiDiv,int thetaDiv)
{
	this->viewWidth = viewWidth;
	this->patchAreaRatio = patchAreaRatio;
	this->patchDiv = patchDiv;
	this->tileNum = tileNum;
	this->oriNum = oriNum;
	this->iNu = iNu;
	this->dSigma = dSigma;
	this->dF = dF;
	this->centerNum = centerNum;
	this->sampleNum = sampleNum;
	this->phiDiv = phiDiv;
	this->thetaDiv = thetaDiv;
	label = NULL;
	featCenter = NULL;
	vocabulary = NULL;
	modelID = NULL;
	sampleMat = NULL;
}

//void Retriever::loadPass(string &path,int layer,int pass)//two pass loading process
//{
//	int cols = oriNum*tileNum*tileNum;
//	if(layer == 0)
//	{
//		featNum = 0;
//		midIndex = 0;
//		rowCount = 0;
//		if(pass == 1)
//		{
//			modelFilePath.clear();
//		}
//	}
//	struct _finddata_t filefind;
//	string  curr=path+"\\*.*";
//	int   done = 0,i,handle;
//	if((handle = _findfirst(curr.c_str(),&filefind)) == -1)
//		return;
//	while(!(done = _findnext(handle,&filefind)))
//	{
//		if(!strcmp(filefind.name,".."))
//			continue;
//		if((_A_SUBDIR == filefind.attrib)) //dir
//		{
//			  curr = path+"\\"+filefind.name;
//			  loadPass(curr,layer+1,pass);
//		}
//		else//file
//		{
//			string fileName = filefind.name;
//			bool illegal = fileName.find(".obj",fileName.length()-4) == string::npos;
//			if(illegal)
//				continue;
//			string fullPath = path+"\\"+fileName;
//			cout<<"pass"<<pass<<" : "<<fullPath<<endl;
//
//			if(pass == 1)
//			{
//				modelFilePath.push_back(fullPath);
//			}
//			GLMmodel *model = glmReadOBJ(const_cast<char*>(fullPath.c_str()));
//			//printf("    OBJ is read.\n");
//			Basis *ori=genOri(phiDiv,thetaDiv);
//			for(int i=0;i<phiDiv*thetaDiv;i++)
//			{
//				Image* image = new Image;
//				image->set(patchAreaRatio,patchDiv,tileNum,oriNum,iNu,dSigma,dF);
//				//printf("......\n");
//				image->iplImage=depthMap_to_sketch(genDepthMap(ori[i],model,viewWidth));
//				CvMat *featMat = image->getFeatMat();
//				if(pass == 0)
//				{
//					CvRNG rng = cvRNG(-1);
//					int k;
//					for(int x=0;x<featMat->rows;x++)
//					{
//						if(rowCount < sampleNum)
//							k = rowCount;
//						else
//							k = cvRandInt(&rng)%rowCount;
//						rowCount++;
//						if(k >= sampleNum)
//							continue;
//						for(int y=0;y<featMat->cols;y++)
//							sampleMat->data.fl[k*featMat->cols+y] = featMat->data.fl[x*featMat->cols+y];
//					}
//				}
//				else
//				{
//					modelID->data.i[featNum] = midIndex;
//					decomposeToHistogram(vocabulary->data.i+featNum*centerNum,featMat);
//				}
//				cvReleaseMat(&featMat);
//				delete image;
//				featNum++;
//				//printf("        View%d is generated.\n",i);
//			}
//			midIndex++;
//			glmDelete(model);
//			delete []ori;
//		}
//	}
//	_findclose(handle);
//	if(layer > 0 || pass == 1)
//		return;
//	modelID = cvCreateMat(1,featNum,CV_32SC1);
//	sampleNum = (sampleNum < rowCount)? sampleNum : rowCount;
//	label = cvCreateMat(sampleNum,1,CV_32SC1);
//	for(int i=0;i<sampleNum;i++)
//		label->data.i[i]=i%centerNum;
//	featCenter = cvCreateMat(centerNum,cols,CV_32FC1);
//	cvKMeans2(sampleMat,centerNum,label,
//		cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,10,1.0),5,0,cv::KMEANS_USE_INITIAL_LABELS,featCenter);
//	cvReleaseMat(&label);
//}

void Retriever::loadPassSketch(string &path,int layer,int pass)//two pass loading process
{
	int cols = oriNum*tileNum*tileNum;
	if(layer == 0)
	{
		featNum = 0;
		midIndex = 0;
		rowCount = 0;
	}
	struct _finddata_t filefind;
	string  curr=path+"\\*.*";
	int   done = 0,i,handle;
	if((handle = _findfirst(curr.c_str(),&filefind)) == -1)
		return;
	while(!(done = _findnext(handle,&filefind)))
	{
		if(!strcmp(filefind.name,".."))
			continue;
		if((_A_SUBDIR == filefind.attrib)) //dir
		{
			  curr = path+"\\"+filefind.name;
			  loadPassSketch(curr,layer+1,pass);
		}
		else//file
		{
			string fileName = filefind.name;
			bool illegal = fileName.find(".obj",fileName.length()-4) == string::npos;
			if(illegal)
				continue;
			string fullPath = path+"\\"+fileName;
			if(binary_search(oldModelFilePath.begin(),oldModelFilePath.end(),fullPath))
				continue;
			cout<<"pass"<<pass<<" : "<<fullPath<<endl;
			
			if(pass == 1)
			{
				modelFilePath.push_back(fullPath);
			}
			
			//GLMmodel *model = glmReadOBJ(const_cast<char*>(fullPath.c_str()));
			//printf("    OBJ is read.\n");
			Basis *ori=genOri(phiDiv,thetaDiv);
			
			for(int i=0;i<phiDiv*thetaDiv;i++)
			{
				Image* image = new Image;
				image->set(patchAreaRatio,patchDiv,tileNum,oriNum,iNu,dSigma,dF);
				//printf("......\n");
				string OBJ_PATH = libPath;
				string SKETCH_PATH = libPath.substr(0,OBJ_PATH.length()-4)+"sketches";
				string sketchPath = fullPath.substr(OBJ_PATH.length(),fullPath.length()-OBJ_PATH.length()-4);
				//cout<<sketchPath<<endl;
				char buf[500];
				sprintf(buf,"%s%s_%d.png",SKETCH_PATH.c_str(),sketchPath.c_str(),i);
				//cout <<buf <<endl;
				//getchar();
				image->iplImage=cvLoadImage(buf,0);//depthMap_to_sketch(genDepthMap(ori[i],model,viewWidth));
				
				
				CvMat *featMat = image->getFeatMat();
				if(pass == 0)
				{
					CvRNG rng = cvRNG(-1);
					int k;
					for(int x=0;x<featMat->rows;x++)
					{
						if(rowCount < sampleNum)
							k = rowCount;
						else
							k = cvRandInt(&rng)%rowCount;
						rowCount++;
						if(k >= sampleNum)
							continue;
						for(int y=0;y<featMat->cols;y++)
							sampleMat->data.fl[k*featMat->cols+y] = featMat->data.fl[x*featMat->cols+y];
					}
				}
				else
				{
					modelID->data.i[featNum] = midIndex;
					decomposeToHistogram(vocabulary->data.i+featNum*centerNum,featMat);
				}
				cvReleaseMat(&featMat);
				delete image;
				featNum++;
				//printf("        View%d is generated.\n",i);
			}
			midIndex++;
			//glmDelete(model);
			delete []ori;
		}
	}
	_findclose(handle);
	if(layer > 0 || pass == 1 || oldModelFilePath.size()>0)
		return;
	modelID = cvCreateMat(1,featNum,CV_32SC1);
	sampleNum = (sampleNum < rowCount)? sampleNum : rowCount;
	label = cvCreateMat(sampleNum,1,CV_32SC1);
	for(int i=0;i<sampleNum;i++)
		label->data.i[i]=i%centerNum;
	featCenter = cvCreateMat(centerNum,cols,CV_32FC1);
	cvKMeans2(sampleMat,centerNum,label,
		cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER,10,1.0),5,0,cv::KMEANS_USE_INITIAL_LABELS,featCenter);
	cvReleaseMat(&label);
}

Retriever::~Retriever()
{
	if(sampleMat)
		cvReleaseMat(&sampleMat);
	if(label)
		cvReleaseMat(&label);
	if(featCenter)
		cvReleaseMat(&featCenter);
	if(vocabulary)
		cvReleaseMat(&vocabulary);
	if(modelID)
		cvReleaseMat(&modelID);
	//delete []modelRank;
}

void Retriever::decomposeToHistogram(int *arr,CvMat *featMat)
{
	for(int i=0;i<centerNum;i++)
		arr[i] = 0;
	#pragma omp parallel for
	for(int i=0;i<featMat->rows;i++)
	{
		int minFeatNum;
		float minDist;
		bool updated = false;
		for(int k=0;k<centerNum;k++)
		{
			float dist = 0;
			for(int j=0;j<featMat->cols;j++)
			{
				float delta = featMat->data.fl[i*featMat->cols+j]-featCenter->data.fl[k*featMat->cols+j];
				dist += delta*delta;
			}
			dist = sqrt(dist);
			if(!updated || dist < minDist)
			{
				updated = true;
				minDist = dist;
				minFeatNum = k;
			}
		}
		arr[minFeatNum]++;
	}
}

void Retriever::saveData(string path)//save all data
{
	FILE *writer;
	writer = fopen(path.c_str(),"wb");
	long_fwrite(&viewWidth,sizeof(int),1,writer);
	long_fwrite(&patchAreaRatio,sizeof(int),1,writer);
	long_fwrite(&patchDiv,sizeof(int),1,writer);
	long_fwrite(&tileNum,sizeof(int),1,writer);
	long_fwrite(&oriNum,sizeof(int),1,writer);
	long_fwrite(&iNu,sizeof(int),1,writer);
	long_fwrite(&dSigma,sizeof(int),1,writer);
	long_fwrite(&dF,sizeof(int),1,writer);
	long_fwrite(&centerNum,sizeof(int),1,writer);
	long_fwrite(&sampleNum,sizeof(int),1,writer);
	long_fwrite(&phiDiv,sizeof(int),1,writer);
	long_fwrite(&thetaDiv,sizeof(int),1,writer);
	saveMat_bin(featCenter,writer);
	saveMat_bin(vocabulary,writer);
	saveMat_bin(modelID,writer);
	saveStr_bin(libPath,writer);
	int size = modelFilePath.size();
	long_fwrite(&size,sizeof(int),1,writer);
	for(int i=0;i<modelFilePath.size();i++)
	{
		saveStr_bin(modelFilePath[i].substr(libPath.length(),modelFilePath[i].length()),writer);
	}
	fclose(writer);
}

void Retriever::loadData(string path,string libPath)//load all data
{
	FILE *reader;
	reader = fopen(path.c_str(),"rb");
	long_fread(&viewWidth,sizeof(int),1,reader);
	long_fread(&patchAreaRatio,sizeof(int),1,reader);
	long_fread(&patchDiv,sizeof(int),1,reader);
	long_fread(&tileNum,sizeof(int),1,reader);
	long_fread(&oriNum,sizeof(int),1,reader);
	long_fread(&iNu,sizeof(int),1,reader);
	long_fread(&dSigma,sizeof(int),1,reader);
	long_fread(&dF,sizeof(int),1,reader);
	long_fread(&centerNum,sizeof(int),1,reader);
	long_fread(&sampleNum,sizeof(int),1,reader);
	long_fread(&phiDiv,sizeof(int),1,reader);
	long_fread(&thetaDiv,sizeof(int),1,reader);
	
	loadMat_bin(featCenter,reader,CV_32FC1);
	loadMat_bin(vocabulary,reader,CV_32SC1);
	loadMat_bin(modelID,reader,CV_32SC1);
	
	if(libPath.length()>0)
	{
		loadStr_bin(this->libPath,reader);
		this->libPath = libPath;
	}
	else
		loadStr_bin(this->libPath,reader);
	cout<<this->libPath<<endl;
	modelFilePath.clear();
	int size;
	long_fread(&size,sizeof(int),1,reader);
	string file;
	for(int i=0;i<size;i++)
	{
		loadStr_bin(file,reader);
		modelFilePath.push_back(this->libPath+file);
	}
	fclose(reader);
}

void Retriever::rankAll(IplImage *inputImage)//search best model and its best view and sort all models
{
	modelRank = new BestView[modelFilePath.size()];
	Image *image = new Image;
	image->iplImage = inputImage;
	image->set(patchAreaRatio,patchDiv,tileNum,oriNum,iNu,dSigma,dF);
	CvMat *target = image->getFeatMat();
	int *tarHistogram = new int[featCenter->rows];
	for(int k=0;k<modelFilePath.size();k++)
		modelRank[k].score = 0;
	decomposeToHistogram(tarHistogram,target);
	//get the histogram of input image

	/*cout<<"hist:";
	for(int j=0;j<vocabulary->cols;j++)
				cout<<tarHistogram[j]<<' ';
	cout<<endl;*/

	
	cvReleaseMat(&target);
	double maxDot = 0;
	int *wordFreq = new int[vocabulary->cols];
	for(int k=0;k<vocabulary->cols;k++)
		wordFreq[k] = 0;
	for(int i=0;i<vocabulary->rows;i++)
	{
		for(int j=0;j<vocabulary->cols;j++)
		{
			wordFreq[j] += vocabulary->data.i[i*vocabulary->cols+j]; 
		}
	}


	/*for(int i=0;i<vocabulary->cols;i++)
		cout<<wordFreq[i]<<' ';
	cout<<endl;*/


	for(int i=0;i<vocabulary->rows;i++)
	{
		double dot = 0;
		double len1 = 0;
		double len2 = 0;
		for(int j=0;j<vocabulary->cols;j++)
		{
			double weight = 1;// tarHistogram[j]*log(vocabulary->rows/double(wordFreq[j]));
			dot += tarHistogram[j]*vocabulary->data.i[i*vocabulary->cols+j]*weight;
			len1 += tarHistogram[j]*tarHistogram[j]*weight*weight;
			len2 += vocabulary->data.i[i*vocabulary->cols+j]*vocabulary->data.i[i*vocabulary->cols+j];
		}
		len1 = sqrt(len1);
		len2 = sqrt(len2);
		dot /= len1*len2;

		if(dot >= modelRank[modelID->data.i[i]].score)
		{
			modelRank[modelID->data.i[i]].mid = modelID->data.i[i];
			modelRank[modelID->data.i[i]].score = dot;
			modelRank[modelID->data.i[i]].viewIndex = i%(phiDiv*thetaDiv);
			//modelRank[modelID->data.i[i]].viewBasis = getOri(i%(phiDiv*thetaDiv),phiDiv,thetaDiv);
		}
	}
	//for(int k=0;k<modelFilePath.size();k++)
		//cout<<"score = "<<rank[k].score<<"  from:"<<modelFilePath[k]<<endl;
	//cout<<"score = "<<maxDot<<endl;
	
	delete image;
	delete []tarHistogram;
	delete []wordFreq;
	sort(modelRank,modelRank+modelFilePath.size(),cmp);
}

//void Retriever::loadAndBuild(string path)
//{
//	libPath = path;
//	sampleMat = cvCreateMat(sampleNum,oriNum*tileNum*tileNum,CV_32FC1);
//	loadPass(path,0,0);
//	//pass0 : load models and get cluster centers
//	cvReleaseMat(&sampleMat);
//	vocabulary = cvCreateMat(featNum,centerNum,CV_32SC1);
//	modelID = cvCreateMat(1,featNum,CV_32SC1);
//	loadPass(path,0,1);
//	//pass1 : decompose each view of models to centers to form its histogram
//}

void Retriever::loadAndBuildSketch(string path)
{
	libPath = path;
	sampleMat = cvCreateMat(sampleNum,oriNum*tileNum*tileNum,CV_32FC1);
	loadPassSketch(path,0,0);
	//pass0 : load models and get cluster centers
	cvReleaseMat(&sampleMat);
	vocabulary = cvCreateMat(featNum,centerNum,CV_32SC1);
	modelID = cvCreateMat(1,featNum,CV_32SC1);
	loadPassSketch(path,0,1);
	//pass1 : decompose each view of models to centers to form its histogram
}

string Retriever::getPath(int rank)
{
	if(rank >= 0 && rank < modelFilePath.size())
		return modelFilePath[modelRank[rank].mid];
	return "";
}
Basis Retriever::getView(int rank)
{
	if(rank >= 0 && rank < modelFilePath.size()){
		return getOri(modelRank[rank].viewIndex%(phiDiv*thetaDiv),phiDiv,thetaDiv);
		//return viewBasis;
	}
	Basis error;
	/*error.front = Vec(0,0,0);
	error.up = Vec(0,0,0);
	error.right = Vec(0,0,0);*/
	return error;
}
double Retriever::getScore(int rank)
{
	if(rank >= 0 && rank < modelFilePath.size())
		return modelRank[rank].score;
	return 0;
}

int Retriever::getLibSize()
{
	return modelFilePath.size();
}

int Retriever::getViewWidth()
{
	return viewWidth;
}

string Retriever::getLibPath()
{
	return libPath;
}

int Retriever::getViewIndex(int rank)
{
	return modelRank[rank].viewIndex;
}

void Retriever::showBestView(int rank)
{
	printf("rank%d : %f\n",rank+1,getScore(rank));
	char path[500];
	string root = "E:\\swl\\DataBase\\sketches";
	string room = getPath(rank).substr(libPath.size(),getPath(rank).size()-libPath.size()-4);
	sprintf(path,"%s%s_%d.png",root.c_str(),room.c_str(),getViewIndex(rank));
	printf("    %s\n\n",path);
	IplImage *image = cvLoadImage(path,0);
	cvNamedWindow("Generated Sketch", 1);
	cvShowImage("Generated Sketch",image);  
	cvWaitKey(0);  
	cvDestroyWindow("Generated Sketch");
}

void Retriever::saveBestView(string root,int rank)
{
	string sketchRoot = "E:\\swl\\DataBase\\sketches";
	char path[500];
	string room = getPath(rank).substr(libPath.size(),getPath(rank).size()-libPath.size()-4);
	sprintf(path,"%s%s_%d.png",sketchRoot.c_str(),room.c_str(),getViewIndex(rank));
	IplImage *image = cvLoadImage(path,0);
	sprintf(path,"%s\\rank%d.png",root.c_str(),rank+1);
	cvSaveImage(path,image);
	cvReleaseImage(&image);
	sprintf(path,"%s\\rank%d.txt",root.c_str(),rank+1);
	FILE *out = fopen(path,"w");
	fprintf(out,"%f\n",getScore(rank));
	fclose(out);
}

void Retriever::rankAllAndSave(IplImage* input,string root,int count)
{
	_mkdir(const_cast<char*>(root.c_str()));
	cvSaveImage(const_cast<char*>((root+"\\query.png").c_str()),input);
	rankAll(input);
	for(int i=0;i<count;i++)
		saveBestView(root,i);

}

void Retriever::reEvaluateScore(IplImage* input,int count,float sigma)
{
	//if(count>modelFilePath.size())
	//	count = modelFilePath.size();
	//rankAll(input);

	//double scWeight = 0.9;
	//double affWeight = 0.1;
	//double scCost,affCost;

	////D:\Lab\Ideas\Sketch2Scene\database\sketches
	//string sketchRoot = sketchPath;
	////"E:\\swl\\sketches";
	////string sketchRoot = "D:/Lab/Ideas/Sketch2Scene/database/sketches";
	//char path[500];
	//for(int rank=0;rank<count;rank++)
	//{
	//	string room = getPath(rank).substr(libPath.size(),getPath(rank).size()-libPath.size()-4);
	//	sprintf(path,"%s%s_%d.png",sketchRoot.c_str(),room.c_str(),getViewIndex(rank));
	//	IplImage *image = cvLoadImage(path,0);
	//	CmShapeContext::shapeMatch(image,input,scCost,affCost);
	//	cvReleaseImage(&image);
	//	double totalCost = scCost*scWeight + affCost*affWeight;
	//	modelRank[rank].score = exp(-sigma*(totalCost-0.2));

	//	//std::cout << "p = " << path << "\n";
	//}
	//sort(modelRank,modelRank+count,cmp);

	//std::cout << "best score = " << modelRank[0].score << "\n";

	////testing
	//for(int rank=0;rank<1;rank++)
	//{
	//	string room = getPath(rank).substr(libPath.size(),getPath(rank).size()-libPath.size()-4);
	//	for(int j=0;j<32;j++){
	//		sprintf(path,"%s%s_%d.png",sketchRoot.c_str(),room.c_str(),j);
	//		IplImage *image = cvLoadImage(path,0);
	//		CmShapeContext::shapeMatch(image,input,scCost,affCost);
	//		cvReleaseImage(&image);
	//		double totalCost = scCost*scWeight + affCost*affWeight;
	//		std::cout << "view " << j << ", score = " << totalCost <<"\n";
	//	}

	//}
}

void Retriever::addAndBuild()
{
	CvMat *oldVocabulary,*oldModelID;
	oldVocabulary = vocabulary;
	oldModelID = modelID;
	oldModelFilePath.resize(modelFilePath.size());
	copy(modelFilePath.begin(),modelFilePath.end(),oldModelFilePath.begin());
	sort(oldModelFilePath.begin(),oldModelFilePath.end());
	sampleMat = cvCreateMat(sampleNum,oriNum*tileNum*tileNum,CV_32FC1);
	loadPassSketch(libPath,0,0);
	cvReleaseMat(&sampleMat);

	if(featNum == 0)
	{
		oldModelFilePath.clear();
		return;
	}

	vocabulary = cvCreateMat(featNum,centerNum,CV_32SC1);
	modelID = cvCreateMat(1,featNum,CV_32SC1);
	
	loadPassSketch(libPath,0,1);
	CvMat *newVocabulary,*newModelID;
	newVocabulary = vocabulary;
	newModelID = modelID;
	vocabulary = cvCreateMat(oldVocabulary->rows+newVocabulary->rows,
		oldVocabulary->cols,oldVocabulary->type);
	modelID = cvCreateMat(1,newModelID->cols+oldModelID->cols,oldModelID->type);
	for(int i=0;i<oldVocabulary->rows*oldVocabulary->cols;i++)
		vocabulary->data.i[i] = oldVocabulary->data.i[i];
	for(int i=0;i<newVocabulary->rows*newVocabulary->cols;i++)
		vocabulary->data.i[oldVocabulary->rows*oldVocabulary->cols+i] = newVocabulary->data.i[i];
	for(int i=0;i<oldModelID->cols;i++)
		modelID->data.i[i] = oldModelID->data.i[i];
	for(int i=0;i<newModelID->cols;i++)
		modelID->data.i[oldModelID->cols+i] = 1+oldModelID->data.i[oldModelID->cols-1]+oldModelID->data.i[i];

	oldModelFilePath.clear();
	cvReleaseMat(&newVocabulary);
	cvReleaseMat(&oldVocabulary);
	cvReleaseMat(&newModelID);
	cvReleaseMat(&oldModelID);
}