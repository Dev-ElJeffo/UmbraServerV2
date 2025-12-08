# 🎯 **GUIA: Implementar SelectClass e Broadcast OnClassSelected**

## 📋 **OBJETIVO**

Implementar a função `SelectClass` no Blueprint `BP_Class_Placeholder` que:
1. Define `bIsSelected = true`
2. Faz broadcast do Event Dispatcher `OnClassSelected` com o `ClassID`

E também mostrar como capturar esse broadcast em outros Blueprints.

---

## ✅ **PARTE 1: Implementar SelectClass no BP_Class_Placeholder**

### **1.1. Abrir a Função SelectClass**

1. No Blueprint `BP_Class_Placeholder`, vá para o **Event Graph**
2. No painel **My Blueprint** → **Functions**, encontre `SelectClass`
3. Clique duas vezes em `SelectClass` para abrir a função

### **1.2. Verificar Inputs da Função**

A função `SelectClass` deve ter:
- **Input:** `ClassID` (Integer)

Se não tiver, adicione:
1. Na função `SelectClass`, clique no botão **+ Input** (ou arraste do pino de entrada)
2. Nomeie como: `ClassID`
3. Tipo: **Integer**

### **1.3. Implementar a Lógica Completa**

**No Event Graph da função `SelectClass`:**

```
[SelectClass] (Function Entry - pino roxo)
    • ClassID (input - laranja)
    ↓ (exec - branco)
[SET] bIsSelected
    • Target: (Self)
    • bIsSelected: true (checkbox marcado)
    ↓ (exec)
[Call Function: Broadcast] OnClassSelected
    • Target: (Self)
    • ClassID: ClassID (conecte o pino de input)
```

**Passo a Passo Detalhado:**

#### **Passo 1: Adicionar SET bIsSelected**

1. No Event Graph da função `SelectClass`
2. Arraste do pino **exec** (saída branca) do nó `SelectClass`
3. Digite: `SET` ou `Set bIsSelected`
4. Selecione: **SET bIsSelected** (deve aparecer como "Set bIsSelected")
5. Conecte:
   - **Target:** Arraste `Self` (ou deixe vazio se estiver no mesmo Blueprint)
   - **bIsSelected:** Marque o checkbox (true)

#### **Passo 2: Adicionar Broadcast OnClassSelected**

1. Arraste do pino **exec** (saída) do nó `SET bIsSelected`
2. Digite: `Call` ou `Broadcast`
3. Selecione: **Call OnClassSelected** ou **Broadcast OnClassSelected**
   - Se não aparecer, tente: **Call Function** → Procure por `OnClassSelected`
4. Conecte:
   - **Target:** Arraste `Self` (ou deixe vazio)
   - **ClassID:** Conecte o pino `ClassID` (input da função) ao pino `ClassID` do Broadcast

**⚠️ IMPORTANTE:** O Event Dispatcher `OnClassSelected` deve ter um parâmetro `ClassID` (Integer).

**Se o Event Dispatcher não tiver o parâmetro:**
1. No painel **My Blueprint** → **Event Dispatchers**
2. Selecione `OnClassSelected`
3. No **Details**, clique em **+ Input**
4. Nomeie como: `ClassID`
5. Tipo: **Integer**

---

## 📡 **PARTE 2: Capturar o Broadcast (Bind ao Event Dispatcher)**

### **2.1. Onde Capturar o Broadcast**

O broadcast pode ser capturado em:
- **BP_CharacterCreationManager** (recomendado)
- **WBP_CreateCharacter** (widget)
- **Game Mode** ou **Player Controller**

### **2.2. Implementar no BP_CharacterCreationManager**

#### **Passo 1: No Event BeginPlay**

```
[Event BeginPlay]
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[SET] AllPlaceholders = (resultado)
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Bind Event to OnClassSelected]
    • Target: Array Element
    • Event: OnClassSelected_Handler (criar este evento)
```

#### **Passo 2: Criar Event Handler**

1. No **Event Graph**, clique com botão direito
2. Selecione: **Add Custom Event**
3. Nomeie como: `OnClassSelected_Handler`
4. Adicione um **Input:**
   - Nome: `ClassID`
   - Tipo: **Integer**

#### **Passo 3: Implementar Lógica no Handler**

```
[OnClassSelected_Handler] (Custom Event)
    • ClassID (input - Integer)
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Select Class]
    • Target: (cast result)
    • Class ID: ClassID (input do evento)
    ↓
[SET] SelectedClassID = ClassID
    ↓
[Find Placeholder by ClassID]
    • ClassID: ClassID
    ↓
[SET] SelectedPlaceholder = (resultado)
    ↓
[Call Function: DespawnOtherPlaceholders]
    • Target: (Self)
    • Keep Placeholder: SelectedPlaceholder
    ↓
[Call Function: MoveCameraToSelection]
    • Target: (Self)
    • Target Placeholder: SelectedPlaceholder
    ↓
[Call Function: ShowClassInfoWidget]
    • Target: (Self)
    • ClassID: ClassID
```

**📌 COMO FAZER O BIND:**

1. No **Event Graph** do `BP_CharacterCreationManager`
2. No `For Each Loop` que itera sobre `AllPlaceholders`
3. Arraste do pino `Array Element` (tipo `BP_Class_Placeholder`)
4. Digite: `Bind` ou `Assign`
5. Selecione: **Bind Event to OnClassSelected** ou **Assign OnClassSelected**
6. Conecte:
   - **Target:** `Array Element`
   - **Event:** Clique no pino `Event` → **Create Event** → Nome: `OnClassSelected_Handler`
   - O evento criado automaticamente terá o parâmetro `ClassID` (Integer)

