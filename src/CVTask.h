#ifndef CVTASK_H
#define CVTASK_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Colormap.h"
#include "FastSalience.h"

using namespace cv;

class CVTask 
{
private:
    
    float m_blendValue;
    
    cv::Mat m_lastResult;
    
    virtual cv::Mat doProcessing(const cv::Mat &inFrame) = 0;
    
public:
    CVTask():m_blendValue(.55){};
    virtual ~CVTask(){};
    
    void process(const cv::Mat &inFrame)
    {
        addWeighted(inFrame, (1-m_blendValue), doProcessing(inFrame),
                    m_blendValue, 0, m_lastResult);
        
        //m_lastResult = doProcessing(inFrame);
    } ;
    
    const cv::Mat& getResult() const {return m_lastResult;};
    
    void setBlendValue(float v){m_blendValue=v;};
    
    static Mat colorOutput(const Mat& confMap, const Colormap &cm = Colormap(), const Size& s=Size());
};

/****************************************************************************/

class TaskXRay : public CVTask 
{
private:
    Colormap m_colorMap;
    cv::Mat doProcessing(const cv::Mat &inFrame);
    
public:
    TaskXRay();
};

/****************************************************************************/

class TaskThermal : public CVTask 
{
private:
    Colormap m_colorMap;
    cv::Mat doProcessing(const cv::Mat &inFrame);
    
public:
    TaskThermal();
    
};

/****************************************************************************/

class TaskSalience : public CVTask 
{
    
private:
    
    Colormap m_colorMap;
    FastSalience m_salienceDetect;
    
    Mat m_salImg;
    
    

    cv::Mat doProcessing(const cv::Mat &inFrame);
    
public:
    
    TaskSalience();
    
};

/****************************************************************************/

class TaskThresh : public CVTask 
{
    
private:
    
    cv::Mat doProcessing(const cv::Mat &inFrame)
    {
        Mat outMat,threshImg,grayImg;
        
        cvtColor(inFrame, grayImg, CV_BGR2GRAY);
        threshold(grayImg, threshImg, 50, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        outMat = threshImg;
        
        //outMat = inFrame > 50;
        
        return outMat;
    };
    
};

#endif//CVTASK_H