#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "Singleton.h"
#include "SetSequenceDialog.h"
#include "JumpToDialog.h"
#include "ProcessingDialog.h"
#include "CVWidget.h"

#include "CVThread.h"

namespace Ui 
{
    class mainWindow;
}

class MainWindow : public QMainWindow,public Singleton<MainWindow>
{
    Q_OBJECT
private:
	
	Ui::mainWindow *ui;
	
	friend class Singleton<MainWindow>;
	
    QWidget* mainWidget;
	
	bool m_tagging;
	CVWidget* imgWidget;
	
	SetSequenceDialog* m_sequenceDialog ;
	JumpToDialog* m_jumpToDialog ;
    ProcessingDialog *m_procDialog;    
    
	QMenu* contextMenu;
	QAction* currentAction;
	
	QAction* m_otherAction;
	QAction* m_stopAction;
	QAction* m_gridAction;
	
	const char* m_statusMsg;
	
	void contextMenuEvent(QContextMenuEvent* e);
    void createContextMenu();
    
    void changeEvent(QEvent *e);
    
    CVThreadPtr m_cvThread;
	
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	
	//keyboard-events
    void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	
	CVWidget* getImgWidget()
	{
		
		return imgWidget;
	};
	
    public slots:
	
    void save();
    void saveAs();
    void open();
	void openStream();
	
	void openSequence();
	
	void openVideo();
	
    void activateUSBCamera(bool b);
    void activateIPCamera(bool b);
    void activateKinect(bool b);

    void toggleUSBCamera(){activateUSBCamera(!m_cvThread->isUSBCameraActive());};
    void toggleIPCamera(){activateIPCamera(!m_cvThread->isIPCameraActive());};
    void toggleKinect(){activateKinect(!m_cvThread->isKinectActive());};

	void togglePlayPause();
    
    void toggleProcessing();
    
    void toggleFullscreen(){isFullScreen() ? showNormal() : showFullScreen() ;};
	
	void handleFileScan(const bool& done);
	
	void onImageChange();
	void onIndexChange(int i=0);
	
	void onCvThreadFinished();
	
};

#endif // MAINWINDOW_H
