# Guia de implementação: Mail + Broadcast Toast — UE 5.6.1

**Engine:** Unreal Engine **5.6.1** · **Código:** `UmbraEternumUE/`  
**Backend:** `www/umbra_api/api/mail/*.php` + `api/admin/admin_send_mail*.php` · **BD:** `mail_messages` / `mail_attachments`  
**Zone:** opcodes **73** (SystemBroadcast) e **74** (MailNotify) em [`MovementProtocol.hpp`](src/zone/MovementProtocol.hpp)

A UI fica em **C++** (`UUmbraBroadcastToastWidget`, `UUmbraMailWidget`). O Blueprint só define **Parent Class**, **hierarquia UMG** e **nomes BindWidget** (e liga o ícone HUD). Não é necessário reimplementar HTTP/opcodes no Graph.

**Classes:**

| Classe | Arquivo |
|--------|---------|
| Toast | [`UmbraBroadcastToastWidget.h/.cpp`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraBroadcastToastWidget.h) |
| Mail | [`UmbraMailWidget.h/.cpp`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraMailWidget.h) |
| Hub | [`UmbraGameInstance`](UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h) — `ShowSystemBroadcast`, `RequestMail*`, `OnMailNotifyReceived` |
| Roteamento | [`NetMovementClient.cpp`](UmbraEternumUE/Source/UmbraEternumUE/Network/NetMovementClient.cpp) — `bIsSocialMessage` inclui 73/74 |

---

## 1) Pré-requisitos

| Passo | Ação |
|--------|------|
| MySQL | Executar [`www/umbra_api/scripts/create_mail_system.sql`](www/umbra_api/scripts/create_mail_system.sql) e [`create_admin_audit.sql`](www/umbra_api/scripts/create_admin_audit.sql) |
| API | Copiar `www/umbra_api` para o webroot do WAMP (`C:\wamp64\www\umbra_api` se for o caso) |
| Zone | Rebuild `zone_server` (opcodes 73/74 + `notify_mail` / `flush_mail_notify_queue`) |
| C++ UE | Recompilar módulo `UmbraEternumUE` após os `.h/.cpp` |
| Manager | UmbraManager com abas Guilds / Auction / Mail + Audit MySQL |

**Regras de anexos:** até **5** slots (`slot_index` 0–4), shape alinhado a `FUmbraInventorySlot` (`item_template_id`, `quantity`, `refinement_level`, `durability`, `bonus_stats_json`). Claim cria linhas em `player_inventory`.

---

## 2) Endpoints HTTP (player)

| Arquivo | Descrição |
|---------|-----------|
| `api/mail/list_mail.php` | Lista cartas do personagem JWT |
| `api/mail/get_mail.php` | Detalhe + `attachments[]` (marca lida) |
| `api/mail/send_mail.php` | Envia (debita inventário se houver anexos) |
| `api/mail/claim_attachment.php` | `slot_index` ou `claim_all` |
| `api/mail/delete_mail.php` | Bloqueia se houver anexo não claimado |
| `api/mail/mark_read.php` | Marca lida |

**Exemplo anexos (admin ou player):**

```json
"attachments": [
  { "slot_index": 0, "item_template_id": 1001, "quantity": 5, "refinement_level": 0, "durability": 100 },
  { "slot_index": 1, "item_template_id": 2003, "quantity": 1, "refinement_level": 3, "durability": 100 }
]
```

**Admin:** `admin_send_mail.php` (1 player), `admin_send_mail_all.php` (`confirm_all=true`), `list_mail_admin.php`.

---

## 3) Contratos WebSocket (little-endian)

### Opcode 73 — SystemBroadcast

`[73][severity:u8][durationMs:u16][nameLen:u16][name:utf8][msgLen:u16][msg:utf8]`

- severity: 0 info, 1 aviso, ≥2 crítico  
- GM `broadcast` envia **70 + 73**

### Opcode 74 — MailNotify

`[74][mailId:u32][fromLen:u16][from:utf8][subjLen:u16][subject:utf8]`

Comandos admin Zone: `notify_mail`, `flush_mail_notify_queue` (consome `mail_notify_queue`).

---

## 4) WBP_BroadcastToast — estrutura e vinculações

**Path sugerido:** `Content/Widgets/UI/Notifications/WBP_BroadcastToast`  
**Parent Class:** `Umbra Broadcast Toast Widget` (`UUmbraBroadcastToastWidget`)

