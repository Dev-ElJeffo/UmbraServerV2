# Procedimento: Sistema de Party (Grupo) completo

Guia para implementar o sistema de party funcional, com mínimo de Blueprint e máximo de C++.

---

## Visão geral

| Componente | Responsabilidade |
|------------|------------------|
| **WBP_ContextMenu** | Botão "Invite to Party" → Execute Context Action (InviteToParty) |
| **UmbraGameInstance** | SendPartyInvite, AcceptPartyInvite, LoadPartyState, LeaveParty |
| **APIs PHP** | send_party_invite, accept_party_invite, get_party_state, leave_party |
| **WBP_PartyPanel** | Herda UmbraPartyWidget, exibe até 6 membros |
| **WBP_PartyMemberSlot** | Herda UmbraPartyMemberSlotWidget, um slot por membro |

**Limite:** 6 jogadores por grupo.

---

## Pré-requisito: Schema do banco

A tabela `players` deve ter as colunas `health`, `max_health`, `mana`, `max_mana` para o `get_party_state.php` retornar HP/MP. Se não existirem, ajuste o SQL no endpoint.

---

## Parte 1: APIs PHP (já criadas)

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `send_party_invite.php` | POST | Envia convite (retorna party_id se inviter já está em grupo). Sem bloqueio por convite pendente. |
| `accept_party_invite.php` | POST | Aceita convite (cria/entra no grupo) |
| `get_party_state.php` | POST | Retorna party_id, leader_id, members[] com HP/MP |
| `leave_party.php` | POST | Sai do grupo |
| `get_pending_invites.php` | POST | Retorna trade/friend/duel pendentes; party_invites removido (convites só via WebSocket) |

---

## Parte 2: C++ (já implementado)

### Structs (UmbraDataStructures.h)
- **FUmbraPartyMember** – PlayerID, CharacterName, Level, CurrentHealth, MaxHealth, CurrentMana, MaxMana, ClassName, bIsLeader
- **FUmbraPartyState** – PartyID, LeaderID, bInParty, Members[]

### UmbraGameInstance
- `SendPartyInvite(TargetPlayerID, TargetPlayerName)`
- `AcceptPartyInvite(FromPlayerID)`
- `DeclinePartyInvite(FromPlayerID)`
- `LeaveParty()` – chama leave_party.php
- `LoadPartyState(PartyID)` – chama get_party_state.php, dispara OnPartyStateLoaded
- `GetCurrentPartyState()` – retorna CurrentPartyState
- `IsInParty()` – true se CurrentPartyID > 0

### Delegates
- `OnPartyInviteSent`, `OnPartyInviteReceived`, `OnPartyInviteFailed`
- `OnPartyMemberJoined`, `OnPartyMemberLeft`, `OnPartyDisbanded`
- `OnPartyStateLoaded(PartyState)` – disparado ao carregar estado do grupo

### Classes de UI
- **UUmbraPartyMemberSlotWidget** – SetMemberData, ClearSlot
- **UUmbraPartyWidget** – MemberSlotWidgets[], UpdatePartyUIFromState

---

## Parte 3: WBP_ContextMenu – Invite to Party

O botão **Invite to Party** deve chamar **Execute Context Action** com o enum **InviteToParty**.

**On Clicked (BTN_InviteToParty):**
```
Get Owning Player → Cast to UmbraPlayerController
    → Get Player Selection Component
        → Execute Context Action (Action = InviteToParty)
Remove From Parent  (ou Hide)
```

**Verificar:** O mapeamento está em `GUIA_WBP_CONTEXT_MENU_BOTOES.md` – invite = InviteToParty.

---

## Parte 4: WBP_PartyMemberSlot (Blueprint)

Crie um widget para **um** membro do grupo, herdando de **UmbraPartyMemberSlotWidget**.

### 4.1 Criar o widget
1. **Content Browser** → Botão direito → **User Interface** → **Widget Blueprint**
2. Nome: **WBP_PartyMemberSlot**
3. **Class Settings** → **Parent Class** → **Umbra Party Member Slot Widget**

### 4.2 Hierarquia (baseada no WBP_PartyPanel)
```
[WBP_PartyMemberSlot]
  └─ [Border] ou [Canvas Panel]
        └─ [Horizontal Box] / [Vertical Box]
              ├─ [Image] Image_Classicon (opcional)
              ├─ [Text Block] Text_Lvl     ← nome exato para BindWidgetOptional
              ├─ [Text Block] TextPlayerName
              ├─ [Text Block] Text_HP
              ├─ [Progress Bar] ProgressBar_HP
              ├─ [Text Block] Text_MP
              └─ [Progress Bar] ProgressBar_MP
```

