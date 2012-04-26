#ifndef ProcessingDialog_H
#define ProcessingDialog_H

#include <QDialog>
#include "CCFThread.h"

namespace Ui 
{
    class ProcessingDialog;
}

class ProcessingDialog : public QDialog 
{
    Q_OBJECT

private:
    Ui::ProcessingDialog *ui;
	
	bool m_blockSlider;
    
    CVThreadPtr m_cvThread;
    
    CCFThread *m_procThread;
	
public slots:
	
	void onSliderMove();
	void onSliderPress();
	void onSliderRelease();
	
public:
    ProcessingDialog(QWidget *parent = 0);
    ~ProcessingDialog();
	
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
    
    void setProcessingThread(const CVThreadPtr &theThread);
    
    CVThreadPtr getProcessingThread(){return m_cvThread;};

protected:
    void changeEvent(QEvent *e);

};

#endif // ProcessingDialog_H
