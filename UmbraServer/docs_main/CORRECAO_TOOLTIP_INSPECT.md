# Correção do tooltip na inspeção de jogador

Documento objetivo: causa, solução e passos exatos (nó a nó) para o tooltip dos itens equipados mostrar os dados do **jogador inspecionado** em vez do jogador local.

---

## 1. Causa do problema

- O tooltip é montado pela função **Set Tooltip Data**, que recebe um parâmetro **InSlotData** (tipo **Umbra Inventory Slot**).
- Dentro de Set Tooltip Data: Break **InSlotData** → Template → Break template → Stats → Combat/Base/Bonus Stats → **Format Text** para os textos do tooltip. Ou seja, o conteúdo do tooltip depende **apenas** do valor passado em **InSlotData**.
- Na inspeção, **InSlotData** está sendo preenchido com o item do **jogador local** (ex.: obtido do Game Instance pelo slot de equipamento), em vez do item do **jogador inspecionado** que já foi desenhado no ícone do slot.
- **Causa direta:** o nó que **chama** Set Tooltip Data está ligando no pin **InSlotData** um struct que **não** é o mesmo que o slot recebeu em **Set Head** / **Set Chest** / **Set Slot Data** (o dado que veio do **UpdateEquipmentSlots** com o CharacterInfo do inspecionado).

---

## 2. Solução (em uma frase)

Fazer com que **InSlotData** da chamada a **Set Tooltip Data** seja **sempre** o struct guardado no **próprio slot** (a variável **SlotData** ou o retorno de **Get Slot Data()** do widget de slot que está sendo hoverado). Esse struct já é o correto (próprio ou inspecionado), porque o **UpdateEquipmentSlots** já passa para cada slot o **InventorySlot** do personagem que está na tela.

---

## 3. Onde está cada coisa (referência rápida)

| O quê | Onde |
|------|------|
| Dado do slot (struct a usar no tooltip) | Widget de slot: variável **SlotData** (tipo FUmbraInventorySlot) ou função **Get Slot Data()** (C++ em `UUmbraInventorySlotWidget`) |
| Quem preenche o slot | **WBP_CharacterInfo** → função **UpdateEquipmentSlots** → para cada slot chama **Set Head** / **Set Chest** / etc. (ou **Set Slot Data**) passando o **InventorySlot** do **Get Equipped Items Array(CharacterInfo)** |
| Função que monta o tooltip | **Set Tooltip Data(InSlotData)** — pode estar no widget do slot ou no widget do tooltip; o importante é **quem chama** e **o que liga em InSlotData** |

---

## 4. WBP_EquipmentSlot – mapeamento exato (Event Graph)

Fluxo atual no **Event Graph** do **WBP_EquipmentSlot** (caminho do dado que vai para o tooltip):

| Nó no export | Tipo | Função |
|--------------|------|--------|
| **K2Node_Event_0** | Event | **OnMouseEnter** — dispara ao passar o mouse no slot |
| **K2Node_VariableGet_2** | Variable Get | Lê a variável **EquipmentSlot** (enum: Head, Chest, etc.) |
| **K2Node_CallFunction_11** | Call Function | **Get Equipped Item** (Target = Game Instance, EquipmentSlot = variável acima). **OutSlot** (pin de saída) é o struct que hoje alimenta o tooltip. |
| **K2Node_Knot_3** | Knot | Repasse do struct (InputPin ← OutSlot do K2Node_CallFunction_11) |
| **K2Node_Knot_2** | Knot | Repasse do struct (InputPin ← OutputPin do Knot_3) |
| **K2Node_CallFunction_4** | Call Function | **Set Tooltip Data** (Target = ItemTooltipWidget). Pin **InSlotData** (8C16B6EF...) recebe o struct do **OutputPin** do Knot_2. |
| **K2Node_BreakStruct_0** | Break Struct | Break **Umbra Inventory Slot**; a **entrada** (pin UmbraInventorySlot) vem do **mesmo** OutSlot do **K2Node_CallFunction_11**. A saída **ItemTemplateID** vai para **K2Node_PromotableOperator_1** (Greater que 0) → **K2Node_IfThenElse_0** (só cria tooltip se “tem item”). |

**Causa no grafo:** O struct que entra em **Set Tooltip Data (InSlotData)** vem de **Get Equipped Item (Game Instance, EquipmentSlot)**. O Game Instance sempre retorna o equipamento do **jogador local**, por isso na inspeção o tooltip mostra o item errado.

