ECHO OFF
ECHO Starting Saitek SDK import...

SET TARGET=C:\Program Files
SET SAITEK=%TARGET%\Saitek\DirectOutput\SDK
IF NOT EXIST SAITEK (
  SET SAITEK=%TARGET%\Logitech\DirectOutput\SDK
)

SET DEST0=Libs\Saitek
SET DEST1=%DEST0%\Import\Source
SET DEST2=%DEST0%\Include
SET FILE=%DEST1%\StdAfx.h

ECHO.
ECHO Creating target directory structure...

CD ..
@RD /S /Q %DEST0%
@MD %DEST1%
@MD %DEST2%

ECHO.
ECHO Copying required files...

@COPY "%SAITEK%\Examples\Test\ThreadLock.*" %DEST1%
@COPY "%SAITEK%\Examples\Test\RawImage.*" %DEST1%
@COPY "%SAITEK%\Include\*.*" %DEST2%

ECHO.
ECHO Creating dummy StdAfx.h file...

ECHO #pragma once > %FILE%
ECHO #include ^<Windows.h^> >> %FILE%
ECHO #include ^<Tchar.h^> >> %FILE%

CD Utils\

ECHO.
ECHO Done!
