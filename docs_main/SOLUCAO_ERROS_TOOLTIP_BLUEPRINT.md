# 🔧 Solução: Erros de Compilação WBP_ItemTooltip1

## ❌ Problemas Identificados

### Erro 1: BindWidgets Obrigatórios Não Encontrados
```
A exigem widget Associação "ItemNameText" of type Text was não encontrado.
A exigem widget Associação "ItemTypeText" of type Text was não encontrado.
A exigem widget Associação "RarityText" of type Text was não encontrado.
```

**Causa:** O Blueprint não tem os 3 TextBlocks obrigatórios que o C++ espera.

### Erro 2: Conflito de Função SetTooltipData
```
The function name in node SetTooltipData is already used
O pin In Slot Data que está em uso não existe mais no nódulo SetTooltipData
A função substituída não é compatível com a função pai SetTooltipData
```

**Causa:** O Blueprint tem uma função Blueprint chamada `SetTooltipData` que conflita com a função C++ do mesmo nome.

---

## ✅ Solução Completa (Passo a Passo)

### ETAPA 1: Remover Função Blueprint Conflitante ⚠️

1. **Abrir WBP_ItemTooltip1** no Unreal
2. **Ir para Event Graph**
3. **Localizar função Blueprint:** `SetTooltipData`
4. **Deletar completamente** essa função (clique direito → Delete)
   - Se houver múltiplos Event Graphs, verificar todos
5. **Compile** (vai dar erro ainda, mas é esperado)

**Por quê?** A função C++ `SetTooltipData` já existe e não pode ser substituída/sobrescrita no Blueprint.

---

### ETAPA 2: Adicionar TextBlocks Obrigatórios no Designer ⚠️

**No Designer do WBP_ItemTooltip1:**

#### 1. Adicionar ItemNameText
1. Arrastar **Text Block** da paleta para o canvas
2. **Renomear** (F2 ou Details): `ItemNameText` (nome **exato**)
3. **Details:**
   - ✅ **Is Variable:** TRUE (MUITO IMPORTANTE!)
   - **Text:** "Nome do Item"
   - **Font Size:** 18-20 (ajuste conforme design)
4. **Posicionar** no topo do tooltip

#### 2. Adicionar ItemTypeText
1. Arrastar **Text Block** da paleta
2. **Renomear:** `ItemTypeText` (nome **exato**)
3. **Details:**
   - ✅ **Is Variable:** TRUE
   - **Text:** "Tipo"
   - **Font Size:** 14-16
4. **Posicionar** abaixo do nome

#### 3. Adicionar RarityText
1. Arrastar **Text Block** da paleta
2. **Renomear:** `RarityText` (nome **exato**)
3. **Details:**
   - ✅ **Is Variable:** TRUE
   - **Text:** "Raridade"
   - **Font Size:** 14-16
4. **Posicionar** ao lado ou abaixo do tipo

---

### ETAPA 3: Adicionar RefinementLevelText (Opcional, mas Recomendado) ⚠️

