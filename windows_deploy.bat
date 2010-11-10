@ECHO OFF
ECHO Copying executable
MKDIR Kapow
COPY release\Kapow.exe Kapow
STRIP Kapow\Kapow.exe
ECHO Copying libraries
COPY %QTDIR%\bin\libgcc_s_dw2-1.dll Kapow
COPY %QTDIR%\bin\mingwm10.dll Kapow
COPY %QTDIR%\bin\QtCore4.dll Kapow
COPY %QTDIR%\bin\QtGui4.dll Kapow
COPY %QTDIR%\bin\QtXml4.dll Kapow
