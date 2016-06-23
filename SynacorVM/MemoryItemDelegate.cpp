#include "stdafx.h"

#include "MemoryEditor.h"
#include "MemoryItemDelegate.h"

MemoryItemDelegate::MemoryItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QWidget *MemoryItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
									  const QModelIndex &index) const
{
	if (index.data().canConvert<QString>())
	{
		MemoryEditor *memoryEditor = new MemoryEditor(parent, index.data().toString());
		return memoryEditor;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void MemoryItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	MemoryEditor *memoryEditor = qobject_cast<MemoryEditor *>(editor);
	memoryEditor->setString(index.data().toString());
}

void MemoryItemDelegate::setModelData(QWidget *editor,
										QAbstractItemModel *,
										const QModelIndex &index) const
{
	MemoryEditor *memoryEditor = qobject_cast<MemoryEditor *>(editor);
	for (int i = 0; i < 16; i++)
	{
		uint16_t value = (uint16_t)memoryEditor->getEditText(i).toUInt(nullptr, 16);
		emit memoryEdited((uint16_t)index.row() * 16 + i, value);
	}
}

QSize MemoryItemDelegate::sizeHint(const QStyleOptionViewItem &,
									 const QModelIndex &) const
{
	static MemoryEditor staticDummy(nullptr, "0000  |  0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000  |  ................");
	return staticDummy.sizeHint();
}
