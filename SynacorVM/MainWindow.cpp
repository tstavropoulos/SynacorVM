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

	connect(this, SIGNAL(aboutToQuit()), sourceDebugger_, SIGNAL(aboutToQuit()));

    resize(1600, 800);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
	
    QAction *loadAction = new QAction("&Open", this);
	loadAction->setShortcut(QKeySequence(tr("Ctrl+O", "File|&Open")));
    fileMenu->addAction(loadAction);
    connect(loadAction, SIGNAL(triggered()), sourceDebugger_, SLOT(load()));

	fileMenu->addSeparator();

	QAction *reduceAction = new QAction("R&educe", this);
	fileMenu->addAction(reduceAction);
	connect(reduceAction, SIGNAL(triggered()), sourceDebugger_, SLOT(reduce()));

	QAction *runAction = new QAction("&Run", this);
	runAction->setShortcut(QKeySequence(tr("Ctrl+R", "File|&Run")));
	fileMenu->addAction(runAction);
	connect(runAction, SIGNAL(triggered()), sourceDebugger_, SLOT(run()));

	QAction *resetAction = new QAction("Re&set", this);
	fileMenu->addAction(resetAction);
	connect(resetAction, SIGNAL(triggered()), sourceDebugger_, SLOT(reset()));

	fileMenu->addSeparator();

	QAction *pauseAction = new QAction("&Pause", this);
	pauseAction->setShortcut(QKeySequence(tr("Shift+F5", "File|&Pause")));
	fileMenu->addAction(pauseAction);
	connect(pauseAction, SIGNAL(triggered()), sourceDebugger_, SLOT(pause()));

	QAction *resumeAction = new QAction("Res&ume", this);
	resumeAction->setShortcut(QKeySequence(tr("F5", "File|Res&ume")));
	fileMenu->addAction(resumeAction);
	connect(resumeAction, SIGNAL(triggered()), sourceDebugger_, SLOT(resume()));

	QAction *stepIntoAction = new QAction("Step &Into", this);
	stepIntoAction->setShortcut(QKeySequence(tr("F11", "Step &Into")));
	fileMenu->addAction(stepIntoAction);
	connect(stepIntoAction, SIGNAL(triggered()), sourceDebugger_, SLOT(stepInto()));

	QAction *stepOverAction = new QAction("Step &Over", this);
	stepOverAction->setShortcut(QKeySequence(tr("F10", "Step &Over")));
	fileMenu->addAction(stepOverAction);
	connect(stepOverAction, SIGNAL(triggered()), sourceDebugger_, SLOT(stepOver()));

	fileMenu->addSeparator();

	QAction *exitAction = new QAction("E&xit", this);
	fileMenu->addAction(exitAction);
	connect(exitAction, SIGNAL(triggered()), sourceDebugger_, SLOT(exit()));
}


MainWindow::~MainWindow()
{
}
