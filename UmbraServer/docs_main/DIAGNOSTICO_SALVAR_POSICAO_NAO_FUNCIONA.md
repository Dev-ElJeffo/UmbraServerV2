# 🔍 **DIAGNÓSTICO: Sistema de Salvar Posição Não Funciona**

## 📋 **PROBLEMA:**
Posição não está sendo salva no banco de dados (pos_x, pos_y, pos_z permanecem 0) mesmo após movimentar o player.

---

## 🔬 **HIPÓTESES A INVESTIGAR:**

### **1. Timer não está sendo executado**
- O `SavePositionTimer` pode não estar sendo chamado a cada 5 segundos
- O timer pode ter sido cancelado ou não iniciado

### **2. GetActivePlayerID retornando 0 ou inválido**
- O PlayerID pode estar 0 quando `SavePlayerPosition` é chamada
- A função C++ pode estar rejeitando por PlayerID <= 0

### **3. SavePlayerPosition não está sendo chamada**
- A função pode não estar sendo executada por algum erro silencioso
- Pode haver um problema de conexão entre os nós

### **4. Função C++ SavePlayerPosition falhando**
- A função pode estar falhando na validação (`bIsAuthenticated` ou `PlayerID <= 0`)
- Pode haver erro ao criar a requisição HTTP
- O VaRest pode não estar funcionando

### **5. API PHP não está recebendo requisições**
- A URL pode estar incorreta
- O servidor PHP pode não estar rodando
- Pode haver erro de autenticação JWT

### **6. Banco de dados não está sendo atualizado**
- A query SQL pode estar falhando
- Pode haver problema de conexão com o banco

---

## 🛠️ **PASSO 1: Adicionar Logs no Blueprint SavePositionTimer**

### **Localização:** `BP_Player` → `SavePositionTimer`

**Adicionar logs ANTES de chamar `SavePlayerPosition`:**

```
SavePositionTimer (Custom Event)
  ↓
Print String: "🔵 [SavePositionTimer] Iniciado"
  ↓
Get MyGameInstance
  ↓
Print String: "🔵 [SavePositionTimer] MyGameInstance válido?" (conectar IsValid)
  ↓
Get Active Player ID
  ↓
Print String: "🔵 [SavePositionTimer] PlayerID: {PlayerID}" (Format Text)
  ↓
Get Actor Location
  ↓
Print String: "🔵 [SavePositionTimer] Position: X={X} Y={Y} Z={Z}" (Format Text)
  ↓
Save Player Position
  ↓
Print String: "🔵 [SavePositionTimer] SavePlayerPosition chamada"
```

**Como adicionar:**

1. **Após `K2Node_FunctionEntry_0` (SavePositionTimer):**
   - Adicionar `Print String`
   - Text: `"🔵 [SavePositionTimer] Iniciado"`
   - Conectar `then` do `K2Node_FunctionEntry_0` ao `execute` do `Print String`

2. **Após `Get MyGameInstance` (K2Node_VariableGet_1):**
   - Adicionar `IsValid` (Target: `MyGameInstance`)
   - Adicionar `Print String`
   - Text: `"🔵 [SavePositionTimer] MyGameInstance válido: {Result}"` (Format Text com Boolean)
   - Conectar `ReturnValue` do `IsValid` ao Format Text

3. **Após `Get Active Player ID` (K2Node_CallFunction_8):**
   - Adicionar `Format Text`
   - Format: `"🔵 [SavePositionTimer] PlayerID: {PlayerID}"`
   - Adicionar `Print String` conectado ao Format Text

4. **Após `Get Actor Location` (K2Node_CallFunction_9):**
   - Adicionar `Break Vector`
   - Adicionar `Format Text`
   - Format: `"🔵 [SavePositionTimer] Position: X={X} Y={Y} Z={Z}"`
   - Adicionar `Print String` conectado ao Format Text

5. **Após `Save Player Position` (K2Node_CallFunction_10):**
   - Adicionar `Print String`
   - Text: `"🔵 [SavePositionTimer] SavePlayerPosition chamada"`
   - Conectar `then` do `SavePlayerPosition` ao `execute` do `Print String`

---

## 🛠️ **PASSO 2: Verificar Logs da Função C++**

### **Localização:** `UmbraGameInstance.cpp` → `SavePlayerPosition`

**A função já tem logs, mas vamos verificar se estão aparecendo:**

