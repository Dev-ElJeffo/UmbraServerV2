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

---

## 4) K2 — Quadros de alvo (jogador e NPC)

### Objetivo

- Quadro do alvo novo para jogador e NPC: cartão escuro com borda dourada, retrato, nome em Cinzel, "Nv N" colorido, X para limpar o alvo, HP com o texto por cima da barra.
- Números compactos: "499,8 mi / 500 mi · 99%" (antes "499836280/500000000").
- Sem quadrado branco: sem textura configurada o retrato some.
- NPC sem linha de MP.
- **ESC** sem janela aberta limpa o alvo primeiro; o próximo ESC abre o menu.

### Assets

| Asset | Path | Parent Class |
|-------|------|--------------|
| Quadro de jogador | `/Game/Widgets/UI/PlayerInteraction/PlayerSelection/WBP_TargetFramePlayer` | `UmbraSelectedPlayerInfoWidget` |
| Quadro de NPC | `/Game/Widgets/UI/PlayerInteraction/PlayerSelection/WBP_TargetFrameNpc` | `UmbraNpcTargetInfoWidget` |

- Os dois foram criados via MCP (sem Graph). `BP_ThirdPersonPlayerController` → componente **Player Selection** → `Target Widget Class` / `Npc Target Widget Class` já apontam para eles.
- `WBP_SelectedPlayerInfo` e `WBP_SelectedNpcInfo` ficaram sem uso (reserva). Podem ser apagados depois do QA.

### Binds

| Nome | Tipo | Função |
|------|------|--------|
| `Border_Window` | `UBorder` | Painel do quadro (posição pelo layout da HUD, elemento "Alvo") |
| `Image_ClassIcon` | `UImage` | Retrato (some sem textura) |
| `Text_PlayerName` / `Text_NpcName` | `UTextBlock` | Nome na cor da relação; "..." se não couber |
| `Text_TargetTag` | `UTextBlock` | "Morto" (vermelho) / "Fora de alcance" |
| `Text_Level` | `UTextBlock` | "Nv N" na cor da diferença de nível |
| `Btn_ClearTarget` | `UButton` | Limpa o alvo (jogador e NPC) |
| `Text_Subtitle` | `UTextBlock` | Jogador: `<Guilda>` (ou título). NPC: "Vendedor" / "Inimigo" / "NPC" |
| `Progress_HP` + `Text_HP` | barra + texto por cima | "atual / máx · %" compacto; "Morto" com HP 0 |
| `Progress_MP` + `Text_MP` | barra + texto (só jogador) | Igual ao HP, mais fina |
| `BuffBar_HBox` | `UHorizontalBox` | Debuffs primeiro, depois buffs |

### Class Defaults (você preenche)

- `WBP_TargetFramePlayer` → **Target|Portrait → Class Icons**: `ClassID` → textura do ícone da classe.
- `WBP_TargetFrameNpc` → **Target|Portrait**: `Npc Icon Hostile`, `Npc Icon Vendor`, `Npc Icon Friendly`.
- Os dois → **Target|Range**: `Out Of Range Distance` (padrão 3000 uu) e `Out Of Range Opacity` (0,55).

### Cores

| Item | Regra |
|------|-------|
| Nome do NPC | Atacável vermelho; vendedor/amigável verde; morto cinza |
| Nome do jogador | Grupo ou mesma guilda verde; demais branco; morto cinza |
| Nível | Diferença alvo − você: ≤ −10 cinza · −9..−3 verde · −2..+2 amarelo · +3..+4 laranja · ≥ +5 vermelho (NPC amigável: cinza) |

### O que o C++ já faz

- `UmbraUI::CompactNumber` / `ResourceBarText` / `LevelDifferenceColor` (`UI/Style/UmbraUIStyle`).
- Estado "Morto" e "Fora de alcance" (checado a cada 0,25 s; jogador fora do AOI conta como fora de alcance).
- X e ESC: `UUmbraPlayerSelectionComponent::ClearAllTargets` (jogador e NPC).

### QA (PIE, 2 players)

