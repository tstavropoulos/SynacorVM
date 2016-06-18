#include "stdafx.h"

#include "OutputWidget.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>

OutputWidget::OutputWidget(QWidget *parent)
	: QDockWidget("Output", parent)
{
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	QWidget *internalWidget = new QWidget(this);

	QVBoxLayout *layout = new QVBoxLayout(internalWidget);

	outputView = new QTextEdit(this);

	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");
	outputView->setFont(monoSpacedFont);

	//outputView->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	outputView->setReadOnly(true);
	outputView->setUndoRedoEnabled(false);

	editWidget = new QLineEdit(this);
	editWidget->setFont(monoSpacedFont);

	connect(editWidget, SIGNAL(returnPressed()), this, SLOT(lineEditReturn()));

	layout->addWidget(outputView);
	layout->addWidget(editWidget);

	internalWidget->setLayout(layout);

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
	outputView->moveCursor(QTextCursor::End);
	outputView->insertPlainText(line);
	outputView->ensureCursorVisible();
}

void OutputWidget::clear()
{
	outputView->setPlainText(QString());
}