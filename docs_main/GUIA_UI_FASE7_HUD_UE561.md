# GUIA — Fase 7: HUD, alvo, escala da interface e chat (UE 5.6.1)

Guia dos lotes da fase 7 ([plano](PLANO_UI_FASE7_HUD_UE561.md)). Cada lote acrescenta uma seção. O C++ resolve a lógica; o Blueprint só define parent class e layout.

## 0) Pré-requisitos

1. `zone_server` recompilado e reiniciado (o K1 mexe no protocolo).
2. Rebuild do módulo `UmbraEternumUE` (Live Coding não basta quando há header novo).
3. PIE com 2 clients (P1 e P2 no mesmo mapa).

---

## 1) K1 — Nomes nos pedidos e remover amigo

### Objetivo

- "Negociar com Player_32?", "Adicionar Player_32 à lista de amigos?" e o convite de grupo passam a mostrar o **nome do personagem**.
- "Remover Fulano da lista de amigos?" passa a mostrar o nome do amigo.
- Nada muda no Editor: não há WBP para editar neste lote.

### Protocolo

Campo novo **trailing** (frame antigo continua válido nos dois sentidos):

| Opcode | Nome | Frame |
|--------|------|-------|
| **11** | `PartyInviteReceived` | `[11][fromId:u32][toId:u32][partyId:u32][nameLen:u16][name:utf8]` |
| **21** | `TradeRequestReceived` | `[21][fromId:u32][toId:u32][nameLen:u16][name:utf8]` |
| **31** | `FriendRequestReceived` | `[31][fromId:u32][toId:u32][nameLen:u16][name:utf8]` |

- Little-endian; nome com até **32 bytes** UTF-8 (cortado sem partir caractere).
- Servidor: `appendSocialSenderName` em `src/zone/MovementProtocol.hpp`; nome vindo de `players_[id].characterName`.
- O `fromId` enviado agora é o **player da conexão** (`socialSenderUnlocked` em `MovementServer.hpp`). Se o cliente mandar outro id, o servidor usa o da conexão e grava um warning: "Pedido social com fromId X diferente do player da conexao Y".
- Cliente: `UUmbraGameInstance::ResolveSocialSenderName` lê o nome trailing; sem ele (servidor antigo), usa o cache de nomes próximos e, por último, "Jogador #id".

### O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Nome do remetente | `Core/UmbraGameInstance.cpp` (opcodes 11/21/31) | Preenche `PendingPartyInvites` / `PendingTradeRequests` / `PendingFriendRequests` e os delegates com o nome real |
| Remover amigo | `UI/UmbraFriendOptionsWidget.cpp` | **Remover** abre a confirmação vermelha padrão ("Remover X da lista de amigos?"); Confirmar/Enter chama `RemoveFriend`; Cancelar/ESC não faz nada |

> O delegate `OnRemoveFriendRequested` não é mais disparado, então o `WBP_FriendRemoveConfirm` antigo deixa de abrir. Não precisa apagar nada no BP.

### QA (PIE, 2 players)

| # | Teste | Esperado |
|---|-------|----------|
| 1 | P2 pede troca a P1 | P1 vê "Negociar com <nome do P2>?" |
| 2 | P2 pede amizade a P1 | "Adicionar <nome do P2> à lista de amigos?" |
| 3 | P2 convida P1 para o grupo | Convite com o nome do P2 |
| 4 | Aceitar a troca / a amizade / o grupo | Funciona como antes |
| 5 | Amigos → clicar no amigo → **Remover** | Confirmação vermelha "Remover <nome> da lista de amigos?"; **Cancelar** não remove; **Remover** remove e a lista atualiza |
| 6 | Servidor antigo (opcional) com cliente novo | Mostra o nome se o P2 estiver por perto; senão "Jogador #id" |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Ainda aparece "Player_32" | `zone_server` antigo ou cliente sem rebuild | Recompilar e reiniciar a zone; rebuild do UE |
| Aparece "Jogador #32" | Zone antiga e o P2 longe (sem nome no cache) | Atualizar a zone |
| Abrem dois diálogos ao remover amigo | BP ainda chama algo além do delegate | Avisar: verificar o Graph do `WBP_FriendList` |
| Warning "Pedido social com fromId..." no log | Cliente mandou id diferente do da conexão | Esperado só com cliente modificado; investigar se aparecer em jogo normal |

