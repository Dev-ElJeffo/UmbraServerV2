# 🔧 SOLUÇÃO ALTERNATIVA: Usar FUNÇÃO ao invés de Custom Event

## 🎯 O PROBLEMA

Quando você usa "Assign" ou tenta criar um Custom Event para receber dados de um delegate com múltiplos parâmetros, o Unreal Engine cria OUTPUTS ao invés de INPUTS.

**Isso acontece porque:**
- Custom Events podem ser usados como Event Dispatchers (OUTPUTS)
- O Unreal Engine pode confundir o propósito do Custom Event

---

## ✅ SOLUÇÃO: Usar uma FUNÇÃO (Function) ao invés de Custom Event

**Funções SEMPRE têm INPUTS** e podem ser conectadas a delegates usando `AddDynamic`.

---

## 📋 PASSO A PASSO COMPLETO

### **PASSO 1: Criar uma FUNÇÃO (não Custom Event)**

1. **No `BP_NetMovementClient`:**
   - Abra o Blueprint
   - No painel **"My Blueprint"**, vá para a aba **"Functions"**
   - Clique em **"+ Function"** (não "Add Custom Event"!)

2. **Nome da Função:**
   - `HandlePlayerInspected`

3. **Adicionar INPUTS:**
   - Clique na função criada
   - No painel **"Details"**, na seção **"Inputs"**, clique em **"+ Input"**
   - **Adicione:**
     - `PlayerID` (tipo: **Integer**)
     - `PlayerInfo` (tipo: **FUmbraCharacterInfo**)

4. **VERIFICAR:**
   - Os pinos devem estar na **ESQUERDA** (Inputs)
   - Funções **SEMPRE** têm INPUTS, nunca OUTPUTS (exceto Return Value)

---

### **PASSO 2: Conectar o Delegate usando "AddDynamic"**

1. **No Event Graph (onde você quer conectar, ex: `Event BeginPlay` ou `ProcessNextFrame`):**
   - **Right Click** → `Get Game Instance`
   - **Right Click** → `Cast to UmbraGameInstance`
   - **Conecte** o "Return Value" do Cast ao "Object" do Cast

2. **Obter o Delegate:**
   - **Arraste o pino `OnPlayerInspected`** (delegate, vermelho) do `As Umbra Game Instance`
   - **IMPORTANTE:** Você verá várias opções:
     - ❌ `Call OnPlayerInspected` (executa imediatamente - NÃO use)
     - ❌ `Assign OnPlayerInspected` (substitui listeners - NÃO use)
     - ✅ **`AddDynamic`** ou **`AddDynamic OnPlayerInspected`** (USE ESTE!)

3. **Selecionar "AddDynamic":**
   - Isso criará um nó `AddDynamic OnPlayerInspected`
   - O nó terá um pino **"Function"** (ou "Delegate")

4. **Conectar à Função:**
   - **Arraste do pino "Function"** do nó `AddDynamic`
   - **Solte no gráfico**
   - **No menu**, procure por `HandlePlayerInspected` (sua Função criada no Passo 1)
   - **Selecione** a Função

   **OU:**
   - **Arraste o nó `HandlePlayerInspected`** (Função) para perto do `AddDynamic`
   - **Conecte o pino "Function"** do `AddDynamic` ao **pino de execução (branco)** da Função
   - **Conecte os pinos de dados:**
     - `PlayerID` (output do AddDynamic) → `PlayerID` (input da Função)
     - `PlayerInfo` (output do AddDynamic) → `PlayerInfo` (input da Função)

---

### **PASSO 3: Implementar a Lógica na Função**

**Dentro da Função `HandlePlayerInspected`:**

1. **Os dados já estarão disponíveis nos parâmetros:**
   - `PlayerID` (Integer) - input
   - `PlayerInfo` (FUmbraCharacterInfo) - input

2. **Quebrar o struct:**
   - **Right Click** no pino `PlayerInfo` → `Break FUmbraCharacterInfo`
   - Isso expõe todos os campos: `CharacterName`, `Level`, `CurrentHealth`, `MaxHealthTotal`, etc.

3. **Criar o struct `FUmbraRemotePlayerInfo`:**
   - **Right Click** → `Make FUmbraRemotePlayerInfo`
   - **Preencha os campos:**
     - `PlayerID`: Conecte o `PlayerID` (input da função)
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
     - `PlayerID`: `PlayerID` (input da função)
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

3. **Na Função `HandlePlayerInspected`:**
   - **Find in Map:**
     - `Map`: `RemotePlayerActors`
     - `Key`: `PlayerID` (input da função)
     - `Value`: Use este como `RemoteActor` no `Make FUmbraRemotePlayerInfo`

---

## 🔍 DIFERENÇA ENTRE FUNÇÃO E CUSTOM EVENT

### **Custom Event:**
- Pode ser usado como Event Dispatcher (OUTPUTS)
- Pode ser chamado de qualquer lugar
- O Unreal Engine pode confundir o propósito

### **Função:**
- **SEMPRE** tem INPUTS (exceto Return Value)
- Pode ser chamada de qualquer lugar
- **NÃO** pode ser usada como Event Dispatcher
- Funciona perfeitamente com `AddDynamic` para delegates

---

## ✅ VERIFICAÇÃO FINAL

Após implementar:

1. ✅ A Função `HandlePlayerInspected` tem **INPUTS** (não OUTPUTS)
2. ✅ O delegate está conectado usando **"AddDynamic"** (não "Assign")
3. ✅ Os pinos de dados do `AddDynamic` estão conectados aos INPUTS da Função
4. ✅ O `RemoteActor` é obtido do Map ou variável temporária
5. ✅ O `FUmbraRemotePlayerInfo` é criado com dados reais da API
6. ✅ `RegisterRemotePlayer` é chamado com os dados corretos

---

## 🆘 SE "AddDynamic" NÃO APARECER

Se você não ver "AddDynamic" quando arrasta o delegate:

1. **Tente digitar "AddDynamic"** no menu de busca
2. **Ou use "Add"** se disponível
3. **Ou conecte manualmente:**
   - Crie a Função primeiro (Passo 1)
   - Arraste o delegate → Selecione qualquer opção que tenha "Add"
   - Conecte o pino "Function" ou "Event" à Função
   - Conecte os pinos de dados manualmente

---

## 📝 RESUMO

**A solução é simples:**
1. **Crie uma FUNÇÃO** (não Custom Event) com INPUTS
2. **Use "AddDynamic"** para conectar o delegate à Função
3. **Conecte os pinos de dados** manualmente se necessário

**Por que funciona:**
- Funções sempre têm INPUTS
- `AddDynamic` é específico para Dynamic Multicast Delegates
- Não há confusão entre Event Dispatcher e receptor

---

**FIM DO GUIA**
