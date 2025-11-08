# 🎯 **IMPLEMENTAÇÃO BLUEPRINT: Sistema de Salvar Posição**

## 📋 **OBJETIVO:**

Implementar 3 funcionalidades:
1. **Aplicar posição salva no spawn** do personagem
2. **Salvar posição periodicamente** (a cada 5 segundos)
3. **Salvar posição ao desconectar**

---

## ✅ **PARTE 1: Aplicar Posição Salva no Spawn**

### **Blueprint: `BP_Player` (ou Character Principal)**

#### **PASSO 1.1: Bind ao Delegate `OnCharacterSelected`**

**NO `Event Graph` → `Event BeginPlay`:**

```
Event BeginPlay
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Bind Event to OnCharacterSelected
  ↓
[Create Event] → Nome: "OnCharacterSelectedEvent"
  - Parâmetro: SelectedPlayer (UmbraPlayerData)
```

**COMO FAZER:**
1. Após `Cast to Umbra Game Instance` → `Branch True`
2. Arraste do pino de saída → Busque: `Bind Event to OnCharacterSelected`
3. Clique no pino `Event` → `Create Event`
4. Nome do Event: `OnCharacterSelectedEvent`
5. O Event terá um parâmetro `SelectedPlayer` (tipo `UmbraPlayerData`)

---

#### **PASSO 1.2: Implementar Lógica no Event**

**NO Event `OnCharacterSelectedEvent` criado:**

```
OnCharacterSelectedEvent (Custom Event)
  Parâmetro: SelectedPlayer (UmbraPlayerData)
  ↓
Break UmbraPlayerData
  ↓
Get Position (FVector)
  ↓
Not Equal (Vector)
  - A: Position
  - B: (0, 0, 0)
  → Boolean
  ↓
Branch
  ├─ True: Position válida
  │    ↓
  │   Set Actor Location
  │     - Target: Self
  │     - New Location: Position
  │     - bTeleport: True ← CRÍTICO!
  │   ↓
  │   Format Text: "✅ Posição aplicada: ({0}, {1}, {2})"
  │     - {0} = Position.X
  │     - {1} = Position.Y
  │     - {2} = Position.Z
  │   ↓
  │   Print String
  │
  └─ False: Position inválida
       ↓
      Print String: "⚠️ Posição inválida (0,0,0), usando padrão"
```

**COMO FAZER:**
1. No Event `OnCharacterSelectedEvent`, arraste o pino `SelectedPlayer`
2. Busque: `Break UmbraPlayerData`
3. Do `Break`, pegue `Position` (FVector)
4. Adicione `Not Equal (Vector)` comparando com `(0, 0, 0)`
5. Adicione `Branch`
6. No `True`: `Set Actor Location` com `bTeleport = True`

---

## ✅ **PARTE 2: Salvar Posição Periodicamente**

### **Blueprint: `BP_Player`**

#### **PASSO 2.1: Criar Custom Event `SavePositionTimer`**

**NO `Event Graph`:**

```
SavePositionTimer (Custom Event)
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Get Actor Location
  ↓
Save Player Position
  - PlayerID: Get Active Player ID
  - Position: Get Actor Location
  - CurrentZone: "Tutorial" (ou variável)
```

**COMO FAZER:**
1. Clique direito → `Add Custom Event`
2. Nome: `SavePositionTimer`
3. Implemente a lógica acima

---

#### **PASSO 2.2: Iniciar Timer no BeginPlay**

**NO `Event BeginPlay`, APÓS aplicar posição:**

```
[Após aplicar posição ou após OnCharacterSelectedEvent]
  ↓
Set Timer by Function Name
  - Function Name: SavePositionTimer
  - Time: 5.0 (segundos)
  - Looping: True
  ↓
Set SavePositionTimerHandle (variável Timer Handle)
```

**COMO FAZER:**
1. Após o `OnCharacterSelectedEvent` (ou no final do `BeginPlay`)
2. Adicione `Set Timer by Function Name`
3. `Function Name`: `SavePositionTimer`
4. `Time`: `5.0` (salvar a cada 5 segundos)
5. `Looping`: `True`
6. Salve o `Return Value` em uma variável `SavePositionTimerHandle` (Timer Handle)

**CRIAR VARIÁVEL:**
- Nome: `SavePositionTimerHandle`
- Tipo: `Timer Handle`
- Valor padrão: Vazio

---

## ✅ **PARTE 3: Salvar Posição ao Desconectar**

### **Blueprint: `BP_NetMovementClient`**

#### **PASSO 3.1: Modificar `Event EndPlay`**

**NO `Event Graph` → `Event EndPlay`:**

```
Event EndPlay
  Parâmetro: EndPlayReason
  ↓
Get First Player Controller
  ↓
Get Pawn
  ↓
Is Valid
  ↓
Branch → True
  ↓
Get Actor Location
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Save Player Position
  - PlayerID: Get Active Player ID
  - Position: Get Actor Location
  - CurrentZone: "Tutorial"
  ↓
[Continuar com fechamento do WebSocket]
```

**COMO FAZER:**
1. No `Event EndPlay` (já deve existir)
2. **ANTES** de fechar o WebSocket, adicione a lógica acima
3. Isso garante que a última posição seja salva antes de desconectar

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **BP_Player:**
- [ ] Bind ao `OnCharacterSelected` no `BeginPlay`
- [ ] Event `OnCharacterSelectedEvent` criado
- [ ] Lógica de aplicar posição implementada
- [ ] Custom Event `SavePositionTimer` criado
- [ ] Timer iniciado no `BeginPlay` (5 segundos, looping)
- [ ] Variável `SavePositionTimerHandle` criada

### **BP_NetMovementClient:**
- [ ] `Event EndPlay` modificado
- [ ] Lógica de salvar posição antes de desconectar

---

## 🧪 **TESTE:**

### **TESTE 1: Posição no Spawn**
1. Selecione um personagem com posição salva no banco
2. Verifique logs: `✅ Posição aplicada: (721.00, 1786.00, 92.00)`
3. Personagem deve spawnar na posição correta

### **TESTE 2: Salvamento Periódico**
1. Mova o personagem
2. Aguarde 5 segundos
3. Verifique logs do servidor: `SavePlayerPosition CHAMADA`
4. Verifique banco de dados: Posição deve estar atualizada

### **TESTE 3: Salvamento ao Desconectar**
1. Mova o personagem
2. Feche o jogo (ESC)
3. Verifique logs: `SavePlayerPosition CHAMADA`
4. Verifique banco de dados: Última posição deve estar salva

---

## ⚠️ **PONTOS CRÍTICOS:**

1. **`bTeleport = True`** no `Set Actor Location` (importante para evitar interpolação)
2. **Timer `Looping = True`** para salvar periodicamente
3. **Salvar ANTES de fechar WebSocket** no `EndPlay`
4. **Validar Position != (0,0,0)`** antes de aplicar

---

**Status:** 🚀 **PRONTO PARA IMPLEMENTAR**

