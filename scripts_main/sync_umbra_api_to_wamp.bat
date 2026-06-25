@echo off
REM ============================================================================
REM  sync_umbra_api_to_wamp.bat
REM  Sincroniza o codigo PHP entre o repositorio e o WAMP.
REM
REM  Uso:
REM    sync_umbra_api_to_wamp.bat              ->  Repo -> WAMP (modo SEGURO:
REM                                                NAO sobrescreve arquivos que
REM                                                estiverem mais NOVOS no destino)
REM    sync_umbra_api_to_wamp.bat --dry        ->  Simulacao (mostra o que copia)
REM    sync_umbra_api_to_wamp.bat --force      ->  Sobrescreve TUDO no destino
REM                                                (perde mudancas locais do WAMP)
REM    sync_umbra_api_to_wamp.bat --reverse    ->  WAMP -> Repo (igual: nao regride)
REM    sync_umbra_api_to_wamp.bat --mirror     ->  Repo -> WAMP com /MIR (CUIDADO:
REM                                                apaga no WAMP arquivos que nao
REM                                                existem no repo)
REM    sync_umbra_api_to_wamp.bat --reverse --dry
REM    sync_umbra_api_to_wamp.bat --mirror --dry
REM
REM  Por padrao NAO apaga arquivos no destino (uploads de gallery preservados)
REM  e NAO sobrescreve arquivos mais novos no destino (use --force para isso).
REM  Pastas auto-geradas pelo editor (backups de gallery) sao sempre ignoradas.
REM ============================================================================

setlocal EnableExtensions EnableDelayedExpansion

set "REPO=D:\UmbraServerV2\www\umbra_api"
set "WAMP=C:\wamp64\www\umbra_api"

REM --- Parse de flags --------------------------------------------------------
set "DRY=0"
set "REVERSE=0"
set "MIRROR=0"
set "FORCE=0"

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--dry"     set "DRY=1"     & shift & goto parse_args
if /i "%~1"=="--reverse" set "REVERSE=1" & shift & goto parse_args
if /i "%~1"=="--mirror"  set "MIRROR=1"  & shift & goto parse_args
if /i "%~1"=="--force"   set "FORCE=1"   & shift & goto parse_args
if /i "%~1"=="-h"        goto show_help
if /i "%~1"=="--help"    goto show_help
if /i "%~1"=="/?"        goto show_help
echo [ERRO] Flag desconhecida: %~1
goto show_help
:end_parse

REM --- Definir SRC/DST conforme direcao --------------------------------------
if "%REVERSE%"=="1" (
    set "SRC=%WAMP%"
    set "DST=%REPO%"
    set "DIRECAO=WAMP -^> Repo"
) else (
    set "SRC=%REPO%"
    set "DST=%WAMP%"
    set "DIRECAO=Repo -^> WAMP"
)

REM --- Montar flags do robocopy ----------------------------------------------
REM /E   = inclui subpastas (vazias tambem)
REM /R:1 = 1 retry em caso de erro
REM /W:1 = espera 1s entre retries
REM /NP  = sem barra de progresso (saida limpa)
REM /NDL = sem listar diretorios sem mudancas
REM /NJH /NJS = sem cabecalho/sumario (compacto)
set "ROBO_FLAGS=/E /R:1 /W:1 /NP /NDL"

REM /XO = eXclude Older source (so copia se origem for mais nova que destino)
REM Isso protege contra regressao de arquivos editados direto no destino.
if "%FORCE%"=="0" (
    if "%MIRROR%"=="0" (
        set "ROBO_FLAGS=!ROBO_FLAGS! /XO"
    )
)

if "%MIRROR%"=="1" (
    set "ROBO_FLAGS=!ROBO_FLAGS! /MIR /PURGE"
)

if "%DRY%"=="1" (
    set "ROBO_FLAGS=!ROBO_FLAGS! /L"
)

REM --- Exclusoes (sempre aplicadas) ------------------------------------------
REM Pastas geradas pelo gallery editor em runtime: NUNCA copiar/apagar.
set "EXCLUDE_DIRS=/XD backups"
set "EXCLUDE_FILES=/XF *.log *.tmp Thumbs.db"

