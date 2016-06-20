#include "stdafx.h"

#include "SourceDebugger.h"

#include "OutputWidget.h"
#include "MemoryWidget.h"
#include "AssemblyWidget.h"

SourceDebugger::SourceDebugger(QMainWindow *parent)
	: QObject(parent)
	, DState(DS_NOT_RUN)
	, parentWindow (parent)
{
	qRegisterMetaType<DebuggerState>("DebuggerState");
	qRegisterMetaType<VMErrors>("VMErrors");
	qRegisterMetaType<StackSource>("StackSource");

	setObjectName("mainWidget");

	toolbar = new QToolBar(parent);

	QAction *loadAction = new QAction("&Open", this);
	loadAction->setIcon(QIcon(":/Open.png"));
	toolbar->addAction(loadAction);
	connect(loadAction, SIGNAL(triggered()), this, SLOT(load()));

	QAction *saveAction = new QAction("&Save State", this);
	saveAction->setIcon(QIcon(":/Save.png"));
	toolbar->addAction(saveAction);
	connect(saveAction, SIGNAL(triggered()), this, SLOT(saveState()));

	toolbar->addSeparator();

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

	QAction *stepOutAction = new QAction("Step Ou&t", this);
	stepOutAction->setIcon(QIcon(":/StepOut.png"));
	toolbar->addAction(stepOutAction);
	connect(stepOutAction, SIGNAL(triggered()), this, SLOT(stepOut()));

	parent->addToolBar(Qt::TopToolBarArea, toolbar);

	outputWidget = new OutputWidget(parentWindow);
	assemblyWidget = new AssemblyWidget(parentWindow);
	memoryWidget = new MemoryWidget(parentWindow);

	parent->addDockWidget(Qt::LeftDockWidgetArea, outputWidget);
	parent->addDockWidget(Qt::RightDockWidgetArea, assemblyWidget);
	parent->addDockWidget(Qt::RightDockWidgetArea, memoryWidget);


	QWidget *tmp = new QWidget(parent);
	tmp->setMaximumSize(0, 0);
	parent->setCentralWidget(tmp);

	synacorVM = new SynacorVM(nullptr);

	QThread *VMThread = new QThread(this);
	synacorVM->moveToThread(VMThread);


	//Connect signals from Assembly Widget to VM
	connect(assemblyWidget, SIGNAL(setBreakpoint(uint16_t, bool)), synacorVM, SLOT(setBreakpoint(uint16_t, bool)));

	//Connect signals from VM to Assembly Widget
	connect(synacorVM, SIGNAL(updatePointer(uint16_t)), assemblyWidget, SLOT(updatePointer(uint16_t)));
	connect(synacorVM, SIGNAL(updateBreakpoint(uint16_t, bool)), assemblyWidget, SLOT(updateBreakpoint(uint16_t, bool)));

	//Connect signals from VM to UI
	connect(synacorVM, SIGNAL(throwError(VMErrors)), this, SLOT(notifyError(VMErrors)));
	connect(synacorVM, SIGNAL(newDebuggerState(DebuggerState)), this, SLOT(updateDebuggerState(DebuggerState)));
	connect(synacorVM, SIGNAL(updateRecentPath(const QString &)), this, SLOT(updateRecentPath(const QString &)));

	//Connect signals from UI to VM
	connect(this, SIGNAL(aboutToQuit()), synacorVM, SLOT(aboutToQuit()));
	connect(this, SIGNAL(pauseVM(bool)), synacorVM, SLOT(pause(bool)));
	connect(this, SIGNAL(activateVM()), synacorVM, SLOT(activateVM()));
	connect(this, SIGNAL(getSaveState(const QString &)), synacorVM, SLOT(getSaveState(const QString &)));
	connect(this, SIGNAL(putLoadState(const QString &)), synacorVM, SLOT(putLoadState(const QString &)));

	//Connect signals from VM to Output Widget
	connect(synacorVM, SIGNAL(print(const QString&)), outputWidget, SLOT(print(const QString&)));
	connect(synacorVM, SIGNAL(clear()), outputWidget, SLOT(clear()));

	//Connect signals from Output Widget to VM
	connect(outputWidget, SIGNAL(submitInput(const QString&)), synacorVM, SLOT(updateInput(const QString&)));

	//Connect signals from VM to Memory Widget
	connect(synacorVM, SIGNAL(updateMemory(uint16_t, uint16_t)), memoryWidget, SLOT(updateMemory(uint16_t, uint16_t)));
	connect(synacorVM, SIGNAL(updateRegister(uint16_t, uint16_t)), memoryWidget, SLOT(updateRegister(uint16_t, uint16_t)));
	connect(synacorVM, SIGNAL(pushStack(uint16_t, StackSource)), memoryWidget, SLOT(pushStack(uint16_t, StackSource)));
	connect(synacorVM, SIGNAL(popStack()), memoryWidget, SLOT(popStack()));
	connect(synacorVM, SIGNAL(clearStack()), memoryWidget, SLOT(clearStack()));
	connect(synacorVM, SIGNAL(setCallAddress(uint16_t)), memoryWidget, SLOT(setCallAddress(uint16_t)));
	connect(synacorVM, SIGNAL(updatePointer(uint16_t)), memoryWidget, SLOT(updatePointer(uint16_t)));

	//Connect signals from Memory Widget for modifying VM
	connect(memoryWidget, SIGNAL(changeMemory(uint16_t, uint16_t)), synacorVM, SLOT(changeMemory(uint16_t, uint16_t)));
	connect(memoryWidget, SIGNAL(changeRegister(uint16_t, uint16_t)), synacorVM, SLOT(changeRegister(uint16_t, uint16_t)));
	connect(memoryWidget, SIGNAL(changeStackPush(uint16_t, StackSource source)), synacorVM, SLOT(changeStackPush(uint16_t, StackSource source)));
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

void RecentOpener::load()
{
	sourceDebugger->loadfile(file);
}

void SourceDebugger::load()
{
	QSettings settings;
	QString path = settings.value("Recent/Path").toString();

	QString filepath = QFileDialog::getOpenFileName(parentWindow, QString("Open File"), path, QString("Binary or Save State(*.bin *.syns);;All files (*.*)"));
	loadfile(filepath);
}

void SourceDebugger::updateRecentPath(const QString &filepath)
{
	QFileInfo file(filepath);
	if (!file.exists())
	{
		return;
	}

	QSettings settings;
	bool foundExisting = false;
	for (uint32_t i = 0; i < MAX_RECENT_FILES; i++)
	{
		if (!foundExisting)
		{
			if (settings.value(QString("Recent/%1").arg(i)).toString().toCaseFolded() == filepath.toCaseFolded())
			{
				foundExisting = true;
			}
		}
		else
		{
			settings.setValue(QString("Recent/%1").arg(i - 1), settings.value(QString("Recent/%1").arg(i)));
		}
	}
	if (foundExisting)
	{
		settings.setValue(QString("Recent/%1").arg(MAX_RECENT_FILES - 1), "");
	}

	settings.setValue("Recent/Path", file.absoluteDir().absolutePath());
	for (uint32_t i = MAX_RECENT_FILES - 1; i > 0; i--)
	{
		settings.setValue(QString("Recent/%1").arg(i), settings.value(QString("Recent/%1").arg(i - 1)));
	}
	settings.setValue("Recent/0", filepath);
	settings.sync();
}

bool SourceDebugger::loadfile(const QString &filepath)
{
	if (filepath.isEmpty())
	{
		return false;
	}

	QFileInfo file(filepath);
	if (!file.exists())
	{
		return false;
	}
	updateRecentPath(filepath);

	if (file.suffix() == "syns")
	{
		emit putLoadState(filepath);
		return true;
	}

	const qint64 fileSize = file.size();
	if (fileSize > c_dwAddressSpace * sizeof(uint16_t))
	{
		return false;
	}

	std::ifstream in(filepath.toStdString(), std::ifstream::in | std::ifstream::binary);
	if (!in.good())
	{
		std::cout << "Failed to open file: " << filepath.toStdString() << std::endl;
		return false;
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

	return true;
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
		QMessageBox::warning(parentWindow,
			QString("Run Error"),
			QString("You cannot run the program until you load a binary file."));
		break;
	}
	case VME_RESET_NO_FILE_LOADED:
	{
		QMessageBox::warning(parentWindow,
			QString("Reset Error"),
			QString("You cannot reset the program until you load a binary file."));
		break;
	}
	default:
	{
		QMessageBox::warning(parentWindow,
			QString("Error Error"),
			QString("This error message has not been implemented.  Error Code: %1").arg(error));
		break;
	}
	}
}

void SourceDebugger::saveState()
{
	QSettings settings;
	QString path = settings.value("Recent/Path").toString();

	QString filepath = QFileDialog::getSaveFileName(parentWindow, QString("Save State"), path, QString("Save State(*.syns)"));
	if (filepath.isEmpty())
	{
		return;
	}
	emit getSaveState(filepath);
	updateRecentPath(filepath);
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

void SourceDebugger::stepOut()
{
	synacorVM->stepOut();
}