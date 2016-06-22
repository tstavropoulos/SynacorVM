#pragma once

#if defined __cplusplus

//stl Includes
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <mutex>

#include <cmath>
#include <cctype>
#include <cstdint>
#include <cassert>

//Qt Includes
#include <QApplication>
#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QMessageBox>
#include <QListView>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QLineEdit>
#include <QDockWidget>
#include <QLabel>

//Windows Includes
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static const uint32_t MAX_RECENT_FILES = 3;

#endif