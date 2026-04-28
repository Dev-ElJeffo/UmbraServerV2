# Guia Completo: Implementação de Tooltip para Itens Refinados (Nó a Nó)

## 📋 Visão Geral da Abordagem

**Objetivo:** Criar uma nova função `SetRefinedItemTooltipData` que:
- Verifica se o item está refinado (`bIsItemRefined`)
- Se FALSE: chama a função `SetTooltipData` existente (SEM modificações)
- Se TRUE: popula o tooltip com stats de refinação usando o `UmbraRefinementSubsystem`

**Vantagens desta abordagem:**
- ✅ Função existente permanece intacta e funcional
- ✅ Backwards compatible (itens não refinados continuam funcionando)
- ✅ Fácil de testar e reverter se necessário
- ✅ Código limpo e organizado

---

## 🎯 Arquitetura da Solução

### Fluxo de Chamada Atual
```
WBP_InventorySlot (Event Construct ou OnHovered)
    ↓
SetTooltipData (função existente)
    ↓
Popula tooltip com ItemTemplate.Stats (stats base)
```

### Novo Fluxo de Chamada
```
WBP_InventorySlot (Event Construct ou OnHovered)
    ↓
SetRefinedItemTooltipData (NOVA função)
    ↓
    ├─ bIsItemRefined == FALSE → SetTooltipData (existente)
    │                              ↓
    │                           Stats base (comportamento atual)
    │
    └─ bIsItemRefined == TRUE → PopulateRefinedTooltip (NOVA lógica)
                                   ↓
                                Stats totais (base + bônus)
                                Nome com sufixo (+X)
                                Indicador de refinação
```

---

## 📂 Localização dos Arquivos

### Blueprints (Unreal Engine)
- **WBP_ItemTooltip**: Tooltip principal do inventário
  - Caminho provável: `/Game/Widgets/UI/Inventory/WBP_ItemTooltip`
  - Ou: `/Game/UI/WBP_ItemTooltip`

- **WBP_InventorySlot**: Slot de inventário que chama o tooltip
  - Caminho provável: `/Game/Widgets/UI/Inventory/WBP_InventorySlot`

### C++ (Referência - NÃO precisa editar)
- `UmbraRefinementSubsystem.h/cpp`: Cálculos de stats
- `UmbraRefinementLibrary.h/cpp`: Funções helper para Blueprint
- `UmbraDataStructures.h`: Structs de dados

---

## 🛠️ PARTE 1: Preparação

### PASSO 1.1: Abrir WBP_ItemTooltip

1. Abrir Unreal Engine 5.6.1
2. No Content Browser, usar busca (Ctrl+Space): `WBP_ItemTooltip`
3. Clicar duas vezes para abrir o Blueprint Editor
4. Ir para aba **Graph** (Event Graph)

### PASSO 1.2: Identificar a Função Existente

1. No painel **My Blueprint** (esquerda), procurar por:
   - `SetTooltipData` (nome provável)
   - Ou: `UpdateTooltip`, `SetItemData`, `Initialize`

2. **Abrir a função** para ver sua estrutura:
   - Input: `SlotData` (tipo `FUmbraInventorySlot`)
   - Corpo: Quebra `SlotData` e popula TextBlocks

3. **NÃO MODIFICAR ESTA FUNÇÃO!** Apenas observar para referência.

### PASSO 1.3: Documentar Componentes UI

Listar todos os TextBlocks do tooltip (para usar na nova função):

**Componentes Comuns:**
- `ItemNameText`: Nome do item
- `ItemTypeText`: Tipo (Weapon, Armor, etc)
- `RarityText`: Raridade (Common, Rare, etc)
- `PhysicalAttackText`: Ataque físico
- `MagicAttackText`: Ataque mágico
- `StrengthText`: Força
- `DexterityText`: Destreza
- `IntelligenceText`: Inteligência
- `VitalityText`: Vitalidade
- `LuckText`: Sorte
- `CriticalText`: Crítico
- `AccuracyText`: Precisão
- `DodgeText`: Esquiva
- `PhysicalDefenseText`: Defesa física
- `MagicDefenseText`: Defesa mágica
- `HealthBonusText`: HP Bônus
- `ManaBonusText`: MP Bônus