**Correção no grafo (única alteração necessária):**

1. **Desconectar** a saída **OutSlot** do nó **K2Node_CallFunction_11 (Get Equipped Item)** de:
   - o pin de entrada **UmbraInventorySlot** do **K2Node_BreakStruct_0**;
   - o pin **InputPin** do **K2Node_Knot_3** (que hoje leva até Set Tooltip Data).
2. **Inserir** um nó que forneça o struct do slot. No WBP_EquipmentSlot existem duas opções (use **uma** delas):
   - **Get Slot Data** (função) — nó **K2Node_CallFunction_8** no export. Target = **self**, saída **Return Value** (struct). Recomendado: é a função indicada no C++ para uso no tooltip.
   - **Slot Data** (variável) — nó **K2Node_VariableGet_0** no export. Saída **SlotData** (struct). Também válido; a variável usa getter com serialização correta.
3. **Ligar** a saída desse nó (**Return Value** de Get Slot Data ou **SlotData** do Variable Get) a:
   - o pin de entrada **UmbraInventorySlot** do **K2Node_BreakStruct_0** (para o Branch continuar usando ItemTemplateID > 0 com o dado do slot);
   - o pin **InputPin** do **K2Node_Knot_3** (para **InSlotData** de Set Tooltip Data vir do slot).
4. O nó **K2Node_CallFunction_11 (Get Equipped Item)** pode ficar no grafo desconectado ou ser removido; não deve mais alimentar Break Struct nem os Knots que vão para Set Tooltip Data.

**Estrutura correta após a correção:**

```
[OnMouseEnter] → [Branch: ItemTemplateID > 0]
                      ItemTemplateID vem de [Break Struct] ← entrada = [Get Slot Data (self)] Return Value
                 [true] →
                      [Create Widget WBP_ItemTooltip]
                      [Set ItemTooltipWidget]
                      [Set Tooltip Data]  InSlotData ← [Get Slot Data (self)] Return Value  (pode ser via Knot como hoje)
                      [Add to Viewport] ...
```

Assim, tanto a condição “slot tem item?” quanto o conteúdo do tooltip usam o **struct guardado no slot** (SlotData / Get Slot Data), que é o mesmo que foi passado em Set Head/Set Chest pelo UpdateEquipmentSlots — correto para próprio personagem e para inspeção.

---

## 5. Passos por nó (genérico – outros Blueprints)

### 5.1 Descobrir onde Set Tooltip Data é chamada

1. Abra o **Content Browser** e localize o Blueprint do **widget de slot de equipamento** (o que mostra Head, Chest, Feet, etc. na janela de personagem). Pode chamar-se **WBP_EquipmentSlot**, **WBP_InventorySlot** ou estar embutido em **WBP_CharacterInfo** como widgets filhos.
2. Se o slot for um Blueprint que **herda de UmbraInventorySlotWidget** (C++), esse widget tem **SlotData** e **Get Slot Data()**.
3. No **Blueprint do slot** e, se necessário, no **WBP_CharacterInfo**, procure no **Event Graph** ou em **Functions** por:
   - **Set Tooltip Data** (nó de função com um pin de entrada **InSlotData**).
   - Ou um evento como **OnHovered** / **OnMouseEnter** que abre ou preenche o tooltip.
4. Anote: **em qual Blueprint** está o nó que **chama** Set Tooltip Data e **de onde** vem o fio ligado ao pin **InSlotData** (nome do nó ou variável).

---

### 5.2 Cenário A: Set Tooltip Data é chamada **dentro do widget de slot**

**Onde:** Event Graph ou função do **Blueprint do slot** (ex.: WBP_EquipmentSlot / WBP_InventorySlot que usa UmbraInventorySlotWidget).

**O que fazer:**

1. Localize o nó **Set Tooltip Data**.
2. Localize o pin de entrada **InSlotData** (tipo Umbra Inventory Slot).
3. **Desconecte** qualquer fio que esteja ligado a **InSlotData** (ex.: saída de Get Equipped Item Value, Get Current Character Info, ou qualquer nó que busque dado no Game Instance).
4. **Ligue** ao pin **InSlotData** uma das opções abaixo (ambas são o struct do próprio slot):
   - **Opção 1:** Nó **Get Slot Data** (função do próprio widget) → pin de saída **Return Value** → **InSlotData**.  
     - Para criar: arraste o **self** (ou o widget do slot) no grafo → procure **Get Slot Data** (BlueprintCallable do UmbraInventorySlotWidget).
   - **Opção 2:** Nó **Get Variable** → variável **SlotData** → **InSlotData**.  
     - Só vale se o Blueprint do slot tiver uma variável exposta **SlotData** (o C++ já tem; em Blueprint pode aparecer como variável do parent).
