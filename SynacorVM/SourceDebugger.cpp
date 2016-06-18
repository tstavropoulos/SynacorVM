#include "stdafx.h"

#include "SourceDebugger.h"

#include "OutputWidget.h"
#include "MemoryWidget.h"
#include "AssemblyWidget.h"

#include <iostream>
#include <fstream>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QApplication>
#include <QtConcurrent>
#include <QToolBar>
#include <QToolButton>

#include <QMessageBox>


SourceDebugger::SourceDebugger(QWidget *parent)
	: QWidget(parent)
	, DState(DS_NOT_RUN)
{
	qRegisterMetaType<DebuggerState>("DebuggerState");
	qRegisterMetaType<VMErrors>("VMErrors");


	setObjectName("mainWidget");

	toolbar = new QToolBar(this);

	QAction *loadAction = new QAction("&Open", this);
	loadAction->setIcon(QIcon(":/Open.png"));
	toolbar->addAction(loadAction);
	connect(loadAction, SIGNAL(triggered()), this, SLOT(load()));

	toolbar->addSeparator();

	QAction *reduceAction = new QAction("R&educe", this);
	reduceAction->setIcon(QIcon(":/Reduce.png"));
	toolbar->addAction(reduceAction);
	connect(reduceAction, SIGNAL(triggered()), this, SLOT(reduce()));

	QAction *resetAction = new QAction("Re&set", this);
	resetAction->setIcon(QIcon(":/Reset.png"));
	toolbar->addAction(resetAction);
	connect(resetAction, SIGNAL(triggered()), this, SLOT(reset()));

	toolbar->addSeparator();


	runAction = new QAction("Run", this);
	runAction->setIcon(QIcon(":/Play.png"));
	toolbutton = new QToolButton();
	toolbutton->setDefaultAction(runAction);
	toolbar->addWidget(toolbutton);
	connect(runAction, SIGNAL(triggered()), this, SLOT(resume()));

	QAction *stepIntoAction = new QAction("Step &Into", this);
	stepIntoAction->setIcon(QIcon(":/StepInto.png"));
	toolbar->addAction(stepIntoAction);
	connect(stepIntoAction, SIGNAL(triggered()), this, SLOT(stepInto()));

	QAction *stepOverAction = new QAction("Step &Over", this);
	stepOverAction->setIcon(QIcon(":/StepOver.png"));
	toolbar->addAction(stepOverAction);
	connect(stepOverAction, SIGNAL(triggered()), this, SLOT(stepOver()));


	QVBoxLayout *superLayout = new QVBoxLayout(this);

	superLayout->addWidget(toolbar);

	QHBoxLayout *mainLayout = new QHBoxLayout();
	superLayout->addLayout(mainLayout);

	outputWidget = new OutputWidget(this);

	assemblyWidget = new AssemblyWidget(this);
	memoryWidget = new MemoryWidget(this);

	QVBoxLayout *rightSideLayout = new QVBoxLayout();
	rightSideLayout->addWidget(assemblyWidget);
	rightSideLayout->addWidget(memoryWidget);

	mainLayout->addWidget(outputWidget);
	mainLayout->addLayout(rightSideLayout);

	setLayout(superLayout);

	synacorVM = new SynacorVM(nullptr);

	QThread *VMThread = new QThread(this);
	synacorVM->moveToThread(VMThread);


	//Connect signals from Assembly Widget to VM
	connect(assemblyWidget, SIGNAL(setBreakpoint(uint16_t, bool)), synacorVM, SLOT(setBreakpoint(uint16_t, bool)));

	//Connect signals from VM to Assembly Widget
	connect(synacorVM, SIGNAL(updatePointer(uint16_t)), assemblyWidget, SLOT(updatePointer(uint16_t)));

	//Connect signals from VM to UI
	connect(synacorVM, SIGNAL(throwError(VMErrors)), this, SLOT(notifyError(VMErrors)));
	connect(synacorVM, SIGNAL(newDebuggerState(DebuggerState)), this, SLOT(updateDebuggerState(DebuggerState)));

	//Connect signals from UI to VM
	connect(this, SIGNAL(aboutToQuit()), synacorVM, SLOT(aboutToQuit()));
	connect(this, SIGNAL(pauseVM(bool)), synacorVM, SLOT(pause(bool)));
	connect(this, SIGNAL(activateVM()), synacorVM, SLOT(activateVM()));

	//Connect signals from VM to Output Widget
	connect(synacorVM, SIGNAL(print(const QString&)), outputWidget, SLOT(print(const QString&)));
	connect(synacorVM, SIGNAL(clear()), outputWidget, SLOT(clear()));

	//Connect signals from Output Widget to VM
	connect(outputWidget, SIGNAL(submitInput(const QString&)), synacorVM, SLOT(updateInput(const QString&)));

	//Connect signals from VM to Memory Widget
	connect(synacorVM, SIGNAL(updateMemory(uint16_t, uint16_t)), memoryWidget, SLOT(updateMemory(uint16_t, uint16_t)));
	connect(synacorVM, SIGNAL(updateRegister(uint16_t, uint16_t)), memoryWidget, SLOT(updateRegister(uint16_t, uint16_t)));
	connect(synacorVM, SIGNAL(pushStack(uint16_t)), memoryWidget, SLOT(pushStack(uint16_t)));
	connect(synacorVM, SIGNAL(popStack()), memoryWidget, SLOT(popStack()));
	connect(synacorVM, SIGNAL(pushCallstack(uint16_t)), memoryWidget, SLOT(pushCallstack(uint16_t)));
	connect(synacorVM, SIGNAL(popCallstack()), memoryWidget, SLOT(popCallstack()));
	connect(synacorVM, SIGNAL(updatePointer(uint16_t)), memoryWidget, SLOT(updatePointer(uint16_t)));

	//Connect signals from Memory Widget for modifying VM
	connect(memoryWidget, SIGNAL(changeMemory(uint16_t, uint16_t)), synacorVM, SLOT(changeMemory(uint16_t, uint16_t)));
	connect(memoryWidget, SIGNAL(changeRegister(uint16_t, uint16_t)), synacorVM, SLOT(changeRegister(uint16_t, uint16_t)));
	connect(memoryWidget, SIGNAL(changeStackPush(uint16_t)), synacorVM, SLOT(changeStackPush(uint16_t)));
	connect(memoryWidget, SIGNAL(changeStackPop()), synacorVM, SLOT(changeStackPop()));
	connect(memoryWidget, SIGNAL(changeStackModify(uint16_t, uint16_t)), synacorVM, SLOT(changeStackModify(uint16_t, uint16_t)));

	// Connect signals from Memory Widget to Assembly Widget
	connect(memoryWidget, SIGNAL(scrollToInstruction(uint16_t)), assemblyWidget, SLOT(scrollToInstruction(uint16_t)));

	// Connect signals from Memory Widget to UI
	connect(memoryWidget, SIGNAL(refreshAssembly()), this, SLOT(refreshAssembly()));

	//Run our VM updates endlessly
	//QFuture<void> future = QtConcurrent::run(QThreadPool::globalInstance(), synacorVM, &SynacorVM::updateForever);

	VMThread->start();
	emit activateVM();
}

