# Encantamento de itens — guia completo Unreal Engine **5.6.1**

O painel de encantar funciona **como a refinação**: `WBP_InventorySlot` no Designer. Você **arrasta** o equipamento e o **cristal** da bag; o ícone **fica visível** no widget (o item **não sai** da bag). Cada ação aparece em `Enchant_Status`.

Compile o C++ **antes** de abrir os Blueprints. No `WBP_ItemEnchant` o **Event Graph fica vazio** (drop e botões são C++).

**Como abrir:** botão no **`WBP_PlayerHUD`** (mesmo painel de atalhos da refinação, storage, mail, etc.). O clique só **abre** o widget **vazio** — **não** pré-seleciona MainHand nem nenhum item. Qualquer equipamento encantável (bag ou vestido, arrastado da bag) entra no slot `InventorySlot_TargetItem` por **drag & drop**.

---

## 0. Leia isto primeiro — três coisas diferentes chamadas “slot”

No projeto a palavra **slot** aparece em **três sistemas**. Misturar os três é o erro mais comum neste guia.

```
Personagem
│
├─ Corpo (onde veste) ────────────── EUmbraEquipmentSlot
│     Head, Chest, MainHand, Ring...
│     “A espada está na Mão Principal”
│
└─ Bolsa / instância do item ─────── FUmbraInventorySlot
      Uma linha da tabela player_inventory
      Tem InventoryID, nome, refine, array Enchantments
      “Esta espada concreta é o inventory_id = 4821”
            │
            └─ Três buracos de encanto nessa peça ── índice 0, 1, 2
                  Widget: Enchant_Slot0, Enchant_Slot1, Enchant_Slot2
                  JSON: { "slot": 0, "stat_key": "strength", "value": 8 }
                  “O 1º afixo desta espada é Força +8”
```

| Nome que você vê | O que é | Exemplo | O que **não** é |
|------------------|---------|---------|-----------------|
| `MainHand` / `Chest` / `Head` | Onde o item está **vestido** no boneco | A arma na mão | Não é afixo de encanto |
| `FUmbra Inventory Slot` | **O item inteiro** (instância) | A espada +3 do jogador, `Inventory ID = 4821` | Não é o Text Block `Enchant_Slot0` |
| `Enchant_Slot0` / `1` / `2` | **Text Block** no WBP (rótulo na tela) | Linha “Slot 1: Força +8” | Não é o item a encantar — só rótulo |
| `Slot Index` de `Remove Enchant At Slot` | Qual dos **3 buracos de afixo** apagar (0, 1 ou 2) | `0` = primeira linha | Não é o índice da bag (0–49) |
| `Slot Index` **dentro** de `FUmbra Inventory Slot` | Posição na **grade do inventário** | Bag slot 7 | Não use para encantar |

### 0.1 O que é `Enchant_Slot0` (e 1 e 2)

São **três caixas de texto** que você cria no Designer do `WBP_ItemEnchant`. O C++ **só** as encontra se o **nome no Hierarchy for exatamente**:

- `Enchant_Slot0` → mostra o afixo do buraco **0** (na tela: “Slot 1: …”)
- `Enchant_Slot1` → buraco **1** (“Slot 2: …”)
- `Enchant_Slot2` → buraco **2** (“Slot 3: …”)

Você **não** arrasta `Enchant_Slot0` para o pino `Slot Data`. Isso é widget de UI, não o equipamento.

### 0.2 O que é `FUmbra Inventory Slot`

Struct Blueprint (`FUmbraInventorySlot`) = **uma instância de item** que o jogador possui. Campos que o encantamento usa:

| Campo no Break | Para que serve no encanto |
|----------------|---------------------------|
| **Inventory ID** | ID da linha no MySQL. É **isso** que a API encanta. Sem esse número, nada acontece. |
| **Item Template** → nome, `Equipment Slot` (tipo da peça: arma, peito…) | Só contexto visual |
| **Refinement Level** | Nome com `+N` no painel após o drop |
| **Enchantments** (array) | Até 3 afixos já rolados nessa instância |

Você **não monta** esse struct na mão (`Make Umbra Inventory Slot`). No fluxo padrão do HUD o jogador **arrasta** o item da bag até `InventorySlot_TargetItem`; o C++ lê o `FUmbra Inventory Slot` do drag.

