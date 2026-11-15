# Copyright © 2013-2025 Tim Angus
# Copyright © 2013-2025 Tom Freeman
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.

#Requires -Version 7.4

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $true

$BUILD_DIR = "build"
$INSTALLER_DIR = "installer"

. ".\$BUILD_DIR\variables.ps1"

$INSTALLER_EXE = "$INSTALLER_DIR\${PRODUCT_NAME}-${VERSION}-installer.exe"
$TEST_INSTALL_DIR = Join-Path ([System.IO.Path]::GetTempPath()) "${PRODUCT_NAME}-installer-test"
$STARTUP_TIMEOUT_SECONDS = 120

function Invoke-NsisExecutable($Executable, $Arguments)
{
    $process = Start-Process -FilePath $Executable -ArgumentList $Arguments -NoNewWindow -Wait -PassThru
    if($process.ExitCode -ne 0)
    {
        throw "$Executable failed with exit code $($process.ExitCode)"
    }
}

# Apparently something may briefly hold a freshly run executable open, so we need to retry
function Remove-ItemWithRetry($Path)
{
    for($attempt = 1; ; $attempt++)
    {
        try
        {
            Remove-Item $Path -Recurse -Force
            return
        }
        catch
        {
            if($attempt -ge 10) { throw }

            Write-Host "Failed to remove ${Path} (attempt ${attempt}): $($_.Exception.Message)"
            Start-Sleep -Seconds 1
        }
    }
}

# Includes anything the executable spawns, e.g. QtWebEngineProcess or CrashReporter
function Stop-InstalledProcesses
{
    Get-Process | Where-Object { $_.Path -like "$TEST_INSTALL_DIR\*" } | ForEach-Object {
        $_.Kill($true)
        $_.WaitForExit()
    }
}

# Returns $null if the executable reaches its main window, otherwise the reason it didn't
function Test-Startup($Executable)
{
    $stdoutFile = New-TemporaryFile
    $stderrFile = New-TemporaryFile

    $process = Start-Process -FilePath $Executable -ArgumentList "--dontUpdate", "--skipWelcome" -PassThru `
        -RedirectStandardOutput $stdoutFile -RedirectStandardError $stderrFile
    $null = $process.Handle # Without this, ExitCode is unavailable once the process has exited

    try
    {
        $deadline = (Get-Date).AddSeconds($STARTUP_TIMEOUT_SECONDS)
        while((Get-Date) -lt $deadline -and -not $process.HasExited)
        {
            $process.Refresh()

            # Error message boxes are also windows, so check the title rather than just its presence;
            # the main window's is the product name, after the open tab's (e.g. the first run tutorial)
            if($process.MainWindowTitle -match "^(.+ - )?$([regex]::Escape($PRODUCT_NAME))$") { return $null }

            Start-Sleep -Milliseconds 500
        }

        if($process.HasExited)
        {
            # e.g. 0xC0000135 means a DLL couldn't be found
            $reason = "exited with code 0x{0:X8}" -f $process.ExitCode
        }
        elseif($process.MainWindowTitle)
        {
            $reason = "showed a window titled `"$($process.MainWindowTitle)`" instead of its main window"
        }
        else
        {
            $reason = "showed no main window within ${STARTUP_TIMEOUT_SECONDS}s"
        }

        $output = (Get-Content $stdoutFile, $stderrFile -Raw) -join "`n"
        if($output.Trim()) { $reason += "; its output was:`n$output" }

        return $reason
    }
    finally
    {
        Stop-InstalledProcesses
        Remove-Item $stdoutFile, $stderrFile -Force
    }
}

# Rather than a dialog, make a missing DLL terminate the process; child processes inherit this
Add-Type -Namespace Win32 -Name ErrorMode -MemberDefinition `
    '[DllImport("kernel32.dll")] public static extern uint SetErrorMode(uint mode);'
$SEM_FAILCRITICALERRORS = 0x0001
[Win32.ErrorMode]::SetErrorMode($SEM_FAILCRITICALERRORS) | Out-Null

if(Test-Path $TEST_INSTALL_DIR) { Remove-ItemWithRetry $TEST_INSTALL_DIR }

Write-Host "------ installing ${INSTALLER_EXE} to ${TEST_INSTALL_DIR}"
Invoke-NsisExecutable $INSTALLER_EXE "/S /D=$TEST_INSTALL_DIR"

Write-Host "------ starting ${PRODUCT_NAME}"
$STARTUP_FAILURE = Test-Startup "$TEST_INSTALL_DIR\${PRODUCT_NAME}.exe"

Write-Host "------ uninstalling"
# _?= stops the uninstaller copying itself to a temporary location and returning immediately,
# but it also means it can't delete itself, hence the explicit removal of the directory
Invoke-NsisExecutable "$TEST_INSTALL_DIR\Uninstall.exe" "/S _?=$TEST_INSTALL_DIR"
Remove-ItemWithRetry $TEST_INSTALL_DIR

if($STARTUP_FAILURE)
{
    throw "${PRODUCT_NAME} failed to start: $STARTUP_FAILURE"
}

Write-Host "${PRODUCT_NAME} started successfully"
