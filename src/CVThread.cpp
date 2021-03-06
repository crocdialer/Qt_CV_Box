/*
 *  CVThread.cpp
 *  TestoRAma
 *
 *  Created by Fabian on 9/13/10.
 *  Copyright 2010. All rights reserved.
 *
 */

#include "CVThread.h"

#include <fstream>

using namespace std;
using namespace cv;
//using namespace boost;

CVThread::CVThread():m_streamType(NO_STREAM),m_currentFileIndex(0),m_sequenceStep(1),
stopped(true),m_kinectDevice(NULL),m_kinectUseIR(false),m_captureFPS(25.f)
{	
    printf("CVThread -> OpenCV-Version: %s\n\n",CV_VERSION);
	
	m_ipCamera = new AxisCamera("217.211.159.251",8080);
	m_ipCamera->setFPS(24);
	m_bufferThread = new CVBufferThread (10) ;
	m_bufferThread->setAxisCam(m_ipCamera);
    
	m_doProcessing = true;
}

CVThread::~CVThread()
{
    stop();
    wait();
    
	m_capture.release();

	m_bufferThread->stop();
	m_bufferThread->wait();
	
    delete m_ipCamera;
	delete m_bufferThread ;
}

void CVThread::openImage(const std::string& imgPath)
{	
	setImage(imread(imgPath));
	m_streamType = NO_STREAM ;

}

void CVThread::loadFrameFromIpCamera()
{
	setImage( m_ipCamera->cgiJPEGRequestDecoded() );
}

void CVThread::playPause()
{
	if(stopped)
	{	
		m_bufferThread->rebufferFromIndex(m_currentFileIndex);
		m_bufferThread->start();
		
		stopped = false ;
		start();
	}
	else 
	{
		stopped = true ; 
		m_bufferThread->stop();
		this->wait();
	}
}

void CVThread::openSequence(const std::vector<std::string>& files)
{	
	streamIPCamera(false);
	streamUSBCamera(false);
	
	m_streamType = STREAM_FILELIST ;
	
	m_currentFileIndex = 0 ;
	m_filesToStream = files;
	m_numVideoFrames = files.size();
	
	m_bufferThread->setSeq(files);
}

bool CVThread::saveCurrentFrame(const std::string& savePath)
{
	return imwrite(savePath,m_doProcessing? m_frames.m_result : m_frames.m_inFrame);
	
}

void CVThread::skipFrames(int num)
{	
	jumpToFrame( m_currentFileIndex + num );

}

void CVThread::jumpToFrame(int newIndex)
{	
	m_currentFileIndex = newIndex < 0 ? 0 : newIndex;
	
	switch (m_streamType) 
	{
		case STREAM_VIDEOFILE:
			
			if(m_currentFileIndex >= m_numVideoFrames)
				m_currentFileIndex = m_numVideoFrames-1;
			
			m_capture.set(CV_CAP_PROP_POS_FRAMES,m_currentFileIndex);
			
			break;
			
		default:
		case STREAM_FILELIST:
			
			if(m_filesToStream.empty())
				return;
			
			if(m_currentFileIndex >= (int)m_filesToStream.size())
				m_currentFileIndex = m_filesToStream.size()-1;
			
			break;

	}

	if(stopped)
	{	
		clock_util timer;
		if(grabNextFrame() && m_frames.m_inFrame.size() != Size(0,0)) 
		{	
			m_lastGrabTime = timer.getElapsedTime();
			timer.reset();
			
			if(m_doProcessing)
			{
				//m_augmentImg = doProcessing(m_procImage);
                
                
				m_lastProcessTime = timer.getElapsedTime();
			}
			
			else 
			{
				m_frames.m_result = m_frames.m_inFrame;
				m_lastProcessTime = 0;
			}

		}
		
		emit imageChanged();
	}
	else if(m_streamType == STREAM_FILELIST)
	{	
		m_bufferThread->rebufferFromIndex(m_currentFileIndex);
		
		if(!m_bufferThread->isRunning())
			m_bufferThread->start();
		
	}

	
}