**Nomes obrigatórios** para auto-bind: `TextPlayerName`, `Text_Lvl`, `Text_HP`, `Text_MP`, `ProgressBar_HP`, `ProgressBar_MP`.

---

## Parte 5: WBP_PartyPanel (Blueprint)

### 5.1 Configurar parent
1. Abra **WBP_PartyPanel**
2. **Class Settings** → **Parent Class** → **Umbra Party Widget**

### 5.2 Preencher MemberSlotWidgets (passo a passo)

`MemberSlotWidgets` é um array da classe C++ que o C++ usa para preencher os dados dos membros. Você precisa adicionar as 6 referências aos seus slots no **Event Construct**.

**Pré-requisito:** O **Vertical Box** do WBP_PartyPanel deve conter 6 instâncias de **WBP_PartyMemberSlot** (não Border genérico). Se ainda tiver Border_PartyPlayerInfo, substitua por WBP_PartyMemberSlot: arraste WBP_PartyMemberSlot da Paleta para dentro do Vertical Box, 6 vezes.

---

#### Método A: Usar Get Child At (slots já no Designer)

Use quando os 6 WBP_PartyMemberSlot já estão como filhos do Vertical Box no Designer.

**Passo a passo:**

1. Abra o **WBP_PartyPanel** e vá ao **Event Graph**.
2. Crie o evento **Event Construct** (se não existir).
3. Adicione um **For Loop**:
   - **First Index** = 0
   - **Last Index** = 5
   - **Loop Body** = índice do loop (0 a 5)

4. Dentro do Loop Body, construa a cadeia:

```
[For Loop] Index = 0..5
    │
    ├─► Get Child At
    │       • Target = [referência ao Vertical Box que contém os 6 slots]
    │       • Index = Index (do loop)
    │       • Return Value = filho na posição Index
    │
    ├─► Cast to Umbra Party Member Slot Widget
    │       • Object = Return Value do Get Child At
    │       • As Umbra Party Member Slot Widget = resultado do cast
    │
    ├─► Branch (Condition: Cast com sucesso?)
    │       • True:
    │       │   └─► Add (Array = MemberSlotWidgets, Item = As Umbra Party Member Slot Widget)
    │       └─► False: (opcional) Print String "Cast falhou no slot X"
```

**Como obter a referência ao Vertical Box:**
1. No **Designer**, selecione o **Vertical Box** que contém os 6 slots.
2. No painel **Details**, em **Designer**, marque **Is Variable** (ou Promove to Variable) para criar uma variável.
3. Nome sugerido: `VB_PartyMembers` ou `VerticalBox_Members`.
4. No Event Graph, arraste o pin dessa variável para obter **Get VB_PartyMembers** – use como Target do Get Child At.

**Alternativa:** Arraste o **Vertical Box** da **Hierarquia** (painel esquerdo) para o Event Graph – o UE cria um nó **Get a reference to [NomeDoWidget]**.

**Nodos necessários:**

| Nodo | Onde encontrar | Entrada |
|------|----------------|---------|
| For Loop | Flow Control | First=0, Last=5 |
| Get Child At | Widget | Target=Vertical Box, Index=Index |
| Cast to Umbra Party Member Slot Widget | UmbraPartyMemberSlotWidget | Object=Get Child At (Return Value) |
| Add (Array) | MemberSlotWidgets | Array=MemberSlotWidgets, Item=Cast (As Umbra Party Member Slot Widget) |

---

#### Método B: Create Widget (slots criados em runtime)

Use quando quiser criar os 6 slots dinamicamente (como no WBP_Trade com CreateTradeSlots).

**Passo a passo:**

1. No **Event Construct**, adicione um **For Loop** (First=0, Last=5).
2. Dentro do Loop Body:

```
[For Loop] Index = 0..5
    │
    ├─► Create Widget
    │       • Class = WBP_PartyMemberSlot
    │       • Owning Player = Get Owning Player (ou Get Player Controller 0)
    │       • Return Value = widget criado
    │
    ├─► Add Child to Vertical Box
    │       • Target = [Vertical Box do painel]
    │       • Content = Return Value do Create Widget
    │
    └─► Add (Array = MemberSlotWidgets, Item = Return Value do Create Widget)
```

