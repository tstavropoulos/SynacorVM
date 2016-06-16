#include "OutputWidget.h"

#include <QListView>
#include <QHBoxLayout>

OutputWidget::OutputWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	listView = new QListView(this);
	layout->addWidget(listView);

	setLayout(layout);
}


void OutputWidget::print(const QString &line)
{

}

void OutputWidget::clear()
{

}