Espelha o padrão de [`UUmbraCombatFeedbackWidget`](UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraCombatFeedbackWidget.h): toast HUD, sem input, some após o tempo.

### 4.1 Tabela BindWidget (obrigatório / opcional)

Todos usam `BindWidgetOptional` — se o nome estiver errado, o C++ **não quebra**, mas o texto não aparece.

| Nome no Designer (exato) | Tipo UMG | Obrigatório? | Uso no C++ |
|--------------------------|----------|--------------|------------|
| **`Broadcast_Text`** | `Text Block` | **Sim** (para ver a mensagem) | `SetText` + cor por severity |
| **`Broadcast_Panel`** | qualquer `Panel Widget` (Border, Overlay, Canvas…) | Não | reservado; pode envolver o texto |

**Regra de nome:** em *Details* → *Name* (nome da variável na hierarquia), **não** o texto exibido. Renomear com botão direito → *Rename*.

### 4.2 Hierarquia sugerida (árvore)

```
Overlay_Root                          ← Size to content / âncora topo-centro
└── Broadcast_Panel (Border)          ← BindWidget opcional; fundo semi-transparente
    └── Broadcast_Text (TextBlock)    ← BindWidget; alinhamento centro; wrap se necessário
```

**Layout / âncoras (HUD):**

1. No `WBP_BroadcastToast`, raiz: **Overlay** ou **Canvas Panel**.  
2. Âncora tipicamente **topo-centro** (Anchors 0.5, 0 · Alignment 0.5, 0 · Position Y ~ 80–120).  
3. `Broadcast_Panel`: *Padding* 12–16; cor de fundo escura com alpha (~0.7).  
4. `Broadcast_Text`: fonte grande o bastante para leitura; *Auto Wrap Text* = true se a mensagem for longa.  
5. **Visibility** default no Designer pode ser *Visible*; o C++ força `Collapsed` no `NativeConstruct` e `HitTestInvisible` ao mostrar (não rouba clique do jogo).

### 4.3 Class Defaults

| Propriedade | Valor sugerido |
|-------------|----------------|
| `Default Duration Sec` | `5.0` (sobrescrito pelo `durationMs` do opcode 73) |

### 4.4 Registro no GameInstance (sem Graph obrigatório)

No `NativeConstruct`, o C++ já chama:

`GI->RegisterBroadcastToastWidget(this)`

No `NativeDestruct`: `UnregisterBroadcastToastWidget`.

**O que você precisa no Blueprint:** apenas **criar o widget uma vez** e mantê-lo vivo (filho do HUD ou Add to Viewport no BeginPlay do PlayerHUD / GameMode).

#### 4.4.1 Opção A — filho do `WBP_PlayerHUD` (recomendada)

1. Abra `WBP_PlayerHUD` (ou o HUD raiz do personagem).  
2. Adicione um **Widget** filho: *User Created* → `WBP_BroadcastToast`.  
3. Nomeie a instância como quiser (`Toast_Broadcast`); o BindWidget é **interno** ao WBP filho.  
4. Âncora topo-centro no slot do HUD.

#### 4.4.2 Opção B — Create Widget no BeginPlay

1. Event BeginPlay (HUD / PlayerController).  
2. **Create Widget** → Class = `WBP_BroadcastToast`.  
3. **Add to Viewport** (ZOrder alto, ex. 100).  
4. Guarde a referência se quiser; o registro no GI é automático.

### 4.5 Fluxo de dados (toast)

```
Zone opcode 73
  → NetMovementClient (bIsSocialMessage)
  → UmbraGameInstance::ProcessSocialWebSocketMessage
  → ShowSystemBroadcast(Msg, Severity, DurationSec)
  → BroadcastToastWidget->ShowBroadcast(...)
       (fallback: ShowCombatFeedback se toast não registrado)
```

**Cores por severity (C++):** 0 = azul claro · 1 = amarelo · ≥2 = vermelho/alaranjado.

### 4.6 Checklist BindWidget — Broadcast

- [ ] Parent Class = `UmbraBroadcastToastWidget`  
- [ ] Existe `Text Block` com nome **`Broadcast_Text`**  
- [ ] Widget está no Viewport/HUD em runtime (não só no Content Browser)  
- [ ] Teste: GM `broadcast message="teste"` → toast aparece e some sozinho  

---

## 5) WBP_Mail — estrutura e vinculações

