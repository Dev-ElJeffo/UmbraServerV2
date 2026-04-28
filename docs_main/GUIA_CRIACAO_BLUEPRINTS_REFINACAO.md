# Guia - Criação de Blueprints de Refinação no Unreal Engine 5.6.1

## 📋 Visão Geral

Este guia explica como criar os Blueprints necessários para o sistema de refinação no Unreal Editor.

## ⚠️ Informações Importantes

### UmbraRefinementLibrary (Helper Functions)

O sistema fornece uma **Blueprint Function Library** com funções úteis:

| Função | Descrição | Uso |
|--------|-----------|-----|
| `GetItemDisplayName` | Retorna nome com sufixo (ex: "Espada +5") | Exibir nome de item refinado |
| `IsItemRefinable` | Verifica se item pode ser refinado | Validar antes de mostrar UI |
| `IsItemTradeable` | Verifica se item pode ser negociado | Trade/Auction/Shop |
| `GetSuccessRateColor` | Retorna cor baseada na taxa | Colorir texto de probabilidade |
| `FormatSuccessRate` | Formata taxa como "75.0%" | Exibir probabilidade |

**Como usar no Blueprint:**
1. Clique com botão direito no Event Graph
2. Digite "Refinement" na busca
3. Selecione a função desejada
4. Conecte o ItemSlot como input

**Exemplo:** Para mostrar nome com refinação em um tooltip:
```
ItemSlot → GetItemDisplayName → Set Text (TextBlock)
```

## 🎨 WBP_RefinementWindow (Widget Principal)

### Passo 1: Criar Widget Blueprint

1. No Content Browser, navegue até `Content/UI/Widgets/`
2. Clique com botão direito → `User Interface` → `Widget Blueprint`
3. Nome: `WBP_RefinementWindow`
4. Abra o Blueprint

### Passo 2: Configurar Parent Class

1. No painel `Class Settings`, configure:
   - **Parent Class**: `UmbraRefinementWidget` (C++)
   - Isso permite usar toda a lógica C++ implementada

### Passo 3: Layout da UI (Hierarchy)

Crie a seguinte hierarquia de widgets:

```
Canvas Panel
├── Border (Background)
│   └── Vertical Box (MainContainer)
│       ├── Text Block (TitleText) - "SISTEMA DE REFINAÇÃO"
│       ├── Horizontal Box (SlotsRow)
│       │   ├── Border (ItemSlotBorder) - **BindWidget**
│       │   │   ├── Image (ItemIcon) - **BindWidget**
│       │   │   └── Text Block (ItemNameText) - **BindWidget**
│       │   ├── Text Block - "+" (decorativo)
│       │   └── Border (MaterialSlotBorder) - **BindWidget**
│       │       ├── Image (MaterialIcon) - **BindWidget**
│       │       └── Text Block (MaterialQuantityText) - **BindWidget**
│       ├── Text Block (CurrentLevelText) - **BindWidget**
│       ├── Horizontal Separator
│       ├── Text Block (SuccessRateText) - **BindWidget**
│       ├── Text Block (RequiredMaterialText) - **BindWidget**
│       ├── Horizontal Box (StatsRow)
│       │   ├── Vertical Box (CurrentStats)
│       │   │   ├── Text Block - "STATS ATUAIS"
│       │   │   └── Text Block (CurrentStatsText) - **BindWidget**
│       │   └── Vertical Box (NextLevelStats)
│       │       ├── Text Block - "PRÓXIMO NÍVEL"
│       │       └── Text Block (NextLevelStatsText) - **BindWidget**
│       ├── Horizontal Separator
│       ├── Text Block (InfoMessageText) - **BindWidget**
│       └── Horizontal Box (ButtonsRow)
│           ├── Button (RefineButton) - **BindWidget**
│           │   └── Text Block - "REFINAR"
│           └── Button (CancelButton) - **BindWidget**
│               └── Text Block - "CANCELAR"
```

### Passo 4: Configurar BindWidget

Para cada componente marcado como **BindWidget**:

1. Selecione o componente na hierarquia
2. No painel Details, configure `Is Variable` = `True`
3. O nome da variável **DEVE** ser exatamente o mesmo do C++ (ex: `ItemSlotBorder`)

### Passo 5: Estilizar a UI

**Background:**
- Color: Preto semi-transparente (R:0, G:0, B:0, A:0.8)
- Border: 2px, Cor dourada (R:1, G:0.84, B:0, A:1)

**ItemSlotBorder e MaterialSlotBorder:**
- Size: 128x128
- Color: Cinza escuro (R:0.1, G:0.1, B:0.1, A:1)
- Border: 2px, Cor azulada

**Botões:**
- RefineButton: Verde/Dourado
- CancelButton: Cinza

**Textos:**
- Títulos: Cor dourada, tamanho 24
- Stats: Cor branca, tamanho 14
- InfoMessageText: Cor branca, tamanho 16

### Passo 6: Eventos Blueprint (Opcional)

Você pode implementar eventos visuais:

1. **Event OnRefinementSuccessEvent**:
   ```
   - Play Animation (Success Flash)
   - Play Sound (Success Sound)
   ```

2. **Event OnRefinementFailureEvent**:
   ```
   - Play Animation (Shake)
   - Play Sound (Failure Sound)
   ```

### Passo 7: Teste no Editor

1. Compile e Save
2. Adicione ao viewport para testar layout
3. Ajuste tamanhos e posições conforme necessário

---

