# 🔧 **GUIA PRÁTICO: Implementar Sistema de Salvar Posição**

## 📋 **PASSO 1: Criar API PHP**

### **Localização:**
`C:\wamp64\www\umbra_api\api\character\update_position.php`

### **Código Completo:**

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

## 📋 **PASSO 2: Adicionar Função C++**

### **2.1: Adicionar Declaração em `UmbraGameInstance.h`**

**Localizar a seção de funções de personagem (aprox. linha 236):**

**ADICIONAR após `SelectCharacter`:**

```cpp
/**
 * Salvar posição do player no banco de dados
 * @param PlayerID ID do personagem
 * @param Position Posição atual (FVector)
 * @param CurrentZone Zona atual (padrão: "Tutorial")
 */
UFUNCTION(BlueprintCallable, Category = "Character")
void SavePlayerPosition(int32 PlayerID, const FVector& Position, const FString& CurrentZone = TEXT("Tutorial"));
```

### **2.2: Adicionar Implementação em `UmbraGameInstance.cpp`**

**Localizar a seção de funções de personagem (aprox. linha 850):**

**ADICIONAR após `OnSelectCharacterRequestFail`:**

```cpp
// ========== SAVE PLAYER POSITION ==========

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
	
	Request->ExecuteProcessRequest();
}
```

---

## 📋 **PASSO 3: Modificar Spawn Inicial no Blueprint**

### **Blueprint: Character Principal (ex: `BP_ThirdPersonCharacter`)**

#### **No `BeginPlay`:**

1. **Obter posição do banco:**
   ```
   Get Game Instance
     ↓
   Cast to UmbraGameInstance
     ↓
   Get Active Character
     ↓
   Break UmbraPlayerData
     ↓
   Get Position (FVector)
   ```

2. **Validar posição:**
   ```
   Not Equal (Vector): Position != (0, 0, 0)
     ↓
   Branch
     ├─ True: Position válida
     └─ False: Position inválida (usar padrão)
   ```

3. **Aplicar posição:**
   ```
   [Caminho True]
     ↓
   Set Actor Location
     - Target: Self
     - New Location: Position (do banco)
     - bTeleport: True
   ```

---

## 📋 **PASSO 4: Adicionar Timer de Salvamento**

### **Blueprint: Character Principal**

#### **No `BeginPlay`:**

1. **Adicionar Timer:**
   ```
   Set Timer by Function Name
     - Function Name: "SavePositionTimer"
     - Time: 5.0
     - Looping: True
   ```

2. **Criar Custom Event `SavePositionTimer`:**
   ```
   SavePositionTimer (Custom Event)
     ↓
   Get Actor Location
     ↓
   Get Game Instance → Cast to UmbraGameInstance
     ↓
   Get Active Player ID
     ↓
   Save Player Position
     - PlayerID: Get Active Player ID
     - Position: Get Actor Location
     - CurrentZone: "Tutorial" (ou variável)
   ```

---

## 📋 **PASSO 5: Salvar ao Desconectar**

### **Blueprint: `BP_NetMovementClient`**

#### **No `Event EndPlay`:**

**ANTES de fechar WebSocket:**

### **5.1: Obter Referência do Character Local**

**OPÇÃO 1 - Usar Função Helper (RECOMENDADO):**

1. **Adicionar `Get First Player Pawn Helper`:**
   - Clique direito → Busque: `Get First Player Pawn Helper`
   - Selecione: `Get First Player Pawn Helper` (da categoria `Umbra|Net|WS|Helpers`)
   - **World Context Object:** Conecte ao `Self` (ou deixe desconectado se auto-referenciar)
   - **Return Value:** Tipo `Pawn` (objeto)

2. **Adicionar `IsValid`:**
   - Do `Return Value` do `Get First Player Pawn Helper` → Arraste → `IsValid`
   - **Target:** Conectado automaticamente ao Return Value
   - **Return Value:** Boolean

3. **Adicionar `Branch`:**
   - Do `Return Value` (Boolean) do `IsValid` → Arraste → `Branch`
   - **Condition:** Conectado automaticamente