**Path sugerido:** `Content/Widgets/UI/Mail/WBP_Mail`  
**Parent Class:** `Umbra Mail Widget` (`UUmbraMailWidget`)

### 5.1 Tabela BindWidget completa

Todos são `BindWidgetOptional`. Botões com nome correto ganham `OnClicked` no C++ automaticamente.

#### Textos / status

| Nome exato | Tipo | Uso |
|------------|------|-----|
| `Text_Title` | TextBlock | Título da janela (opcional) |
| `Text_From` | TextBlock | Remetente da carta selecionada |
| `Text_Subject` | TextBlock | Assunto |
| `Text_Body` | TextBlock | Corpo (prefira *Auto Wrap*) |
| `Text_UnreadBadge` | TextBlock | Contagem de não lidas (`GetUnreadCount` / após list) |
| `Text_AttachLabel` | TextBlock | Opcional — C++ seta “Anexos” no Construct |
| `Text_MailStatus` | TextBlock | Feedback (claim/delete/erros), ex.: “Resgate os anexos antes de apagar” |

#### Lista

| Nome exato | Tipo | Uso |
|------------|------|-----|
| `Scroll_MailList` | ScrollBox | BindWidgetOptional — scroll da coluna esquerda |
| `VB_MailList` | VerticalBox | BindWidgetOptional — **obrigatório** para a lista; C++ faz ClearChildren + AddChild |

> **BP mínimo:** não marque `VB_MailList` como variável Graph e **não** escreva ForEach. O C++ (`RebuildMailListUI`) popula sozinho após `ApplyMailList`. Se o nome não bater, o log avisa `VB_MailList ausente`.

#### Botões (OnClicked no C++ — BindWidget)

| Nome exato no Designer | Handler C++ | Ação |
|------------------------|-------------|------|
| `BTN_Refresh` | `OnRefreshClicked` | `RefreshMailList` |
| `BTN_ClaimAll` (sem espaço; também aceita `BTN_Claim All`) | `OnClaimAllClicked` | claim de **todos** os anexos |
| `BTN_Claim` | `OnClaimClicked` | claim do **anexo selecionado** na grade |
| `BTN_Delete` | `OnDeleteClicked` | apaga carta (**bloqueado** se houver anexo não resgatado) |
| `BTN_Send` | `OnSendClicked` | `SendMailToTarget` |
| `BTN_Close` | `OnCloseClicked` | `CloseMail` → Collapsed |

**Consistência visual:** coloque `BTN_Claim` no Designer (mesmo estilo do Delete), nome exato `BTN_Claim`. O C++ só cria fallback feio se o BindWidget faltar.

**Refresh / Claim All:** fiquem no rodapé (fora do `HBox_DetailActions`). Nomes: `BTN_Refresh` / `BTN_ClaimAll`. O C++ **não** move o Refresh para o painel de Claim/Delete.

**Delete:** cliente + PHP bloqueiam apagar com anexos pendentes (`Resgate os anexos antes de apagar`).

**Text_MailStatus:** some sozinho após `MailStatusDurationSec` (padrão 3.5s; Class Defaults).

#### Anexos (grade C++ 1×5 — igual ao loot)

| Nome / propriedade | Tipo | Uso |
|--------------------|------|-----|
| `Grid_MailAttachments` | **Uniform Grid Panel** | Preferido. C++ cria slots 0–4 |
| `HBox_Attachments` | Horizontal Box | Compat: se não houver Grid, C++ limpa o HBox e injeta um Uniform Grid |
| Class Defaults → **Mail Attach Slot Class** | WBP parent `UmbraInventorySlotWidget` | **Obrigatório** (ex.: `WBP_InventorySlot`). Sem isso a grade fica vazia |

`MailAttach_0`…`_4` no Designer **não são mais necessários** (legado). Remova-os do HBox se for usar só a grade.

Fluxo: GI preenche `InventoryID` sentinel + **nome/tipo/rarity do PHP** (tooltip) → grade → `SetSlotData` + ícone. LMB no slot **seleciona**; `BTN_Claim` resgata. Drag/accept desligados.

**Designer mínimo:**
1. Troque `HBox_Attachments` por Uniform Grid `Grid_MailAttachments` **ou** deixe o HBox vazio (C++ injeta).
2. `WBP_Mail` → Class Defaults → Mail\|Classes → **Mail Attach Slot Class** = `WBP_InventorySlot`.
3. Compile / Save. Zero Graph.

