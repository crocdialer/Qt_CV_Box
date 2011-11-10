#ifndef JUMPTODIALOG_H
#define JUMPTODIALOG_H

#include <QDialog>

namespace Ui 
{
    class JumpToDialog;
}

class JumpToDialog : public QDialog 
{
    Q_OBJECT

private:
    Ui::JumpToDialog *ui;
	
	bool m_blockSlider;
	
public slots:
	
	void onSliderMove();
	void onSliderPress();
	void onSliderRelease();
	
public:
    JumpToDialog(QWidget *parent = 0);
    ~JumpToDialog();
	
	void setIndex(const int& i);
	void setSequenceLength(const int& i);
	
	
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	
	
signals:
	
	void indexChanged(int i);

protected:
    void changeEvent(QEvent *e);

	void updateLabels(const int& i=0);


};

#endif // JUMPTODIALOG_H
