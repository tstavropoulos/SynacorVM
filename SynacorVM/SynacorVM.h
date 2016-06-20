#ifndef SYNACOR_VM_H_
#define SYNACOR_VM_H_

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
	VMS_STEP_OUT,
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

enum StackSource
{
	SS_PUSH_R0,
	SS_PUSH_R1,
	SS_PUSH_R2,
	SS_PUSH_R3,
	SS_PUSH_R4,
	SS_PUSH_R5,
	SS_PUSH_R6,
	SS_PUSH_R7,
	SS_CALL
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
	std::vector<uint16_t> stackSource;

	std::vector<bool> breakpoints;
	std::string fullOutput;

	uint16_t inst;
	uint16_t callAddr;

	bool loaded;
	bool quitting;
	bool started;

	QString bufferedInput;

	VMState state;

	int numReturnsUntilStepOverEnds;
	bool ignoreNextBreakpoint;

	int stepOutCallDepth;

public slots:
	//System Slots
	void activateVM();
	void reset();
	void pause(bool pause);
	void updateExec();
	void updateInput(const QString &input);
	void stepInto();
	void stepOver();
	void stepOut();
	void aboutToQuit();

	//Memory Slots
	void changeMemory(uint16_t address, uint16_t value);
	void changeRegister(uint16_t reg, uint16_t value);
	void changeStackPush(uint16_t value, StackSource source);
	void changeStackPop();
	void changeStackModify(uint16_t index, uint16_t value);

	//Assembly Slots
	void setBreakpoint(uint16_t address, bool set);

	// State serialization
	void getSaveState(const QString &path);
	void putLoadState(const QString &path);

signals:
	//Output Signals
	void print(const QString &output);
	void clear();
	void awaitingInput();

	//UI Signals
	void throwError(VMErrors error);
	void newDebuggerState(DebuggerState dState);
	void updateRecentPath(const QString &filepath);

	//Memory Signals
	void updateMemory(uint16_t address, uint16_t value);
	void updateRegister(uint16_t reg, uint16_t value);
	void pushStack(uint16_t value, StackSource source);
	void popStack();
	void clearStack();
	void setCallAddress(uint16_t address);
	void updateBreakpoint(uint16_t address, bool set);

	//Assembly Signals
	void updatePointer(uint16_t address);
};

#endif