1. **Abrir `UmbraGameInstance.cpp`**
2. **Localizar função `SavePlayerPosition` (aprox. linha 877)**
3. **Verificar se os logs estão corretos:**

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
	
	// ... resto do código
}
```

**Se os logs não aparecerem:**
- A função não está sendo chamada
- Ou está falhando na validação inicial

---

## 🛠️ **PASSO 3: Adicionar Logs na Função C++ SavePlayerPosition**

**Adicionar logs mais detalhados:**

```cpp
void UUmbraGameInstance::SavePlayerPosition(int32 PlayerID, const FVector& Position, const FString& CurrentZone)
{
	UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] 🔵 SavePlayerPosition CHAMADA - PlayerID: %d, Position: (%.2f, %.2f, %.2f), Zone: %s"), 
		PlayerID, Position.X, Position.Y, Position.Z, *CurrentZone);
	
	UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] 🔵 bIsAuthenticated: %s, PlayerID > 0: %s"), 
		bIsAuthenticated ? TEXT("true") : TEXT("false"),
		(PlayerID > 0) ? TEXT("true") : TEXT("false"));
	
	if (!bIsAuthenticated || PlayerID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ Não autenticado ou PlayerID inválido para salvar posição"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Salvando posição do player %d: X=%.2f Y=%.2f Z=%.2f"), 
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

	UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] 🔵 JSON criado - player_id: %d, pos_x: %.2f, pos_y: %.2f, pos_z: %.2f"), 
		PlayerID, Position.X, Position.Y, Position.Z);

	Request->SetRequestObject(JsonObject);
	
	UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] 🔵 Executando requisição HTTP..."));
	Request->ExecuteProcessRequest();
	
	UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] 🔵 Requisição HTTP enviada"));
}
```

---

## 🛠️ **PASSO 4: Verificar API PHP**

### **Localização:** `www/umbra_api/api/character/update_position.php`

**Adicionar logs no PHP:**

```php
<?php
error_reporting(E_ERROR | E_PARSE);
ini_set('display_errors', '0');

require_once __DIR__ . '/../config/database.php';
require_once __DIR__ . '/../config/jwt.php';

header('Content-Type: application/json');

// LOG: Recebendo requisição
error_log("🔵 [update_position.php] Requisição recebida");

$data = json_decode(file_get_contents('php://input'), true);

// LOG: Dados recebidos
error_log("🔵 [update_position.php] Dados recebidos: " . json_encode($data));

// 1. Validar Token JWT
$token = $data['token'] ?? '';
if (empty($token)) {
    error_log("⚠️ [update_position.php] Token não fornecido");
    echo json_encode(['success' => false, 'message' => 'Token não fornecido.']);
    exit;
}

$decoded = decodeJWT($token);
if (!$decoded) {
    error_log("⚠️ [update_position.php] Token inválido ou expirado");
    echo json_encode(['success' => false, 'message' => 'Token inválido ou expirado.']);
    exit;
}

$account_id = $decoded['account_id'];
error_log("🔵 [update_position.php] Account ID: " . $account_id);

// 2. Obter dados da requisição
$player_id = filter_var($data['player_id'] ?? 0, FILTER_VALIDATE_INT);
$pos_x = filter_var($data['pos_x'] ?? 0.0, FILTER_VALIDATE_FLOAT);
$pos_y = filter_var($data['pos_y'] ?? 0.0, FILTER_VALIDATE_FLOAT);
$pos_z = filter_var($data['pos_z'] ?? 0.0, FILTER_VALIDATE_FLOAT);
$current_zone = filter_var($data['current_zone'] ?? 'Tutorial', FILTER_SANITIZE_STRING);

error_log("🔵 [update_position.php] PlayerID: $player_id, Position: ($pos_x, $pos_y, $pos_z), Zone: $current_zone");

// 3. Validar dados
if (!$player_id || $player_id <= 0) {
    error_log("⚠️ [update_position.php] ID do player inválido: $player_id");
    echo json_encode(['success' => false, 'message' => 'ID do player inválido.']);
    exit;
}
if ($pos_x === false || $pos_y === false || $pos_z === false) {
    error_log("⚠️ [update_position.php] Coordenadas inválidas");
    echo json_encode(['success' => false, 'message' => 'Coordenadas de posição inválidas.']);
    exit;
}