void CVThread::streamVideo(const std::string& path2Video)
{
	if(m_capture.isOpened())
		m_capture.release();
	
	if(! m_capture.isOpened())
	{
        if(false)
        {
            //extracts framecount from an avi-file, since opencv fails at this point	
            
            //const char* path2Video = "/Users/Fabian/Desktop/iccv07-data/results/video-iccv07-seq1.avi";
            unsigned char tempSize[4];
            
            // Trying to open the video file
            std::ifstream videoFile( path2Video.c_str() , std::ios::in | std::ios::binary );
            // Checking the availablity of the file
            if ( !videoFile ) 
            {
                std::string errStr = path2Video+"not found (streamVideo)";
                
                //std::exception e;
                throw std::exception();
            }
            m_videoPath = path2Video ;
            
            // get the number of frames (out of AVI Header)
            videoFile.seekg( 0x30 , std::ios::beg );
            videoFile.read( (char*)tempSize , 4 );
            m_numVideoFrames = tempSize[0] + 0x100 * tempSize[1] + 0x10000 * tempSize[2] + 0x1000000 * tempSize[3];
            m_currentFileIndex = 0;
            
            videoFile.close(  );
		}
        m_capture.open(path2Video);
        
		//Doesn´t work either :( -> read aviheader once more
		m_captureFPS = m_capture.get(CV_CAP_PROP_FPS);
        m_numVideoFrames = m_capture.get(CV_CAP_PROP_FRAME_COUNT);
		printf("%d frames in video - fps: %.2f\n",m_numVideoFrames,m_captureFPS);
		
		
        
        m_streamType = STREAM_VIDEOFILE ;
		this->start();
		
	}
	else
	{	
		this->stop();
		m_capture.release();
		
		m_streamType = NO_STREAM ;
	}
	
}

void CVThread::streamUSBCamera(bool b,int camId)
{
	
	if(b && ! m_capture.isOpened())
	{
		m_capture.open(camId);
		
		this->start();
		
		m_streamType = STREAM_CAPTURE ;
		
	}
	else
	{	
		this->stop();
		m_capture.release();
		
		m_streamType = NO_STREAM ;
	}

}

void CVThread::streamIPCamera(bool b)
{
	
	m_ipCameraActive = b ;
	m_bufferThread->setUseAxisCam(b);
	m_bufferThread->rebufferFromIndex(0);
	
	if(b)
	{
		if(m_capture.isOpened())
			m_capture.release();
		
		m_streamType = STREAM_IP_CAM;
		this->start();
	}
	else
	{	
		m_bufferThread->stop();
		m_bufferThread->wait();
		
		this->stop();
		m_streamType = NO_STREAM ;
	}
	
}

void CVThread::streamKinect(bool b)
{
    if(b)
    {
        m_freenect = boost::shared_ptr<Freenect::Freenect>( new Freenect::Freenect());
        m_kinectDevice = &(m_freenect->createDevice<KinectDevice>(0));
        
        
        //setKinectUseIR(m_kinectUseIR);
        m_kinectDevice->startVideo();
        m_kinectDevice->startDepth();
        
        m_streamType = STREAM_KINECT;
        this->start();
    }
    else
    {
        
        m_freenect.reset();
        m_kinectDevice=NULL;
        
        this->stop();
        m_streamType = NO_STREAM;
    }
}

void CVThread::setKinectUseIR(bool b)
{
    m_kinectUseIR = b;
    
    if(m_kinectDevice)
        m_kinectDevice->setVideoFormat(b ?  FREENECT_VIDEO_IR_8BIT : 
                                            FREENECT_VIDEO_RGB); 
}

