#include "axiscamera.h"

#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
//#include <conio.h>
#include <algorithm>

const int AxisCamera::DEFAULT_WIDTH = 640;
const int AxisCamera::DEFAULT_HEIGHT = 480;
const int AxisCamera::DEFAULT_FPS = 25;

const int AxisCamera::DEFAULT_COMPRESSION = 25;
const int AxisCamera::DEFAULT_COLOR_LEVEL = -1;

const int AxisCamera::DEFAULT_SECONDS = -1;
const int AxisCamera::DEFAULT_FRAMES = -1;

const char* const AxisCamera::CONTENT_TYPE = "Content-Type:";
const char* const AxisCamera::JPEG_MIMETYPE = "image/jpeg";
const char* const AxisCamera::CONTENT_LENGTH = "Content-Length:";
const char* const AxisCamera::MULTIPART_MIXED_MIMETYPE = "multipart/x-mixed-replace";
const char* const AxisCamera::BOUNDARY = "boundary=";

const int AxisCamera::SOCKET_TIMEOUT = 2000;



AxisCamera::AxisCamera( const std::string& hostIP, const unsigned short& port ) :
m_hostIP( hostIP ), m_port( port ), m_width( DEFAULT_WIDTH ),m_height( DEFAULT_HEIGHT ),
m_compression( DEFAULT_COMPRESSION ), m_colorLevel( DEFAULT_COLOR_LEVEL ),
m_seconds( DEFAULT_SECONDS ), m_numberOfFrames( DEFAULT_FRAMES ),
m_fps( DEFAULT_FPS ), m_colorEnabled( true ), m_clockEnabled( false ),
m_dateEnabled( false )
{
	m_socket = NULL;
	m_dataStream = NULL;
	
	initSocket();
	
}

AxisCamera::~AxisCamera()
{
	m_socket->close();
	
	delete m_dataStream;
	delete m_socket;
}


void AxisCamera::initSocket()
{
	if(m_dataStream) delete m_dataStream;
	
	if(m_socket) 
	{
		m_socket->close();
		delete m_socket;
	}
	
	m_socket = new QTcpSocket();
	m_socket->connectToHost(m_hostIP.c_str(), m_port);
	
	if(!m_socket->waitForConnected(SOCKET_TIMEOUT))
	{
		printf("could not connect to axis-camera (%s, port: %d)\n",m_hostIP.c_str(),m_port);
	}
	
	m_dataStream = new QDataStream (m_socket);
}

void AxisCamera::onSocketData()
{
	int numBytes = m_socket->bytesAvailable() ;
	char* b = new char[numBytes];
	
	m_validBufferLength = m_dataStream->readRawData(b,numBytes);
	
	printf("received %d bytes:\n%s\n",m_validBufferLength,b);
	
	delete[] b;
}

bool AxisCamera::sendRequest( const std::string& request )
{
	
	if ( m_socket->state() == QTcpSocket::ConnectedState )
		m_socket->close(); 
	
	// connect
	m_socket->connectToHost( m_hostIP.c_str(), m_port );
	
	if ( !m_socket->waitForConnected(SOCKET_TIMEOUT) )
		return false;
	
	int bytesWritten = m_dataStream->writeRawData(request.c_str(), request.size());
	
	m_socket->flush();
	
	
	return bytesWritten == (int)request.size() ;
	
}

