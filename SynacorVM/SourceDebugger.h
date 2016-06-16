#ifndef SOURCE_DEBUGGER_H_
#define SOURCE_DEBUGGER_H_

#include <qwidget.h>
#include <vector>

QT_BEGIN_NAMESPACE
class QListView;
class QFileDialog;
QT_END_NAMESPACE

class OutputWidget;
class MemoryWidget;
class AssemblyWidget;
class SynacorVM;


class SourceDebugger : public QWidget
{
	Q_OBJECT
public:
	SourceDebugger(QWidget *parent);

public slots:
	void load();
	void reduce();
	void run();

protected:
	OutputWidget *outputWidget;
	MemoryWidget *memoryWidget;
	AssemblyWidget *assemblyWidget;

	SynacorVM *synacorVM;

	QFileDialog *fileDialog;

	//std::vector<uint16_t> memory;

};

#endif // SOURCE_DEBUGGER_H_