### 5.2 Hierarquia sugerida (árvore)

```
Canvas_Root / Overlay_Root
└── Border_Frame (fundo da janela)
    └── VerticalBox_Root
        ├── HBox_Header
        │   ├── Text_Title
        │   ├── Text_UnreadBadge          ← badge "3"
        │   ├── BTN_Refresh
        │   └── BTN_Close
        ├── HBox_Body                     ← Size Fill
        │   ├── Border_List (peso ~0.4)
        │   │   └── Scroll_MailList
        │   │       └── VB_MailList       ← linhas criadas em C++
        │   └── Border_Detail (peso ~0.6)
        │       └── VerticalBox_Detail
        │           ├── Text_From
        │           ├── Text_Subject
        │           ├── Text_Body          ← Fill + wrap
        │           ├── Text_AttachLabel ("Anexos")
        │           ├── Grid_MailAttachments  ← Uniform Grid; slots 0–4 em C++
        │           ├── HBox_DetailActions
        │           │   ├── BTN_Claim
        │           │   ├── BTN_ClaimAll
        │           │   ├── BTN_Refresh
        │           │   └── BTN_Delete
        │           └── Text_MailStatus      ← feedback claim/delete
        │           └── (opcional) Border_Compose — ver §5.5
        └── (rodapé opcional)
```

### 5.3 Variáveis Blueprint / propriedades C++

Expostas no C++ (Category **Mail**):

| Propriedade | Tipo | Quem preenche |
|-------------|------|----------------|
| `Selected Mail Id` | int32 | `SelectMail` / detalhe |
| `Compose To Name` | String | jogador (Graph ou EditableText → Set) |
| `Compose Subject` | String | idem |
| `Compose Body` | String | idem |
| `Mail Entries` | Array `FUmbraMailListEntry` | só leitura; após list |
| `Attachment Slots` | Array `FUmbraInventorySlot` | só leitura; após detalhe |
| `Unread Count` | int32 | após list |

**Struct `FUmbraMailListEntry` (por linha):** `MailId`, `FromName`, `Subject`, `bIsRead`, `AttachmentCount`, `UnclaimedAttachments`, `Preview`, `CreatedAt`.

### 5.4 Lista de cartas — **100% C++** (sem Graph)

**Não escreva ForEach / Create Widget / Add Child no Event Graph do `WBP_Mail`.**

Fluxo já ligado na classe pai:

1. `Open Mail` / notify 74 → `RefreshMailList` → HTTP `list_mail.php`
2. GI → `ApplyMailList` → preenche `MailEntries` + badge
3. `RebuildMailListUI()` (C++):
   - `VB_MailList->ClearChildren()`
   - `CreateWidget<UUmbraMailEntryWidget>` por carta
   - `SetupEntry` + `AddChildToVerticalBox`
4. Clique na linha → `SelectMail` (C++)

#### O que o Blueprint faz (mínimo)

| Passo | Ação |
|-------|------|
| 1 | Parent Class = `UmbraMailWidget` |
| 2 | Na hierarquia: `Scroll_MailList` → filho `VB_MailList` (**nomes exatos**) |
| 3 | Compile / Save — **zero nós de lista no Graph** |
| 4 | (Opcional) Class Defaults → `Mail Entry Row Class` = WBP custom |

#### Linha da lista (`UUmbraMailEntryWidget`) — como é construída

Hoje, se `Mail Entry Row Class` estiver vazio, o C++ monta em runtime (`EnsureDefaultLayout`):

```
UUmbraMailEntryWidget (UserWidget)
└── BTN_Select          ← UButton (RootWidget)
    └── HBox_Row        ← UHorizontalBox
        ├── Text_From   ← Automatic, MinDesiredWidth 110, ellipsis
        └── Text_Subject ← Fill, ellipsis (+ " [N]" se anexos)
```

- Clique em `BTN_Select` → `SelectMail(MailId)`.
- Cores: não lida = branco / assunto dourado; lida = cinza.
- **Para mudar o design:** crie `WBP_MailListRow` com Parent = `UmbraMailEntryWidget`, desenhe o visual (borda, ícone, fontes), nomes BindWidget `BTN_Select` / `Text_From` / `Text_Subject`, e em `WBP_Mail` → Class Defaults → **Mail Entry Row Class** = `WBP_MailListRow`. Com BindWidgets presentes, o C++ **não** cria o layout default.

#### Checklist lista

