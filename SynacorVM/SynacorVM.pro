#-------------------------------------------------
#
# Project created by QtCreator 2016-06-13T21:34:12
#
#-------------------------------------------------

QT += core gui widgets concurrent
DEPENDPATH += .
INCLUDEPATH += .

TARGET = Synacor
TEMPLATE = app

CONFIG += precompile_header

SOURCES += main.cpp

PRECOMPILED_HEADER = stdafx.h
PRECOMPILED_SOURCE = stdafx.cpp

HEADERS += stdafx.h

HEADERS += MainWindow.h
SOURCES += MainWindow.cpp

HEADERS += SourceDebugger.h
SOURCES += SourceDebugger.cpp

HEADERS += AssemblyWidget.h
SOURCES += AssemblyWidget.cpp

HEADERS += MemoryWidget.h
SOURCES += MemoryWidget.cpp

HEADERS += OutputWidget.h
SOURCES += OutputWidget.cpp

HEADERS += SynacorVM.h
SOURCES += SynacorVM.cpp

# install
target.path = $$(SD_TREE)/install/SynacorVM
INSTALLS += target

# Output
build_pass:CONFIG(debug, debug|release) {
  DESTDIR = $$(SD_TREE)/output/bin/debug
  OBJECTS_DIR	= $$(SD_TREE)/intermediates/obj/SynacorVM/debug
}
build_pass:CONFIG(release, debug|release) {
  DESTDIR = $$(SD_TREE)/output/bin/release
  OBJECTS_DIR	= $$(SD_TREE)/intermediates/obj/SynacorVM/release
}

RCC_DIR = $$(SD_TREE)/intermediates/resources/SynacorVM
UI_DIR = $$(SD_TREE)/intermediates/ui/SynacorVM
MOC_DIR = $$(SD_TREE)/intermediates/moc/SynacorVM

build_pass::CONFIG(debug, debug|release) {
	QMAKE_CXXFLAGS+=/Fd$(IntDir)
}

CONFIG += embed_manifest_exe
QMAKE_LFLAGS += /MACHINE:X64



RESOURCES += SynacorVM.qrc