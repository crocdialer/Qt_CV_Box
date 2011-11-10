/*
 *  CVThread.h
 *  TestoRAma
 *
 *  Created by Fabian on 9/13/10.
 *  Copyright 2010 LMU. All rights reserved.
 *
 */

#ifndef CHTHREAD_H
#define CHTHREAD_H

#include <QThread>
#include <QMutex>
#include <QReadWriteLock>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "axiscamera.h"
#include "CVBufferThread.h"

#include "boost/shared_ptr.hpp"
#include "tbb/tbb.h"

#include "KinectDevice.h"

using namespace cv ;

class CVThread : public QThread
{
    Q_OBJECT
	
public:
	enum CVStreamType{NO_STREAM, STREAM_FILELIST,STREAM_IP_CAM, STREAM_CAPTURE,STREAM_KINECT, STREAM_VIDEOFILE};
	
    CVThread();
    virtual ~CVThread();
	
    class FrameBundle
    {
    public:
        FrameBundle(const cv::Mat &frame=cv::Mat(),
                    const cv::Mat &depth=cv::Mat())
        :   m_inFrame(frame),
            m_depthMap(depth)
        {}
        
        cv::Mat m_inFrame;
        cv::Mat m_depthMap;
        cv::Mat m_result;
    };
    
	double getElapsedTime(){return m_timer.getElapsedTime();};

	void openImage(const std::string& imgPath);
	void openSequence(const std::vector<std::string>& files);
	
	void streamVideo(const std::string& path2Video);
	void streamUSBCamera(bool b,int camId = 0);
	void streamIPCamera(bool b);
    void streamKinect(bool b);
	
	bool saveCurrentFrame(const std::string& savePath);
    
   	void loadFrameFromIpCamera();

	void playPause();
	void jumpToFrame(const int& index);
	void skipFrames(const int& num);
	
	void stop(){stopped=true;};
    
	const std::vector<std::string>& getSeq(){ return m_filesToStream;};
	
	bool isCameraActive(){return isUSBCameraActive() || isIPCameraActive() ||
        isKinectActive();};
	
	bool isUSBCameraActive() const {return m_capture.isOpened();} ;
	bool isIPCameraActive() const {return m_ipCameraActive ;} ;
    bool isKinectActive() const {return m_freenect.use_count() > 0 ;};
    void setKinectUseIR(bool b);
    bool isKinectUseIR(){return m_kinectUseIR;};
	
	void setImage(const cv::Mat& img);
	const cv::Mat& getImage() const;
    const cv::Mat& getDepthImage() const;
	
    const FrameBundle& getFrameBundle() const {return m_frames;};
	
	void setDoProcessing(bool b){m_doProcessing=b;};
	bool hasProcessing(){return m_doProcessing;};
	
	int getCurrentIndex();
	inline const int& getNumFrames(){return m_numVideoFrames;};
	inline const std::string& getVideoPath(){return m_videoPath;};
	
    double getLastGrabTime();
	double getLastProcessTime();
	
	void setFPS(const double& fps){m_captureFPS=fps;};
	double getFPS(){return m_captureFPS;};
	
	CVStreamType getStreamType(){return m_streamType;};
	
	std::string getCurrentImgPath();

private:
	
    class clock_util 
    {
        tbb::tick_count m_startTick;
        
    public:
        
        clock_util():m_startTick(tbb::tick_count::now()){};
        
        inline void reset(){m_startTick=tbb::tick_count::now();};
        
        // return elapsed time since last reset in ms
        inline double getElapsedTime()
        {
            return (tbb::tick_count::now()-m_startTick).seconds() * 1000.0;
        };
    };
    
    clock_util m_timer;
    
	CVStreamType m_streamType;
	
	std::vector<std::string> m_filesToStream ;
	int m_currentFileIndex;
	unsigned int m_sequenceStep;
	
	volatile bool stopped;
	volatile bool m_doProcessing;
    
	// fetch next frame, depending on current m_streamType
	bool grabNextFrame();
	
	//Qt Thread-snychronisation
	mutable QMutex m_mutex;
	
	//-- OpenCV
    FrameBundle m_frames;
    
	VideoCapture m_capture ;
	
    // Kinect
    boost::shared_ptr<Freenect::Freenect> m_freenect;
    KinectDevice* m_kinectDevice;
    bool m_kinectUseIR;
    
    // Buffering frames (eg. from image-seuquence or ip-cam)
	CVBufferThread* m_bufferThread;
	
	//desired capturing / seconds  -> used to time threadmanagment
	double m_captureFPS;
	
	//number of frames in current videofile from VideoCapture
	int m_numVideoFrames;
	std::string m_videoPath;
	
    // handle for IP-camera
	AxisCamera* m_ipCamera;
	bool m_ipCameraActive;
    
	double m_lastGrabTime;
	double m_lastProcessTime;
    
    
    
    virtual cv::Mat doProcessing(const FrameBundle &bundle) = 0;
	
signals:
	//-- MVC style notifyObservers() like signal
	void imageChanged();
    
protected:
	
	void run();
	
	// implementation in subclass
	//
	
};

#endif // CHTHREAD_H
