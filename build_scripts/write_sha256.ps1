param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath
)

$ErrorActionPreference = 'Stop'
$stream = $null
$algorithm = $null
try {
    $stream = [IO.File]::OpenRead($InputPath)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    $hash = [BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-', '').ToLowerInvariant()
    $line = $hash + '  ' + [IO.Path]::GetFileName($InputPath) + "`n"
    [IO.File]::WriteAllText($OutputPath, $line, [Text.UTF8Encoding]::new($false))
} finally {
    if ($stream) { $stream.Dispose() }
    if ($algorithm) { $algorithm.Dispose() }
}
