#include "stdafx.h"

#include "AssemblyWidget.h"
#include "SynacorVM.h"

#include <QListView>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QListView>
#include <QMessageBox>

enum RowStates
{
	RS_INSTRUCTION_POINTER,
	RS_BREAKPOINT,
	RS_RECENT_JUMP,
	RS_MAX
};

static const Qt::GlobalColor s_RowStateColors[(1 << RS_MAX)] =
{
	Qt::transparent,	// 000
	Qt::darkCyan,		// 001
	Qt::darkRed,		// 010
	Qt::darkMagenta,	// 011
	Qt::darkYellow,		// 100
	Qt::darkCyan,		// 101
	Qt::darkRed,		// 110
	Qt::darkMagenta,	// 111
};

class RowColorStringListModel : public QStringListModel
{
public:
	explicit RowColorStringListModel(QObject *parent = Q_NULLPTR) : QStringListModel(parent) {}
	explicit RowColorStringListModel(const QStringList &strings, QObject *parent = Q_NULLPTR) : QStringListModel(strings, parent) {}

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE
	{
		if (role == Qt::BackgroundRole)
		{
			auto statesPair = rowStateMap.equal_range(index.row());
			uint32_t stateMask = 0;
			for (auto itr = statesPair.first; itr != statesPair.second; itr++)
			{
				stateMask |= 1 << itr->second;
			}
			QBrush background(s_RowStateColors[stateMask]);
			return background;
		}

		return QStringListModel::data(index, role);
	}

	void setRowState(int row, RowStates state, bool set)
	{
		auto statesPair = rowStateMap.equal_range(row);
		auto item = std::find_if(statesPair.first, statesPair.second, [state](const auto &pair)
		{
			return pair.second == state;
		});
		if (set)
		{
			if (item == statesPair.second)
			{
				rowStateMap.insert(std::make_pair(row, state));
			}
		}
		else
		{
			if (item != statesPair.second)
			{
				rowStateMap.erase(item);
			}
		}
	}

protected:
	std::unordered_multimap<int, RowStates> rowStateMap;
};

AssemblyWidget::AssemblyWidget(QWidget *parent)
	: QDockWidget("Dissassembly", parent), loaded(false), currentExecAddress(0), recentJumpAddress(0xFFFF)
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

	listModel = new RowColorStringListModel(internalWidget);
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
			listModel->setRowState(i, RS_INSTRUCTION_POINTER, curInstAddr == currentExecAddress);
			listModel->setRowState(i, RS_BREAKPOINT, breakpoints[curInstAddr]);
		}
		listModel->setRowState(i, RS_RECENT_JUMP, recentJumpAddress == curInstAddr);
	}
}

void AssemblyWidget::operationBreakToggled(const QModelIndex &index)
{
	const int instrNum = instrAddress[index.row()];
	breakpoints[instrNum] = !breakpoints[instrNum];

	emit setBreakpoint(instrNum, breakpoints[instrNum]);

	currentAssembly[index.row()].replace(1, 1, (breakpoints[instrNum] ? "*" : " "));

	listModel->setData(index, currentAssembly[index.row()]);
	listModel->setRowState(index.row(), RS_BREAKPOINT, breakpoints[instrNum]);
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
	listModel->setRowState(oldIndex, RS_INSTRUCTION_POINTER, false);
	listModel->setRowState(newIndex, RS_INSTRUCTION_POINTER, true);

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

	iter = std::lower_bound(instrAddress.begin(), instrAddress.end(), recentJumpAddress);
	uint16_t oldIndex = iter - instrAddress.begin();
	listModel->setRowState(oldIndex, RS_RECENT_JUMP, false);

	iter = std::lower_bound(instrAddress.begin(), instrAddress.end(), address);
	uint16_t newIndex = iter - instrAddress.begin();
	listModel->setRowState(newIndex, RS_RECENT_JUMP, true);

	recentJumpAddress = address;
}
