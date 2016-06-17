#include "mainwindow.h"
#include <QApplication>
#include <QMetaObject>
#include <signal.h>
#include <QObject>


void signalhandler(int sig) {
	if (sig == SIGINT) {
		qApp->quit();
	}
}

int main(int argc, char *argv[])
{
	qRegisterMetaType<uint16_t>("uint16_t");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

	QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SIGNAL(aboutToQuit()));
	signal(SIGINT, signalhandler);
    return a.exec();
}