---

## 🔧 PARTE 2: Criar a Nova Função

### PASSO 2.1: Criar Função `SetRefinedItemTooltipData`

1. No painel **My Blueprint**, clicar em **+ Function**
2. Renomear para: `SetRefinedItemTooltipData`
3. Configurar Access: **Public** (para ser chamada de outros Blueprints)

### PASSO 2.2: Adicionar Input Parameter

1. Na aba **Details** da função (lado direito):
2. Seção **Inputs**, clicar em **+ Add**
3. Configurar:
   - **Name**: `InSlotData`
   - **Type**: `FUmbraInventorySlot` (struct)
   - **Pass by**: `Reference` (const)

---

## 📝 PARTE 3: Implementar a Lógica (Nó a Nó)

### PASSO 3.1: Criar Nó de Entrada

O nó de entrada `InSlotData` já existe automaticamente. Ele representa o parâmetro da função.

**Localização:** Início do Event Graph da função (lado esquerdo)

### PASSO 3.2: Verificar se Item está Refinado

**NÓ 1: Break FUmbraInventorySlot**

1. Arrastar `InSlotData` para o graph
2. Soltar e escolher: **Get** (ou arrastar pin de saída)
3. Arrastar o pin de saída de `InSlotData`
4. Soltar no vazio e buscar: `Break UmbraInventorySlot`
5. Selecionar o nó **Break UmbraInventorySlot**

**Pinos de saída relevantes:**
- `Refinement Level` (int32)
- `Refinement Bonus Stats` (FUmbraItemStats)
- `Item Template` (FUmbraItemTemplate)
- Outros...

**NÓ 2: Comparação - RefinementLevel > 0**

1. Arrastar o pin `Refinement Level` (saída do Break)
2. Soltar e buscar: `> (integer)`
3. Conectar `Refinement Level` ao primeiro input
4. No segundo input, digitar: `0`

**Resultado:** Retorna TRUE se item está refinado (+1 ou mais)

**NÓ 3: Branch (Decisão)**

1. Arrastar o pin de saída do nó `>` (booleano)
2. Soltar e buscar: `Branch`
3. Conectar:
   - **Condition**: saída do `>`
   - **Execution**: entry point da função

**Resultado:** 
- **True**: Item refinado → executar nova lógica
- **False**: Item normal → chamar função existente

---

### PASSO 3.3: Ramo FALSE - Chamar Função Existente

**NÓ 4: Chamar SetTooltipData (função existente)**

1. Do pin **False** do Branch, arrastar e soltar
2. Buscar: `SetTooltipData` (ou nome da função existente)
3. Conectar:
   - **Execution**: pin False do Branch
   - **SlotData** (input): `InSlotData` (variável de entrada)

**NÓ 5: Return**

1. Do pin de saída (execução) de `SetTooltipData`, arrastar e soltar
2. Buscar: `Return Node`
3. Conectar para finalizar este ramo

**Pronto!** Quando `RefinementLevel == 0`, usa função original.

---

### PASSO 3.4: Ramo TRUE - Nova Lógica de Tooltip Refinado

Este é o coração da implementação. Vamos criar nó por nó.

#### **SEÇÃO A: Obter RefinementSubsystem**

**NÓ 6: Get Game Instance**

1. Do pin **True** do Branch, arrastar e soltar
2. Buscar: `Get Game Instance`
3. Este nó não precisa de input

**NÓ 7: Cast to UmbraGameInstance**

1. Arrastar o pin **Return Value** de `Get Game Instance`
2. Soltar e buscar: `Cast to UmbraGameInstance`
3. Conectar:
   - **Object**: Return Value do Get Game Instance
   - **Execution**: pin True do Branch

**NÓ 8: Get Subsystem (UmbraRefinementSubsystem)**

1. Arrastar o pin **As Umbra Game Instance** (saída do Cast)
2. Soltar e buscar: `Get Subsystem`
3. Configurar:
   - **Class**: `UmbraRefinementSubsystem` (dropdown)
4. Conectar **Execution**

**NÓ 9: IsValid (validar subsystem)**

