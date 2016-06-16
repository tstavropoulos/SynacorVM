#include <QMenu>
#include <QtWidgets>
#include <QSignalMapper>
#include <QAction>
#include <QActionGroup>

#include "MainWindow.h"
#include "SourceDebugger.h"

MainWindow::MainWindow(QWidget *parent)
{
    sourceDebugger_ = new SourceDebugger(this);
    setCentralWidget(sourceDebugger_);

    setWindowTitle(tr("SynacorDebugger"));

    createMenus();

    resize(1060, 600);
}

void MainWindow::createMenus()
{
    //signalMapper = new QSignalMapper(this);

    fileMenu = menuBar()->addMenu(tr("&File"));

	/*
    QAction *saveAction = new QAction("&Save", this);
    fileMenu->addAction(saveAction);
    connect(saveAction, SIGNAL(triggered()), sourceDebugger_, SLOT(save()));
	*/
    QAction *loadAction = new QAction("&Load", this);
    fileMenu->addAction(loadAction);
    connect(loadAction, SIGNAL(triggered()), sourceDebugger_, SLOT(load()));

	fileMenu->addSeparator();

	QAction *reduceAction = new QAction("R&educe", this);
	fileMenu->addAction(reduceAction);
	connect(reduceAction, SIGNAL(triggered()), sourceDebugger_, SLOT(reduce()));

	QAction *runAction = new QAction("&Run", this);
	fileMenu->addAction(runAction);
	connect(runAction, SIGNAL(triggered()), sourceDebugger_, SLOT(run()));

	QAction *resetAction = new QAction("Re&set", this);
	fileMenu->addAction(resetAction);
	connect(resetAction, SIGNAL(triggered()), sourceDebugger_, SLOT(reset()));

	fileMenu->addSeparator();

	QAction *exitAction = new QAction("E&xit", this);
	fileMenu->addAction(exitAction);
	connect(exitAction, SIGNAL(triggered()), sourceDebugger_, SLOT(exit()));


	/*
    QAction *downloadAction = new QAction("&Download", this);
    fileMenu->addAction(downloadAction);
    connect(downloadAction, SIGNAL(triggered()), sourceDebugger_, SLOT(fetch()));

    fileMenu->addSeparator();
    styleMenu = fileMenu->addMenu(tr("S&tyle"));
	*/
}


MainWindow::~MainWindow()
{
}
