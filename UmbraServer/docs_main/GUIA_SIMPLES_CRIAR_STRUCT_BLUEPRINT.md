# 🔧 SOLUÇÃO DEFINITIVA: Delegate com 2 Parâmetros

## 🎯 O PROBLEMA REAL

Quando você usa **"Assign"** em um delegate com múltiplos parâmetros, o Unreal Engine cria um Custom Event com **OUTPUTS** (Event Dispatcher), não INPUTS (receptor).

**O XML que você enviou mostra:**
- `PlayerID` e `PlayerInfo` como **OUTPUTS** (`Direction="EGPD_Output"`)
- Isso significa que o Custom Event está sendo usado como **emissor**, não **receptor**

**Quando você tenta criar um Custom Event manualmente com INPUTS e usar "Assign", dá erro porque:**
- "Assign" **substitui** todos os listeners anteriores
- "Assign" espera um Event Dispatcher (OUTPUTS), não um receptor (INPUTS)

---

## ✅ SOLUÇÃO CORRETA: Usar "Add" ao invés de "Assign"

### **DIFERENÇA FUNDAMENTAL:**

- **"Assign"**: Substitui todos os listeners. Pode criar Custom Event com OUTPUTS.
- **"Add"**: Adiciona um listener à lista. Funciona com Custom Events que têm INPUTS.

---

## 📋 PASSO A PASSO COMPLETO

### **PASSO 1: Criar Custom Event Manualmente (COM INPUTS)**

1. **No Event Graph do `BP_NetMovementClient`:**
   - **Right Click** → `Add Custom Event`
   - **Nome:** `HandlePlayerInspected`

2. **Configurar INPUTS:**
   - **Clique no nó do Custom Event** para abrir o painel "Details"
   - **Na seção "Inputs"**, clique em **"+ Input"**
   - **Adicione:**
     - `PlayerID` (tipo: **Integer**)
     - `PlayerInfo` (tipo: **FUmbraCharacterInfo**)

3. **VERIFICAR:**
   - Os pinos devem estar na **ESQUERDA** (Inputs)
   - **NÃO** devem estar na direita (Outputs)

---

### **PASSO 2: Conectar o Delegate usando "Add Custom Event" (NÃO "Assign")**

**IMPORTANTE:** Para Dynamic Multicast Delegates, você deve usar **"Add Custom Event"** diretamente, não "Assign".

1. **No Event Graph (onde você quer conectar, ex: `Event BeginPlay` ou `ProcessNextFrame`):**
   - **Right Click** → `Get Game Instance`
   - **Right Click** → `Cast to UmbraGameInstance`
   - **Conecte** o "Return Value" do Cast ao "Object" do Cast

2. **Obter o Delegate e Criar Custom Event Automaticamente:**
   - **Arraste o pino `OnPlayerInspected`** (delegate, vermelho) do `As Umbra Game Instance`
   - **IMPORTANTE:** Você verá várias opções:
     - ❌ `Call OnPlayerInspected` (executa imediatamente - NÃO use)
     - ❌ `Assign OnPlayerInspected` (substitui listeners e cria OUTPUTS - NÃO use)
     - ✅ **`Add Custom Event for OnPlayerInspected`** ou **`Add OnPlayerInspected`** (USE ESTE!)

3. **Selecionar "Add Custom Event":**
   - Isso criará **automaticamente** um Custom Event com o nome `OnPlayerInspected` (ou similar)
   - **O Custom Event criado automaticamente terá INPUTS corretos:**
     - `PlayerID` (Integer) - INPUT
     - `PlayerInfo` (FUmbraCharacterInfo) - INPUT

4. **Renomear o Custom Event (Opcional):**
   - Clique no nó do Custom Event criado
   - No painel "Details", altere o nome para `HandlePlayerInspected` (ou mantenha o nome padrão)

5. **Verificar os INPUTS:**
   - Clique no nó do Custom Event
   - No painel "Details" → "Inputs"
   - Deve ter `PlayerID` (Integer) e `PlayerInfo` (FUmbraCharacterInfo) como **INPUTS** (não OUTPUTS)