**Ordem:** Create Widget → Add Child to Vertical Box → MemberSlotWidgets.Add(widget).

---

#### Resumo

| Método | Quando usar |
|--------|-------------|
| **A** (Get Child At) | Slots já estão no Designer como filhos do Vertical Box |
| **B** (Create Widget) | Slots são criados em runtime |

**Importante:** `MemberSlotWidgets` deve ter exatamente 6 elementos, na ordem em que os membros aparecem (índice 0 = primeiro membro, etc.).

### 5.3 Bind de OnPartyStateLoaded
No **Event Construct**:
```
Get Game Instance → Cast to UmbraGameInstance
    → Bind Event to On Party State Loaded
        → Event: Custom Event OnPartyStateLoaded_Handler (PartyState)
            → Update Party UI From State (Target = self, Party State = PartyState)
```

### 5.4 Mostrar/ocultar o painel

A lógica fica no **Custom Event OnPartyStateLoaded_Handler** (o mesmo que chama Update Party UI From State). O parâmetro `PartyState` tem o campo `bInParty`. Use um **Branch** para decidir visibilidade.

**Passo a passo:**

1. Abra o **Event Graph** do **WBP_PartyPanel**.
2. Localize o **Custom Event OnPartyStateLoaded_Handler** (o evento que recebe `PartyState`).
3. **Antes** de conectar ao **Update Party UI From State**, insira um **Branch**:
   - **Condition** = `PartyState` → arraste o pin e escolha **Get bInParty** (ou use `Split Struct` em PartyState e pegue o pin `bInParty`).

4. No **Branch**:
   - **True**:
     - Adicione **Set Visibility** (busque "Set Visibility" em Widget):
       - **Target** = Self (arraste o próprio widget ou use o pin de exec que vem do evento)
       - **In Visibility** = **Visible**
     - Depois conecte ao **Update Party UI From State** (Target = Self, Party State = PartyState).
   - **False**:
     - Adicione **Set Visibility**:
       - **Target** = Self
       - **In Visibility** = **Collapsed**

5. **Ordem da cadeia no pin True:**
   ```
   Branch (True) → Set Visibility (Visible) → Update Party UI From State
   ```

6. **Ordem da cadeia no pin False:**
   ```
   Branch (False) → Set Visibility (Collapsed)
   ```

**Nodos necessários:**

| Nodo | Onde encontrar | Entrada |
|------|----------------|---------|
| Branch | Flow Control | Condition = PartyState.bInParty |
| Set Visibility | Widget | Target = Self, In Visibility = Visible ou Collapsed |
| Update Party UI From State | UmbraPartyWidget | Target = Self, Party State = PartyState |

**Observação:** O painel precisa estar na árvore de widgets (Add to Viewport ou filho de um Canvas). Normalmente o HUD cria o WBP_PartyPanel no **BeginPlay** ou **Event Construct**, faz **Add to Viewport** (ou Add Child ao Canvas do HUD), e inicia com **Set Visibility = Collapsed**. Depois, o próprio painel controla visibilidade pelo OnPartyStateLoaded_Handler.

### 5.5 Criar e posicionar o painel no HUD (evitar painel não aparecer)

**Problema:** Se o painel for criado apenas em `OnCharacterInfoLoaded` ou em outro evento tardio, o jogador pode aceitar um convite de grupo (via `LoadPendingInvites` ao logar) **antes** do painel existir. Nesse caso, `OnPartyStateLoaded` dispara antes do painel estar criado e o painel nunca aparece.

**Solução recomendada:** Criar o WBP_PartyPanel no **Event Construct** do HUD (ou no **BeginPlay** do HUD), e não depender de `OnCharacterInfoLoaded`.

**Passo a passo no HUD:**

1. **Event Construct** (ou BeginPlay):
   - **Create Widget** (Class = WBP_PartyPanel)
   - **Add to Viewport** (ou Add Child ao Canvas do HUD)
   - **Set Visibility** = **Collapsed** (inicial)
   - Salvar referência em variável (ex.: `PartyPanelRef`)

2. **Imediatamente após** Add to Viewport:
   - **Get Game Instance** → **Cast to UmbraGameInstance** (salve em variável ou use o pin de retorno)
   - **Branch** – a condição NÃO vem de PartyState: use **Is In Party** do GameInstance:
     - Arraste o pin de saída do **Cast to UmbraGameInstance**
     - Escolha **Is In Party** (retorna bool) → conecte ao **Condition** do Branch
   - **True**: chamar **Load Party State** com o Party ID atual:
     - Do mesmo Cast: **Get Current Party State** → quebre o struct (split) e pegue **Party ID**
     - **Load Party State** (Party ID = valor obtido)
   - **False**: nada (o painel permanece collapsed até receber OnPartyStateLoaded com bInParty=true)

