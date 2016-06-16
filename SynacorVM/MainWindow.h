#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QMenu;
class QSignalMapper;
QT_END_NAMESPACE

class SourceDebugger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	SourceDebugger *sourceDebugger_;

	void createMenus();
	QMenu *fileMenu;
	QMenu *styleMenu;
};

#endif // MAIN_WINDOW_H_
