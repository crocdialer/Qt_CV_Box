#ifndef AXISCAMERA_H
#define AXISCAMERA_H

#include <string>

#include "boost/shared_array.hpp"
#include "opencv2/opencv.hpp"
#include <QTcpSocket>


const int READ_BUFFER_SIZE = 512;

typedef boost::shared_array<const char> JPEGBuffer;

class AxisCamera
{
    
public:
    
    static const int DEFAULT_WIDTH;
    static const int DEFAULT_HEIGHT;
    static const int DEFAULT_FPS;
    
    static const int DEFAULT_COMPRESSION;
    static const int DEFAULT_COLOR_LEVEL;
    static const int DEFAULT_SECONDS;
    static const int DEFAULT_FRAMES;
    
    static const char* const CONTENT_TYPE;
    static const char* const JPEG_MIMETYPE;
    static const char* const CONTENT_LENGTH;
    static const char* const MULTIPART_MIXED_MIMETYPE;
    static const char* const BOUNDARY;
    
    static const int SOCKET_TIMEOUT;
    
	AxisCamera( const std::string& hostIP, const unsigned short& port = 80 );
	~AxisCamera();
	
	// public interface
	
	// get single jpeg-frame
	JPEGBuffer cgiJPEGRequest( const std::string& userParameters = "" );
	
	// convenience function for decoded version of cgiJPEGRequest()
	cv::Mat cgiJPEGRequestDecoded();
	
	// get next frame from mjpeg-stream, needs cgiMJPEGRequest() first!
	JPEGBuffer nextImage();
	
	// convenience function for decoded version of nextImage()
	cv::Mat nextImageDecoded(){return decodeJPEGBuffer(nextImage());};
	
	// MJPEG CGI request
	bool cgiMJPEGRequest( const std::string& userParameters = "" );
	
	// checks for valid socket
	bool isImageAvailable() const;
	
	// closes socket
	void abortTransmission();
	
	// init QT-socket and datastream
	void initSocket();
	
	
private:
	AxisCamera( const AxisCamera& rhs );
	AxisCamera& operator=( const AxisCamera& rhs );
	
	bool sendRequest( const std::string& request );
	JPEGBuffer readSingleJPEG();
	
	void clearReadBuffer();
	
	// reads content of buffer until it encounters "\r\n" and returns the preceeding content
	// fills up buffer with content from socket when the buffer is empty
	std::string readLine();
	
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::string getCredentials();
	
	// QT TCP-socket stuff
	QTcpSocket* m_socket;
	QDataStream* m_dataStream;
	
	
	std::string m_boundary;
	
	char m_readBuffer[READ_BUFFER_SIZE]; // 512 should be fine, right?
	int m_validBufferLength;
	int m_bufferIndex;
	
	std::string m_hostIP;
	unsigned short m_port;
	
	// Camera settings
	//std::string m_resolution;
	unsigned int m_width;
	unsigned int m_height;
	
	int m_compression;
	int m_colorLevel;
	int m_seconds;
	int m_numberOfFrames;
	int m_fps;
	bool m_colorEnabled;
	bool m_clockEnabled;
	bool m_dateEnabled;
	
	// parameter functions
	// user parameters e.g. "blah=123&foo=22" (no leading '&'!)
	std::string resolutionParameter() const;
	std::string compressionParameter() const;
	std::string colorLevelParameter() const;
	std::string colorParameter() const;
	std::string showClockParameter() const;
	std::string showDateParameter() const;
	std::string durationParameter() const;
	std::string numberOfFramesParameter() const;
	std::string fpsParameter() const;
	
	std::string genericIntParameter( int param, const std::string& key ) const;
	std::string commonParameters() const;
	
	cv::Mat decodeJPEGBuffer(const JPEGBuffer& buf);
	
	//private slots:
	
	// currently not used, using blocking socket-api instead
	void onSocketData();
	
public:
	
	// Getters / Setter
	
	QTcpSocket* getSocket(){return m_socket;};
	
	const std::string& getHostUrl(){return m_hostIP;};
	const unsigned short& getPort(){return m_port;};
	
	void setHostUrl(const std::string& host,const unsigned short& p=80)
	{
		m_hostIP = host;
		m_port = p; 
		initSocket();
	};
	
	// JPEG + MJPEG options
	void getResolution(int& w,int& h){w=m_width;h=m_height;};
	void setResolution( const unsigned int& w,const unsigned int& h );
	
	void setCompression( int compression );
	void setColorLevel( int colorLevel );
	void setColor( bool enabled );
	void setShowClock( bool enabled );
	void setShowDate( bool enabled );
	
	// MJPEG only
	void setDuration( int seconds );
	void setNumberOfFrames( int numberOfFrames );
	void setFPS( int fps );
	int getFPS(){return m_fps;};
};

#endif