---

## 2) K4 — Escala da interface

### Objetivo

- Nova opção **Configurações → Vídeo → Escala da interface**, de **50% a 125%** (passo de 5%; faixa revista no K3). Muda HUD, janelas, tooltips e diálogos de uma vez.
- Atalhos: **Ctrl + =** (ou **Ctrl + +** do numérico) sobe 5%, **Ctrl + −** desce 5%, **Ctrl + 0** volta para 100%. Os atalhos salvam na hora.
- O "100%" tem o mesmo tamanho relativo em qualquer resolução (curva de DPI pelo lado menor da tela).
- Sem protocolo, SQL ou Manager.
- **Reiniciar o editor** depois do rebuild: a curva de DPI nova do `DefaultEngine.ini` só é lida na abertura.

### Como funciona

| Peça | Onde | Comportamento |
|------|------|---------------|
| Curva de DPI | `Config/DefaultEngine.ini` → `[/Script/Engine.UserInterfaceSettings]` | `ShortestSide`: 720 → 0,75; 1080 → 1,0; 1440 → 1,33; 2160 → 2,0 |
| Escala do jogador | `UUmbraClientSettings::UiScale` (slot `UmbraClientSettings`) | 0,5–1,25; save antigo carrega 1,0 |
| Aplicação | `UUmbraGameInstance::ApplyUiScale` | Grava em `UUserInterfaceSettings::ApplicationScale` (multiplicador global da UI) |
| Limite pela tela | `UUmbraGameInstance::GetEffectiveUiScale` | Se a área útil ficaria com menos de **640** unidades de altura, usa o máximo que cabe; o rótulo mostra "(limitado a N% pela resolução)" |
| Troca de resolução | `FViewport::ViewportResizedEvent` | Reaplica a escala (o limite muda com a janela) |
| Saída do PIE | `UUmbraGameInstance::Shutdown` | Volta `ApplicationScale` para 1,0 (não vaza para o editor) |
| Janelas abertas | `UUmbraWindowWidget` (já existia) | Posição salva como fração da tela; ao mudar a área útil, a janela é reposicionada dentro da tela |

### Editor — `/Game/Widgets/UI/Settings/WBP_Options`

Na página de vídeo (`Page_Display`), uma linha nova (montada via MCP, copiada da linha de sensibilidade do mouse):

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `Slider_UiScale` | `USlider` | Não | 0,5–1,25, passo 0,05; enquanto arrasta só o % muda; **ao soltar** aplica a prévia |
| `Txt_UiScale` | `UTextBlock` | Não | "100%" ou "125% (limitado a 110% pela resolução)" |
| `BTN_UiScaleReset` | `UButton` | Não | Volta para 100% (prévia) |

- **Aplicar** salva a escala. **Fechar sem Aplicar** desfaz a prévia. **Restaurar padrões** volta para 100% (prévia).

### O que o C++ já faz

- Lê, aplica e salva a escala; reaplica ao mudar a resolução; limita em tela pequena; desfaz a prévia ao fechar sem aplicar; atalhos Ctrl + = / − / 0 no `AUmbraEternumUEPlayerController`.

### QA (PIE)