- [ ] `VB_MailList` existe com nome exato (BindWidget)
- [ ] Nenhum Graph ForEach / Get Mail Entries para montar UI
- [ ] Rebuild C++ após puxar as classes
- [ ] Abrir mail → linhas aparecem sozinhas; clique abre detalhe

### 5.5 Painel Enviar (compose) — vinculação

O C++ **não** tem BindWidget de EditableText. Para enviar:

1. Coloque `EditableTextBox` / `MultiLineEditableText` no Designer (nomes livres, ex. `ET_To`, `ET_Subject`, `ET_Body`).  
2. Antes de clicar `BTN_Send` (ou no OnTextChanged):  
   - **Set Compose To Name** / **Compose Subject** / **Compose Body** no Self (`UmbraMailWidget`).  
3. `BTN_Send` (nome exato) chama `SendMailToTarget` → `RequestSendMail` no GI.

Anexos no envio player→player (debitar inventário) exigem payload `attachments[]` no PHP; a UI de arrastar itens para anexo **não** está no C++ v1 — use Manager/admin para presentes com itens, ou estenda o Graph depois.

### 5.6 Claim / Delete de anexos

- Clique no slot da grade → seleciona (highlight).  
- `BTN_Claim` → resgata só o slot selecionado (`ClaimSelectedSlot`).  
- `BTN_ClaimAll` → todos os anexos não claimados.  
- `BTN_Delete` → **só** se não houver anexos pendentes (cliente + `delete_mail.php`).  
- Após claim OK: GI chama `LoadInventory` + reabre detalhe.  
- Tooltip: usa `item_name` / type / rarity do PHP (não fica em `Item #ID`).

### 5.7 Registro e abertura (HUD)

No `NativeConstruct`, o C++:

- `RegisterMailWidget(this)`  
- Bind `OnMailNotifyReceived` → refresh lista  
- Bind `OnMailListUpdated` → badge  
- Liga OnClicked dos `BTN_*`  
- Começa **Collapsed**

#### 5.7.1 Ligar `Icon_Mail` no HUD

No `WBP_PlayerHUD` (clique do ícone Mail):

```
OnClicked (Icon_Mail / BTN_Mail)
  → Get Game Instance (Umbra)
  → Branch: referência WBP_Mail válida?
       No  → Create Widget (WBP_Mail) → Add to Viewport (ZOrder alto) → guardar ref
       Yes → (já existe)
  → Open Mail   (Self do WBP_Mail)
```

`Open Mail` = Visible + `RefreshMailList`.  
`Close Mail` / `BTN_Close` = Collapsed (não precisa RemoveFromParent se reutilizar a mesma instância).

#### 5.7.2 Notify opcode 74

```
Opcode 74 → GI::OnMailNotifyReceived
  → MailWidget::HandleMailNotify → RefreshMailList
  → ShowSystemBroadcast("Nova carta de …")  (toast curto)
```

Atualize o badge no HUD com `Get Unread Count` após `OnMailListUpdated` se quiser ícone com número.

### 5.8 Checklist BindWidget — Mail

- [ ] Parent = `UmbraMailWidget`  
- [ ] `Text_From`, `Text_Subject`, `Text_Body` com nomes exatos  
- [ ] `BTN_Refresh`, `BTN_ClaimAll`, `BTN_Claim`, `BTN_Delete`, `BTN_Close` (C++ cria Claim/Refresh/ClaimAll se faltarem)  
- [ ] `MailAttach_0` … `MailAttach_4` parent `UmbraInventorySlotWidget`  
- [ ] `VB_MailList` com nome exato (lista montada em C++)  
- [ ] HUD chama `Open Mail` no ícone  
- [ ] Teste: carta admin com anexos → Select → Claim All → inventário  

---

## 6) Funções BlueprintCallable / Pure (referência)

### Broadcast (`UmbraBroadcastToastWidget`)

| Função | Notas |
|--------|--------|
| `Show Broadcast` | Normalmente só o GI chama; útil para debug no Graph |

### Mail (`UmbraMailWidget`)

| Função | Notas |
|--------|--------|
| `Open Mail` / `Close Mail` | Visibilidade + refresh |
| `Refresh Mail List` | HTTP list |
| `Select Mail (Mail Id)` | HTTP get + preenche textos/slots |
| `Claim Selected Slot` / `Claim All Attachments` | HTTP claim |
| `Delete Selected Mail` | HTTP delete |
| `Send Mail To Target` | usa Compose* |
| `Apply Mail List` / `Apply Mail Detail` | chamados pelo GI; raramente no Graph |
| `Get Unread Count` / `Get Mail Entries` | UI lista / badge |

