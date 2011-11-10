#ifndef CVBUFFERTHREAD_H
#define CVBUFFERTHREAD_H

#include <QThread>
#include <QSemaphore>

#include "ThreadSafeQueue.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QTcpSocket>
#include "axiscamera.h"

#include "opencv2/opencv.hpp"

class CVBufferThread : public QThread
{
	Q_OBJECT
	
private:
	
	volatile bool m_stopped;
	unsigned int m_numSlots;
	
	ThreadSafeQueue<cv::Mat> m_matQueue;
	std::list<cv::Mat> m_MatList;
	
	QSemaphore m_freeSlots;
	QSemaphore m_usedSlots;
	
	// stream from file-sequence stuff
	std::vector<std::string> m_filesToStream ;
	unsigned int m_currentFileIndex;
	unsigned int m_sequenceStep;
	
	// handle for IP-camera
	AxisCamera* m_axisCam;
	
	bool m_useAxisCam;
	
	void resetBuffer();

public:
    CVBufferThread(const unsigned int& slt = 50);
	~CVBufferThread();
	
	void stop();

	void rebufferFromIndex(const int& i);
	
	//consume
	cv::Mat grabNextFrame();
	
	void setSeq(const std::vector<std::string>& s);
	const unsigned int& getCurrentIndex(){return m_currentFileIndex;};
	
	void setAxisCam(AxisCamera* c){m_axisCam=c;};
	
	void setUseAxisCam(const bool& b){m_useAxisCam=b;};
	const bool& isUsingAxisCam(){return m_useAxisCam;};
	
public slots:
	

	//void replyFinished(QNetworkReply* reply);
	
protected:
	
	//produce
	void run();
	
};

#endif // CVBUFFERTHREAD_H
