#include "stdafx.h"

#include "MemoryEditor.h"

MemoryEditor::MemoryEditor(QWidget *parent, const QString &text) : QWidget(parent)
{
	setAutoFillBackground(true);

	layout = new QHBoxLayout();
	label = new QLabel();
	label->setFixedWidth(label->fontMetrics().width("0000  |  "));
	label->setAccessibleName("edit");
	layout->addWidget(label);
	for (int i = 0; i < 16; i++)
	{
		edits[i] = new QLineEdit();
		edits[i]->setMaxLength(4);
		edits[i]->setFixedWidth(edits[i]->fontMetrics().width("0000") + 14);
		edits[i]->setAccessibleName("edit");

		layout->addWidget(edits[i]);
		connections[i] = connect(edits[i], &QLineEdit::editingFinished, this, &MemoryEditor::sendEditingFinished);
	}
	layout->addStretch();
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);

	setString(text);
}

MemoryEditor::~MemoryEditor()
{
	for (int i = 0; i < 16; i++)
	{
		disconnect(connections[i]);
		layout->removeWidget(edits[i]);
		delete edits[i];
	}
	layout->removeWidget(label);
	delete label;
	delete layout;
}

void MemoryEditor::setString(const QString &s)
{
	int cutIndex = s.indexOf("  |  ");
	if (cutIndex == -1)
	{
		labelText = s;
	}
	else
	{
		cutIndex += 5;
		labelText = s.left(cutIndex);
		for (int i = 0; i < 16; i++)
		{
			editText[i] = s.mid(cutIndex, 4);
			cutIndex += 5;
		}
	}

	label->setText(labelText);
	for (int i = 0; i < 16; i++)
	{
		edits[i]->setText(editText[i]);
	}
}

void MemoryEditor::sendEditingFinished()
{
	for (int i = 0; i < 16; i++)
	{
		editText[i] = edits[i]->text();
	}

	emit editingFinished();
}
