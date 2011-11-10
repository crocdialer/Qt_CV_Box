#include "ScanFilesThread.h"

#ifndef WIN32
#include <dirent.h>
#include <errno.h>
#include <algorithm>

#else
#include "opencv2/opencv.hpp"
#endif

#include <QDir>

using namespace std;

ScanFilesThread::ScanFilesThread(const std::string& dir):m_stopped(true),m_rootDir(dir)
{
	
}


void ScanFilesThread::run()
{
	m_stopped = false ;
	m_foundFiles.clear();
	
	emit scanEnded(false) ;
	
	#ifndef WIN32
	scanDir(m_rootDir,m_foundFiles,m_recursion);
	#else
	//works, but ridiciously slow !?
	scanDirQT(m_rootDir,m_foundFiles,m_recursion);
	#endif
	
	sort(m_foundFiles.begin(), m_foundFiles.end());
	
	emit scanEnded(true) ;
}

#ifndef WIN32
unsigned int ScanFilesThread::scanDir(const string& dir, vector<string> &files,const bool& recursive,const bool& isRoot)
{
	
	fileState out = FOLDER;
    DIR *dp;
    struct dirent *dirp;
	char buf [512];
	
	//could not open dir because it´s an invalid path or a plain file instead of a folder
    if( !(dp  = opendir(dir.c_str())) ) 
	{
        //cout << "Error(" << errno << ") opening " << dir << endl;
        out = FAIL;
		return out;
    }
	else if(!(recursive || isRoot))
	{
		out = NORECURSION;
		return out;
	}
	
	
    while ( (dirp = readdir(dp)) && !m_stopped) 
	{	
		if( strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 )
		{
			sprintf(buf, "%s/%s",dir.c_str(),dirp->d_name);
			
			// Recursively dive deeper
			out = (fileState)scanDir(string(buf),files,recursive,false);
			
			string fileName,ext_string ;
			switch(out)
			{
				case FOLDER:
				case NORECURSION:
				default:
					break;
				
					//TODO: confusing but works
				case FAIL: // FILE:
					fileName = string(buf);
					ext_string = fileName.substr(fileName.find_last_of(".") + 1);
					
					//check if we got an image
					if(ext_string == "jpg" || ext_string == "png" || ext_string == "pnm")
						files.push_back(fileName);
					
					break;

			}
		}
		
    }
	
    closedir(dp);
    return out;
}
#endif

unsigned int ScanFilesThread::scanDirQT(const string& path, vector<string> &files,const bool& recursive)
{
	QDir dir;
 	dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	
	dir.setPath(path.c_str());
	
	QFileInfoList list = dir.entryInfoList();
	QFileInfo fileInfo;
	
	for (int i = 0; i < list.size(); ++i)
	{
		fileInfo = list.at(i);
		
		if(fileInfo.isDir() && recursive)
			scanDirQT(fileInfo.absoluteFilePath().toStdString(),files,recursive);
		
		else 
		{
			string fileExt = fileInfo.completeSuffix().toStdString();
			
			if( fileExt == "jpg" || fileExt == "png" || fileExt == "pnm")
				files.push_back(fileInfo.absoluteFilePath().toStdString());
		}
		
		//printf( "%10li %s\n", (long)fileInfo.size(), fileInfo.fileName().toStdString().c_str() );
		
	}
	
    return 0;
}