**ALTERNATIVA (se "Add Custom Event" não aparecer):**

Se você não ver "Add Custom Event", mas apenas "Assign":

1. **Crie o Custom Event manualmente primeiro** (Passo 1)
2. **Arraste o delegate** → Selecione **"AddDynamic"** ou **"Add"** (se disponível)
3. **Conecte o pino "Event"** do nó `Add` ao **pino de execução (branco)** do Custom Event
4. **Conecte os pinos de dados:**
   - `PlayerID` (output do Add) → `PlayerID` (input do Custom Event)
   - `PlayerInfo` (output do Add) → `PlayerInfo` (input do Custom Event)

---

### **PASSO 3: Implementar a Lógica no Custom Event**

**Dentro do Custom Event `HandlePlayerInspected`:**

1. **Os dados já estarão disponíveis nos parâmetros:**
   - `PlayerID` (Integer) - input
   - `PlayerInfo` (FUmbraCharacterInfo) - input

2. **Quebrar o struct:**
   - **Right Click** no pino `PlayerInfo` → `Break FUmbraCharacterInfo`
   - Isso expõe todos os campos: `CharacterName`, `Level`, `CurrentHealth`, `MaxHealthTotal`, etc.

3. **Criar o struct `FUmbraRemotePlayerInfo`:**
   - **Right Click** → `Make FUmbraRemotePlayerInfo`
   - **Preencha os campos:**
     - `PlayerID`: Conecte o `PlayerID` (input do evento)
     - `PlayerName`: Conecte `CharacterName` (do Break)
     - `Level`: Conecte `Level` (do Break)
     - `CurrentHealth`: Conecte `CurrentHealth` (do Break)
     - `MaxHealth`: Conecte `MaxHealthTotal` (do Break)
     - `CurrentMana`: Conecte `CurrentMana` (do Break)
     - `MaxMana`: Conecte `MaxManaTotal` (do Break)
     - `RemoteActor`: Você precisa obter o actor do player. Veja Passo 4.

4. **Registrar no `UmbraPlayerSelectionComponent`:**
   - **Get Player Controller**
   - **Get Component by Class** → `UmbraPlayerSelectionComponent`
   - **Call Function** → `RegisterRemotePlayer`
     - `PlayerID`: `PlayerID` (input do evento)
     - `PlayerInfo`: O struct `FUmbraRemotePlayerInfo` criado acima

---

### **PASSO 4: Obter o RemoteActor**

**O problema:** Você precisa do `RemoteActor` (AActor*) para criar o `FUmbraRemotePlayerInfo`.

**Solução:** Você já tem o `RemoteActor` no `ProcessNextFrame` quando spawna o actor. Você precisa armazená-lo em uma variável ou Map.

**Opção A: Usar um Map (Recomendado)**

1. **Criar variável no `BP_NetMovementClient`:**
   - **Tipo:** `Map` → Key: `Integer` (PlayerID), Value: `Actor` (AActor*)
   - **Nome:** `RemotePlayerActors`

2. **No `ProcessNextFrame`, após spawnar o actor:**
   - **Add to Map:**
     - `Map`: `RemotePlayerActors`
     - `Key`: `PlayerID` (do frame)
     - `Value`: O actor spawnado

3. **No `HandlePlayerInspected`:**
   - **Find in Map:**
     - `Map`: `RemotePlayerActors`
     - `Key`: `PlayerID` (input do evento)
     - `Value`: Use este como `RemoteActor` no `Make FUmbraRemotePlayerInfo`

**Opção B: Passar o Actor como parâmetro (Alternativa)**

Se você chamar `InspectPlayer` logo após spawnar o actor, você pode armazenar o actor em uma variável temporária e usá-la no `HandlePlayerInspected`.

---

## 🔍 ANÁLISE DO FLUXO COMPLETO

### **Fluxo Correto:**