JPEGBuffer AxisCamera::readSingleJPEG()
{
	std::string response( readLine() );
	
	//printf("readSingleJPEG -> response: %s\n",response.c_str());
	
	bool jpegContent = false;
	int contentLength = 0;
	while ( !response.empty() ) 
	{
		//printf("response: %s\n",response.c_str());
		
		if ( response.find( CONTENT_TYPE ) != std::string::npos &&
			response.find( JPEG_MIMETYPE ) != std::string::npos )
			jpegContent = true;
		
		
		else if ( response.find( CONTENT_LENGTH ) != std::string::npos )
			contentLength = ::strtol( response.substr( ::strlen( CONTENT_LENGTH ) ).c_str(), ( char** )NULL, 10 );
		
		// ignore the other entities
		response = readLine();
	}
	
	if ( !jpegContent || contentLength == 0 )
		return JPEGBuffer();
	
	//std::cerr << "contentLength = " << contentLength << std::endl;
	char* jpegData = new char[contentLength];
	
	// First copy the remaining buffer content...
	int index = m_validBufferLength - m_bufferIndex;
	::memcpy( jpegData, &m_readBuffer[m_bufferIndex], index );
	
	// Then get the remaining data from the server
	while ( index < contentLength ) 
	{
		if(!m_socket->bytesAvailable())
			m_socket->waitForReadyRead();
		
		int tmp = m_dataStream->readRawData(m_readBuffer,
											std::min( READ_BUFFER_SIZE, contentLength - index ));
		
		if ( tmp == -1 ) // reached the end prematurely?
			break;
		
		::memcpy( &jpegData[index], m_readBuffer, tmp );
		index +=tmp;
	}
	clearReadBuffer(); // We manipulated the buffer directly, but ensured
	// it's empty, so this is needed to sync the other variables.
	
	if ( readLine().empty() ) // \r\n at the end of the data
		return JPEGBuffer(jpegData);
	
	else 
	{
		// Something weird happened, ignore that image
		std::cerr << "Error when reading the image data, dismissing this image." << std::endl;
		delete [] jpegData;
		return JPEGBuffer();
	}
}

void AxisCamera::clearReadBuffer()
{
	m_bufferIndex = 0;
	m_validBufferLength = 0;
}

std::string AxisCamera::readLine()
{	
	//printf("readLine() -> bytes available:%d\n",(int)m_socket->bytesAvailable());
	
	std::string result;
	bool previousWasCR = false;
	
	// yep, freaky construction :-(
	do 
	{
		while ( m_bufferIndex < m_validBufferLength ) 
		{
			if ( m_readBuffer[m_bufferIndex] == '\r' )
				previousWasCR = true;
			
			else if ( m_readBuffer[m_bufferIndex] == '\n' && previousWasCR ) 
			{
				++m_bufferIndex;
				return result;
			}
			else 
			{
				result.push_back( m_readBuffer[m_bufferIndex] );
				previousWasCR = false;
			}
			++m_bufferIndex;
		}
		
		// refill the buffer
		m_bufferIndex = 0;
		
		// read in chunks of buffersize (there might be data left in buffer after return)
		m_validBufferLength = m_dataStream->readRawData(m_readBuffer,READ_BUFFER_SIZE);
		
		//printf("readLine() read %d bytes\n",m_validBufferLength);
		
	} while ( m_validBufferLength > 0 );
	
	// If we left the loop that way some error occurred or no more
	// data is available, we're closing the socket. AFAICT this is
	// legal for a HTTP/1.0 client... but I'm not really sure.
	
	//printf("readLine(): error or no more data -> closing socket\n");
	//m_socket->close();
	
	return result;
}

JPEGBuffer AxisCamera::cgiJPEGRequest( const std::string& userParameters )
{
	if(m_socket->state() == QTcpSocket::UnconnectedState)
	{
		m_socket->connectToHost(m_hostIP.c_str(), m_port);
		m_socket->waitForConnected(SOCKET_TIMEOUT);
	}
	
	
	std::string request( "GET /axis-cgi/jpg/image.cgi?" );
	request += commonParameters();
	request += userParameters;
	
	if ( request[ request.size() - 1 ] == '&' || request[ request.size() - 1 ] == '?' )
		request.erase( request.size() - 1, 1 );
	
	request += " HTTP/1.0\r\n";
	request += "\r\n";
	
	//TODO: tesing only -> remove
	printf("request: %s\n",request.c_str());
	
	if ( !sendRequest( request ) )
	{
		printf("request failed\n");
		return JPEGBuffer();
	}
	
	clearReadBuffer();
	
	// block thread until data is available
	m_socket->waitForReadyRead();
	
	std::string response( readLine() );
	
	if ( response.find("401 Unauthorized") != std::string::npos )
	{
		std::string credentials = getCredentials();
		request = "GET /axis-cgi/jpg/image.cgi?";
		request += commonParameters();
		request += userParameters;
		
		if ( request[ request.size() - 1 ] == '&' || request[ request.size() - 1 ] == '?' )
			request.erase( request.size() - 1, 1 );
		
		request += " HTTP/1.0\r\n";
		
		//authorization required
		request += "Authorization: Basic " + credentials + "\r\n";
		
		request += "\r\n";
		
		if ( !sendRequest( request ) )
		{
			return JPEGBuffer();
		}
		
		clearReadBuffer();
		response = readLine();
	}
	
	if ( response.find( "HTTP/1." ) != 0 || response.find( "200 OK" ) == std::string::npos )
	{
		return JPEGBuffer();
	}
	
	JPEGBuffer ret = readSingleJPEG();
	
	m_socket->close();
	
	return ret;
}

