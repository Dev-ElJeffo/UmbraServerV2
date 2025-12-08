# 🐛 **GUIA: Debug - SelectClass Não Funciona**

## ❌ **PROBLEMA**

A câmera funciona, mas o personagem não está sendo selecionado ao clicar. Tudo parece estar configurado corretamente.

---

## ✅ **VERIFICAÇÃO PASSO A PASSO**

### **PASSO 1: Verificar se OnClicked Está Disparando**

No `BP_Class_Placeholder`, no Event Graph onde está o `OnClicked` do `Collision_Box`:

**Adicione um Print String ANTES do SelectClass:**

```
[OnClicked] (Collision_Box)
    ↓
[Print String]
    • In String: "🖱️ CLIQUE DETECTADO!"
    ↓
[Get] ClassID
    ↓
[Format Text]
    • Format: "ClassID: {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[Call Function: SelectClass]
    • ClassID: ClassID
```

**Teste:** Ao clicar, você deve ver os prints no Output Log.

**Se NÃO aparecer:**
- O `Collision_Box` pode não estar configurado corretamente
- Verifique se o `Collision_Box` tem **"Generate Overlap Events"** ou **"Generate Hit Events"** marcado
- Verifique se o `Collision_Box` está visível e não está sendo bloqueado por outro componente

---

### **PASSO 2: Verificar se SelectClass Está Sendo Chamado**

Na função `SelectClass` do `BP_Class_Placeholder`:

**Adicione prints:**

```
[SelectClass]
    • ClassID (input)
    ↓
[Format Text]
    • Format: "🎯 SelectClass chamado: ID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[RemoveHighlight]
    ↓
[SET] bIsSelected = true
    ↓
[Format Text]
    • Format: "✅ bIsSelected = true"
    ↓
[Print String]
    • In String: (resultado)
    ↓
[Call Delegate] OnClassSelected
    • ClassID: ClassID
    ↓
[Format Text]
    • Format: "📡 Broadcast OnClassSelected: ID {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[HighlightSelected]
```

**Teste:** Ao clicar, você deve ver todos os prints na ordem.

---

### **PASSO 3: Verificar se o Bind Está Funcionando**

No `BP_CharacterCreationManager`, no `Event BeginPlay`:

**Adicione prints:**

```
[Event BeginPlay]
    ↓
[Get All Actors of Class] (BP_Class_Placeholder)
    ↓
[SET] AllPlaceholders
    ↓
[Format Text]
    • Format: "📋 Placeholders encontrados: {0}"
    • {0}: (Length of AllPlaceholders)
    ↓
[Print String]
    • In String: (resultado)
    ↓
[For Each Loop]
    • Array: AllPlaceholders
    ↓ Loop Body
    • Array Element: (BP_Class_Placeholder)
    ↓
[Get] ClassID (do Array Element)
    ↓
[Format Text]
    • Format: "🔗 Fazendo bind para ClassID: {0}"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[Assign Delegate] OnClassSelected
    • Target: Array Element
    • Event: OnClassSelected_Event
```

**Teste:** Ao iniciar o jogo, você deve ver os prints mostrando quantos placeholders foram encontrados e para quais ClassIDs o bind foi feito.

---

### **PASSO 4: Verificar se o Handler Está Recebendo o Broadcast**

No `BP_CharacterCreationManager`, no `OnClassSelected_Event`:

**Adicione prints no INÍCIO:**

```
[OnClassSelected_Event] (Custom Event)
    • ClassID (input)
    ↓
[Format Text]
    • Format: "📡📡📡 HANDLER RECEBEU BROADCAST: ClassID {0} 📡📡📡"
    • {0}: ClassID
    ↓
[Print String]
    • In String: (resultado)
    ↓
[Get Game Instance]
    ↓
[Resto da lógica...]
```

**Teste:** Ao clicar, você deve ver este print **ANTES** de qualquer outra coisa.

**Se NÃO aparecer:**
- O bind não está funcionando
- O Event Dispatcher não está sendo broadcast corretamente
- O handler não está conectado corretamente

---

## 🔍 **PROBLEMAS COMUNS E SOLUÇÕES**

### **Problema 1: OnClicked Não Dispara**

**Sintomas:**
- Nenhum print aparece ao clicar
- A câmera não move (se estiver conectada ao OnClicked)

