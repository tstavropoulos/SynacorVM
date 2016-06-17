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

	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");

	QWidget *memoryPage = new QWidget();
	memoryPage->setLayout(new QHBoxLayout(memoryPage));
	memoryView = new QListView(memoryPage);
	memoryPage->layout()->addWidget(memoryView);
	memoryModel = new QStringListModel(memoryPage);
	memoryView->setModel(memoryModel);
	memoryView->setFont(monoSpacedFont);
	memoryView->setMovement(QListView::Static);
	memoryView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tabWidget->addTab(memoryPage, "Memory");

	QWidget *registerPage = new QWidget();
	registerPage->setLayout(new QHBoxLayout(registerPage));
	registerView = new QListView(registerPage);
	registerPage->layout()->addWidget(registerView);
	registerModel = new QStringListModel(registerPage);
	registerView->setModel(registerModel);
	registerView->setFont(monoSpacedFont);
	registerView->setMovement(QListView::Static);
	registerView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tabWidget->addTab(registerPage, "Registers");

	QWidget *stackPage = new QWidget();
	stackPage->setLayout(new QHBoxLayout(stackPage));
	stackView = new QListView(stackPage);
	stackPage->layout()->addWidget(stackView);
	stackModel = new QStringListModel(stackPage);
	stackView->setModel(stackModel);
	stackView->setFont(monoSpacedFont);
	stackView->setMovement(QListView::Static);
	stackView->setEditTriggers(QAbstractItemView::NoEditTriggers);

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
		memory[address] = QString("%1:\t%2").arg(address, 4, 16, QChar('0')).arg(value, 4, 16, QChar('0'));
	}
	else if (address <= 32775)
	{
		registers[address - 32768] = QString("r%1:\t%2").arg(address - 32768, 2, 16, QChar('0')).arg(value, 4, 16, QChar('0'));
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
		startMemoryBU << QString("%1:\t%2").arg(index++, 4, 16, QChar('0')).arg(*iter, 4, 16, QChar('0'));
	}

	memory = QStringList(startMemoryBU);
	refreshView(MM_MEMORY);

	registers.clear();
	for (int i = 0; i < c_dwNumRegisters; i++)
	{
		registers << QString("r%1:\t%2").arg(i, 2, 16, QChar('0')).arg(0, 4, 16, QChar('0'));
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
		registers << QString("r%1:\t%2").arg(i, 2, 16, QChar('0')).arg(0, 4, 16, QChar('0'));
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
	stack.push_back(QString("%1").arg(value,4,16,QChar('0')));

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
