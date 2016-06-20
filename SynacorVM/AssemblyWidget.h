#ifndef ASSEMBLY_WIDGET_H_
#define ASSEMBLY_WIDGET_H_

#include <QDockWidget>

QT_BEGIN_NAMESPACE
class QListView;
class QStringListModel;
class RowColorStringListModel;
QT_END_NAMESPACE

class AssemblyWidget : public QDockWidget
{
	Q_OBJECT
public:
	AssemblyWidget(QWidget *parent);
	void setAssembly(const QStringList &instructions, const QStringList &arguments, const std::vector<uint16_t> &instrAddress);

protected:
	void rebuild();

	QListView *listView;
	RowColorStringListModel *listModel;

	QStringList instr;
	QStringList args;

	//The instruction order of the whole data set
	std::vector<uint16_t> instrAddress;

	//The string list currently rendered in the model
	QStringList currentAssembly;

	std::vector<bool> breakpoints;
	std::vector<uint16_t> pendingBreakpointAddresses;

	bool loaded;
	uint16_t currentExecAddress;
	uint16_t pendingUpdatePointerAddress;

	uint16_t recentJumpAddress;

public slots:
	void operationBreakToggled(const QModelIndex &index);
	void updateBreakpoint(uint16_t address, bool set);
	void updatePointer(uint16_t address);
	void updatePointerFromPending();
	void scrollToInstruction(uint16_t address);

signals:
	void setBreakpoint(uint16_t instr, bool set);
};

#endif // ASSEMBLY_WIDGET_H_