1. Arrastar o pin de saída do `Get Subsystem`
2. Soltar e buscar: `IsValid`
3. Conectar **Execution**

**NÓ 10: Branch (validação)**

1. Arrastar o pin **Is Valid** (boolean)
2. Soltar e buscar: `Branch`
3. Conectar:
   - **Condition**: Is Valid
   - **Execution**: saída do nó anterior

**NÓ 11: Return (se subsystem inválido)**

1. Do pin **False** deste Branch:
2. Arrastar e buscar: `Return Node`
3. Isso previne erros se o subsystem não existir

**Resultado:** `RefinementSubsystem` está disponível para uso no pin True.

---

#### **SEÇÃO B: Calcular Stats Totais**

**NÓ 12: Calculate Total Stats**

1. Do pin **True** do Branch (validação subsystem)
2. Arrastar o pin de saída do `Get Subsystem` (RefinementSubsystem)
3. Soltar e buscar: `Calculate Total Stats`
4. Conectar:
   - **Target**: RefinementSubsystem (saída do Get Subsystem)
   - **Item Slot**: `InSlotData` (variável de entrada)
   - **Execution**: pin True do Branch

**Resultado:** Retorna `FUmbraItemStats` com stats totais (base + bônus)

**NÓ 13: Break UmbraItemStats**

1. Arrastar o **Return Value** de `Calculate Total Stats`
2. Soltar e buscar: `Break UmbraItemStats`
3. Este nó expõe:
   - **Base**: Atributos base (Strength, Dexterity, etc)
   - **Combat**: Stats de combate (PhysicalAttack, etc)
   - **Bonus**: Bônus (HealthBonus, ManaBonus, etc)

---

#### **SEÇÃO C: Atualizar Nome do Item**

**NÓ 14: Get Item Display Name**

1. Clicar com botão direito no graph
2. Buscar: `Get Item Display Name` (UmbraRefinementLibrary)
3. Conectar:
   - **Item Slot**: `InSlotData`

**Resultado:** Retorna string como "Espada de Ferro +12"

**NÓ 15: Set Text (ItemNameText)**

1. Arrastar `ItemNameText` (variável do painel My Blueprint)
2. Soltar e escolher: `Set Text`
3. Conectar:
   - **In Text**: converter Return Value de Get Item Display Name
     - Arrastar Return Value → buscar `To Text (String)`
   - **Execution**: conectar ao fluxo principal

---

#### **SEÇÃO D: Exibir Indicador de Refinação**

**NÓ 16: Format Text (Nível de Refinação)**

1. Clicar com botão direito e buscar: `Format Text`
2. No nó, definir pattern: `Nível: +{0}`
3. Conectar:
   - **{0}**: `Refinement Level` (do Break UmbraInventorySlot inicial)

**NÓ 17: Set Text (RefinementLevelText)**

1. Arrastar `RefinementLevelText` (TextBlock - pode precisar criar)
2. Soltar e escolher: `Set Text`
3. Conectar:
   - **In Text**: Return Value do Format Text
   - **Execution**: fluxo principal

**NÓ 18: Set Visibility (RefinementLevelText)**

1. Arrastar `RefinementLevelText` novamente
2. Soltar e buscar: `Set Visibility`
3. Configurar:
   - **In Visibility**: `Visible` (dropdown)
   - **Execution**: após Set Text

---

#### **SEÇÃO E: Popular Stats de Combate**

**Grupo de Nós para cada stat de combate:**

Para **cada stat** (PhysicalAttack, MagicAttack, Critical, etc), repetir:

**Template de Nó (exemplo: PhysicalAttack):**

**NÓ 19a: Break UmbraItemStatsCombat**

1. Arrastar o pin **Combat** (do Break UmbraItemStats, nó 13)
2. Soltar e buscar: `Break UmbraItemStatsCombat`
3. Expõe todos os stats de combate

**NÓ 19b: Format Text**

1. Buscar: `Format Text`
2. Pattern: `Ataque Físico: {0}`
3. Conectar **{0}**: `Physical Attack` (do Break Combat)

**NÓ 19c: Set Text (PhysicalAttackText)**