| # | Teste | Esperado |
|---|-------|----------|
| 7 | Configurações → Vídeo → arrastar a escala para 125% e soltar | Toda a UI cresce ao soltar; rótulo "125%" |
| 8 | Fechar sem **Aplicar** | Volta para a escala anterior |
| 9 | 75% → **Aplicar** → fechar e reabrir o jogo | Continua em 75% |
| 10 | Janela 1280×720 (ou PIE pequeno) com 125% | Rótulo "(limitado a N% pela resolução)"; nenhuma janela maior que a tela |
| 11 | **Ctrl + =**, **Ctrl + −**, **Ctrl + 0** fora do chat | Sobe, desce 5% e volta para 100% |
| 12 | Janela de inventário aberta e mudar a escala | A janela continua visível dentro da tela |
| 13 | Parar o PIE | O editor volta ao tamanho normal |
| 14 | 1080p e 1440p (ou janela maximizada) em 100% | HUD com o mesmo tamanho relativo |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Slider não aparece | Linha ausente no `WBP_Options` ou nome errado | Conferir `Slider_UiScale` / `Txt_UiScale` / `BTN_UiScaleReset` |
| Escala muda e volta sozinha | Janela fechada sem **Aplicar** | Esperado; aplicar antes de fechar |
| Editor ficou grande depois do PIE | PIE encerrado à força (crash) | Reiniciar o editor |
| Atalho não faz nada | Foco num campo de texto (chat) ou janela modal com input só de UI | Clicar no jogo e repetir |
| UI enorme em 4K | Esperado no 100% (mesmo tamanho relativo do 1080p) | Baixar a escala |

---

## 3) K3 — HUD responsiva e modo "Editar HUD"

### Objetivo

- A HUD não se sobrepõe nem some ao mudar resolução ou escala: chat, barra de skills, HP/MP, menu, mapa, missões, alvo e barra de conjuração têm posição pelo **layout da HUD**.
- Botão **HUD** no fim do menu e **Configurações → Editar HUD**: abre o modo de edição para arrastar cada elemento e salvar.
- Escala da interface agora vai de **50% a 125%** (correção do K4).
- Corrigido: tracker de missões encolhendo e mapa sumindo depois de mudar a escala.

### Como funciona

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Layout da HUD | `UI/Hud/UmbraHudLayoutSubsystem.{h,cpp}` (um por jogador local) | Cada elemento é o painel principal de um widget de tela cheia; aplica âncora + deslocamento (unidades de UI), prende dentro da tela e reaplica quando a tela ou a escala mudam |
| Layout padrão | idem | Barra de baixo: chat (esq.) · HP/MP sobre a barra de skills (centro) · menu (dir.); barra de conjuração acima do HP/MP; mapa no canto superior direito com missões logo abaixo; alvo no topo-centro |
| Tela estreita | idem | Largura do chat e quebra do menu = o que sobra dos lados da barra de skills (260–460) |
| Posições salvas | `UUmbraClientSettings::HudLayout` | Só os elementos movidos; o resto segue o padrão. Posição antiga do minimapa (`MinimapPosX/Y`) é migrada uma vez |
| Modo edição | `UI/Hud/UmbraHudEditOverlayWidget.{h,cpp}` (C++, sem WBP) | Escurece o jogo, moldura com nome em cada elemento visível, arrastar; ao soltar prende na borda/canto mais próximo; vermelho = sobreposto; **Salvar** (Enter), **Cancelar** (Esc), **Restaurar padrão** |
| Janelas da HUD | `UUmbraWindowWidget::SetExternalPlacement` | Tracker de missões e quadros de alvo deixam de ter posição própria/arraste pela barra de título |
| Minimapa | `UUmbraMinimapWidget::SetManagedByHudLayout` | Sem `SetPositionInViewport` e sem arraste próprio |
| Janelas comuns | `UUmbraWindowWidget` | Ao mudar a tela, mantém a mesma fração da tela; tamanho fixo vem do slot (antes a geometria velha encolhia a janela) |

### Nomes que o C++ usa (não renomear)