### GameInstance (Category Mail / Broadcast)

| Função / Evento | Notas |
|-----------------|--------|
| `Show System Broadcast` | toast 73 ou fallback combat feedback |
| `Request Mail List` / `Detail` / `Claim` / `Send` / `Delete` | HTTP |
| `On Mail Notify Received` | multicast (MailId, FromName, Subject) |
| `On Mail List Updated` | após list OK |

---

## 7) Passo a passo resumido (ordem recomendada)

1. Rebuild C++ UE 5.6.1 (módulo com as classes novas).  
2. Criar `WBP_BroadcastToast` (§4) → colocar no HUD.  
3. Criar `WBP_MailAttachSlot` (opcional, cópia do inventory slot) → 5 instâncias em `WBP_Mail`.  
4. Criar `WBP_Mail` (§5) com todos os nomes BindWidget.  
5. Compose (§5.5) só se for enviar carta pelo jogador (EditableText → Set Compose*).  
6. Ligar `Icon_Mail` (§5.7.1).  
7. Testes §8.

---

## 8) Testes (Play-in-Editor)

1. GM Console: `broadcast message="Teste toast"` → chat global **e** toast 73.  
2. Manager → Mail: enviar carta a 1 player com 1–5 anexos → destinatário online recebe notify 74.  
3. Abrir `WBP_Mail` → selecionar carta → Claim All → inventário recebe itens.  
4. Manager → Mail “enviar a todos” → confirmação → `flush_mail_notify_queue`.  
5. Badge / `Text_UnreadBadge` atualiza após notify + refresh.

---

## 9) Checklist de regressão

- [ ] Chat 69–72 intacto  
- [ ] Guild 80–83 intacto  
- [ ] Combat feedback toast ainda funciona se BroadcastToast ausente (fallback)  
- [ ] Inventário após claim  
- [ ] Nomes BindWidget sem typo (`Broadcast_Text`, `MailAttach_0`…`4`, `BTN_*`)  
- [ ] Audit MySQL na aba Audit do Manager (fallback SQLite se API offline)

---

## 10) Referência rápida Manager

| Aba | Ação |
|-----|------|
| Guilds | listar / kick / transfer líder / dissolver |
| Auction | listar / force cancel / expire stale |
| Mail | 5 slots + envio único ou em massa |
| Audit | `list_admin_audit.php` (dual-write em `AuditLogService`) |

---

## 11) Problemas comuns

| Sintoma | Causa provável | Correção |
|---------|----------------|----------|
| Toast não aparece | WBP não está no Viewport / Parent errado | §4.4; Parent = `UmbraBroadcastToastWidget` |
| Toast sem texto | Nome ≠ `Broadcast_Text` | Rename no Designer |
| Mail abre vazio / sem linhas | Nome ≠ `VB_MailList` ou C++ antigo | Rebuild UE; checar log `VB_MailList ausente`; §5.4 |
| Botões não aparecem | Rodapé clipado / nome ≠ BindWidget | Rebuild; C++ move Refresh/ClaimAll para `HBox_DetailActions`; renomeie para `BTN_Refresh` / `BTN_ClaimAll` / `BTN_Claim` |
| Tooltip só `Item #ID` | Cache sem nome e JSON ignorado | Rebuild (PHP `item_name` sobrescreve fallback) |
| Claim não faz nada | Nenhum slot selecionado | Clique no anexo na grade, depois **Claim** |
| Delete com anexo | Bloqueio esperado | Resgate com Claim/Claim All antes; toast “Resgate os anexos…” |
| Anexos invisíveis | `Mail Attach Slot Class` None; ou HBox Fill sem grade; ou C++ antigo | Class Defaults = `WBP_InventorySlot`; rebuild; log `Grade de anexos criada` |
| Lista com texto misturado | From+Subject ambos Fill | Rebuild (layout C++ Auto+Fill+ellipsis) |
| Claim OK mas inventário velho | UI não escutou refresh | GI já chama `LoadInventory`; abra/feche inventário |
| Notify 74 sem efeito | Zone antiga / player offline | Rebuild zone; `notify_mail` / fila |
| API 401 | Token sem `player_id` | Selecionar personagem antes |