1. Arrastar `PhysicalAttackText` (TextBlock)
2. Buscar: `Set Text`
3. Conectar:
   - **In Text**: Return Value do Format Text
   - **Execution**: fluxo principal

**Repetir para todos os stats:**
- MagicAttack → `MagicAttackText`
- Critical → `CriticalText`
- Accuracy → `AccuracyText`
- Dodge → `DodgeText`
- PhysicalDefense → `PhysicalDefenseText`
- MagicDefense → `MagicDefenseText`
- CriticalResistance → `CriticalResistanceText`
- DoubleAttackRate → `DoubleAttackRateText`
- DoubleAttackResistance → `DoubleAttackResistanceText`

---

#### **SEÇÃO F: Popular Atributos Base**

**NÓ 20a: Break UmbraItemStatsBase**

1. Arrastar o pin **Base** (do Break UmbraItemStats, nó 13)
2. Soltar e buscar: `Break UmbraItemStatsBase`
3. Expõe: Strength, Dexterity, Intelligence, Vitality, Luck

**Para cada atributo, criar grupo de nós:**

**Exemplo: Strength**

**NÓ 20b: Format Text**
- Pattern: `Força: {0}`
- **{0}**: `Strength` (do Break Base)

**NÓ 20c: Set Text (StrengthText)**
- **In Text**: Return Value
- **Execution**: fluxo

**Repetir para:**
- Dexterity → `DexterityText`
- Intelligence → `IntelligenceText`
- Vitality → `VitalityText`
- Luck → `LuckText`

---

#### **SEÇÃO G: Popular Bônus**

**NÓ 21a: Break UmbraItemStatsBonus**

1. Arrastar o pin **Bonus** (do Break UmbraItemStats, nó 13)
2. Soltar e buscar: `Break UmbraItemStatsBonus`
3. Expõe: HealthBonus, ManaBonus, Movement

**Para cada bônus:**

**Exemplo: HealthBonus**

**NÓ 21b: Format Text**
- Pattern: `HP Bônus: {0}`
- **{0}**: `Health Bonus`

**NÓ 21c: Set Text (HealthBonusText)**
- **In Text**: Return Value
- **Execution**: fluxo

**Repetir para:**
- ManaBonus → `ManaBonusText`
- Movement → `MovementText`

---

#### **SEÇÃO H: Exibir Bônus de Refinação em Verde (OPCIONAL)**

Se quiser mostrar os bônus separadamente em verde:

**NÓ 22: Break UmbraInventorySlot (RefinementBonusStats)**

1. Usar o Break do nó 1 (já existe)
2. Arrastar o pin `Refinement Bonus Stats`
3. Soltar e buscar: `Break UmbraItemStats`

**NÓ 23: Break UmbraItemStatsCombat (Bônus)**

1. Arrastar **Combat** do Break RefinementBonusStats
2. Buscar: `Break UmbraItemStatsCombat`

**Para cada stat de bônus:**

**Exemplo: Physical Attack Bônus**

**NÓ 23a: Greater (PhysicalAttack > 0)**
- Comparar se bônus existe: `Physical Attack > 0`

**NÓ 23b: Branch**
- Se TRUE: mostrar bônus

**NÓ 23c: Format Text**
- Pattern: `+{0}` (bônus sempre com +)
- **{0}**: Physical Attack (bônus)

**NÓ 23d: Set Text (PhysicalAttackBonusText)**
- TextBlock separado para bônus
- **In Text**: Return Value

