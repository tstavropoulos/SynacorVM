#include "AssemblyWidget.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QListView>
#include <QMessageBox>

AssemblyWidget::AssemblyWidget(QWidget *parent)
	: QWidget(parent), loaded(false), showReduced(false), currentExecAddress(0)
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
	listView->setMovement(QListView::Static);
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(listView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(operationBreakToggled(const QModelIndex&)));
}

void AssemblyWidget::setAssembly(const QStringList &instructions, const QStringList &arguments, const std::vector<uint16_t> &instrAddress)
{
	instr = instructions;
	args = arguments;
	this->instrAddress = instrAddress;

	QStringList temp;

	loaded = true;

	if (breakpoints.size() != instr.length())
	{
		breakpoints = std::vector<bool>(instr.length());
	}

	rebuild();
}

void AssemblyWidget::rebuild()
{
	if (showReduced)
	{
		rebuildReduced();
	}
	else
	{
		rebuildSimple();
	}
}

void AssemblyWidget::rebuildSimple()
{
	QStringList temp;

	for (int i = 0; i < instr.length(); i++)
	{
		if (instr[i] == "")
		{
			break;
		}
		temp << QString("%1%2  %3:\t")
			.arg((instrAddress[i] == currentExecAddress) ? ">" : " ")
			.arg(breakpoints[i] ? "*" : " ")
			.arg(instrAddress[i], 4, 16, QChar('0'))
			+ instr[i] + " " + args[i];
	}

	currentInstrAddress = instrAddress;

	listModel->setStringList(temp);
	currentAssembly = temp;
}

void AssemblyWidget::rebuildReduced()
{

	QString lastInstr;
	QString tmpPrintBuffer;

	int instBegin = -1;

	QStringList reducedList;
	currentInstrAddress.clear();
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
			reducedList << QString("%1%2 %3-%4:\t")
				.arg((instrAddress[i] == currentExecAddress) ? ">" : " ")
				.arg(breakpoints[instBegin] ? "*" : " ")
				.arg(instBegin, 4, 16, QChar('0'))
				.arg(instrAddress[i - 1], 4, 16, QChar('0'))
				+ "PRNT " + tmpPrintBuffer;
			tmpPrintBuffer = "";
			currentInstrAddress.push_back(instBegin);
			instBegin = -1;
		}
		else if (lastInstr == "DATA" && (thisInstr != "DATA" || numDatas >= 8))
		{
			reducedList << QString("%1%2 %3-%4:\t")
				.arg((instrAddress[i] == currentExecAddress) ? ">" : " ")
				.arg(breakpoints[instBegin] ? "*" : " ")
				.arg(instBegin, 4, 16, QChar('0'))
				.arg(instrAddress[i - 1], 4, 16, QChar('0'))
				+ "DATA " + tmpPrintBuffer;
			tmpPrintBuffer = "";
			currentInstrAddress.push_back(instBegin);
			numDatas = 0;
			instBegin = -1;
		}
		else if (lastInstr == "NOOP" && thisInstr != "NOOP")
		{
			reducedList << QString("%1%2 %3-%4:\tNOOP\t(%5)")
				.arg((instrAddress[i] == currentExecAddress) ? ">" : " ")
				.arg(breakpoints[instBegin] ? "*" : " ")
				.arg(instBegin, 4, 16, QChar('0'))
				.arg(instrAddress[i - 1], 4, 16, QChar('0'))
				.arg(numNoops);
			currentInstrAddress.push_back(instBegin);
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
				instBegin = instrAddress[i];
			}
		}
		else if (thisInstr == "PRNT")
		{
			tmpPrintBuffer += thisArg;
			if (instBegin == -1)
			{
				instBegin = instrAddress[i];
			}
		}
		else if (thisInstr == "DATA")
		{
			tmpPrintBuffer += thisArg + " ";
			numDatas++;
			if (instBegin == -1)
			{
				instBegin = instrAddress[i];
			}
		}
		else
		{
			reducedList << QString("%1%2      %3:\t")
				.arg((instrAddress[i] == currentExecAddress)?">":" ")
				.arg(breakpoints[i] ? "*" : " ")
				.arg(instrAddress[i], 4, 16, QChar('0'))
				+ thisInstr
				+ ((thisArg == "") ? ("") : (" " + thisArg));
			currentInstrAddress.push_back(instrAddress[i]);
		}
		lastInstr = thisInstr;
	}

	listModel->setStringList(reducedList);

	currentAssembly = reducedList;
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

	showReduced = !showReduced;

	rebuild();
}

void AssemblyWidget::operationBreakToggled(const QModelIndex &index)
{
	const int instrNum = currentInstrAddress[index.row()];
	breakpoints[instrNum] = !breakpoints[instrNum];

	emit setBreakpoint(instrNum, breakpoints[instrNum]);

	currentAssembly[index.row()].replace(1,1, (breakpoints[instrNum] ? "*" : " "));

	listModel->setData(index, currentAssembly[index.row()]);
}

void AssemblyWidget::updatePointer(uint16_t address)
{
	if (address == currentExecAddress)
	{
		return;
	}

	auto iter = std::lower_bound(currentInstrAddress.begin(), currentInstrAddress.end(), currentExecAddress);
	uint16_t oldIndex = iter - currentInstrAddress.begin();
	currentAssembly[oldIndex].replace(0, 1, " ");

	currentExecAddress = address;

	iter = std::lower_bound(currentInstrAddress.begin(), currentInstrAddress.end(), currentExecAddress);
	uint16_t newIndex = iter - currentInstrAddress.begin();
	currentAssembly[newIndex].replace(0, 1, ">");

	
	listModel->setData(listModel->index(oldIndex), currentAssembly[oldIndex]);
	listModel->setData(listModel->index(newIndex), currentAssembly[newIndex]);

	QModelIndex index = listModel->index(newIndex);

	listView->scrollTo(index);
	listView->setCurrentIndex(index);
}

void AssemblyWidget::scrollToInstruction(uint16_t address)
{
	auto iter = std::lower_bound(currentInstrAddress.begin(), currentInstrAddress.end(), address);
	uint16_t instIndex = iter - currentInstrAddress.begin();
	QModelIndex index = listModel->index(instIndex);
	listView->scrollTo(index);
	listView->setCurrentIndex(index);
}
