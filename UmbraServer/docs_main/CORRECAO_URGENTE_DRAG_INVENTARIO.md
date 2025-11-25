# 🚨 CORREÇÃO URGENTE: INVENTÁRIO FECHA AO ARRASTAR

## ❌ **PROBLEMAS IDENTIFICADOS NO SEU BLUEPRINT:**

### **PROBLEMA 1: OnMouseButtonDown - Offset calculado ERRADO**

**O que você fez:**
```
A = GetMousePositionOnViewport (posição do mouse)
B = PointerEvent_GetScreenSpacePosition (posição do mouse)
Offset = A - B (isso dá ~0!)
```

**O CORRETO:**
```
A = PointerEvent_GetScreenSpacePosition (posição do mouse)
B = GetPositionInViewport (self) (posição do WIDGET)
Offset = A - B
```

---

### **PROBLEMA 2: OnDrop - Está DIVIDINDO ao invés de SUBTRAIR**

**O que você fez:**
```
Position = GetScreenSpacePosition / DragOffset  ← ERRADO!
```

**O CORRETO:**
```
Position = GetScreenSpacePosition - DragOffset  ← SUBTRAIR!
```

---

### **PROBLEMA 3: FALTA o OnDragDetected!**

**Você NÃO implementou o `OnDragDetected`!**

Sem esse override, o drag não funciona corretamente e por isso o inventário fecha.

---

## ✅ **CORREÇÕES A FAZER:**

### **CORREÇÃO 1: OnMouseButtonDown**

**DELETAR:**
- O nó `GetMousePositionOnViewport` (você está usando 2 métodos de pegar mouse ao mesmo tempo!)

**ADICIONAR:**
- `Get Position in Viewport` (Target: self)

**RECONECTAR:**
O Subtract deve ficar assim:
```
A = PointerEvent_GetScreenSpacePosition (MOUSE)
B = GetPositionInViewport (self) (WIDGET)
Resultado = A - B = OFFSET
```

**Passo a passo:**
1. Delete o nó `Get Mouse Position on Viewport`
2. Adicione `Get Position in Viewport` (Target: self)
3. Reconecte o `Subtract`:
   - Pin A: `PointerEvent_GetScreenSpacePosition`
   - Pin B: `Get Position in Viewport` (self)

---

### **CORREÇÃO 2: OnDrop**

**TROCAR:**
- O nó `Divide (/)` por `Subtract (-)`

**Passo a passo:**
1. Delete o nó `Divide`
2. Adicione `Subtract (Vector2D - Vector2D)`
3. Conecte:
   - Pin A: `PointerEvent_GetScreenSpacePosition`
   - Pin B: `Get DragOffset`
4. Conecte o resultado ao `Set Position in Viewport`

---

### **CORREÇÃO 3: Criar OnDragDetected**

**ADICIONAR o override OnDragDetected:**

1. No painel **Functions** (esquerda)
2. Clique em **Override**
3. Selecione **"On Drag Detected"**

4. Implemente assim:

```blueprint
[On Drag Detected]
    │
    ▼
[Create Drag Drop Operation]
    │ Class: DragDropOperation
    │ Pivot: Mouse Down
    │ Offset: (0, 0)
    │ Default Drag Visual: self
    │
    └─> Return Value
        │
        └─> [Return Node]
```

**Passo a passo:**
1. Adicione `Create Drag Drop Operation`
2. Configure:
   - Class: DragDropOperation (deixe padrão)
   - Pivot: Mouse Down
   - Default Drag Visual: self (o próprio inventário)
3. Conecte o Return Value ao Return Node

---

## 📋 **RESUMO DAS MUDANÇAS:**

### **OnMouseButtonDown:**
```
❌ ANTES:
[Get Mouse Position on Viewport] ─┐
                                   ├─> [Subtract]
[PointerEvent Get Screen Space] ──┘

✅ AGORA:
[PointerEvent Get Screen Space] ──┐
                                   ├─> [Subtract] ─> [Set DragOffset]
[Get Position in Viewport (self)]─┘
```

### **OnDrop:**
```
❌ ANTES:
[Get Screen Space] ─┐
                    ├─> [DIVIDE] ─> [Set Position]
[Get DragOffset] ───┘

✅ AGORA:
[Get Screen Space] ─┐
                    ├─> [SUBTRACT] ─> [Set Position]
[Get DragOffset] ───┘
```

### **OnDragDetected:**
```
❌ ANTES: NÃO EXISTE

✅ AGORA:
[On Drag Detected]
    ↓
[Create Drag Drop Operation] (Default Drag Visual: self)
    ↓
[Return Node]
```

---

## 🎯 **POR QUE O INVENTÁRIO FECHAVA?**

1. **Offset errado:** Você estava subtraindo 2 posições de mouse (dá ~0)
2. **Divisão errada:** Você estava DIVIDINDO ao invés de SUBTRAIR
3. **Falta OnDragDetected:** Sem isso, o sistema de drag não inicializa corretamente

O `OnMouseButtonDown` só prepara o drag, mas o `OnDragDetected` é quem REALMENTE inicia a operação de drag!

---

## ✅ **CHECKLIST DE VERIFICAÇÃO:**

**OnMouseButtonDown:**
- [ ] Removido `Get Mouse Position on Viewport`
- [ ] Adicionado `Get Position in Viewport (self)`
- [ ] Subtract conectado: `(PointerEvent Screen Space) - (Widget Position)`
- [ ] DragOffset sendo salvo corretamente
- [ ] DetectDragIfPressed ainda está lá

**OnDragDetected:**
- [ ] Override criado
- [ ] Create Drag Drop Operation adicionado
- [ ] Default Drag Visual = self
- [ ] Return Value conectado ao Return Node

**OnDrop:**
- [ ] Trocado Divide por Subtract
- [ ] Conectado: `(Get Screen Space) - (DragOffset)`
- [ ] Set Position in Viewport com Remove DPI Scale = FALSE
- [ ] Return Value do Return Node pode ficar vazio

---

## 🔥 **FAÇA ISSO AGORA:**

1. ✅ **OnMouseButtonDown:** Troque `GetMousePositionOnViewport` por `GetPositionInViewport(self)`
2. ✅ **OnDrop:** Troque `Divide` por `Subtract`
3. ✅ **Adicione OnDragDetected:** Override + Create Drag Drop Operation

Depois dessas 3 correções, o inventário VAI funcionar!

---