```
[ProcessNextFrame]
    ↓
[Spawn Remote Actor]
    ↓
[Add to Map: RemotePlayerActors]
    Key: PlayerID, Value: Actor
    ↓
[Get Game Instance] → [Cast to UmbraGameInstance]
    ↓
[InspectPlayer]
    PlayerID: PlayerID do frame
    ↓
[API retorna dados]
    ↓
[OnPlayerInspected.Broadcast(PlayerID, PlayerInfo)]
    ↓
[HandlePlayerInspected] (Custom Event - INPUTS)
    PlayerID: (recebido)
    PlayerInfo: (recebido)
    ↓
[Break FUmbraCharacterInfo]
    ↓
[Find in Map: RemotePlayerActors]
    Key: PlayerID
    ↓
[Make FUmbraRemotePlayerInfo]
    PlayerID: PlayerID
    PlayerName: CharacterName
    Level: Level
    CurrentHealth: CurrentHealth
    MaxHealth: MaxHealthTotal
    CurrentMana: CurrentMana
    MaxMana: MaxManaTotal
    RemoteActor: (do Map)
    ↓
[RegisterRemotePlayer]
    PlayerID: PlayerID
    PlayerInfo: (struct criado)
```

---

## ⚠️ POR QUE "ADD CUSTOM EVENT" FUNCIONA E "ASSIGN" NÃO

### **"Assign":**
- **Substitui** todos os listeners anteriores
- Cria Custom Event automaticamente com **OUTPUTS** (Event Dispatcher)
- O Custom Event criado é usado para **emitir** dados, não **receber**
- Não funciona para receber dados de delegates

### **"Add Custom Event" ou "Add":**
- **Adiciona** um listener à lista (não substitui)
- Cria Custom Event automaticamente com **INPUTS** corretos
- O Custom Event criado é usado para **receber** dados do delegate
- Permite múltiplos listeners no mesmo delegate
- Os dados são passados automaticamente para os INPUTS quando o delegate dispara

---

## ✅ VERIFICAÇÃO FINAL

Após implementar:

1. ✅ O Custom Event `HandlePlayerInspected` tem **INPUTS** (não OUTPUTS)
2. ✅ O delegate está conectado usando **"Add"** (não "Assign")
3. ✅ Os pinos de dados do `Add` estão conectados aos INPUTS do Custom Event
4. ✅ O `RemoteActor` é obtido do Map ou variável temporária
5. ✅ O `FUmbraRemotePlayerInfo` é criado com dados reais da API
6. ✅ `RegisterRemotePlayer` é chamado com os dados corretos

---

## 🆘 SE AINDA NÃO FUNCIONAR

Se você ainda tiver problemas:

1. **Verifique se o Custom Event tem INPUTS:**
   - Clique no nó → Details → Inputs
   - Deve ter `PlayerID` (Integer) e `PlayerInfo` (FUmbraCharacterInfo)

2. **Verifique a conexão:**
   - O pino "Event" do `Add OnPlayerInspected` deve estar conectado ao pino de execução (branco) do Custom Event
   - Os pinos de dados (`PlayerID`, `PlayerInfo`) do `Add` devem estar conectados aos INPUTS do Custom Event

3. **Teste se o delegate está disparando:**
   - Adicione um `Print String` no início do `HandlePlayerInspected`
   - Chame `InspectPlayer` manualmente
   - Se o print aparecer, o delegate está funcionando

4. **Verifique se `InspectPlayer` está sendo chamado:**
   - Adicione um `Print String` antes de chamar `InspectPlayer` no `ProcessNextFrame`
   - Verifique se a API está retornando dados corretos

---

## 🔄 SOLUÇÃO ALTERNATIVA: Usar FUNÇÃO ao invés de Custom Event

**Se o problema persistir, use uma FUNÇÃO ao invés de Custom Event:**

1. **Crie uma FUNÇÃO** (não Custom Event) com INPUTS
2. **Use "AddDynamic"** para conectar o delegate à Função
3. **Funções sempre têm INPUTS**, então não há confusão com OUTPUTS

**Veja o guia completo:** `GUIA_SOLUCAO_ALTERNATIVA_FUNCAO_DELEGATE.md`

---

**FIM DO GUIA**
