# Análise: Implementação de Report e Block no Unreal Engine

## 📋 Resumo Executivo

**Status atual:**
- ✅ **Block**: Implementação completa no C++ (bloquear/desbloquear funcionando)
- ⚠️ **Report**: Função C++ implementada, mas **falta UI** para coletar motivo
- ❌ **Carregamento inicial**: Lista de bloqueados não é carregada do servidor ao iniciar

---

## ✅ O QUE JÁ ESTÁ IMPLEMENTADO

### 1. **Block (Bloquear)** - ✅ COMPLETO

#### C++ (`UmbraGameInstance.h/cpp`)

**Funções implementadas:**
- ✅ `BlockPlayer(int32 TargetPlayerID, const FString& PlayerName)` - Bloqueia um jogador
- ✅ `UnblockPlayer(int32 TargetPlayerID)` - Desbloqueia um jogador
- ✅ `IsPlayerBlocked(int32 PlayerID)` - Verifica se está bloqueado
- ✅ `OnBlockPlayerComplete()` - Callback de sucesso
- ✅ `OnBlockPlayerFail()` - Callback de erro
- ✅ `OnUnblockPlayerComplete()` - Callback de desbloqueio
- ✅ `OnUnblockPlayerFail()` - Callback de erro no desbloqueio

**Delegates/Eventos:**
- ✅ `FOnPlayerBlocked` - Disparado quando bloqueia com sucesso
- ✅ `FOnPlayerUnblocked` - Disparado quando desbloqueia com sucesso
- ✅ `FOnBlockFailed` - Disparado em caso de erro

**Integração no Context Menu:**
- ✅ `HandlePlayerContextAction()` já chama `BlockPlayer()` quando ação é `EUmbraPlayerContextAction::Block`

**Armazenamento local:**
- ✅ `TArray<int32> BlockedPlayerIDs` - Mantém lista de IDs bloqueados em memória

**APIs PHP utilizadas:**
- ✅ `/api/social/block_player.php` - POST com `blocked_player_id` e `token`
- ✅ `/api/social/unblock_player.php` - POST com `blocked_player_id` e `token`

**Validações implementadas:**
- ✅ Verifica se `TargetPlayerID > 0`
- ✅ Verifica se não está bloqueando a si mesmo (`TargetPlayerID == ActivePlayerID`)
- ✅ Verifica se já está bloqueado (`BlockedPlayerIDs.Contains()`)

---

### 2. **Report (Denunciar)** - ⚠️ PARCIALMENTE IMPLEMENTADO

#### C++ (`UmbraGameInstance.h/cpp`)

**Funções implementadas:**
- ✅ `ReportPlayer(int32 TargetPlayerID, const FString& Reason)` - Denuncia um jogador
- ✅ `OnReportPlayerComplete()` - Callback de sucesso
- ✅ `OnReportPlayerFail()` - Callback de erro

**Delegates/Eventos:**
- ✅ `FOnPlayerReported` - Disparado quando denúncia é registrada
- ✅ `FOnPlayerReportFailed` - Disparado em caso de erro

**Validações implementadas:**
- ✅ Verifica se `TargetPlayerID > 0`
- ✅ Verifica se `Reason` não está vazio

**API PHP utilizada:**
- ✅ `/api/social/report_player.php` - POST com `reported_player_id`, `reason` e `token`

**Problema:**
- ❌ `HandlePlayerContextAction()` **NÃO chama** `ReportPlayer()` quando ação é `EUmbraPlayerContextAction::Report`
- ❌ Apenas faz log: *"Blueprint deve abrir UI de denúncia"*
- ❌ **Falta UI** para coletar o motivo (`Reason`) do usuário

---

## ❌ O QUE FALTA IMPLEMENTAR

### 1. **Report - UI e Integração** 🔴 PRIORIDADE ALTA

#### A. Criar Widget de Denúncia (Blueprint)

**Arquivo:** `WBP_ReportPlayer.uasset` (ou similar)

**Campos necessários:**
- Campo de texto para **motivo da denúncia** (`Reason`)
  - Textarea/multiline
  - Validação: mínimo 10 caracteres (conforme API)
  - Placeholder: "Descreva o motivo da denúncia..."
- Botão **"Enviar Denúncia"**
- Botão **"Cancelar"**
- Label com nome do jogador sendo denunciado

**Fluxo:**
1. Usuário clica em "Report" no context menu
2. Abre `WBP_ReportPlayer` modal
3. Usuário preenche motivo (mín. 10 caracteres)
4. Clica "Enviar Denúncia"
5. Widget chama `UmbraGameInstance::ReportPlayer(TargetPlayerID, Reason)`
6. Escuta `OnPlayerReported` ou `OnPlayerReportFailed`
7. Mostra mensagem de sucesso/erro
8. Fecha widget

