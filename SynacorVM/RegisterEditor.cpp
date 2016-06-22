#include "stdafx.h"

#include "RegisterEditor.h"

RegisterEditor::RegisterEditor(QWidget *parent, const QString &text) : QWidget(parent)
{
	setAutoFillBackground(true);

	layout = new QHBoxLayout();
	label = new QLabel();
	edit = new QLineEdit();
	layout->addWidget(label);
	layout->addWidget(edit);
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);

	connect(edit, &QLineEdit::editingFinished, this, &RegisterEditor::sendEditingFinished);

	setString(text);
}

void RegisterEditor::setString(const QString &s)
{
	int cutIndex = s.indexOf(":\t");
	if (cutIndex == -1)
	{
		editText = s;
	}
	else
	{
		cutIndex += 2;
		labelText = s.left(cutIndex);
		editText = s.mid(cutIndex);
	}

	label->setText(labelText);
	edit->setText(editText);
}

void RegisterEditor::sendEditingFinished()
{
	editText = edit->text();

	emit editingFinished();
}
