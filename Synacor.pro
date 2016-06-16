#-------------------------------------------------
#
# Project created by QtCreator 2016-06-13T21:34:12
#
#-------------------------------------------------


TARGET = Synacor
TEMPLATE = subdirs

QT += core gui widgets
CONFIG -= flat
CONFIG += c++14
CONFIG += ordered

defineTest(addSubdirs) {
    for(subdirs, 1) {
        entries = $$files($$subdirs)
        for(entry, entries) {
            name = $$replace(entry, [/\\\\], _)
            SUBDIRS += $$name
            eval ($${name}.subdir = $$entry)
            for(dep, 2):eval ($${name}.depends += $$replace(dep, [/\\\\], _))
            export ($${name}.subdir)
            export ($${name}.depends)
        }
    }
    export (SUBDIRS)
}

addSubdirs(SynacorVM)

QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Core.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Gui.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Network.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Script.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Q5tScriptTools.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Sql.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Xml.dll
QTLIBS.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Widgets.dll
QTLIBS.path = $$(SD_TREE)/output/bin/release
INSTALLS += QTLIBS

QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Cored.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Guid.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Networkd.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Scriptd.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5ScriptToolsd.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Sqld.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Xmld.dll
QTLIBS_DEBUG.files += $$[QT_INSTALL_PREFIX]/lib/Qt5Widgetsd.dll
QTLIBS_DEBUG.path = $$(SD_TREE)/output/bin/debug
INSTALLS += QTLIBS_DEBUG


CRUNTIMEPRIVATEASSEMBLY.path = $$(SD_TREE)/output/bin/release
INSTALLS += CRUNTIMEPRIVATEASSEMBLY
CRUNTIMEPRIVATEASSEMBLY_IMAGEFORMATS.path = $$(SD_TREE)/output/bin/release/imageformats
INSTALLS += CRUNTIMEPRIVATEASSEMBLY_IMAGEFORMATS

CRUNTIMEPRIVATEASSEMBLY_DEBUG.path = $$(SD_TREE)/output/bin/debug
INSTALLS += CRUNTIMEPRIVATEASSEMBLY_DEBUG
CRUNTIMEPRIVATEASSEMBLY_IMAGEFORMATS_DEBUG.path = $$(SD_TREE)/output/bin/debug/imageformats
INSTALLS += CRUNTIMEPRIVATEASSEMBLY_IMAGEFORMATS_DEBUG
CRUNTIMEPRIVATEASSEMBLY_DEBUG_RELEASE_CRT.path = $$(SD_TREE)/output/bin/debug
INSTALLS += CRUNTIMEPRIVATEASSEMBLY_DEBUG_RELEASE_CRT