---

## 🎨 **PARTE 3: Implementar no WBP_CreateCharacter (Alternativa)**

Se você quiser capturar diretamente no Widget:

### **3.1. No Event Construct**

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[SET] MyGameInstance = (cast result)
    ↓
[Get All Actors of Class]
    • Actor Class: BP_Class_Placeholder
    ↓
[For Each Loop]
    • Array: (resultado)
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Bind Event to OnClassSelected]
    • Target: Array Element
    • Event: OnClassSelected_Handler (criar este evento)
```

### **3.2. Criar Event Handler no Widget**

```
[OnClassSelected_Handler] (Custom Event)
    • ClassID (input - Integer)
    ↓
[Get] MyGameInstance
    ↓
[Select Class]
    • Target: MyGameInstance
    • Class ID: ClassID
    ↓
[Get Class Data By ID]
    • Target: MyGameInstance
    • Class ID: ClassID
    • Out Class Data: (variável local)
    ↓
[Call Function: ShowClassInfo]
    • Target: (Self)
    • ClassID: ClassID
    • ClassData: Out Class Data
```

---

## 🔍 **PARTE 4: Verificação e Debug**

### **4.1. Verificar se o Broadcast está Funcionando**

Adicione **Print String** para debug:

**No BP_Class_Placeholder → SelectClass:**
```
[SelectClass]
    ↓
[SET] bIsSelected = true
    ↓
[Format Text]
    • Format: "🎯 Classe selecionada: ID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado do Format Text)
    ↓
[Broadcast] OnClassSelected
    • ClassID: ClassID
```

**No Handler (onde captura o broadcast):**
```
[OnClassSelected_Handler]
    • ClassID (input)
    ↓
[Format Text]
    • Format: "📡 Broadcast recebido: ClassID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado do Format Text)
    ↓
[Resto da lógica...]
```

### **4.2. Checklist de Verificação**

- [ ] Função `SelectClass` tem input `ClassID` (Integer)
- [ ] `SET bIsSelected = true` está conectado
- [ ] `Broadcast OnClassSelected` está conectado
- [ ] O pino `ClassID` do Broadcast está conectado ao input `ClassID`
- [ ] Event Dispatcher `OnClassSelected` tem parâmetro `ClassID` (Integer)
- [ ] O bind está sendo feito no `Event BeginPlay` (ou `Event Construct`)
- [ ] O Event Handler foi criado com o parâmetro `ClassID`
- [ ] Os prints aparecem no Output Log quando clica no placeholder

---

## ⚠️ **PROBLEMAS COMUNS E SOLUÇÕES**

### **Problema 1: Broadcast não aparece no menu**

**Solução:**
- Verifique se o Event Dispatcher `OnClassSelected` existe no painel **My Blueprint**
- Verifique se o Event Dispatcher tem o parâmetro `ClassID` configurado
- Tente digitar: `Call` ou `Call Function` e procure por `OnClassSelected`

### **Problema 2: Bind não funciona**

**Solução:**
- Certifique-se de que está fazendo o bind no `Event BeginPlay` (não no `Event Construct` do Widget)
- Verifique se o `Array Element` é do tipo correto (`BP_Class_Placeholder`)
- Tente usar `Assign OnClassSelected` em vez de `Bind Event`

### **Problema 3: Handler não recebe o broadcast**

**Solução:**
- Verifique se o bind está sendo feito **antes** de clicar no placeholder
- Adicione prints para verificar se o bind está sendo executado
- Verifique se o Event Handler tem o parâmetro `ClassID` do tipo correto (Integer)

### **Problema 4: ClassID não está sendo passado**

**Solução:**
- Verifique se o pino `ClassID` do Broadcast está conectado ao input `ClassID` da função
- Verifique se o Event Dispatcher tem o parâmetro `ClassID` configurado
- Adicione um print para verificar o valor do `ClassID` antes do broadcast

---

## 📝 **RESUMO VISUAL DA ESTRUTURA**

```
BP_Class_Placeholder:
├── Event Graph
│   ├── Event BeginPlay
│   │   └── Set Input Mode Game Only
│   │
│   ├── On Clicked (Collision_Box)
│   │   └── Call Function: SelectClass
│   │       └── ClassID: (Get ClassID)
│   │
│   └── Function: SelectClass
│       ├── Input: ClassID (Integer)
│       ├── SET bIsSelected = true
│       └── Broadcast OnClassSelected
│           └── ClassID: (input)
│
└── Event Dispatchers
    └── OnClassSelected
        └── Parameter: ClassID (Integer)

BP_CharacterCreationManager:
├── Event Graph
│   ├── Event BeginPlay
│   │   ├── Get All Actors of Class (BP_Class_Placeholder)
│   │   └── For Each Loop
│   │       └── Bind Event to OnClassSelected
│   │           └── Event: OnClassSelected_Handler
│   │
│   └── Custom Event: OnClassSelected_Handler
│       ├── Input: ClassID (Integer)
│       ├── Get Game Instance
│       ├── Cast to Umbra Game Instance
│       ├── Select Class (ClassID)
│       ├── Despawn Other Placeholders
│       ├── Move Camera to Selection
│       └── Show Class Info Widget
```

---

**Fim do Guia**

