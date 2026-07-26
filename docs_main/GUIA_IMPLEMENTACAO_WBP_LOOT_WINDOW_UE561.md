# Guia completo — `WBP_LootWindow` (UE 5.6.1)

Janela de **loot do killer** (abre sozinha ao matar o mob). Parent C++: **`UUmbraLootWindowWidget`**.

- Grade **2×5** (`Grid_LootSlots`) com slots `WBP_InventorySlot` (ou o mesmo asset da loja).
- Botões: **`BTN_LootItem`** (slot selecionado), **`BTN_LootAll`**, **`Btn_Close`**.
- Slot de **ouro** (`entry_kind=1`): ícone via Class Defaults (`GoldIcon` / `GoldIconPath`) + quantidade = valor do gold.
- Protocolo Zone WS: **111** open, **115** update, **112** close; cliente envia **113** / **114**.

**Código de referência:** `UmbraLootWindowWidget.cpp/.h`, `UmbraGameInstance::OpenLootWindowUI` / `SendLootTakeItem` / `SendLootTakeAll`, `FUmbraLootSlot` / `FUmbraLootWindowState`.

---

## 1. Pré-requisitos

| # | Onde | Ação |
|---|------|------|
| 1.1 | Compilar módulo C++ | Menu **Build** no Editor até zerar erros (`UmbraLootWindowWidget`, delegates de loot). |
| 1.2 | Content Browser → Widget Blueprint | Nome sugerido: **`WBP_LootWindow`**. |
| 1.3 | File → Reparent Blueprint | **Parent Class** = `UmbraLootWindowWidget`. |
| 1.4 | `BP_UmbraGameInstance` (Class Defaults) | **Loot \| UI → Loot Window Widget Class** = este WBP (não a classe C++ crua). |
| 1.5 | `WBP_LootWindow` **Class Defaults** (ícone de predefinições no toolbar — **não** o Hierarchy) | **Loot \| Classes → Loot Slot Widget Class** = **`WBP_InventorySlot`** (parent `UmbraInventorySlotWidget`). **Se estiver None, a grade fica vazia mesmo com `Grid_LootSlots` no Designer.** |
| 1.6 | Gold icon | Em Class Defaults: **Gold Icon** (textura) **ou** **Gold Icon Path**. |
| 1.7 | Servidor / DB | Rodar SQL loot + reiniciar Zone. |

### Layout (botões sumindo no PIE)

Causa típica: `Grid_LootSlots` com **Size = Fill** espreme a linha dos botões (altura 0) ou o `Border_InventoryPanel` no Canvas é baixo demais (corta o rodapé).

No Designer, no slot do `Vertical Box` pai:

| Widget | Size |
|--------|------|
| wrapper da grade / `Grid_LootSlots` | **Fill** |
| `HBox_LootOptions` | **Auto** |
| Horizontal Box vazio (se existir entre gold e botões) | **Auto** ou **Collapsed** |

`Border_LootPanel` (Canvas): o tamanho fica **só no Designer** — o C++ **não** altera Size X/Y.

Compile o módulo após puxar as alterações e teste de novo.

---

## 2. Nomes obrigatórios no Designer (`Variable Name`)

O C++ resolve por **`GetWidgetFromName`** — os nomes têm de coincidir **exatamente**.

| Nome no Hierarchy | Tipo sugerido | Função |
|-------------------|---------------|--------|
| **`Border_LootPanel`** | `Border` (Canvas) | Moldura do painel; Size X/Y definidos no Designer (o C++ não altera). |
| **`Grid_LootSlots`** | `Uniform Grid Panel` | Grade 2×5; o C++ cria 10 filhos em runtime. |
| **`BTN_LootItem`** | `Button` | Loot do slot selecionado (opcode 113). |
| **`BTN_LootAll`** | `Button` | Loot all (opcode 114). |
| **`HBox_LootOptions`** | `Horizontal Box` | Linha dos botões (Size = **Auto**). |
| **`Btn_Close`** | `Button` | Fecha o painel (`RemoveFromParent`). |
| **`Text_LootTitle`** | `Text` | Título; C++ preenche com `"Loot"`. |
| **`TXT_DroppedGold`** | `Text` (opcional) | Soma do gold nos slots do corpse. |

Layout sugerido: painel central → título → `Grid_LootSlots` → `TXT_DroppedGold` → `HBox_LootOptions` (Loot / Loot All).

---

## 3. Visual do slot de gold

1. Quando o servidor manda `kind=1`, o C++ chama `CreateInventorySlotFromLootGold(qty, slotIndex, GoldIcon)`.
2. O slot usa `WBP_InventorySlot` → a função Blueprint **`UpdateSlotVisual`** / `TryCallBlueprintUpdateSlotVisual` desenha ícone + quantidade.
3. Configure **Gold Icon** no Class Defaults do `WBP_LootWindow` (ex.: textura de moeda já usada na UI de gold).
4. Quantidade no slot = valor de ouro daquele drop (não é stack de item).
5. Destaque selecionado: o C++ altera `SetColorAndOpacity` no slot clicado (mais claro = selecionado).

Não é necessário Graph especial para gold além de `UpdateSlotVisual` já existente no `WBP_InventorySlot`.

---

## 4. Fluxo de teste (Goblin)

1. Subir Auth/Gateway/Zone com o SQL aplicado (`kill_exp` + `npc_loot_entries` do Goblin `npc_template_id=10`).
2. No Editor: `BP_UmbraGameInstance` com **Loot Window Widget Class** = `WBP_LootWindow`.
3. Login → entrar na zona → matar um Goblin (template 10).
4. Esperado:
   - EXP (opcodes **106** / **107** se level up).
   - Janela de loot **abre sozinha** (111) com gold (5–20) e itens seedados se existirem no DB.
5. Clicar num slot → destaque → **Loot Item** → item vai ao inventário / gold atualiza (`PlayerGold` via 115).
6. **Loot All** esvazia o corpse; janela fecha (112 ou update com 0 slots).
7. Só o **killer** recebe 111; outro jogador não vê a janela.

---

## 5. Checklist rápido

| Item | OK? |
|------|-----|
| Parent = `UmbraLootWindowWidget` | |
| `Grid_LootSlots` + 3 botões com nomes exatos | |
| `Loot Slot Widget Class` = `WBP_InventorySlot` | |
| `Loot Window Widget Class` no GameInstance | |
| `GoldIcon` ou `GoldIconPath` preenchido | |
| SQL loot + Zone reiniciado | |
| Matar Goblin abre loot + EXP | |

---

## 6. Problemas comuns

| Sintoma | Causa provável | Correção |
|---------|----------------|----------|
| Não abre janela | `LootWindowWidgetClass` None | §1.4 |
| Grade vazia / erro no log | `LootSlotWidgetClass` None ou classe errada | §1.5 |
| Gold sem ícone | `GoldIcon` vazio | §1.6 / §3 |
| Take não faz nada | Zone sem WS / corpse expirado | Conferir conexão Zone e TTL ~60s |
| EXP sem loot | droplist vazia ou só EXP | Normal se rolls falharem; seed Goblin tem gold 100% |

---

## 7. Fora do escopo deste WBP (v1)

- Mesh/corpse clicável no mundo.
- Party need/greed.
- Auto-loot real (`ShouldAutoLoot` stub = false no C++).
