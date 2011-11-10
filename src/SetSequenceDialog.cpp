#include "SetSequenceDialog.h"
#include "ui_SetSequenceDialog.h"

#include <QFileDialog>



//#include <iostream>

using namespace std;

SetSequenceDialog::SetSequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetSequenceDialog)
{
    ui->setupUi(this);
	
	m_subDirs = ui->m_subDirBox->checkState() == Qt::Checked ;
	m_sequencePath = ui->m_textLine->displayText().toStdString() ;
	
	scanThread = new ScanFilesThread(m_sequencePath) ;
	
	connect(scanThread, SIGNAL(scanEnded(const bool&)), parent, SLOT(handleFileScan(const bool&)) );
	
	connect(ui->m_dirButton, SIGNAL(pressed()), this, SLOT(browseFolders()) );
	
}

SetSequenceDialog::~SetSequenceDialog()
{
    delete ui;
	scanThread->stop();
	scanThread->wait();
	
	delete scanThread;
}

void SetSequenceDialog::accept()
{
//	printf("accept\n");
	m_subDirs = ui->m_subDirBox->checkState() == Qt::Checked ;
	m_sequencePath = ui->m_textLine->displayText().toStdString() ;
	
	
	
	vector<string> files;
	
	ui->m_numLabel->setText("scanning folders ...");
	
	scanThread->setRootDir(m_sequencePath);
	scanThread->setUseRecursion(m_subDirs);
	scanThread->start();
	
	
	
	hide();
}

void SetSequenceDialog::reject()
{
	//leave settings as be and ciao
	
	ui->m_subDirBox->setChecked(m_subDirs);
	ui->m_textLine->setText(m_sequencePath.c_str()) ;
	hide();

}

void SetSequenceDialog::browseFolders()
{
	QString folderName = QFileDialog::getExistingDirectory(this, tr("Choose Root Folder ..."), "~/",
													QFileDialog::ShowDirsOnly);
	if (!folderName.isEmpty())
	{
		m_sequencePath = folderName.toStdString();
		ui->m_textLine->setText(m_sequencePath.c_str()) ;
	}

}





void SetSequenceDialog::changeEvent(QEvent *e)
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
