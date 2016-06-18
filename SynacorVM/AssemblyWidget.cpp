#include "stdafx.h"

#include "AssemblyWidget.h"
#include "SynacorVM.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QListView>
#include <QMessageBox>

AssemblyWidget::AssemblyWidget(QWidget *parent)
	: QDockWidget("Dissassembly", parent), loaded(false), currentExecAddress(0)
{
	setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	setFeatures(DockWidgetMovable | DockWidgetFloatable);
	QWidget *internalWidget = new QWidget(this);

	QHBoxLayout *layout = new QHBoxLayout(internalWidget);

	breakpoints = std::vector<bool>(c_dwAddressSpace, false);

	listView = new QListView(this);
	listView->setUniformItemSizes(true);
	QFont monoSpacedFont;
	monoSpacedFont.setStyleHint(QFont::Monospace);
	monoSpacedFont.setFamily("Consolas");
	listView->setFont(monoSpacedFont);
	layout->addWidget(listView);

	internalWidget->setLayout(layout);

	listModel = new QStringListModel(internalWidget);
	listView->setModel(listModel);
	listView->setMovement(QListView::Static);
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(listView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(operationBreakToggled(const QModelIndex&)));

	setWidget(internalWidget);
	internalWidget->setMinimumWidth((parent->size().width() - 20) / 2);
}

void AssemblyWidget::setAssembly(const QStringList &instructions, const QStringList &arguments, const std::vector<uint16_t> &instrAddress)
{
	instr = instructions;
	args = arguments;
	this->instrAddress = instrAddress;

	QStringList temp;

	loaded = true;

	rebuild();
}

void AssemblyWidget::rebuild()
{
	if (listModel->rowCount() < instr.length())
	{
		listModel->insertRows(listModel->rowCount(), instr.length() - listModel->rowCount());
		for (int i = currentAssembly.length(); i < instr.length(); i++)
		{
			currentAssembly.append(QString());
		}
	}

	for (int i = 0; i < instr.length(); i++)
	{
		uint16_t curInstAddr = instrAddress[i];
		QString newLineText = QString("%1%2  %3:\t")
			.arg((curInstAddr == currentExecAddress) ? ">" : " ")
			.arg(breakpoints[curInstAddr] ? "*" : " ")
			.arg(curInstAddr, 4, 16, QChar('0'))
			+ instr[i] + " " + args[i];
		if (currentAssembly[i] != newLineText)
		{
			currentAssembly[i] = newLineText;
			listModel->setData(listModel->index(i), currentAssembly[i]);
		}
	}
}

void AssemblyWidget::operationBreakToggled(const QModelIndex &index)
{
	const int instrNum = instrAddress[index.row()];
	breakpoints[instrNum] = !breakpoints[instrNum];

	emit setBreakpoint(instrNum, breakpoints[instrNum]);

	currentAssembly[index.row()].replace(1, 1, (breakpoints[instrNum] ? "*" : " "));

	listModel->setData(index, currentAssembly[index.row()]);
}

void AssemblyWidget::updatePointer(uint16_t address)
{
	if (address == currentExecAddress)
	{
		return;
	}

	auto iter = std::lower_bound(instrAddress.begin(), instrAddress.end(), currentExecAddress);
	uint16_t oldIndex = iter - instrAddress.begin();
	currentAssembly[oldIndex].replace(0, 1, " ");

	currentExecAddress = address;

	iter = std::lower_bound(instrAddress.begin(), instrAddress.end(), currentExecAddress);
	uint16_t newIndex = iter - instrAddress.begin();
	currentAssembly[newIndex].replace(0, 1, ">");

	listModel->setData(listModel->index(oldIndex), currentAssembly[oldIndex]);
	listModel->setData(listModel->index(newIndex), currentAssembly[newIndex]);

	QModelIndex index = listModel->index(newIndex);

	listView->scrollTo(index);
	listView->setCurrentIndex(index);
}

void AssemblyWidget::scrollToInstruction(uint16_t address)
{
	auto iter = std::lower_bound(instrAddress.begin(), instrAddress.end(), address);
	uint16_t instIndex = iter - instrAddress.begin();
	QModelIndex index = listModel->index(instIndex);
	listView->scrollTo(index);
	listView->setCurrentIndex(index);
}
