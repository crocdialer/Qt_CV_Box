#ifndef ProcessingDialog_H
#define ProcessingDialog_H

#include <QDialog>
#include "CCFThread.h"
#include "boost/weak_ptr.hpp"

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
    
    boost::shared_ptr<CVThread> m_cvThread;
    
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
    
    void setProcessingThread(boost::shared_ptr<CVThread> theThread);
    
    boost::shared_ptr<CVThread> getProcessingThread(){return m_cvThread;};

protected:
    void changeEvent(QEvent *e);

};

#endif // ProcessingDialog_H
