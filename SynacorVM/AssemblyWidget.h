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
	void setAssembly(const QStringList &instructions, const QStringList &arguments);
	void reduce();
protected:
	QListView *listView;
	QStringListModel *listModel;

	QStringList instr;
	QStringList args;
	QList<int> instrNum;

	bool loaded;
};

#endif // ASSEMBLY_WIDGET_H_
