<#
.SYNOPSIS
  Checklist de stress PvP (Templar x Martial) apos correcoes de crash do zone.

.DESCRIPTION
  Nao automatiza o combate in-game — guia o operador apos rebuild do zone_server.
  Skills sugeridas: 31/33/37/47/49/51/53/54/60/92/96.

.EXAMPLE
  powershell -File scripts_main\stress_zone_pvp_combat.ps1
#>

$ErrorActionPreference = 'Continue'
$LogPath = 'D:\UmbraServerV2\build\bin\Release\logs\zone_server.log'
$DumpDir = 'D:\UmbraServerV2\crash_dumps\zone'
$WerKey = 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\zone_server.exe'

Write-Host '=== Stress Zone PvP — checklist ===' -ForegroundColor Cyan
Write-Host ''
Write-Host '[1] LocalDumps habilitados?'
if (Test-Path $WerKey) {
  Get-ItemProperty $WerKey | Select-Object DumpFolder, DumpCount, DumpType | Format-List
} else {
  Write-Host '  FALTA: rode scripts_main\enable_zone_crash_dumps.ps1 como Admin' -ForegroundColor Red
}

Write-Host '[2] ENUM combat_log — rode no MySQL:'
Write-Host "  SHOW COLUMNS FROM combat_log LIKE 'action_type';"
Write-Host '  Esperado: ...CRIT,DOUBLE,REACTION'

Write-Host ''
Write-Host '[3] Rebuild + restart zone_server Release'

Write-Host ''
Write-Host '[4] No Play: dois chars (Templar + Martial), 10+ minutos alternando:'
Write-Host '  skills 31,33,37,47,49,51,53,54,60,92,96'

Write-Host ''
Write-Host '[5] Apos o stress, conferir log:'
if (Test-Path $LogPath) {
  $trunc = @(Select-String -Path $LogPath -Pattern 'Data truncated for column .action_type.' -ErrorAction SilentlyContinue)
  $pool = @(Select-String -Path $LogPath -Pattern 'Pool exhausted' -ErrorAction SilentlyContinue)
  Write-Host ("  Data truncated action_type: {0}" -f $trunc.Count)
  Write-Host ("  Pool exhausted: {0}" -f $pool.Count)
  Write-Host ("  Log: {0}" -f $LogPath)
} else {
  Write-Host ("  Log ainda nao existe: {0}" -f $LogPath) -ForegroundColor Yellow
}

Write-Host ''
Write-Host '[6] Dumps (se crashou):'
if (Test-Path $DumpDir) {
  Get-ChildItem $DumpDir -Filter '*.dmp' -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 5 Name, Length, LastWriteTime
} else {
  Write-Host ("  Pasta dumps ausente: {0}" -f $DumpDir)
}

Write-Host ''
Write-Host 'Criterios de sucesso: zero Data truncated; zone estavel 10+ min; sem dump novo.' -ForegroundColor Green