| # | Teste | Esperado |
|---|-------|----------|
| 27 | Selecionar o `dummy_upado` (500 mi de HP) | "500 mi / 500 mi · 100%" dentro da barra; sem MP; sem quadrado branco; nome vermelho; "Inimigo" |
| 28 | Bater no dummy | Barra e texto descem ("499,8 mi / 500 mi · 99%") |
| 29 | Selecionar um vendedor | Nome verde, "Vendedor", nível cinza |
| 30 | Selecionar o P2 (no grupo ou na mesma guilda) | Nome verde, `<Guilda>` na 2ª linha, barra de MP fina |
| 31 | NPC 6+ níveis acima / 10+ abaixo | "Nv" vermelho / cinza |
| 32 | Matar o alvo | "Morto" vermelho no nome e na barra; nome cinza |
| 33 | Afastar mais de 3000 uu | Quadro esmaecido com "Fora de alcance"; voltar restaura |
| 34 | **X** do quadro | Alvo limpo, quadro some |
| 35 | ESC sem janela / com o inventário aberto | Sem janela: limpa o alvo (2º ESC abre o menu). Com janela: fecha só a janela |
| 36 | Editar HUD com alvo selecionado | Moldura "Alvo" e pode ser movido |
| 37 | Alvo com debuff e buff | Debuffs primeiro na fila |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Quadro antigo aparece | `BP_ThirdPersonPlayerController` voltou a apontar para `WBP_SelectedPlayerInfo`/`WBP_SelectedNpcInfo` | Componente Player Selection → classes `WBP_TargetFrame*` |
| Sem ícones de buff no NPC | Classes de buff vazias no `WBP_TargetFrameNpc` | O C++ aplica as padrão do PlayerController; conferir `DefaultBuffIconWidgetClass` |
| Retrato não aparece | Sem textura em `Class Icons` / `Npc Icon *` | Esperado; preencher as texturas |
| Botão X branco grande | Estilo padrão do botão | Ajustar o estilo do `Btn_ClearTarget` no Designer (só visual) |

---

## 5) K5 — Chat

### Objetivo

- Abas com filtro de verdade: **Geral** (tudo), **Local**, **Global**, **Grupo**, **Guilda**, **Combate** (log de combate). A aba ativa fica dourada.
- Cores por canal, horário `[12:34]`, histórico de 300 linhas (antes crescia sem limite).
- **Canal de guilda real** (opcodes 120/121).
- Comandos, histórico de envio (↑/↓), sussurros e mensagens de sistema dentro do chat.
- "Novas mensagens ↓", fundo que some quando ocioso e alça para mudar o tamanho.

### Protocolo

| Opcode | Nome | Direção | Frame |
|--------|------|---------|-------|
| **120** | `ChatGuildMessage` | cliente → zone | `[120][fromId:u32][msgLen:u16][msg:utf8]` (igual aos 66–68) |
| **121** | `ChatGuildReceived` | zone → membros da guilda conectados | `[121][fromId:u32][nameLen:u16][name:utf8][msgLen:u16][msg:utf8]` (igual aos 69–71) |

- Remetente = player da conexão (o `fromId` do payload é ignorado, como nos outros canais); mesmos limites de tamanho e de mensagens por minuto.
- Membros vêm de `guild_members` (`ZoneServer.cpp`, `resolveGuildMembers`), com **cache de 30 s** por jogador; a cache é limpa quando a guilda muda (`GuildStateRefresh`).
- Sem guilda: erro "Voce nao esta em uma guilda" (opcode 72).
- Cliente: `UUmbraGameInstance::SendGuildChatViaWebSocket` e delegate `OnGuildChatReceived`; o 121 está na lista de forward do `NetMovementClient.cpp`.

### Comandos

| Comando | Efeito |
|---------|--------|
| `/l texto` (ou `/s`) | Local |
| `/g texto` | Global |
| `/p texto` | Grupo |
| `/gu texto` | Guilda |
| `/l`, `/g`, `/p`, `/gu` sem texto | Só troca o canal de envio ("Falando em: ...") |
| `/w Nome texto` | Sussurro para jogador conhecido (por perto, amigo ou do grupo) |
| `/r texto` | Responde ao último sussurro recebido |
| `/?` | Ajuda |

- **Enter** abre o campo; **Esc** fecha; **↑/↓** repete o que você enviou (últimas 20).
- **Clique numa linha** com autor: abre o campo com `/w Nome `.
- Na aba **Geral** o envio usa o último canal escolhido (padrão Local); nas abas de canal, o canal da aba.

### Cores

| Canal | Cor |
|-------|-----|
| Local | branco |
| Global | laranja |
| Grupo | azul |
| Guilda | verde |
| Sussurro (`[De X]` / `[Para X]`) | rosa |
| Sistema (EXP, nível, avisos) | amarelo; erro em vermelho |

