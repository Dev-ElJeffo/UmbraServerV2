<#
.SYNOPSIS
  Habilita LocalDumps (WER) para zone_server.exe — full dump em crash nativo.

.DESCRIPTION
  Configura HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\zone_server.exe
  DumpType=2 (full), DumpCount=10, pasta D:\UmbraServerV2\crash_dumps\zone.

  Preferivel: duplo clique em enable_zone_crash_dumps.bat (pede UAC e nao fecha sozinho).
  Alternativa: PowerShell elevado com este .ps1.

  Dumps NAO devem ser commitados (.gitignore).

.EXAMPLE
  .\scripts_main\enable_zone_crash_dumps.bat
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File scripts_main\enable_zone_crash_dumps.ps1
#>

$ErrorActionPreference = 'Stop'
$exitCode = 0

function Test-IsAdministrator {
  $id = [Security.Principal.WindowsIdentity]::GetCurrent()
  $principal = New-Object Security.Principal.WindowsPrincipal($id)
  return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Wait-IfInteractive {
  # O .bat ja faz pause; evita Enter duas vezes.
  if ($env:UMBRA_DUMPS_FROM_BAT -eq '1') { return }
  try {
    if ($Host.Name -eq 'ConsoleHost' -or [Environment]::UserInteractive) {
      Write-Host ''
      Read-Host 'Pressione Enter para sair' | Out-Null
    }
  } catch {
    # Host nao interativo — ignora
  }
}

try {
  Write-Host '=== LocalDumps para zone_server.exe ===' -ForegroundColor Cyan

  if (-not (Test-IsAdministrator)) {
    Write-Host ''
    Write-Host 'ERRO: este script precisa de PowerShell como Administrador.' -ForegroundColor Red
    Write-Host 'Use uma destas opcoes:' -ForegroundColor Yellow
    Write-Host '  1) Duplo clique em scripts_main\enable_zone_crash_dumps.bat (aceite o UAC)'
    Write-Host '  2) Clique direito no .bat -> Executar como administrador'
    Write-Host '  3) Abra PowerShell como Admin e rode este .ps1'
    $exitCode = 1
    exit $exitCode
  }

  $DumpFolder = 'D:\UmbraServerV2\crash_dumps\zone'
  $ExeName = 'zone_server.exe'
  $KeyPath = "HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\$ExeName"

  if (-not (Test-Path $DumpFolder)) {
    New-Item -ItemType Directory -Path $DumpFolder -Force | Out-Null
    Write-Host "Pasta criada: $DumpFolder"
  } else {
    Write-Host "Pasta OK: $DumpFolder"
  }

  if (-not (Test-Path $KeyPath)) {
    New-Item -Path $KeyPath -Force | Out-Null
  }

  New-ItemProperty -Path $KeyPath -Name 'DumpFolder' -PropertyType ExpandString -Value $DumpFolder -Force | Out-Null
  New-ItemProperty -Path $KeyPath -Name 'DumpCount' -PropertyType DWord -Value 10 -Force | Out-Null
  New-ItemProperty -Path $KeyPath -Name 'DumpType' -PropertyType DWord -Value 2 -Force | Out-Null

  Write-Host ''
  Write-Host 'Chave WER:' -ForegroundColor Green
  Get-ItemProperty -Path $KeyPath | Select-Object DumpFolder, DumpCount, DumpType | Format-List

  Write-Host 'Pronto. Em crash do zone, procure *.dmp em:' -ForegroundColor Yellow
  Write-Host "  $DumpFolder"
  Write-Host 'Preserve tambem zone_server.exe + .pdb e as ultimas linhas de logs/zone_server.log'
} catch {
  $exitCode = 1
  Write-Host ''
  Write-Host 'FALHA ao configurar LocalDumps:' -ForegroundColor Red
  Write-Host $_.Exception.Message -ForegroundColor Red
  if ($_.ScriptStackTrace) {
    Write-Host $_.ScriptStackTrace -ForegroundColor DarkGray
  }
} finally {
  Wait-IfInteractive
}

exit $exitCode
