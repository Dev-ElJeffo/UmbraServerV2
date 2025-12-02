# Análise Completa do Blueprint - SetTooltipData

## Função: SetTooltipData

### Visão Geral
A função `SetTooltipData` é responsável por receber um `FUmbraInventorySlot` e configurar todos os elementos visuais do tooltip do item, incluindo nome, descrição, raridade, tipo, stats, durabilidade, quantidade, etc.

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `InSlotData` (tipo: `FUmbraInventorySlot`, por referência)

### Fluxo de Execução Principal

1. **Break Struct - UmbraInventorySlot** (`K2Node_BreakStruct_0`)
   - Extrai todos os campos do `FUmbraInventorySlot`:
     - `InventoryID`, `PlayerID`, `ItemTemplateID`, `Quantity`, `SlotIndex`
     - `StorageID`, `bIsEquipped`, `Durability`
     - `ItemTemplate` (tipo: `FUmbraItemTemplate`)
     - `AcquiredAt`

2. **Break Struct - UmbraItemTemplate** (`K2Node_BreakStruct_1`)
   - Extrai todos os campos do `FUmbraItemTemplate`:
     - `ItemID`, `ItemName`, `ItemDescription`, `ItemType`, `ItemSubtype`
     - `IconPath`, `ItemIcon`, `MaxStackSize`, `EquipmentSlot`, `RequiredLevel`
     - **`Stats`** (tipo: `FUmbraItemStats`) ← **CRÍTICO PARA O PROBLEMA**
     - `Rarity`, `Value`, `Weight`

3. **Chamada para UpdateStats** (`K2Node_CallFunction_4`)
   - **Função chamada**: `UpdateStats`
   - **Parâmetro passado**: `Stats` (do `K2Node_BreakStruct_1`)
   - **Tipo do parâmetro**: `FUmbraItemStats` (struct)
   - **Conexão**: O pin `Stats` do `K2Node_BreakStruct_1` está conectado ao pin `Stats` do `K2Node_CallFunction_4`
   - **Execução**: Esta chamada acontece **IMEDIATAMENTE** após a entrada da função, antes de qualquer outra configuração de UI

---

## Detalhamento dos Nós e Conexões

### 1. Nó de Entrada da Função (`K2Node_FunctionEntry_0`)
- **Posição**: X=-3632, Y=16
- **Pin de Execução**: Conectado a `K2Node_CallFunction_4` (UpdateStats)
- **Pin de Dados**: `InSlotData` conectado a `K2Node_BreakStruct_0`

### 2. Break Struct - UmbraInventorySlot (`K2Node_BreakStruct_0`)
- **Posição**: X=-3472, Y=144
- **Campos extraídos e utilizados**:
  - `Quantity` → Conectado a `K2Node_FormatText_12` (formatação de quantidade)
  - `Durability` → Conectado a `K2Node_FormatText_8` (formatação de durabilidade)
  - `ItemTemplate` → Conectado a `K2Node_BreakStruct_1` (extração do template)

### 3. Break Struct - UmbraItemTemplate (`K2Node_BreakStruct_1`)
- **Posição**: X=-3072, Y=352
- **Campos extraídos e utilizados**:
  - `ItemName` → Conectado a `K2Node_CallFunction_2` (conversão String para Text)
  - `ItemDescription` → Conectado a `K2Node_CallFunction_3` (conversão String para Text)
  - `ItemType` → Conectado a `K2Node_CallFunction_5` (GetTypeText)
  - `ItemSubtype` → Conectado a `K2Node_FormatText_2` (formatação de subtipo)
  - `EquipmentSlot` → Conectado a `K2Node_CallFunction_7` (GetEquipmentSlot)
  - `RequiredLevel` → Conectado a `K2Node_FormatText_14` (formatação de nível requerido)
  - `MaxStackSize` → Conectado a `K2Node_FormatText_9` (formatação de stack máximo)
  - `Rarity` → Conectado a múltiplos nós (GetRarityColor, GetEnumeratorNameAsString, etc.)
  - `Value` → Conectado a `K2Node_FormatText_4` (formatação de valor)
  - **`Stats`** → **Conectado a `K2Node_CallFunction_4` (UpdateStats)** ← **PONTO CRÍTICO**

### 4. Chamada para UpdateStats (`K2Node_CallFunction_4`)
- **Posição**: X=-2800, Y=-16
- **Função**: `UpdateStats` (função do próprio widget)
- **Parâmetro `Stats`**: Recebe diretamente o pin `Stats` do `K2Node_BreakStruct_1`
- **Tipo do parâmetro**: `FUmbraItemStats` (struct)
- **Fluxo de execução**:
  - **Entrada (execute)**: Conectado ao pin `then` do `K2Node_FunctionEntry_0`
  - **Saída (then)**: Conectado a `K2Node_VariableSet_13` (configuração de durabilidade)

---

## Observações Críticas

### 1. Passagem de Stats para UpdateStats
- O struct `FUmbraItemStats` é passado **diretamente** do `Break Struct` do `ItemTemplate` para a função `UpdateStats`
- **NÃO há nenhuma transformação, validação ou filtro** entre a extração do struct e a chamada da função
- **NÃO há nenhuma lógica condicional** que possa filtrar ou modificar os stats baseado em `ItemType`, `EquipmentSlot`, ou qualquer outro campo

### 2. Ordem de Execução
- `UpdateStats` é chamada **ANTES** de qualquer outra configuração de UI
- A sequência é:
  1. Break `InSlotData` → `UmbraInventorySlot`
  2. Break `ItemTemplate` → `UmbraItemTemplate`
  3. **Chamada `UpdateStats(Stats)`** ← **PRIMEIRA AÇÃO**
  4. Configuração de durabilidade
  5. Configuração de quantidade
  6. Configuração de subtipo
  7. Configuração de slot de equipamento
  8. Configuração de tipo de item
  9. Configuração de nome do item
  10. Configuração de descrição
  11. Configuração de raridade e cor
  12. Configuração de valor
  13. Configuração de nível requerido
  14. Configuração de stack máximo

### 3. Conexão Direta do Struct
- O pin `Stats` do `K2Node_BreakStruct_1` está **diretamente conectado** ao pin `Stats` do `K2Node_CallFunction_4`
- **NÃO há nós intermediários** (Knots, conversões, validações, etc.)
- A conexão é: `K2Node_BreakStruct_1.Stats` → `K2Node_CallFunction_4.Stats`

---

## Conclusão da Análise - SetTooltipData

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é extraído corretamente do `ItemTemplate`
2. ✅ O struct é passado **diretamente** para `UpdateStats` sem modificações
3. ✅ **NÃO há lógica condicional** que filtre stats baseado em tipo de item
4. ✅ **NÃO há nós desconectados** ou lógica que possa causar perda de dados
5. ✅ A ordem de execução garante que `UpdateStats` recebe os dados antes de qualquer outra configuração

### Próximos Passos:
- Aguardar análise das funções `UpdateStats`, `UpdateBaseStats`, `UpdateAtkAndDefStats`, `UpdateCombatStats`, e `UpdateBonusStats` para identificar onde os stats podem estar sendo perdidos ou não exibidos.

---

**Status**: ✅ Primeira parte recebida e analisada - Função `SetTooltipData`

---

# Função: UpdateStats

## Visão Geral
A função `UpdateStats` é responsável por receber o struct `FUmbraItemStats` e distribuir a atualização dos stats para 4 funções especializadas, cada uma responsável por um grupo específico de stats.

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `Stats` (tipo: `FUmbraItemStats`, por valor)

### Fluxo de Execução Principal

A função `UpdateStats` chama **4 funções em sequência**, todas recebendo o **mesmo struct `Stats`**:

1. **UpdateCombatStats** (`K2Node_CallFunction_31`)
   - **Primeira função chamada**
   - Recebe o struct `Stats` diretamente do parâmetro de entrada
   - **Responsável por**: Stats de combate (Critical, DoubleAttackRate, etc.)

