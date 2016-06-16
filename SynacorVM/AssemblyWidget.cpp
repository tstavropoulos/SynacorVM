#include "AssemblyWidget.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QListView>
#include <QMessageBox>

AssemblyWidget::AssemblyWidget(QWidget *parent)
	: QWidget(parent), loaded(false)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	listView = new QListView(this);
	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");
	listView->setFont(monoSpacedFont);
	layout->addWidget(listView);

	setLayout(layout);

	listModel = new QStringListModel(this);
	listView->setModel(listModel);
}

void AssemblyWidget::setAssembly(const QStringList &instructions, const QStringList &arguments)
{
	instr = instructions;
	args = arguments;

	QStringList temp;

	for (int i = 0; i < instr.length(); i++)
	{
		if (instr[i] == "")
		{
			break;
		}
		temp << QString("%1\t").arg(i) + instr[i] + " " + args[i];
		instrNum << i;
	}


	listModel->setStringList(temp);

	loaded = true;
}

void AssemblyWidget::reduce()
{
	if (!loaded)
	{
		QMessageBox::warning(this,
			QString("Reduce Error"),
			QString("You cannot reduce the program until you load a binary file."));
		return;
	}

	QString lastInstr;
	QString tmpPrintBuffer;

	int instBegin = -1;

	QStringList reducedList;
	int numDatas = 0;
	int numNoops = 0;

	for (int i = 0; i < instr.length(); i++)
	{
		QString thisInstr = instr[i];
		QString thisArg = args[i];
		if (lastInstr == "DATA" && thisInstr == "HALT")
		{
			thisInstr = "DATA";
			thisArg = "0000";
		}
		else if (lastInstr == "DATA" && thisInstr == "NOOP")
		{
			thisInstr = "DATA";
			thisArg = "0015";
		}
		else if (thisInstr != "DATA")
		{
			numDatas = 0;
		}

		if (lastInstr == "PRNT" && thisInstr != "PRNT")
		{
			reducedList << QString("%1-%2:\t").arg(instBegin).arg(instrNum[i - 1]) + "PRNT " + tmpPrintBuffer;
			tmpPrintBuffer = "";
			instBegin = -1;
		}
		else if (lastInstr == "DATA" && (thisInstr != "DATA" || numDatas >= 8))
		{
			reducedList << QString("%1-%2:\t").arg(instBegin).arg(instrNum[i - 1]) + "DATA " + tmpPrintBuffer;
			tmpPrintBuffer = "";
			numDatas = 0;
			instBegin = -1;
		}
		else if (lastInstr == "NOOP" && thisInstr != "NOOP")
		{
			reducedList << QString("%1-%2:\tNOOP\t(%3)").arg(instBegin).arg(instrNum[i - 1]).arg(numNoops);
			numNoops = 0;
			instBegin = -1;
		}
		
		if (thisInstr == "")
		{
			break;
		}

		if (thisInstr == "NOOP")
		{
			numNoops++;
			if (instBegin == -1)
			{
				instBegin = instrNum[i];
			}
		}
		else if (thisInstr == "PRNT")
		{
			tmpPrintBuffer += thisArg;
			if (instBegin == -1)
			{
				instBegin = instrNum[i];
			}
		}
		else if (thisInstr == "DATA")
		{
			tmpPrintBuffer += thisArg + " ";
			numDatas++;
			if (instBegin == -1)
			{
				instBegin = instrNum[i];
			}
		}
		else
		{
			reducedList << QString("%1:\t").arg(instrNum[i]) + thisInstr + ((thisArg == "") ? ("") : (" " + thisArg));
		}
		lastInstr = thisInstr;
	}

	listModel->setStringList(reducedList);
}
