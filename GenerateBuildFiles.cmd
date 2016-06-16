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
@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\release Qt*Core.dll Qt*Gui.dll Qt*Xml.dll Qt*Sql.dll Qt*Widgets.dll Qt*Test.dll Qt*WinExtras.dll libGLESv2.dll libEGL.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\debug Qt*Cored.dll Qt*Guid.dll Qt*Xmld.dll Qt*Sqld.dll Qt*Widgetsd.dll Qt*Testd.dll Qt*WinExtrasd.dll libGLESv2d.dll libEGLd.dll Qt*Cored.pdb Qt*Guid.pdb Qt*Xmld.pdb Qt*Sqld.pdb Qt*Widgetsd.pdb Qt*Testd.pdb Qt*WinExtrasd.pdb libGLESv2d.pdb libEGLd.pdb /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\release icuin*.dll icuuc*.dll icudt*.dll D3DCompiler_43.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\bin %SD_TREE%\output\bin\debug icuin*.dll icuuc*.dll icudt*.dll D3DCompiler_43.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\platforms %SD_TREE%\output\bin\release\platforms qwindows.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\platforms %SD_TREE%\output\bin\debug\platforms qwindowsd.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\sqldrivers %SD_TREE%\output\bin\release\sqldrivers qsqlite.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\sqldrivers %SD_TREE%\output\bin\debug\sqldrivers qsqlited.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\imageformats %SD_TREE%\output\bin\release\imageformats qico.dll /NFL /NDL /NJH /NJS 1> nul

@ROBOCOPY %QTDIR%\plugins\imageformats %SD_TREE%\output\bin\debug\imageformats qicod.dll /NFL /NDL /NJH /NJS 1> nul

@popd
@ENDLOCAL