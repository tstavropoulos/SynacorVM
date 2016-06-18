#include "stdafx.h"

#include "MainWindow.h"
#include "SourceDebugger.h"

#include <QWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QActionGroup>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
    sourceDebugger_ = new SourceDebugger(this);

	setWindowIcon(QIcon(":/Play.png"));

	//Load Stylesheet
	QFile styleFile(":/DarkTheme.qss");
	styleFile.open(QFile::ReadOnly);

	//Apply the loaded stylesheet
	QString style(styleFile.readAll());
	qApp->setStyleSheet(style);

    setWindowTitle(tr("SynacorDebugger"));

    createMenus();

	connect(this, SIGNAL(aboutToQuit()), sourceDebugger_, SIGNAL(aboutToQuit()));

    resize(1600, 800);

	//Load a file from a commandline argument
	for (int i = 1; i < QCoreApplication::arguments().size(); i++)
	{
		const QString arg = QCoreApplication::arguments().at(i);
		if (!arg.isEmpty() && arg.at(0) != '-' && QFile(arg).exists())
		{
			if (sourceDebugger_->loadfile(arg))
			{
				break;
			}
		}
	}
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
	
    QAction *loadAction = new QAction("&Open", this);
	loadAction->setShortcut(QKeySequence(tr("Ctrl+O", "File|&Open")));
    fileMenu->addAction(loadAction);
    connect(loadAction, SIGNAL(triggered()), sourceDebugger_, SLOT(load()));

	fileMenu->addSeparator();

	QAction *exitAction = new QAction("E&xit", this);
	fileMenu->addAction(exitAction);
	connect(exitAction, SIGNAL(triggered()), sourceDebugger_, SLOT(exit()));

	debugMenu = menuBar()->addMenu(tr("&Debug"));

	QAction *runAction = new QAction("&Run", this);
	runAction->setShortcut(QKeySequence(tr("Ctrl+R", "File|&Run")));
	debugMenu->addAction(runAction);
	connect(runAction, SIGNAL(triggered()), sourceDebugger_, SLOT(resume()));

	QAction *resetAction = new QAction("Re&set", this);
	debugMenu->addAction(resetAction);
	connect(resetAction, SIGNAL(triggered()), sourceDebugger_, SLOT(reset()));

	debugMenu->addSeparator();

	QAction *pauseAction = new QAction("&Pause", this);
	pauseAction->setShortcut(QKeySequence(tr("Shift+F5", "File|&Pause")));
	debugMenu->addAction(pauseAction);
	connect(pauseAction, SIGNAL(triggered()), sourceDebugger_, SLOT(pause()));

	QAction *resumeAction = new QAction("Res&ume", this);
	resumeAction->setShortcut(QKeySequence(tr("F5", "File|Res&ume")));
	debugMenu->addAction(resumeAction);
	connect(resumeAction, SIGNAL(triggered()), sourceDebugger_, SLOT(resume()));

	QAction *stepIntoAction = new QAction("Step &Into", this);
	stepIntoAction->setShortcut(QKeySequence(tr("F11", "Step &Into")));
	debugMenu->addAction(stepIntoAction);
	connect(stepIntoAction, SIGNAL(triggered()), sourceDebugger_, SLOT(stepInto()));

	QAction *stepOverAction = new QAction("Step &Over", this);
	stepOverAction->setShortcut(QKeySequence(tr("F10", "Step &Over")));
	debugMenu->addAction(stepOverAction);
	connect(stepOverAction, SIGNAL(triggered()), sourceDebugger_, SLOT(stepOver()));

	QAction *stepOutAction = new QAction("Step Ou&t", this);
	stepOutAction->setShortcut(QKeySequence(tr("Shift+F11", "Step Ou&t")));
	debugMenu->addAction(stepOutAction);
	connect(stepOutAction, SIGNAL(triggered()), sourceDebugger_, SLOT(stepOut()));
}


MainWindow::~MainWindow()
{
}