5. **Estrutura desejada (fluxo):**
   ```
   [Evento que abre o tooltip, ex.: OnHovered (then)]
       →
   [Get Slot Data]  (Target = self / this slot widget)
       Return Value  →  [Set Tooltip Data]
                            InSlotData  ← (ligado aqui)
                            (outros pins conforme já existirem)
   ```
6. Salve o Blueprint.

---

### 5.3 Cenário B: Set Tooltip Data é chamada **no parent** (ex.: WBP_CharacterInfo)

**Onde:** Event Graph ou função do **WBP_CharacterInfo** (ou do widget que contém a grade de slots).

**O que fazer:**

1. Localize o nó **Set Tooltip Data** no WBP_CharacterInfo (ou parent).
2. Localize o pin **InSlotData**.
3. Se hoje **InSlotData** recebe dado do Game Instance (ex.: **Get Equipped Item Value(Equipment Slot)** ou **Get Equipped Item Value From Character Info(...)**), esse é o erro.
4. **Correção:** o valor de **InSlotData** deve vir do **widget de slot que está sendo hoverado**, não do Game Instance.
   - **Opção B1 – Recomendada:** Mover a lógica para o slot. No **widget de slot**, no evento **OnHovered** (ou equivalente), chamar **Set Tooltip Data** passando **Get Slot Data()** do próprio slot em **InSlotData** (igual ao Cenário A). Assim o parent não precisa mais passar nada para o tooltip nesse caso; cada slot passa seu próprio struct.
   - **Opção B2:** Se precisar manter a chamada no parent: o parent precisa ter uma **referência ao slot que foi hoverado**. Quando o slot for hoverado, ele deve informar ao parent “sou eu” e o parent deve chamar **Set Tooltip Data** com o retorno de **Get Slot Data()** **desse slot**. Por exemplo:
     - No slot: **OnHovered** → chama uma função do parent do tipo **OnSlotHovered(SlotWidget)** passando **self**.
     - No parent: **OnSlotHovered(SlotWidget)** → **SlotWidget** → **Get Slot Data** → **Return Value** → **Set Tooltip Data** → **InSlotData**.
5. **Estrutura desejada (B2):**
   ```
   [Parent: OnSlotHovered recebe SlotWidget]
       →
   [SlotWidget]  (parâmetro)
       Get Slot Data  (Target = SlotWidget)
           Return Value  →  [Set Tooltip Data]
                                InSlotData  ← (ligado aqui)
   ```
6. Salve os Blueprints envolvidos.

---

## 6. Checklist final

- [ ] Encontrei o nó que **chama** Set Tooltip Data e o Blueprint onde ele está.
- [ ] O pin **InSlotData** desse nó **não** está ligado a Get Equipped Item Value, Get Equipped Item Value From Character Info, nem a Get Current Character Info.
- [ ] O pin **InSlotData** está ligado ao **Get Slot Data()** do **widget de slot que está sendo hoverado** (ou à variável **SlotData** desse mesmo slot).
- [ ] Testei: abrir a janela de personagem (C) e passar o mouse no equipamento → tooltip do meu personagem.
- [ ] Testei: inspecionar outro jogador, abrir a janela e passar o mouse no equipamento → tooltip do jogador inspecionado.

---

## 7. Resumo técnico

- **Classe C++ do slot:** `UUmbraInventorySlotWidget` (`UmbraInventorySlotWidget.h/cpp`). Contém **SlotData** (FUmbraInventorySlot) e **GetSlotData()**.
- **Set Tooltip Data:** função Blueprint com entrada **InSlotData** (Umbra Inventory Slot). Internamente usa Break InSlotData → Template → Stats → Format Text. Não existe “Get Equipped Item Value” no fluxo do tooltip; o único input é **InSlotData**.
- **Correção:** garantir que **InSlotData** = struct do slot hoverado (SlotData / Get Slot Data do slot). Nenhuma outra alteração de lógica é necessária para o tooltip na inspeção.
