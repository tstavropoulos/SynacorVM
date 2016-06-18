#include "stdafx.h"

#include "MemoryWidget.h"
#include "SynacorVM.h"

#include <assert.h>
#include <mutex>

#include <QListView>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QStringListModel>

std::mutex mtxMemoryLock;

MemoryWidget::MemoryWidget(QWidget *parent)
	: QDockWidget("Memory", parent)
{
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	QWidget *internalWidget = new QWidget(this);

	QHBoxLayout *layout = new QHBoxLayout(internalWidget);

	QTabWidget *tabWidget = new QTabWidget(internalWidget);
	layout->addWidget(tabWidget);

	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");

	QWidget *memoryPage = new QWidget();
	memoryPage->setLayout(new QHBoxLayout(memoryPage));
	memoryView = new QListView(memoryPage);
	memoryView->setUniformItemSizes(true);
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
	registerView->setUniformItemSizes(true);
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
	stackView->setUniformItemSizes(true);
	stackPage->layout()->addWidget(stackView);
	stackModel = new QStringListModel(stackPage);
	stackView->setModel(stackModel);
	stackView->setFont(monoSpacedFont);
	stackView->setMovement(QListView::Static);
	stackView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tabWidget->addTab(stackPage, "Stack");
	
	QWidget *callstackPage = new QWidget();
	callstackPage->setLayout(new QHBoxLayout(callstackPage));
	callstackView = new QListView(callstackPage);
	callstackView->setUniformItemSizes(true);
	callstackPage->layout()->addWidget(callstackView);
	callstackModel = new QStringListModel(callstackPage);
	callstackView->setModel(callstackModel);
	callstackView->setFont(monoSpacedFont);
	callstackView->setMovement(QListView::Static);
	callstackView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(callstackView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(callstackClicked(const QModelIndex&)));

	tabWidget->addTab(callstackPage, "Callstack");

	internalWidget->setLayout(layout);

	for (int i = 0; i < MM_MAX_ELEM; i++)
	{
		memoryDirty[i] = false;
	}

	setWidget(internalWidget);
	internalWidget->setMinimumWidth((parent->size().width() - 20) / 2);
}

void MemoryWidget::update(uint16_t address, uint16_t value)
{
	std::lock_guard<std::mutex> lock(mtxMemoryLock);

	pendingMemoryUpdates[address] = value;
}

static char ToAscii(uint16_t value)
{
	if (value <= 127 && value != '\t' && value != '\n')
	{
		if (isprint(value))
		{
			return (char)value;
		}
	}
	return '.';
}

static const int VALUES_PER_LINE = 16;
static QString GetMemoryRowAt(const std::vector<uint16_t> &buffer, int address)
{
	QString addressStr = QString("%1").arg(address, 4, 16, QChar('0'));
	QString hexString;
	QString asciiString;
	for (int offset = 0; offset < VALUES_PER_LINE && address + offset < buffer.size(); offset++)
	{
		hexString += QString("%1").arg(buffer[address + offset], 4, 16, QChar('0'));
		if (offset < VALUES_PER_LINE - 1)
		{
			hexString += ' ';
		}

		asciiString += ToAscii(buffer[address + offset]);
	}
	if (address + VALUES_PER_LINE > buffer.size())
	{
		for (int offset = (int)buffer.size() - address; offset < VALUES_PER_LINE - 1; offset++)
		{
			hexString += "     ";
		}
		hexString += "    ";
	}
	return QString("%1  |  %2  |  %3").arg(addressStr, hexString, asciiString);
}

void MemoryWidget::load(const std::vector<uint16_t> &buffer)
{
	std::lock_guard<std::mutex> lock(mtxMemoryLock);

	memory.clear();
	for (int address = 0; address < buffer.size(); address += VALUES_PER_LINE)
	{
		memory << GetMemoryRowAt(buffer, address);
	}
	rawMemory = buffer;
	refreshView(MM_MEMORY);

	registers.clear();
	for (int i = 0; i < c_dwNumRegisters; i++)
	{
		registers << QString("r%1:\t%2").arg(i, 2, 16, QChar('0')).arg(0, 4, 16, QChar('0'));
	}
	refreshView(MM_REGISTERS);

	rawStack.clear();
	stack.clear();
	callstack.clear();
	refreshView(MM_STACK);
}