cv::Mat AxisCamera::cgiJPEGRequestDecoded()
{
	if(!isImageAvailable())
	{
		printf("oops, camera socket not connected\n");
		return cv::Mat();
	}
	else
		return decodeJPEGBuffer(cgiJPEGRequest());
}

bool AxisCamera::cgiMJPEGRequest( const std::string& userParameters )
{
	
	if(m_socket->state() != QTcpSocket::ConnectedState)
	{
		m_socket->connectToHost(m_hostIP.c_str(), m_port);
		m_socket->waitForConnected(SOCKET_TIMEOUT);
	}
	
	
	std::string request( "GET /axis-cgi/mjpg/video.cgi?" );
	request += durationParameter();
	request += numberOfFramesParameter();
	request += fpsParameter();
	request += commonParameters();
	request += userParameters;
	
	if ( request[ request.size() - 1 ] == '&' || request[ request.size() - 1 ] == '?' )
		request.erase( request.size() - 1, 1 );
	
	request += " HTTP/1.0\r\n";
	request += "\r\n";
	
	//TODO: tesing only -> remove
	//printf("request: %s\n",request.c_str());
	
	// send request to camera
	if ( !sendRequest( request ) )
	{
		printf("request not send\n");
		
		return false;
	}
	
	clearReadBuffer();
	
	// block thread until data is available
	m_socket->waitForReadyRead();
	
	std::string response( readLine() );
	printf("response: %s\n",response.c_str());
	
	if ( response.find("401 Unauthorized") != std::string::npos )
	{
		std::string credentials = getCredentials();
		request = "GET /axis-cgi/mjpg/video.cgi?";
		request += durationParameter();
		request += numberOfFramesParameter();
		request += fpsParameter();
		request += commonParameters();
		request += userParameters;
		
		if ( request[ request.size() - 1 ] == '&' || request[ request.size() - 1 ] == '?' )
			request.erase( request.size() - 1, 1 );
		
		request += " HTTP/1.0\r\n";
		
		//authorization required
		request += "Authorization: Basic " + credentials + "\r\n";
		
		request += "\r\n";
		
		if ( !sendRequest( request ) )
		{
			return false;
		}
		
		clearReadBuffer();
		response = readLine();
	}
	
	if ( response.find( "HTTP/1." ) != 0 ||
		response.find( "200 OK" ) == std::string::npos )
	{
		return false;
	}
	
	response = readLine();
		
	bool contentTypeOk = false;
	m_boundary.clear();
	
	while ( !response.empty() ) 
	{
		printf("response: %s\n",response.c_str());
		
		if ( response.find( CONTENT_TYPE ) != std::string::npos &&
			response.find( MULTIPART_MIXED_MIMETYPE ) != std::string::npos ) 
		{
			contentTypeOk = true;
			
			std::string::size_type pos = response.find( BOUNDARY );
			if ( pos != std::string::npos )
				m_boundary = response.substr( pos + ::strlen( BOUNDARY ) );
		}
		// ignore the other entities
		response = readLine();
	}
	
	if ( !contentTypeOk || m_boundary.empty() )
		return false;
    
	return readLine() == m_boundary;
}

bool AxisCamera::isImageAvailable() const
{
	return m_socket->state() == QTcpSocket::ConnectedState ;
}

JPEGBuffer AxisCamera::nextImage()
{
	// block thread until data is available
	if( !(isImageAvailable() && m_socket->waitForReadyRead()) ) return JPEGBuffer();
	
	JPEGBuffer ret( readSingleJPEG() );
	
	if ( !ret.get() ) 
	{
		//printf("nextImage() could not read jpg-frame\n");
		
		// Try to resynchronize by dropping the data till we
		// find the next boundary
		while ( (m_socket->state() == QTcpSocket::ConnectedState) && 
			   readLine() != m_boundary ) 
		{
			//printf("AxisCamera::nextImage() -> resynching...\n"); // <-- intended ";"
			
			/*
			abortTransmission();
			initSocket();
			cgiMJPEGRequest();
			*/
			
			break;
		}
	}
	else 
	{
		if ( readLine() != m_boundary )
		{
			//std::cerr << "Hmmm, why didn't we find the boundary where we expected? Looks like we are done." << std::endl;
		}
	}
	return ret;
}

