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
	VMS_BREAK,
	VMS_STEP_INTO,
	VMS_STEP_OVER,
	VMS_MAX
};

//Used so the Debugger can know what to call it's run button
enum DebuggerState
{
	DS_NOT_RUN = 0,
	DS_RUNNING,
	DS_PAUSED,
	DS_HALTED,
	DS_MAX
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
	SynacorVM(QObject *parent);
	void load(const std::vector<uint16_t> &buffer);

	void getAssembly (QStringList &instr, QStringList &args, std::vector<uint16_t> &instrNum);

	void updateForever();

protected:
	QString StringTranslate(uint16_t value);
	QString StringTranslateChar(uint16_t value);
	uint16_t Translate(uint16_t value);
	void Write(uint16_t address, uint16_t value, bool emitUpdate = true);

	uint16_t handleOp(const uint16_t opAddress);

	std::vector<uint16_t> startMemoryBU;
	std::vector<uint16_t> memory;
	std::vector<uint16_t> registers;
	std::vector<uint16_t> stack;

	std::vector<bool> breakpoints;

	uint16_t inst;

	bool loaded;
	bool quitting;
	bool started;

	QString bufferedInput;

	VMState state;

	int numReturnsUntilStepOverEnds;
	bool ignoreNextBreakpoint;

public slots:
	//System Slots
	void reset();
	void pause(bool pause);
	void updateExec();
	void updateInput(const QString &input);
	void stepInto();
	void stepOver();
	void aboutToQuit();

	//Memory Slots
	void changeMemory(uint16_t address, uint16_t value);
	void changeRegister(uint16_t reg, uint16_t value);
	void changeStackPush(uint16_t value);
	void changeStackPop();
	void changeStackModify(uint16_t index, uint16_t value);

	//Assembly Slots
	void setBreakpoint(uint16_t address, bool set);

signals:
	//Output Signals
	void print(const QString &output);
	void clear();
	void awaitingInput();

	//UI Signals
	void throwError(VMErrors error);
	void newDebuggerState(DebuggerState dState);

	//Memory Signals
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value);
	void popStack();
	void pushCallstack(uint16_t value);
	void popCallstack();


	//Assembly Signals
	void updatePointer(uint16_t address);
};

#endif