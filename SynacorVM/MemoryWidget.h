#ifndef MEMORY_WIDGET_H_
#define MEMORY_WIDGET_H_

#include <QWidget>


QT_BEGIN_NAMESPACE
class QListView;
QT_END_NAMESPACE

class MemoryWidget : public QWidget
{
	Q_OBJECT
public:
	MemoryWidget(QWidget *parent);

protected:
	QListView *listView;
};

#endif // MEMORY_WIDGET_H_
