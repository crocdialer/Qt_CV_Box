#ifndef SETSEQUENCEDIALOG_H
#define SETSEQUENCEDIALOG_H

#include <QDialog>
#include "ScanFilesThread.h"

namespace Ui {
    class SetSequenceDialog;
}

class SetSequenceDialog : public QDialog {
    Q_OBJECT

private:
	
	bool m_subDirs;
	std::string m_sequencePath;
    Ui::SetSequenceDialog *ui;
	
	ScanFilesThread* scanThread;
	
public:
    SetSequenceDialog(QWidget *parent = 0);
    ~SetSequenceDialog();
	
	ScanFilesThread* getScanFileThread(){return scanThread;};
	Ui::SetSequenceDialog* getUI(){return ui;};

protected:
    void changeEvent(QEvent *e);

	
public slots:
	void accept();
	void reject();
	void browseFolders();
	
};

#endif // SETSEQUENCEDIALOG_H
