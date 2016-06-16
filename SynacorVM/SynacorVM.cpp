// SynacorVM.cpp : Defines the entry point for the console application.
//
#include "SynacorVM.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <assert.h>

#define VERBOSE_PRINTS 0

SynacorVM::SynacorVM()
	: registers(c_dwNumRegisters)//, memory(c_dwAddressSpace)
{

}

QString SynacorVM::StringTranslateChar(uint16_t value)
{
	QString returnVal = QString("%1 ").arg(0xFFFF);
	if (value <= 32767)
	{
		returnVal = (wchar_t)value;
	}
	else if (value <= 32775)
	{
		returnVal = QString("r%1 ").arg(value - 32768);
	}

	return returnVal;
}

QString SynacorVM::StringTranslate(uint16_t value)
{
	QString returnVal = QString("%1 ").arg(0xFFFF);
	if (value <= 32767)
	{
		returnVal = QString("%1 ").arg(value);
	}
	else if (value <= 32775)
	{
		returnVal = QString("r%1 ").arg(value - 32768);
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

void SynacorVM::Write(uint16_t address, uint16_t value)
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
}

void SynacorVM::load(const uint16_t* buffer)
{
	memory.insert(memory.end(), &buffer[0], &buffer[c_dwAddressSpace]);
}

void SynacorVM::run()
{
	uint16_t inst = 0;
	bool halted = false;
	while (!halted)
	{
		const uint16_t op = memory[inst++];

		switch (op)
		{
			// halt: 0
			// stop execution and terminate the program
			case 0:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "HALT" << std::endl;
#endif
				halted = true;
				break;
			}

			//set: 1 a b
			//set register <a> to the value of <b>
			case 1:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "SET " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				assert(a >= 32768 && a < 32768 + c_dwNumRegisters);
				registers[a - 32768] = b;
				break;
			}

			//push : 2 a
			//push <a> onto the stack
			case 2:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "PUSH " << memory[inst] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				stack.push_back(a);
				break;
			}

			//pop : 3 a
			//remove the top element from the stack and write it into <a>; empty stack = error
			case 3:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "POP " << memory[inst] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				assert(!stack.empty());
				const uint16_t top = stack.back();
				stack.pop_back();
				Write(a, top);
				break;
			}

			//eq : 4 a b c
			//set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
			case 4:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "EQ " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				const uint16_t c = Translate(memory[inst++]);
				Write(a, b == c ? 1 : 0);
				break;
			}

			//gt : 5 a b c
			//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
			case 5:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "GT " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				const uint16_t c = Translate(memory[inst++]);
				Write(a, b > c ? 1 : 0);
				break;
			}

			//jmp : 6 a
			//jump to <a>
			case 6:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "JMP " << memory[inst] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				inst = a;
				break;
			}

			//jt : 7 a b
			//if <a> is nonzero, jump to <b>
			case 7:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "JT " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				const uint16_t b = Translate(memory[inst++]);
				if (a != 0)
				{
					inst = b;
				}
				break;
			}

			//jf : 8 a b
			//if <a> is zero, jump to <b>
			case 8:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "JF " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				const uint16_t b = Translate(memory[inst++]);
				if (a == 0)
				{
					inst = b;
				}
				break;
			}

			//add : 9 a b c
			//assign into <a> the sum of <b> and <c> (modulo 32768)
			case 9:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "ADD " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint32_t b = Translate(memory[inst++]);
				const uint32_t c = Translate(memory[inst++]);
				Write(a, (uint16_t)((b + c) % 32768));
				break;
			}

			//mult : 10 a b c
			//store into <a> the product of <b> and <c> (modulo 32768)
			case 10:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "MULT " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint64_t b = Translate(memory[inst++]);
				const uint64_t c = Translate(memory[inst++]);
				Write(a, (uint16_t)((b * c) % 32768));
				break;
			}

			//mod : 11 a b c
			//store into <a> the remainder of <b> divided by <c>
			case 11:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "MOD " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				const uint16_t c = Translate(memory[inst++]);
				Write(a, b % c);
				break;
			}

			//and : 12 a b c
			//stores into <a> the bitwise and of <b> and <c>
			case 12:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "AND " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				const uint16_t c = Translate(memory[inst++]);
				Write(a, b & c);
				break;
			}

			//or : 13 a b c
			//stores into <a> the bitwise or of <b> and <c>
			case 13:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "OR " << memory[inst] << " " << memory[inst + 1] << " " << memory[inst + 2] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				const uint16_t c = Translate(memory[inst++]);
				Write(a, b | c);
				break;
			}

			//not: 14 a b
			//stores 15 - bit bitwise inverse of <b> in <a>
			case 14:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "NOT " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint32_t b = Translate(memory[inst++]);
				Write(a, (uint16_t)(~b & 0x7FFF));
				break;
			}

			//rmem : 15 a b
			//read memory at address <b> and write it to <a>
			case 15:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "RMEM " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t b = Translate(memory[inst++]);
				Write(a, memory[b]);
				break;
			}

			//wmem : 16 a b
			//write the value from <b> into memory at address <a>
			case 16:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "WMEM " << memory[inst] << " " << memory[inst + 1] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				const uint16_t b = Translate(memory[inst++]);
				memory[a] = b;
				break;
			}

			//call : 17 a
			//write the address of the next instruction to the stack and jump to <a>
			case 17:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "CALL " << memory[inst] << std::endl;
