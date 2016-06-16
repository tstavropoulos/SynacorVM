#ifndef SYNACOR_VM_H_
#define SYNACOR_VM_H_

#include <QObject>
#include <vector>

static const size_t c_dwAddressSpace = 1 << 15;
static const size_t c_dwNumRegisters = 8;

enum VMState
{
	VMS_HALTED = 0,
	VMS_AWAITING_INPUT,
	VMS_RUNNING,
	VMS_MAX
};

enum VMErrors
{
	VME_RUN_NO_FILE_LOADED = 0,
	VME_RESET_NO_FILE_LOADED,
	VME_MAX
};


class SynacorVM : public QObject
{
	Q_OBJECT
public:
	SynacorVM();
	void load(const std::vector<uint16_t> &buffer);

	void getAssembly (QStringList &instr, QStringList &args);
	void run();

protected:
	QString StringTranslate(uint16_t value);
	QString StringTranslateChar(uint16_t value);
	uint16_t Translate(uint16_t value);
	void Write(uint16_t address, uint16_t value);

	uint16_t handleOp(const uint16_t opAddress);

	std::vector<uint16_t> startMemoryBU;
	std::vector<uint16_t> memory;
	std::vector<uint16_t> registers;
	std::vector<uint16_t> stack;

	uint16_t inst;

	bool loaded;
	bool paused;

	QString bufferedInput;

	VMState state;

	int executionsPerUpdate;

public slots:
	void reset();
	void pause(bool pause);
	void update();
	void updateInput(const QString &input);

signals:
	//Output Signals
	void print(const QString &output);
	void clear();
	void awaitingInput();

	//UI Signals
	void throwError(VMErrors error);

	//Memory Signals
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value);
	void popStack();
};

#endif