### Editor — `/Game/Widgets/UI/Chat/WBP_ChatMain`

Já feito via MCP:

| Nome | Tipo | Onde | Função |
|------|------|------|--------|
| `BTN_TabAll` + `Text_TabAll` ("Geral") | Button | 1ª aba de `HorizontalBox_Tabs` | Aba Geral |
| `BTN_NewMessages` + `Text_NewMessages` | Button | fim do `VB_ChatContent`, centralizado, Collapsed | "Novas mensagens ↓" |
| `Border_ResizeGrip` + `Text_ResizeGrip` ("↗") | Border | fim de `HorizontalBox_Tabs` | Alça de redimensionar |
| `Combo_ChatChannel` | ComboBoxString | 1º item de `HorizontalBox_Tabs` | Seletor de canal (Geral, Local, Global, Grupo, Guilda, Combate). Com ele, os `BTN_Tab*` ficam escondidos (cabe em chat estreito) |

Binds usados (não renomear): `Border_ChatPanel`, `Scroll_ChatFeed`, `VB_ChatLines`, `ET_ChatInput`, `Border_ChatInput`, `BTN_SendChat`, `TXT_ChatInfo`, `CombatLogPanel`, `BTN_Tab*`, `BTN_CloseChat`.

Class Defaults (**Chat|Ajustes**): `Max Stored Lines` (300), `Show Timestamps`, `Chat Line Font Size` (11), `Idle Fade Seconds` (8), `Idle Background Opacity` (0,15).

### O que o C++ já faz

- Histórico único; a aba remonta a lista a partir dele; linhas antigas saem acima do limite.
- Eco do próprio envio ignorado (a linha entra na hora do envio).
- "Mensagem ... enviada" não aparece mais no chat; erros de envio/sussurro aparecem em vermelho.
- EXP ganha e nível novo como linhas de sistema.
- Tamanho escolhido na alça salvo em `UUmbraClientSettings::ChatPanelSize` e respeitado pelo layout da HUD.

### QA (PIE, 2 players)

| # | Teste | Esperado |
|---|-------|----------|
| 38 | P1 fala no Local; P2 no Global | Cada linha com a cor do canal e `[hh:mm]`; na aba Local só a do Local |
| 39 | Trocar de aba várias vezes | Lista remonta sem duplicar; aba ativa dourada; "Falando em: ..." muda |
| 40 | P1 e P2 na mesma guilda: aba Guilda → mensagem | P2 recebe em verde `[Guilda]`; jogador fora da guilda não recebe |
| 41 | Sem guilda: `/gu oi` | Erro vermelho "Voce nao esta em uma guilda" (ou "Você não está em uma guilda") |
| 42 | `/w <nome do P2> oi` / P2 responde `/r tudo bem` | `[Para P2]` / `[De P2]` em rosa nos dois lados |
| 43 | `/w NomeInexistente oi` / `/x` | Erro "não encontrado" / "Comando desconhecido" |
| 44 | ↑ / ↓ no campo | Volta as últimas mensagens enviadas |
| 45 | Clicar na linha de outro jogador | Campo abre com `/w Nome ` |
| 46 | Rolar para cima e receber mensagem | Não pula para o fim; aparece "Novas mensagens ↓"; clicar desce |
| 47 | Ficar 8 s sem mexer | Fundo do chat some (texto fica); passar o mouse volta |
| 48 | Arrastar a alça ↗ | Chat muda de tamanho; reabrir o jogo mantém |
| 49 | Matar um mob / subir de nível | `+N EXP` e "Você alcançou o nível N!" em amarelo |
| 50 | 350+ mensagens | Lista não passa de 300 linhas |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Chat de guilda não chega | `zone_server` antigo ou jogador fora de `guild_members` | Reiniciar a zone nova; conferir a tabela |
| Membro novo da guilda não recebe por até 30 s | Cache de membros | Esperado; `GuildStateRefresh` limpa a cache |
| `/w` não acha o jogador | Jogador longe, fora dos amigos e do grupo | Esperado (sussurro por nome para qualquer jogador fica para um lote futuro) |
| Alça não redimensiona | `Border_ResizeGrip` fora do chat ou `Border_ChatPanel` renomeado | Restaurar os nomes |

### Ajustes do QA do K5 (17/11/2026)