#endif
				const uint16_t a = Translate(memory[inst++]);
				stack.push_back(inst);
				inst = a;
				break;
			}

			//ret : 18
			//remove the top element from the stack and jump to it; empty stack = halt
			case 18:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "RET" << std::endl;
#endif
				if (stack.empty())
				{
					halted = true;
				}
				else
				{
					const uint16_t top = stack.back();
					stack.pop_back();
					inst = top;
					break;
				}
			}

			//out : 19 a
			//write the character represented by ascii code <a> to the terminal
			case 19:
			{
				const uint16_t a = Translate(memory[inst++]);
				emit print(QString((char)a));
				//std::cout << (char)a;
				break;
			}
			//in : 20 a
			//read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts, it will continue until a newline is encountered; this means that you can safely read whole lines from the keyboard and trust that they will be fully read
			case 20:
			{
#if VERBOSE_PRINTS
				std::cout << "[" << inst - 1 << "]" << "IN " << memory[inst] << std::endl;
#endif
				const uint16_t a = memory[inst++];
				const uint16_t input = (uint16_t)std::cin.get();
				Write(a, input);
				break;
			}

			//noop : 21
			//no operation
			case 21:
			{
				break;
			}
		}
	}
}

void SynacorVM::getAssembly(QStringList &instr, QStringList &args)
{
	for (uint16_t i = 0; i < c_dwAddressSpace;)
	{
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
				instructions = "SET";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
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
				instructions = "POP";
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
				instructions = "EQ";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//gt : 5 a b c
			//set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
			case 5:
			{
				instructions = "GT";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//jmp : 6 a
			//jump to <a>
			case 6:
			{
				instructions = "JMP";
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
				instructions = "JT";
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
				instructions = "JF";
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
				instructions = "ADD";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
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
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//mod : 11 a b c
			//store into <a> the remainder of <b> divided by <c>
			case 11:
			{
				instructions = "MOD";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//and : 12 a b c
			//stores into <a> the bitwise and of <b> and <c>
			case 12:
			{
				instructions = "AND";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//or : 13 a b c
			//stores into <a> the bitwise or of <b> and <c>
			case 13:
			{
				instructions = "OR";
				if (i + 3 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
					arguments += StringTranslate(memory[i++]);
					arguments += StringTranslate(memory[i++]);
				}
				break;
			}

			//not: 14 a b
			//stores 15 - bit bitwise inverse of <b> in <a>
			case 14:
			{
				instructions = "NOT";
				if (i + 2 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
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
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
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
				instructions = "RET";
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
				instructions = "IN";
				if (i + 1 < c_dwAddressSpace)
				{
					arguments += QString("r%1 ").arg(memory[i++] - 32768);
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

