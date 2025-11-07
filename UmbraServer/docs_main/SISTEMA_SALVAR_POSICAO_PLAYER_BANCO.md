# 💾 **SISTEMA COMPLETO: Salvar e Usar Posição do Player no Banco de Dados**

## 📋 **VISÃO GERAL:**

O sistema precisa:
1. **Salvar posição periodicamente** do player no banco de dados (`pos_x`, `pos_y`, `pos_z`)
2. **Carregar posição ao fazer login** e usar no spawn inicial
3. **Atualizar posição** quando o player se move

---

## 🔍 **ANÁLISE DO ESTADO ATUAL:**

### **✅ O que já existe:**

1. **Banco de dados:** Tabela `players` já tem campos `pos_x`, `pos_y`, `pos_z`
2. **Parse no login:** `UmbraGameInstance.cpp` já faz parse de `pos_x`, `pos_y`, `pos_z` (linha 425-430)
3. **PlayerDAO:** Já tem função `updatePosition()` para salvar no banco
4. **Servidor C++:** `PlayerManager::addPlayer()` já usa `player.posX, posY, posZ` do banco

### **❌ O que falta:**

1. **API PHP:** Endpoint para receber atualização de posição do cliente
2. **Função UE5:** Função para salvar posição periodicamente
3. **Spawn inicial:** Usar posição do banco ao invés de `(0, 0, 0)`

---

## 🎯 **SOLUÇÃO COMPLETA:**

### **PARTE 1: Criar API PHP para Atualizar Posição**

#### **Arquivo: `www/umbra_api/api/character/update_position.php`**

```php
<?php
require_once __DIR__ . '/../../config/database.php';
require_once __DIR__ . '/../../utils/auth.php';
require_once __DIR__ . '/../../utils/response.php';

header('Content-Type: application/json');

// Verificar método
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    sendErrorResponse(405, 'Método não permitido');
    exit;
}

// Obter dados do body
$input = json_decode(file_get_contents('php://input'), true);

if (!$input) {
    sendErrorResponse(400, 'JSON inválido');
    exit;
}

// Validar token
$token = $input['token'] ?? '';
$accountId = validateToken($token);

if (!$accountId) {
    sendErrorResponse(401, 'Token inválido ou expirado');
    exit;
}

// Validar campos obrigatórios
$playerId = $input['player_id'] ?? 0;
$posX = $input['pos_x'] ?? null;
$posY = $input['pos_y'] ?? null;
$posZ = $input['pos_z'] ?? null;
$currentZone = $input['current_zone'] ?? 'Tutorial';

if ($playerId <= 0 || $posX === null || $posY === null || $posZ === null) {
    sendErrorResponse(400, 'Campos obrigatórios: player_id, pos_x, pos_y, pos_z');
    exit;
}

try {
    // Verificar se o player pertence à conta
    $stmt = $pdo->prepare("SELECT account_id FROM players WHERE id = ?");
    $stmt->execute([$playerId]);
    $player = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$player || $player['account_id'] != $accountId) {
        sendErrorResponse(403, 'Player não pertence à sua conta');
        exit;
    }
    
    // Atualizar posição
    $stmt = $pdo->prepare("
        UPDATE players 
        SET pos_x = ?, pos_y = ?, pos_z = ?, current_zone = ?, last_played_at = NOW()
        WHERE id = ?
    ");
    
    $success = $stmt->execute([$posX, $posY, $posZ, $currentZone, $playerId]);
    
    if ($success) {
        sendSuccessResponse([
            'message' => 'Posição atualizada com sucesso',
            'player_id' => $playerId,
            'position' => [
                'x' => $posX,
                'y' => $posY,
                'z' => $posZ
            ],
            'current_zone' => $currentZone
        ]);
    } else {
        sendErrorResponse(500, 'Erro ao atualizar posição');
    }
} catch (PDOException $e) {
    error_log("Erro ao atualizar posição: " . $e->getMessage());
    sendErrorResponse(500, 'Erro interno do servidor');
}
?>
```

---

### **PARTE 2: Adicionar Função no UmbraGameInstance (C++)**

#### **No arquivo `UmbraGameInstance.h`:**

**ADICIONAR na seção de funções públicas:**

```cpp
/**
 * Salvar posição do player no banco de dados
 * @param PlayerID ID do personagem
 * @param Position Posição atual (FVector)
 * @param CurrentZone Zona atual
 */
UFUNCTION(BlueprintCallable, Category = "Character")
void SavePlayerPosition(int32 PlayerID, const FVector& Position, const FString& CurrentZone = TEXT("Tutorial"));
```

#### **No arquivo `UmbraGameInstance.cpp`:**

