#include <QMenu>
#include <QtWidgets>
#include <QAction>
#include <QActionGroup>

#include "MainWindow.h"
#include "SourceDebugger.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
    sourceDebugger_ = new SourceDebugger(this);
    setCentralWidget(sourceDebugger_);

    setWindowTitle(tr("SynacorDebugger"));

    createMenus();

    resize(1600, 800);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
	
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
}


MainWindow::~MainWindow()
{
}
