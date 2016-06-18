#ifndef ASSEMBLY_WIDGET_H_
#define ASSEMBLY_WIDGET_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
class QStringListModel;
QT_END_NAMESPACE

class AssemblyWidget : public QWidget
{
	Q_OBJECT
public:
	AssemblyWidget(QWidget *parent);
	void setAssembly(const QStringList &instructions, const QStringList &arguments, const std::vector<uint16_t> &instrAddress);

protected:
	void rebuild();

	QListView *listView;
	QStringListModel *listModel;

	QStringList instr;
	QStringList args;

	//The instruction order of the whole data set
	std::vector<uint16_t> instrAddress;

	//The string list currently rendered in the model
	QStringList currentAssembly;

	std::vector<bool> breakpoints;

	bool loaded;
	uint16_t currentExecAddress;

public slots:
	void operationBreakToggled(const QModelIndex &index);
	void updatePointer(uint16_t address);
	void scrollToInstruction(uint16_t address);

signals:
	void setBreakpoint(uint16_t instr, bool set);
};

#endif // ASSEMBLY_WIDGET_H_
