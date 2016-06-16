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
		QFile file(filepath);

		if (!file.open(QIODevice::ReadOnly))
		{
			qDebug() << "Cound not open file (reading).";
			return;
		}

		const qint64 bufferSize = c_dwAddressSpace * (sizeof(uint16_t) / sizeof(char));
		//char tempMemory[bufferSize];
		
		QByteArray blob = file.readAll();
		

		uint16_t tempMemory[c_dwAddressSpace];

		for (int i = 0; i < c_dwAddressSpace && 2*i+1 < blob.length(); i++)
		{
			tempMemory[i] = blob[2*i+1] << 8 | blob[2*i];
		}
		
		/*
		//handle file here
		if (!file.read(tempMemory, bufferSize))
		{
			qDebug() << "Read appears to have failed.";
		}
		*/
		synacorVM->load(tempMemory);

		QStringList instr, args;

		synacorVM->getAssembly(instr, args);
		assemblyWidget->setAssembly(instr, args);

		file.close();
	}
}

void SourceDebugger::reduce()
{
	assemblyWidget->reduce();
}
