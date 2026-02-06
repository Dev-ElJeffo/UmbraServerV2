# ✅ INSTRUÇÕES EXATAS: Adicionar PlayerInfoUpdate no OnWSBinaryMessage

## 🎯 OBJETIVO
Adicionar processamento de mensagens `PlayerInfoUpdate` (tipo 4) no evento `OnWSBinaryMessage` do Blueprint `BP_NetMovementClient2`.

---

## 📍 ONDE ADICIONAR

**Localização:** No evento `OnWSBinaryMessage` do Blueprint `BP_NetMovementClient2`

**Posição:** **ANTES** do bloco que verifica `Data[0] == 3` (PlayerDisconnected)

---

## 🔧 PASSO A PASSO DETALHADO

### **PASSO 1: Adicionar Verificação do Tipo 4 (PlayerInfoUpdate)**

**Onde:** Logo após o `K2Node_Knot_33` (que recebe o `Data` do evento), **ANTES** do `K2Node_GetArrayItem_0` que verifica `Data[0] == 3`

**O que fazer:**

1. **Criar um novo `K2Node_GetArrayItem`** (similar ao existente):
   - **Nome do nó:** `K2Node_GetArrayItem_PlayerInfoUpdate`
   - **Array:** Conecte ao `OutputPin` do `K2Node_Knot_33` (mesmo que o `K2Node_GetArrayItem_0` existente)
   - **Dimension 1:** `0` (para pegar o primeiro byte, que é o msgType)
   - **Output:** Será um `byte` (o tipo de mensagem)

2. **Criar um `K2Node_PromotableOperator` (EqualEqual_ByteByte)**:
   - **Nome do nó:** `K2Node_PromotableOperator_PlayerInfoUpdate`
   - **A:** Conecte ao `Output` do `K2Node_GetArrayItem_PlayerInfoUpdate`
   - **B:** `4` (valor fixo, tipo PlayerInfoUpdate)
   - **ReturnValue:** Será um `bool` (true se o tipo for 4)

3. **Criar um `K2Node_IfThenElse`**:
   - **Nome do nó:** `K2Node_IfThenElse_PlayerInfoUpdate`
   - **Condition:** Conecte ao `ReturnValue` do `K2Node_PromotableOperator_PlayerInfoUpdate`
   - **then:** (executará se for tipo 4)
   - **else:** (conectará ao fluxo existente que verifica tipo 3)

---

### **PASSO 2: Processar PlayerInfoUpdate (ramo "then")**

**No ramo "then" do `K2Node_IfThenElse_PlayerInfoUpdate`:**

1. **Chamar `WSBinaryBPFL::ParsePlayerInfoUpdate`**:
   - **Nó:** `K2Node_CallFunction` → Busque por `ParsePlayerInfoUpdate`
   - **Target:** `WSBinaryBPFL` (classe estática, não precisa de self)
   - **Data:** Conecte ao `OutputPin` do `K2Node_Knot_33` (mesmo array de bytes)
   - **OutPlayerID:** Crie uma **variável local** `OutPlayerID_PlayerInfoUpdate` (Integer)
   - **OutCharacterName:** Crie uma **variável local** `OutCharacterName_PlayerInfoUpdate` (String)
   - **OutCharacterTitle:** Crie uma **variável local** `OutCharacterTitle_PlayerInfoUpdate` (String)
   - **ReturnValue:** Será um `bool` (true se parse foi bem-sucedido)

2. **Criar um `K2Node_IfThenElse` para verificar sucesso do parse**:
   - **Nome do nó:** `K2Node_IfThenElse_ParseSuccess`
   - **Condition:** Conecte ao `ReturnValue` do `ParsePlayerInfoUpdate`
   - **then:** (se parse foi bem-sucedido)
   - **else:** (opcional - pode conectar um Print String de erro)

3. **No ramo "then" do `K2Node_IfThenElse_ParseSuccess`:**

   a. **Obter Game Instance**:
      - **Nó:** `K2Node_CallFunction` → `Get Game Instance`
      - **World Context Object:** Conecte ao `self` do Blueprint
      - **Return Value:** Será um `Game Instance`

   b. **Cast para Umbra Game Instance**:
      - **Nó:** `K2Node_CallFunction` → `Cast to Umbra Game Instance`
      - **Object:** Conecte ao `Return Value` do `Get Game Instance`
      - **As Umbra Game Instance:** Será o objeto castado

   c. **Chamar `UpdateRemotePlayerNameplate`**:
      - **Nó:** `K2Node_CallFunction` → `Update Remote Player Nameplate`
      - **Target:** Conecte ao `As Umbra Game Instance` do cast
      - **PlayerID:** Conecte à variável local `OutPlayerID_PlayerInfoUpdate`
      - **CharacterName:** Conecte à variável local `OutCharacterName_PlayerInfoUpdate`
      - **CharacterTitle:** Conecte à variável local `OutCharacterTitle_PlayerInfoUpdate`

   d. **(Opcional) Print String para debug**:
      - **Nó:** `K2Node_CallFunction` → `Print String`
      - **InString:** `"[OnWSBinaryMessage] ✅ PlayerInfoUpdate processado: PlayerID={OutPlayerID_PlayerInfoUpdate}, Name={OutCharacterName_PlayerInfoUpdate}, Title={OutCharacterTitle_PlayerInfoUpdate}"`
      - Conecte após o `UpdateRemotePlayerNameplate`

---

### **PASSO 3: Conectar ao Fluxo Existente**