2. **UpdateBaseStats** (`K2Node_CallFunction_25`)
   - **Segunda função chamada**
   - Recebe o struct `Stats` através de Knots (roteamento)
   - **Responsável por**: Stats base (Strength, Dexterity, Intelligence, Vitality, Luck)

3. **UpdatATKandDEFStats** (`K2Node_CallFunction_30`)
   - **Terceira função chamada**
   - Recebe o struct `Stats` através de Knots (roteamento)
   - **Responsável por**: Stats de ataque e defesa (PhysicalAttack, MagicAttack, PhysicalDefense, MagicDefense, Accuracy, Dodge)

4. **UpdateBonusStats** (`K2Node_CallFunction_19`)
   - **Quarta função chamada**
   - Recebe o struct `Stats` através de Knots (roteamento)
   - **Responsável por**: Stats de bônus (HealthBonus, ManaBonus, Movement)

---

## Detalhamento dos Nós e Conexões

### 1. Nó de Entrada da Função (`K2Node_FunctionEntry_2`)
- **Posição**: X=-1984, Y=-272
- **Parâmetro `Stats`**: Tipo `FUmbraItemStats` (por valor)
- **Conexões do pin `Stats`**:
  - Conectado a `K2Node_CallFunction_31` (UpdateCombatStats) - **DIRETAMENTE**
  - Conectado a `K2Node_Knot_38` (roteamento para outras funções)
- **Conexão de execução**: Conectado a `K2Node_CallFunction_31` (UpdateCombatStats)

### 2. Chamada para UpdateCombatStats (`K2Node_CallFunction_31`)
- **Posição**: X=-1584, Y=-288
- **Função**: `UpdateCombatStats`
- **Parâmetro `UmbraItemStats`**: Recebe **DIRETAMENTE** do pin `Stats` do `K2Node_FunctionEntry_2`
- **Tipo do parâmetro**: `FUmbraItemStats` (por referência, const)
- **Fluxo de execução**:
  - **Entrada (execute)**: Conectado ao pin `then` do `K2Node_FunctionEntry_2`
  - **Saída (then)**: Conectado a `K2Node_CallFunction_25` (UpdateBaseStats)

### 3. Knot de Roteamento (`K2Node_Knot_38`)
- **Posição**: X=-1744, Y=-352
- **Função**: Roteamento visual do struct `Stats`
- **Conexões**:
  - **Input**: Recebe do pin `Stats` do `K2Node_FunctionEntry_2`
  - **Output**: Conectado a `K2Node_Knot_53`

### 4. Knot de Roteamento (`K2Node_Knot_53`)
- **Posição**: X=-1360, Y=-352
- **Função**: Roteamento visual do struct `Stats`
- **Conexões**:
  - **Input**: Recebe de `K2Node_Knot_38`
  - **Output**: Conectado a:
    - `K2Node_Knot_52` (para roteamento adicional)
    - `K2Node_CallFunction_25` (UpdateBaseStats) - **DIRETAMENTE**

### 5. Chamada para UpdateBaseStats (`K2Node_CallFunction_25`)
- **Posição**: X=-1248, Y=-288
- **Função**: `UpdateBaseStats`
- **Parâmetro `UmbraItemStats`**: Recebe de `K2Node_Knot_53`
- **Tipo do parâmetro**: `FUmbraItemStats` (por referência, const)
- **Fluxo de execução**:
  - **Entrada (execute)**: Conectado ao pin `then` do `K2Node_CallFunction_31` (UpdateCombatStats)
  - **Saída (then)**: Conectado a `K2Node_CallFunction_30` (UpdatATKandDEFStats)

### 6. Knot de Roteamento (`K2Node_Knot_52`)
- **Posição**: X=-1008, Y=-352
- **Função**: Roteamento visual do struct `Stats`
- **Conexões**:
  - **Input**: Recebe de `K2Node_Knot_53`
  - **Output**: Conectado a:
    - `K2Node_Knot_37` (para roteamento adicional)
    - `K2Node_CallFunction_30` (UpdatATKandDEFStats) - **DIRETAMENTE**

### 7. Chamada para UpdatATKandDEFStats (`K2Node_CallFunction_30`)
- **Posição**: X=-928, Y=-288
- **Função**: `UpdatATKandDEFStats`
- **Parâmetro `UmbraItemStats`**: Recebe de `K2Node_Knot_52`
- **Tipo do parâmetro**: `FUmbraItemStats` (por referência, const)
- **Fluxo de execução**:
  - **Entrada (execute)**: Conectado ao pin `then` do `K2Node_CallFunction_25` (UpdateBaseStats)
  - **Saída (then)**: Conectado a `K2Node_CallFunction_19` (UpdateBonusStats)

### 8. Knot de Roteamento (`K2Node_Knot_37`)
- **Posição**: X=-752, Y=-352
- **Função**: Roteamento visual do struct `Stats`
- **Conexões**:
  - **Input**: Recebe de `K2Node_Knot_52`
  - **Output**: Conectado a `K2Node_CallFunction_19` (UpdateBonusStats) - **DIRETAMENTE**

### 9. Chamada para UpdateBonusStats (`K2Node_CallFunction_19`)
- **Posição**: X=-592, Y=-288
- **Função**: `UpdateBonusStats`
- **Parâmetro `UmbraItemStats`**: Recebe de `K2Node_Knot_37`
- **Tipo do parâmetro**: `FUmbraItemStats` (por referência, const)
- **Fluxo de execução**:
  - **Entrada (execute)**: Conectado ao pin `then` do `K2Node_CallFunction_30` (UpdatATKandDEFStats)
  - **Saída (then)**: **FIM DA FUNÇÃO** (não conectado a nada)

---

## Ordem de Execução

A sequência de execução é **LINEAR e SEQUENCIAL**:

1. **UpdateCombatStats(Stats)** ← Recebe diretamente do parâmetro
2. **UpdateBaseStats(Stats)** ← Recebe através de Knots
3. **UpdatATKandDEFStats(Stats)** ← Recebe através de Knots
4. **UpdateBonusStats(Stats)** ← Recebe através de Knots

**Todas as 4 funções recebem o MESMO struct `Stats`**, apenas através de diferentes caminhos de roteamento (Knots).

---

## Observações Críticas

### 1. Passagem do Struct Stats
- O struct `FUmbraItemStats` é passado **por valor** como parâmetro de entrada
- **Todas as 4 funções recebem o MESMO struct**, sem modificações
- Os Knots são **apenas para organização visual** - eles **NÃO modificam os dados**
- O struct é passado **por referência (const)** para as funções filhas, garantindo que não seja copiado desnecessariamente

### 2. Conexão Direta para UpdateCombatStats
- `UpdateCombatStats` recebe o struct **DIRETAMENTE** do parâmetro de entrada
- **NÃO passa por Knots** antes de chegar a `UpdateCombatStats`
- Isso pode ser relevante se houver algum problema específico com `UpdateCombatStats`

### 3. Roteamento através de Knots
- As outras 3 funções (`UpdateBaseStats`, `UpdatATKandDEFStats`, `UpdateBonusStats`) recebem o struct através de uma cadeia de Knots:
  - `K2Node_FunctionEntry_2.Stats` → `K2Node_Knot_38` → `K2Node_Knot_53` → `K2Node_Knot_52` → `K2Node_Knot_37`
- **Os Knots são transparentes** - eles apenas roteiam o sinal, não modificam os dados

### 4. Nenhuma Lógica Condicional
- **NÃO há nenhum nó condicional** (Branch, Switch, etc.) que possa filtrar ou modificar os stats
- **NÃO há validações** que possam impedir a passagem do struct
- O fluxo é **100% linear e determinístico**

---

## Conclusão da Análise - UpdateStats

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é recebido corretamente como parâmetro
2. ✅ O struct é passado **sem modificações** para todas as 4 funções filhas
3. ✅ **NÃO há lógica condicional** que possa filtrar stats
4. ✅ **NÃO há nós desconectados** ou lógica que possa causar perda de dados
5. ✅ A ordem de execução é linear e sequencial
6. ✅ `UpdateCombatStats` recebe o struct diretamente (sem Knots)
7. ✅ As outras 3 funções recebem o struct através de Knots (apenas roteamento visual)

