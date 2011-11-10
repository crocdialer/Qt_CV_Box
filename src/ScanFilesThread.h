#ifndef SCANFILESTHREAD_H
#define SCANFILESTHREAD_H

#include <QThread>
#include <vector>
#include <string>

//Helper class meant to be run on a separate thread, doing filesystem stuff
//and gathering a list of filenames to process
class ScanFilesThread : public QThread
{
	Q_OBJECT
	
private:
	volatile bool m_stopped;
	
	std::string m_rootDir;
	std::vector<std::string> m_foundFiles;
	bool m_recursion;
	
	enum fileState {FOLDER,FILE,FAIL,NORECURSION} ;

signals:
	
	void scanEnded(const bool& done=false);
	
	
public:
    ScanFilesThread(const std::string& dir="");
	
	void stop(){m_stopped=true;};
	
	void setRootDir(const std::string& p){m_rootDir = p ;};
	const std::string& getRootDir(){return m_rootDir;};
	
	void setUseRecursion(const bool& r){m_recursion = r ;};
	const bool& isRecursive(){return m_recursion;};
	
	const std::vector<std::string>& getFoundFiles(){return m_foundFiles;};

protected:
	
	#ifndef WIN32
	unsigned int scanDir(const std::string& path,std::vector<std::string> &files,
		const bool& recursive = false,const bool& isRoot = true);
	#endif

	unsigned int scanDirQT(const std::string& path,std::vector<std::string> &files,const bool& recursive = false);
    virtual void run();
};

#endif // SCANFILESTHREAD_H
