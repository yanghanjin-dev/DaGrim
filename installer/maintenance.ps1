param([switch]$Uninstall, [int]$ParentPid = 0, [string]$SetupPath, [string]$Version)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
$root = [Environment]::GetFolderPath('ProgramFiles')
if ($env:ProgramW6432) { $root = $env:ProgramW6432 }
$target = Join-Path $root 'DaGrim'
$app = Join-Path $target 'DaGrim.exe'
$setup = Join-Path $target 'DaGrimSetup.exe'
$script = Join-Path $target 'maintenance.ps1'
$menu = Join-Path ([Environment]::GetFolderPath('CommonPrograms')) 'DaGrim.lnk'
$key = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\DaGrim'
function Confirm-Action($text) {
    return [System.Windows.Forms.MessageBox]::Show($text, 'DaGrim', 'YesNo', 'Question') -eq 'Yes'
}
function Assert-Unlocked($path) {
    if (Test-Path -LiteralPath $path) {
        $handle = [IO.File]::Open($path, 'Open', 'ReadWrite', 'None')
        $handle.Dispose()
    }
}
try {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw '관리자 권한이 필요합니다. DaGrimSetup.exe를 실행하세요.'
    }
    if ($Uninstall) {
        if ($ParentPid) { Wait-Process -Id $ParentPid -ErrorAction SilentlyContinue }
        if (!(Confirm-Action 'DaGrim을 제거하시겠습니까? 사용자가 만든 문서는 삭제하지 않습니다.')) { exit 0 }
        Assert-Unlocked $app
        Assert-Unlocked $setup
        # Only the three files owned by the installer are removed; never recurse.
        foreach ($path in @($app, $setup, $menu)) {
            if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Force }
        }
        if (Test-Path -LiteralPath $key) { Remove-Item -LiteralPath $key -Recurse }
        Remove-Item -LiteralPath $script -Force
        if (!(Get-ChildItem -LiteralPath $target -Force)) { Remove-Item -LiteralPath $target }
        [System.Windows.Forms.MessageBox]::Show('DaGrim을 제거했습니다.', 'DaGrim') | Out-Null
    } else {
        if (!(Confirm-Action "DaGrim $Version 을 모든 사용자용으로 설치합니다.`n설치 위치: $target`n계속하시겠습니까?")) { exit 0 }
        Assert-Unlocked $app
        Assert-Unlocked $setup
        New-Item -ItemType Directory -Path $target -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'DaGrim.exe') -Destination $app -Force
        Copy-Item -LiteralPath $PSCommandPath -Destination $script -Force
        if ([IO.Path]::GetFullPath($SetupPath) -ine $setup) {
            Copy-Item -LiteralPath $SetupPath -Destination $setup -Force
        }
        $shell = New-Object -ComObject WScript.Shell
        $shortcut = $shell.CreateShortcut($menu)
        $shortcut.TargetPath = $app
        $shortcut.WorkingDirectory = $target
        $shortcut.Description = 'DaGrim'
        $shortcut.Save()
        New-Item -Path $key -Force | Out-Null
        $values = @{
            DisplayName = 'DaGrim'; DisplayVersion = $Version; Publisher = 'DaGrim';
            InstallLocation = $target; DisplayIcon = $app;
            UninstallString = ('"{0}" /uninstall' -f $setup)
        }
        foreach ($name in $values.Keys) {
            New-ItemProperty -Path $key -Name $name -Value $values[$name] -PropertyType String -Force | Out-Null
        }
        foreach ($name in @('NoModify', 'NoRepair')) {
            New-ItemProperty -Path $key -Name $name -Value 1 -PropertyType DWord -Force | Out-Null
        }
        [System.Windows.Forms.MessageBox]::Show('설치가 완료되었습니다. 시작 메뉴에서 DaGrim을 실행하세요.', 'DaGrim') | Out-Null
    }
    exit 0
} catch {
    [System.Windows.Forms.MessageBox]::Show("작업을 완료하지 못했습니다. 실행 중인 DaGrim을 종료한 후 다시 시도하세요.`n$($_.Exception.Message)", 'DaGrim', 'OK', 'Error') | Out-Null
    exit 1
}
