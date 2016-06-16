#ifndef OUTPUT_WIDGET_H_
#define OUTPUT_WIDGET_H_

#include <QWidget>

QT_BEGIN_NAMESPACE
class QListView;
class QStringListModel;
class QLineEdit;
QT_END_NAMESPACE

class OutputWidget : public QWidget
{
	Q_OBJECT
public:
	OutputWidget(QWidget *parent);

protected:
	QListView *listView;
	QStringList output;
	QStringListModel *listModel;
	QLineEdit *editWidget;

public slots:
	void print(const QString &line);
	void clear();
	void lineEditReturn();

signals:
	void submitInput(const QString &line);
};

#endif // OUTPUT_WIDGET_H_