3. O painel já deve ter **Bind Event to On Party State Loaded** no seu Event Construct (5.3). Assim, quando `LoadPartyState` retornar, o painel receberá `OnPartyStateLoaded` e exibirá os membros.

**Resumo dos nós no HUD (sem depender de PartyState/OnPartyStateLoaded):**

| Objetivo | Nó | Onde encontrar |
|----------|-----|----------------|
| Condição do Branch | **Is In Party** | Cast to UmbraGameInstance → pin de saída → Is In Party |
| Party ID para LoadPartyState | **Get Current Party State** → **Party ID** | Cast to UmbraGameInstance → Get Current Party State → Split Struct → Party ID |

**Nota:** O C++ em `NetMovementClient.cpp` também chama `LoadPartyState` em `HandleWebSocketConnected` quando `IsInParty()`, para cobrir o caso em que o WebSocket conecta antes do painel estar criado.

### 5.6 Branch IsInParty: conectar AMBOS os pins (true e false)

**Importante:** O **false** do Branch **precisa** estar conectado ao fluxo; caso contrário, os **Bind Events** (OnPartyStateLoaded, etc.) não serão executados quando o jogador não está em grupo.

O fluxo deve ser:
- **true** → Load Party State (Party ID) → [continua para o próximo nó]
- **false** → [conecte ao MESMO nó que recebe o "then" do Load Party State]

Ambos os caminhos devem convergir para o próximo passo (ex.: DynamicCast ou outros bind events). Se o pin **false** ficar desconectado, quando `IsInParty()` for false o fluxo para e os binds/configurações posteriores não rodam.

---

## Parte 6: Fluxo completo

### 6.1 Convidar (Invite to Party)
1. Jogador A clica direito em B → menu abre
2. Clica em **Invite to Party**
3. Execute Context Action (InviteToParty) → GameInstance.HandlePlayerContextAction
4. GameInstance chama **SendPartyInvite**(TargetPlayerID, TargetPlayerName)
5. API persiste convite, retorna success
6. OnPartyInviteSent dispara
7. (Opcional) WebSocket notifica B em tempo real

### 6.2 Aceitar convite
1. B recebe convite (LoadPendingInvites ou WebSocket)
2. PendingPartyInvites.Add(FromPlayerID, FromPlayerName)
3. OnPartyInviteReceived dispara → UI mostra "X convidou você para o grupo"
4. B clica Aceitar → **AcceptPartyInvite**(FromPlayerID)
5. API cria/entra no grupo, retorna party_id, leader_id, member_ids
6. OnPartyMemberJoined dispara
7. **LoadPartyState**(PartyID) é chamado automaticamente
8. OnPartyStateLoaded dispara → WBP_PartyPanel atualiza

### 6.3 Exibir painel
- Ao receber **OnPartyStateLoaded** com bInParty=true: mostrar WBP_PartyPanel
- Ao receber **OnPartyDisbanded**: esconder WBP_PartyPanel

### 6.4 Sair do grupo
- Botão **Leave Party** (no WBP_PartyPanel ou em outro lugar):
  - Get Game Instance → **Leave Party**
  - OnPartyDisbanded dispara → esconder painel

---

## Parte 7: Atualização de HP/MP em tempo real (opcional)

O `get_party_state.php` retorna HP/MP atuais. Para atualizar em tempo real:
- **Polling:** Timer que chama LoadPartyState a cada X segundos (ex.: 5s)
- **Eventos:** Quando um membro toma dano/cura, o servidor pode notificar via WebSocket (futuro)

---

## Parte 8: Notificação de convite pendente

O **LoadPendingInvites(type="all")** já retorna `party_invites`. Ao logar, o GameInstance chama LoadPendingInvitesOnLogin. O Blueprint deve:
- Fazer bind de **On Party Invite Received**
- Mostrar notificação (ex.: "X convidou você para o grupo - Aceitar? Recusar?")

---

## Troubleshooting

### Erro "Você já está em um grupo" ao aceitar convite
- O jogador **já está em grupo no servidor** (ex.: sessão anterior) mas o cliente não sabe.
- O C++ chama `LoadPartyState(0)` automaticamente quando detecta esse erro – sincroniza com o servidor e exibe o painel.
- O `NetMovementClient` também chama `LoadPartyState(0)` ao conectar o WebSocket, para sincronizar o estado do grupo ao entrar na zona.