## 🏷️ WBP_RefinementTooltip (Widget de Tooltip)

### Passo 1: Criar Widget Blueprint

1. Content Browser → `Content/UI/Widgets/`
2. Criar novo `Widget Blueprint`
3. Nome: `WBP_RefinementTooltip`
4. Parent Class: `UmbraRefinementTooltipWidget`

### Passo 2: Layout da UI

```
Canvas Panel
└── Border (Background)
    └── Vertical Box (MainContainer)
        ├── Text Block (ItemNameText) - **BindWidget**
        ├── Text Block (RefinementLevelText) - **BindWidget**
        ├── Text Block (SuccessRateText) - **BindWidget**
        ├── Horizontal Separator
        ├── Text Block - "STATS BASE"
        ├── Vertical Box (StatsContainer) - **BindWidget**
        ├── Horizontal Separator
        ├── Text Block - "BÔNUS DE REFINAÇÃO"
        └── Vertical Box (BonusStatsContainer) - **BindWidget**
```

### Passo 3: Estilizar

- Background: Preto semi-transparente
- Tamanho: 300x400 (aproximado)
- Padding: 10px
- Textos em verde para bônus

---

## 🔗 Integrando com o Inventário

### Atualizar WBP_Inventory (se existir)

1. Abra seu widget de inventário existente
2. Adicione um botão "Refinar" na UI
3. No Event Graph:

```blueprint
Event OnRefineButtonClicked
    └── Create Widget (Class: WBP_RefinementWindow)
        └── Add to Viewport
```

### Configurar Drag & Drop

No widget de slot de inventário:

```blueprint
Event OnDragDetected
    └── Create Drag Drop Operation
        └── Set Payload (Inventory Slot Data)

Event OnDrop (no WBP_RefinementWindow)
    └── Get Drag Drop Payload
        └── Call SetItemToRefine (ou SetRefinementMaterial)
```

---

## 🎮 Testando o Sistema

### 1. Preparar Banco de Dados

Execute o script SQL:
```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/add_refinement_system.sql
```

### 2. Criar Itens de Teste

Via `create_item.html`:
- Criar uma arma com `can_be_refined = true`
- Adicionar Fragmentos e Pedras ao inventário

### 3. Testar no Jogo

1. Faça login no jogo
2. Abra o inventário
3. Clique no botão "Refinar"
4. Arraste um item e um material
5. Clique em "Refinar"
6. Observe o resultado (sucesso/falha)

### 4. Verificar Logs

**Output Log (UE):**
```
UmbraRefinementSubsystem: Loaded 13 refinement configs
UmbraRefinementWidget: Item selected: Espada de Teste (Level +0)
UmbraRefinementSubsystem: Refinement SUCCESS! New level: +1
```

**Console do Navegador (test_refinement.html):**
- Abra `http://localhost/umbra_api/admin/test_refinement.html`
- Teste os endpoints diretamente

---

## 🐛 Troubleshooting

### Widget não aparece
- Verificar se Parent Class está correta
- Verificar se todos os BindWidget estão configurados

### BindWidget errors
- Nome da variável deve ser **EXATAMENTE** igual ao C++
- Variável deve ter `Is Variable = True`

### Refinação não funciona
- Verificar logs do servidor PHP
- Verificar se JWT token é válido
- Verificar se configuração foi carregada (`LoadRefinementConfig()`)

### Stats não aparecem
- Verificar se `RefinementBonusStats` está sendo calculado
- Verificar JSON no banco de dados
- Recarregar inventário após refinação

### "GetDisplayName() não encontrado no Blueprint"
- **Causa:** A função foi removida da struct (structs não suportam UFUNCTION)
- **Solução:** Use `UmbraRefinementLibrary::GetItemDisplayName()` ao invés
- No Blueprint: busque por "Get Item Display Name" e passe o ItemSlot

### Erro de compilação C++: `Resistance` não encontrado
- **Causa:** Nome incorreto do membro da struct
- **Solução:** Use `CriticalResistance` ao invés de `Resistance`

### Tooltip não exibe stats corretos
- Verificar se `UmbraRefinementTooltipWidget::SetupTooltip()` está sendo chamado
- Verificar se `RefinementBonusStats` contém valores válidos
- Usar `CriticalResistance` ao invés de `Resistance` nos stats de combate

---

## ✅ Checklist Final

- [ ] WBP_RefinementWindow criado e estilizado
- [ ] WBP_RefinementTooltip criado
- [ ] Todos os BindWidget configurados corretamente
- [ ] Botão "Refinar" adicionado ao inventário
- [ ] Drag & Drop implementado
- [ ] Animações/Sons adicionados (opcional)
- [ ] Testado com itens +0 a +12
- [ ] Testado falha de refinação
- [ ] Validação de materiais funcionando

---

## 📚 Referências

- Documentação completa: `docs_main/GUIA_SISTEMA_REFINACAO_UE561.md`
- Código C++: `UmbraEternumUE/Source/UmbraEternumUE/Systems/UmbraRefinementSubsystem.*`
- Teste HTML: `www/umbra_api/admin/test_refinement.html`
- Helper Library: `UmbraEternumUE/Source/UmbraEternumUE/Systems/UmbraRefinementLibrary.*`

---

**Versão:** 1.1  
**Data:** 27/04/2026  
**Última atualização:** Adicionadas informações sobre UmbraRefinementLibrary e correções comuns  

**Boa sorte com a implementação! 🎮✨**
