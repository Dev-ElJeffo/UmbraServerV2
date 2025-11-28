# 🖱️ GUIA FINAL: Double Click para Equipar/Desequipar

## ✅ **O QUE REALMENTE EXISTE:**

1. ✅ `OnMouseButtonDown` - Evento
2. ✅ `Is Mouse Button Down` - Retorna bool, permite escolher o botão
3. ❓ `Click Count` - Precisa descobrir como acessar do MouseEvent

---

## 🔧 **PASSO A PASSO:**

### **PASSO 1: Criar o Evento OnMouseButtonDown**

1. No `WBP_EquipmentSlot` (ou `WBP_InventorySlot`), Event Graph
2. Clique direito → **Override** → `OnMouseButtonDown`
3. Você terá:
   - `exec`
   - `In My Geometry` (FGeometry)
   - `In Mouse Event` (FPointerEvent) ← **ESTE!**

---

### **PASSO 2: Verificar se é Left Mouse Button**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Is Mouse Button Down`**
3. Selecione: **`Is Mouse Button Down`**
4. Você terá:
   - `Mouse Button` (EKeys) ← **ESCOLHA O BOTÃO AQUI!**
   - `Return Value` (bool) ← **TRUE se o botão está pressionado**

**COMO ESCOLHER O BOTÃO:**
- No pin `Mouse Button`, clique no dropdown
- Selecione: **`Left Mouse Button`**

---

### **PASSO 3: Tentar Obter o Click Count**

**OPÇÃO A: Procurar "Get Click Count" diretamente do MouseEvent**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. Digite: **`Get Click Count`** ou **`Click Count`**
3. Veja se aparece alguma função
4. Se aparecer, use ela

**OPÇÃO B: Arrastar o MouseEvent e ver todas as opções**

1. Arraste o pin `In Mouse Event` (FPointerEvent)
2. **NÃO digite nada ainda**
3. Veja todas as funções/propriedades que aparecem
4. Procure por algo relacionado a **"Click"** ou **"Count"**
5. Se encontrar, use

**OPÇÃO C: Se não encontrar Click Count**

Talvez você precise usar uma variável local para contar os cliques manualmente, ou usar um timer para detectar double click.

---

## 📊 **ESTRUTURA (SE ENCONTRAR CLICK COUNT):**

```
[OnMouseButtonDown]
  ├─ exec
  ├─ In My Geometry: (FGeometry)
  └─ In Mouse Event: (FPointerEvent)
       │
       ├─────────────────────────────────────┐
       │                                     │
       ▼                                     ▼
  [Is Mouse Button Down]          [Get Click Count] ← TENTE ESTE!
    ├─ Mouse Button: Left Mouse Button      └─ Return Value: (int)
    └─ Return Value: (bool)                        │
         │                                         ▼
         │                              [Equal (Int Int)]
         │                                ├─ A: Click Count
         │                                ├─ B: 2
         │                                └─ Return Value: (bool)
         │                                         │
         │                                         │
         └─────────────────────────────────────────┼─┐
                                                   │ │
                                                   ▼ ▼
                                            [Boolean AND]
                                              ├─ A: Is Mouse Button Down (bool)
                                              ├─ B: Equal (bool)
                                              └─ Return Value: (bool)
                                                   │
                                                   ├─ TRUE → [Sua lógica aqui]
                                                   └─ FALSE → [Unhandled]
```

---

## ❓ **SE NÃO ENCONTRAR CLICK COUNT:**

**Alternativa: Contar cliques manualmente**

1. Crie uma variável local: `LastClickTime` (float)
2. Crie uma variável local: `ClickCount` (int)
3. No `OnMouseButtonDown`:
   - Verifique se `Is Mouse Button Down` (Left Mouse Button)
   - Se sim:
     - Obtenha o tempo atual
     - Se a diferença entre o tempo atual e `LastClickTime` for menor que 0.3 segundos:
       - Incremente `ClickCount`
       - Se `ClickCount == 2`: Execute sua lógica
     - Senão:
       - Reset `ClickCount` para 1
     - Atualize `LastClickTime`

---

## 🎯 **O QUE VOCÊ PRECISA FAZER:**

1. ✅ Use `Is Mouse Button Down` para verificar o botão
2. ❓ **Tente encontrar como acessar o Click Count do MouseEvent:**
   - Arraste o `In Mouse Event`
   - Veja todas as opções disponíveis
   - Procure por "Click" ou "Count"
3. ❓ **Se não encontrar, me diga o que aparece quando você arrasta o MouseEvent**

---

## 📝 **ME AJUDE:**

**Quando você arrasta o pin `In Mouse Event` (FPointerEvent) e não digita nada, quais opções aparecem no menu?**

Isso vai me ajudar a descobrir como realmente acessar o Click Count no Unreal Engine! 🙏

