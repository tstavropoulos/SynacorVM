#ifndef REGISTER_EDITOR_H_
#define REGISTER_EDITOR_H_

class MemoryEditor : public QWidget
{
	Q_OBJECT

public:
	explicit MemoryEditor(QWidget *parent, const QString &text);
	virtual ~MemoryEditor();

	void setString(const QString &s);
	const QString &getLabelText() const { return labelText; }
	const QString &getEditText(int i) const { return editText[i]; }

public slots:
	void sendEditingFinished();

signals:
	void editingFinished();

protected:
	QHBoxLayout *layout;
	QLabel *label;
	QLineEdit *edits[16];

	QString labelText;
	QString editText[16];
	QMetaObject::Connection connections[16];
};

#endif //REGISTER_EDITOR_H_
