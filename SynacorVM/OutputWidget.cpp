#include "OutputWidget.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>

OutputWidget::OutputWidget(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	listView = new QListView(this);
	layout->addWidget(listView);

	setLayout(layout);

	output << QString();

	listModel = new QStringListModel(this);
	listView->setModel(listModel);

	listModel->setStringList(output);
}


void OutputWidget::print(const QString &line)
{
	for (int i = 0; i < line.length(); i++)
	{
		if (line[i] == char(10))
		{
			output << QString();
		}
		else
		{
			output.back() += line[i];
		}
	}

	listModel->setStringList(output);
	
}

void OutputWidget::clear()
{
	output.clear();

	output << QString();

	listModel->setStringList(output);
}