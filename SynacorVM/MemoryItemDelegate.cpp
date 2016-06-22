#include "stdafx.h"

#include "RegisterEditor.h"
#include "RegisterItemDelegate.h"

RegisterItemDelegate::RegisterItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *RegisterItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
									  const QModelIndex &index) const
{
	if (index.data().canConvert<QString>())
	{
		RegisterEditor *registerEditor = new RegisterEditor(parent, index.data().toString());
		connect(registerEditor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
		return registerEditor;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void RegisterItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	RegisterEditor *registerEditor = qobject_cast<RegisterEditor *>(editor);
	registerEditor->setString(index.data().toString());
}

void RegisterItemDelegate::setModelData(QWidget *editor,
										QAbstractItemModel *,
										const QModelIndex &index) const
{
	RegisterEditor *registerEditor = qobject_cast<RegisterEditor *>(editor);
	uint16_t value = (uint16_t)registerEditor->getEditText().toUInt(nullptr, 16);
	emit registerEdited((uint16_t)index.row(), value);
}

QSize RegisterItemDelegate::sizeHint(const QStyleOptionViewItem &,
									 const QModelIndex &) const
{
	static RegisterEditor staticDummy(nullptr, "r0:\t0000");
	return staticDummy.sizeHint();
}

void RegisterItemDelegate::commitAndCloseEditor()
{
	RegisterEditor *registerEditor = qobject_cast<RegisterEditor *>(sender());
	emit commitData(registerEditor);
	emit closeEditor(registerEditor);
}