**ADICIONAR implementação:**

```cpp
void UUmbraGameInstance::SavePlayerPosition(int32 PlayerID, const FVector& Position, const FString& CurrentZone)
{
	if (!bIsAuthenticated || PlayerID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ Não autenticado ou PlayerID inválido para salvar posição"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] Salvando posição do player %d: X=%.2f Y=%.2f Z=%.2f"), 
		PlayerID, Position.X, Position.Y, Position.Z);

	UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/character/update_position.php"));
	if (!Request)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ Erro ao criar requisição para salvar posição"));
		return;
	}

	UVaRestSubsystem* VaRest = GEngine->GetEngineSubsystem<UVaRestSubsystem>();
	if (!VaRest)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ VaRest Subsystem não encontrado"));
		return;
	}

	UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
	JsonObject->SetStringField(TEXT("token"), CurrentToken);
	JsonObject->SetNumberField(TEXT("player_id"), PlayerID);
	JsonObject->SetNumberField(TEXT("pos_x"), Position.X);
	JsonObject->SetNumberField(TEXT("pos_y"), Position.Y);
	JsonObject->SetNumberField(TEXT("pos_z"), Position.Z);
	JsonObject->SetStringField(TEXT("current_zone"), CurrentZone);

	Request->SetRequestObject(JsonObject);
	Request->SetVerb(TEXT("POST"));
	
	// Opcional: adicionar callback se necessário
	// Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnSavePositionComplete);
	// Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnSavePositionFail);

	Request->ExecuteProcessRequest();
}
```

---

### **PARTE 3: Modificar Spawn para Usar Posição do Banco**

#### **No Blueprint `BP_NetMovementClient`:**

**Quando o player spawna pela primeira vez (no `BeginPlay` ou quando conecta ao WebSocket):**

1. **Obter posição do personagem selecionado:**
   - `Get Game Instance` → `Cast to UmbraGameInstance`
   - `Get Active Character` → `Break UmbraPlayerData`
   - Obter `Position` (FVector)

2. **Validar posição:**
   - Se `Position != (0, 0, 0)`, usar esta posição
   - Se `Position == (0, 0, 0)`, usar posição padrão (ex: PlayerStart)

3. **Aplicar posição:**
   - `Set Actor Location` com a posição do banco

#### **Estrutura no Blueprint:**

```
BeginPlay
  ↓
[Conectar WebSocket]
  ↓
[Após conexão bem-sucedida]
  ↓
Get Game Instance → Cast to UmbraGameInstance
  ↓
Get Active Character → Break UmbraPlayerData
  ↓
Get Position (FVector)
  ↓
Not Equal (Vector): Position != (0, 0, 0)
  ↓
Branch
  ├─ True: Position válida
  │   ↓
  │   Set Actor Location (Position do banco)
  │
  └─ False: Position inválida
      ↓
      [Buscar PlayerStart ou usar posição padrão]
      ↓
      Set Actor Location (Posição padrão)
```

---

### **PARTE 4: Salvar Posição Periodicamente**

#### **No Blueprint do Character (ex: `BP_ThirdPersonCharacter`):**

**Adicionar no `Event Tick`:**

1. **Criar variável:** `LastSavedPositionTime` (Float)
2. **Criar variável:** `PositionSaveInterval` (Float, padrão: 5.0 segundos)

**Lógica:**

```
Event Tick
  ↓
Get Game Time In Seconds
  ↓
Subtract: CurrentTime - LastSavedPositionTime
  ↓
Greater (Float): DeltaTime >= PositionSaveInterval
  ↓
Branch
  ├─ True: Tempo suficiente passou
  │   ↓
  │   Get Actor Location
  │   ↓
  │   Get Game Instance → Cast to UmbraGameInstance
  │   ↓
  │   Get Active Player ID
  │   ↓
  │   Save Player Position (PlayerID, Location, Zone)
  │   ↓
  │   Set LastSavedPositionTime = CurrentTime
  │
  └─ False: Aguardar mais tempo
```

**OU usar `Timer` ao invés de `Event Tick`:**

```
BeginPlay
  ↓
Set Timer by Function Name
  - Function Name: "SavePositionTimer"
  - Time: 5.0 (segundos)
  - Looping: True
  ↓
[Timer executa a cada 5 segundos]

SavePositionTimer (Custom Event)
  ↓
Get Actor Location
  ↓
Get Game Instance → Cast to UmbraGameInstance
  ↓
Get Active Player ID
  ↓
Save Player Position (PlayerID, Location, Zone)
```

---

### **PARTE 5: Salvar Posição ao Desconectar**

