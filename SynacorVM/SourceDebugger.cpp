#include "SourceDebugger.h"
#include "OutputWidget.h"
#include "MemoryWidget.h"
#include "AssemblyWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QApplication>
#include <QtConcurrent>

#include <QMessageBox>

#include <iostream>
#include <fstream>


SourceDebugger::SourceDebugger(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("mainWidget");

	QHBoxLayout *mainLayout = new QHBoxLayout(this);

	outputWidget = new OutputWidget(this);

	assemblyWidget = new AssemblyWidget(this);
	memoryWidget = new MemoryWidget(this);

	QVBoxLayout *rightSideLayout = new QVBoxLayout();
	rightSideLayout->addWidget(assemblyWidget);
	rightSideLayout->addWidget(memoryWidget);

	mainLayout->addWidget(outputWidget);
	mainLayout->addLayout(rightSideLayout);

	setLayout(mainLayout);

	synacorVM = new SynacorVM(this);

	//Connect signals from Assembly Widget to VM
	connect(assemblyWidget, SIGNAL(setBreakpoint(uint16_t, bool)), synacorVM, SLOT(setBreakpoint(uint16_t, bool)));

	//Connect signals from VM to Assembly Widget
	connect(synacorVM, SIGNAL(updatePointer(uint16_t)), assemblyWidget, SLOT(updatePointer(uint16_t)));

	//Connect signals from VM to UI
	connect(synacorVM, SIGNAL(throwError(VMErrors)), this, SLOT(notifyError(VMErrors)));

	//Connect signals from UI to VM
	connect(this, SIGNAL(aboutToQuit()), synacorVM, SLOT(aboutToQuit()));

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

	//Run our VM updates endlessly
	QFuture<void> future = QtConcurrent::run(QThreadPool::globalInstance(), synacorVM, &SynacorVM::updateForever);
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

void SourceDebugger::run()
{
	synacorVM->run();
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

	QStringList instr, args;

	std::vector<uint16_t> instrNum;

	synacorVM->getAssembly(instr, args, instrNum);

	if (instr.length() != 0)
	{
		assemblyWidget->setAssembly(instr, args, instrNum);
	}
}

void SourceDebugger::resume()
{
	synacorVM->pause(false);
}

void SourceDebugger::pause()
{
	synacorVM->pause(true);
}

void SourceDebugger::stepInto()
{
	synacorVM->stepInto();
}

void SourceDebugger::stepOver()
{
	synacorVM->stepOver();
}