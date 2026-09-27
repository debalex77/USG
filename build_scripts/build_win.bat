@echo off
setlocal EnableExtensions
chcp 65001 >nul

rem USG - pachet Windows Release (Qt 6.9.3 / MSVC 2022 / Qt IFW 4.11)
rem Variabile suprascriptibile: QT_PATH, QIF_PATH, BUILD_EXE,
rem MYSQL_CLIENT_DLL, OPENSSL_BIN (optional shared runtime), LIMEREPORT_RELEASE, QT_SQLDRIVERS,
rem LIMEREPORT_SOURCE_DIR, ZIP_EXE, MYSQL_RUNTIME_DIR, CI_DEPENDENCY_LICENSES.

for %%I in ("%~dp0..") do set "PROJECT_PATH=%%~fI"
set "BUILD_PATH=%PROJECT_PATH%\build"
set "PREBUILD_PATH=%BUILD_PATH%\prebuild_windows"
set "INSTALLER_PATH=%BUILD_PATH%\installer_windows"

if not defined QT_PATH set "QT_PATH=C:\Qt\6.9.3\msvc2022_64"
if not defined QIF_PATH set "QIF_PATH=C:\Qt\Tools\QtInstallerFramework\4.11\bin"
if not defined BUILD_EXE set "BUILD_EXE=%BUILD_PATH%\Desktop_Qt_6_9_3_MSVC2022_64bit_Release\release\USG.exe"
rem OpenSSL is statically linked in the current build. Do not distribute .lib files.
rem Set OPENSSL_BIN explicitly only when shared OpenSSL DLLs are required.
if not defined LIMEREPORT_SOURCE_DIR set "LIMEREPORT_SOURCE_DIR=%PROJECT_PATH%\..\LimeReport-1.7.23"
if not exist "%LIMEREPORT_SOURCE_DIR%\LICENSE" if exist "%PROJECT_PATH%\..\LimeReport\LICENSE" set "LIMEREPORT_SOURCE_DIR=%PROJECT_PATH%\..\LimeReport"
if not defined ZIP_EXE set "ZIP_EXE=C:\Program Files\7-Zip\7z.exe"

set "WDEPLOY=%QT_PATH%\bin\windeployqt.exe"
set "BINARYCREATOR=%QIF_PATH%\binarycreator.exe"
if not defined LIMEREPORT_RELEASE set "LIMEREPORT_RELEASE=%PROJECT_PATH%\3rdparty\LimeReport\release"
if not defined QT_SQLDRIVERS set "QT_SQLDRIVERS=%QT_PATH%\plugins\sqldrivers"

call :require_file "%PROJECT_PATH%\version.txt" "Lipsește version.txt." || exit /b 1
call :require_file "%BUILD_EXE%" "Lipsește executabilul Release USG.exe." || exit /b 1
call :require_file "%WDEPLOY%" "Lipsește windeployqt.exe. Verifică QT_PATH." || exit /b 1
call :require_file "%ZIP_EXE%" "Lipsește 7-Zip pentru arhiva sursei LimeReport." || exit /b 1
call :require_file "%QT_PATH%\plugins\tls\qschannelbackend.dll" "Lipsește backendul TLS Windows Schannel." || exit /b 1
call :require_file "%BINARYCREATOR%" "Lipsește binarycreator.exe. Verifică QIF_PATH." || exit /b 1
call :require_file "%LIMEREPORT_RELEASE%\limereport.dll" "Lipsește LimeReport Release pentru Windows." || exit /b 1
call :require_file "%LIMEREPORT_RELEASE%\QtZint.dll" "Lipsește QtZint Release pentru Windows." || exit /b 1
call :require_file "%QT_SQLDRIVERS%\qsqlite.dll" "Lipsește pluginul Qt SQLite." || exit /b 1
call :require_file "%QT_SQLDRIVERS%\qsqlmysql.dll" "Lipsește pluginul Qt MySQL/MariaDB." || exit /b 1
call :require_file "%PROJECT_PATH%\installer\linux\config\config.xml" "Lipsește șablonul Qt IFW." || exit /b 1
call :require_file "%PROJECT_PATH%\installer\linux\packages\com.alovada.usg\meta\package.xml" "Lipsește package.xml." || exit /b 1
call :require_file "%LIMEREPORT_SOURCE_DIR%\LICENSE" "Lipsește licența din sursa LimeReport 1.7.23." || exit /b 1
call :require_file "%LIMEREPORT_SOURCE_DIR%\COPYING" "Lipsește COPYING din sursa LimeReport 1.7.23." || exit /b 1
call :require_file "%PROJECT_PATH%\patches\limereport\1.7.23\0001-make-singleton-destruction-idempotent.patch" "Lipsește patch-ul LimeReport distribuit de USG." || exit /b 1