#### **No Blueprint `BP_NetMovementClient`:**

**No `Event EndPlay` ou `OnWSClosed`:**

```
Event EndPlay / OnWSClosed
  ↓
Get Actor Location (do player local)
  ↓
Get Game Instance → Cast to UmbraGameInstance
  ↓
Get Active Player ID
  ↓
Save Player Position (PlayerID, Location, Zone)
  ↓
[Fechar WebSocket]
```

---

## 📊 **FLUXO COMPLETO:**

### **1. Login e Seleção de Personagem:**

```
Login
  ↓
Parse Response → PlayerData.Position (pos_x, pos_y, pos_z do banco)
  ↓
Select Character
  ↓
Parse Response → SelectedPlayer.Position (pos_x, pos_y, pos_z do banco)
  ↓
[Posição salva em UmbraGameInstance]
```

### **2. Spawn Inicial:**

```
BeginPlay (Character)
  ↓
Get Game Instance → Get Active Character → Get Position
  ↓
Position != (0, 0, 0)?
  ├─ True: Set Actor Location (Position do banco) ✅
  └─ False: Set Actor Location (Posição padrão/PlayerStart)
```

### **3. Durante o Jogo:**

```
Event Tick / Timer (a cada 5 segundos)
  ↓
Get Actor Location
  ↓
Save Player Position (PlayerID, Location, Zone)
  ↓
[API PHP atualiza banco de dados]
```

### **4. Ao Desconectar:**

```
Event EndPlay / OnWSClosed
  ↓
Get Actor Location
  ↓
Save Player Position (PlayerID, Location, Zone)
  ↓
[Última posição salva no banco]
```

---

## 🔧 **IMPLEMENTAÇÃO PASSO A PASSO:**

### **PASSO 1: Criar API PHP**

1. Criar arquivo `www/umbra_api/api/character/update_position.php`
2. Copiar código fornecido acima
3. Testar com Postman ou similar

### **PASSO 2: Adicionar Função C++**

1. Adicionar declaração em `UmbraGameInstance.h`
2. Adicionar implementação em `UmbraGameInstance.cpp`
3. Compilar projeto UE5

### **PASSO 3: Modificar Spawn no Blueprint**

1. Abrir `BP_ThirdPersonCharacter` (ou Character principal)
2. No `BeginPlay`, adicionar lógica para obter posição do banco
3. Aplicar posição antes de conectar ao WebSocket

### **PASSO 4: Adicionar Timer de Salvamento**

1. No Character Blueprint, adicionar `Timer` no `BeginPlay`
2. Criar Custom Event `SavePositionTimer`
3. Chamar `Save Player Position` periodicamente

### **PASSO 5: Salvar ao Desconectar**

1. No `BP_NetMovementClient`, no `Event EndPlay`
2. Adicionar chamada para `Save Player Position` antes de fechar WebSocket

---

## ✅ **CHECKLIST:**

- [ ] API PHP `update_position.php` criada e testada
- [ ] Função `SavePlayerPosition` adicionada em `UmbraGameInstance.h`
- [ ] Função `SavePlayerPosition` implementada em `UmbraGameInstance.cpp`
- [ ] Projeto UE5 compilado com sucesso
- [ ] Spawn inicial modificado para usar posição do banco
- [ ] Timer de salvamento periódico implementado
- [ ] Salvamento ao desconectar implementado
- [ ] Testado: Login → Spawn na posição correta
- [ ] Testado: Movimento → Posição salva no banco
- [ ] Testado: Desconectar → Última posição salva

---

## 🎯 **RESULTADO ESPERADO:**

1. **Ao fazer login:** Player spawna na última posição salva no banco
2. **Durante o jogo:** Posição é salva automaticamente a cada 5 segundos
3. **Ao desconectar:** Última posição é salva imediatamente
4. **Próximo login:** Player spawna exatamente onde estava quando desconectou

---

## 📝 **NOTAS IMPORTANTES:**

1. **Intervalo de salvamento:** 5 segundos é um bom equilíbrio entre performance e precisão. Ajuste conforme necessário.

2. **Validação de posição:** Se `pos_x = 0, pos_y = 0, pos_z = 0`, pode indicar primeiro login ou posição inválida. Use posição padrão neste caso.

3. **Zona atual:** Mantenha `current_zone` atualizado junto com a posição para facilitar spawn em zonas diferentes.

4. **Performance:** Salvar posição a cada frame seria muito custoso. Use timer ou intervalo mínimo entre salvamentos.

5. **Erro de rede:** Se `SavePlayerPosition` falhar, não deve bloquear o jogo. Apenas logue o erro e tente novamente no próximo intervalo.

