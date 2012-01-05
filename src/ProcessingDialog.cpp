#include "ProcessingDialog.h"
#include "ui_ProcessingDialog.h"

#include "mainwindow.h"
#include <QtGui>

using namespace std;
using namespace boost;

ProcessingDialog::ProcessingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessingDialog),
	m_blockSlider(false),
    m_procThread(NULL)
{
    ui->setupUi(this);
    
    connect(ui->m_blendSlider, SIGNAL(sliderMoved(int)), this, SLOT(onSliderMove()));
	connect(ui->m_blendSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPress()));
	
	connect(ui->m_blendSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderRelease()));
	
}

ProcessingDialog::~ProcessingDialog()
{
    delete ui;
}

void ProcessingDialog::setProcessingThread(shared_ptr<CVThread> theThread)
{
    if(CCFThread* thr=static_cast<CCFThread*>(theThread.get()))
        m_procThread = thr;
    
    ui->m_inputImg->setCVThread(theThread);
    //ui->m_procImg->setCVThread(theThread);
}

void ProcessingDialog::onSliderMove()
{
    float frac = ui->m_blendSlider->value()/(float)    ui->m_blendSlider->maximum();
	m_procThread->getActiveTask()->setBlendValue(frac);
}

void ProcessingDialog::onSliderPress()
{
	m_blockSlider = true ;
	//updateLabels(ui->m_slider->value());
}

void ProcessingDialog::onSliderRelease()
{
	m_blockSlider = false ;
	
	//MainWindow* mainWindow = (MainWindow*)parent() ;
	
}

void ProcessingDialog::keyPressEvent(QKeyEvent* event)
{
	MainWindow* mainWindow = (MainWindow*)parent() ;

	mainWindow->keyPressEvent(event);
}

void  ProcessingDialog::keyReleaseEvent(QKeyEvent* event)
{
	MainWindow* mainWindow = (MainWindow*)parent() ;
	
	mainWindow->keyReleaseEvent(event);
}

void ProcessingDialog::changeEvent(QEvent *e)
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