#### B. Integrar no `HandlePlayerContextAction()`

**Arquivo:** `UmbraGameInstance.cpp` linha ~5235

**Mudança necessária:**
```cpp
case EUmbraPlayerContextAction::Report:
    // Abrir widget de denúncia (Blueprint)
    // O widget deve chamar ReportPlayer() após coletar o motivo
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 🚨 Report solicitado para %s - Abrindo UI de denúncia"), *PlayerInfo.CharacterName);
    
    // Opção 1: Disparar evento para Blueprint criar widget
    OnReportPlayerRequested.Broadcast(PlayerInfo.PlayerID, PlayerInfo.CharacterName);
    
    // Opção 2: Criar widget diretamente no C++ (se preferir)
    // CreateReportWidget(PlayerInfo.PlayerID, PlayerInfo.CharacterName);
    break;
```

**Novo delegate necessário (se usar Opção 1):**
```cpp
// Em UmbraGameInstance.h
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReportPlayerRequested, int32, PlayerID, const FString&, PlayerName);
UPROPERTY(BlueprintAssignable, Category = "Events|Social")
FOnReportPlayerRequested OnReportPlayerRequested;
```

---

### 2. **Carregar Lista de Bloqueados ao Iniciar** 🟡 PRIORIDADE MÉDIA

#### Problema atual:
- `BlockedPlayerIDs` é um `TArray<int32>` que só é populado quando o jogador bloqueia alguém **durante a sessão**
- Se o jogador já tinha bloqueados antes de entrar no jogo, eles não são carregados
- `IsPlayerBlocked()` pode retornar `false` para jogadores que já estão bloqueados no servidor

#### Solução: Criar API e função de carregamento

**A. Criar API PHP:** `get_blocked_players.php`

**Arquivo:** `www/umbra_api/api/social/get_blocked_players.php`

**Endpoint:**
- **Método:** POST
- **Body:** `{ "token": "..." }`
- **Resposta:**
```json
{
  "success": true,
  "blocked_players": [
    {
      "blocked_player_id": 2,
      "blocked_player_name": "PlayerName",
      "block_id": 1,
      "created_at": "2026-02-07 10:00:00"
    }
  ]
}
```

**B. Implementar função no C++:**

**Arquivo:** `UmbraGameInstance.h/cpp`

**Nova função:**
```cpp
// Em UmbraGameInstance.h
UFUNCTION(BlueprintCallable, Category = "Social|Block")
void LoadBlockedPlayers();

// Em UmbraGameInstance.cpp
void UUmbraGameInstance::LoadBlockedPlayers()
{
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 📥 Carregando lista de jogadores bloqueados..."));
    
    UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/social/get_blocked_players.php"), TEXT("POST"));
    if (!Request)
    {
        UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ Erro ao criar requisição para carregar bloqueados"));
        return;
    }

    UVaRestJsonObject* RequestObject = Request->GetRequestObject();
    RequestObject->SetStringField(TEXT("token"), CurrentToken);

    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnLoadBlockedPlayersComplete);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnLoadBlockedPlayersFail);

    Request->ExecuteProcessRequest();
}

void UUmbraGameInstance::OnLoadBlockedPlayersComplete(UVaRestRequestJSON* Request)
{
    UVaRestJsonObject* ResponseObject = Request->GetResponseObject();
    
    if (!ResponseObject->GetBoolField(TEXT("success")))
    {
        FString ErrorMsg = ResponseObject->GetStringField(TEXT("message"));
        UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ Erro ao carregar bloqueados: %s"), *ErrorMsg);
        return;
    }

    BlockedPlayerIDs.Empty(); // Limpar lista atual
    
    TArray<UVaRestJsonObject*> BlockedArray;
    ResponseObject->GetObjectArrayField(TEXT("blocked_players"), BlockedArray);
    
    for (UVaRestJsonObject* BlockedObj : BlockedArray)
    {
        int32 BlockedID = BlockedObj->GetIntegerField(TEXT("blocked_player_id"));
        FString BlockedName = BlockedObj->GetStringField(TEXT("blocked_player_name"));
        BlockedPlayerIDs.Add(BlockedID);
        UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Bloqueado carregado: %s (ID: %d)"), *BlockedName, BlockedID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Total de bloqueados carregados: %d"), BlockedPlayerIDs.Num());
}
```

**C. Chamar ao fazer login/selecionar personagem:**

**Arquivo:** `UmbraGameInstance.cpp` (onde faz login/seleção)

**Adicionar após login bem-sucedido:**
```cpp
// Após selecionar personagem ou fazer login
LoadBlockedPlayers();
```

---