bool CVThread::grabNextFrame()
{	
	bool success = true ;
	
	switch (m_streamType) 
	{
		case STREAM_FILELIST:
			
			if(stopped)
				m_frames.m_inFrame = cv::imread(m_filesToStream[m_currentFileIndex]);
			else
			{
				m_frames.m_inFrame = m_bufferThread->grabNextFrame();

				m_currentFileIndex = m_currentFileIndex + m_sequenceStep;
			}
			
			// last frame reached ?
			if(m_currentFileIndex >= (int)m_filesToStream.size())
			{
				m_currentFileIndex = m_filesToStream.size()-1;
				
				this->stop();
				success = false ;
			}
			
			break;
			
		case STREAM_CAPTURE:
		case STREAM_VIDEOFILE:
			
			if(m_capture.isOpened() && m_capture.grab())
			{		
				Mat capFrame;
				m_capture.retrieve(capFrame, 0) ;
				
				// going safe, have a copy of our own of the data
				m_frames.m_inFrame = capFrame.clone();
				
				if(m_streamType==STREAM_VIDEOFILE)
				{
					// last frame reached ?
					if(m_currentFileIndex+1 >= m_numVideoFrames)
					{
						m_currentFileIndex = m_numVideoFrames-1;
						
						this->stop();
						success = false ;
					}
					else
						m_currentFileIndex = m_capture.get(CV_CAP_PROP_POS_FRAMES);
					
				}
				
			}
			
			else 
				success = false ;

			break;
            
        case STREAM_IP_CAM:
			
			if(stopped)
				loadFrameFromIpCamera();
			else
				m_frames.m_inFrame = m_bufferThread->grabNextFrame();
			break ;

        case STREAM_KINECT:
			
			m_kinectDevice->getVideo(m_frames.m_inFrame,m_kinectUseIR);
            m_kinectDevice->getDepth(m_frames.m_depthMap,m_frames.m_inFrame);
            
			break ;
            
		case NO_STREAM:
			success = false ;
			break ;
			
		default:
			break;
	}
	
	return success;
}

void CVThread::run()
{	
	stopped=false;
	
	// measure elapsed time with these
	clock_util timer,otherTimer;
	
	double elapsedTime,sleepTime;
	
	// gets next frame, which will be hold inside m_procImage
	while( !stopped )
	{
		//restart timer
		timer.reset();
		otherTimer.reset();
		
		// fetch frame, cancel loop when not possible
		// this call is supposed to be fast and not block the thread too long
		if (!grabNextFrame()) break;
		
		//skip iteration when invalid frame is returned (eg. from camera)
		if(m_frames.m_inFrame.empty()) continue;
		
		m_lastGrabTime = otherTimer.getElapsedTime();
		otherTimer.reset();
		
		// image processing
		if(m_doProcessing)
		{ 
            m_frames.m_result = doProcessing(m_frames);
		}
		else
			m_frames.m_result = m_frames.m_inFrame;
		
		m_lastProcessTime = otherTimer.getElapsedTime();
		otherTimer.reset();
		
		// emit signal for other objects/threads to be notified
		emit imageChanged();
		
		elapsedTime = timer.getElapsedTime();
		
		sleepTime = (1000.0 / m_captureFPS - elapsedTime);
		sleepTime = sleepTime > 0 ? sleepTime : 0 ;
		
		//printf("elapsed time: %.2f  -- sleeping: %.2f \n",elapsedTime,sleepTime);
		
		// set thread asleep for a time to achieve desired framerate when possible
		QThread::msleep(sleepTime);
		
	}
	
}

string CVThread::getCurrentImgPath()
{
	string out;
	switch (m_streamType) 
	{
		case STREAM_FILELIST:
			
			out = m_filesToStream[m_currentFileIndex];
			break;
			
		case STREAM_VIDEOFILE:
			
			out = getVideoPath();
			break;
			
		case STREAM_CAPTURE:
		case STREAM_IP_CAM:
			
			out = "camera input";
			break;
			
		default:
			break;
	}
	return out;
}

// -- Getter / Setter
const cv::Mat& CVThread::getImage() const
{	
	QMutexLocker ql(&m_mutex);
	
	return m_frames.m_inFrame;
}

void CVThread::setImage(const cv::Mat& img)
{
	m_frames.m_result = m_frames.m_inFrame = img.clone();
	
	emit imageChanged(); 
}

double CVThread::getLastGrabTime()
{
	QMutexLocker ql(&m_mutex);
	return m_lastGrabTime;
}

double CVThread::getLastProcessTime()
{
	QMutexLocker ql(&m_mutex);
	return m_lastProcessTime;
}

int CVThread::getCurrentIndex()
{
	QMutexLocker ql(&m_mutex);
	return m_currentFileIndex;
}