### Sai do grupo automaticamente ao desconectar
- **Servidor (zone server):** Quando o jogador desconecta do WebSocket (fecha o jogo, perde conexão, etc.), o **zone server** detecta e remove o jogador de `party_members` no banco. O WebSocket permanece ativo para outros jogadores.
- **Cliente:** Se o cliente ainda estiver rodando ao desconectar (ex.: voltar ao menu), tenta notificar via `leave_party.php`. O servidor é a fonte confiável.
- Ao fazer Logout, o estado de grupo também é limpo.

### Grupo criado (logs mostram "Entrou no grupo X") mas WBP_PartyPanel não aparece
1. **OBRIGATÓRIO: WBP_PartyPanel deve herdar de UmbraPartyWidget** – No Blueprint, abra WBP_PartyPanel → Class Settings → Parent Class = **UmbraPartyWidget**.
2. **Painel para TODOS os clientes:** Duas fontes criam o painel: (a) **PlayerController** no `BeginPlay`; (b) **GameInstance** em `EnsurePartyPanelToViewport()` ao conectar o WebSocket (fallback para Client 2). O caminho padrão é `/Game/Widgets/UI/PlayerInteraction/Party/WBP_PartyPanel`. Se o HUD também criar o painel, **remova** a criação do HUD para evitar duplicatas.
3. **Logs:** `[UmbraPlayerController] ✅ WBP_PartyPanel criado` e `[UmbraPartyWidget] OnPartyStateLoaded` aparecem quando funciona.
4. **MemberSlotWidgets** – O WBP_PartyPanel cria 6 slots no Event Construct (ForLoop 0–5). O C++ agenda atualização em 0,15s para garantir que os slots existam antes de preencher.
5. **Slot flutuando fora do painel** – Ver `PROCEDIMENTO_WBP_PARTYPANEL_SLOT_FORA_LAYOUT.md` para corrigir membro (ex.: TheKillZone) aparecendo fora do widget.
6. **Botão Leave Party** – Ver `PROCEDIMENTO_WBP_PARTYPANEL_BOTAO_LEAVE_PARTY.md`.

### "Branch Is In Party" – de onde vem a condição?
- **Não** vem de PartyState nem de OnPartyStateLoaded.
- É uma função no **UmbraGameInstance**: `Is In Party` (BlueprintCallable).
- Fluxo: **Get Game Instance** → **Cast to UmbraGameInstance** → arraste o pin e escolha **Is In Party** (retorna bool).
- Para obter o Party ID: **Get Current Party State** → quebre o struct e pegue **Party ID**.

---

## Checklist final

| Item | Verificação |
|------|-------------|
| WBP_ContextMenu: BTN_InviteToParty → Execute Context Action (InviteToParty) | ⬜ |
| WBP_PartyMemberSlot criado, herda UmbraPartyMemberSlotWidget | ⬜ |
| WBP_PartyMemberSlot: TextPlayerName, Text_Lvl, Text_HP, Text_MP, ProgressBar_HP, ProgressBar_MP | ⬜ |
| WBP_PartyPanel herda UmbraPartyWidget | ⬜ |
| WBP_PartyPanel: MemberSlotWidgets preenchido com 6 slots | ⬜ |
| Bind On Party State Loaded → Update Party UI From State | ⬜ |
| Mostrar WBP_PartyPanel quando IsInParty / OnPartyStateLoaded | ⬜ |
| Botão Leave Party → Leave Party (GameInstance) | ⬜ |
| Notificação de convite → AcceptPartyInvite / DeclinePartyInvite | ⬜ |
| Tabelas parties, party_members, party_invites no banco | ⬜ |

---

## Referência: Arquivos modificados/criados

| Arquivo | Alteração |
|---------|-----------|
| `www/umbra_api/api/social/get_party_state.php` | Novo |
| `www/umbra_api/api/social/leave_party.php` | Novo |
| `www/umbra_api/api/social/send_party_invite.php` | Retorna party_id |
| `UmbraDataStructures.h` | FUmbraPartyMember, FUmbraPartyState |
| `UmbraGameInstance.h/cpp` | LoadPartyState, LeaveParty (API), OnPartyStateLoaded |
| `UmbraPartyMemberSlotWidget.h/cpp` | Novo |
| `UmbraPartyWidget.h/cpp` | Novo |
