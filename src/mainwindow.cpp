#include "mainwindow.h"
#include "ui_Mainwindow.h"
#include "CCFThread.h"
#include <QtGui>

//#include "boost/shared_ptr.hpp"

using boost::shared_ptr;


template<> MainWindow* Singleton<MainWindow>::ms_Singleton = 0;

const char* GREETINGS_MSG = "welcome to Persona Boy application - load a sequence or start a camera";

const char* IPCAM_MSG = "streaming from network-camera" ;
const char* USBCAM_MSG = "streaming from USB-camera" ;
const char* KINECT_MSG = "streaming from Kinect" ;

const char* SCAN_MSG = "Scanning folders..." ;

const char* IDLE_MSG = "Idle" ;

bool overlay = false;

MainWindow::MainWindow(QWidget *parent)
:	QMainWindow(parent),ui(new Ui::mainWindow),m_tagging(false),m_statusMsg(GREETINGS_MSG)
{
#ifdef __APPLE__
	//chdir("../..");
#endif
	
    ui->setupUi(this);
	
	// init imageWidget and attach a CVThread to it
    imgWidget = new CVWidget();
	setCentralWidget(imgWidget);
	imgWidget->setFocus(Qt::MouseFocusReason);

	//create new CVThread and pass it to our imgWidget
    m_cvThread = shared_ptr<CVThread>(new CCFThread());
    
	imgWidget->setCVThread(m_cvThread);
	imgWidget->setDrawFPS(true);
    
    /*
    CVWidget* myWid = new CVWidget();
	this->layout()->addWidget(myWid);
    myWid->setCVThread(m_cvThread);
    myWid->setWidgetType(CVWidget::WIDGET_DEPTH);
    */
    
	//test images
	m_cvThread->openImage("./test.jpg");
    
	//imgWidget->getCVThread()->loadFrameFromIpCamera();
	
	m_sequenceDialog = new SetSequenceDialog(this) ;
	m_jumpToDialog = new JumpToDialog(this) ;
   	m_procDialog = new ProcessingDialog(this) ;
	
	//connect signals/slots
	connect(ui->actionOpen_Image, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(save()));
	
	// show "Choose sequence" dialog
	connect(ui->actionSet_Directory, SIGNAL(triggered()), m_sequenceDialog, SLOT(show()),
			Qt::QueuedConnection);
	
	// show "transport controls" dialog
	connect(ui->m_actionShowControl, SIGNAL(triggered()), m_jumpToDialog, SLOT(show()),
			Qt::QueuedConnection);
    
    // show processing dialog
	connect(ui->m_actionProcInfo, SIGNAL(triggered()), m_procDialog, SLOT(show()),
			Qt::QueuedConnection);
	
	connect(ui->action_VideoFile, SIGNAL(triggered()), this, SLOT(openVideo()));
	connect(ui->actionOpen_camera, SIGNAL(triggered()), this, SLOT(toggleUSBCamera()));
	
    // start streaming from network cam
	connect(ui->actionNetwork_camera, SIGNAL(triggered()), this, SLOT(toggleIPCamera()));
    
    // start streaming from kinect
	connect(ui->actionKinect, SIGNAL(triggered()), this, SLOT(toggleKinect()));
    
	// toggle fullscreen from menu
	connect(ui->actionFullscreen, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));
	
	// update labels and trackbar when imagechanges occur
	connect(m_cvThread.get(), SIGNAL(imageChanged()), this, SLOT(onImageChange()),Qt::QueuedConnection);
	
	// update when our CVThread finishes execution (eg. display idle msg)
	connect(m_cvThread.get(), SIGNAL(finished()), this, SLOT(onCvThreadFinished()),Qt::QueuedConnection);
	
	// create rightclick contextmenu
	createContextMenu();
	
	// "greetings!" status message
	statusBar()->showMessage(GREETINGS_MSG);
	
	printf("%s\n",GREETINGS_MSG);
}