### 3. **Melhorias Opcionais** 🟢 PRIORIDADE BAIXA

#### A. Feedback visual no Context Menu

- Mostrar "Bloqueado" ou ícone diferente para jogadores já bloqueados
- Desabilitar botão "Block" se já estiver bloqueado
- Mostrar "Desbloquear" em vez de "Bloquear" se já estiver bloqueado

**Implementação:**
- No widget do context menu (Blueprint), verificar `IsPlayerBlocked(PlayerID)` antes de mostrar botões
- Escutar `OnPlayerBlocked` e `OnPlayerUnblocked` para atualizar UI dinamicamente

#### B. Widget de Lista de Bloqueados

- Criar `WBP_BlockedPlayersList` para mostrar/gerenciar bloqueados
- Permitir desbloquear diretamente da lista
- Mostrar data de bloqueio (se API retornar)

#### C. Validação de motivo no cliente

- Adicionar validação no C++ antes de enviar: `Reason.Len() >= 10`
- Mostrar erro imediatamente se motivo for muito curto

---

## 📝 CHECKLIST DE IMPLEMENTAÇÃO

### Report (Denunciar)
- [ ] Criar widget `WBP_ReportPlayer` (Blueprint)
  - [ ] Campo de texto para motivo (textarea)
  - [ ] Validação de mínimo 10 caracteres
  - [ ] Botões "Enviar" e "Cancelar"
  - [ ] Mostrar nome do jogador sendo denunciado
- [ ] Adicionar delegate `OnReportPlayerRequested` (ou criar widget diretamente)
- [ ] Modificar `HandlePlayerContextAction()` para abrir widget quando ação é `Report`
- [ ] Conectar widget aos eventos `OnPlayerReported` e `OnPlayerReportFailed`
- [ ] Testar fluxo completo: Context Menu → Widget → Enviar → Verificar resposta

### Carregar Bloqueados
- [ ] Criar API `get_blocked_players.php`
- [ ] Implementar `LoadBlockedPlayers()` no C++
- [ ] Implementar `OnLoadBlockedPlayersComplete()` e `OnLoadBlockedPlayersFail()`
- [ ] Chamar `LoadBlockedPlayers()` após login/seleção de personagem
- [ ] Testar: fazer login → verificar se bloqueados anteriores aparecem

### Melhorias (Opcional)
- [ ] Atualizar context menu para mostrar status de bloqueado
- [ ] Criar widget de lista de bloqueados
- [ ] Adicionar validação de motivo no cliente

---

## 🔗 ARQUIVOS RELEVANTES

### C++ (Cliente UE)
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h` - Declarações
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` - Implementações
  - Linha ~5235: `HandlePlayerContextAction()` - Integração Report
  - Linha ~7924: `ReportPlayer()` - Função de denúncia
  - Linha ~7984: `BlockPlayer()` - Função de bloqueio
  - Linha ~8051: `UnblockPlayer()` - Função de desbloqueio

### PHP (API)
- `www/umbra_api/api/social/report_player.php` - Endpoint de denúncia ✅
- `www/umbra_api/api/social/block_player.php` - Endpoint de bloqueio ✅
- `www/umbra_api/api/social/unblock_player.php` - Endpoint de desbloqueio ✅
- `www/umbra_api/api/social/get_blocked_players.php` - **FALTA CRIAR** ❌

### Blueprint/UI (Cliente UE)
- Widget de Context Menu (onde está o botão Report/Block) - **Verificar se existe**
- `WBP_ReportPlayer` - **FALTA CRIAR** ❌
- Widget de lista de bloqueados - **Opcional**

---

## 📊 RESUMO POR PRIORIDADE

| Item | Status | Prioridade | Complexidade |
|------|--------|------------|--------------|
| Report - Widget UI | ❌ Falta | 🔴 Alta | Média |
| Report - Integração no Context Menu | ❌ Falta | 🔴 Alta | Baixa |
| Carregar Bloqueados - API | ❌ Falta | 🟡 Média | Baixa |
| Carregar Bloqueados - C++ | ❌ Falta | 🟡 Média | Média |
| Block - Funcionalidade | ✅ Completo | - | - |
| Melhorias UI Context Menu | ⚠️ Opcional | 🟢 Baixa | Média |

---

## 🎯 PRÓXIMOS PASSOS RECOMENDADOS

1. **Criar widget `WBP_ReportPlayer`** (Blueprint)
2. **Integrar Report no `HandlePlayerContextAction()`** (C++)
3. **Criar API `get_blocked_players.php`** (PHP)
4. **Implementar `LoadBlockedPlayers()`** (C++)
5. **Testar fluxo completo** de Report e Block

---

**Data da análise:** 2026-02-07  
**Versão:** 1.0
