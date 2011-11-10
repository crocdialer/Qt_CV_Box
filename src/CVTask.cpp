#include "CVTask.h"

using namespace std;
using namespace cv;

Mat CVTask::colorOutput(const Mat& confMap,const Colormap &cm, const Size& s)
{
    if(confMap.empty()) return cm.apply(Mat(s,CV_8UC1,0.0));
    Size outSize = s;
    if(outSize==Size()) outSize = confMap.size(); 
    
    Mat tmp = confMap;
    
    //cv::normalize(confMap,tmp,0,255,CV_MINMAX);
    
    if(confMap.type() & CV_32F )
    {
        tmp = confMap * 255;
    }
    
    tmp.convertTo(tmp,CV_8UC1);
    
    Mat resized;
    resize (tmp, resized,outSize);
    
    // generate colormap-output
    tmp=cm.apply(resized);
    
    return tmp;
    
}

TaskXRay::TaskXRay():CVTask()
{
    m_colorMap = Colormap(Colormap::BONE);
};

Mat TaskXRay::doProcessing(const Mat &inFrame)
{
    Mat grayImg,out;
    cvtColor(inFrame, grayImg, CV_BGR2GRAY);
    
    bitwise_not(grayImg, grayImg);
    //out = m_colorMap.apply(grayImg);
    
    out = colorOutput(grayImg,m_colorMap);
    
    return out;
};

TaskThermal::TaskThermal():CVTask()
{
    m_colorMap = Colormap(Colormap::JET);
}

Mat TaskThermal::doProcessing(const Mat &inFrame)
{
    Mat grayImg,outMat;
    
    if(inFrame.channels() == 3) 
        cvtColor(inFrame, grayImg, CV_BGR2GRAY);
    else
        grayImg = inFrame;
    
    outMat = m_colorMap.apply(grayImg);
    
    return outMat;
}

TaskSalience::TaskSalience():CVTask()
{
    m_colorMap = Colormap(Colormap::BONE);
    
    
}

cv::Mat TaskSalience::doProcessing(const cv::Mat &inFrame)
{
    cv::Mat downSized,outMat;
    
    double ratio = 320 * 1. / inFrame.cols; 
    resize(inFrame, downSized, cv::Size(0,0), ratio, ratio, INTER_NEAREST);
    m_salienceDetect.updateSalience(downSized);
    m_salienceDetect.getSalImage(m_salImg);
    
    //resize(salImg,salImg,inFrame.size(),CV_INTER_LINEAR);
    
    outMat = colorOutput(m_salImg, m_colorMap) ;
    
    resize(outMat,outMat,inFrame.size(),CV_INTER_LINEAR);
    
    return outMat;
};


