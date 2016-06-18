#include "stdafx.h"

#include "SynacorVM.h"

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <assert.h>

#include <QCoreApplication>
#include <QThread>

#define VERBOSE_PRINTS 0

SynacorVM::SynacorVM(QObject *parent)
	: QObject(parent)
	, registers(c_dwNumRegisters)
	, breakpoints(c_dwAddressSpace)
	, inst(0)
	, state(VMS_HALTED)
	, loaded(false)
	, numReturnsUntilStepOverEnds(0)
	, ignoreNextBreakpoint(false)
	, stepOutCallDepth(0)
	, quitting(false)
	, started(false)
{

}

QString SynacorVM::StringTranslateChar(uint16_t value)
{
	QString returnVal = QString("%1 ").arg(0xFFFF, 4, 16, QChar('0'));
	if (value <= 32767)
	{
		returnVal = (wchar_t)value;
	}
	else if (value <= 32775)
	{
		returnVal = QString("r%1  ").arg(value - 32768, 2, 16, QChar('0'));
	}

	return returnVal;
}

QString SynacorVM::StringTranslate(uint16_t value)
{
	QString returnVal = QString("%1 ").arg(QString::number(0xFFFF, 16).toUpper().rightJustified(4, '0'));
	if (value <= 32767)
	{
		returnVal = QString("%1 ").arg(value, 4, 16, QChar('0'));
	}
	else if (value <= 32775)
	{
		returnVal = QString("r%1  ").arg(value - 32768, 2, 16, QChar('0'));
	}

	return returnVal;
}

uint16_t SynacorVM::Translate(uint16_t value)
{
	if (value <= 32767)
	{
		return value;
	}
	if (value <= 32775)
	{
		return registers[value - 32768];
	}
	assert(0 && "Invalid value");
	return 0xFFFF;
}

void SynacorVM::Write(uint16_t address, uint16_t value, bool emitUpdate)
{
	if (address <= 32767)
	{
		memory[address] = value;
	}
	else if (address <= 32775)
	{
		registers[address - 32768] = value;
	}
	else
	{
		assert(0 && "Invalid address");
	}

	if (emitUpdate)
	{
		emit updateMemory(address, value);
	}
}

void SynacorVM::reset()
{
	if (!loaded)
	{
		emit throwError(VME_RESET_NO_FILE_LOADED);
		return;
	}

	inst = 0;
	state = VMS_BREAK;
	emit newDebuggerState(DS_NOT_RUN);
	emit updatePointer(inst);
	memory = startMemoryBU;
	registers = std::vector<uint16_t>(c_dwNumRegisters);
	bufferedInput = QString();
	started = false;
}

void SynacorVM::activateVM()
{
	updateForever();
}

void SynacorVM::updateForever()
{
	int execCounter = 0;

	forever
	{
		if (quitting)
		{
			break;
		}
		updateExec();
		if (++execCounter > 10)
		{
			execCounter = 0;
			QCoreApplication::processEvents();
		}
	}
}

void SynacorVM::aboutToQuit()
{
	quitting = true;
}

void SynacorVM::pause(bool pause)
{
	switch (state)
	{
	case VMS_RUNNING:
	{
		emit updatePointer(inst);

		//keep goin'
	}
	case VMS_BREAK:
	{
		//Update started if it's false and we're unpausing
		started |= !pause;

		state = pause ? VMS_BREAK : VMS_RUNNING;
		emit newDebuggerState(pause ? DS_PAUSED : DS_RUNNING);
		if (!pause && breakpoints[inst])
		{
			ignoreNextBreakpoint = true;
		}
		break;
	}
	case VMS_AWAITING_INPUT:
	case VMS_HALTED:
	{
		break;
	}
	default:
	{
		assert(0 && "State unsupported.");
		break;
	}
	}
}

void SynacorVM::stepInto()
{
	if (state != VMS_BREAK)
	{
		return;
	}
	state = VMS_STEP_INTO;
	ignoreNextBreakpoint = breakpoints[inst];
}

void SynacorVM::stepOver()
{
	if (state != VMS_BREAK)
	{
		return;
	}

	// 17 == CALL
	if (memory[inst] != 17)
	{
		state = VMS_STEP_INTO;
	}
	else
	{
		state = VMS_STEP_OVER;
		numReturnsUntilStepOverEnds = 1;
	}
	ignoreNextBreakpoint = breakpoints[inst];
}

void SynacorVM::stepOut()
{
	if (state != VMS_BREAK)
	{
		return;
	}

	state = VMS_STEP_OUT;
	stepOutCallDepth = 1;
	ignoreNextBreakpoint = breakpoints[inst];
}

