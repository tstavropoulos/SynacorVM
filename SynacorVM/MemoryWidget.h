#ifndef MEMORY_WIDGET_H_
#define MEMORY_WIDGET_H_

#include <QWidget>


QT_BEGIN_NAMESPACE
class QListView;
class QStringListModel;
QT_END_NAMESPACE

enum MemoryModule
{
	MM_MEMORY = 0,
	MM_REGISTERS,
	MM_STACK,
	MM_CALLSTACK,
	MM_MAX
};

class MemoryWidget : public QWidget
{
	Q_OBJECT
public:
	MemoryWidget(QWidget *parent);

	void load(const std::vector<uint16_t> &buffer);

protected:
	void update(uint16_t address, uint16_t value);
	void refreshView(MemoryModule module);
	void flagDirty(MemoryModule module);

	bool memoryDirty[MM_MAX];

	QListView *memoryView;
	QListView *registerView;
	QListView *stackView;
	QListView *callstackView;

	QStringListModel *memoryModel;
	QStringListModel *registerModel;
	QStringListModel *stackModel;
	QStringListModel *callstackModel;

	QStringList startMemoryBU;
	QStringList memory;
	QStringList registers;
	QStringList stack;
	QStringList callstack;

	std::map<uint16_t, uint16_t> pendingMemoryUpdates;
	std::vector<uint16_t> rawStack;
	std::vector<uint16_t> rawCallstack;

	uint16_t inst;

public slots:
	//VM Slots
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value);
	void popStack();
	void pushCallstack(uint16_t value);
	void popCallstack();
	void updatePointer(uint16_t address);

	//System Slots
	void reset();
	void update();
	void callstackClicked(const QModelIndex &index);

signals:
	//VM Signals
	void changeMemory(uint16_t address, uint16_t value);
	void changeRegister(uint16_t reg, uint16_t value);
	void changeStackPush(uint16_t value);
	void changeStackPop();
	void changeStackModify(uint16_t index, uint16_t value);

	// System Signals
	void scrollToInstruction(uint16_t address);
};

#endif // MEMORY_WIDGET_H_