cv::Mat AxisCamera::decodeJPEGBuffer(const JPEGBuffer& buf)
{
    if(!buf.get()) return cv::Mat();
    // constructor for matrix headers pointing to user-allocated data
    cv::Mat bufMat = cv::Mat(m_width, m_height, CV_8UC3, (void*)buf.get());
    cv::Mat img = cv::imdecode(bufMat,1);
    
    return img;
}

void AxisCamera::abortTransmission()
{
	m_socket->close();
}

void AxisCamera::setResolution( const unsigned int& w,const unsigned int& h )
{
	m_width = w;
	m_height = h;
}

void AxisCamera::setCompression( int compression )
{
	m_compression = compression;
}

void AxisCamera::setColorLevel( int colorLevel )
{
	m_colorLevel = colorLevel;
}

void AxisCamera::setColor( bool enabled )
{
	m_colorEnabled = enabled;
}

void AxisCamera::setShowClock( bool enabled )
{
	m_clockEnabled = enabled;
}

void AxisCamera::setShowDate( bool enabled )
{
	m_dateEnabled = enabled;
}

void AxisCamera::setDuration( int seconds )
{
	m_seconds = seconds;
}

void AxisCamera::setNumberOfFrames( int numberOfFrames )
{
	m_numberOfFrames = numberOfFrames;
}

void AxisCamera::setFPS( int fps )
{
	m_fps = fps;
}

std::string AxisCamera::resolutionParameter() const
{
	char b[128];
	sprintf(b, "resolution=%dx%d&",m_width,m_height);
	
	return std::string( b );
}

std::string AxisCamera::compressionParameter() const
{
	return genericIntParameter( m_compression, "compression=" );
}

std::string AxisCamera::colorLevelParameter() const
{
	return genericIntParameter( m_colorLevel, "colorlevel=" );
}

std::string AxisCamera::colorParameter() const
{
	// color default ?
	return m_colorEnabled ? std::string() : std::string( "color=0&" );
}

std::string AxisCamera::showClockParameter() const
{
	return m_clockEnabled ? std::string( "clock=1&" ) : std::string();
}

std::string AxisCamera::showDateParameter() const
{
	return m_dateEnabled ? std::string( "date=1&" ) : std::string();
}

std::string AxisCamera::durationParameter() const
{
	return genericIntParameter( m_seconds, "duration=" );
}

std::string AxisCamera::numberOfFramesParameter() const
{
	return genericIntParameter( m_numberOfFrames, "nbrofframes=" );
}

std::string AxisCamera::fpsParameter() const
{
	return genericIntParameter( m_fps, "req_fps=" );
}

std::string AxisCamera::genericIntParameter( int param, const std::string& key ) const
{
	if ( param < 0 )
		return std::string();
	else {
		std::stringstream stream(std::stringstream::out | std::stringstream::app);
		stream << key;
		stream << param;
		stream << '&';
		return stream.str();
	}
}

std::string AxisCamera::commonParameters() const
{
	std::string parameters( resolutionParameter() );
	parameters += compressionParameter();
	parameters += colorLevelParameter();
	parameters += colorParameter();
	parameters += showClockParameter();
	parameters += showDateParameter();
	return parameters;
}

std::string AxisCamera::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
	const std::string base64_chars = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";
	
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	
	while (in_len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			
			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
		
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		
		while((i++ < 3))
			ret += '=';
	}
	
	return ret;
}

std::string AxisCamera::getCredentials()
{
	//std::string user, pass;
	//std::cout << "Username> ";
	//std::cin >> user;
	//std::cout << "Password> ";
	
	//char input;
	//while ((input = getch()) != 13) // 13 is usually enter
	//{
	//	if (input == 8) // backspace
	//	{
	//		if (pass.length() > 0)
	//		{
	//			pass.erase(pass.length() - 1);
	//			continue;
	//		}
	//		continue;
	//	}
	//	pass += input;
	//	//std::cout << "*";
	//	//std::cout.flush();
	//}
	
	//std::cout << std::endl;
	
	//user += ":";
	//user += pass;
	
	// FIXXME... :-)
	std::string user = "guest:noaccess";
	return base64_encode(reinterpret_cast<const unsigned char*>(user.c_str()), user.length());
}