**Importante:** O fluxo existente que verifica `Data[0] == 3` (PlayerDisconnected) deve continuar funcionando.

**O que fazer:**

1. **Conecte o "else" do `K2Node_IfThenElse_PlayerInfoUpdate`** ao **início do bloco existente** que verifica `Data[0] == 3`:
   - O "else" deve conectar ao `K2Node_GetArrayItem_0` existente (ou ao nó que vem antes dele)

2. **O fluxo final deve ser:**
   ```
   [OnWSBinaryMessage] → Data
   ↓
   [Verificar se Data[0] == 4] (PlayerInfoUpdate)
   ├─ True → [ParsePlayerInfoUpdate] → [UpdateRemotePlayerNameplate] → FIM
   └─ False → [Verificar se Data[0] == 3] (PlayerDisconnected)
              ├─ True → [ParsePlayerDisconnected] → [RemoveRemoteActor] → FIM
              └─ False → [Verificar se Data.Num() == 5]
                         ├─ True → [Print String: "Mensagem de 5 bytes com tipo desconhecido"] → FIM
                         └─ False → [ProcessBinaryBuffer] → [ProcessNextFrame] → FIM
   ```

---

## 📋 RESUMO VISUAL DA ESTRUTURA

```
[OnWSBinaryMessage]
    Data (TArray<uint8>)
    ↓
[K2Node_Knot_33] (distribui Data)
    ↓
[NOVO: K2Node_GetArrayItem_PlayerInfoUpdate]
    Array: Data (do Knot_33)
    Index: 0
    ↓
[NOVO: K2Node_PromotableOperator_PlayerInfoUpdate]
    A: Output do GetArrayItem
    B: 4
    ↓
[NOVO: K2Node_IfThenElse_PlayerInfoUpdate]
    Condition: ReturnValue do Equal
    ├─ then → [ParsePlayerInfoUpdate]
    │            Data: Data (do Knot_33)
    │            OutPlayerID: (variável local)
    │            OutCharacterName: (variável local)
    │            OutCharacterTitle: (variável local)
    │            ↓
    │         [IfThenElse_ParseSuccess]
    │            Condition: ReturnValue do ParsePlayerInfoUpdate
    │            ├─ then → [Get Game Instance]
    │            │            ↓
    │            │         [Cast to Umbra Game Instance]
    │            │            ↓
    │            │         [Update Remote Player Nameplate]
    │            │            PlayerID: OutPlayerID_PlayerInfoUpdate
    │            │            CharacterName: OutCharacterName_PlayerInfoUpdate
    │            │            CharacterTitle: OutCharacterTitle_PlayerInfoUpdate
    │            └─ else → (opcional: Print String de erro)
    │
    └─ else → [EXISTENTE: K2Node_GetArrayItem_0] (verifica Data[0] == 3)
               ↓
               [EXISTENTE: K2Node_PromotableOperator_2] (EqualEqual_ByteByte)
               ↓
               [EXISTENTE: K2Node_IfThenElse_12] (PlayerDisconnected)
               ↓
               ... (resto do fluxo existente)
```

---

## ⚠️ IMPORTANTE

1. **Ordem das verificações:** PlayerInfoUpdate (tipo 4) deve ser verificado **ANTES** de PlayerDisconnected (tipo 3), pois ambos podem ter tamanhos diferentes e a verificação do tipo é mais confiável.

2. **Variáveis locais:** Crie as variáveis locais (`OutPlayerID_PlayerInfoUpdate`, `OutCharacterName_PlayerInfoUpdate`, `OutCharacterTitle_PlayerInfoUpdate`) **dentro do evento `OnWSBinaryMessage`**, não como variáveis do Blueprint.

3. **Não quebrar o fluxo existente:** Certifique-se de que o "else" do `K2Node_IfThenElse_PlayerInfoUpdate` conecta corretamente ao fluxo existente que verifica tipo 3.

4. **Tamanho mínimo:** A mensagem `PlayerInfoUpdate` tem tamanho mínimo de 7 bytes, mas pode ser maior dependendo do tamanho do nome e título. Por isso, verificamos o tipo (primeiro byte) em vez do tamanho.

---

## ✅ CHECKLIST

- [ ] Criado `K2Node_GetArrayItem_PlayerInfoUpdate` para pegar `Data[0]`
- [ ] Criado `K2Node_PromotableOperator_PlayerInfoUpdate` para comparar com `4`
- [ ] Criado `K2Node_IfThenElse_PlayerInfoUpdate` com ramos then/else
- [ ] No ramo "then": Chamado `ParsePlayerInfoUpdate` com variáveis locais
- [ ] Criado `K2Node_IfThenElse_ParseSuccess` para verificar sucesso
- [ ] No ramo "then" do parse: Obtido Game Instance
- [ ] Feito Cast para Umbra Game Instance
- [ ] Chamado `UpdateRemotePlayerNameplate` com os valores parseados
- [ ] Conectado o "else" do `K2Node_IfThenElse_PlayerInfoUpdate` ao fluxo existente
- [ ] Testado: Mensagem tipo 4 deve processar PlayerInfoUpdate
- [ ] Testado: Mensagem tipo 3 deve continuar processando PlayerDisconnected
- [ ] Testado: Outras mensagens devem continuar no fluxo normal

---

**Após essas implementações, o sistema deve processar corretamente as mensagens `PlayerInfoUpdate` e atualizar os nameplates dos remote actors!**
