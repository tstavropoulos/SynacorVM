#include "stdafx.h"

#include "mainwindow.h"

#include <signal.h>

#include <QApplication>
#include <QMetaObject>
#include <QSettings>
#include <QDir>
#include <QtWinExtras>


void signalhandler(int sig) {
	if (sig == SIGINT) {
		qApp->quit();
	}
}

int main(int argc, char *argv[])
{
	qRegisterMetaType<uint16_t>("uint16_t");

    QApplication a(argc, argv);

	QString exeFileName = QCoreApplication::applicationFilePath();
	exeFileName = exeFileName.right(exeFileName.length() - exeFileName.lastIndexOf("/") - 1);
	QString appName = "SynacorVM";

	QSettings regApplications("HKEY_CURRENT_USER\\Software\\Classes\\Applications\\" + exeFileName, QSettings::NativeFormat);
	regApplications.setValue("FriendlyAppName", appName);

	regApplications.beginGroup("SupportedTypes");
	regApplications.setValue(".bin", QString());
	regApplications.endGroup();

	regApplications.beginGroup("shell");
	regApplications.beginGroup("open");
	regApplications.setValue("FriendlyAppName", appName);
	regApplications.beginGroup("command");
	regApplications.setValue(".", '"' + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\" \"%1\"");
	regApplications.endGroup();
	regApplications.endGroup();
	regApplications.endGroup();

	//Create the Jumplist for easy opening of recent files
	QWinJumpList jumplist;
	jumplist.recent()->setTitle("Recent Files");
	jumplist.recent()->setVisible(true);



    MainWindow w;
    w.show();

	QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SIGNAL(aboutToQuit()));
	signal(SIGINT, signalhandler);
    return a.exec();
}
