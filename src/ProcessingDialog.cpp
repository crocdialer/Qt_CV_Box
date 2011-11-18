#include "ProcessingDialog.h"
#include "ui_ProcessingDialog.h"

#include "mainwindow.h"
#include <QtGui>

ProcessingDialog::ProcessingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessingDialog),
	m_blockSlider(false)
{
    ui->setupUi(this);
	
	}

ProcessingDialog::~ProcessingDialog()
{
    delete ui;
}


void ProcessingDialog::onSliderMove()
{
	
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
