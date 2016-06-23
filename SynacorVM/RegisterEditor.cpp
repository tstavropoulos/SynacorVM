#include "stdafx.h"

#include "RegisterEditor.h"

RegisterEditor::RegisterEditor(QWidget *parent, const QString &text)
	: QWidget(parent)
	, editingHasFinished(false)
{
	setAutoFillBackground(true);
	setWindowOpacity(1.f);

	layout = new QHBoxLayout();
	label = new QLabel();
	label->setFixedWidth(label->fontMetrics().width("r00\t"));
	label->setAccessibleName("edit");
	edit = new QLineEdit();
	edit->setMaxLength(4);
	edit->setFixedWidth(edit->fontMetrics().width("0000") + 14);
	edit->setAccessibleName("edit");
	layout->addWidget(label);
	layout->addWidget(edit);
	layout->addStretch();
	layout->setMargin(0);
	layout->setSpacing(0);
	setLayout(layout);

	connection = connect(edit, &QLineEdit::editingFinished, this, &RegisterEditor::sendEditingFinished);

	setString(text);
}

RegisterEditor::~RegisterEditor()
{
	disconnect(connection);
	layout->removeWidget(edit);
	layout->removeWidget(label);
	delete edit;
	delete label;
	delete layout;
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
	if (editingHasFinished)
	{
		return;
	}
	editingHasFinished = true;

	editText = edit->text();
	
	emit editingFinished();
}