### 0.3 Abrir o painel ≠ escolher o item

- **Abrir** = `Create Widget` (`WBP_ItemEnchant`) + `Add to Viewport` no **`WBP_PlayerHUD`** (botão `BTN_OpenEnchant`).
- **Escolher o item** = **arrastar** equipamento até `InventorySlot_TargetItem` (qualquer peça encantável: arma, peito, anel, etc.).
- O cristal vai em `InventorySlot_Crystal` (também por drag).
- **Não** use botão no Character Info. **Não** pré-selecione MainHand no clique do HUD.

`Set Target From Slot` existe no C++ (útil para debug avançado), mas **não** faz parte do fluxo de teste pelo HUD.

---

## 1. Exemplo concreto (guarde este número)

Imagine o personagem com:

- **Mão Principal:** Espada de Ferro +3  
- No banco: `player_inventory.id = 4821`  
- Encantos já rolados: só o buraco 0 preenchido (`strength` +8)  
- Na bag: 2× Cristal de Encantamento (`item_subtype = enchant_crystal`) e 1× Extrator

O que você deve ver no `WBP_ItemEnchant` **depois de arrastar** essa espada (da bag) para `InventorySlot_TargetItem`:

```
Enchant_ItemName  →  Espada de Ferro +3
Enchant_Slot0     →  Slot 1: Força +8
Enchant_Slot1     →  Slot 2: vazio
Enchant_Slot2     →  Slot 3: vazio
```

O mesmo vale para peito, anel, botas, etc. — qualquer template com `Equipment Slot ≠ None`.

Clicar **Aplicar cristal** preenche o **primeiro buraco vazio** (aqui o índice 1).  
Clicar **Remover** ao lado de `Enchant_Slot0` chama `Remove Enchant At Slot` com **Slot Index = 0** (o afixo da Força some; a espada continua a mesma, `Inventory ID` 4821).

O resto do guia monta essa tela e a abertura pelo HUD.

---

## 2. Pré-requisitos

| # | Onde | Ação |
|---|------|------|
| 2.1 | UE **5.6.1** | Abrir `UmbraEternumUE.uproject`. |
| 2.2 | PIE | **Parar Play** (nenhuma sessão ativa). |
| 2.3 | Compile | Menu **Build → Compile**. As classes `UmbraItemEnchantWidget`, `UmbraItemTooltipWidget`, `UmbraCcChancesWidget` precisam existir. Se o Parent Class não listar, **Tools → Refresh Visual Studio Project** e compile de novo. |
| 2.4 | MySQL + WAMP | Script `www/umbra_api/scripts/add_item_enchantment_system.sql` rodado; PHP copiado para `C:\wamp64\www\umbra_api` se o Apache não aponta para o repo. |
| 2.5 | Personagem de teste | Ter **equipamento** (qualquer slot) + cristal + extrator **na bag** (não precisa estar vestido / MainHand). |

---

## 3. Criar `WBP_ItemEnchant` — Designer (clique a clique)

**Não** arraste a classe C++ `UmbraItemEnchantWidget` para a Viewport. Ela não tem layout.

### 3.1 Criar o asset

| # | Clique | Resultado |
|---|--------|-----------|
| 3.1.1 | Content Browser → pasta `Content/Widgets/UI/` (ou a pasta de Character Info) | — |
| 3.1.2 | Botão direito no vazio → **User Interface → Widget Blueprint** | Diálogo de criação |
| 3.1.3 | **User Widget** como base no assistente (depois trocamos o parent) | — |
| 3.1.4 | Nome do asset: **`WBP_ItemEnchant`** → Enter | Asset criado |
| 3.1.5 | Duplo clique no asset | Abre UMG |

### 3.2 Trocar o Parent Class

| # | Onde | Valor |
|---|------|--------|
| 3.2.1 | Barra superior → **Class Settings** (ícone de engrenagem) | Painel Details |
| 3.2.2 | **Class Options → Parent Class** | Digite `UmbraItemEnchantWidget` e selecione a classe **C++** (não um WBP) |
| 3.2.3 | Botão **Compile** (verde) | Sem erros |
| 3.2.4 | **Save** | — |

