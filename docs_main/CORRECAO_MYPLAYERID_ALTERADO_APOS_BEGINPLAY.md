# 🔧 **CORREÇÃO: MyPlayerId Alterado Após BeginPlay**

## 📋 **PROBLEMA IDENTIFICADO:**

**EVIDÊNCIA DOS LOGS:**
```
LogTemp: [UmbraGameInstance] ✅ Personagem selecionado: ElJeffo (ID: 1, Level: 1)
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Active Player ID:1  ← CORRETO no BeginPlay
LogTemp: [BuildMoveUpdateFrame] PlayerID recebido: 18  ← INCORRETO ao enviar
```

**PROBLEMA:**
- ✅ `BeginPlay` seta `MyPlayerId = 1` corretamente
- ❌ Mas quando `SendMoveUpdate` é chamado, `MyPlayerId` tem valor **18**
- ❌ Isso indica que `MyPlayerId` está sendo alterado **APÓS** o `BeginPlay`

**CAUSA RAIZ:**
O `BeginPlay` está sendo executado **ANTES** do `SelectCharacter`, então:
1. `BeginPlay` executa → `Get Active Player ID` retorna valor antigo (18)
2. `MyPlayerId` é setado com 18
3. `SelectCharacter` é chamado → `ActivePlayerID` muda para 1
4. Mas `MyPlayerId` **NÃO** é atualizado
5. `SendMoveUpdate` usa `MyPlayerId = 18` (valor antigo)

---

## ✅ **SOLUÇÃO:**

### **OPÇÃO 1: Atualizar MyPlayerId no WebSocket Connected (Recomendado)**

Quando o WebSocket conecta, atualizar `MyPlayerId` novamente:

**NO BLUEPRINT `BP_NetMovementClient`:**

```
Event OnWSConnected (ou callback de conexão)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Print String: "WebSocket Connected - Atualizando MyPlayerId: [Return Value]"
  ↓
Set MyPlayerId = [Return Value do Get Active Player ID]
  ↓
Get MyPlayerId
  ↓
Print String: "MyPlayerId atualizado após conexão: [Get MyPlayerId]"
```

**VANTAGEM:**
- Garante que `MyPlayerId` está atualizado quando o WebSocket conecta
- WebSocket conecta **APÓS** o personagem ser selecionado

---

### **OPÇÃO 2: Atualizar MyPlayerId no SendMoveUpdate (Mais Seguro)**

Sempre ler o `Active Player ID` diretamente no `SendMoveUpdate`:

**NO BLUEPRINT `BP_NetMovementClient` - `SendMoveUpdate`:**

```
SendMoveUpdate (Custom Event)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Print String: "SendMoveUpdate - Usando Active Player ID: [Return Value]"
  ↓
BuildMoveUpdateFrame
  - PlayerId: [Return Value do Get Active Player ID] ← LER DIRETO, não usar MyPlayerId
  - Location: ...
  - YawDegrees: ...
  - TimestampMs: ...
```

**VANTAGEM:**
- Sempre usa o valor mais atualizado
- Não depende de variável que pode estar desatualizada

---

### **OPÇÃO 3: Usar Delegate/Event para Atualizar MyPlayerId**

No `UmbraGameInstance`, após `SelectCharacter`, disparar um evento:

**NO C++ `UmbraGameInstance.cpp`:**
```cpp
// Declarar delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerIDChanged, int32, NewPlayerID);

// Em SelectCharacter, após ActivePlayerID ser setado:
OnPlayerIDChanged.Broadcast(ActivePlayerID);
```

**NO BLUEPRINT `BP_NetMovementClient` - `BeginPlay`:**
```
Event BeginPlay
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Bind Event to OnPlayerIDChanged
  ↓
Event OnPlayerIDChanged (NewPlayerID)
  ↓
Print String: "PlayerID mudou para: [NewPlayerID]"
  ↓
Set MyPlayerId = [NewPlayerID]
```

**VANTAGEM:**
- Atualização automática quando `ActivePlayerID` muda
- Não depende de timing

---

## 🚀 **RECOMENDAÇÃO:**

**Usar OPÇÃO 2 (Mais Segura):**

Ler `Active Player ID` diretamente no `SendMoveUpdate` em vez de usar `MyPlayerId`. Isso garante que sempre usamos o valor mais atualizado, independente de quando `BeginPlay` foi executado.

**MODIFICAÇÃO NO BLUEPRINT:**

1. Abra `BP_NetMovementClient`
2. Abra a função `SendMoveUpdate`
3. **REMOVER** a conexão de `Get MyPlayerId` → `BuildMoveUpdateFrame.PlayerId`
4. **ADICIONAR:**
   ```
   Get Game Instance
     ↓
   Cast to Umbra Game Instance
     ↓ (Branch True)
   Get Active Player ID
     ↓
   BuildMoveUpdateFrame.PlayerId = [Return Value do Get Active Player ID]
   ```

---

## 📊 **RESULTADO ESPERADO:**

Após a correção:

**LOGS DO CLIENTE:**
```
LogTemp: [UmbraGameInstance] ✅ Personagem selecionado: ElJeffo (ID: 1, Level: 1)
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] Active Player ID:1
LogTemp: [BuildMoveUpdateFrame] PlayerID recebido: 1  ← CORRETO
LogTemp: [BuildMoveUpdateFrame] Frame bytes [0-4]: 01 01 00 00 00 (PlayerID=1 serializado)
```

**LOGS DO SERVIDOR:**
```
[debug] Received MoveUpdate from client X: player_id=1
```

---

## ⚠️ **IMPORTANTE:**

**Por que o BeginPlay mostra "Active Player ID:1" mas BuildMoveUpdateFrame recebe 18?**

Possíveis causas:
1. **`BeginPlay` executa antes de `SelectCharacter`** → pega valor antigo (18) do personagem anterior
2. **`MyPlayerId` está sendo setado em outro lugar** com valor 18
3. **Há múltiplas instâncias** do Blueprint e uma está com valor errado

**A solução mais segura é ler `Active Player ID` diretamente no `SendMoveUpdate`, sem depender de `MyPlayerId`.**