**Soluções:**
1. **Verificar Collision_Box:**
   - No `BP_Class_Placeholder`, selecione o `Collision_Box`
   - No **Details**, verifique:
     - ✅ **Collision Enabled:** "Query and Physics"
     - ✅ **Object Type:** "WorldDynamic" ou "WorldStatic"
     - ✅ **Generate Overlap Events:** Desmarcado (não precisa)
     - ✅ **Generate Hit Events:** Desmarcado (não precisa)
     - ✅ O `Collision_Box` está **visível** e **não está dentro de outro componente**

2. **Verificar se há outro componente bloqueando:**
   - Se houver um Skeletal Mesh ou Static Mesh na frente, ele pode estar bloqueando o clique
   - Solução: Aumente o tamanho do `Collision_Box` ou mova-o para frente

3. **Verificar Input Mode:**
   - No `BP_Class_Placeholder`, no `Event BeginPlay`, adicione:
     ```
     [Event BeginPlay]
         ↓
     [Get Player Controller]
         • Player Index: 0
         ↓
     [Set Input Mode Game Only]
         • Target: (Player Controller)
     ```

### **Problema 2: SelectClass Não É Chamado**

**Sintomas:**
- Print "🖱️ CLIQUE DETECTADO!" aparece
- Print "🎯 SelectClass chamado" NÃO aparece

**Soluções:**
1. **Verificar conexão:**
   - Certifique-se de que o `OnClicked` está conectado ao `SelectClass`
   - Verifique se o `ClassID` está sendo passado corretamente

2. **Verificar se a função existe:**
   - No painel **My Blueprint** → **Functions**, verifique se `SelectClass` existe
   - Verifique se tem o input `ClassID` (Integer)

### **Problema 3: Broadcast Não Chega ao Handler**

**Sintomas:**
- Print "📡 Broadcast OnClassSelected" aparece
- Print "📡📡📡 HANDLER RECEBEU BROADCAST" NÃO aparece

**Soluções:**
1. **Verificar se o bind está sendo feito:**
   - Certifique-se de que o `Event BeginPlay` está executando
   - Verifique se o `For Each Loop` está iterando sobre os placeholders
   - Verifique se o `Assign Delegate` está sendo executado

2. **Verificar se o Event Dispatcher tem o parâmetro:**
   - No `BP_Class_Placeholder`, no painel **My Blueprint** → **Event Dispatchers**
   - Selecione `OnClassSelected`
   - Verifique se tem o parâmetro `ClassID` (Integer)

3. **Verificar se o Handler tem o parâmetro:**
   - No `BP_CharacterCreationManager`, no `OnClassSelected_Event`
   - Verifique se tem o input `ClassID` (Integer)

4. **Tentar usar "Bind Event" em vez de "Assign Delegate":**
   - No `For Each Loop`, em vez de `Assign Delegate`, use:
     ```
     [Bind Event to OnClassSelected]
         • Target: Array Element
         • Event: OnClassSelected_Event
     ```

---

## ✅ **CHECKLIST FINAL**

- [ ] Print "🖱️ CLIQUE DETECTADO!" aparece ao clicar
- [ ] Print "🎯 SelectClass chamado" aparece ao clicar
- [ ] Print "📡 Broadcast OnClassSelected" aparece ao clicar
- [ ] Print "📡📡📡 HANDLER RECEBEU BROADCAST" aparece ao clicar
- [ ] `Collision_Box` está configurado corretamente
- [ ] `Collision_Box` está visível e não está bloqueado
- [ ] Event Dispatcher `OnClassSelected` tem parâmetro `ClassID`
- [ ] Handler `OnClassSelected_Event` tem parâmetro `ClassID`
- [ ] Bind está sendo feito no `Event BeginPlay`
- [ ] Todos os prints aparecem na ordem correta

---

## 🎯 **RESUMO**

**Ordem esperada dos prints:**
1. 🖱️ CLIQUE DETECTADO!
2. ClassID: X
3. 🎯 SelectClass chamado: ID X
4. ✅ bIsSelected = true
5. 📡 Broadcast OnClassSelected: ID X
6. 📡📡📡 HANDLER RECEBEU BROADCAST: ClassID X 📡📡📡
7. (Resto da lógica do handler)

**Se algum print não aparecer, o problema está naquele ponto específico.**

---

**Fim do Guia**

