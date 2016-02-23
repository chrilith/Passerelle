ECHO OFF
ECHO Starting Saitek SDK import...

SET SAITEK=C:\Program Files\Saitek\DirectOutput\SDK

SET DEST1=Libs\Saitek\Import\Source
SET DEST2=Libs\Saitek\Include
SET FILE=%DEST1%\StdAfx.h

ECHO.
ECHO Creating target directory structure...

CD ..
@RD /S /Q %DEST%
@MD %DEST1%
@MD %DEST2%

ECHO.
ECHO Copying required files...

@COPY "%SAITEK%\Examples\Test\DirectOutputImpl.*" %DEST1%
@COPY "%SAITEK%\Examples\Test\ThreadLock.*" %DEST1%
@COPY "%SAITEK%\Examples\Test\RawImage.*" %DEST1%
@COPY "%SAITEK%\Include\*.*" %DEST2%

ECHO.
ECHO Creating dummy StdAfx.h file...

ECHO #pragma once > %FILE%
ECHO #include ^<Windows.h^> >> %FILE%
ECHO #include ^<Tchar.h^> >> %FILE%

ECHO.
ECHO Done!
