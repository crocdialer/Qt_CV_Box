#ifndef TRAJECTOR_H
#define TRAJECTOR_H

#include "opencv2/core/core.hpp"
#include <list>

// Trajectory class to filter out 1-hit wonders and enhance trajectories
class Trajector  
{
	
private:
	
	cv::Size m_size;
	
	std::list<cv::Mat> m_pastMats;
	
	cv::Mat m_morphElement;
	
	void initHistory(const cv::Size& s,const int& length);
	
public:
	
	Trajector(const cv::Size& s,const int& history=8);
	
	~Trajector();
	
	void filter(const cv::Mat& src,cv::Mat& dst);
	
	const cv::Size& getSize(){return m_size;};
	void setSize(const cv::Size& s){m_size=s;initHistory(s,m_pastMats.size());};
	
	int getHistoryLength(){return m_pastMats.size();};
	void setHistoryLength(const int& l){initHistory(m_size,l);};
	
};


#endif//TRAJECTOR_H