Em **My Blueprint → Variables** devem aparecer (vindas do C++): `Target Inventory ID`, `Enchant Item Name`, `Enchant Slot 0/1/2`, `Enchant Status`, `Inventory Slot Target Item`, `Inventory Slot Crystal`, `BTN Apply Crystal`, `BTN Remove 0/1/2`, `BTN Close`. Elas só preenchem sozinhas se os widgets no Designer tiverem **os mesmos nomes**.

### 3.3 Hierarquia no Designer (slots de inventário + mensagem)

Aba **Designer**. Palette à esquerda. O drop **não** se faz no Graph: o C++ configura `InventorySlot_TargetItem` / `InventorySlot_Crystal` no `NativeConstruct` (igual à refinação).

**Passo A — fundo**

1. Palette → **Panel → Border** → root. F2 → `Border_Root`.
2. Details: Brush escuro, Padding 12. Largura sugerida ~ 420.

**Passo B — coluna**

1. **Vertical Box** dentro do Border. Nome: `VB_Main`.

**Passo C — slots para arrastar (ícone fica no widget)**

Este é o equivalente a `InventorySlot_TargetItem` + `InventorySlot_RefineMaterial` da janela de refino. A Palette **não** tem “Inventory Slot”: arraste o asset **`WBP_InventorySlot`** do Content Browser para dentro do layout.

1. **Horizontal Box** no topo do `VB_Main`. Nome: `HB_DropSlots`.
2. **Esquerda — equipamento**
   - Vertical Box `VB_DropItem`.
   - Text (só rótulo): `Equipamento`.
   - Aninhe `WBP_InventorySlot`. F2 → nome **exato** `InventorySlot_TargetItem`.
   - **Is Variable** marcado. Tamanho ~ 64×64.
3. **Direita — cristal**
   - Vertical Box `VB_DropCrystal`.
   - Text rótulo: `Cristal`.
   - Outro `WBP_InventorySlot`. F2 → nome **exato** `InventorySlot_Crystal`.
   - **Is Variable** marcado. ~ 64×64.
4. **Opcional — extrator:** mesmo padrão, nome **`InventorySlot_Extractor`**. Se não criar, o **Remover** usa o primeiro extrator da bag.

No PIE, Output Log deve mostrar:

`[EnchantUI] TargetSlot=OK CrystalSlot=OK ... Status=OK`

`CrystalSlot=NULL` = nome errado no Hierarchy.

O drop **não tira** o item da bag: só **espelha o ícone** no slot do painel (como o material da refinação).

**Passo D — nome do item**

1. **Text** no `VB_Main`. F2 → **`Enchant_ItemName`**. Is Variable. Placeholder: `Solte o equipamento`.

**Passo E — três linhas de afixo + Remover**

Repita para 0, 1 e 2:

1. Horizontal Box `HB_Line0`.
2. Text **`Enchant_Slot0`** (depois `Enchant_Slot1`, `Enchant_Slot2`). Is Variable. Placeholder: `Slot 1: vazio`.
3. Button **`BTN_Remove0`** (`BTN_Remove1`, `BTN_Remove2`). Is Variable. Text filho: `Remover`.

**Passo F — aplicar e fechar**

1. Button **`BTN_ApplyCrystal`**, texto `Aplicar cristal`. Is Variable.
2. Button **`BTN_Close`**, texto `Fechar`. Is Variable.

**Passo G — texto de mensagem (obrigatório)**

Cada atividade (drop certo/errado, aplicar, remover, sucesso/falha da API) escreve aqui.

1. **Text** no **final** do `VB_Main`.
2. F2 → nome **exato** `Enchant_Status` (opcional: crie também `Text_Info`; o C++ copia a mesma frase nos dois).
3. Is Variable. Auto Wrap ligado. Min height ~ 48.
4. Apague o “Text Block” padrão (deixe vazio).

Hierarquia final:

```
Border_Root
  VB_Main
    HB_DropSlots
      VB_DropItem
        (Text) Equipamento
        InventorySlot_TargetItem     ← WBP_InventorySlot (64x64)
      VB_DropCrystal
        (Text) Cristal
        InventorySlot_Crystal        ← WBP_InventorySlot (64x64)
    Enchant_ItemName
    HB_Line0
      Enchant_Slot0
      BTN_Remove0
    HB_Line1
      Enchant_Slot1
      BTN_Remove1
    HB_Line2
      Enchant_Slot2
      BTN_Remove2
    BTN_ApplyCrystal
    Enchant_Status                   ← mensagens de cada atividade
    BTN_Close
```

**Compile** → **Save**.

Em **My Blueprint** devem aparecer também `Inventory Slot Target Item`, `Inventory Slot Crystal`, `BTN Apply Crystal`, etc., se os nomes baterem.

---

## 4. Graph do `WBP_ItemEnchant` — **vazio**

Igual à refinação: **não** ligue OnClicked, **não** ligue OnDrop, **não** dê Assign On Item Enchant Result neste WBP.

O C++ no `NativeConstruct`:

- configura drop nos `InventorySlot_*`
- liga `BTN_ApplyCrystal` / `BTN_Remove0..2` / `BTN_Close`
- escuta a API e o reload do inventário
- escreve cada atividade em `Enchant_Status`

Se você **também** ligar OnClicked no Graph, aplicar/remover dispara **duas vezes**.

Quem abre o painel é o **`WBP_PlayerHUD`** (`BTN_OpenEnchant`) — só Create Widget + Add to Viewport. **Não** ligue `Set Target From Slot` nesse clique.

### 4.1 O que acontece no drop

| Você arrasta da bag… | Solta em | Visual | `Enchant_Status` |
|----------------------|----------|--------|------------------|
| Espada / armadura | `InventorySlot_TargetItem` | Ícone no slot esquerdo | `Equipamento selecionado: Espada de Ferro +3 (ID 4821). Arraste o cristal…` |
| Poção / lixo | `InventorySlot_TargetItem` | Não muda | `Só é possível encantar equipamento…` |
| Cristal de Encantamento | `InventorySlot_Crystal` | Ícone **permanece** no slot direito | `Cristal colocado: … x2. Clique em Aplicar cristal…` |
| Qualquer outro item | `InventorySlot_Crystal` | Não muda | `Este slot aceita apenas Cristal de Encantamento…` |
| Extrator | `InventorySlot_Extractor` | Ícone no terceiro slot | `Extrator colocado: … Use Remover…` |

### 4.2 Mensagens de cada atividade

| Atividade | Texto |
|-----------|--------|
| Abrir o painel | `Arraste o equipamento para o slot da esquerda e o cristal para o da direita.` |
| Drop equipamento OK | `Equipamento selecionado: {nome} (ID {n}). Arraste o cristal…` |
| Drop equipamento inválido | `Só é possível encantar equipamento…` |
| Drop cristal OK | `Cristal colocado: {nome} x{qtd}. Clique em Aplicar cristal…` |
| Drop cristal inválido | `Este slot aceita apenas Cristal de Encantamento…` |
| Aplicar sem peça | `Solte um equipamento no slot da esquerda…` |
| Aplicar sem cristal no slot | `Arraste um Cristal de Encantamento até o slot do cristal…` |
| Aplicar enviado | `Aplicando cristal em {nome}...` |
| API ok | `Sucesso: {message da API}` |
| API falhou | `Falha: {message}` |
| Remover enviado | `Removendo encanto do slot 1 de {nome}...` |
| Pilha do cristal acabou | `O cristal da pilha acabou. Arraste outro cristal…` |
| Fechar | `Fechando painel de encantamento.` |

### 4.3 Teste do cristal no PIE

1. No HUD, clique **`BTN_OpenEnchant`** (painel de atalhos, ao lado de Refinamento / Storage / Mail…).
2. Confira `Enchant_Status`: frase pedindo para arrastar equipamento e cristal.
3. Abra o inventário. Arraste **qualquer equipamento** (peito, anel, arma, etc.) até o quadrado **Equipamento** — o ícone tem que **ficar** lá.
4. Arraste o cristal até o quadrado **Cristal** — o ícone tem que **ficar** lá.
5. **Aplicar cristal** → `Aplicando…` depois `Sucesso: …`; uma linha `Enchant_SlotN` deixa de ser vazio.

---