MainWindow::~MainWindow()
{	
	//delete imgWidget;
	
	
	delete contextMenu;
	delete m_sequenceDialog;
	delete m_jumpToDialog;
	delete ui;
	
	delete m_stopAction;
}


//QT SLOTS

void MainWindow::save()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image ..."), "~/",
													tr("Images (*.jpg *.png *.gif *.tif)"));
	
	if (!fileName.isEmpty())
		m_cvThread.get()->saveCurrentFrame(fileName.toStdString());
}

void MainWindow::saveAs()
{
	
}

void MainWindow::open()
{	
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image ..."), "~/",
													tr("Images (*.jpg *.png *.gif *.tif *.pnm)"));
	if (!fileName.isEmpty())
		m_cvThread.get()->openImage(fileName.toStdString());
}

void MainWindow::openStream()
{
	//directory seems to be ignored on osx
	QStringList files = QFileDialog::getOpenFileNames(
													  this,
													  "Select one or more files to open",
													  "~/",
													  "Images (*.jpg *.png *.gif *.tif)");
	printf("%d files selected\n",files.length());
	std::vector<std::string> outList ;
	
	// transform to std::list<std::string>
	// ( no Qt dependencies for CV-code )
	QStringList::const_iterator constIterator;
	for (constIterator = files.constBegin(); constIterator != files.constEnd();
		 ++constIterator)
	{
		outList.push_back( (*constIterator).toStdString() );
	}
	
	if (! outList.empty() )
		m_cvThread->openSequence(outList);
	
}

void MainWindow::openVideo()
{	
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Video File ..."), "~/",
													tr("Video Files (*.avi *.mp4)"));
	if (!fileName.isEmpty())
		m_cvThread->streamVideo(fileName.toStdString());
	
	char b[256];
	int numImgs = m_cvThread->getNumFrames();
	sprintf(b, "%d images in sequence",numImgs);
	ui->action123_files_in_sequence->setText(b);
	m_jumpToDialog->setSequenceLength(numImgs);
}

void MainWindow::openSequence()
{
	m_cvThread->openSequence(m_sequenceDialog->getScanFileThread()->getFoundFiles());
	//jumpToDialog->updateLabels();
}

void MainWindow::togglePlayPause()
{
	if (m_cvThread->isRunning())
		m_stopAction->setText("stop");
	else 
		m_stopAction->setText("play");
	
	m_cvThread->playPause();
	
}

void MainWindow::onCvThreadFinished()
{
	m_stopAction->setText("play");
	statusBar()->showMessage(IDLE_MSG);
}

void MainWindow::handleFileScan(const bool& done)
{	
	char b[100];
	int numImgs = 0;
	
	if(!done)
	{
		//statusBar()->showMessage();
	}
	else 
	{
		printf("scanning ended\n");
		
		numImgs = m_sequenceDialog->getScanFileThread()->getFoundFiles().size();
		
		sprintf(b, "%d images in sequence",numImgs);
		
		if(numImgs > 0 )
		{
			m_cvThread->openSequence(m_sequenceDialog->getScanFileThread()->getFoundFiles());
		}
		
		ui->action123_files_in_sequence->setText(b);
		m_jumpToDialog->setSequenceLength(numImgs);
	}
	
	//ui->actionProcess_sequence->setEnabled(done);
	
	statusBar()->showMessage(done? tr(b) : tr(SCAN_MSG));
	
}

void MainWindow::onImageChange()
{	
	int i = m_cvThread->getCurrentIndex() ;
	
	string imgPath = m_cvThread->getCurrentImgPath();
	
	//if(activeWidget->getCVThread()->getStreamType() == CVThread::STREAM_FILELIST)
	//	imgPath = imgPath.substr(imgPath.find_last_of("/") + 1);
	
	statusBar()->showMessage(imgPath.c_str());
	
	m_jumpToDialog->setIndex(i);
}

void MainWindow::onIndexChange(int i)
{
	m_cvThread->jumpToFrame(i);
}

