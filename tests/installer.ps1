$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$sandbox = Join-Path $repo ('build\installer-test-' + [guid]::NewGuid().ToString('N'))
if (Test-Path -LiteralPath $sandbox) { throw 'Test directory already exists; inspect before rerunning.' }
New-Item -ItemType Directory -Path $sandbox | Out-Null
$target = Join-Path $sandbox 'installed'
$menu = Join-Path $sandbox 'DaGrim.lnk'
$key = (Join-Path $sandbox 'registry')
if (Test-Path $key) { throw 'Test registry key already exists.' }
$source = Get-Content -LiteralPath (Join-Path $repo 'installer\maintenance.ps1') -Raw
# Redirect privileged destinations and suppress interactive prompts in this test copy only.
$overrides = "`$target = '$target'`n`$app = Join-Path `$target 'DaGrim.exe'`n`$setup = Join-Path `$target 'DaGrimSetup.exe'`n`$script = Join-Path `$target 'maintenance.ps1'`n`$menu = '$menu'`n`$key = '$key'`n"
$source = $source.Replace('function Confirm-Action($text) {', $overrides + 'function Confirm-Action($text) {')
$source = $source.Replace("return [System.Windows.Forms.MessageBox]::Show(`$text, 'DaGrim', 'YesNo', 'Question') -eq 'Yes'", 'return $true')
$source = $source.Replace('if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {', 'if ($false) {')
$source = [regex]::Replace($source, '(?m)^\s*\[System.Windows.Forms.MessageBox\]::Show\([^\r\n]*', '    $null = 0')
$source = $source.Replace('New-ItemProperty -Path', 'Set-TestRegistryProperty -Path')
$source = $source.Replace('New-Item -Path $key -Force', 'New-Item -ItemType Directory -Path $key -Force')
$source = $source.Replace('} catch {', '} catch { Write-Output $_')
$source = $source.Replace('function Confirm-Action', 'function Set-TestRegistryProperty { param($Path, $Name, $Value, $PropertyType, [switch]$Force) Set-Content -LiteralPath (Join-Path $Path $Name) -Value $Value }; function Confirm-Action')
$testScript = Join-Path $sandbox 'maintenance.ps1'
Set-Content -LiteralPath $testScript -Value $source -Encoding UTF8
Copy-Item (Join-Path $repo 'build\DaGrim.exe') $sandbox
$setupSource = Join-Path $repo 'build\DaGrimSetup.exe'
function Run-Installer([string[]]$Arguments, [int]$Expected = 0) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $testScript @Arguments
    if ($LASTEXITCODE -ne $Expected) { throw "Unexpected installer exit: $LASTEXITCODE (expected $Expected)" }
}
try {
    Run-Installer @('-SetupPath', $setupSource, '-Version', 'test')
    if (!(Test-Path $menu) -or (Get-Content (Join-Path $key 'DisplayVersion')) -ne 'test') { throw 'Registration missing' }
    if ((Get-FileHash (Join-Path $target 'DaGrim.exe')).Hash -ne (Get-FileHash (Join-Path $sandbox 'DaGrim.exe')).Hash) { throw 'Payload mismatch' }
    Set-Content (Join-Path $target 'user-document.dagrim') 'keep me'
    Run-Installer @('-SetupPath', $setupSource, '-Version', 'test-update')
    if ((Get-Content (Join-Path $key 'DisplayVersion')) -ne 'test-update') { throw 'Upgrade failed' }
    $lock = [IO.File]::Open((Join-Path $target 'DaGrim.exe'), 'Open', 'Read', 'None')
    try { Run-Installer @('-Uninstall') 1 } finally { $lock.Dispose() }
    if (!(Test-Path $key) -or !(Test-Path $menu)) { throw 'Locked uninstall changed registration' }
    Run-Installer @('-Uninstall')
    if ((Test-Path $key) -or (Test-Path $menu) -or (Test-Path (Join-Path $target 'DaGrim.exe'))) { throw 'Uninstall incomplete' }
    if ((Get-Content (Join-Path $target 'user-document.dagrim')) -ne 'keep me') { throw 'User document lost' }
    'PASS: install, upgrade, locked-file rejection, uninstall, user-file preservation'
} finally {
    # Keep sandbox evidence for inspection. Only remove the dedicated test registry key.
    if (Test-Path $key) { Remove-Item -LiteralPath $key -Recurse }
}