1. Arrastar **Text Block** da paleta
2. **Renomear:** `RefinementLevelText` (nome **exato**)
3. **Details:**
   - ✅ **Is Variable:** TRUE
   - **Visibility:** **Collapsed** (oculto por padrão)
   - **Text:** "Nível: +12"
   - **Color:** Amarelo (#FFD700)
   - **Font Size:** 16-18
4. **Posicionar** abaixo do nome (aparecerá apenas em itens refinados)

---

### ETAPA 4: Compile e Verificar ⚠️

1. **Compile** (botão verde)
2. **Verificar erros:**
   - ✅ Não deve ter mais "was não encontrado" para os 3 obrigatórios
   - ✅ Não deve ter mais erro de "SetTooltipData is already used"

**Se ainda houver erros:**
- Verificar se os nomes estão **exatos** (case-sensitive)
- Verificar se **Is Variable = TRUE** em todos os 3
- Verificar se a função Blueprint `SetTooltipData` foi **deletada**

3. **Save** (Ctrl+S)

---

### ETAPA 5: Atualizar Chamadas ao Tooltip (Se Necessário) ⚠️

**Se você estava chamando o tooltip assim (Blueprint):**
```
Create Widget (WBP_ItemTooltip1)
  → Set Tooltip Data (com múltiplos pins)
  → Add to Viewport
```

**Agora deve ser assim (C++):**
```
Create Widget (WBP_ItemTooltip1)
  → SetTooltipData (1 pin: ItemSlot do tipo FUmbraInventorySlot)
  → Add to Viewport
```

**Diferenças:**
- Antes: múltiplos parâmetros (ItemName, ItemType, etc.)
- Agora: **1 único parâmetro** (`ItemSlot` do tipo `FUmbraInventorySlot`)

---

## 🎯 Checklist de Validação

### Designer
- [ ] `ItemNameText` existe e **Is Variable = TRUE**
- [ ] `ItemTypeText` existe e **Is Variable = TRUE**
- [ ] `RarityText` existe e **Is Variable = TRUE**
- [ ] `RefinementLevelText` existe (opcional) e **Is Variable = TRUE**
- [ ] Todos os nomes estão **exatos** (case-sensitive)

### Event Graph
- [ ] Função Blueprint `SetTooltipData` foi **deletada**
- [ ] Não há outras funções Blueprint que conflitam com C++

### Compilação
- [ ] **Compile** sem erros
- [ ] Sem "was não encontrado" para os 3 obrigatórios
- [ ] Sem "is already used"

### Chamadas
- [ ] Quem chama o tooltip usa `SetTooltipData(ItemSlot)` com 1 parâmetro

---

## 📋 Estrutura Mínima do Designer

```
WBP_ItemTooltip1 (Parent: UmbraItemTooltipWidget)
├── Canvas Panel / Border / Overlay (container)
│   ├── ItemNameText (TextBlock) ← OBRIGATÓRIO
│   ├── ItemTypeText (TextBlock) ← OBRIGATÓRIO
│   ├── RarityText (TextBlock) ← OBRIGATÓRIO
│   ├── RefinementLevelText (TextBlock, Collapsed) ← OPCIONAL
│   ├── StrengthText (TextBlock) ← OPCIONAL
│   ├── DexterityText (TextBlock) ← OPCIONAL
│   ├── PhysicalAttackText (TextBlock) ← OPCIONAL
│   └── ... (outros stats opcionais)
```

**Hierarquia:** Todos os TextBlocks devem estar dentro de um container (Canvas, Border, Overlay, Vertical/Horizontal Box).

---

## 🐛 Troubleshooting

### Erro: "was não encontrado" persiste após adicionar TextBlock

**Solução:**
1. Verificar **nome exato** (case-sensitive): `ItemNameText` ≠ `itemnametext`
2. Verificar **Is Variable = TRUE** (Details panel)
3. **Compile** novamente
4. Se persistir: **Fechar e reabrir o Blueprint**

### Erro: "is already used" persiste

**Solução:**
1. Ir para **Event Graph**
2. Verificar **TODOS os Event Graphs** (pode ter mais de um)
3. Procurar por função/evento chamado `SetTooltipData`
4. **Deletar completamente**
5. **Compile** novamente

### Erro: "O pin In Slot Data não existe mais"

**Solução:**
1. Isso ocorre porque o Blueprint tinha uma função com parâmetros diferentes
2. **Deletar a função Blueprint SetTooltipData** (completa)
3. Atualizar chamadas para usar o novo parâmetro `ItemSlot` (struct completo)

### Erro: "Cannot encomendar parâmetros ItemSlot"

**Solução:**
1. A função C++ espera `const FUmbraInventorySlot& ItemSlot`
2. Quem chama deve passar o **struct completo**, não campos individuais
3. Exemplo correto:
```
// Onde você cria o tooltip:
FUmbraInventorySlot MyItemSlot = GetCurrentItemSlot(); // pega do slot de inventário
SetTooltipData(MyItemSlot); // passa o struct completo
```

---

## 📖 Exemplo de Uso Correto

### Código que chama o tooltip (Blueprint ou C++):

**Blueprint:**
```
Event On Mouse Enter (no slot de inventário)
  → Get Slot Data (retorna FUmbraInventorySlot)
  → Create Widget (Class: WBP_ItemTooltip1)
  → SetTooltipData (ItemSlot: conectar o struct completo)
  → Add to Viewport
```

**C++:**
```cpp
// No UmbraInventorySlotWidget.cpp ou similar
void UUmbraInventorySlotWidget::ShowTooltip()
{
    if (TooltipWidgetClass)
    {
        UUmbraItemTooltipWidget* Tooltip = CreateWidget<UUmbraItemTooltipWidget>(GetWorld(), TooltipWidgetClass);
        if (Tooltip)
        {
            Tooltip->SetTooltipData(SlotData); // SlotData é FUmbraInventorySlot
            Tooltip->AddToViewport();
        }
    }
}
```

---

## ✅ Resultado Esperado Após Correção

### Compilação
```
Compilation of WBP_ItemTooltip1 successful. 0 Warnings, 0 Errors
```

### Logs (Output Log)
```
LogTemp: Log: [UmbraItemTooltipWidget] Tooltip construído
LogTemp: Log: [UmbraItemTooltipWidget] Item: Espada de Ferro, Refinado: SIM, Level: 12
LogTemp: Verbose: [UmbraItemTooltipWidget] Populando tooltip refinado (Level: 12)
```

### Visual
- ✅ Tooltip aparece com nome correto
- ✅ Tipo exibido (ex: "Arma", "Armadura")
- ✅ Raridade exibida (ex: "Raro", "Épico")
- ✅ Se refinado: nome com "+12", indicador visível, stats totais

---

## 📚 Documentação Relacionada

- **Guia Completo C++:** `docs_main/GUIA_TOOLTIP_REFINACAO_CPP.md`
- **Próximos Passos:** `docs_main/PROXIMOS_PASSOS_TOOLTIP_CPP.md`
- **Checklist:** `docs_main/CHECKLIST_VALIDACAO_TOOLTIP_CPP.md`

---

**Versão:** 1.0  
**Data:** 28/04/2026  
**Status:** Guia de solução de erros  
**Próximo Passo:** Executar ETAPA 1 (remover função Blueprint)