**NÓ 23e: Set Color and Opacity**
- Target: PhysicalAttackBonusText
- Color: Verde (#00FF00 ou R:0, G:1, B:0, A:1)

**NÓ 23f: Set Visibility (Visible)**
- Tornar visível

**Branch FALSE:**
**NÓ 23g: Set Visibility (Collapsed)**
- Esconder se bônus == 0

**Repetir para todos os stats com bônus.**

---

#### **SEÇÃO I: Return**

**NÓ 24: Return Node**

1. No final do fluxo de execução
2. Buscar: `Return Node`
3. Conectar ao último nó de Set Text

---

## 🔌 PARTE 4: Integrar com WBP_InventorySlot

### PASSO 4.1: Abrir WBP_InventorySlot

1. No Content Browser, buscar: `WBP_InventorySlot`
2. Abrir o Blueprint
3. Ir para **Event Graph**

### PASSO 4.2: Localizar Chamada do Tooltip

Procurar por eventos:
- **Event Construct**
- **On Mouse Enter** (hover)
- **On Mouse Leave**
- **UpdateSlotVisual** (função customizada)

Encontrar onde o tooltip é criado/atualizado.

### PASSO 4.3: Substituir Chamada

**Antes:**
```
[Event On Mouse Enter]
    → [Create Widget (WBP_ItemTooltip)]
    → [SetTooltipData]
    → [Add to Viewport]
```

**Depois:**
```
[Event On Mouse Enter]
    → [Create Widget (WBP_ItemTooltip)]
    → [SetRefinedItemTooltipData]  ← NOVA função
    → [Add to Viewport]
```

**Nós detalhados:**

**NÓ A: Event On Mouse Enter**
- Já existe no slot

**NÓ B: Create Widget**
- Class: WBP_ItemTooltip
- Owning Player: Get Player Controller

**NÓ C: Chamar SetRefinedItemTooltipData**
1. Arrastar Return Value do Create Widget
2. Buscar: `SetRefinedItemTooltipData`
3. Conectar:
   - **Target**: Return Value (WBP_ItemTooltip)
   - **In Slot Data**: `SlotData` (variável do slot)

**NÓ D: Add to Viewport**
- Target: Return Value do Create Widget
- Z-Order: 999 (alto para ficar acima de tudo)

**NÓ E: Set Positioning (opcional)**
- Posicionar tooltip perto do cursor ou slot

---

## 🎨 PARTE 5: Adicionar Componentes UI (Se Necessário)

Se o tooltip atual NÃO tem alguns TextBlocks necessários:

### PASSO 5.1: Adicionar RefinementLevelText

1. Abrir **WBP_ItemTooltip**
2. Ir para aba **Designer**
3. Adicionar **Text Block** ao layout
4. Renomear: `RefinementLevelText`
5. Configurar:
   - **Is Variable**: TRUE
   - **Visibility**: Collapsed (padrão)
   - **Color**: Amarelo/Dourado (#FFD700)
   - **Font Size**: 16
   - **Text**: "Nível: +12" (placeholder)

### PASSO 5.2: Adicionar TextBlocks de Bônus (Opcional)

Para cada stat que quer mostrar bônus em verde:

1. Duplicar TextBlock existente (ex: `PhysicalAttackText`)
2. Renomear com sufixo "Bonus" (ex: `PhysicalAttackBonusText`)
3. Posicionar ao lado do stat original
4. Configurar:
   - **Is Variable**: TRUE
   - **Visibility**: Collapsed (padrão)
   - **Color**: Verde (#00FF00)
   - **Font Size**: 14
   - **Text**: "+55" (placeholder)

---

## 🧪 PARTE 6: Testar a Implementação

### PASSO 6.1: Compilar Blueprints

1. No WBP_ItemTooltip: **Compile** e **Save**
2. No WBP_InventorySlot: **Compile** e **Save**
3. Verificar se há erros no painel **Compiler Results**

### PASSO 6.2: Testar Item SEM Refinação

1. Play in Editor
2. Abrir inventário
3. Passar mouse sobre item **não refinado** (ex: Poção, material)
4. **Verificar:**
   - ✅ Tooltip aparece normalmente
   - ✅ Stats base estão corretos
   - ✅ Sem indicador de refinação
   - ✅ Nome sem sufixo (+X)

### PASSO 6.3: Testar Item COM Refinação

1. Passar mouse sobre **Espada de Ferro +12**
2. **Verificar:**
   - ✅ Nome mostra "Espada de Ferro +12"
   - ✅ Indicador "Nível: +12" visível (amarelo)
   - ✅ Stats são maiores (ex: 155 Atk ao invés de 100)
   - ✅ Bônus em verde (se implementou): "+55" próximo ao stat
   - ✅ Todos os stats atualizados corretamente

### PASSO 6.4: Testar Diferentes Níveis

1. Testar item +1, +6, +12
2. Verificar que stats aumentam proporcionalmente
3. Verificar que nome sempre tem sufixo correto

### PASSO 6.5: Verificar Logs

Abrir **Output Log** (Window → Developer Tools → Output Log):

```
LogTemp: Display: UmbraRefinementSubsystem: Loaded 13 refinement configs
LogTemp: Display: WBP_ItemTooltip: SetRefinedItemTooltipData called
LogTemp: Display: WBP_ItemTooltip: Item is refined (Level: 12)
LogTemp: Display: WBP_ItemTooltip: Total PhysicalAttack: 155 (Base: 100, Bonus: 55)
```

Se não houver logs, adicionar nós **Print String** no Blueprint para debug.

---

## 🐛 PARTE 7: Troubleshooting

### Problema 1: Tooltip não aparece

**Possíveis causas:**
- Create Widget retornando NULL
- Add to Viewport não executando
- Z-Order muito baixo (fica atrás de outros widgets)

**Solução:**
1. Adicionar nó **IsValid** após Create Widget
2. Adicionar **Print String** para debug
3. Verificar se Class está correta no Create Widget
4. Aumentar Z-Order do Add to Viewport

---

### Problema 2: Nome não mostra sufixo de refinação

**Causa:** `Get Item Display Name` não está sendo chamado

**Solução:**
1. Verificar se nó 14 está conectado
2. Verificar se Return Value está conectado ao Set Text
3. Adicionar **Print String** para debug:
   ```
   Get Item Display Name → Print String
   ```

---

### Problema 3: Stats ainda mostram valores base

**Causa:** `Calculate Total Stats` não está sendo usado

**Solução:**
1. Verificar se nó 12 existe e está conectado
2. Verificar se `InSlotData` está sendo passado corretamente
3. Adicionar debug:
   ```
   Calculate Total Stats → Break UmbraItemStats → Combat → Break Combat
   → Physical Attack → Print String ("Total Atk: {0}")
   ```

---

### Problema 4: RefinementSubsystem é NULL

**Causa:** Configuração não foi carregada

**Solução no C++ (UmbraGameInstance):**

Adicionar no `OnLoginSuccess()`:
```cpp
UUmbraRefinementSubsystem* RefinementSubsystem = GetSubsystem<UUmbraRefinementSubsystem>();
if (RefinementSubsystem)
{
    RefinementSubsystem->LoadRefinementConfig();
}
```

**Solução no Blueprint:**
1. Adicionar verificação NULL após Get Subsystem (nós 9-11)
2. Se NULL, usar fallback para SetTooltipData original

---

### Problema 5: Stats aparecem como 0

**Causa:** Problema de serialização de structs aninhados

**Solução:**
1. Verificar se `Break UmbraItemStats` está usando o resultado de `Calculate Total Stats`
2. NÃO usar `Break UmbraInventorySlot → ItemTemplate → Stats` diretamente
3. Sempre usar `Calculate Total Stats` para stats refinados

---

### Problema 6: Branch sempre vai para FALSE

**Causa:** `RefinementLevel` não está sendo lido corretamente do Break

**Solução:**
1. Verificar se o Break UmbraInventorySlot está antes da comparação
2. Adicionar **Print String** para debug:
   ```
   Refinement Level → Print String ("Refinement Level: {0}")
   ```
3. Verificar se item realmente tem refinação no banco de dados

---

### Problema 7: Tooltip não atualiza após refinação

**Causa:** Inventário não recarrega após refinação

**Solução:**
1. No `UmbraRefinementWidget`, após refinação bem-sucedida:
   ```cpp
   // C++
   UUmbraGameInstance* GI = Cast<UUmbraGameInstance>(GetGameInstance());
   if (GI) {
       GI->LoadInventory();
   }
   ```

2. Ou no Blueprint, vincular ao delegate `OnRefinementComplete`:
   ```
   OnRefinementComplete (Event)
       → Get Game Instance
       → Cast to UmbraGameInstance
       → Load Inventory
   ```

---

## 📊 PARTE 8: Estrutura Visual do Graph

### Layout Recomendado

```
┌─────────────────────────────────────────────────────────────┐
│  Entry (InSlotData)                                         │
│      ↓                                                       │
│  Break UmbraInventorySlot                                   │
│      ↓                                                       │
│  RefinementLevel > 0?                                       │
│      ↓                                                       │
│  Branch ──────────┬─ FALSE → SetTooltipData → Return       │
│                   │                                         │
│                   └─ TRUE → Get Game Instance              │
│                              ↓                               │
│                          Cast to UmbraGameInstance          │
│                              ↓                               │
│                          Get Subsystem (Refinement)         │
│                              ↓                               │
│                          IsValid?                           │
│                              ↓                               │
│                          Branch ─ FALSE → Return            │
│                              ↓ TRUE                         │
│                          Calculate Total Stats              │
│                              ↓                               │
│                          Break UmbraItemStats               │
│                         ┌────┼────┐                         │
│                      Base  Combat Bonus                     │
│                         │     │     │                       │
│                      Break  Break Break                     │
│                         │     │     │                       │
│                      ┌──┴─────┴─────┴──┐                   │
│                      │  Set Text Nodes  │                   │
│                      │  (todos stats)   │                   │
│                      └──────────────────┘                   │
│                              ↓                               │
│                          Return                             │
└─────────────────────────────────────────────────────────────┘
```

### Agrupamento de Nós (para organização)

1. Criar **Comment Boxes** para agrupar:
   - "Validação de Refinação"
   - "Obter Subsystem"
   - "Calcular Stats Totais"
   - "Atualizar Nome"
   - "Popular Stats de Combate"
   - "Popular Atributos Base"
   - "Popular Bônus"
   - "Exibir Bônus de Refinação (Verde)"

2. Para criar Comment Box:
   - Selecionar múltiplos nós (Ctrl+Click)
   - Clicar com botão direito → "Create Comment from Selection"
   - Renomear e colorir para facilitar navegação

---

## 📚 PARTE 9: Referências Rápidas

### Funções C++ Disponíveis no Blueprint

#### **UmbraRefinementLibrary** (Helper Functions)

```cpp
// Obter nome com sufixo de refinação
Get Item Display Name (ItemSlot) → String
// Retorna: "Espada de Ferro +12" ou "Poção" (se não refinado)

// Verificar se item é refinável
Is Item Refinable (ItemSlot) → Boolean

// Verificar se item é negociável
Is Item Tradeable (ItemSlot) → Boolean

// Obter cor para taxa de sucesso
Get Success Rate Color (SuccessRate) → LinearColor
// Verde: >=70%, Laranja: 30-70%, Vermelho: <30%

// Formatar taxa como porcentagem
Format Success Rate (Rate) → String
// Retorna: "75.0%"
```

#### **UmbraRefinementSubsystem** (Cálculos)

```cpp
// Calcular stats totais (base + bônus)
Calculate Total Stats (ItemSlot) → FUmbraItemStats

// Calcular apenas bônus de refinação
Calculate Bonus Stats (BaseStats, RefinementLevel) → FUmbraItemStats

// Verificar se item pode ser refinado
Can Refine Item (ItemSlot, OutErrorMessage) → Boolean

// Obter configuração para um nível
Get Config For Level (RefinementLevel) → FUmbraRefinementConfig

// Obter material necessário
Get Required Material (CurrentLevel, OutItemID, OutQuantity) → Boolean
```

### Structs Importantes

#### **FUmbraItemStats**
```cpp
struct FUmbraItemStats {
    FUmbraItemStatsBase Base;        // Strength, Dexterity, Intelligence, Vitality, Luck
    FUmbraItemStatsCombat Combat;    // PhysicalAttack, MagicAttack, Critical, Dodge, etc
    FUmbraItemStatsBonus Bonus;      // HealthBonus, ManaBonus, Movement
};
```

#### **FUmbraInventorySlot**
```cpp
struct FUmbraInventorySlot {
    int32 InventoryID;
    int32 RefinementLevel;           // 0 a 12
    FUmbraItemStats RefinementBonusStats; // Stats de bônus calculados
    FUmbraItemTemplate ItemTemplate; // Template base do item
    // ... outros campos
};
```

### Fórmulas de Cálculo

#### **Multiplicadores por Nível**
- +0: 1.00 (sem bônus)
- +1: 1.05 (+5%)
- +2: 1.10 (+10%)
- +3: 1.15 (+15%)
- ...
- +12: 1.55 (+55%)

#### **Cálculo de Bônus**
```
BonusStats = (BaseStats × Multiplier) - BaseStats

Exemplo: Espada +12 (100 Atk base)
Bônus = (100 × 1.55) - 100 = 55 Atk
Total = 100 + 55 = 155 Atk
```

---

## ✅ Checklist Final

### Implementação
- [ ] Função `SetRefinedItemTooltipData` criada
- [ ] Input `InSlotData` configurado
- [ ] Break UmbraInventorySlot implementado
- [ ] Comparação RefinementLevel > 0 funcionando
- [ ] Branch de decisão correto
- [ ] Ramo FALSE chama SetTooltipData existente
- [ ] Ramo TRUE obtém RefinementSubsystem
- [ ] Validação de subsystem implementada
- [ ] Calculate Total Stats chamado
- [ ] Break UmbraItemStats funcionando
- [ ] Get Item Display Name atualiza nome
- [ ] Indicador de refinação (Nível: +X) visível
- [ ] Todos os stats de combate atualizados
- [ ] Todos os atributos base atualizados
- [ ] Todos os bônus atualizados
- [ ] (Opcional) Bônus em verde implementado
- [ ] Return node no final

### Integração
- [ ] WBP_InventorySlot atualizado
- [ ] Chamada alterada de SetTooltipData para SetRefinedItemTooltipData
- [ ] Tooltip aparece no hover

### UI
- [ ] RefinementLevelText adicionado ao Designer
- [ ] RefinementLevelText configurado como variável
- [ ] (Opcional) TextBlocks de bônus adicionados
- [ ] Cores configuradas (amarelo para nível, verde para bônus)

### Testes
- [ ] Item sem refinação funciona (usa função existente)
- [ ] Item refinado mostra nome com sufixo
- [ ] Item refinado mostra indicador de nível
- [ ] Stats totais são exibidos corretamente
- [ ] Diferentes níveis (+1, +6, +12) testados
- [ ] Tooltip atualiza após refinação
- [ ] Sem erros no Output Log

---

## 🎓 Dicas e Boas Práticas

### 1. Organização do Graph
- Usar Comment Boxes para agrupar nós relacionados
- Alinhar nós horizontalmente para melhor leitura
- Usar cores nos Comment Boxes (azul para lógica, verde para UI)

### 2. Nomenclatura
- Prefixo `In` para inputs (InSlotData)
- Prefixo `Out` para outputs (OutErrorMessage)
- Sufixo `Text` para TextBlocks (ItemNameText)

### 3. Debug
- Adicionar Print String em pontos críticos
- Usar cores no Print String:
  - Azul: informações
  - Amarelo: avisos
  - Vermelho: erros

### 4. Performance
- `Calculate Total Stats` é chamado UMA vez
- Reutilizar o resultado do Break em múltiplos nós
- Evitar cálculos duplicados

### 5. Manutenção
- Documentar a função com comentário:
  ```
  /**
   * Atualiza tooltip com stats de refinação
   * Se item não refinado, usa SetTooltipData existente
   * Se item refinado, calcula e mostra stats totais
   */
  ```

---

## 📞 Suporte

### Se encontrar problemas:

1. **Verificar logs:**
   - Output Log do Unreal
   - Print Strings adicionados
   - Compiler Results

2. **Verificar dados:**
   - MySQL: `SELECT * FROM player_inventory WHERE refinement_level > 0`
   - Verificar se `refinement_bonus_stats` tem valores

3. **Verificar configuração:**
   - RefinementSubsystem carregou configs?
   - API de refinação funciona no test_refinement.html?

4. **Rollback seguro:**
   - A função existente não foi modificada
   - Basta voltar a chamar `SetTooltipData` no WBP_InventorySlot

---

**Status:** Guia completo para implementação de tooltip de itens refinados sem modificar código existente.

**Tempo estimado:** 2-3 horas (incluindo testes)

**Versão:** 1.0  
**Data:** 28/04/2026  
**UE Version:** 5.6.1