## 5. Abrir o painel pelo HUD — `WBP_PlayerHUD`

Padrão igual à refinação (`BTN_RefinementPanel` → `WBP_RefinementWindow`):

| # | Ação |
|---|------|
| 1 | Abrir `/Game/Widgets/HUD/WBP_PlayerHUD` |
| 2 | Designer: botão **`BTN_OpenEnchant`** no mesmo painel/grupo dos outros atalhos (`BTN_RefinementPanel`, `BTN_StoragePanel`, `BTN_Mailbox`, …). Texto: `Encantar` (ou ícone). **Is Variable** = sim |
| 3 | Variável `EnchantPanelRef` tipo **`WBP_ItemEnchant`** (Object Reference) — evita empilhar janelas |
| 4 | Graph: **só** abrir/fechar o widget — **sem** `Get Equipped Item`, **sem** MainHand, **sem** `Set Target From Slot` |

### 5.1 Graph — OnClicked (copie nesta ordem)

**1.** Arraste `BTN_OpenEnchant` → **On Clicked**.

**2.** Fechar painel antigo (se existir)

- Get `EnchantPanelRef` → **Is Valid**  
- Is Valid → **Remove from Parent** (Target = `EnchantPanelRef`)  
- Continuar o fluxo nos dois ramos (válido já removido / inválido).

**3.** Create Widget

- **Create Widget** → Class = asset **`WBP_ItemEnchant`** (UMG). **Não** escolha a classe C++ `UmbraItemEnchantWidget`.  
- Owning Player = **Get Player Controller**.

**4.** Add to Viewport

- **Add to Viewport** (Return Value do Create Widget), ZOrder sugerido `10`.  
- Opcional: **Set Alignment / Set Position in Viewport** (ex. `420, 80`) se quiser fixar a posição.  
- **SET EnchantPanelRef** = Return Value do Create Widget.

Cadeia resumida:

```
OnClicked BTN_OpenEnchant
  → (se EnchantPanelRef válido) Remove from Parent
  → Create Widget (Class = WBP_ItemEnchant)
  → Add to Viewport (ZOrder 10)
  → SET EnchantPanelRef
```

**Compile** o `WBP_PlayerHUD` → **Save**.

### 5.2 O que o jogador faz depois de abrir

O painel nasce **vazio** (`Enchant_Status` pede drag). Fluxo:

1. Abrir inventário (bag).  
2. Arrastar **qualquer item encantável** (template com `Equipment Slot ≠ None`) → `InventorySlot_TargetItem`.  
3. Arrastar **cristal** (`item_subtype = enchant_crystal`) → `InventorySlot_Crystal`.  
4. (Opcional) Arrastar **extrator** → `InventorySlot_Extractor`, ou deixar o C++ achar o primeiro da bag ao Remover.  
5. **Aplicar cristal** / **Remover** nas linhas.

**Não** é necessário o item estar na MainHand. Peito, anel, botas, off-hand, etc. funcionam desde que estejam na bag (ou você consiga arrastar a instância até o slot do painel).

### 5.3 O que **não** fazer

| Evite | Motivo |
|-------|--------|
| Botão Encantar no `WBP_CharacterInfo` | Os outros sistemas de teste já usam o painel do HUD |
| `Get Equipped Item` + MainHand no OnClicked do HUD | Trava o teste numa só peça; o painel deve abrir vazio |
| `Set Target From Slot` no clique do HUD | Opcional/debug; o fluxo oficial é drag no `InventorySlot_TargetItem` |
| Graph OnDrop / OnClicked dentro do `WBP_ItemEnchant` | Duplica o C++ (Graph fica **vazio**) |

### 5.4 (Opcional) `Set Target From Slot`

Só se precisar pré-preencher o alvo em um fluxo especial (NPC, debug). Pino **Slot Data** = struct `FUmbra Inventory Slot` completo. **Não** use no botão do HUD de teste.

---

## 6. Conferência: abertura pelo HUD

