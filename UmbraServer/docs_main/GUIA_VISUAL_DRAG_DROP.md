# Guia Completo: Visual de Drag & Drop para Inventário

## 📋 Visão Geral

Este guia explica como criar e implementar o visual que aparece enquanto você arrasta um item no inventário/storage.

### Por que é importante?

O visual de drag:
1. **Mostra ao jogador o que está sendo arrastado**
2. **Deve ser configurado como Not Hit-Testable** para não interceptar o drop
3. **Deve ser um widget SEPARADO** (nunca use o próprio slot como visual!)

---

## 🛠️ Passo 1: Criar o Widget de Drag Visual

### 1.1 Criar o Widget Blueprint

1. No **Content Browser**, navegue até sua pasta de widgets (ex: `Content/Widgets/UI/Inventory/`)
2. **Clique direito** → **User Interface** → **Widget Blueprint**
3. Nomeie como `WBP_DragVisual` (ou similar)

### 1.2 Estruturar o Layout

No **Designer** do widget:

```
[Canvas Panel] (raiz)
└── [Size Box] (opcional, para tamanho fixo)
    └── [Border ou Image] (fundo semi-transparente)
        └── [Image] (ícone do item)
```

**Configurações recomendadas:**
- **Size Box**: 64x64 ou 80x80 pixels
- **Border/Background**: Cor semi-transparente (ex: azul com alpha 0.5)
- **Image**: Bind ao ícone do item sendo arrastado

### 1.3 ⚠️ CRÍTICO: Configurar Visibility

**Este é o passo mais importante!**

No widget **WBP_DragVisual**:
1. Selecione o **Canvas Panel** (raiz)
2. No painel **Details**, procure **Behavior** → **Visibility**
3. Altere para: **`Not Hit-Testable (Self and All Children)`**

Isso garante que o visual não intercepte os cliques de drop!

---

## 🔧 Passo 2: Configurar o Slot de Inventário

### 2.1 Definir a Classe do Visual

No widget **WBP_InventorySlot**:

1. Abra o Blueprint
2. Selecione o próprio widget (Self) no **Components** ou clique fora do canvas
3. No painel **Details**, procure **Drag & Drop**
4. Em **Drag Visual Widget Class**, selecione `WBP_DragVisual`

### 2.2 Alternativa via Blueprint

Se preferir definir via Blueprint:

```
Event Begin Play
    │
    └─→ Set Drag Visual Widget Class
            └── Target: Self
            └── New Drag Visual Widget Class: WBP_DragVisual
```

---

## 📝 Passo 3: Passar Dados para o Visual (Opcional)

Se quiser mostrar o ícone/nome do item no visual:

### 3.1 Criar Variáveis no WBP_DragVisual

No **WBP_DragVisual**, adicione variáveis:
- `ItemIcon` (Texture2D ou SlateBrush)
- `ItemName` (String)
- `ItemQuantity` (Integer)

Marque como **Instance Editable** e **Expose on Spawn**.

### 3.2 Modificar CreateItemDragOperation (C++)

O código C++ já cria o widget visual. Para passar dados, você pode:

**Opção A: Definir via Blueprint após criar**
No Blueprint do slot, após chamar `CreateItemDragOperation`:

```
CreateItemDragOperation()
    │
    └─→ [Se válido]
            │
            └─→ Get Default Drag Visual
                    │
                    └─→ Cast to WBP_DragVisual
                            │
                            └─→ Set ItemIcon (do SlotData)
                            └─→ Set ItemName (do SlotData)
```

**Opção B: Usar um evento customizado**
Crie um evento no C++ que o Blueprint pode implementar para configurar o visual.

---

## 🎨 Passo 4: Exemplo de Design do WBP_DragVisual

### Hierarquia Sugerida

```
[Canvas Panel] - Visibility: Not Hit-Testable
│
└── [Size Box] - Width: 64, Height: 64
    │
    └── [Overlay]
        │
        ├── [Border] - Brush: Arredondado, Cor: (0, 0.5, 1, 0.7)
        │
        └── [Image] - Bind: ItemIcon
            │
            └── [Text Block] - Bind: ItemQuantity (canto inferior direito)
```

### Blueprint: Configurar Visual

```
Event Construct
    │
    ├─→ Set Image Brush (ItemIconImage)
    │       └── Brush: ItemIcon
    │
    └─→ Set Text (QuantityText)
            └── Text: ItemQuantity (se > 1)
```

---

## ✅ Checklist de Implementação

- [ ] Criar widget `WBP_DragVisual`
- [ ] Configurar **Visibility = Not Hit-Testable** no widget raiz
- [ ] Adicionar visual (Image, Border, etc.)
- [ ] No `WBP_InventorySlot`, definir `Drag Visual Widget Class`
- [ ] Testar: arrastar item deve mostrar o visual
- [ ] Testar: soltar item deve funcionar no slot correto

---

## 🔍 Troubleshooting

### O visual não aparece
- Verifique se `DragVisualWidgetClass` está definido no slot
- Verifique se o widget foi criado corretamente (não tem erros)

### O drop não funciona (vai para o slot errado)
- **CRÍTICO**: Verifique se o visual tem `Visibility = Not Hit-Testable`
- Se usar o próprio slot como visual, SEMPRE haverá este problema!

### O visual aparece no lugar errado
- Verifique o `Pivot` da operação de drag
- Use `EDragPivot::CenterCenter` para centralizar no cursor

---

## 📚 Código de Referência (C++)

O código em `UmbraInventorySlotWidget.cpp` já suporta visual customizado:

```cpp
// Se tiver um visual customizado definido, criar uma instância
UUserWidget* DragVisualWidget = nullptr;
if (DragVisualWidgetClass)
{
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        DragVisualWidget = CreateWidget<UUserWidget>(PC, DragVisualWidgetClass);
        // O widget é criado e será usado como visual
    }
}

DragOperation->DefaultDragVisual = DragVisualWidget;
DragOperation->Pivot = EDragPivot::CenterCenter;
```

---

## 🎯 Resultado Esperado

Após implementar:
1. Ao arrastar um item, um visual semi-transparente segue o cursor
2. O visual mostra o ícone do item
3. Ao soltar, o item vai para o slot CORRETO (não o de origem)
4. O sistema funciona tanto para inventário quanto para storage