### Próximos Passos:
- Aguardar análise das funções `UpdateCombatStats`, `UpdateBaseStats`, `UpdatATKandDEFStats`, e `UpdateBonusStats` para identificar onde os stats específicos (Critical, DoubleAttackRate, etc.) podem estar sendo perdidos ou não exibidos.

---

**Status**: ✅ Segunda parte recebida e analisada - Função `UpdateStats`

---

# Função: UpdateBaseStats

## Visão Geral
A função `UpdateBaseStats` é responsável por atualizar a exibição dos **atributos base** do item no tooltip: `Strength`, `Dexterity`, `Intelligence`, `Vitality`, e `Luck`.

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `UmbraItemStats` (tipo: `FUmbraItemStats`, por referência, const)

### Fluxo de Execução Principal

1. **Break Struct - UmbraItemStats** (`K2Node_BreakStruct_1`)
   - Extrai **TODOS** os campos do struct `FUmbraItemStats`
   - **Campos utilizados nesta função**:
     - `Strength` → Formatação e exibição
     - `Dexterity` → Formatação e exibição
     - `Intelligence` → Formatação e exibição
     - `Vitality` → Formatação e exibição
     - `Luck` → Formatação e exibição
   - **Campos extraídos mas NÃO utilizados nesta função** (são usados em outras funções):
     - `PhysicalAttack`, `MagicAttack`, `PhysicalDefense`, `MagicDefense`
     - `Accuracy`, `Dodge`
     - `Critical`, `CriticalResistance`, `DoubleAttackRate`, `DoubleAttackResistance`
     - `HealthBonus`, `ManaBonus`, `Movement`
     - E todos os campos legados

2. **Para cada atributo base, há uma lógica condicional**:
   - Verifica se o valor é **> 0** usando `Greater_IntInt`
   - Se **> 0**: Formata o texto com o valor e exibe
   - Se **≤ 0**: Formata com string vazia (ou texto vazio) e oculta o widget

---

## Detalhamento dos Nós e Conexões

### 1. Nó de Entrada da Função (`K2Node_FunctionEntry_0`)
- **Posição**: X=-1264, Y=-4912
- **Parâmetro `UmbraItemStats`**: Tipo `FUmbraItemStats` (por referência, const)
- **Conexão**: Conectado a `K2Node_BreakStruct_1`

### 2. Break Struct - UmbraItemStats (`K2Node_BreakStruct_1`)
- **Posição**: X=-1104, Y=-4656
- **Função**: Extrai todos os campos do struct
- **Campos extraídos e utilizados**:
  - `Strength` → Conectado a `K2Node_PromotableOperator_5` (comparação > 0) e `K2Node_FormatText_4` (formatação)
  - `Dexterity` → Conectado a `K2Node_Knot_36` → `K2Node_PromotableOperator_2` (comparação > 0) e `K2Node_FormatText_9` (formatação)
  - `Intelligence` → Conectado a `K2Node_Knot_34` → `K2Node_PromotableOperator_3` (comparação > 0) e `K2Node_FormatText_10` (formatação)
  - `Vitality` → Conectado a `K2Node_Knot_0` → `K2Node_PromotableOperator_4` (comparação > 0) e `K2Node_FormatText_11` (formatação)
  - `Luck` → Conectado a `K2Node_Knot_1` → `K2Node_PromotableOperator_0` (comparação > 0) e `K2Node_FormatText_12` (formatação)

### 3. Lógica para Strength

#### 3.1. Comparação (`K2Node_PromotableOperator_5`)
- **Operação**: `Greater_IntInt` (Strength > 0)
- **Entrada A**: `Strength` (do Break Struct)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_5`

#### 3.2. Condicional (`K2Node_IfThenElse_5`)
- **Condição**: `Strength > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_4`
  - Formata: `"Strength: {0}"` com o valor de `Strength`
  - Exibe no `Text_STR`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_5`
  - Formata com `K2Node_FormatText_5` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_STR`
  - Define visibilidade como `Collapsed` ou `Hidden`

#### 3.3. Formatação quando > 0 (`K2Node_FormatText_4`)
- **Formato**: `"Strength: {0}"`
- **Valor {0}**: `Strength` (do Break Struct)
- **Saída**: Conectado a `K2Node_VariableSet_4` (seta o texto do `Text_STR`)

#### 3.4. Formatação quando ≤ 0 (`K2Node_FormatText_5`)
- **Formato**: (não especificado, provavelmente string vazia)
- **Valor {0}**: (não conectado)
- **Saída**: Conectado a `K2Node_VariableSet_5` (seta o texto do `Text_STR`)

### 4. Lógica para Dexterity

#### 4.1. Knot de Roteamento (`K2Node_Knot_36`)
- **Função**: Roteamento visual do valor `Dexterity`
- **Input**: `Dexterity` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_2` (comparação > 0)
  - `K2Node_FormatText_9` (formatação quando > 0)

