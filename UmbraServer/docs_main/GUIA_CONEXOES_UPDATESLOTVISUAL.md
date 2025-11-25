# 🔌 GUIA COMPLETO DE CONEXÕES: UpdateSlotVisual

## 📋 **LISTA DE NODOS IMPORTADOS**

Você deve ter os seguintes nós na função `UpdateSlotVisual`:

1. **K2Node_FunctionEntry_0** - Entry Point (início da função)
2. **K2Node_VariableGet_2** - Get SlotData
3. **K2Node_BreakStruct_1** - Break UmbraInventorySlot
4. **K2Node_CallFunction_DEBUG1** - Print String (Amarelo)
5. **K2Node_IfThenElse_1** - Branch (if InventoryID > 0)
6. **K2Node_VariableSet_IsEmpty (FALSE)** - Set bIsEmpty = false
7. **K2Node_VariableSet_IsEmpty (TRUE)** - Set bIsEmpty = true
8. **K2Node_BreakStruct_2** - Break UmbraItemTemplate
9. **K2Node_CallFunction_DEBUG2** - Print String (Verde)
10. **K2Node_CallFunction_IsValid** - Is Valid (ItemIcon)
11. **K2Node_CallFunction_SelectString** - Select String
12. **K2Node_CallFunction_DEBUG3** - Print String (Ciano - IsValid)
13. **K2Node_VariableGet_ImageIcon** - Get Image_ItemIcon
14. **K2Node_CallFunction_SetBrush** - Set Brush from Texture
15. **K2Node_CallFunction_DEBUG4** - Print String (Ciano - SetBrush)
16. **K2Node_CallFunction_SetText** - Set Text (Quantity)
17. **K2Node_CallFunction_SetPercent** - Set Percent (Durability)

---

## 🔗 **CONEXÕES DE EXECUÇÃO (BRANCAS/CINZAS)**

### **FLUXO PRINCIPAL:**

```
1. Entry Point (then)
   └──> Print String DEBUG1 (execute)

2. Print String DEBUG1 (then)
   └──> Branch (execute)

3. Branch (then) - Se TRUE (InventoryID > 0):
   └──> Set bIsEmpty = false (execute)

4. Branch (else) - Se FALSE (InventoryID <= 0):
   └──> Set bIsEmpty = true (execute)

5. Set bIsEmpty = false (then)
   └──> Print String DEBUG2 (execute)

6. Print String DEBUG2 (then)
   └──> Is Valid (execute)

7. Is Valid (then)
   └──> Print String DEBUG3 (execute)

8. Print String DEBUG3 (then)
   └──> Set Brush from Texture (execute)

9. Set Brush from Texture (then)
   └──> Print String DEBUG4 (execute)

10. Print String DEBUG4 (then)
    └──> Set Text (execute)

11. Set Text (then)
    └──> Set Percent (execute)
```

---

## 🔗 **CONEXÕES DE DADOS**

### **GRUPO 1: Get SlotData → Break UmbraInventorySlot**

```
Get SlotData (saída: SlotData)
└──> Break UmbraInventorySlot (entrada: UmbraInventorySlot)
```

---

### **GRUPO 2: Break UmbraInventorySlot → Branch**

```
Break UmbraInventorySlot (saída: InventoryID)
├──> Print String DEBUG1 (entrada: InString) - para mostrar o ID
└──> Branch (entrada: Condition) - para verificar se > 0
```

**IMPORTANTE:** O `InventoryID` deve ser **convertido para bool** automaticamente. Se não funcionar, adicione um nó `> (Greater)` com valor `0`:

```
Break UmbraInventorySlot (saída: InventoryID)
└──> > (Greater) [compare com 0]
     └──> Branch (entrada: Condition)
```

---

### **GRUPO 3: Break UmbraInventorySlot → Break UmbraItemTemplate**

```
Break UmbraInventorySlot (saída: ItemTemplate)
└──> Break UmbraItemTemplate (entrada: UmbraItemTemplate)
```

---