| Check | Correto | Errado |
|-------|---------|--------|
| Botão | `BTN_OpenEnchant` no `WBP_PlayerHUD` | Botão só no Character Info |
| Create Widget Class | Asset `WBP_ItemEnchant` | Classe C++ `UmbraItemEnchantWidget` |
| Após o clique | Painel vazio + mensagem de arrastar | Já mostra MainHand / espada sem drag |
| Escolher peça | Drag da bag → `InventorySlot_TargetItem` | Enum MainHand / só arma |
| Cristal | Drag → `InventorySlot_Crystal` | Pré-set no Graph |

No PIE: clique o botão do HUD → inventário aberto → arraste peito **ou** anel **ou** arma → ícone no slot esquerdo → cristal no direito → Aplicar.  
**Exceção:** cristais de chance/resist CC só em acessórios (anel, amuleto, colar, brinco, bracelete).

---

## 7. Tooltip — `WBP_ItemTooltipRefined`

### 7.1 Parent

1. Content Browser → `WBP_ItemTooltipRefined` (ou `WBP_ItemTooltip` se for o asset em uso).  
2. Class Settings → Parent Class = **`UmbraItemTooltipWidget`**.  
3. Compile.

### 7.2 Designer — seção de cristais (ícone + texto)

Abaixo dos stats, monte uma seção com **até 3 linhas**. Nomes **exatos** (Is Variable = sim):

```
Panel_Enchantments          ← Vertical Box (opcional; some se não houver afixo)
  Text_EnchantmentsHeader   ← "Cristais / Encantos" (C++ preenche)
  HB_EnchantCrystal0        ← Horizontal Box ~24–32px de altura
    Image_EnchantCrystal0   ← Image 24x24
    Text_EnchantCrystal0    ← "Força +8"
  HB_EnchantCrystal1
    Image_EnchantCrystal1
    Text_EnchantCrystal1
  HB_EnchantCrystal2
    Image_EnchantCrystal2
    Text_EnchantCrystal2
```

| Widget | Tipo | Uso |
|--------|------|-----|
| `Panel_Enchantments` | Panel / Vertical Box | Collapsed se item sem encanto |
| `Text_EnchantmentsHeader` | Text Block | Título da seção |
| `HB_EnchantCrystal0..2` | Horizontal Box | Uma linha por buraco de afixo |
| `Image_EnchantCrystal0..2` | Image | Ícone pequeno do cristal |
| `Text_EnchantCrystal0..2` | Text Block | `{DisplayName} +{value}` |

Visibility inicial de tudo: **Collapsed**. O C++ abre só as linhas ocupadas.

`Tooltip_Enchantments` (bloco de texto antigo) continua opcional: se existirem as linhas ícone+texto, o C++ **esconde** o bloco legado para não duplicar.

### 7.3 Graph — **não precisa**

`SetTooltipData` em C++ já chama `RefreshEnchantmentFromSlot` (ícones + textos). Não ligue Graph extra para encantos.

---

## 8. Painel de CC — `WBP_CcChances`

### 8.1 Criar

User Widget Blueprint nome `WBP_CcChances`. Parent = **`UmbraCcChancesWidget`**.

**Event Graph vazio.** Fill, refresh e close são C++ (`SetFromCharacterInfo`, bind em `OnCharacterInfoLoaded`, `BTN_Close`).

### 8.2 Oito Text Blocks (nomes literais)

Vertical Box, Is Variable em todos:

`Cc_StunChance`, `Cc_SilenceChance`, `Cc_RootChance`, `Cc_SlowChance`,  
`Cc_StunResist`, `Cc_SilenceResist`, `Cc_RootResist`, `Cc_SlowResist`.

Opcional: botão `BTN_Close` (mesmo nome) — o C++ fecha o painel.

O C++ escreve sozinho `Atordoar (causa): 12%` etc.

### 8.3 Abrir ao lado do info (não substitui o info)

1. No **Game Instance** (defaults / Class Defaults): **Cc Chances Widget Class** = asset `WBP_CcChances`.
2. No **`WBP_CharacterInfo1`** (Parent = `UmbraCharacterInfoWidget`): botão Hierarchy nome **`BTN_OpenCc`** (Is Variable). **Sem nós no Event Graph** — o C++ do Character Info chama `ToggleCcChancesPanel(this)` no clique.

O C++ cria o `WBP_CcChances`, preenche e **ancora ao lado** do Character Info (direita; se não couber na tela, esquerda). Não usa posição fixa no centro. Em modo inspeção o botão some.