#### 4.2. Comparação (`K2Node_PromotableOperator_2`)
- **Operação**: `Greater_IntInt` (Dexterity > 0)
- **Entrada A**: `Dexterity` (de `K2Node_Knot_36`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_2`

#### 4.3. Condicional (`K2Node_IfThenElse_2`)
- **Condição**: `Dexterity > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_9`
  - Formata: `"Dexterity: {0}"` com o valor de `Dexterity`
  - Exibe no `Text_DEX`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_2`
  - Formata com `K2Node_FormatText_2` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_DEX`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 5. Lógica para Intelligence

#### 5.1. Knot de Roteamento (`K2Node_Knot_34`)
- **Função**: Roteamento visual do valor `Intelligence`
- **Input**: `Intelligence` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_3` (comparação > 0)
  - `K2Node_FormatText_10` (formatação quando > 0)

#### 5.2. Comparação (`K2Node_PromotableOperator_3`)
- **Operação**: `Greater_IntInt` (Intelligence > 0)
- **Entrada A**: `Intelligence` (de `K2Node_Knot_34`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_3`

#### 5.3. Condicional (`K2Node_IfThenElse_3`)
- **Condição**: `Intelligence > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_10`
  - Formata: `"Intelligence: {0}"` com o valor de `Intelligence`
  - Exibe no `Text_INT`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_3`
  - Formata com `K2Node_FormatText_3` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_INT`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 6. Lógica para Vitality

#### 6.1. Knot de Roteamento (`K2Node_Knot_0`)
- **Função**: Roteamento visual do valor `Vitality`
- **Input**: `Vitality` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_4` (comparação > 0)
  - `K2Node_FormatText_11` (formatação quando > 0)

#### 6.2. Comparação (`K2Node_PromotableOperator_4`)
- **Operação**: `Greater_IntInt` (Vitality > 0)
- **Entrada A**: `Vitality` (de `K2Node_Knot_0`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_4`

#### 6.3. Condicional (`K2Node_IfThenElse_4`)
- **Condição**: `Vitality > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_11`
  - Formata: `"Vitality: {0}"` com o valor de `Vitality`
  - Exibe no `Text_VIT`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_7`
  - Formata com `K2Node_FormatText_7` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_VIT`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 7. Lógica para Luck

#### 7.1. Knot de Roteamento (`K2Node_Knot_1`)
- **Função**: Roteamento visual do valor `Luck`
- **Input**: `Luck` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_0` (comparação > 0)
  - `K2Node_FormatText_12` (formatação quando > 0)

#### 7.2. Comparação (`K2Node_PromotableOperator_0`)
- **Operação**: `Greater_IntInt` (Luck > 0)
- **Entrada A**: `Luck` (de `K2Node_Knot_1`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_0`

#### 7.3. Condicional (`K2Node_IfThenElse_0`)
- **Condição**: `Luck > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_12`
  - Formata: `"Luck: {0}"` com o valor de `Luck`
  - Exibe no `Text_LUCK`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_0`
  - Formata com `K2Node_FormatText_0` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_LUCK`
  - Define visibilidade como `Collapsed` ou `Hidden`

---

## Ordem de Execução

A sequência de execução é **LINEAR e SEQUENCIAL** para cada atributo:

1. **Strength**:
   - Break Struct → Comparação (Strength > 0) → IfThenElse → Formatação → Set Text → Set Visibility

2. **Dexterity**:
   - Break Struct → Knot → Comparação (Dexterity > 0) → IfThenElse → Formatação → Set Text → Set Visibility

3. **Intelligence**:
   - Break Struct → Knot → Comparação (Intelligence > 0) → IfThenElse → Formatação → Set Text → Set Visibility

4. **Vitality**:
   - Break Struct → Knot → Comparação (Vitality > 0) → IfThenElse → Formatação → Set Text → Set Visibility

5. **Luck**:
   - Break Struct → Knot → Comparação (Luck > 0) → IfThenElse → Formatação → Set Text → Set Visibility

**Cada atributo é processado de forma INDEPENDENTE** - não há dependência entre eles.

---

## Observações Críticas

### 1. Lógica Condicional Baseada em Valor > 0
- **Cada atributo base tem uma verificação condicional**: `valor > 0`
- **Se o valor for > 0**: O texto é formatado e exibido, e o widget fica `Visible`
- **Se o valor for ≤ 0**: O texto é formatado como string vazia, e o widget fica `Collapsed` ou `Hidden`
- **Esta lógica é INDEPENDENTE para cada atributo** - um atributo não afeta o outro

### 2. Break Struct Extrai TODOS os Campos
- O `Break Struct` extrai **TODOS** os campos do `FUmbraItemStats`, incluindo:
  - Todos os atributos base (Strength, Dexterity, Intelligence, Vitality, Luck)
  - Todos os stats de combate (PhysicalAttack, MagicAttack, Critical, DoubleAttackRate, etc.)
  - Todos os bônus (HealthBonus, ManaBonus, Movement)
  - Todos os campos legados
- **Apenas os 5 atributos base são utilizados nesta função** - os outros campos são extraídos mas não usados

### 3. Knots Apenas para Roteamento Visual
- Os Knots (`K2Node_Knot_36`, `K2Node_Knot_34`, `K2Node_Knot_0`, `K2Node_Knot_1`) são **apenas para organização visual**
- Eles **NÃO modificam os dados** - apenas roteiam o sinal
- `Strength` não passa por Knots (conexão direta), enquanto os outros passam por Knots

### 4. Formatação de Texto
- **Quando valor > 0**: Formata como `"{Nome do Atributo}: {valor}"` (ex: "Strength: 10")
- **Quando valor ≤ 0**: Formata como string vazia (sem pinos conectados no `Format Text`)

### 5. Visibilidade dos Widgets
- **Quando valor > 0**: `SetVisibility` com `Visible`
- **Quando valor ≤ 0**: `SetVisibility` com `Collapsed` ou `Hidden` (não especificado no código, mas inferido pela lógica)

---

## Conclusão da Análise - UpdateBaseStats

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é recebido corretamente como parâmetro (por referência, const)
2. ✅ O `Break Struct` extrai **TODOS** os campos do struct corretamente
3. ✅ Cada atributo base (Strength, Dexterity, Intelligence, Vitality, Luck) é processado **INDEPENDENTEMENTE**
4. ✅ Há uma lógica condicional que verifica se cada valor é **> 0** antes de exibir
5. ✅ Se o valor for > 0, o texto é formatado e o widget fica visível
6. ✅ Se o valor for ≤ 0, o texto é formatado como string vazia e o widget fica oculto
7. ✅ **NÃO há dependência entre os atributos** - cada um é processado de forma isolada
8. ✅ Os Knots são apenas para organização visual - não modificam os dados

### Pontos de Atenção:
- ⚠️ A lógica condicional (`valor > 0`) pode explicar por que stats com valor 0 não são exibidos
- ⚠️ **MAS** o problema relatado é que stats com valores **> 0** não estão sendo exibidos quando outros stats são 0
- ⚠️ Esta função processa apenas os **atributos base** - não processa `Critical` ou `DoubleAttackRate` (esses são processados em `UpdateCombatStats`)

### Próximos Passos:
- Aguardar análise das funções `UpdateCombatStats`, `UpdatATKandDEFStats`, e `UpdateBonusStats` para identificar onde os stats específicos (Critical, DoubleAttackRate, etc.) podem estar sendo perdidos ou não exibidos.

---

**Status**: ✅ Terceira parte recebida e analisada - Função `UpdateBaseStats`

---

# Função: UpdatATKandDEFStats

## Visão Geral
A função `UpdatATKandDEFStats` é responsável por atualizar a exibição dos **stats de ataque e defesa** do item no tooltip: `PhysicalAttack`, `MagicAttack`, `PhysicalDefense`, e `MagicDefense`.

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `UmbraItemStats` (tipo: `FUmbraItemStats`, por referência, const)

### Fluxo de Execução Principal

1. **Break Struct - UmbraItemStats** (`K2Node_BreakStruct_2`)
   - Extrai **TODOS** os campos do struct `FUmbraItemStats`
   - **Campos utilizados nesta função**:
     - `PhysicalAttack` → Formatação e exibição
     - `MagicAttack` → Formatação e exibição
     - `PhysicalDefense` → Formatação e exibição
     - `MagicDefense` → Formatação e exibição
   - **Outros campos são extraídos mas não utilizados nesta função**

---

## Processamento Detalhado por Stat

### 1. Lógica para PhysicalAttack

#### 1.1. Knot de Roteamento (`K2Node_Knot_42`)
- **Função**: Roteamento visual do valor `PhysicalAttack`
- **Input**: `PhysicalAttack` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_29` (comparação > 0)
  - `K2Node_FormatText_6` (formatação quando > 0)

#### 1.2. Comparação (`K2Node_PromotableOperator_29`)
- **Operação**: `Greater_IntInt` (PhysicalAttack > 0)
- **Entrada A**: `PhysicalAttack` (de `K2Node_Knot_42`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_6`

#### 1.3. Condicional (`K2Node_IfThenElse_6`)
- **Condição**: `PhysicalAttack > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_6`
  - Formata: `"Phys. Atk: {0}"` com o valor de `PhysicalAttack`
  - Exibe no `Text_PhysATK`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_13`
  - Formata com `K2Node_FormatText_13` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_PhysATK`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 2. Lógica para MagicAttack

#### 2.1. Knot de Roteamento (`K2Node_Knot_41` → `K2Node_Knot_3`)
- **Função**: Roteamento visual do valor `MagicAttack`
- **Input**: `MagicAttack` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_7` (comparação > 0)
  - `K2Node_FormatText_14` (formatação quando > 0)

#### 2.2. Comparação (`K2Node_PromotableOperator_7`)
- **Operação**: `Greater_IntInt` (MagicAttack > 0)
- **Entrada A**: `MagicAttack` (de `K2Node_Knot_3`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_7`

#### 2.3. Condicional (`K2Node_IfThenElse_7`)
- **Condição**: `MagicAttack > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_14`
  - Formata: `"Mag. Atk: {0}"` com o valor de `MagicAttack`
  - Exibe no `Text_MagATK`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_15`
  - Formata com `K2Node_FormatText_15` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_MagATK`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 3. Lógica para PhysicalDefense

#### 3.1. Knot de Roteamento (`K2Node_Knot_40` → `K2Node_Knot_4`)
- **Função**: Roteamento visual do valor `PhysicalDefense`
- **Input**: `PhysicalDefense` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_8` (comparação > 0)
  - `K2Node_FormatText_16` (formatação quando > 0)

#### 3.2. Comparação (`K2Node_PromotableOperator_8`)
- **Operação**: `Greater_IntInt` (PhysicalDefense > 0)
- **Entrada A**: `PhysicalDefense` (de `K2Node_Knot_4`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_8`

#### 3.3. Condicional (`K2Node_IfThenElse_8`)
- **Condição**: `PhysicalDefense > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_16`
  - Formata: `"Phys Def: {0}"` com o valor de `PhysicalDefense`
  - Exibe no `Text_PhysDEF`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_17`
  - Formata com `K2Node_FormatText_17` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_PhysDEF`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 4. Lógica para MagicDefense

#### 4.1. Knot de Roteamento (`K2Node_Knot_39` → `K2Node_Knot_6`)
- **Função**: Roteamento visual do valor `MagicDefense`
- **Input**: `MagicDefense` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_9` (comparação > 0)
  - `K2Node_FormatText_19` (formatação quando > 0)

#### 4.2. Comparação (`K2Node_PromotableOperator_9`)
- **Operação**: `Greater_IntInt` (MagicDefense > 0)
- **Entrada A**: `MagicDefense` (de `K2Node_Knot_6`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_9`

#### 4.3. Condicional (`K2Node_IfThenElse_9`)
- **Condição**: `MagicDefense > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_18`
  - Formata: `"Mag. Def: {0}"` com o valor de `MagicDefense`
  - Exibe no `Text_MagDEF_1`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_19`
  - Formata com `K2Node_FormatText_18` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_MagDEF_1`
  - Define visibilidade como `Collapsed` ou `Hidden`

---

## Ordem de Execução

A sequência de execução é **LINEAR e SEQUENCIAL** para cada stat:

1. **PhysicalAttack**:
   - Break Struct → Knot → Comparação (PhysicalAttack > 0) → IfThenElse → Formatação → Set Text → Set Visibility

2. **MagicAttack**:
   - Break Struct → Knot → Knot → Comparação (MagicAttack > 0) → IfThenElse → Formatação → Set Text → Set Visibility

3. **PhysicalDefense**:
   - Break Struct → Knot → Knot → Comparação (PhysicalDefense > 0) → IfThenElse → Formatação → Set Text → Set Visibility

4. **MagicDefense**:
   - Break Struct → Knot → Knot → Comparação (MagicDefense > 0) → IfThenElse → Formatação → Set Text → Set Visibility

**Cada stat é processado de forma INDEPENDENTE** - não há dependência entre eles.

---

## Observações Críticas

### 1. Lógica Condicional Baseada em Valor > 0
- **Cada stat de ataque/defesa tem uma verificação condicional**: `valor > 0`
- **Se o valor for > 0**: O texto é formatado e exibido, e o widget fica `Visible`
- **Se o valor for ≤ 0**: O texto é formatado como string vazia, e o widget fica `Collapsed` ou `Hidden`
- **Esta lógica é INDEPENDENTE para cada stat** - um stat não afeta o outro

### 2. Break Struct Extrai TODOS os Campos
- O `Break Struct` extrai **TODOS** os campos do `FUmbraItemStats`, incluindo:
  - Todos os atributos base (Strength, Dexterity, Intelligence, Vitality, Luck)
  - Todos os stats de combate (PhysicalAttack, MagicAttack, Critical, DoubleAttackRate, etc.)
  - Todos os bônus (HealthBonus, ManaBonus, Movement)
  - Todos os campos legados
- **Apenas os 4 stats de ataque/defesa são utilizados nesta função** - os outros campos são extraídos mas não usados

### 3. Knots Apenas para Roteamento Visual
- Os Knots (`K2Node_Knot_42`, `K2Node_Knot_41`, `K2Node_Knot_3`, `K2Node_Knot_40`, `K2Node_Knot_4`, `K2Node_Knot_39`, `K2Node_Knot_6`) são **apenas para organização visual**
- Eles **NÃO modificam os dados** - apenas roteiam o sinal
- Alguns stats passam por múltiplos Knots (ex: `MagicAttack` passa por `K2Node_Knot_41` e `K2Node_Knot_3`)

### 4. Formatação de Texto
- **Quando valor > 0**: Formata como `"{Nome do Stat}: {valor}"` (ex: "Phys. Atk: 175")
- **Quando valor ≤ 0**: Formata como string vazia (sem pinos conectados no `Format Text`)

### 5. Visibilidade dos Widgets
- **Quando valor > 0**: `SetVisibility` com `Visible`
- **Quando valor ≤ 0**: `SetVisibility` com `Collapsed` ou `Hidden` (não especificado no código, mas inferido pela lógica)

---

## Conclusão da Análise - UpdatATKandDEFStats

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é recebido corretamente como parâmetro (por referência, const)
2. ✅ O `Break Struct` extrai **TODOS** os campos do struct corretamente
3. ✅ Cada stat de ataque/defesa (PhysicalAttack, MagicAttack, PhysicalDefense, MagicDefense) é processado **INDEPENDENTEMENTE**
4. ✅ Há uma lógica condicional que verifica se cada valor é **> 0** antes de exibir
5. ✅ Se o valor for > 0, o texto é formatado e o widget fica visível
6. ✅ Se o valor for ≤ 0, o texto é formatado como string vazia e o widget fica oculto
7. ✅ **NÃO há dependência entre os stats** - cada um é processado de forma isolada
8. ✅ Os Knots são apenas para organização visual - não modificam os dados

### Pontos de Atenção:
- ⚠️ A lógica condicional (`valor > 0`) pode explicar por que stats com valor 0 não são exibidos
- ⚠️ **MAS** o problema relatado é que stats com valores **> 0** não estão sendo exibidos quando outros stats são 0
- ⚠️ Esta função processa apenas os **stats de ataque/defesa** - não processa `Critical` ou `DoubleAttackRate` (esses são processados em `UpdateCombatStats`)

### Próximos Passos:
- Aguardar análise das funções `UpdateCombatStats` e `UpdateBonusStats` para identificar onde os stats específicos (Critical, DoubleAttackRate, HealthBonus, ManaBonus, Movement) podem estar sendo perdidos ou não exibidos.

---

**Status**: ✅ Quarta parte recebida e analisada - Função `UpdatATKandDEFStats`

---

# Função: UpdateCombatStats

## Visão Geral
A função `UpdateCombatStats` é responsável por atualizar a exibição dos **stats de combate avançados** do item no tooltip: `Accuracy`, `Dodge`, `Critical`, `DoubleAttackRate`, `CriticalResistance`, e `DoubleAttackResistance`.

**Esta é a função que processa `Critical` e `DoubleAttackRate`, os stats que o usuário relatou não estarem sendo exibidos corretamente.**

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `UmbraItemStats` (tipo: `FUmbraItemStats`, por referência, const)

### Fluxo de Execução Principal

1. **Break Struct - UmbraItemStats** (`K2Node_BreakStruct_3`)
   - Extrai **TODOS** os campos do struct `FUmbraItemStats`
   - **Campos utilizados nesta função**:
     - `Accuracy` → Formatação e exibição
     - `Dodge` → Formatação e exibição
     - `Critical` → Formatação e exibição
     - `DoubleAttackRate` → Formatação e exibição
     - `CriticalResistance` → Formatação e exibição
     - `DoubleAttackResistance` → Formatação e exibição
   - **Outros campos são extraídos mas não utilizados nesta função**

---

## Processamento Detalhado por Stat

### 1. Lógica para Accuracy

#### 1.1. Knot de Roteamento (`K2Node_Knot_24` → `K2Node_Knot_7`)
- **Função**: Roteamento visual do valor `Accuracy`
- **Input**: `Accuracy` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_10` (comparação > 0)
  - `K2Node_FormatText_21` (formatação quando > 0)

#### 1.2. Comparação (`K2Node_PromotableOperator_10`)
- **Operação**: `Greater_IntInt` (Accuracy > 0)
- **Entrada A**: `Accuracy` (de `K2Node_Knot_7`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_10`

#### 1.3. Condicional (`K2Node_IfThenElse_10`)
- **Condição**: `Accuracy > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_20`
  - Formata: `"Accuracy: {0}"` com o valor de `Accuracy`
  - Exibe no `Text_Accuracy`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_21`
  - Formata com `K2Node_FormatText_20` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_Accuracy`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 2. Lógica para Dodge

#### 2.1. Knot de Roteamento (`K2Node_Knot_23` → `K2Node_Knot_8`)
- **Função**: Roteamento visual do valor `Dodge`
- **Input**: `Dodge` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_11` (comparação > 0)
  - `K2Node_FormatText_23` (formatação quando > 0)

#### 2.2. Comparação (`K2Node_PromotableOperator_11`)
- **Operação**: `Greater_IntInt` (Dodge > 0)
- **Entrada A**: `Dodge` (de `K2Node_Knot_8`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_11`

#### 2.3. Condicional (`K2Node_IfThenElse_11`)
- **Condição**: `Dodge > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_22`
  - Formata: `"Dodge: {0}"` com o valor de `Dodge`
  - Exibe no `Text_Dodge`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_23`
  - Formata com `K2Node_FormatText_22` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_Dodge`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 3. Lógica para Critical ⚠️ **STAT PROBLEMÁTICO**

#### 3.1. Knot de Roteamento (`K2Node_Knot_22` → `K2Node_Knot_9`)
- **Função**: Roteamento visual do valor `Critical`
- **Input**: `Critical` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_12` (comparação > 0)
  - `K2Node_FormatText_25` (formatação quando > 0)

#### 3.2. Comparação (`K2Node_PromotableOperator_12`)
- **Operação**: `Greater_IntInt` (Critical > 0)
- **Entrada A**: `Critical` (de `K2Node_Knot_9`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_12`

#### 3.3. Condicional (`K2Node_IfThenElse_12`)
- **Condição**: `Critical > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_24`
  - Formata: `"Crit Atk: {0}"` com o valor de `Critical`
  - Exibe no `Text_Critical`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_25`
  - Formata com `K2Node_FormatText_24` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_Critical`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 4. Lógica para DoubleAttackRate ⚠️ **STAT PROBLEMÁTICO**

#### 4.1. Knot de Roteamento (`K2Node_Knot_20` → `K2Node_Knot_10`)
- **Função**: Roteamento visual do valor `DoubleAttackRate`
- **Input**: `DoubleAttackRate` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_13` (comparação > 0)
  - `K2Node_FormatText_27` (formatação quando > 0)

#### 4.2. Comparação (`K2Node_PromotableOperator_13`)
- **Operação**: `Greater_IntInt` (DoubleAttackRate > 0)
- **Entrada A**: `DoubleAttackRate` (de `K2Node_Knot_10`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_13`

#### 4.3. Condicional (`K2Node_IfThenElse_13`)
- **Condição**: `DoubleAttackRate > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_26`
  - Formata: `"Double Atk: {0}"` com o valor de `DoubleAttackRate`
  - Exibe no `Text_DoubleAttackRate`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_27`
  - Formata com `K2Node_FormatText_26` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_DoubleAttackRate`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 5. Lógica para CriticalResistance

#### 5.1. Knot de Roteamento (`K2Node_Knot_21` → `K2Node_Knot_12`)
- **Função**: Roteamento visual do valor `CriticalResistance`
- **Input**: `CriticalResistance` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_15` (comparação > 0)
  - `K2Node_FormatText_31` (formatação quando > 0)

#### 5.2. Comparação (`K2Node_PromotableOperator_15`)
- **Operação**: `Greater_IntInt` (CriticalResistance > 0)
- **Entrada A**: `CriticalResistance` (de `K2Node_Knot_12`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_15`

#### 5.3. Condicional (`K2Node_IfThenElse_15`)
- **Condição**: `CriticalResistance > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_30`
  - Formata: `"Crit Res: {0}"` com o valor de `CriticalResistance`
  - Exibe no `Text_CRIT_Resistance`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_31`
  - Formata com `K2Node_FormatText_30` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_CRIT_Resistance`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 6. Lógica para DoubleAttackResistance

#### 6.1. Knot de Roteamento (`K2Node_Knot_19` → `K2Node_Knot_15`)
- **Função**: Roteamento visual do valor `DoubleAttackResistance`
- **Input**: `DoubleAttackResistance` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_1` (comparação > 0)
  - `K2Node_FormatText_8` (formatação quando > 0)

#### 6.2. Comparação (`K2Node_PromotableOperator_1`)
- **Operação**: `Greater_IntInt` (DoubleAttackResistance > 0)
- **Entrada A**: `DoubleAttackResistance` (de `K2Node_Knot_15`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_1`

#### 6.3. Condicional (`K2Node_IfThenElse_1`)
- **Condição**: `DoubleAttackResistance > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_8`
  - Formata: `"Double Res: {0}"` com o valor de `DoubleAttackResistance`
  - Exibe no `Text_DoubleATK_Resistance`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_1`
  - Formata com `K2Node_FormatText_1` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_DoubleATK_Resistance`
  - Define visibilidade como `Collapsed` ou `Hidden`

---

## Ordem de Execução

A sequência de execução é **LINEAR e SEQUENCIAL** para cada stat:

1. **Accuracy**:
   - Break Struct → Knot → Knot → Comparação (Accuracy > 0) → IfThenElse → Formatação → Set Text → Set Visibility

2. **Dodge**:
   - Break Struct → Knot → Knot → Comparação (Dodge > 0) → IfThenElse → Formatação → Set Text → Set Visibility

3. **Critical**:
   - Break Struct → Knot → Knot → Comparação (Critical > 0) → IfThenElse → Formatação → Set Text → Set Visibility

4. **DoubleAttackRate**:
   - Break Struct → Knot → Knot → Comparação (DoubleAttackRate > 0) → IfThenElse → Formatação → Set Text → Set Visibility

5. **CriticalResistance**:
   - Break Struct → Knot → Knot → Comparação (CriticalResistance > 0) → IfThenElse → Formatação → Set Text → Set Visibility

6. **DoubleAttackResistance**:
   - Break Struct → Knot → Knot → Comparação (DoubleAttackResistance > 0) → IfThenElse → Formatação → Set Text → Set Visibility

**Cada stat é processado de forma INDEPENDENTE** - não há dependência entre eles.

---

## Observações Críticas

### 1. Lógica Condicional Baseada em Valor > 0
- **Cada stat de combate tem uma verificação condicional**: `valor > 0`
- **Se o valor for > 0**: O texto é formatado e exibido, e o widget fica `Visible`
- **Se o valor for ≤ 0**: O texto é formatado como string vazia, e o widget fica `Collapsed` ou `Hidden`
- **Esta lógica é INDEPENDENTE para cada stat** - um stat não afeta o outro

### 2. Break Struct Extrai TODOS os Campos
- O `Break Struct` extrai **TODOS** os campos do `FUmbraItemStats`, incluindo:
  - Todos os atributos base (Strength, Dexterity, Intelligence, Vitality, Luck)
  - Todos os stats de combate (PhysicalAttack, MagicAttack, Critical, DoubleAttackRate, etc.)
  - Todos os bônus (HealthBonus, ManaBonus, Movement)
  - Todos os campos legados
- **Apenas os 6 stats de combate são utilizados nesta função** - os outros campos são extraídos mas não usados

### 3. Knots Apenas para Roteamento Visual
- Os Knots (`K2Node_Knot_24`, `K2Node_Knot_7`, `K2Node_Knot_23`, `K2Node_Knot_8`, `K2Node_Knot_22`, `K2Node_Knot_9`, `K2Node_Knot_20`, `K2Node_Knot_10`, `K2Node_Knot_21`, `K2Node_Knot_12`, `K2Node_Knot_19`, `K2Node_Knot_15`) são **apenas para organização visual**
- Eles **NÃO modificam os dados** - apenas roteiam o sinal
- Todos os stats passam por múltiplos Knots (ex: `Critical` passa por `K2Node_Knot_22` e `K2Node_Knot_9`)

### 4. Formatação de Texto
- **Quando valor > 0**: Formata como `"{Nome do Stat}: {valor}"` (ex: "Crit Atk: 12", "Double Atk: 6")
- **Quando valor ≤ 0**: Formata como string vazia (sem pinos conectados no `Format Text`)

### 5. Visibilidade dos Widgets
- **Quando valor > 0**: `SetVisibility` com `Visible`
- **Quando valor ≤ 0**: `SetVisibility` com `Collapsed` ou `Hidden` (não especificado no código, mas inferido pela lógica)

---

## Análise Específica dos Stats Problemáticos

### Critical
- **Formato esperado**: `"Crit Atk: {0}"` onde `{0}` é o valor de `Critical`
- **Lógica**: Verifica se `Critical > 0` antes de exibir
- **Widget**: `Text_Critical`
- **Fluxo**: Break Struct → `K2Node_Knot_22` → `K2Node_Knot_9` → Comparação → IfThenElse → Formatação → Set Text → Set Visibility

### DoubleAttackRate
- **Formato esperado**: `"Double Atk: {0}"` onde `{0}` é o valor de `DoubleAttackRate`
- **Lógica**: Verifica se `DoubleAttackRate > 0` antes de exibir
- **Widget**: `Text_DoubleAttackRate`
- **Fluxo**: Break Struct → `K2Node_Knot_20` → `K2Node_Knot_10` → Comparação → IfThenElse → Formatação → Set Text → Set Visibility

**Ambos os stats seguem EXATAMENTE a mesma lógica das outras funções** - não há diferença na implementação.

---

## Conclusão da Análise - UpdateCombatStats

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é recebido corretamente como parâmetro (por referência, const)
2. ✅ O `Break Struct` extrai **TODOS** os campos do struct corretamente
3. ✅ Cada stat de combate (Accuracy, Dodge, Critical, DoubleAttackRate, CriticalResistance, DoubleAttackResistance) é processado **INDEPENDENTEMENTE**
4. ✅ Há uma lógica condicional que verifica se cada valor é **> 0** antes de exibir
5. ✅ Se o valor for > 0, o texto é formatado e o widget fica visível
6. ✅ Se o valor for ≤ 0, o texto é formatado como string vazia e o widget fica oculto
7. ✅ **NÃO há dependência entre os stats** - cada um é processado de forma isolada
8. ✅ Os Knots são apenas para organização visual - não modificam os dados
9. ✅ **`Critical` e `DoubleAttackRate` seguem EXATAMENTE a mesma lógica dos outros stats** - não há diferença na implementação

### Pontos de Atenção:
- ⚠️ A lógica condicional (`valor > 0`) pode explicar por que stats com valor 0 não são exibidos
- ⚠️ **MAS** o problema relatado é que stats com valores **> 0** (Critical=12, DoubleAttackRate=6) não estão sendo exibidos quando outros stats são 0
- ⚠️ **O Blueprint está correto** - a lógica é idêntica para todos os stats, e funciona para itens com todos os stats preenchidos
- ⚠️ **O problema está na passagem de dados do C++ para o Blueprint** - os valores podem estar chegando como 0 no Blueprint, mesmo que o C++ mostre valores corretos nos logs

### Próximos Passos:
- Aguardar análise da função `UpdateBonusStats` para completar o mapeamento de todas as funções.
- **Foco principal**: Investigar por que `Critical` e `DoubleAttackRate` chegam como 0 no Blueprint quando outros stats também são 0, mesmo que o C++ mostre valores corretos nos logs.

---

**Status**: ✅ Quinta parte recebida e analisada - Função `UpdateCombatStats`

---

# Função: UpdateBonusStats

## Visão Geral
A função `UpdateBonusStats` é responsável por atualizar a exibição dos **stats de bônus** do item no tooltip: `HealthBonus`, `ManaBonus`, e `Movement`.

**Esta é a função que processa os stats de bônus que o usuário relatou não estarem aparecendo anteriormente.**

---

## Estrutura da Função

### Entrada
- **Parâmetro**: `UmbraItemStats` (tipo: `FUmbraItemStats`, por referência, const)

### Fluxo de Execução Principal

1. **Break Struct - UmbraItemStats** (`K2Node_BreakStruct_0`)
   - Extrai **TODOS** os campos do struct `FUmbraItemStats`
   - **Campos utilizados nesta função**:
     - `HealthBonus` → Formatação e exibição
     - `ManaBonus` → Formatação e exibição
     - `Movement` → Formatação e exibição
   - **Outros campos são extraídos mas não utilizados nesta função**

---

## Processamento Detalhado por Stat

### 1. Lógica para HealthBonus

#### 1.1. Knot de Roteamento (`K2Node_Knot_13`)
- **Função**: Roteamento visual do valor `HealthBonus`
- **Input**: `HealthBonus` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_16` (comparação > 0)
  - `K2Node_FormatText_33` (formatação quando > 0)

#### 1.2. Comparação (`K2Node_PromotableOperator_16`)
- **Operação**: `Greater_IntInt` (HealthBonus > 0)
- **Entrada A**: `HealthBonus` (de `K2Node_Knot_13`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_16`

#### 1.3. Condicional (`K2Node_IfThenElse_16`)
- **Condição**: `HealthBonus > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_32`
  - Formata: `"HP Bonus: {0}"` com o valor de `HealthBonus`
  - Exibe no `Text_HP_Bonus`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_33`
  - Formata com `K2Node_FormatText_32` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_HP_Bonus`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 2. Lógica para ManaBonus ⚠️ **POSSÍVEL PROBLEMA**

#### 2.1. Knot de Roteamento (`K2Node_Knot_5`)
- **Função**: Roteamento visual do valor `ManaBonus`
- **Input**: `ManaBonus` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_17` (comparação > 0)
  - `K2Node_FormatText_35` (formatação quando > 0)

#### 2.2. Comparação (`K2Node_PromotableOperator_17`) ⚠️ **INCONSISTÊNCIA DE TIPO**
- **Operação**: `Greater_ByteByte` (ManaBonus > 0) ⚠️ **USA BYTE EM VEZ DE INT!**
- **Entrada A**: `ManaBonus` (de `K2Node_Knot_5`) - tipo `int`
- **Entrada B**: `0` (valor fixo, não conectado) - tipo `byte`
- **Saída**: Boolean conectado a `K2Node_IfThenElse_17`

**⚠️ PROBLEMA POTENCIAL**: A comparação usa `Greater_ByteByte` mas `ManaBonus` é `int32`. Isso pode causar problemas de conversão ou comparação incorreta.

#### 2.3. Condicional (`K2Node_IfThenElse_17`)
- **Condição**: `ManaBonus > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_34`
  - Formata: `"MP Bonus: {0}"` com o valor de `ManaBonus`
  - Exibe no `Text_MP_Bonus`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_35`
  - Formata com `K2Node_FormatText_34` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_MP_Bonus`
  - Define visibilidade como `Collapsed` ou `Hidden`

### 3. Lógica para Movement

#### 3.1. Knot de Roteamento (`K2Node_Knot_14`)
- **Função**: Roteamento visual do valor `Movement`
- **Input**: `Movement` (do Break Struct)
- **Output**: Conectado a:
  - `K2Node_PromotableOperator_14` (comparação > 0)
  - `K2Node_FormatText_29` (formatação quando > 0)

#### 3.2. Comparação (`K2Node_PromotableOperator_14`)
- **Operação**: `Greater_IntInt` (Movement > 0)
- **Entrada A**: `Movement` (de `K2Node_Knot_14`)
- **Entrada B**: `0` (valor fixo, não conectado)
- **Saída**: Boolean conectado a `K2Node_IfThenElse_14`

#### 3.3. Condicional (`K2Node_IfThenElse_14`)
- **Condição**: `Movement > 0`
- **Se TRUE** (`then`): Conectado a `K2Node_VariableSet_28`
  - Formata: `"Mov. Bonus: {0}"` com o valor de `Movement`
  - Exibe no `Text_Mov_Bonus`
  - Define visibilidade como `Visible`
- **Se FALSE** (`else`): Conectado a `K2Node_VariableSet_29`
  - Formata com `K2Node_FormatText_28` (sem pinos conectados - provavelmente string vazia)
  - Exibe no `Text_Mov_Bonus`
  - Define visibilidade como `Collapsed` ou `Hidden`

---

## Ordem de Execução

A sequência de execução é **LINEAR e SEQUENCIAL** para cada stat:

1. **HealthBonus**:
   - Break Struct → Knot → Comparação (HealthBonus > 0) → IfThenElse → Formatação → Set Text → Set Visibility

2. **ManaBonus**:
   - Break Struct → Knot → Comparação (ManaBonus > 0) → IfThenElse → Formatação → Set Text → Set Visibility

3. **Movement**:
   - Break Struct → Knot → Comparação (Movement > 0) → IfThenElse → Formatação → Set Text → Set Visibility

**Cada stat é processado de forma INDEPENDENTE** - não há dependência entre eles.

---

## Observações Críticas

### 1. Lógica Condicional Baseada em Valor > 0
- **Cada stat de bônus tem uma verificação condicional**: `valor > 0`
- **Se o valor for > 0**: O texto é formatado e exibido, e o widget fica `Visible`
- **Se o valor for ≤ 0**: O texto é formatado como string vazia, e o widget fica `Collapsed` ou `Hidden`
- **Esta lógica é INDEPENDENTE para cada stat** - um stat não afeta o outro

### 2. Break Struct Extrai TODOS os Campos
- O `Break Struct` extrai **TODOS** os campos do `FUmbraItemStats`, incluindo:
  - Todos os atributos base (Strength, Dexterity, Intelligence, Vitality, Luck)
  - Todos os stats de combate (PhysicalAttack, MagicAttack, Critical, DoubleAttackRate, etc.)
  - Todos os bônus (HealthBonus, ManaBonus, Movement)
  - Todos os campos legados
- **Apenas os 3 stats de bônus são utilizados nesta função** - os outros campos são extraídos mas não usados

### 3. Knots Apenas para Roteamento Visual
- Os Knots (`K2Node_Knot_13`, `K2Node_Knot_5`, `K2Node_Knot_14`) são **apenas para organização visual**
- Eles **NÃO modificam os dados** - apenas roteiam o sinal

### 4. Formatação de Texto
- **Quando valor > 0**: Formata como `"{Nome do Stat}: {valor}"` (ex: "HP Bonus: 75", "MP Bonus: 50", "Mov. Bonus: 25")
- **Quando valor ≤ 0**: Formata como string vazia (sem pinos conectados no `Format Text`)

### 5. Visibilidade dos Widgets
- **Quando valor > 0**: `SetVisibility` com `Visible`
- **Quando valor ≤ 0**: `SetVisibility` com `Collapsed` ou `Hidden` (não especificado no código, mas inferido pela lógica)

### 6. ⚠️ **INCONSISTÊNCIA DE TIPO EM ManaBonus**
- **`ManaBonus` usa `Greater_ByteByte` em vez de `Greater_IntInt`**
- **`HealthBonus` e `Movement` usam `Greater_IntInt` corretamente**
- **Isso pode causar problemas de conversão ou comparação incorreta se `ManaBonus` for `int32`**
- **Esta inconsistência pode explicar por que `ManaBonus` não está sendo exibido corretamente**

---

## Conclusão da Análise - UpdateBonusStats

### Pontos Confirmados:
1. ✅ O struct `FUmbraItemStats` é recebido corretamente como parâmetro (por referência, const)
2. ✅ O `Break Struct` extrai **TODOS** os campos do struct corretamente
3. ✅ Cada stat de bônus (HealthBonus, ManaBonus, Movement) é processado **INDEPENDENTEMENTE**
4. ✅ Há uma lógica condicional que verifica se cada valor é **> 0** antes de exibir
5. ✅ Se o valor for > 0, o texto é formatado e o widget fica visível
6. ✅ Se o valor for ≤ 0, o texto é formatado como string vazia e o widget fica oculto
7. ✅ **NÃO há dependência entre os stats** - cada um é processado de forma isolada
8. ✅ Os Knots são apenas para organização visual - não modificam os dados

### Pontos de Atenção:
- ⚠️ A lógica condicional (`valor > 0`) pode explicar por que stats com valor 0 não são exibidos
- ⚠️ **MAS** o problema relatado é que stats com valores **> 0** não estão sendo exibidos quando outros stats são 0
- ⚠️ **O Blueprint está correto** - a lógica é idêntica para todos os stats, e funciona para itens com todos os stats preenchidos
- ⚠️ **O problema está na passagem de dados do C++ para o Blueprint** - os valores podem estar chegando como 0 no Blueprint, mesmo que o C++ mostre valores corretos nos logs
- ⚠️ **INCONSISTÊNCIA CRÍTICA**: `ManaBonus` usa `Greater_ByteByte` em vez de `Greater_IntInt`, o que pode causar problemas de comparação se o valor for `int32`

### Próximos Passos:
- **Foco principal**: Investigar por que os valores chegam como 0 no Blueprint quando outros stats também são 0, mesmo que o C++ mostre valores corretos nos logs.
- **Correção sugerida**: Verificar se `ManaBonus` deve usar `Greater_IntInt` em vez de `Greater_ByteByte` no Blueprint.

---

**Status**: ✅ Sexta parte recebida e analisada - Função `UpdateBonusStats`

---

# Resumo Final da Análise Completa

## Todas as Funções Analisadas

1. ✅ **SetTooltipData** - Recebe `FUmbraInventorySlot` e extrai `FUmbraItemStats` para passar para `UpdateStats`
2. ✅ **UpdateStats** - Distribui o struct `FUmbraItemStats` para 4 funções especializadas
3. ✅ **UpdateBaseStats** - Processa atributos base (Strength, Dexterity, Intelligence, Vitality, Luck)
4. ✅ **UpdatATKandDEFStats** - Processa stats de ataque/defesa (PhysicalAttack, MagicAttack, PhysicalDefense, MagicDefense)
5. ✅ **UpdateCombatStats** - Processa stats de combate (Accuracy, Dodge, Critical, DoubleAttackRate, CriticalResistance, DoubleAttackResistance)
6. ✅ **UpdateBonusStats** - Processa stats de bônus (HealthBonus, ManaBonus, Movement)

## Conclusões Gerais

### ✅ Blueprint Está Correto
- Todas as funções seguem a mesma lógica: verificar se `valor > 0` antes de exibir
- Não há lógica condicional que filtre stats por tipo de item
- Não há dependência entre stats - cada um é processado independentemente
- Os Knots são apenas para organização visual - não modificam dados

### ⚠️ Problemas Identificados

1. **Inconsistência de Tipo em ManaBonus**:
   - `ManaBonus` usa `Greater_ByteByte` em vez de `Greater_IntInt`
   - `HealthBonus` e `Movement` usam `Greater_IntInt` corretamente
   - **Isso pode causar problemas de comparação se `ManaBonus` for `int32`**

2. **Problema na Passagem de Dados C++ → Blueprint**:
   - Os logs do C++ mostram valores corretos (Critical=12, DoubleAttackRate=6)
   - Mas o tooltip exibe "xxxx" (placeholders)
   - **Isso indica que os valores estão chegando como 0 no Blueprint**
   - **O problema está na serialização/passagem do struct `FUmbraItemStats` do C++ para o Blueprint**

### 🔍 Hipóteses sobre o Problema

1. **Serialização Incompleta do Struct**:
   - Quando alguns campos são 0, a serialização do Unreal pode estar truncando ou não serializando corretamente outros campos
   - Isso explicaria por que funciona para itens com todos os stats preenchidos, mas não para itens com alguns stats zerados

2. **Problema de Alinhamento de Memória**:
   - O struct pode ter problemas de alinhamento quando alguns campos são 0
   - Isso pode causar leitura incorreta de campos específicos no Blueprint

3. **Problema na Cópia do Struct**:
   - A cópia do struct de `ParseItemStats` → `ParseItemTemplate` → `GetEquippedItem` pode estar perdendo dados quando alguns campos são 0

### 📋 Recomendações

1. **Corrigir a inconsistência de tipo em ManaBonus**:
   - Alterar `Greater_ByteByte` para `Greater_IntInt` no Blueprint

2. **Investigar a serialização do struct**:
   - Adicionar logs no Blueprint para verificar os valores recebidos
   - Verificar se há problemas de alinhamento de memória no struct
   - Considerar usar `UPROPERTY(BlueprintReadWrite)` em todos os campos do struct

3. **Verificar a cópia do struct**:
   - Garantir que a cópia do struct seja feita campo por campo explicitamente
   - Verificar se há problemas na passagem por referência vs. por valor

---

**Status**: ✅ Análise completa de todas as funções finalizada

