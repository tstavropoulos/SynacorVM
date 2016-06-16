#include "MemoryWidget.h"
#include "SynacorVM.h"

#include <QListView>
#include <QHBoxLayout>
#include <assert.h>
#include <QTabWidget>
#include <QStringListModel>

MemoryWidget::MemoryWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	QTabWidget *tabWidget = new QTabWidget(this);
	layout->addWidget(tabWidget);

	QWidget *memoryPage = new QWidget();
	memoryPage->setLayout(new QHBoxLayout(memoryPage));
	memoryView = new QListView(memoryPage);
	memoryPage->layout()->addWidget(memoryView);
	memoryModel = new QStringListModel(memoryPage);
	memoryView->setModel(memoryModel);

	tabWidget->addTab(memoryPage, "Memory");

	QWidget *registerPage = new QWidget();
	registerPage->setLayout(new QHBoxLayout(registerPage));
	registerView = new QListView(registerPage);
	registerPage->layout()->addWidget(registerView);
	registerModel = new QStringListModel(registerPage);
	registerView->setModel(registerModel);

	tabWidget->addTab(registerPage, "Registers");

	QWidget *stackPage = new QWidget();
	stackPage->setLayout(new QHBoxLayout(stackPage));
	stackView = new QListView(stackPage);
	stackPage->layout()->addWidget(stackView);
	stackModel = new QStringListModel(stackPage);
	stackView->setModel(stackModel);

	tabWidget->addTab(stackPage, "Stack");
	
	setLayout(layout);

	for (int i = 0; i < MM_MAX; i++)
	{
		memoryDirty[i] = false;
	}
}

void MemoryWidget::update(uint16_t address, uint16_t value)
{
	if (address <= 32767)
	{
		memory[address] = QString::number(address, 16).toUpper().rightJustified(4,'0') + ":\t" + QString::number(value, 16).toUpper().rightJustified(4, '0');
	}
	else if (address <= 32775)
	{
		registers[address - 32768] = QString::number(address - 32768, 16).toUpper().rightJustified(4, '0') + ":\t" + QString::number(value, 16).toUpper().rightJustified(4, '0');;
	}
	else
	{
		assert(0 && "Invalid address");
	}
}


void MemoryWidget::load(const std::vector<uint16_t> &buffer)
{
	startMemoryBU.clear();
	int index = 0;
	for (auto iter = buffer.begin(); iter != buffer.end(); iter++)
	{
		startMemoryBU << QString::number(index++, 16).toUpper().rightJustified(4, '0') + ":\t" + QString::number(*iter, 16).toUpper().rightJustified(4, '0');
	}

	memory = QStringList(startMemoryBU);
	refreshView(MM_MEMORY);

	registers.clear();
	for (int i = 0; i < c_dwNumRegisters; i++)
	{
		registers << QString::number(i, 16).toUpper().rightJustified(4, '0') + ":\t" + QString::number(0, 16).toUpper().rightJustified(4, '0');;
	}
	refreshView(MM_REGISTERS);

	stack.clear();
	refreshView(MM_STACK);
}

void MemoryWidget::reset()
{
	memory = QStringList(startMemoryBU);
	refreshView(MM_MEMORY);

	registers.clear();
	for (int i = 0; i < c_dwNumRegisters; i++)
	{
		registers << QString::number(i, 16).toUpper().rightJustified(4, '0') + ":\t" + QString::number(0, 16).toUpper().rightJustified(4, '0');;
	}
	refreshView(MM_REGISTERS);

	stack.clear();
	refreshView(MM_STACK);
}

void MemoryWidget::update()
{
	for (int i = 0; i < MM_MAX; i++)
	{
		if (memoryDirty[i])
		{
			memoryDirty[i] = false;
			refreshView((MemoryModule)i);
		}
	}
}

void MemoryWidget::updateMemory(uint16_t address, uint16_t value)
{
	update(address, value);

	if (address <= 32767)
	{
		flagDirty(MM_MEMORY);
	}
	else
	{
		flagDirty(MM_REGISTERS);
	}
}

void MemoryWidget::updateRegister(uint16_t reg, uint16_t value)
{
	update(reg + 32768, value);

	flagDirty(MM_REGISTERS);
}

void MemoryWidget::pushStack(uint16_t value)
{
	stack.push_back(QString::number(value, 16).toUpper().rightJustified(4, '0'));

	flagDirty(MM_STACK);
}

void MemoryWidget::popStack()
{
	stack.pop_back();

	flagDirty(MM_STACK);
}

void MemoryWidget::refreshView(MemoryModule module)
{
	switch (module)
	{
	case MM_MEMORY:
	{
		memoryModel->setStringList(memory);
		break;
	}
	case MM_STACK:
	{
		stackModel->setStringList(stack);
		break;
	}
	case MM_REGISTERS:
	{
		registerModel->setStringList(registers);
		break;
	}
	}
}

void MemoryWidget::flagDirty(MemoryModule module)
{
	memoryDirty[module] = true;
}
