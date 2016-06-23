#ifndef MEMORY_WIDGET_H_
#define MEMORY_WIDGET_H_

QT_BEGIN_NAMESPACE
class QListView;
class QStringListModel;
QT_END_NAMESPACE

enum MemoryModule
{
	MM_MEMORY = 0,
	MM_REGISTERS,
	MM_STACK,
	MM_MAX_ELEM
};

enum StackSource;

class MemoryWidget : public QDockWidget
{
	Q_OBJECT
public:
	MemoryWidget(QWidget *parent);

	void load(const std::vector<uint16_t> &buffer);

protected:
	void update(uint16_t address, uint16_t value);
	void refreshView(MemoryModule module);
	void flagDirty(MemoryModule module);

	bool memoryDirty[MM_MAX_ELEM];

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

	std::unordered_map<uint16_t, uint16_t> pendingMemoryUpdates;
	std::vector<std::pair<uint16_t, StackSource>> rawStack;
	std::vector<uint16_t> rawMemory;

	uint16_t inst;
	uint16_t callAddr;

public slots:
	//VM Slots
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value, StackSource source);
	void popStack();
	void clearStack();
	void setCallAddress(uint16_t address);
	void updatePointer(uint16_t address);

	//System Slots
	void reset();
	void update();
	void callstackClicked(const QModelIndex &index);
	void editedRegisterValue(uint16_t reg, uint16_t value);
	void editedMemoryValue(uint16_t addr, uint16_t value);

signals:
	//VM Signals
	void changeMemory(uint16_t address, uint16_t value);
	void changeRegister(uint16_t reg, uint16_t value);
	void changeStackPush(uint16_t value, StackSource source);
	void changeStackPop();
	void changeStackModify(uint16_t index, uint16_t value);

	// System Signals
	void scrollToInstruction(uint16_t address);
	void refreshAssembly();
};

#endif // MEMORY_WIDGET_H_