void MemoryWidget::reset()
{
	memory = QStringList(startMemoryBU);
	rawMemory.clear();
	refreshView(MM_MEMORY);

	registers.clear();
	for (int i = 0; i < c_dwNumRegisters; i++)
	{
		registers << QString("r%1:\t%2").arg(i, 2, 16, QChar('0')).arg(0, 4, 16, QChar('0'));
	}
	refreshView(MM_REGISTERS);

	rawStack.clear();
	stack.clear();
	callstack.clear();
	refreshView(MM_STACK);
}

void MemoryWidget::updatePointer(uint16_t address)
{
	inst = address;

	//Time to update our memory
	update();
}

void MemoryWidget::update()
{
	std::lock_guard<std::mutex> lock(mtxMemoryLock);

	std::unordered_set<int> modifiedRows;
	modifiedRows.reserve(pendingMemoryUpdates.size());
	for (auto itr = pendingMemoryUpdates.begin(); itr != pendingMemoryUpdates.end(); itr++)
	{
		uint16_t address = itr->first;
		uint16_t value = itr->second;
		if (address <= 32767)
		{
			if (modifiedRows.find(address / VALUES_PER_LINE) == modifiedRows.end())
			{
				modifiedRows.insert(address / VALUES_PER_LINE);
			}
			rawMemory[address] = value;
			flagDirty(MM_MEMORY);
		}
		else if (address <= 32775)
		{
			registers[address - 32768] = QString("r%1:\t%2").arg(address - 32768, 2, 16, QChar('0')).arg(value, 4, 16, QChar('0'));
			flagDirty(MM_REGISTERS);
		}
		else
		{
			assert(0 && "Invalid address");
		}
	}
	pendingMemoryUpdates.clear();
	for (int row : modifiedRows)
	{
		memory[row] = GetMemoryRowAt(rawMemory, row * VALUES_PER_LINE);
	}

	for (int i = 0; i < MM_MAX_ELEM; i++)
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
}

void MemoryWidget::updateRegister(uint16_t reg, uint16_t value)
{
	update(reg + 32768, value);
}

void MemoryWidget::pushStack(uint16_t value, StackSource source)
{
	rawStack.push_back(std::make_pair(value, source));
	flagDirty(MM_STACK);
}

void MemoryWidget::popStack()
{
	rawStack.pop_back();
	flagDirty(MM_STACK);
}

void MemoryWidget::setCallAddress(uint16_t address)
{
	callAddr = address;
}

void MemoryWidget::refreshView(MemoryModule module)
{
	switch (module)
	{
		case MM_MEMORY:
		{
			memoryModel->setStringList(memory);
			emit refreshAssembly();
			break;
		}
		case MM_STACK:
		{
			stack.clear();
			callstack.clear();

			// The most recently called address is going to be the top of the callstack,
			// since that is where we are currently executing from.
			callstack.push_back(QString("%1").arg(callAddr, 4, 16, QChar('0')));

			for (auto itr = rawStack.rbegin(); itr != rawStack.rend(); itr++)
			{
				QString stackStr = QString("%1").arg(itr->first, 4, 16, QChar('0'));
				switch (itr->second)
				{
					case SS_PUSH_R0:
					case SS_PUSH_R1:
					case SS_PUSH_R2:
					case SS_PUSH_R3:
					case SS_PUSH_R4:
					case SS_PUSH_R5:
					case SS_PUSH_R6:
					case SS_PUSH_R7:
					{
						int reg = itr->second - SS_PUSH_R0;
						stackStr += QString("  |  PUSH r%1").arg(reg, 2, 16, QChar('0'));
						break;
					}
					case SS_CALL:
					{
						callstack.push_back(stackStr);
						stackStr += "  |  CALL";
						break;
					}
				}
				stack.push_back(stackStr);
			}
			stackModel->setStringList(stack);
			callstackModel->setStringList(callstack);
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

void MemoryWidget::callstackClicked(const QModelIndex &index)
{
	if (index.row() >= callstack.size())
	{
		return;
	}

	emit scrollToInstruction(callstack[index.row()].toInt(nullptr, 16));
}