void SynacorVM::load(const std::vector<uint16_t> &buffer)
{
	memory = std::vector<uint16_t>(buffer);
	startMemoryBU = std::vector<uint16_t>(buffer);

	loaded = true;
	reset();
}

static bool IsRunningState(VMState state)
{
	return state == VMS_RUNNING || state == VMS_STEP_INTO || state == VMS_STEP_OVER || state == VMS_STEP_OUT;
}

void SynacorVM::updateExec()
{
	switch (state)
	{
	case VMS_BREAK:
	case VMS_AWAITING_INPUT:
	case VMS_HALTED:
	{
		break;
	}
	case VMS_RUNNING:
	case VMS_STEP_INTO:
	case VMS_STEP_OVER:
	case VMS_STEP_OUT:
	{
		if (breakpoints[inst] && !ignoreNextBreakpoint)
		{
			emit updatePointer(inst);
			emit newDebuggerState(DS_PAUSED);
			state = VMS_BREAK;
			break;
		}
		ignoreNextBreakpoint = false;

		inst = handleOp(inst);

		if (state == VMS_STEP_INTO)
		{
			emit updatePointer(inst);
			state = VMS_BREAK;
		}

		break;
	}
	default:
	{
		assert(0 && "State unsupported.");
		break;
	}
	}
}

// Executes the operation at the indicated address and returns the new address pointer
uint16_t SynacorVM::handleOp(const uint16_t opAddress)
{
	uint16_t tempInst = opAddress;
	const uint16_t op = memory[tempInst++];

	switch (op)
	{
		// halt: 0
		// stop execution and terminate the program
	case 0:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "HALT" << std::endl;
#endif
		state = VMS_HALTED;
		emit newDebuggerState(DS_HALTED);
		emit updatePointer(opAddress);
		break;
	}

	//set: 1 a b
	//set register <a> to the value of <b>
	case 1:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "SET " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		assert(a >= 32768 && a < 32768 + c_dwNumRegisters);
		registers[a - 32768] = b;

		emit updateRegister(a - 32768, b);
		break;
	}

	//push : 2 a
	//push <a> onto the stack
	case 2:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "PUSH " << memory[tempInst] << std::endl;
#endif
		int reg = memory[tempInst++];
		assert(reg >= 32768 && reg < 32768 + c_dwNumRegisters);
		const uint16_t a = Translate(reg);
		stack.push_back(a);

		emit pushStack(a, (StackSource)(SS_PUSH_R0 + (reg - 32768)));
		break;
	}

	//pop : 3 a
	//remove the top element from the stack and write it into <a>; empty stack = error
	case 3:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "POP " << memory[tempInst] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		assert(!stack.empty());
		const uint16_t top = stack.back();

		stack.pop_back();
		emit popStack();

		Write(a, top);
		break;
	}

	//eq : 4 a b c
	//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
	case 4:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "EQ " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		const uint16_t c = Translate(memory[tempInst++]);
		Write(a, b == c ? 1 : 0);
		break;
	}

	//gt : 5 a b c
	//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
	case 5:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "GT " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		const uint16_t c = Translate(memory[tempInst++]);
		Write(a, b > c ? 1 : 0);
		break;
	}

	//jmp : 6 a
	//jump to <a>
	case 6:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "JMP " << memory[tempInst] << std::endl;
