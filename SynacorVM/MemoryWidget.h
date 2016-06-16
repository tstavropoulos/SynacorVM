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

	QStringListModel *memoryModel;
	QStringListModel *registerModel;
	QStringListModel *stackModel;

	QStringList startMemoryBU;
	QStringList memory;
	QStringList registers;
	QStringList stack;

public slots:
	//VM Slots
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value);
	void popStack();

	//System Slots
	void reset();
	void update();

signals:
	//VM Signals
	void changeMemory(uint16_t address, uint16_t value);
	void changeRegister(uint16_t reg, uint16_t value);
	void changeStackPush(uint16_t value);
	void changeStackPop();
	void changeStackModify(uint16_t index, uint16_t value);
};

#endif // MEMORY_WIDGET_H_
