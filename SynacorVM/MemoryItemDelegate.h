#ifndef MEMORY_ITEM_DELEGATE_H_
#define MEMORY_ITEM_DELEGATE_H_

class MemoryItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit MemoryItemDelegate(QObject *parent = Q_NULLPTR);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						  const QModelIndex &index) const Q_DECL_OVERRIDE;

	void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;

	void setModelData(QWidget *editor,
					  QAbstractItemModel *model,
					  const QModelIndex &index) const Q_DECL_OVERRIDE;

	QSize sizeHint(const QStyleOptionViewItem &option,
				   const QModelIndex &index) const Q_DECL_OVERRIDE;
signals:
	void memoryEdited(uint16_t index, uint16_t value) const;
};

#endif //MEMORY_ITEM_DELEGATE_H_