try {
    $pdo = getDatabaseConnection();

    // 4. Verificar se o personagem pertence à conta
    $stmt = $pdo->prepare("SELECT id FROM players WHERE id = ? AND account_id = ?");
    $stmt->execute([$player_id, $account_id]);
    if ($stmt->rowCount() === 0) {
        error_log("⚠️ [update_position.php] Personagem não encontrado ou não pertence à conta");
        echo json_encode(['success' => false, 'message' => 'Personagem não encontrado ou não pertence a esta conta.']);
        exit;
    }

    // 5. Atualizar posição e zona do player
    error_log("🔵 [update_position.php] Executando UPDATE...");
    $stmt = $pdo->prepare("UPDATE players SET pos_x = ?, pos_y = ?, pos_z = ?, current_zone = ?, last_played_at = NOW() WHERE id = ?");
    $stmt->execute([$pos_x, $pos_y, $pos_z, $current_zone, $player_id]);

    if ($stmt->rowCount() > 0) {
        error_log("✅ [update_position.php] Posição atualizada com sucesso - Rows affected: " . $stmt->rowCount());
        echo json_encode(['success' => true, 'message' => 'Posição do player atualizada com sucesso.']);
    } else {
        error_log("⚠️ [update_position.php] Nenhuma alteração - Rows affected: " . $stmt->rowCount());
        echo json_encode(['success' => false, 'message' => 'Nenhuma alteração na posição do player ou player não encontrado.']);
    }

} catch (PDOException $e) {
    error_log("❌ [update_position.php] Erro no banco: " . $e->getMessage());
    echo json_encode(['success' => false, 'message' => 'Erro interno do servidor.']);
}
?>
```

**Verificar logs do PHP:**
- **WAMP:** `C:\wamp64\logs\php_error.log`
- **XAMPP:** `C:\xampp\apache\logs\error.log`
- **Ou:** Verificar logs do servidor web configurado

---

## 🛠️ **PASSO 5: Verificar Timer no BeginPlay**

### **Localização:** `BP_Player` → `BeginPlay`

**Verificar se o timer está sendo configurado corretamente:**

1. **Verificar se `Set Timer by Function Name` está conectado:**
   - Deve estar após `Set Actor Location` (quando posição válida)
   - Function Name: `"SavePositionTimer"`
   - Time: `5.0`
   - Looping: `True`

2. **Adicionar log após configurar timer:**
   ```
   Set Timer by Function Name
     ↓
   Print String: "🔵 [BeginPlay] Timer SavePositionTimer configurado (5s, looping)"
   ```

---

## 📊 **CHECKLIST DE DIAGNÓSTICO:**

Execute os testes na ordem e marque o que aparece nos logs:

### **Teste 1: Timer está sendo executado?**
- [ ] Log `"🔵 [SavePositionTimer] Iniciado"` aparece a cada 5 segundos?
- [ ] Se NÃO: Timer não está funcionando

### **Teste 2: MyGameInstance está válido?**
- [ ] Log `"🔵 [SavePositionTimer] MyGameInstance válido: true"` aparece?
- [ ] Se NÃO: MyGameInstance está null

### **Teste 3: PlayerID está válido?**
- [ ] Log `"🔵 [SavePositionTimer] PlayerID: {número > 0}"` aparece?
- [ ] Se PlayerID = 0: GetActivePlayerID está retornando 0

### **Teste 4: Posição está sendo obtida?**
- [ ] Log `"🔵 [SavePositionTimer] Position: X={X} Y={Y} Z={Z}"` mostra valores diferentes de (0,0,0)?
- [ ] Se Position = (0,0,0): Get Actor Location está retornando zero

### **Teste 5: SavePlayerPosition está sendo chamada?**
- [ ] Log `"🔵 [SavePositionTimer] SavePlayerPosition chamada"` aparece?
- [ ] Se NÃO: Há erro antes de chamar a função

### **Teste 6: Função C++ está sendo executada?**
- [ ] Log `"[UmbraGameInstance] 🔵 SavePlayerPosition CHAMADA"` aparece?
- [ ] Se NÃO: A função não está sendo chamada do Blueprint

### **Teste 7: Validação C++ está passando?**
- [ ] Log `"[UmbraGameInstance] 🔵 bIsAuthenticated: true"` aparece?
- [ ] Log `"[UmbraGameInstance] 🔵 PlayerID > 0: true"` aparece?
- [ ] Se NÃO: Validação está falhando

### **Teste 8: Requisição HTTP está sendo enviada?**
- [ ] Log `"[UmbraGameInstance] 🔵 Executando requisição HTTP..."` aparece?
- [ ] Log `"[UmbraGameInstance] 🔵 Requisição HTTP enviada"` aparece?
- [ ] Se NÃO: Erro ao criar requisição ou executar

### **Teste 9: API PHP está recebendo requisição?**
- [ ] Log `"🔵 [update_position.php] Requisição recebida"` aparece no log do PHP?
- [ ] Se NÃO: Requisição não está chegando ao servidor

### **Teste 10: Banco de dados está sendo atualizado?**
- [ ] Log `"✅ [update_position.php] Posição atualizada com sucesso"` aparece?
- [ ] Se NÃO: Query SQL está falhando

---

## 🎯 **PRÓXIMOS PASSOS:**

1. **Adicionar todos os logs acima**
2. **Executar o jogo e movimentar o player**
3. **Aguardar 5-10 segundos**
4. **Verificar logs do Unreal Engine (Output Log)**
5. **Verificar logs do PHP (error_log)**
6. **Identificar onde está falhando usando o checklist**
7. **Corrigir o problema identificado**

---

## 📝 **NOTAS:**

- **Logs do Unreal Engine:** Abrir `Window` → `Developer Tools` → `Output Log`
- **Filtrar logs:** Buscar por `"🔵"` ou `"SavePositionTimer"` ou `"UmbraGameInstance"`
- **Logs do PHP:** Verificar arquivo de log do servidor web configurado

