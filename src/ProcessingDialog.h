#ifndef ProcessingDialog_H
#define ProcessingDialog_H

#include <QDialog>

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
	
public slots:
	
	void onSliderMove();
	void onSliderPress();
	void onSliderRelease();
	
public:
    ProcessingDialog(QWidget *parent = 0);
    ~ProcessingDialog();
	
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

protected:
    void changeEvent(QEvent *e);

};

#endif // ProcessingDialog_H
