#ifndef SOURCE_DEBUGGER_H_
#define SOURCE_DEBUGGER_H_

#include "SynacorVM.h"

QT_BEGIN_NAMESPACE
class QListView;
class QFileDialog;
class QToolBar;
class QToolButton;
class QMainWindow;
QT_END_NAMESPACE

class OutputWidget;
class MemoryWidget;
class AssemblyWidget;


class SourceDebugger : public QObject
{
	Q_OBJECT
public:
	SourceDebugger(QMainWindow *parent);

	void loadfile(const QString &filepath);

public slots:
	void load();
	void reset();
	void refreshAssembly();
	void exit();
	void notifyError(VMErrors error);
	void resume();
	void pause();
	void stepInto();
	void stepOver();
	void stepOut();

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

	QMainWindow *parentWindow;

};

#endif // SOURCE_DEBUGGER_H_
