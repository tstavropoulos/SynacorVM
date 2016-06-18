#include "stdafx.h"

#include "OutputWidget.h"

#include <QListView>
#include <QVBoxLayout>
#include <QStringListModel>
#include <QLineEdit>

OutputWidget::OutputWidget(QWidget *parent)
	: QDockWidget("Output", parent)
{
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	QWidget *internalWidget = new QWidget(this);

	QVBoxLayout *layout = new QVBoxLayout(internalWidget);

	listView = new QListView(this);
	listView->setUniformItemSizes(true);
	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");
	listView->setFont(monoSpacedFont);

	listView->setMovement(QListView::Static);
	listView->setWordWrap(true);
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	editWidget = new QLineEdit(this);
	editWidget->setFont(monoSpacedFont);

	connect(editWidget, SIGNAL(returnPressed()), this, SLOT(lineEditReturn()));

	layout->addWidget(listView);
	layout->addWidget(editWidget);

	internalWidget->setLayout(layout);

	output << QString();

	listModel = new QStringListModel(internalWidget);
	listView->setModel(listModel);

	listModel->setStringList(output);

	setWidget(internalWidget);
	internalWidget->setMinimumWidth((parent->size().width() - 20) / 2);
}

void OutputWidget::lineEditReturn()
{
	//Add a linefeed to the end of the input
	QString input = editWidget->text() + QString(10);

	//Clear the text
	editWidget->clear();

	emit submitInput(input);
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

	listView->scrollToBottom();

}

void OutputWidget::clear()
{
	output.clear();

	output << QString();

	listModel->setStringList(output);
}