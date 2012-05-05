#include "CCFThread.h"
#include "CVTask.h"

//#include "FastSalience.h"
#include <fstream>

#include <boost/thread/thread.hpp>

using namespace cv;
using namespace boost;

CCFThread::CCFThread():CVThread()
{
	// set people detector standard HOG
	m_plainHOG.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    
//    addProcessingTask( new TaskXRay() );
    addProcessingTask( new TaskThermal() );
//    addProcessingTask( new TaskThresh() );
//    addProcessingTask(new TaskSalience());
    
}

CCFThread::~CCFThread()
{
	
}

Mat CCFThread::doProcessing(const FrameBundle& bundle)
{	
    Mat out;
    
    
    vector<Mat> results = processTasks(bundle.m_inFrame, m_cvTasks);
    out = results.empty() ? bundle.m_inFrame : results.front();
    
    return out;
}

void CCFThread::addProcessingTask(CVTask *task)
{
    addProcessingTask(TaskPtr(task));
}

void CCFThread::addProcessingTask(const TaskPtr &task)
{
    m_cvTasks.push_back(task);
}

vector<Mat> CCFThread::processTasks(const Mat &img,const TaskList &tasks)
{
    vector<Mat> retVec;
    TaskList::const_iterator it = tasks.begin();
    for (; it!=m_cvTasks.end(); it++) 
    {
        (*it)->process(img);
        retVec.push_back( (*it)->getResult() );
    }
    
    return retVec;
}


Mat CCFThread::applyMap(const Mat& src,const Mat& floatMap)
{
	Mat ret;
	vector<Mat> splits;
	src.convertTo(ret, CV_32F);
	
	
	split(ret, splits);
	
	for(uint i=0;i<splits.size();i++)
	{	
		multiply(splits[i], floatMap , splits[i]);
	}
	
	merge(splits,ret);
	
    //Mat bla;
    //multiply(src, floatMap, bla);
    
	ret.convertTo(ret, CV_8U);
	return ret;
}

void CCFThread::detectPersonsHOG(Mat& img)
{
	vector<Rect> found, found_filtered;
	
	// run the detector with default parameters. to get a higher hit-rate
	// (and more false alarms, respectively), decrease the hitThreshold and
	// groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
	//int can = img.channels();
	//cv::gpu::GpuMat gpuImg = cv::gpu::GpuMat(img);
	
	m_plainHOG.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, .9);
	
	size_t i, j;
	for( i = 0; i < found.size(); i++ )
	{
		Rect r = found[i];
		for( j = 0; j < found.size(); j++ )
			if( j != i && (r & found[j]) == r)
				break;
		if( j == found.size() )
			found_filtered.push_back(r);
	}
	for( i = 0; i < found_filtered.size(); i++ )
	{
		Rect r = found_filtered[i];
		// the HOG detector returns slightly larger rectangles than the real objects.
		// so we slightly shrink the rectangles to get a nicer output.
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0,200,255), 2);
	}
	
	
}