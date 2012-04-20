#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	string path(argv[0]);
	path = path.substr(0,path.find_last_of("/")+1);
	printf("executable path: %s\n",path.c_str());
	
	chdir( (path + "../Resources/").c_str() );
#endif
	char buf[512];
	getcwd(buf, 512);
	printf("current dir: %s\n",buf);
	
    QApplication a(argc, argv);
    MainWindow w;

    w.resize(1000, 600);
	w.move(QPoint(300,150));
    w.show();
	
	w.setAnimated(true);
	
//    QMainWindow w2;
//    w2.resize(640, 480);
//    w2.show();
    
	/*
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/Icons/persona.png"), QSize(), QIcon::Normal, QIcon::On);
	a.setWindowIcon(icon);
	*/
	
    return a.exec();
}
