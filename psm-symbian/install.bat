@echo off

rem In order to be able to link this project to the psmclient library,
rem the relevant headers and libraries must be copied to Symbian SDK
rem directory, either manually or with this batch file.

set symbianSdk=c:\QtSDK\Symbian\SDKs\Symbian3Qt474\epoc32

if not exist %symbianSdk%\include\gdi.h goto badSdk
if not exist install.bat goto badSource

xcopy include %symbianSdk%\include /S /Y /F
xcopy release %symbianSdk%\release /S /Y /F

goto end

:badSdk
        echo SDK directory %symbianSdk% is not valid
        goto end


:badSource
        echo Current directory is wrong. Run install.bat from steps\psm-symbian
        goto end

:end