---

## 9. Teste guiado (qualquer equipamento)

1. Compile C++ + `WBP_ItemEnchant` + `WBP_PlayerHUD`. PIE: login → mundo.  
2. Clique **`BTN_OpenEnchant`** no painel de atalhos do HUD (ao lado de Refinamento / Storage…).  
3. Painel abre **vazio** com mensagem de arrastar.  
4. Abra o inventário. Arraste um peito **ou** anel **ou** arma (qualquer encantável) → `InventorySlot_TargetItem`.  
5. Arraste o cristal → `InventorySlot_Crystal` → **Aplicar**.  
6. Remover na linha com afixo (BTN da mesma linha) → precisa de extrator na bag ou no slot.  
7. Hover no mesmo item → tooltip com as 3 linhas de encanto.  
8. (Opcional) **Chances CC**: `BTN_OpenCc` no `WBP_CharacterInfo1` (C++ no `UmbraCharacterInfoWidget`).

---

## 10. Banco / PHP / Manager (fora do Editor)

- SQL: `www/umbra_api/scripts/add_item_enchantment_system.sql`  
- JSON da instância: `enchantments_json` → `[{"slot":0,"stat_key":"strength","value":8}]` — o `"slot":0` é o **buraco de afixo**, o mesmo do `Enchant_Slot0`.  
- APIs: `POST /api/inventory/enchant_apply.php`, `enchant_remove.php`  
- UmbraManager: aba Encantamentos (pesos 0/1/2/3 slots = **quantos buracos** o drop pode nascer, não o slot do corpo).  
- Combate: Zone soma afixos; CC = `clamp(skill.chance_percent + caster_*_chance − max(0, resist − pen), 0, 100)`.
- **Cristais de CC** (`stun/silence/root/slow_chance` e `*_resist`): só em **acessórios** (`ring`, `amulet`, `necklace`, `earring`, `bracelet`). Armas/armaduras rejeitam apply e não sorteiam esses afixos no spawn/loot.

---

## 11. Problemas comuns

| O que você vê | Causa | O que fazer |
|---------------|--------|-------------|
| Botão só no Character Info | Fluxo antigo do guia | Use `BTN_OpenEnchant` no `WBP_PlayerHUD` (§5) |
| Painel já abre com MainHand | OnClicked chama `Get Equipped Item` / `Set Target From Slot` | Remova esses nós; só Create Widget + Add to Viewport |
| Só dá para testar arma | Pré-seleção MainHand | Arraste peito/anel/etc. da bag para `InventorySlot_TargetItem` |
| Apply “equipamento inválido” | Não arrastou peça válida ainda | Drop em `InventorySlot_TargetItem`; Inventory ID > 0 |
| Linhas sempre “Slot N: vazio” mesmo com encanto no tooltip | Nomes `Enchant_Slot0` diferentes no Designer | F2 no Hierarchy; Compile |
| Remover a linha 1 apaga o afixo da linha 2 | Slot Index 1 no botão da linha 0 | BTN_Remove0 → índice **0** |
| Create Widget aparece em branco | Class = classe C++ | Class = asset `WBP_ItemEnchant` |
| Apply CC em peito/arma | Afixo é chance/resist de CC | Use anel/amulet/colar/brinco/bracelete; PHP/UI bloqueiam o resto |
| Cristal não achado / apply recusa | Não soltou no `InventorySlot_Crystal` ou subtype errado | Arraste o cristal até o quadrado **Cristal**; o ícone tem que aparecer |
| `CrystalSlot=NULL` no log | Nome do widget ≠ `InventorySlot_Crystal` | F2 no Hierarchy; Is Variable; Compile |
| Ícone do cristal não fica no slot | Parent errado ou Graph também trata OnDrop | Graph vazio; Compile C++ novo |
| Mensagem nunca muda | Falta Text `Enchant_Status` | Passo G da §3.3 |
| `InventorySlot_EquippedCrystal1/2/3` vazios com afixo | Nome Hierarchy ≠ BindWidget | F2: `InventorySlot_EquippedCrystal1` etc.; Compile; log `[EnchantUI] EquippedCrystals=OK/OK/OK` |
