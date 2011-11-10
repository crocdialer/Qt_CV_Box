//
//  KinectDevice.cpp
//  helloQtCmake
//
//  Created by Fabian Schmidt on 9/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include "KinectDevice.h"

using namespace cv;
using namespace std;

KinectDevice::KinectDevice(freenect_context *_ctx, int _index)
: Freenect::FreenectDevice(_ctx, _index), m_gamma(2048),
depthMat(Size(640,480),CV_16UC1), rgbMat(Size(640,480),CV_8UC3,Scalar(0)), ownMat(Size(640,480),CV_8UC3,Scalar(0)),
m_new_rgb_frame(false), m_new_depth_frame(false)
{
    
    for( unsigned int i = 0 ; i < 2048 ; i++) {
        float v = i/2048.0;
        v = std::pow(v, 3)* 6;
        m_gamma[i] = v*6*256;
    }
}

KinectDevice::~KinectDevice()
{
    this->stopVideo();
    this->stopDepth();
    
}

// Do not call directly even in child
void KinectDevice::VideoCallback(void* _rgb, uint32_t timestamp) {
    //std::cout << "RGB callback" << std::endl;
    m_rgb_mutex.lock();
    uint8_t* rgb = static_cast<uint8_t*>(_rgb);
    rgbMat.data = rgb;
    m_new_rgb_frame = true;
    m_rgb_mutex.unlock();
};
// Do not call directly even in child
void KinectDevice::DepthCallback(void* _depth, uint32_t timestamp) {
    //std::cout << "Depth callback" << std::endl;
    m_depth_mutex.lock();
    uint16_t* depth = static_cast<uint16_t*>(_depth);
    depthMat.data = (uchar*) depth;
    m_new_depth_frame = true;
    m_depth_mutex.unlock();
}

bool KinectDevice::getVideo(Mat& output,bool irBool) {
    m_rgb_mutex.lock();
    if(m_new_rgb_frame) {
        
        if(irBool){
            //IR 8BIT
            output = Mat(rgbMat.size(),CV_8UC1,rgbMat.data + rgbMat.cols * 4).clone();
        }
        else
            //RGB
            cv::cvtColor(rgbMat, output, CV_RGB2BGR);
            
        m_new_rgb_frame = false;
        m_rgb_mutex.unlock();
        return true;
    } else {
        m_rgb_mutex.unlock();
        return false;
    }
}

bool KinectDevice::getDepth(Mat& output) {
    m_depth_mutex.lock();
    if(m_new_depth_frame) {
        //depthMat.copyTo(output);
        
        //output=depthMat.clone();
        
        depthMat.convertTo(output,CV_32F,1.0/2048.0);
        
        output = 1.0 - output;
        
        m_new_depth_frame = false;
        m_depth_mutex.unlock();
        return true;
    } else {
        m_depth_mutex.unlock();
        return false;
    }
}
