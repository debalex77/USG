# Run in an MSVC x64 developer environment after install-qt-action.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Invoke-Native([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe failed with exit code $LASTEXITCODE" }
}

$projectDir = Split-Path $PSScriptRoot -Parent
$buildDir = Join-Path $projectDir 'build'
$qtDir = $env:QT_ROOT_DIR
if (-not $qtDir) { throw 'QT_ROOT_DIR is required' }
$vcpkgDir = Join-Path $buildDir 'vcpkg'
Invoke-Native "$vcpkgDir/bootstrap-vcpkg.bat" @('-disableMetrics')
# Keep authentication plugins inside the client DLL, so they do not depend on
# vcpkg's build-machine plugin path. The optional ed25519 plugin is not enabled.
$overlayDir = "$buildDir/vcpkg-overlay"
New-Item -ItemType Directory -Force $overlayDir | Out-Null
Copy-Item "$vcpkgDir/ports/libmariadb" $overlayDir -Recurse -Force
$portPath = "$overlayDir/libmariadb/portfile.cmake"
$portText = [IO.File]::ReadAllText($portPath)
$portText = $portText.Replace('string(TOUPPER "${VCPKG_LIBRARY_LINKAGE}" plugin_type)', 'set(plugin_type STATIC)')
$portText = $portText.Replace('-DCLIENT_PLUGIN_CLIENT_ED25519=DYNAMIC', '-DCLIENT_PLUGIN_CLIENT_ED25519=OFF')
[IO.File]::WriteAllText($portPath, $portText)
# OpenSSL is pinned to the same 3.5.x LTS release as Linux (build_scripts/build_openssl).
# The pinned vcpkg tree ships 3.5.2; only the version and source checksum change.
$opensslVersion = '3.5.9'
$opensslSha512 = 'ab29a6ed40dd8c74420b9bd19c5ac119b0f191aee72d555ba5a0601456155d39c809c77300d98c7e0bce607f32c1f7a83fc7f69f86b9909fe557bba3aa2e468f'
Copy-Item "$vcpkgDir/ports/openssl" $overlayDir -Recurse -Force
$opensslManifest = "$overlayDir/openssl/vcpkg.json"
$manifestText = [IO.File]::ReadAllText($opensslManifest)
$pinnedManifest = [regex]::Replace($manifestText, '"version":\s*"[^"]+"', "`"version`": `"$opensslVersion`"")
if ($pinnedManifest -eq $manifestText) { throw 'Cannot pin the OpenSSL port version' }
[IO.File]::WriteAllText($opensslManifest, $pinnedManifest)
$opensslPortfile = "$overlayDir/openssl/portfile.cmake"
$portfileText = [IO.File]::ReadAllText($opensslPortfile)
$pinnedPortfile = [regex]::Replace($portfileText, 'SHA512\s+[0-9a-fA-F]{128}', "SHA512 $opensslSha512")
if ($pinnedPortfile -eq $portfileText) { throw 'Cannot pin the OpenSSL source checksum' }
[IO.File]::WriteAllText($opensslPortfile, $pinnedPortfile)
Invoke-Native "$vcpkgDir/vcpkg.exe" @('install', 'openssl:x64-windows-static-md', 'libmariadb[core,schannel]:x64-windows', '--classic', "--overlay-ports=$overlayDir")
$staticDir = "$vcpkgDir/installed/x64-windows-static-md"
$clientDir = "$vcpkgDir/installed/x64-windows"
$opensslHeader = [IO.File]::ReadAllText("$staticDir/include/openssl/opensslv.h")
if ($opensslHeader -notmatch "OPENSSL_VERSION_TEXT\s+`"OpenSSL $([regex]::Escape($opensslVersion)) ") {
    throw "vcpkg did not install OpenSSL $opensslVersion"
}
New-Item -ItemType Directory -Force "$projectDir/3rdparty/openssl/include", "$projectDir/3rdparty/openssl/lib" | Out-Null
Copy-Item "$staticDir/include/openssl" "$projectDir/3rdparty/openssl/include" -Recurse -Force
Copy-Item "$staticDir/lib/libssl.lib", "$staticDir/lib/libcrypto.lib" "$projectDir/3rdparty/openssl/lib"

# QSQLCIPHER embeds SQLCipher and links the same static OpenSSL as USG.
$env:OPENSSL_ROOT_DIR = [IO.Path]::GetFullPath("$projectDir/3rdparty/openssl")
$env:INSTALL = '1'
$env:QSQLCIPHER_BUILD_JOBS = '2'
try {
    Invoke-Native pwsh @('-NoProfile', '-File', "$projectDir/third_party/qsqlcipher/source/build.ps1", $qtDir)
} finally {
    Remove-Item Env:INSTALL, Env:QSQLCIPHER_BUILD_JOBS -ErrorAction SilentlyContinue
}
if (-not (Test-Path "$qtDir/plugins/sqldrivers/qsqlcipher.dll")) { throw 'QSQLCIPHER plugin was not installed' }

# The Qt SQL plugin must be compiled against the same Qt version as USG.
$qtSource = "$buildDir/qtbase"
Invoke-Native git @('clone', '--depth=1', '--branch', "v$env:QT_VERSION", 'https://github.com/qt/qtbase.git', $qtSource)
$sqlBuild = "$buildDir/sql-driver"
Invoke-Native "$qtDir/bin/qt-cmake.bat" @('-S', "$qtSource/src/plugins/sqldrivers", '-B', $sqlBuild, '-G', 'Ninja',
    '-DCMAKE_BUILD_TYPE=Release', "-DCMAKE_INSTALL_PREFIX=$qtDir", '-DFEATURE_sql_mysql=ON',
    "-DMySQL_INCLUDE_DIR=$clientDir/include/mysql", "-DMySQL_LIBRARY=$clientDir/lib/libmariadb.lib")
Invoke-Native cmake @('--build', $sqlBuild, '--parallel', '2')
Invoke-Native cmake @('--install', $sqlBuild)
Copy-Item "$clientDir/bin/*.dll" "$qtDir/bin" -Force

$limeSource = "$buildDir/LimeReport-source"
Invoke-Native git @('init', $limeSource)
Invoke-Native git @('-C', $limeSource, 'remote', 'add', 'origin', 'https://github.com/fralx/LimeReport.git')
Invoke-Native git @('-C', $limeSource, 'fetch', '--depth=1', 'origin', $env:LIMEREPORT_COMMIT)
Invoke-Native git @('-C', $limeSource, 'checkout', '--detach', 'FETCH_HEAD')
Invoke-Native git @('-C', $limeSource, 'apply', "$projectDir/patches/limereport/1.7.23/0001-make-singleton-destruction-idempotent.patch")
$limeBuild = "$buildDir/LimeReport-build"
New-Item -ItemType Directory -Force $limeBuild | Out-Null
Push-Location $limeBuild
try {
    Invoke-Native "$qtDir/bin/qmake.exe" @("$limeSource/limereport.pro", 'CONFIG+=release', 'CONFIG-=debug_and_release', 'CONFIG+=no_build_translations')
    Invoke-Native nmake @('/NOLOGO', 'sub-limereport-all-ordered')
} finally { Pop-Location }
$limeLib = "$limeSource/build/$env:QT_VERSION/win64/release/lib"
New-Item -ItemType Directory -Force "$projectDir/3rdparty/LimeReport/include", "$projectDir/3rdparty/LimeReport/release" | Out-Null
Copy-Item "$limeSource/include/*" "$projectDir/3rdparty/LimeReport/include" -Recurse -Force
foreach ($name in @('limereport.lib', 'limereport.dll', 'QtZint.lib', 'QtZint.dll')) {
    Copy-Item "$limeLib/$name" "$projectDir/3rdparty/LimeReport/release/$name"
}

$usgBuild = "$buildDir/windows-ci"
New-Item -ItemType Directory -Force $usgBuild | Out-Null
Push-Location $usgBuild
try {
    Invoke-Native "$qtDir/bin/qmake.exe" @("$projectDir/USG.pro", 'CONFIG+=release')
    Invoke-Native nmake @('/NOLOGO', 'release')
} finally { Pop-Location }

# The legacy aqt tools_ifw feed only provides 4.7. Use Qt's versioned 4.11 feed.
$ifwUrl = 'https://download.qt.io/online/qtsdkrepository/windows_x86/ifw/tools_ifw_411/qt.tools.ifw.411/4.11.0-0-202603231357ifw-win-x64.7z'
$ifwArchive = "$buildDir/ifw.7z"
Invoke-WebRequest $ifwUrl -OutFile $ifwArchive
Invoke-WebRequest "$ifwUrl.sha1" -OutFile "$ifwArchive.sha1"
$expectedHash = (Get-Content "$ifwArchive.sha1" -Raw).Trim()
if ($expectedHash -notmatch '^[a-fA-F0-9]{40}$' -or (Get-FileHash $ifwArchive -Algorithm SHA1).Hash -ne $expectedHash) {
    throw 'Qt Installer Framework archive checksum mismatch'
}
Invoke-Native 'C:/Program Files/7-Zip/7z.exe' @('x', $ifwArchive, "-o$buildDir/ifw", '-y')
$env:QT_PATH = $qtDir
$env:QIF_PATH = "$buildDir/ifw/bin"
$env:BUILD_EXE = "$usgBuild/release/USG.exe"
$env:LIMEREPORT_SOURCE_DIR = $limeSource
$env:MYSQL_CLIENT_DLL = "$clientDir/bin/libmariadb.dll"
$env:MYSQL_RUNTIME_DIR = "$clientDir/bin"
$env:CI_DEPENDENCY_LICENSES = "$buildDir/dependency-licenses"
New-Item -ItemType Directory -Force $env:CI_DEPENDENCY_LICENSES | Out-Null
foreach ($entry in @(@($staticDir, 'openssl'), @($clientDir, 'libmariadb'), @($clientDir, 'zlib'))) {
    Copy-Item "$($entry[0])/share/$($entry[1])/copyright" "$env:CI_DEPENDENCY_LICENSES/$($entry[1]).txt"
}
# cmd.exe COPY/XCOPY do not reliably accept the mixed separators used above.
# Normalize every path crossing the PowerShell -> batch boundary.
foreach ($name in @('QT_PATH', 'QIF_PATH', 'BUILD_EXE', 'LIMEREPORT_SOURCE_DIR',
                    'MYSQL_CLIENT_DLL', 'MYSQL_RUNTIME_DIR', 'CI_DEPENDENCY_LICENSES')) {
    $value = [Environment]::GetEnvironmentVariable($name)
    [Environment]::SetEnvironmentVariable($name, [IO.Path]::GetFullPath($value).Replace('/', '\'))
}
Invoke-Native "$PSScriptRoot/build_win.bat" @('--check')
Invoke-Native "$PSScriptRoot/build_win.bat" @()

# Verify deployed plugins without opening USG or connecting to a database.
$smokeBuild = "$buildDir/runtime-check"
New-Item -ItemType Directory -Force $smokeBuild | Out-Null
Push-Location $smokeBuild
try {
    Invoke-Native "$qtDir/bin/qmake.exe" @("$PSScriptRoot/ci/windows_runtime_check.pro")
    Invoke-Native nmake @('/NOLOGO')
} finally { Pop-Location }
$smokeExe = Get-ChildItem $smokeBuild -Recurse -Filter windows_runtime_check.exe | Select-Object -First 1
if (-not $smokeExe) { throw 'Runtime check executable was not built' }
Copy-Item $smokeExe.FullName "$buildDir/prebuild_windows/windows_runtime_check.exe"
$savedPath = $env:PATH
try {
    $env:PATH = "$env:SystemRoot/system32;$env:SystemRoot"
    Invoke-Native "$buildDir/prebuild_windows/windows_runtime_check.exe" @()
} finally { $env:PATH = $savedPath }