void MainWindow::activateUSBCamera(bool b)
{
	//bool camRunning = m_cvThread->isUSBCameraActive();
	
    if( b || m_cvThread->isUSBCameraActive())
        m_cvThread->streamUSBCamera(b);
	
	if(b)
	{
        activateIPCamera(false);
        activateKinect(false);
        
		m_stopAction->setText("stop");
		
		ui->actionOpen_camera->setText("stop USB-camera ...");
		
		m_statusMsg = USBCAM_MSG;
	}
	else 
	{
        statusBar()->showMessage("camera turned off");
		ui->actionOpen_camera->setText("start USB-camera ...");
	}
	
}

void MainWindow::activateIPCamera(bool b)
{
	//bool camRunning = m_cvThread->isIPCameraActive();
    
    if( b || m_cvThread->isIPCameraActive())
        m_cvThread->streamIPCamera(b);
	
	if(b)
	{
        activateUSBCamera(false);
        activateKinect(false);
        
		m_stopAction->setText("stop");
		
		ui->actionNetwork_camera->setText("stop IP-camera ...");
		
		m_statusMsg = IPCAM_MSG;
	}
	else 
	{
		statusBar()->showMessage("camera turned off");
		ui->actionNetwork_camera->setText("start IP-camera ...");
	}
}

void MainWindow::activateKinect(bool b)
{
	//bool camRunning = m_cvThread->isKinectActive();
	
    if( b || m_cvThread->isKinectActive())
        m_cvThread->streamKinect(b);
    
	if(b)
	{
        activateUSBCamera(false);
        activateIPCamera(false);
        
		m_stopAction->setText("stop");
		
		ui->actionKinect->setText("stop Kinect ...");
		
		m_statusMsg = KINECT_MSG;
	}
	else 
	{
		statusBar()->showMessage("camera turned off");
		ui->actionKinect->setText("start Kinect ...");
	}
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
	int frameToSkip = 0;
	
	switch (event->key())
    {
		case Qt::Key_Right :
			
			if (event->modifiers() & Qt::AltModifier)
				frameToSkip = 100 ;
			
			else if (event->modifiers() & Qt::ShiftModifier)
				frameToSkip = 10 ;
			else 
				frameToSkip = 1;
			
			m_cvThread->skipFrames(frameToSkip);
			
            break;
			
		case Qt::Key_Left :
			
			if (event->modifiers() & Qt::AltModifier)
				frameToSkip = -100 ;
			
			else if (event->modifiers() & Qt::ShiftModifier)
				frameToSkip = -10 ;
			else 
				frameToSkip = -1;
			
			m_cvThread->skipFrames(frameToSkip);
			
            break;
			
        default:
            break;
	}
}

void  MainWindow::keyReleaseEvent(QKeyEvent* event)
{
	event->accept();
	
    switch (event->key())
    {
        case Qt::Key_Q :
			
            close();
            break;
			
		case Qt::Key_Escape :
			
            showNormal();
            break;
			
		case Qt::Key_F :
			
            toggleFullscreen();
			setFocus();
            break;
            
        case Qt::Key_I :
			
            m_cvThread->setKinectUseIR(!m_cvThread->isKinectUseIR());
            
            break;
			
		case Qt::Key_Space :
			
			m_cvThread->playPause();
            break;
			
		default:
			
            break;
    }
	
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
	toggleFullscreen();
	event->accept();
}

void MainWindow::createContextMenu()
{
    currentAction = 0;
	
    setContextMenuPolicy(Qt::DefaultContextMenu);
    contextMenu = new QMenu(this);
	
	m_stopAction = new QAction("stop", this);
	contextMenu->addAction(m_stopAction);
	
	connect(m_stopAction, SIGNAL(triggered()), this, SLOT(togglePlayPause()));
	
}

void MainWindow::contextMenuEvent(QContextMenuEvent* e)
{
	contextMenu->exec(mapToGlobal(e->pos()));
}