| WBP | Painel do elemento |
|-----|--------------------|
| `WBP_PlayerHUD` | `Border_58` (HP/MP), `Border_MainPanel` (menu, com `WB_MicroMenu`), `WBP_SkillCastBar`, `WBP_QuestHudTracker` |
| `WBP_SkillBar1` | `Background_Border` |
| `WBP_ChatMain` | `Border_ChatPanel` |
| `WBP_Minimap` | `SB_Root` |
| `WBP_QuestHudTracker` / `WBP_SelectedPlayerInfo` / `WBP_SelectedNpcInfo` | painel da janela (`Border_Window` ou 1º filho do canvas raiz) |

Sem o nome, o C++ usa o primeiro filho do canvas raiz.

### Editor

1. **Ícones do menu (opcional):** `WBP_PlayerHUD` → **Class Defaults** → **Umbra|HUD|MicroMenu → Menu Icons**: chave = nome do botão (`BTN_Skillbook`, `BTN_QuestJournal`, `BTN_PlayerStore`, `BTN_Auction`, `BTN_StoragePanel`, `BTN_RefinementPanel`, `BTN_OpenEnchant`, `BTN_Craft`, `BTN_FriendList`, `BTN_BlockedPlayerList`, `BTN_Mailbox`, `BTN_GuildPanel`, `BTN_Settings`, `BTN_EditHud`), valor = textura. `Menu Icon Size` = 28. Sem ícone o botão continua com o texto.
2. `WBP_Options`: botão **Editar HUD** (`BTN_EditHud` + `Text_EditHud`) já criado na barra de baixo, ao lado de **Redefinir janelas**.

### O que o C++ já faz

- Registra os elementos ao montar a HUD; quadros de alvo se registram ao aparecer.
- Reaplica o layout a cada 0,1 s se a tela ou a escala mudaram (ticker, não depende do Tick dos WBPs).
- Modo edição: input só de UI enquanto edita; ao sair volta ao modo de jogo da HUD.

### QA (PIE)

| # | Teste | Esperado |
|---|-------|----------|
| 15 | Escala 50%, 75%, 100% e 125% (Configurações ou Ctrl + = / −) em 1080p | Nada se sobrepõe na barra de baixo; mapa e missões no canto superior direito; nada some |
| 16 | Janela 1280×720 (PIE pequeno) em 100% e 125% | Chat e menu estreitam; nada sai da tela |
| 17 | Mudar a escala várias vezes | Tracker de missões mantém o tamanho; mapa continua visível |
| 18 | Menu → **HUD** | Tela escurece, molduras com nome em cada elemento, barra Salvar / Cancelar / Restaurar padrão no topo |
| 19 | Arrastar o chat para o canto superior esquerdo e soltar | Prende no canto; **Salvar**; reabrir o jogo: chat continua lá |
| 20 | Arrastar o mapa para cima do chat | Molduras ficam vermelhas enquanto sobrepõem |
| 21 | Mover algo → **Cancelar** (ou Esc) | Tudo volta como estava |
| 22 | **Restaurar padrão** → **Salvar** | Layout padrão |
| 23 | Configurações → **Editar HUD** | Fecha as Configurações e abre o modo edição |
| 24 | Selecionar um alvo e editar | O quadro do alvo aparece como "Alvo" e pode ser movido (sem alvo ele não aparece na edição) |
| 25 | PIE com 2 clients: P1 edita e salva | P2 não muda (layout por jogador local; no PIE os dois usam o mesmo arquivo de configurações) |
| 26 | Ícone em `Menu Icons` para `BTN_Skillbook` | Botão mostra a imagem; tooltip "Livro de skills (K)" |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Elemento não aparece no modo edição | Está escondido (alvo sem seleção, barra de conjuração fora de cast) | Esperado; deixar visível e reabrir a edição |
| Elemento volta para o lugar do designer | Nome do painel mudou no WBP | Restaurar o nome (tabela acima) |
| Mapa ainda arrasta pela barra dele | Build antigo | Rebuild |
| Log "[UmbraHudLayout] Elemento X ignorado" | Painel fora de um CanvasPanel | O painel do elemento precisa ser filho direto do canvas raiz do WBP |
| HP/MP ainda grande | Compactação do HP/MP ficou para o K6 | — |
