#ifndef OUTPUT_WIDGET_H_
#define OUTPUT_WIDGET_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
QT_END_NAMESPACE

class OutputWidget : public QWidget
{
	Q_OBJECT
public:
	OutputWidget(QWidget *parent);

protected:
	QListView *listView;

public slots:
	void print(const QString &line);
	void clear();
};

#endif // OUTPUT_WIDGET_H_
