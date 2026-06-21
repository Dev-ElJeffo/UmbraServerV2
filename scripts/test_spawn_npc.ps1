# Testa spawn NPC via API admin (Windows / PowerShell)
# Uso: .\scripts\test_spawn_npc.ps1 -AdminUsername "SEU_USER_ADMIN"
param(
    [Parameter(Mandatory = $false)]
    [string]$AdminUsername = "",

    [int]$TemplateId = 1,
    [int]$ZoneId = 1,
    [double]$PosX = 100,
    [double]$PosY = 0,
    [double]$PosZ = 200,
    [double]$Yaw = 0,

    [string]$ApiBase = "http://localhost/umbra_api/api"
)

function Invoke-UmbraAdmin {
    param([string]$Path, [hashtable]$Body)
    $Body["admin_username"] = $AdminUsername
    $json = $Body | ConvertTo-Json
    $url = "$ApiBase$Path"
    Write-Host "`n>>> POST $url" -ForegroundColor Cyan
    Write-Host $json -ForegroundColor DarkGray
    try {
        $resp = Invoke-WebRequest -Uri $url -Method POST `
            -ContentType "application/json; charset=utf-8" `
            -Body ([System.Text.Encoding]::UTF8.GetBytes($json)) `
            -UseBasicParsing
        $text = $resp.Content
        Write-Host "HTTP $($resp.StatusCode)" -ForegroundColor Green
        Write-Host $text
        return $text | ConvertFrom-Json
    }
    catch {
        $r = $_.Exception.Response
        if ($r) {
            $reader = New-Object System.IO.StreamReader($r.GetResponseStream())
            $text = $reader.ReadToEnd()
            Write-Host "HTTP $([int]$r.StatusCode)" -ForegroundColor Red
            Write-Host $text
            return $text | ConvertFrom-Json
        }
        Write-Host $_.Exception.Message -ForegroundColor Red
        return $null
    }
}

if ([string]::IsNullOrWhiteSpace($AdminUsername)) {
    $AdminUsername = Read-Host "Username admin (mesmo login do UmbraManager)"
}

Write-Host "=== 1) Listar templates ===" -ForegroundColor Yellow
$t = Invoke-UmbraAdmin -Path "/admin/list_npc_templates.php" -Body @{}
if (-not $t -or -not $t.success) {
    Write-Host "`nFALHOU na listagem. Corrija auth/WAMP antes do spawn." -ForegroundColor Red
    Write-Host "Dica: use o MESMO username com isadmin=1 que funciona no login do UmbraManager."
    exit 1
}
Write-Host "Templates: $($t.templates.Count)"

Write-Host "`n=== 2) Spawn NPC ===" -ForegroundColor Yellow
$s = Invoke-UmbraAdmin -Path "/admin/spawn_npc.php" -Body @{
    npc_template_id = $TemplateId
    zone_id         = $ZoneId
    pos_x           = $PosX
    pos_y           = $PosY
    pos_z           = $PosZ
    yaw             = $Yaw
}
if (-not $s -or -not $s.success) {
    Write-Host "`nFALHOU no spawn. Leia a mensagem JSON acima." -ForegroundColor Red
    exit 1
}
Write-Host "OK instance_id=$($s.npc_instance_id)" -ForegroundColor Green

Write-Host "`n=== 3) Listar instancias ===" -ForegroundColor Yellow
$i = Invoke-UmbraAdmin -Path "/admin/list_npc_instances.php" -Body @{ zone_id = $ZoneId }
if ($i -and $i.success) {
    Write-Host "Instancias na zone $ZoneId : $($i.total)"
}

Write-Host "`nConcluido. No UmbraManager: NPCs -> Instancias no mundo -> Atualizar instancias" -ForegroundColor Green
