@ECHO OFF

SET APP=Kapow
SET VERSION=1.5.1

ECHO Copying executable
MKDIR %APP%
TYPE COPYING | FIND "" /V > %APP%\COPYING.txt
TYPE CREDITS | FIND "" /V > %APP%\CREDITS.txt
TYPE README | FIND "" /V > %APP%\README.txt
COPY release\%APP%.exe %APP% >nul
strip %APP%\%APP%.exe

ECHO Copying translations
SET TRANSLATIONS=%APP%\translations
MKDIR %TRANSLATIONS%
COPY translations\*.qm %TRANSLATIONS% >nul
COPY %QTDIR%\translations\qtbase_*.qm %TRANSLATIONS% >nul

ECHO Copying Qt libraries
COPY %QTDIR%\bin\libgcc_s_dw2-1.dll %APP% >nul
COPY "%QTDIR%\bin\libstdc++-6.dll" %APP% >nul
COPY %QTDIR%\bin\libwinpthread-1.dll %APP% >nul
COPY %QTDIR%\bin\Qt5Core.dll %APP% >nul
COPY %QTDIR%\bin\Qt5Gui.dll %APP% >nul
COPY %QTDIR%\bin\Qt5Network.dll %APP% >nul
COPY %QTDIR%\bin\Qt5PrintSupport.dll %APP% >nul
COPY %QTDIR%\bin\Qt5Widgets.dll %APP% >nul

MKDIR %APP%\bearer
COPY %QTDIR%\plugins\bearer\qgenericbearer.dll %APP%\bearer >nul
COPY %QTDIR%\plugins\bearer\qnativewifibearer.dll %APP%\bearer >nul

MKDIR %APP%\platforms
COPY %QTDIR%\plugins\platforms\qwindows.dll %APP%\platforms >nul

MKDIR %APP%\printsupport
COPY %QTDIR%\plugins\printsupport\windowsprintersupport.dll %APP%\printsupport >nul

ECHO Making portable
MKDIR %APP%\Data

ECHO Creating compressed file
CD %APP%
7z a -mx=9 %APP%_%VERSION%.zip * >nul
CD ..
MOVE %APP%\%APP%_%VERSION%.zip . >nul

ECHO Cleaning up
RMDIR /S /Q %APP%
