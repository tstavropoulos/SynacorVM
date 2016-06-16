#ifndef SYNACOR_VM_H_
#define SYNACOR_VM_H_

#include <QObject>
#include <vector>

static const size_t c_dwAddressSpace = 1 << 15;
static const size_t c_dwNumRegisters = 8;

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

	std::vector<uint16_t> memory;
	std::vector<uint16_t> registers;
	std::vector<uint16_t> stack;


signals:
	void print(const QString &output);
	void clear();
};

#endif