**OPÇÃO 2 - Método Padrão do Unreal:**

1. **Adicionar `Get First Player Controller`:**
   - Clique direito → Busque: `Get First Player Controller`
   - Selecione: `Get First Player Controller` (da categoria `Game`)
   - **World Context Object:** Conecte ao `Self` (ou deixe desconectado)

2. **Adicionar `Get Pawn`:**
   - Do `Return Value` do `Get First Player Controller` → Arraste → `Get Pawn`
   - **Target:** Conectado automaticamente ao Return Value
   - **Return Value:** Tipo `Pawn`

3. **Adicionar `IsValid` e `Branch`** (mesmo processo da Opção 1)

---

### **5.2: Obter Posição do Character**

**No caminho `True` do Branch (Pawn válido):**

1. **Adicionar `Get Actor Location`:**
   - Do `Return Value` (Pawn) do `Get First Player Pawn Helper` (ou `Get Pawn`) → Arraste → `Get Actor Location`
   - **Target:** Conectado automaticamente ao Return Value
   - **Return Value:** Tipo `Vector` (posição)

---

### **5.3: Salvar Posição**

**Após `Get Actor Location`:**

```
[Branch True]
  ↓
Get Actor Location (do Pawn)
  ↓
Get Game Instance → Cast to UmbraGameInstance
  ↓
Get Active Player ID
  ↓
Save Player Position
  - PlayerID: Get Active Player ID
  - Position: Get Actor Location (Vector)
  - CurrentZone: "Tutorial" (ou variável)
```

**No caminho `False` do Branch (Pawn inválido):**
- Apenas continue para fechar o WebSocket (não salva posição se não houver character)

---

### **5.4: Fluxo Completo**

```
Event EndPlay
  ↓
Get First Player Pawn Helper (Self)
  ↓
IsValid (Pawn)
  ↓
Branch
  ├─ True:
  │    ↓
  │   Get Actor Location (do Pawn)
  │    ↓
  │   Get Game Instance → Cast to UmbraGameInstance
  │    ↓
  │   Get Active Player ID
  │    ↓
  │   Save Player Position
  │    - PlayerID: Get Active Player ID
  │    - Position: Get Actor Location
  │    - CurrentZone: "Tutorial"
  │    ↓
  │   [Continuar para fechar WebSocket]
  │
  └─ False:
       ↓
      [Continuar para fechar WebSocket - sem salvar]
```

---

## ✅ **TESTE COMPLETO:**

### **Teste 1: Spawn na Posição Correta**

1. Fazer login
2. Selecionar personagem
3. Verificar se spawna na posição salva no banco (não em `(0,0,0)`)

### **Teste 2: Salvamento Periódico**

1. Mover o personagem
2. Aguardar 5 segundos
3. Verificar no banco se `pos_x`, `pos_y`, `pos_z` foram atualizados

### **Teste 3: Salvamento ao Desconectar**

1. Mover o personagem
2. Fechar o jogo (ESC)
3. Verificar no banco se última posição foi salva

### **Teste 4: Próximo Login**

1. Fazer logout
2. Fazer login novamente
3. Selecionar mesmo personagem
4. Verificar se spawna na última posição salva

---

## 🎯 **RESULTADO ESPERADO:**

✅ Player spawna na última posição salva  
✅ Posição é salva automaticamente a cada 5 segundos  
✅ Última posição é salva ao desconectar  
✅ Próximo login spawna na posição correta  

---

## 📝 **NOTAS:**

1. **Intervalo de salvamento:** 5 segundos é recomendado. Ajuste conforme necessário.

2. **Validação:** Se `Position == (0,0,0)`, use posição padrão (PlayerStart).

3. **Zona:** Mantenha `current_zone` atualizado para facilitar spawn em zonas diferentes.

4. **Performance:** Não salve a cada frame. Use timer ou intervalo mínimo.

5. **Erros:** Se `SavePlayerPosition` falhar, apenas logue. Não bloqueie o jogo.

