#ifndef CCFTHREAD_H
#define CCFTHREAD_H

#include "CVThread.h"
#include "CVTask.h"

//class CVTask;

class CCFThread : public CVThread
{
private:

	
	// used only for testing
	cv::HOGDescriptor m_plainHOG;
	
	
	std::list<cv::Mat> m_frameQueue;
	
	cv::Mat applyMap(const cv::Mat& src,const cv::Mat& floatMap);
    
    cv::Mat doProcessing(const FrameBundle &bundle) ;

    typedef boost::shared_ptr<CVTask> TaskPtr;
    
	typedef std::list<TaskPtr> TaskList;
    
    TaskList m_cvTasks;
    vector<Mat> processTasks(const Mat &img,const TaskList &tasks);
    
    // standard HOG-Detections
	void detectPersonsHOG(Mat& img);
    
public:
    CCFThread();
	~CCFThread();
	
	void addProcessingTask(CVTask *task);
    void addProcessingTask(const TaskPtr &task);
    
    TaskPtr getActiveTask(){return m_cvTasks.front();};

};

#endif // CCFTHREAD_H