set "VERSION="
set /p VERSION=<"%PROJECT_PATH%\version.txt"
if not defined VERSION call :die "version.txt este gol." || exit /b 1
echo(%VERSION%| findstr /R /X "[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*" >nul
if errorlevel 1 call :die "Versiune invalidă în version.txt: %VERSION%" || exit /b 1

rem Clientul cerut de qsqlmysql.dll diferă în funcție de kit.
if not defined MYSQL_CLIENT_DLL if exist "%QT_PATH%\bin\libmysql.dll" set "MYSQL_CLIENT_DLL=%QT_PATH%\bin\libmysql.dll"
if not defined MYSQL_CLIENT_DLL if exist "%QT_PATH%\bin\libmariadb.dll" set "MYSQL_CLIENT_DLL=%QT_PATH%\bin\libmariadb.dll"
if not defined MYSQL_CLIENT_DLL call :die "Nu am găsit libmysql.dll/libmariadb.dll. Setează MYSQL_CLIENT_DLL." || exit /b 1
call :require_file "%MYSQL_CLIENT_DLL%" "Clientul MySQL/MariaDB indicat nu există." || exit /b 1
rem The supplied MySQL client uses shared OpenSSL even when USG links it statically.
for %%F in ("%MYSQL_CLIENT_DLL%") do set "MYSQL_CLIENT_DIR=%%~dpF"
if not defined OPENSSL_BIN if exist "%MYSQL_CLIENT_DIR%libssl-3-x64.dll" if exist "%MYSQL_CLIENT_DIR%libcrypto-3-x64.dll" set "OPENSSL_BIN=%MYSQL_CLIENT_DIR%"
if defined OPENSSL_BIN if not exist "%OPENSSL_BIN%\*.dll" call :die "Nu există DLL-uri OpenSSL în OPENSSL_BIN." || exit /b 1

echo [INFO] Proiect: %PROJECT_PATH%
echo [INFO] Versiune: %VERSION%
echo [INFO] Qt: %QT_PATH%
echo [INFO] Executabil: %BUILD_EXE%
if /I "%~1"=="--check" exit /b 0

rem Pregătirea runtime-ului.
if exist "%PREBUILD_PATH%" rd /s /q "%PREBUILD_PATH%"
if errorlevel 1 call :die "Nu s-a putut elimina directorul prebuild." || exit /b 1
mkdir "%PREBUILD_PATH%"
if errorlevel 1 call :die "Nu s-a putut crea directorul prebuild." || exit /b 1

copy /Y "%BUILD_EXE%" "%PREBUILD_PATH%\USG.exe" >nul
if errorlevel 1 call :die "Copierea USG.exe a eșuat." || exit /b 1
call :copy_required "%LIMEREPORT_RELEASE%\limereport.dll" "%PREBUILD_PATH%\limereport.dll" || exit /b 1
call :copy_required "%LIMEREPORT_RELEASE%\QtZint.dll" "%PREBUILD_PATH%\QtZint.dll" || exit /b 1

for %%F in (Qt6OpenGLWidgets.dll Qt6UiTools.dll Qt6Designer.dll Qt6DesignerComponents.dll Qt6Xml.dll) do call :copy_required "%QT_PATH%\bin\%%F" "%PREBUILD_PATH%\%%F" || exit /b 1
if defined OPENSSL_BIN for %%F in ("%OPENSSL_BIN%\libssl*.dll" "%OPENSSL_BIN%\libcrypto*.dll") do if exist "%%~fF" call :copy_required "%%~fF" "%PREBUILD_PATH%\%%~nxF" || exit /b 1
for %%F in ("%MYSQL_CLIENT_DLL%") do copy /Y "%%~fF" "%PREBUILD_PATH%\%%~nxF" >nul
if errorlevel 1 call :die "Copierea clientului MySQL/MariaDB a eșuat." || exit /b 1
if defined MYSQL_RUNTIME_DIR for %%F in ("%MYSQL_RUNTIME_DIR%\*.dll") do call :copy_required "%%~fF" "%PREBUILD_PATH%\%%~nxF" || exit /b 1

rem LimeReport trebuie să fie deja lângă executabil pentru scanarea dependențelor.
"%WDEPLOY%" --release --compiler-runtime --verbose 1 "%PREBUILD_PATH%\USG.exe"
if errorlevel 1 call :die "windeployqt a eșuat." || exit /b 1
rem Qt TLS is separate from OpenSSL statically linked into USG.
if not exist "%PREBUILD_PATH%\tls" mkdir "%PREBUILD_PATH%\tls"
call :copy_required "%QT_PATH%\plugins\tls\qschannelbackend.dll" "%PREBUILD_PATH%\tls\qschannelbackend.dll" || exit /b 1

if not exist "%PREBUILD_PATH%\sqldrivers" mkdir "%PREBUILD_PATH%\sqldrivers"
call :copy_required "%QT_SQLDRIVERS%\qsqlite.dll" "%PREBUILD_PATH%\sqldrivers\qsqlite.dll" || exit /b 1
call :copy_required "%QT_SQLDRIVERS%\qsqlmysql.dll" "%PREBUILD_PATH%\sqldrivers\qsqlmysql.dll" || exit /b 1

xcopy "%PROJECT_PATH%\resources\icons" "%PREBUILD_PATH%\icons\" /E /I /Y >nul
if errorlevel 1 call :die "Copierea iconurilor a eșuat." || exit /b 1
xcopy "%PROJECT_PATH%\resources\templets" "%PREBUILD_PATH%\templets\" /E /I /Y >nul
if errorlevel 1 call :die "Copierea șabloanelor LimeReport a eșuat." || exit /b 1

rem Documentație și licențe.
if defined CI_DEPENDENCY_LICENSES (
    xcopy "%CI_DEPENDENCY_LICENSES%" "%PREBUILD_PATH%\licenses\dependencies\" /E /I /Y >nul
    if errorlevel 1 exit /b 1
)
mkdir "%PREBUILD_PATH%\licenses\LimeReport" >nul 2>&1
call :copy_required "%PROJECT_PATH%\LICENSE.txt" "%PREBUILD_PATH%\licenses\LICENSE.txt" || exit /b 1
call :copy_required "%PROJECT_PATH%\README.md" "%PREBUILD_PATH%\README.md" || exit /b 1
call :copy_required "%PROJECT_PATH%\README-RO.md" "%PREBUILD_PATH%\README-RO.md" || exit /b 1
call :copy_required "%PROJECT_PATH%\third_party\THIRD_PARTY_ICONS.md" "%PREBUILD_PATH%\licenses\THIRD_PARTY_ICONS.md" || exit /b 1
call :copy_required "%PROJECT_PATH%\third_party\LIMEREPORT.md" "%PREBUILD_PATH%\licenses\LimeReport\README.md" || exit /b 1
call :copy_required "%LIMEREPORT_SOURCE_DIR%\LICENSE" "%PREBUILD_PATH%\licenses\LimeReport\LICENSE" || exit /b 1
call :copy_required "%LIMEREPORT_SOURCE_DIR%\COPYING" "%PREBUILD_PATH%\licenses\LimeReport\COPYING" || exit /b 1
call :copy_required "%PROJECT_PATH%\patches\limereport\1.7.23\0001-make-singleton-destruction-idempotent.patch" "%PREBUILD_PATH%\licenses\LimeReport\0001-make-singleton-destruction-idempotent.patch" || exit /b 1

rem Structura Qt Installer Framework.
if exist "%INSTALLER_PATH%" rd /s /q "%INSTALLER_PATH%"
if errorlevel 1 call :die "Nu s-a putut elimina structura installer temporară." || exit /b 1
xcopy "%PROJECT_PATH%\installer\linux" "%INSTALLER_PATH%\" /E /I /Y >nul
if errorlevel 1 call :die "Copierea șablonului installer a eșuat." || exit /b 1
xcopy "%PREBUILD_PATH%" "%INSTALLER_PATH%\packages\com.alovada.usg\data\" /E /I /Y >nul
if errorlevel 1 call :die "Copierea fișierelor runtime în installer a eșuat." || exit /b 1

set "CONFIGXML_PATH=%INSTALLER_PATH%\config\config.xml"
set "PACKAGEXML_PATH=%INSTALLER_PATH%\packages\com.alovada.usg\meta\package.xml"
set "SCRIPT_PATH=%INSTALLER_PATH%\packages\com.alovada.usg\meta\installscript.qs"

powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $p='%CONFIGXML_PATH%'; $x=New-Object System.Xml.XmlDocument; $x.Load($p); $x.Installer.Version='%VERSION%'; $x.Installer.StartMenuDir='USG-Evidența examinărilor ecografice v%VERSION%'; $x.Installer.TargetDir='@RootDir@/USG'; $x.Save($p)"
if errorlevel 1 call :die "Actualizarea config.xml a eșuat." || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference='Stop'; $p='%PACKAGEXML_PATH%'; $x=New-Object System.Xml.XmlDocument; $x.Load($p); $x.Package.Version='%VERSION%'; $x.Package.SelectSingleNode('ReleaseDate').InnerText=[DateTime]::Now.ToString('yyyy-MM-dd', [Globalization.CultureInfo]::InvariantCulture); $x.Package.DisplayName='USG - Evidența examinărilor ecografice v%VERSION%'; $x.Package.Description='Instalarea aplicației USG v%VERSION% pentru gestionarea pacienților și rapoartelor ecografice.'; $x.Save($p)"
if errorlevel 1 call :die "Actualizarea package.xml a eșuat." || exit /b 1

rem Shortcut-uri Windows. MSVC runtime este distribuit de --compiler-runtime.
>"%SCRIPT_PATH%" echo function Component^() {}
>>"%SCRIPT_PATH%" echo.
>>"%SCRIPT_PATH%" echo Component.prototype.createOperations = function^() {
>>"%SCRIPT_PATH%" echo     component.createOperations^();
>>"%SCRIPT_PATH%" echo     var desktopFile = "@HomeDir@/Desktop/USG - Evidența investigațiilor ecografice.lnk";
>>"%SCRIPT_PATH%" echo     var startMenuFile = "@StartMenuDir@/USG.lnk";
>>"%SCRIPT_PATH%" echo     var exePath = "@TargetDir@/USG.exe";
>>"%SCRIPT_PATH%" echo     var iconPath = "@TargetDir@/icons/eco_248x248.ico";
>>"%SCRIPT_PATH%" echo     component.addOperation^("CreateShortcut", exePath, desktopFile, "workingDirectory=@TargetDir@", "iconPath=" + iconPath, "description=USG - Evidența investigațiilor ecografice"^);
>>"%SCRIPT_PATH%" echo     component.addOperation^("CreateShortcut", exePath, startMenuFile, "workingDirectory=@TargetDir@", "iconPath=" + iconPath, "description=USG - Evidența investigațiilor ecografice"^);
>>"%SCRIPT_PATH%" echo }

rem Pachet și checksum.
set "PACKAGE_FILE=%BUILD_PATH%\USG_v%VERSION%_Windows_amd64.exe"
set "SHA_FILE=%PACKAGE_FILE%.sha256"
if exist "%PACKAGE_FILE%" del /f /q "%PACKAGE_FILE%"
if exist "%SHA_FILE%" del /f /q "%SHA_FILE%"
"%BINARYCREATOR%" -c "%CONFIGXML_PATH%" -p "%INSTALLER_PATH%\packages" "%PACKAGE_FILE%"
if errorlevel 1 call :die "binarycreator a eșuat." || exit /b 1
call :require_file "%PACKAGE_FILE%" "Pachetul Windows nu a fost creat." || exit /b 1

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0write_sha256.ps1" -InputPath "%PACKAGE_FILE%" -OutputPath "%SHA_FILE%"
if errorlevel 1 call :die "Calcularea SHA-256 a eșuat." || exit /b 1

rem Sursa corespunzătoare LimeReport, ca artefact separat.
if exist "%BUILD_PATH%\LimeReport_v1.7.23_USG_source.zip" del /f /q "%BUILD_PATH%\LimeReport_v1.7.23_USG_source.zip"
"%ZIP_EXE%" a -tzip "%BUILD_PATH%\LimeReport_v1.7.23_USG_source.zip" "%LIMEREPORT_SOURCE_DIR%\*" -xr!.git -xr!build >nul
if errorlevel 1 call :die "Arhivarea sursei LimeReport a eșuat." || exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0write_sha256.ps1" -InputPath "%BUILD_PATH%\LimeReport_v1.7.23_USG_source.zip" -OutputPath "%BUILD_PATH%\LimeReport_v1.7.23_USG_source.zip.sha256"
if errorlevel 1 call :die "Calcularea SHA-256 pentru sursa LimeReport a eșuat." || exit /b 1

echo [OK] Pachet creat: %PACKAGE_FILE%
echo [OK] SHA-256: %SHA_FILE%
exit /b 0

:copy_required
if not exist "%~1" call :die "Lipsește fișierul: %~1" || exit /b 1
copy /Y "%~1" "%~2" >nul
if errorlevel 1 call :die "Copiere eșuată: %~1" || exit /b 1
exit /b 0

:require_file
if not exist "%~1" call :die "%~2 [%~1]" || exit /b 1
exit /b 0

:die
echo [EROARE] %~1 1>&2
exit /b 1
