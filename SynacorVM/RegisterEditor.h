#ifndef REGISTER_EDITOR_H_
#define REGISTER_EDITOR_H_

class RegisterEditor : public QWidget
{
	Q_OBJECT

public:
	explicit RegisterEditor(QWidget *parent, const QString &text);
	void setString(const QString &s);
	const QString &getLabelText() const { return labelText; }
	const QString &getEditText() const { return editText; }

public slots:
	void sendEditingFinished();

signals:
	void editingFinished();

protected:
	QHBoxLayout *layout;
	QLabel *label;
	QLineEdit *edit;

	QString labelText;
	QString editText;
};

#endif //REGISTER_EDITOR_H_
