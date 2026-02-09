# Botão Leave Party no WBP_PartyPanel

Procedimento para adicionar um botão que permite sair do grupo.

## Pré-requisito

- `UmbraGameInstance` já expõe `LeaveParty()` (BlueprintCallable).
- Ao sair com sucesso, `OnPartyStateLoaded` é disparado com `bInParty=false` e o painel é colapsado.

## Passo a passo no Blueprint

### 1. Adicionar o botão ao WBP_PartyPanel

1. Abra **WBP_PartyPanel** no editor.
2. Dentro do painel (ex.: abaixo do VB_PartyBox ou no canto), adicione um **Button**.
3. Nome sugerido: **BTN_LeaveParty**.
4. Texto do botão: "Sair do grupo" ou "Leave Party".

### 2. Conectar o evento OnClicked

**Opção A (mais simples):** O WBP_PartyPanel herda de UmbraPartyWidget que tem `Leave Party` (BlueprintCallable).

1. No **Event Graph** do WBP_PartyPanel, arraste o **BTN_LeaveParty**.
2. Escolha **OnClicked**.
3. Conecte a: **Leave Party** (chame direto no **self** – o próprio WBP_PartyPanel).

**Opção B:** Via GameInstance.

1. OnClicked → **Get Game Instance** → **Cast to UmbraGameInstance** → **Leave Party**.

### 3. Fluxo visual (Opção A)

```
[BTN_LeaveParty OnClicked] → Leave Party (target = self)
```

### 4. Opcional: confirmação

Se quiser um popup "Tem certeza?":

1. Use **Create Widget** para um WBP_ConfirmDialog (ou similar).
2. No botão OK do dialog, chame **Leave Party**.
3. No OnClicked do BTN_LeaveParty, mostre o dialog em vez de chamar Leave Party direto.

### 5. Comportamento esperado

- Usuário clica em **Leave Party**.
- `LeaveParty()` chama a API `leave_party.php`.
- Em sucesso: `OnPartyStateLoaded` com `bInParty=false` → painel fica **Collapsed**.
- O painel some e o jogador deixa de estar no grupo.

## Referência C++

- **Função:** `UUmbraGameInstance::LeaveParty()`
- **Blueprint:** Get Game Instance → Cast to UmbraGameInstance → **Leave Party**
- **API:** `POST /api/social/leave_party.php` (token no body)
