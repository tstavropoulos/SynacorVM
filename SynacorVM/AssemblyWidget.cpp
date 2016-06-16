#include "AssemblyWidget.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QListView>

AssemblyWidget::AssemblyWidget(QWidget *parent)
	: QWidget(parent)
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
	//const uint16_t *assembly = (const uint16_t *)buffer;

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
}

void AssemblyWidget::reduce()
{
	QString lastArg;
	QString tmpPrintBuffer;
	//int lastNum = 0;

	QStringList reducedList;
	QList<int> reducedInstrNum;
	int numDatas = 0;

	for (int i = 0; i < instr.length(); i++)
	{
		bool haltWasData = false;

		if (lastArg == "PRNT" && instr[i] != "PRNT")
		{
			reducedList << QString("%1:\t").arg(instrNum[i - 1]) + lastArg + " " + tmpPrintBuffer;
			reducedInstrNum << instrNum[i - 1];
			tmpPrintBuffer = "";
		}
		else if (lastArg == "DATA")
		{
			if (instr[i] == "HALT")
			{
				tmpPrintBuffer += QString("0000") + (((numDatas++ % 8) == 7) ? "\n" : " ");
				haltWasData = true;
			}
			else if (instr[i] != "DATA")
			{
				reducedList << QString("%1:\t").arg(instrNum[i - 1]) + lastArg + "\n" + tmpPrintBuffer;
				reducedInstrNum << instrNum[i - 1];
				tmpPrintBuffer = "";
			}
		}
		
		if (instr[i] == "")
		{
			break;
		}

		if (instr[i] == "NOOP")
		{
			continue;
		}

		if (instr[i] == "PRNT")
		{
			tmpPrintBuffer += args[i];
		}
		else if (instr[i] == "DATA")
		{
			tmpPrintBuffer += args[i] + (((numDatas++ % 8) == 7) ? "\n" : " ");
		}
		else
		{
			reducedList << QString("%1:\t").arg(instrNum[i]) + instr[i] + ((args[i] == "") ? ("") : (" " + args[i]));
			reducedInstrNum << instrNum[i];
		}
		
		lastArg = haltWasData ? "DATA" : instr[i];
		if (lastArg != "DATA")
		{
			numDatas = 0;
		}
	}

	listModel->setStringList(reducedList);
}