### **GRUPO 4: Break UmbraInventorySlot → Set Text (Quantity)**

```
Break UmbraInventorySlot (saída: Quantity)
└──> Set Text (entrada: InText)
```

**IMPORTANTE:** `Quantity` é `int`, mas `InText` espera `text`. O Unreal deve fazer a conversão automaticamente. Se não funcionar, adicione um nó `ToText (int)`:

```
Break UmbraInventorySlot (saída: Quantity)
└──> ToText (int) (entrada: Value)
     └──> Set Text (entrada: InText)
```

---

### **GRUPO 5: Break UmbraInventorySlot → Set Percent (Durability)**

```
Break UmbraInventorySlot (saída: Durability)
└──> Set Percent (entrada: InPercent)
```

**IMPORTANTE:** `Durability` está como `float` (0-100), mas `InPercent` espera um valor normalizado (0.0-1.0). Você precisa **dividir por 100**:

```
Break UmbraInventorySlot (saída: Durability)
└──> / (Divide) [dividir por 100.0]
     └──> Set Percent (entrada: InPercent)
```

---

### **GRUPO 6: Break UmbraItemTemplate → Is Valid**

```
Break UmbraItemTemplate (saída: ItemIcon)
├──> Is Valid (entrada: Object)
└──> Set Brush from Texture (entrada: Texture)
```

---

### **GRUPO 7: Is Valid → Select String**

```
Is Valid (saída: ReturnValue - bool)
└──> Select String (entrada: Condition)
```

---

### **GRUPO 8: Select String → Print String DEBUG3**

```
Select String (saída: ReturnValue - string)
└──> Print String DEBUG3 (entrada: InString)
```

---

### **GRUPO 9: Get Image_ItemIcon → Set Brush / Set Text / Set Percent**

```
Get Image_ItemIcon (saída: Image_ItemIcon)
├──> Set Brush from Texture (entrada: self/Target)
├──> Set Text (entrada: self/Target)
└──> Set Percent (entrada: self/Target)
```

**⚠️ ATENÇÃO:** Se você não tiver `Text_Quantity` ou `ProgressBar_Durability` no seu widget, essas conexões vão falhar! Nesse caso:
- **REMOVA** os nós `Set Text` e `Set Percent`
- Reconecte:
  ```
  Print String DEBUG4 (then)
  └──> (fim da função - não conecta em nada)
  ```

---

## 📊 **DIAGRAMA VISUAL COMPLETO**

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                         UPDATESLOTVISUAL - FLUXO COMPLETO                     │
└──────────────────────────────────────────────────────────────────────────────┘

[Entry Point]
    │ then
    ▼
[Get SlotData] ──┬── SlotData ──> [Break UmbraInventorySlot]
                 │                         │
                 │                         ├─ InventoryID ──┬──> [Print DEBUG1]
                 │                         │                │         │ then
                 │                         │                └──> [Branch] <──┘
                 │                         │                      │     │
                 │                         │             then ────┘     └──── else
                 │                         │              │                   │
                 │                         │              ▼                   ▼
                 │                         │      [Set bIsEmpty=false]  [Set bIsEmpty=true]
                 │                         │              │
                 │                         │              │ then
                 │                         │              ▼
                 │                         │      [Print DEBUG2 "Item NÃO vazio"]
                 │                         │              │ then
                 │                         │              ▼
                 │                         ├─ ItemTemplate ──> [Break UmbraItemTemplate]
                 │                         │                            │
                 │                         │                            ├─ ItemIcon ─┬──> [Is Valid]
                 │                         │                            │            │       │
                 │                         │                            │            │       ├─ bool ──> [Select String]
                 │                         │                            │            │       │ then           │
                 │                         │                            │            │       ▼                │
                 │                         │                            │            │   [Print DEBUG3] <─────┘
                 │                         │                            │            │       │ then
                 │                         │                            │            │       ▼
                 │                         │                            │            └──> [Set Brush from Texture] <── [Get Image_ItemIcon]
                 │                         │                            │                    │ then
                 │                         │                            │                    ▼
                 │                         │                            │                [Print DEBUG4]
                 │                         │                            │                    │ then
                 │                         ├─ Quantity ────────────────────────────────────────> [Set Text] <── [Get Image_ItemIcon]
                 │                         │                            │                    │ then
                 │                         │                            │                    ▼
                 │                         └─ Durability ─────────────────────────────────────> [Set Percent] <── [Get Image_ItemIcon]
                 │                                                      │
                 │                                                      │
                 └──────────────────────────────────────────────────────┘