- **Canal por combo:** `Combo_ChatChannel` no lugar da fileira de abas (que não cabia no chat estreito). As abas continuam no WBP, escondidas.
- **Caracteres estranhos (chinês) no chat e no sussurro:** a conversão UTF-8 → texto lia além do fim da mensagem (`FString(Conv.Get())` sem o tamanho). Corrigido em 10 pontos do `UmbraGameInstance.cpp` (chat, sussurro, correio, loja) com `FString(Conv.Length(), Conv.Get())`.
- **Log de combate no padrão novo:** a aba **Combate** usa o mesmo feed do chat (o `CombatLogPanel` antigo fica escondido), com histórico próprio de 200 linhas (a luta não apaga a conversa):

| Tipo | Exemplo | Cor |
|------|---------|-----|
| Dano causado | `Você causou 1.234 de dano em Goblin (crítico)` | laranja claro |
| Dano recebido | `Goblin causou 80 de dano em você` / `Você recebeu 30 de dano do ambiente` | vermelho |
| Cura | `Você recuperou 50 de vida (contínua)` / `Fulano curou você em 120` / `Você curou Fulano em 90` | verde |
| Efeitos de status | `Você recebeu o efeito Atordoar` / `O efeito Veneno terminou em Goblin` | lilás / cinza |

- Nomes: jogador ou NPC (`UUmbraGameInstance::ResolveCombatantName`, mesma regra da tela de morte). Efeitos: você e o seu alvo atual (NPC ou jogador); o que já estava ativo quando o alvo apareceu não é anunciado.

| # | Teste | Esperado |
|---|-------|----------|
| 51 | Chat estreito (alça) | O combo de canal cabe; trocar no combo filtra como as abas |
| 52 | Mensagem e sussurro com acento ("ação, você") | Texto certo, sem caracteres estranhos no fim |
| 53 | Combate → bater num NPC / apanhar / curar | Linhas de dano causado, recebido e cura com nomes certos (NPC pelo nome, não "outro jogador") |
| 54 | Receber um debuff / aplicar um no alvo | "Você recebeu o efeito X" / "Alvo recebeu o efeito X"; ao acabar, "terminou" |
| 55 | Lutar bastante e voltar à Geral | Conversa continua lá (combate não apaga o chat) |

---

## 6) K7 — Painel de grupo

### Objetivo

- Painel de grupo novo, no padrão do quadro de alvo e mais compacto: uma linha por membro (196 px) com ★ do líder, nome, "Nv N", barra de HP com o texto por cima ("700 / 1.000 · 70%") e MP fina.
- **Todos** os buffs e debuffs de cada membro, em ícones de 18 px que quebram em várias linhas, com os debuffs primeiro. O tempo restante fica no tooltip (o ícone pequeno mostra só o preenchimento).
- Membro morto: "Morto" na barra, nome e barra em cinza.
- A linha do membro que é o seu alvo fica dourada. Clique na linha seleciona o membro (abre o quadro de alvo).
- Cabeçalho "Grupo (N)" com o botão **Sair**.
- Elemento **Grupo** no layout da HUD (padrão: canto superior esquerdo), movível no **Editar HUD**.

### Assets

| Asset | Path | Parent Class |
|-------|------|--------------|
| Painel | `/Game/Widgets/UI/PlayerInteraction/Party/WBP_PartyFrame` | `UmbraPartyWidget` |
| Linha do membro | `/Game/Widgets/UI/PlayerInteraction/Party/WBP_PartyMemberRow` | `UmbraPartyMemberSlotWidget` |

- Os dois foram criados via MCP, sem Graph. As linhas são criadas pelo C++ (`MemberRowClass`), sem o ForLoop antigo.
- `WBP_PartyPanel` (antigo) fica sem uso, como reserva. O PlayerController tenta `WBP_PartyFrame` primeiro e cai no antigo se ele não existir.

### Binds

**`WBP_PartyFrame`**

| Nome | Tipo | Função |
|------|------|--------|
| `Border_Window` | `UBorder` | Painel (posição pelo layout da HUD, elemento "Grupo") |
| `Text_PartyTitle` | `UTextBlock` | "Grupo (N)" |
| `Btn_LeaveParty` | `UButton` | Sai do grupo |
| `VB_PartyBox` | `UVerticalBox` | Recebe as linhas criadas pelo C++ (deixar vazio no Designer) |

**`WBP_PartyMemberRow`**