void SourceDebugger::load()
{
	QString filepath = QFileDialog::getOpenFileName(this, QString("Select Synacor Binary File"), QString(), QString("*.bin"));

	loadfile(filepath);
}

void SourceDebugger::loadfile(const QString &filepath)
{
	if (!filepath.isEmpty())
	{

		std::ifstream in(filepath.toStdString(), std::ifstream::in | std::ifstream::binary);
		if (!in.good())
		{
			std::cout << "Failed to open file: " << filepath.toStdString() << std::endl;
			return;
		}

		std::vector<uint16_t> memory(c_dwAddressSpace);

		size_t dwCurOffset = 0;
		while (in.good())
		{
			in.read((char *)&memory[dwCurOffset++], sizeof(uint16_t));
		}

		synacorVM->load(memory);

		//Update the AssemblyWidget
		QStringList instr, args;
		std::vector<uint16_t> instrNum;

		synacorVM->getAssembly(instr, args, instrNum);
		assemblyWidget->setAssembly(instr, args, instrNum);

		//Pass a copy of the memory to the MemoryWidget
		memoryWidget->load(memory);
	}
}

void SourceDebugger::reduce()
{
	assemblyWidget->reduce();
}

void SourceDebugger::exit()
{
	QApplication::quit();
}

void SourceDebugger::notifyError(VMErrors error)
{
	switch (error)
	{
	case VME_RUN_NO_FILE_LOADED:
	{
		QMessageBox::warning(this,
			QString("Run Error"),
			QString("You cannot run the program until you load a binary file."));
		break;
	}
	case VME_RESET_NO_FILE_LOADED:
	{
		QMessageBox::warning(this,
			QString("Reset Error"),
			QString("You cannot reset the program until you load a binary file."));
		break;
	}
	default:
	{
		QMessageBox::warning(this,
			QString("Error Error"),
			QString("This error message has not been implemented.  Error Code: %1").arg(error));
		break;
	}
	}
}

void SourceDebugger::reset()
{
	synacorVM->reset();
	outputWidget->clear();
	memoryWidget->reset();

	refreshAssembly();
}

void SourceDebugger::refreshAssembly()
{
	QStringList instr, args;
	std::vector<uint16_t> instrNum;

	synacorVM->getAssembly(instr, args, instrNum);
	if (instr.length() != 0)
	{
		assemblyWidget->setAssembly(instr, args, instrNum);
	}
}

void SourceDebugger::updateDebuggerState(DebuggerState dState)
{
	if (dState == DState)
	{
		return;
	}

	DState = dState;

	switch (DState)
	{
	case DS_NOT_RUN:
		runAction->setDisabled(false);
		runAction->setText("&Run");
		runAction->setIcon(QIcon(":/Play.png"));
		break;
	case DS_PAUSED:
		runAction->setText("&Resume");
		runAction->setIcon(QIcon(":/Play.png"));
		break;
	case DS_RUNNING:
		runAction->setText("&Pause");
		runAction->setIcon(QIcon(":/Pause.png"));
		break;
	case DS_HALTED:
		runAction->setText("&Run");
		runAction->setIcon(QIcon(":/Play.png"));
		runAction->setDisabled(true);
		break;
	}
	toolbutton->setDefaultAction(runAction);
}

void SourceDebugger::resume()
{
	switch (DState)
	{
	case DS_NOT_RUN:
	case DS_PAUSED:
		emit pauseVM(false);
		break;
	case DS_RUNNING:
		emit pauseVM(true);
		break;
	case DS_HALTED:
		//doNothing
		break;
	}
}

void SourceDebugger::pause()
{
	emit pauseVM(true);
}

void SourceDebugger::stepInto()
{
	synacorVM->stepInto();
}

void SourceDebugger::stepOver()
{
	synacorVM->stepOver();
}