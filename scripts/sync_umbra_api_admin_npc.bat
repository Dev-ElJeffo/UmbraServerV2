@echo off
REM Copia endpoints admin NPC (UmbraManager aba NPCs) para WAMP
setlocal

set SRC=%~dp0..\www\umbra_api\api\admin
set DST=C:\wamp64\www\umbra_api\api\admin

if not exist "%DST%" (
  echo [ERRO] Pasta WAMP nao encontrada: %DST%
  echo Ajuste DST neste script ou crie o symlink umbra_api no Apache.
  exit /b 1
)

for %%F in (
  require_admin_auth.php
  list_npc_templates.php
  create_npc_template.php
  update_npc_template.php
  spawn_npc.php
  list_npc_instances.php
  delete_npc_instance.php
) do (
  if not exist "%SRC%\%%F" (
    echo [ERRO] Arquivo ausente no repo: %SRC%\%%F
    exit /b 1
  )
  copy /Y "%SRC%\%%F" "%DST%\%%F" >nul
  echo [OK] %%F
)

echo.
echo Concluido. Teste spawn:
echo curl -X POST http://localhost/umbra_api/api/admin/spawn_npc.php -H "Content-Type: application/json" -d "{\"admin_username\":\"ADMIN\",\"npc_template_id\":1,\"zone_id\":1,\"pos_x\":0,\"pos_y\":0,\"pos_z\":200}"
endlocal