#endif
		const uint16_t a = Translate(memory[tempInst++]);
		tempInst = a;
		break;
	}

	//jt : 7 a b
	//if <a> is nonzero, jump to <b>
	case 7:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "JT " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = Translate(memory[tempInst++]);
		const uint16_t b = Translate(memory[tempInst++]);
		if (a != 0)
		{
			tempInst = b;
		}
		break;
	}

	//jf : 8 a b
	//if <a> is zero, jump to <b>
	case 8:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "JF " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = Translate(memory[tempInst++]);
		const uint16_t b = Translate(memory[tempInst++]);
		if (a == 0)
		{
			tempInst = b;
		}
		break;
	}

	//add : 9 a b c
	//assign into <a> the sum of <b> and <c> (modulo 32768)
	case 9:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "ADD " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint32_t b = Translate(memory[tempInst++]);
		const uint32_t c = Translate(memory[tempInst++]);
		Write(a, (uint16_t)((b + c) % 32768));
		break;
	}

	//mult : 10 a b c
	//store into <a> the product of <b> and <c> (modulo 32768)
	case 10:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "MULT " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint64_t b = Translate(memory[tempInst++]);
		const uint64_t c = Translate(memory[tempInst++]);
		Write(a, (uint16_t)((b * c) % 32768));
		break;
	}

	//mod : 11 a b c
	//store into <a> the remainder of <b> divided by <c>
	case 11:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "MOD " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		const uint16_t c = Translate(memory[tempInst++]);
		Write(a, b % c);
		break;
	}

	//and : 12 a b c
	//stores into <a> the bitwise and of <b> and <c>
	case 12:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "AND " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		const uint16_t c = Translate(memory[tempInst++]);
		Write(a, b & c);
		break;
	}

	//or : 13 a b c
	//stores into <a> the bitwise or of <b> and <c>
	case 13:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "OR " << memory[tempInst] << " " << memory[tempInst + 1] << " " << memory[tempInst + 2] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		const uint16_t c = Translate(memory[tempInst++]);
		Write(a, b | c);
		break;
	}

	//not: 14 a b
	//stores 15 - bit bitwise inverse of <b> in <a>
	case 14:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "NOT " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint32_t b = Translate(memory[tempInst++]);
		Write(a, (uint16_t)(~b & 0x7FFF));
		break;
	}

	//rmem : 15 a b
	//read memory at address <b> and write it to <a>
	case 15:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "RMEM " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = memory[tempInst++];
		const uint16_t b = Translate(memory[tempInst++]);
		Write(a, memory[b]);
		break;
	}

	//wmem : 16 a b
	//write the value from <b> into memory at address <a>
	case 16:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "WMEM " << memory[tempInst] << " " << memory[tempInst + 1] << std::endl;
#endif
		const uint16_t a = Translate(memory[tempInst++]);
		const uint16_t b = Translate(memory[tempInst++]);
		memory[a] = b;
		emit updateMemory(a, b);
		break;
	}

	//call : 17 a
	//write the address of the next tempInstruction to the stack and jump to <a>
	case 17:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "CALL " << memory[tempInst] << std::endl;
#endif
		const uint16_t a = Translate(memory[tempInst++]);
		stack.push_back(tempInst);
		emit pushStack(tempInst, SS_CALL);
		tempInst = a;

		if (state == VMS_STEP_OVER)
		{
			numReturnsUntilStepOverEnds++;
		}
		else if (state == VMS_STEP_OUT)
		{
			stepOutCallDepth++;
		}
		break;
	}

	//ret : 18
	//remove the top element from the stack and jump to it; empty stack = halt
	case 18:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "RET" << std::endl;
#endif
		if (stack.empty())
		{
			emit updatePointer(opAddress);
			state = VMS_HALTED;
			emit newDebuggerState(DS_HALTED);
		}
		else
		{
			const uint16_t top = stack.back();
			stack.pop_back();
			emit popStack();
			tempInst = top;

			if (state == VMS_STEP_OVER)
			{
				if (--numReturnsUntilStepOverEnds == 0)
				{
					state = VMS_BREAK;
					emit updatePointer(tempInst);
				}
			}
			else if (state == VMS_STEP_OUT)
			{
				if (--stepOutCallDepth == 0)
				{
					state = VMS_BREAK;
					emit updatePointer(tempInst);
				}
			}
			break;
		}
	}

	//out : 19 a
	//write the character represented by ascii code <a> to the terminal
	case 19:
	{
		const uint16_t a = Translate(memory[tempInst++]);
		emit print(QString((char)a));
		//std::cout << (char)a;
		break;
	}
	//in : 20 a
	//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts, it will continue until a newline is encountered; this means that you can safely read whole lines from the keyboard and trust that they will be fully read
	case 20:
	{
#if VERBOSE_PRINTS
		std::cout << "[" << tempInst - 1 << "]" << "IN " << memory[tempInst] << std::endl;
#endif
		if (bufferedInput.length() > 0)
		{
			const uint16_t a = memory[tempInst++];
			const int16_t input = bufferedInput[0].toLatin1();
			bufferedInput.remove(0, 1);
			Write(a, input);
			emit print(QString((char)input));
		}
		else
		{
			emit updatePointer(opAddress);
			state = VMS_AWAITING_INPUT;

			//Move back to the operation that caused us to wait for input
			tempInst--;

			emit awaitingInput();
		}
		//const uint16_t input = (uint16_t)std::cin.get();
		//Write(a, input);
		break;
	}

	//noop : 21
	//no operation
	case 21:
	{
		break;
	}
	}
	
	return tempInst;
}

