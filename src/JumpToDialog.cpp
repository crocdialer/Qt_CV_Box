#include "JumpToDialog.h"
#include "ui_JumpToDialog.h"

#include "mainwindow.h"
#include <QtGui>

JumpToDialog::JumpToDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JumpToDialog),
	m_blockSlider(false)
{
    ui->setupUi(this);
	
	this->installEventFilter(parent);
	
	connect(ui->m_slider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMove()));
	connect(ui->m_slider, SIGNAL(sliderPressed()), this, SLOT(onSliderPress()));
	
	connect(ui->m_slider, SIGNAL(sliderReleased()), this, SLOT(onSliderRelease()));
	
	setSequenceLength(0);
}

JumpToDialog::~JumpToDialog()
{
    delete ui;
}

void JumpToDialog::setSequenceLength(const int& i)
{
	char b[256];
	
	CVThread* cvt = MainWindow::getSingletonPtr()->getImgWidget()->getCVThread().get();
	
	QTime t(0,0,0,0);
	int msecs = (int)(i*1000.0/cvt->getFPS() );
	t=t.addMSecs( msecs );
	
	sprintf(b, "%s",t.toString().toStdString().c_str());
	//ui->m_labelCurrent->setText( b );
	
	ui->m_slider->setRange(0, i-1);
	ui->m_labelMax->setText( b );
	
	updateLabels();
}

void JumpToDialog::setIndex(const int& i)
{
	char b[256];
	sprintf(b, "%d",i);
	
	if(!m_blockSlider)
	{
		ui->m_slider->setValue(i);
		updateLabels(i);
	}
}

void JumpToDialog::onSliderMove()
{
	//char b[256];
	//sprintf(b, "%d",ui->m_slider->value());
	
	updateLabels(ui->m_slider->value());
	
	//ui->m_labelCurrent->setText( b );
}

void JumpToDialog::onSliderPress()
{
	m_blockSlider = true ;
	updateLabels(ui->m_slider->value());
}

void JumpToDialog::onSliderRelease()
{
	m_blockSlider = false ;
	
	MainWindow* mainWindow = (MainWindow*)parent() ;
	
	mainWindow->onIndexChange(ui->m_slider->value());
}

void JumpToDialog::updateLabels(const int& i)
{
	MainWindow* mainWindow = (MainWindow*)parent() ;
	CVThread* cvt = mainWindow->getImgWidget()->getCVThread().get();
	int lastIndex = cvt->getNumFrames()-1;
	lastIndex = lastIndex < 0 ? 0:lastIndex ;
	
	char b[512];
	
	
	QTime t(0,0,0,0);
	int msecs = i*1000.0/cvt->getFPS();
	t=t.addMSecs( msecs );
	
	sprintf(b, "%s - index: %d from %d",t.toString().toStdString().c_str(),
										i,
										lastIndex);
	ui->m_labelCurrent->setText( b );
	
	sprintf(b, "img: %dx%d",cvt->getImage().cols,cvt->getImage().rows);
	
	string src = cvt->getCurrentImgPath();
	
	switch (cvt->getStreamType()) 
	{
		case CVThread::STREAM_FILELIST:
		case CVThread::STREAM_VIDEOFILE:
			src = src.substr(src.find_last_of("/") + 1);
			ui->m_slider->setEnabled(true);
			
			break;
			
		case CVThread::STREAM_CAPTURE:
		case CVThread::STREAM_IP_CAM:
			ui->m_slider->setEnabled(false);
			
		default:
			break;
	}
	
	sprintf(b, "%s  -- %s",b,src.c_str());	
	//img label
	ui->m_imgLabel->setText(b);
	
	//grabtime label
	sprintf(b, "grab time: %.2f ms",cvt->getLastGrabTime());
	ui->m_grabLabel->setText(b);
	
	//processTime label
	sprintf(b, "process time: %.2f ms",cvt->getLastProcessTime());
	ui->m_processLabel->setText(b);
	
}

void JumpToDialog::keyPressEvent(QKeyEvent* event)
{
	MainWindow* mainWindow = (MainWindow*)parent() ;

	mainWindow->keyPressEvent(event);
}

void  JumpToDialog::keyReleaseEvent(QKeyEvent* event)
{
	MainWindow* mainWindow = (MainWindow*)parent() ;
	
	mainWindow->keyReleaseEvent(event);
}

void JumpToDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
