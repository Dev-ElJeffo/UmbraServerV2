# 🔧 **GUIA: Corrigir SelectClass ao Clicar no Personagem**

## ❌ **PROBLEMA**

Ao clicar no personagem, a câmera funciona, mas o personagem não é selecionado. O `SelectClass` está chamando apenas a função do Game Instance, mas não está fazendo o broadcast do Event Dispatcher.

---

## ✅ **SOLUÇÃO: Completar a Função SelectClass**

### **PASSO 1: Abrir a Função SelectClass**

1. No Blueprint `BP_Class_Placeholder`
2. No painel **My Blueprint** → **Functions**, encontre `SelectClass`
3. Clique duas vezes para abrir a função

### **PASSO 2: Verificar a Estrutura Atual**

A função `SelectClass` deve ter:
- **Input:** `ClassID` (Integer)
- **Lógica atual:** Provavelmente só chama a função do Game Instance

### **PASSO 3: Implementar a Lógica Completa**

**A função `SelectClass` deve fazer 3 coisas:**

```
[SelectClass] (Function Entry)
    • ClassID (input)
    ↓
[SET] bIsSelected = true
    • Target: (Self)
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Select Class]
    • Target: (cast result)
    • Class ID: ClassID
    ↓
[Broadcast] OnClassSelected
    • Target: (Self)
    • ClassID: ClassID (conecte o input)
```

---

## 🔍 **VERIFICAÇÃO: Event Dispatcher OnClassSelected**

### **PASSO 1: Verificar se Existe**

1. No painel **My Blueprint** → **Event Dispatchers**
2. Procure por `OnClassSelected`
3. Se **NÃO existir**, crie:
   - Clique em **+ Event Dispatcher**
   - Nomeie como: `OnClassSelected`

### **PASSO 2: Adicionar Parâmetro ClassID**

1. Selecione `OnClassSelected` no painel
2. No **Details**, clique em **+ Input**
3. Nomeie como: `ClassID`
4. Tipo: **Integer**

---

## 📡 **PARTE 2: Capturar o Broadcast no BP_CharacterCreationManager**

### **PASSO 1: Verificar se o Bind Existe**

No `BP_CharacterCreationManager`, no **Event BeginPlay**:

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

### **PASSO 2: Criar Event Handler OnClassSelected_Handler**

1. No **Event Graph**, clique com botão direito
2. Selecione: **Add Custom Event**
3. Nomeie como: `OnClassSelected_Handler`
4. Adicione **Input:**
   - Nome: `ClassID`
   - Tipo: **Integer**

### **PASSO 3: Implementar Lógica no Handler**

```
[OnClassSelected_Handler] (Custom Event)
    • ClassID (input)
    ↓
[Print String]
    • In String: "📡 Broadcast recebido: ClassID {ClassID}"
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[Select Class]
    • Target: (cast result)
    • Class ID: ClassID
    ↓
[Find Placeholder by ClassID]
    • ClassID: ClassID
    • Found Placeholder: (variável local)
    ↓
[SET] SelectedPlaceholder = Found Placeholder
    ↓
[Call Function: DespawnOtherPlaceholders]
    • Target: (Self)
    • Keep Placeholder: SelectedPlaceholder
    ↓
[Get] CameraActor
    ↓
[Call Function: Move Camera To Selection]
    • Target: (Self)
    • Target Placeholder: SelectedPlaceholder
    • Camera Actor: CameraActor
    • Duration: 1.0
    • Offset X: 200.0
    • Offset Z: 100.0
    ↓
[Call Function: ShowClassInfoWidget]
    • Target: (Self)
    • ClassID: ClassID
```

---

## ⚠️ **SOBRE O ButtonPressed**

**NÃO precisa verificar o `ButtonPressed`!**

O `OnClicked` do `PrimitiveComponent` já dispara quando você clica, independente do botão do mouse. O `ButtonPressed` é apenas informação adicional (qual botão foi pressionado), mas não é necessário para a lógica funcionar.

---

## ✅ **CHECKLIST DE VERIFICAÇÃO**

- [ ] Função `SelectClass` tem input `ClassID` (Integer)
- [ ] `SET bIsSelected = true` está conectado
- [ ] `Get Game Instance` → `Cast to Umbra Game Instance` → `Select Class` está conectado
- [ ] `Broadcast OnClassSelected` está conectado **DEPOIS** do `Select Class`
- [ ] O pino `ClassID` do Broadcast está conectado ao input `ClassID` da função
- [ ] Event Dispatcher `OnClassSelected` existe e tem parâmetro `ClassID` (Integer)
- [ ] No `BP_CharacterCreationManager`, o bind está sendo feito no `Event BeginPlay`
- [ ] O Event Handler `OnClassSelected_Handler` foi criado com parâmetro `ClassID`
- [ ] A lógica completa está implementada no Handler (despawn, mover câmera, mostrar widget)

---

## 🐛 **DEBUG: Adicionar Prints**

Para verificar se está funcionando, adicione prints:

**No SelectClass:**
```
[SelectClass]
    ↓
[Format Text]
    • Format: "🎯 SelectClass chamado: ID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[SET] bIsSelected = true
    ↓
[Broadcast] OnClassSelected
```

**No Handler:**
```
[OnClassSelected_Handler]
    ↓
[Format Text]
    • Format: "📡 Handler recebeu: ID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[Resto da lógica...]
```

---

## 🎯 **RESUMO**

**O que estava faltando:**
1. ❌ `SET bIsSelected = true` no `SelectClass`
2. ❌ `Broadcast OnClassSelected` no `SelectClass`
3. ❌ Bind do Event Dispatcher no `BP_CharacterCreationManager`
4. ❌ Event Handler `OnClassSelected_Handler` com toda a lógica

**Agora deve funcionar:**
1. ✅ Clicar no personagem → `OnClicked` → `SelectClass`
2. ✅ `SelectClass` → SET bIsSelected → Select Class (Game Instance) → Broadcast
3. ✅ Broadcast → Handler no Manager → Despawn → Mover Câmera → Mostrar Widget

---

**Fim do Guia**

