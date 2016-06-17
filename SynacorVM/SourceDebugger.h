#ifndef SOURCE_DEBUGGER_H_
#define SOURCE_DEBUGGER_H_

#include "SynacorVM.h"

#include <QWidget>
#include <vector>

QT_BEGIN_NAMESPACE
class QListView;
class QFileDialog;
QT_END_NAMESPACE

class OutputWidget;
class MemoryWidget;
class AssemblyWidget;


class SourceDebugger : public QWidget
{
	Q_OBJECT
public:
	SourceDebugger(QWidget *parent);

	void loadfile(const QString &filepath);

public slots:
	void load();
	void reduce();
	void reset();
	void run();
	void exit();
	void notifyError(VMErrors error);

protected:
	OutputWidget *outputWidget;
	MemoryWidget *memoryWidget;
	AssemblyWidget *assemblyWidget;

	SynacorVM *synacorVM;

	QFileDialog *fileDialog;

};

#endif // SOURCE_DEBUGGER_H_