```

---

## 🛠️ **PASSO A PASSO PARA CONECTAR MANUALMENTE**

### **ETAPA 1: CONEXÕES DE EXECUÇÃO (FLUXO BRANCO)**

1. **Entry Point → Print DEBUG1:**
   - Arraste do pin `then` de **Entry Point**
   - Conecte ao pin `execute` de **Print String DEBUG1**

2. **Print DEBUG1 → Branch:**
   - Arraste do pin `then` de **Print String DEBUG1**
   - Conecte ao pin `execute` de **Branch**

3. **Branch → Set bIsEmpty (false):**
   - Arraste do pin `then` (superior) de **Branch**
   - Conecte ao pin `execute` de **Set bIsEmpty = false**

4. **Branch → Set bIsEmpty (true):**
   - Arraste do pin `else` (inferior) de **Branch**
   - Conecte ao pin `execute` de **Set bIsEmpty = true**

5. **Set bIsEmpty (false) → Print DEBUG2:**
   - Arraste do pin `then` de **Set bIsEmpty = false**
   - Conecte ao pin `execute` de **Print String DEBUG2**

6. **Print DEBUG2 → Is Valid:**
   - Arraste do pin `then` de **Print String DEBUG2**
   - Conecte ao pin `execute` de **Is Valid**

7. **Is Valid → Print DEBUG3:**
   - Arraste do pin `then` de **Is Valid**
   - Conecte ao pin `execute` de **Print String DEBUG3**

8. **Print DEBUG3 → Set Brush:**
   - Arraste do pin `then` de **Print String DEBUG3**
   - Conecte ao pin `execute` de **Set Brush from Texture**

9. **Set Brush → Print DEBUG4:**
   - Arraste do pin `then` de **Set Brush from Texture**
   - Conecte ao pin `execute` de **Print String DEBUG4**

10. **Print DEBUG4 → Set Text:**
    - Arraste do pin `then` de **Print String DEBUG4**
    - Conecte ao pin `execute` de **Set Text** (se existir)

11. **Set Text → Set Percent:**
    - Arraste do pin `then` de **Set Text**
    - Conecte ao pin `execute` de **Set Percent** (se existir)

---

### **ETAPA 2: CONEXÕES DE DADOS (COLORIDAS)**

#### **2.1 - Conectar SlotData:**

1. Arraste do pin de saída `SlotData` de **Get SlotData**
2. Conecte ao pin de entrada `UmbraInventorySlot` de **Break UmbraInventorySlot**

---

#### **2.2 - Conectar InventoryID:**

1. Arraste do pin de saída `InventoryID` de **Break UmbraInventorySlot**
2. **SOLTE NO ESPAÇO VAZIO** e digite `>` (Greater)
3. Crie um nó `> (integer)`
4. No segundo pin do `>`, digite `0`
5. Arraste do resultado do `>` para o pin `Condition` de **Branch**

**E também:**

6. Arraste novamente do pin de saída `InventoryID` de **Break UmbraInventorySlot**
7. Conecte ao pin `InString` de **Print String DEBUG1**
   - O Unreal vai converter automaticamente `int` para `string`

---

#### **2.3 - Conectar ItemTemplate:**

1. **EXPANDA** o **Break UmbraInventorySlot** clicando na **seta** ao lado do nome
2. Localize o pin de saída `ItemTemplate` (deve estar em "Advanced")
3. Arraste do pin `ItemTemplate` de **Break UmbraInventorySlot**
4. Conecte ao pin de entrada `UmbraItemTemplate` de **Break UmbraItemTemplate**

---

#### **2.4 - Conectar ItemIcon:**

1. **EXPANDA** o **Break UmbraItemTemplate** clicando na **seta**
2. Localize o pin de saída `ItemIcon` (deve estar em "Advanced")
3. Arraste do pin `ItemIcon` de **Break UmbraItemTemplate**
4. Conecte em **DOIS LUGARES**:
   - Pin `Object` de **Is Valid**
   - Pin `Texture` de **Set Brush from Texture**

---

#### **2.5 - Conectar Is Valid → Select String:**

1. Arraste do pin de saída `ReturnValue` (bool) de **Is Valid**
2. Conecte ao pin `Condition` de **Select String**

---

#### **2.6 - Conectar Select String → Print DEBUG3:**

1. Arraste do pin de saída `ReturnValue` (string) de **Select String**
2. Conecte ao pin `InString` de **Print String DEBUG3**

---

#### **2.7 - Conectar Get Image_ItemIcon → Set Brush:**

1. Arraste do pin de saída `Image_ItemIcon` de **Get Image_ItemIcon**
2. Conecte ao pin `self` (ou `Target`) de **Set Brush from Texture**

---

#### **2.8 - Conectar Quantity → Set Text (OPCIONAL):**

**⚠️ PULE ESTA ETAPA SE NÃO TIVER O COMPONENTE `Text_Quantity`**

1. **EXPANDA** o **Break UmbraInventorySlot**
2. Arraste do pin `Quantity` de **Break UmbraInventorySlot**
3. **SOLTE NO ESPAÇO VAZIO** e digite `ToText`
4. Crie um nó `ToText (int)`
5. Conecte:
   ```
   Quantity → ToText (int) → Set Text (InText)
   ```
6. Arraste do pin de saída de **Get Image_ItemIcon**
7. Conecte ao pin `self` de **Set Text**
   - ⚠️ **ISSO VAI FALHAR** porque você precisa ter um componente `Text_Quantity`!
   - Se não tiver, **DELETE** este nó `Set Text`

---

#### **2.9 - Conectar Durability → Set Percent (OPCIONAL):**

**⚠️ PULE ESTA ETAPA SE NÃO TIVER O COMPONENTE `ProgressBar_Durability`**

1. **EXPANDA** o **Break UmbraInventorySlot**
2. Arraste do pin `Durability` de **Break UmbraInventorySlot**
3. **SOLTE NO ESPAÇO VAZIO** e digite `/` (Divide)
4. Crie um nó `/ (float)`
5. No segundo pin do `/`, digite `100.0`
6. Conecte:
   ```
   Durability → / (100.0) → Set Percent (InPercent)
   ```
7. Arraste do pin de saída de **Get Image_ItemIcon**
8. Conecte ao pin `self` de **Set Percent**
   - ⚠️ **ISSO VAI FALHAR** porque você precisa ter um componente `ProgressBar_Durability`!
   - Se não tiver, **DELETE** este nó `Set Percent`

---

## ✅ **VERIFICAÇÃO FINAL**

### **Todos os nós devem estar conectados assim:**

| Nó | Entrada de Execução | Saída de Execução |
|----|-------------------|------------------|
| Entry Point | (início) | → Print DEBUG1 |
| Print DEBUG1 | ← Entry Point | → Branch |
| Branch | ← Print DEBUG1 | → Set bIsEmpty (false) [then]<br>→ Set bIsEmpty (true) [else] |
| Set bIsEmpty (false) | ← Branch [then] | → Print DEBUG2 |
| Set bIsEmpty (true) | ← Branch [else] | (sem saída) |
| Print DEBUG2 | ← Set bIsEmpty (false) | → Is Valid |
| Is Valid | ← Print DEBUG2 | → Print DEBUG3 |
| Print DEBUG3 | ← Is Valid | → Set Brush |
| Set Brush | ← Print DEBUG3 | → Print DEBUG4 |
| Print DEBUG4 | ← Set Brush | → Set Text (opcional) |

---

### **Conexões de Dados Essenciais:**

✅ `Get SlotData` → `Break UmbraInventorySlot`
✅ `Break UmbraInventorySlot (InventoryID)` → `> (0)` → `Branch (Condition)`
✅ `Break UmbraInventorySlot (InventoryID)` → `Print DEBUG1 (InString)`
✅ `Break UmbraInventorySlot (ItemTemplate)` → `Break UmbraItemTemplate`
✅ `Break UmbraItemTemplate (ItemIcon)` → `Is Valid (Object)` + `Set Brush (Texture)`
✅ `Is Valid (ReturnValue)` → `Select String (Condition)`
✅ `Select String (ReturnValue)` → `Print DEBUG3 (InString)`
✅ `Get Image_ItemIcon` → `Set Brush (self)`

---

## 🚨 **PROBLEMAS COMUNS**

### **Problema 1: "Não consigo conectar InventoryID ao Branch"**

**Solução:** InventoryID é `int`, Branch espera `bool`. Adicione um nó `> (Greater)`:

```
Break UmbraInventorySlot (InventoryID)
└──> > (Greater) [valor 0]
     └──> Branch (Condition)
