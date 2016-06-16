#include "MemoryWidget.h"

#include <QListView>
#include <QHBoxLayout>

MemoryWidget::MemoryWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	listView = new QListView(this);
	layout->addWidget(listView);

	setLayout(layout);
}
