@set SD_TREE=%CD%
@set SD_ROOT_DIR=C:
@set SD_MACHINE_TYPE=X64
@set SD_ENV_SET=TRUE

@REM Setting the directories for each of the supported builds
@set SD_QT_INSTALL_TOP_DIR=Qt
@set SD_QT_MSVC_DIR=msvc2015_64


set QTTOP=%SD_ROOT_DIR%\%SD_QT_INSTALL_TOP_DIR%\5.6
set PATH=%QTTOP%\%SD_QT_MSVC_DIR%\bin;%PATH%
set QTDIR=%QTTOP%\%SD_QT_MSVC_DIR%

@REM SETLOCAL creates a local scope for environment variables
@REM Just doing this to contain the local, build-dependent variables
@SETLOCAL

@SET SD_SOLUTION_NAME=SynacorDebugger.sln


@echo Generating Visual Studio Solution
@qmake -recursive -tp vc

popd
@ENDLOCAL


pushd %SD_TREE%

@echo Getting Qt libraries
@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\release Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5WinExtras.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\debug Qt5Cored.dll Qt5Guid.dll Qt5Widgetsd.dll Qt5WinExtrasd.dll Qt5Cored.pdb Qt5Guid.pdb Qt5Widgetsd.pdb Qt5WinExtrasd.pdb /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\platforms %SD_TREE%\output\bin\release\platforms qwindows.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\platforms %SD_TREE%\output\bin\debug\platforms qwindowsd.dll /NFL /NDL /NJH /NJS 1> nul

@popd
@ENDLOCAL