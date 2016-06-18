#ifndef SOURCE_DEBUGGER_H_
#define SOURCE_DEBUGGER_H_

#include "SynacorVM.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
class QFileDialog;
class QToolBar;
class QToolButton;
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
	void refreshAssembly();
	void exit();
	void notifyError(VMErrors error);
	void resume();
	void pause();
	void stepInto();
	void stepOver();

	void updateDebuggerState(DebuggerState dState);

signals:
	void aboutToQuit();
	void activateVM();
	void pauseVM(bool pause);

protected:
	OutputWidget *outputWidget;
	MemoryWidget *memoryWidget;
	AssemblyWidget *assemblyWidget;

	SynacorVM *synacorVM;

	QFileDialog *fileDialog;

	QAction *runAction;
	QToolBar *toolbar;
	QToolButton *toolbutton;

	DebuggerState DState;

};

#endif // SOURCE_DEBUGGER_H_