| Nome | Tipo | Função |
|------|------|--------|
| `Button_Slot` | `UButton` | Linha clicável; fundo e destaque do alvo aplicados pelo C++ |
| `Text_Leader` | `UTextBlock` | ★ dourado só no líder |
| `TextPlayerName` | `UTextBlock` | Nome (cinza quando morto) |
| `Text_Lvl` | `UTextBlock` | "Nv N" |
| `ProgressBar_HP` + `Text_HP` | barra + texto por cima | `ResourceBarText` compacto; "Morto" com HP 0 |
| `ProgressBar_MP` | `UProgressBar` | MP fina, sem texto (`Text_MP` é opcional) |
| `BuffBar_Wrap` | `UWrapBox` | Buffs e debuffs em várias linhas; sem ele, usa `BuffBar_HBox` (legado) |

### Class Defaults

Preenchidos pelo script `Saved/MCP_party_defaults.py`, que eu rodo pelo MCP depois do rebuild. Para conferir à mão:

| Asset | Campo | Valor |
|-------|-------|-------|
| `WBP_PartyFrame` | **Party → Member Row Class** | `WBP_PartyMemberRow` |
| `WBP_PartyFrame` e `WBP_PartyMemberRow` | **Party \| Buffs → Buff Icon / Buff Tooltip Widget Class** | os mesmos do `WBP_PartyPanel` (`WBP_BuffIcon` + tooltip) |
| `WBP_PartyMemberRow` | **Party \| Buffs → Buff Icon Size** | 18 (0 = tamanho do `WBP_BuffIcon`, 48) |
| `WBP_PartyMemberRow` | **Party \| Style → Apply Compact Style** | ligado |
| `BP_ThirdPersonPlayerController` | **Party Panel Widget Class** | `WBP_PartyFrame` |

### O que o C++ já faz

- Cria até 6 linhas a partir do `MemberRowClass`, preenche, colapsa as que sobram e atualiza o título.
- Ordena os efeitos com debuffs primeiro e reordena os ícones sem recriá-los (`UUmbraBuffIconWidget::SyncBuffBar`, que agora aceita qualquer painel, HorizontalBox ou WrapBox).
- Reduz o ícone (`UUmbraBuffIconWidget::ApplyIconSize`: SizeBox do `WBP_BuffIcon` e texto de duração escondido abaixo de 28 px).
- Destaca a linha do alvo atual (checado a cada 0,25 s) e aplica o estilo de lista da UI (`UmbraUI::ApplyListRowStyle`).
- Registra o elemento "Grupo" (`UUmbraHudLayoutSubsystem::Party`) e reaplica o layout quando o painel muda de tamanho.

### QA (PIE, 2 players)

| # | Teste | Esperado |
|---|-------|----------|
| 56 | P1 convida P2 e P2 aceita | Painel compacto no canto superior esquerdo: "Grupo (2)", duas linhas, ★ no líder |
| 57 | HP/MP dos dois | Barra de HP com "atual / máx · %" compacto por cima; MP fina abaixo |
| 58 | P2 recebe 3+ buffs e 1 debuff | Ícones pequenos, debuff primeiro; com muitos efeitos, quebram para a linha de baixo |
| 59 | Passar o mouse num ícone | Tooltip com nome e tempo restante |
| 60 | Clicar na linha do P2 | P2 vira o alvo (quadro de alvo abre) e a linha fica dourada |
| 61 | P2 morre | "Morto" na barra, nome e barra cinza |
| 62 | Editar HUD com grupo | Moldura "Grupo", pode mover; Salvar mantém após relogar |
| 63 | Botão **Sair** | Sai do grupo e o painel some (nos dois clients, se o grupo acabar) |
| 64 | Sem grupo, abrir o Editar HUD | "Grupo" não aparece (só com grupo ativo) |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Painel antigo aparece | `Party Panel Widget Class` do `BP_ThirdPersonPlayerController` aponta para `WBP_PartyPanel` | Apontar para `WBP_PartyFrame` |
| Painel só com o cabeçalho | `Member Row Class` vazio | Preencher com `WBP_PartyMemberRow` (ou me pedir para rodar o script) |
| Linhas sem ícones de buff | Classes de buff vazias | Copiar as do `WBP_PartyPanel` |
| Ícones grandes (48 px) | `Buff Icon Size` = 0 ou rebuild não feito | 18 e rebuild do módulo |
| Painel não move no Editar HUD | Fora de grupo ou rebuild antigo | Entrar num grupo; conferir o rebuild |
