#include "CVBufferThread.h"
#include "opencv2/highgui/highgui.hpp"
#include "mainwindow.h"

CVBufferThread::CVBufferThread(const unsigned int& s): m_stopped(true), m_numSlots(s),
m_currentFileIndex(0), m_sequenceStep(1),m_useAxisCam(false)
{	
	resetBuffer();
}

CVBufferThread::~CVBufferThread()
{

}

void CVBufferThread::stop()
{
	//m_axisCam->abortTransmission();
	m_stopped=true;
}

void CVBufferThread::resetBuffer()
{
	//reset semaphore values
    m_freeSlots.release(m_numSlots - m_freeSlots.available());
	m_usedSlots.acquire(m_usedSlots.available());

	//clear buffer
	m_matQueue.clear();
	
}

void CVBufferThread::rebufferFromIndex(const int& i)
{
	m_currentFileIndex = i;
	resetBuffer();
}

//Consume a frame, blocks if buffer empty
cv::Mat CVBufferThread::grabNextFrame()
{
	cv::Mat ret;
	
	if(m_usedSlots.tryAcquire(1, 400))
	{
		ret = m_matQueue.pop();
		m_freeSlots.release();
	}
	else if(!isRunning())
	{
		this->rebufferFromIndex(m_currentFileIndex);
		this->start();
	}
	
	return ret;
}

void CVBufferThread::setSeq(const std::vector<std::string>& s)
{
	m_filesToStream = s;
	m_currentFileIndex = 0;
}

void CVBufferThread::run()
{	
	
	if(m_useAxisCam)
	{
		m_axisCam->initSocket();
		if(!m_axisCam->cgiMJPEGRequest())
			printf("cgiMJPEGRequest failed\n");;
	}
	
	if(m_filesToStream.empty() && !m_useAxisCam) return ;
	
	m_stopped = false ;
	
	while (!m_stopped && (m_currentFileIndex<m_filesToStream.size() || m_axisCam->isImageAvailable() ))
	{
		if(!m_freeSlots.tryAcquire(1, 40)) //
		{
			if(m_useAxisCam) m_axisCam->nextImage();
			continue;
		}

		//load frame from somewhere
		if(m_useAxisCam)
		{
			m_matQueue.push(m_axisCam->nextImageDecoded());	
		}
		else 
		{
			m_matQueue.push(cv::imread(m_filesToStream[m_currentFileIndex]));
			m_currentFileIndex = m_currentFileIndex + m_sequenceStep;
		}

		//printf("buffered frame %d ...\n",frameID++);
		
		m_usedSlots.release();
	}
	
	m_stopped = true ;
	//resetBuffer();
	
}