REM --- Cabecalho -------------------------------------------------------------
echo.
echo ===============================================================
echo   Sincronizacao UmbraAPI
echo ===============================================================
echo   Direcao: %DIRECAO%
echo   Origem : %SRC%
echo   Destino: %DST%
if "%DRY%"=="1"    echo   Modo   : DRY-RUN (simulacao, nada sera escrito)
if "%MIRROR%"=="1" echo   Modo   : MIRROR  (apaga arquivos extras no destino)
if "%FORCE%"=="1"  echo   Modo   : FORCE   (sobrescreve TUDO, mesmo mais novos)
if "%FORCE%"=="0" if "%MIRROR%"=="0" echo   Modo   : SEGURO  (so copia arquivos mais novos que o destino)
echo   Excluindo: pastas backups\ + *.log + *.tmp
echo ===============================================================
echo.

REM --- Validacoes ------------------------------------------------------------
if not exist "%SRC%" (
    echo [ERRO] Diretorio fonte nao existe: %SRC%
    exit /b 1
)

if not exist "%DST%" (
    if "%DRY%"=="1" (
        echo [INFO] Destino nao existe ^(seria criado^): %DST%
    ) else (
        echo [INFO] Criando destino: %DST%
        mkdir "%DST%" 2>nul
    )
)

REM --- Confirmacao quando MIRROR estiver ativo (e nao for dry) ----------------
if "%MIRROR%"=="1" if "%DRY%"=="0" (
    echo [AVISO] Modo MIRROR vai APAGAR no destino qualquer arquivo que
    echo         nao exista na origem. Isso inclui uploads de gallery!
    set /p "CONFIRM=Tem certeza? Digite SIM para continuar: "
    if /i not "!CONFIRM!"=="SIM" (
        echo Cancelado pelo usuario.
        exit /b 0
    )
)

REM --- Executa robocopy ------------------------------------------------------
echo Executando robocopy...
echo.
robocopy "%SRC%" "%DST%" %ROBO_FLAGS% %EXCLUDE_DIRS% %EXCLUDE_FILES%
set "RC=%ERRORLEVEL%"

echo.
echo ===============================================================
echo   Robocopy exit code: %RC%

REM Robocopy: 0 = nada a copiar, 1 = arquivos copiados, 2 = extras detectados,
REM 3 = 1+2, 4 = mismatched, 5 = 4+1, 6 = 4+2, 7 = todos. >=8 = erro grave.
if %RC% EQU 0 (
    echo   [OK] Nada a fazer ^(diretorios ja sincronizados^).
) else if %RC% LSS 8 (
    echo   [OK] Sincronizacao concluida com sucesso.
) else (
    echo   [ERRO] Robocopy retornou codigo %RC% ^(>=8 indica falha^).
    exit /b 1
)
echo ===============================================================
echo.

if "%DRY%"=="1" (
    echo Para aplicar as mudancas, execute novamente sem --dry.
)

exit /b 0

:show_help
echo.
echo Uso: %~nx0 [--dry] [--reverse] [--force] [--mirror]
echo.
echo   (sem flags)  Repo -^> WAMP em modo SEGURO (so copia arquivos mais
echo                novos que o destino, nao apaga nada).
echo   --dry        Simulacao: mostra o que seria copiado/apagado, sem alterar.
echo   --reverse    Inverte: WAMP -^> Repo (use se editou direto no WAMP).
echo   --force      Sobrescreve TUDO, ignorando timestamp do destino.
echo                Use com cuidado: perde mudancas locais do destino.
echo   --mirror     Espelhamento total: apaga no destino o que nao existe
echo                na origem. Pede confirmacao. CUIDADO com uploads.
echo.
echo Exemplos:
echo   %~nx0 --dry                 # Simulacao segura
echo   %~nx0                       # Sync seguro (recomendado)
echo   %~nx0 --reverse --dry       # Ver o que viria do WAMP
echo   %~nx0 --reverse             # Trazer do WAMP coisas mais novas
echo   %~nx0 --force               # Forcar repo como verdade absoluta
echo   %~nx0 --mirror              # Espelhar (apaga extras)
echo.
exit /b 0
