# Compilează SQLCipher + pluginul QSQLCIPHER pentru Qt 6.9.3 (Windows, MSVC 2022 x64)
#
# Se rulează cu PowerShell 7 (pwsh) într-un mediu x64 Native Tools
# (cl, nmake, cmake, ninja, dumpbin în PATH).
#
# Utilizare:
#   ./build.ps1 [cale_kit_Qt]           implicit: C:\Qt\6.9.3\msvc2022_64
#
# Variabile opționale:
#   SQLCIPHER_TAG=v4.19.0    versiunea SQLCipher
#   SQLCIPHER_COMMIT=...      commitul exact corespunzător tagului
#   OPENSSL_ROOT_DIR=<dir>    OpenSSL static (/MD): include\openssl, lib\libcrypto.lib
#   INSTALL=1                 instalează pluginul în <kit>\plugins\sqldrivers
#   QSQLCIPHER_BUILD_JOBS=<număr> numărul de procese paralele (implicit: 75% CPU)
param([string]$QtDir = 'C:\Qt\6.9.3\msvc2022_64')

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Invoke-Native([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe a eșuat cu codul $LASTEXITCODE" }
}

$sqlcipherTag = if ($env:SQLCIPHER_TAG) { $env:SQLCIPHER_TAG } else { 'v4.19.0' }
$sqlcipherCommit = if ($env:SQLCIPHER_COMMIT) { $env:SQLCIPHER_COMMIT } else { 'c4b275a47932888216bade83aff2bbc73df0ff85' }
$opensslRoot = $env:OPENSSL_ROOT_DIR
$root = $PSScriptRoot
$work = Join-Path $root '_deps'
$build = Join-Path $root 'build'
$jobs = if ($env:QSQLCIPHER_BUILD_JOBS) { $env:QSQLCIPHER_BUILD_JOBS } else {
    [Math]::Max(1, [int][Math]::Ceiling([Environment]::ProcessorCount * 3 / 4))
}

$qtCmake = Join-Path $QtDir 'bin\qt-cmake.bat'
if (-not (Test-Path $qtCmake)) { throw "Nu găsesc $qtCmake" }
foreach ($tool in @('git', 'cl', 'nmake', 'cmake', 'ninja', 'dumpbin')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "Lipsește: $tool (rulează din x64 Native Tools Command Prompt for VS 2022)"
    }
}
if (-not $opensslRoot) { throw 'Setează OPENSSL_ROOT_DIR (OpenSSL static, /MD).' }
if (-not (Test-Path (Join-Path $opensslRoot 'include\openssl\evp.h'))) {
    throw "Nu găsesc headerele OpenSSL în $opensslRoot\include"
}
if (-not (Test-Path (Join-Path $opensslRoot 'lib\libcrypto.lib'))) {
    throw "Nu găsesc $opensslRoot\lib\libcrypto.lib"
}

# 1. SQLCipher -> amalgamare sqlite3.c / sqlite3.h
# Makefile.msc folosește jimsh inclus în sursă, deci nu este nevoie de Tcl.
New-Item -ItemType Directory -Force $work | Out-Null
$sqlcipherDir = Join-Path $work 'sqlcipher'
if (-not (Test-Path $sqlcipherDir)) {
    Invoke-Native git @('clone', '--depth', '1', '--branch', $sqlcipherTag,
                        'https://github.com/sqlcipher/sqlcipher.git', $sqlcipherDir)
}
$actualCommit = (& git -C $sqlcipherDir rev-parse HEAD).Trim()
if ($actualCommit -ne $sqlcipherCommit) {
    throw "Commit SQLCipher neașteptat: $actualCommit (așteptat: $sqlcipherCommit)"
}
if (-not (Test-Path (Join-Path $sqlcipherDir 'sqlite3.c'))) {
    Push-Location $sqlcipherDir
    try {
        Invoke-Native nmake @('/NOLOGO', '/f', 'Makefile.msc', 'sqlite3.c')
    } finally { Pop-Location }
}

# 2. Pluginul (OpenSSL static, CRT /MD ca Qt)
Invoke-Native $qtCmake @('-S', $root, '-B', $build, '-G', 'Ninja',
    '-DCMAKE_BUILD_TYPE=Release',
    "-DSQLCIPHER_AMALGAMATION_DIR=$sqlcipherDir",
    "-DOPENSSL_ROOT_DIR=$opensslRoot",
    '-DOPENSSL_USE_STATIC_LIBS=TRUE',
    '-DOPENSSL_MSVC_STATIC_RT=FALSE')
Invoke-Native cmake @('--build', $build, '--parallel', "$jobs")

$plugin = Join-Path $build 'plugins\sqldrivers\qsqlcipher.dll'
if (-not (Test-Path $plugin)) { throw "Pluginul nu a fost creat: $plugin" }

# OpenSSL trebuie să fie inclus static: fără libcrypto/libssl DLL.
$dependents = & dumpbin /NOLOGO /DEPENDENTS $plugin
if ($LASTEXITCODE -ne 0) { throw 'dumpbin a eșuat' }
if ($dependents -match '(?i)lib(crypto|ssl)[^ ]*\.dll') {
    $dependents | Write-Output
    throw 'qsqlcipher.dll depinde de un DLL OpenSSL; se așteaptă legare statică.'
}

# 3. Test (rulează pluginul direct din directorul de build)
$savedPath = $env:PATH
try {
    $env:PATH = "$(Join-Path $QtDir 'bin');$env:PATH"
    Push-Location $build
    try {
        Invoke-Native (Join-Path $build 'qsqlcipher_test.exe') @()
    } finally { Pop-Location }
} finally { $env:PATH = $savedPath }

# 4. Instalare
if ($env:INSTALL -eq '1') {
    Invoke-Native cmake @('--install', $build)
    $installed = Join-Path $QtDir 'plugins\sqldrivers\qsqlcipher.dll'
    if (-not (Test-Path $installed)) { throw "Instalarea nu a creat $installed" }
    Write-Output "Instalat în: $installed"
} else {
    Write-Output "Plugin: $plugin"
    Write-Output "Pentru instalare în kit:  `$env:INSTALL='1'; ./build.ps1 $QtDir"
}
