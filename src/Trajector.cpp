#include "Trajector.h"
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

char morphVals7[] =
{
	0,0,1,1,1,0,0,
	0,1,1,1,1,1,0,
	1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,
	0,1,1,1,1,1,0,
	0,0,1,1,1,0,0,
};

char morphVals5[] =
{
	0,0,1,0,0,
	0,1,1,1,0,
	1,1,1,1,1,
	0,1,1,1,0,
	0,0,1,0,0
};


Trajector::Trajector(const Size& s,const int& history):m_size(s)
{
	
	initHistory(s,history);
	
	// define our element for erosion/dilation
	m_morphElement = Mat(5,5,CV_8UC1,morphVals5);
}


Trajector::~Trajector()
{
	
	
}

void Trajector::initHistory(const Size& s,const int& length)
{
	m_pastMats.clear();
	
	for (int i=0; i<length; i++) 
	{
		m_pastMats.push_back(Mat(s,CV_32F,0.0));
	}

}

void Trajector::filter(const Mat& src,Mat& dst)
{
	Mat histMean = m_pastMats.front();
	list<Mat>::iterator historyIt = m_pastMats.begin();
	historyIt++;
	
	// start weight for oldest historyFrame
	float minWeight = 0.2;
	float step = (1.f - minWeight) / (float) m_pastMats.size();
	
	for (float w=minWeight; historyIt != m_pastMats.end(); historyIt++,w+=step)
		histMean += *historyIt * w ;
		
	multiply(src+histMean, histMean, dst);
	
	//threshold(dst, dst, .25, 1.0,CV_THRESH_TOZERO);
	//erode(dst, dst, Mat());
	//dilate(dst, dst, Mat());
	
	// morphological open-operation
	morphologyEx(dst, dst, CV_MOP_OPEN, m_morphElement);
	
	morphologyEx(dst, dst, CV_MOP_CLOSE, m_morphElement);
	
	// normalizing
	double maxVal;
	minMaxIdx(dst, NULL, &maxVal);
	if(maxVal > 1.0) dst /= maxVal;
	
	m_pastMats.push_back(src.clone());
	m_pastMats.pop_front();
	
	
}