void SynacorVM::updateInput(const QString &input)
{
	if (input.length() == 0)
	{
		assert(0 && "Empty String submitted");
		return;
	}

	bufferedInput += input;
	if (state == VMS_AWAITING_INPUT)
	{
		state = VMS_RUNNING;
		emit newDebuggerState(DS_RUNNING);
	}
}


void SynacorVM::changeMemory(uint16_t address, uint16_t value)
{
	Write(address, value, false);
}

void SynacorVM::changeRegister(uint16_t reg, uint16_t value)
{
	Write(reg + 32768, value, false);
}

void SynacorVM::changeStackPush(uint16_t value)
{
	stack.push_back(value);
}

void SynacorVM::changeStackPop()
{
	stack.pop_back();
}

void SynacorVM::changeStackModify(uint16_t index, uint16_t value)
{
	if (stack.size() > index)
	{
		stack[index] = value;
	}
}

void SynacorVM::setBreakpoint(uint16_t address, bool set)
{
	breakpoints[address] = set;
}


void SynacorVM::getAssembly(QStringList &instr, QStringList &args, std::vector<uint16_t> &instrNum)
{
	if (!loaded)
	{
		//No need to throw an error right now - it's being handled by earlier processes
		assert(0 && "Tried to getAssembly without loading data");
		return;
	}

	for (uint16_t i = 0; i < c_dwAddressSpace;)
	{
		instrNum.push_back(i);
		const uint16_t op = memory[i++];
		QString instructions;
		QString arguments;
		switch (op)
		{
			// halt: 0
			// stop execution and terminate the program
			case 0:
			{
				instructions = "HALT";
				break;
			}

			//set: 1 a b
			//set register <a> to the value of <b>
			case 1:
			{
				instructions = "SET ";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//push : 2 a
			//push <a> onto the stack
			case 2:
			{
				instructions = "PUSH";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//pop : 3 a
			//remove the top element from the stack and write it into <a>; empty stack = error
			case 3:
			{
				instructions = "POP ";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//eq : 4 a b c
			//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
			case 4:
			{
				instructions = "EQ  ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//gt : 5 a b c
			//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
			case 5:
			{
				instructions = "GT  ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//jmp : 6 a
			//jump to <a>
			case 6:
			{
				instructions = "JMP ";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//jt : 7 a b
			//if <a> is nonzero, jump to <b>
			case 7:
			{
				instructions = "JT  ";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//jf : 8 a b
			//if <a> is zero, jump to <b>
			case 8:
			{
				instructions = "JF  ";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//add : 9 a b c
			//assign into <a> the sum of <b> and <c> (modulo 32768)
			case 9:
			{
				instructions = "ADD ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//mult : 10 a b c
			//store into <a> the product of <b> and <c> (modulo 32768)
			case 10:
			{
				instructions = "MULT";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//mod : 11 a b c
			//store into <a> the remainder of <b> divided by <c>
			case 11:
			{
				instructions = "MOD ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//and : 12 a b c
			//stores into <a> the bitwise and of <b> and <c>
			case 12:
			{
				instructions = "AND ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//or : 13 a b c
			//stores into <a> the bitwise or of <b> and <c>
			case 13:
			{
				instructions = "OR  ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//not: 14 a b
			//stores 15 - bit bitwise inverse of <b> in <a>
			case 14:
			{
				instructions = "NOT ";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//rmem : 15 a b
			//read memory at address <b> and write it to <a>
			case 15:
			{
				instructions = "RMEM";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//wmem : 16 a b
			//write the value from <b> into memory at address <a>
			case 16:
			{
				instructions = "WMEM";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//call : 17 a
			//write the address of the next instruction to the stack and jump to <a>
			case 17:
			{
				instructions = "CALL";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//ret : 18
			//remove the top element from the stack and jump to it; empty stack = halt
			case 18:
			{
				instructions = "RET ";
				break;
			}

			//out : 19 a
			//write the character represented by ascii code <a> to the terminal
			case 19:
			{
				instructions = "PRNT";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += StringTranslateChar(memory[i++]);
				}
				break;
			}
			//in : 20 a
			//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts, it will continue until a newline is encountered; this means that you can safely read whole lines from the keyboard and trust that they will be fully read
			case 20:
			{
				instructions = "IN  ";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += QString("r%1  ").arg(memory[i++] - 32768, 2, 16, QChar('0'));
				}
				break;
			}

			//noop : 21
			//no instructions
			case 21:
			{
				instructions = "NOOP";
				break;
			}

			default:
			{
				instructions = "DATA";
				arguments += QString("%1").arg(op, 4, 16, QChar('0'));
				break;
			}
		}

		instr << instructions;
		args << arguments;
	}
}

