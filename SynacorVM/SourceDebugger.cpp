#include "SourceDebugger.h"
#include "OutputWidget.h"
#include "MemoryWidget.h"
#include "AssemblyWidget.h"
#include "SynacorVM.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

#include <iostream>
#include <fstream>


SourceDebugger::SourceDebugger(QWidget *parent)
	: QWidget(parent)//, memory(c_dwAddressSpace)
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

	synacorVM = new SynacorVM();

	connect(synacorVM, SIGNAL(print(const QString&)), outputWidget, SLOT(print(const QString&)));
	connect(synacorVM, SIGNAL(clear()), outputWidget, SLOT(clear()));
}

void SourceDebugger::load()
{
	QString filepath = QFileDialog::getOpenFileName(this, QString("Select Synacor Binary File"));

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

		QStringList instr, args;

		synacorVM->getAssembly(instr, args);
		assemblyWidget->setAssembly(instr, args);
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