```

---

### **Problema 2: "Não consigo conectar Quantity ao Set Text"**

**Solução:** Quantity é `int`, Set Text espera `text`. Adicione um nó `ToText (int)`:

```
Break UmbraInventorySlot (Quantity)
└──> ToText (int)
     └──> Set Text (InText)
```

---

### **Problema 3: "Set Text e Set Percent dão erro de 'Target must have a connection'"**

**Solução:** Você precisa criar os componentes correspondentes no Designer:

1. Vá para a aba **Designer** de `WBP_InventorySlot`
2. Adicione um **Text Block** chamado `Text_Quantity`
3. Adicione um **Progress Bar** chamado `ProgressBar_Durability`
4. Volte para a aba **Graph**
5. Reconecte os nós

**OU** se você não quiser esses componentes:

1. **DELETE** os nós `Set Text` e `Set Percent`
2. Reconecte:
   ```
   Print DEBUG4 (then)
   └──> (fim - não conecta em nada)
   ```

---

### **Problema 4: "ItemIcon não aparece no Break UmbraItemTemplate"**

**Solução:** Clique na **seta para baixo** (▼) no canto superior direito do nó `Break UmbraItemTemplate` para **mostrar pins avançados**.

---

### **Problema 5: "Não tenho o nó Get Image_ItemIcon"**

**Solução:** Você precisa criar uma variável:

1. Na aba **Variables**, clique em **+ (Add Variable)**
2. Nome: `Image_ItemIcon`
3. Tipo: **Image** (Object Reference)
4. Marque como **Instance Editable** e **Expose on Spawn**
5. Arraste para o Graph para criar o nó `Get Image_ItemIcon`

**OU** mais simples:

1. Vá para a aba **Designer**
2. Selecione o componente **Image_ItemIcon** na hierarquia
3. Marque **"Is Variable"** no painel Details
4. Volte para a aba **Graph**
5. Arraste o componente `Image_ItemIcon` da lista de variáveis para o Graph

---

## 🎯 **RESULTADO ESPERADO**

Após conectar tudo corretamente e abrir o inventário, você deve ver:

```
🟡 [UpdateSlotVisual] InventoryID: 1
🟢 [UpdateSlotVisual] Item NAO vazio - bIsEmpty = false
🔵 [UpdateSlotVisual] ItemIcon válido: TRUE - ItemIcon é válido!
🔵 [UpdateSlotVisual] ✅ Set Brush from Texture EXECUTADO!

(repetido para cada item do inventário)
```

E o **ícone deve aparecer** no slot!

---

## 📸 **DICA VISUAL**

Se você tirar um **print de tela** do seu Event Graph após importar os nós, eu posso ajudar a identificar quais conexões específicas estão faltando!

---

**Boa sorte! 🚀**


