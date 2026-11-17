# GUIA — Fase 6: estilo central, fonte de título e tooltips novos (UE 5.6.1)

Plano: [PLANO_UI_FASE6_POLIMENTO_UE561.md](PLANO_UI_FASE6_POLIMENTO_UE561.md).

## 1) Fonte Cinzel (títulos) — Editor

Arquivos: `UmbraEternumUE/ThirdParty/Fonts/Cinzel/` (`Cinzel-Regular.ttf`, `Cinzel-Bold.ttf`, `OFL.txt`). A licença OFL permite uso e distribuição no jogo; mantenha o `OFL.txt` junto.

Já importado pelo MCP:

- `/Game/Widgets/Fonts/Cinzel-Bold` (**Font Face**) — é o que o C++ usa: monta uma fonte de runtime com ela.
- `/Game/Widgets/Fonts/Font_Cinzel` (**Font**, vazio) — o Python do Editor não consegue preencher o composite. O C++ ignora fonte vazia. Opcional: abra o asset, em **Default Font Family** adicione uma entrada apontando para `Cinzel-Bold`, e ele passa a ser usado no lugar da fonte de runtime.
- `Config/DefaultGame.ini`: `/Game/Widgets/Fonts` em **Directories to Always Cook**, para a face entrar no build empacotado (o C++ a referencia só por caminho).

A fonte é carregada uma vez por sessão: depois de trocar a fonte, reinicie o Editor. Sem a face, os títulos ficam em Roboto Bold, sem erro.

Para outro caminho ou nome: crie `DA_UmbraUIStyle` em `/Game/Widgets/Style` (**Miscellaneous → Data Asset → UmbraUIStyle**) e aponte **Title Font**. O mesmo asset ajusta cores e tamanhos (`Umbra|Style`).

## 2) Tooltips novos

| WBP | Estrutura | O C++ monta |
|-----|-----------|-------------|
| `WBP_SkillTooltip` | `CanvasPanel_35` → `Border_44` (moldura, 320 de largura) → `VB_Content` | Ícone + nome + "Rank · tipo · elemento"; grade com custo, recarga, conjuração, alcance, duração e alvo; descrição; efeitos em verde; poder e escala no rodapé |
| `WBP_ItemTooltipRefined` | `CanvasPanel_32` → `Border_BG` (moldura, 340) → `VB_Root` → `HB_Header` (`Btn_Close`) + `VB_Content` | Nome na cor da raridade; raridade · tipo · subtipo; nível · slot; classes (vermelho se não pode); stats ≠ 0 com bônus em verde; cristais; descrição; valor, durabilidade e pilha; tags Negociável/Refinável |

- Não ponha nada dentro de `VB_Content`: o C++ limpa e remonta a cada item ou skill.
- A altura cresce com o conteúdo; para mudar a largura, mude o **Size X** do slot da moldura.
- O X do tooltip de item (modo "Item Info") fecha pelo C++; o Graph do WBP ficou sem nós de lógica.

## 2.1) Ficha do personagem (J4)

- Cada linha de stat/perfil é `HorizontalBox` com ícone (opcional) + `Text_*`. O C++ cria `Text_*_Label` (rótulo pt-BR) antes do `Text_*` e escreve só o valor nele. Para mudar um rótulo, mude o NSLOCTEXT em `UmbraCharacterInfoWidget::SetupRowLabels`.
- Não troque o pai de um `Text_*` por algo que não seja `HorizontalBox`: sem ele o rótulo não é criado (o valor continua aparecendo).
- `Text_Total_EXP` fica escondido pelo C++ (EXP em uma linha só no `Text_EXP`).
- Os slots de equipamento seguem pelo Blueprint (`RefreshEquipmentSlots` → `CreateEquipmentSlots`/`UpdateEquipmentSlots`); o X fecha pela base C++ (o Graph do X foi removido).

## 2.2) Diálogo de quantidade (J2)

- Asset: `/Game/Widgets/UI/Dialogs/WBP_AmountDialog`, **Parent Class** `UmbraAmountDialogWidget` (depois do rebuild; antes disso ele fica em `UmbraWindowWidget`).
- Binds (nomes exatos): `Border_Window`, `Border_TitleBar`, `Text_WindowTitle`, `Btn_WindowClose`, `Text_Message`, `Input_Amount` (Editable Text Box), `Btn_Minus`, `Btn_Plus`, `Slider_Amount`, `Text_Range`, `Btn_Min`, `Btn_Half`, `Btn_Max`, `BTN_Cancel`, `BTN_Confirm`. Graph vazio.
- Título, mensagem e limite vêm do C++ conforme quem abriu: Troca (+/− ouro), Armazém (depositar/sacar ouro) e Shift + arrastar pilha no inventário (dividir).
- Enter confirma, ESC cancela; Confirmar fica desligado com valor inválido; digitar acima do máximo vira o máximo.

## 2.3) Guilda, amigos, bloqueados, sussurro e denúncia (J7)

**Guilda (`/Game/Widgets/UI/Guild/WBP_GuildPanel`)**

- Binds novos (já criados pelo MCP): `ProgressBar_GuildXP` (em `HB_GuildXP`), `Btn_TabMembers`, `Btn_TabManage` (em `HB_GuildTabs`), `ETB_AddGuildXPAmount` (em `HB_AddGuildXP`).
- O C++ mostra só uma seção por vez: aba Membros = `VBox_GuildMembers`; aba Gerenciar = `Border_756`/`VBox_GuildManagementPanel`. No Designer as duas aparecem (o C++ não roda no preview).
- Sem guilda: o C++ esconde `HBox_GuildHeader`, `HBox_GuildHeader_1`, `HB_GuildXP` e `HB_GuildTabs` e mostra só "Criar guilda".
- A barra usa a curva de `www/umbra_api/helpers/guild_helper.php` (nível L começa em (L−1)² × 1000). Se a curva mudar no PHP, mudar `GuildXpForLevelStart` em `UmbraGuildWidget.cpp`.

**Confirmação (`WBP_AmountDialog`)**: o mesmo diálogo da quantidade, em modo confirmação. O C++ esconde `HB_Input`, `Slider_Amount`, `Text_Range` e `HB_Shortcuts` e troca o texto de `Text_Confirm`. Não renomeie esses quatro.

**Amigos e bloqueados — passo depois do rebuild** (o MCP faz, se pedir):

1. `WBP_FriendList` → **Class Settings → Parent Class** = `UmbraFriendListWidget`.
2. `WBP_BlockedPlayersList` → **Parent Class** = `UmbraBlockedListWidget`.
3. Apagar todos os nós do Event Graph dos dois (a lógica agora é C++; se ficar, a lista monta em dobro).
4. Binds usados: `Text_FriendAmount`, `VBox_Friends`, `BTN_PendingInvites`, `TXT_Pending_Invites` / `Text_BlockedAmount`, `VB_Slots`.
5. As classes de linha e de opções vêm por padrão de `/Game/Widgets/UI/PlayerInteraction/...` (`WBP_FriendSlot`, `WBP_FriendOptions`, `WBP_BlockedPlayerSlot`, `WBP_BlockedPlayerOptions`). Para trocar: **Class Defaults → Umbra|Social**.

**Sussurro (`WBP_Whisper`)**: Graph vazio; Enviar, Enter e X são C++. O botão "Fechar" saiu.

**Denúncia (`WBP_ReportPlayer`)**: `Text_ReasonCount` abaixo do motivo; Enviar fica desligado até o mínimo (`MinReasonLength`, padrão 10).

## 2.4) Criação, refinamento e encantamento (J8)

**Criação (`/Game/Widgets/UI/Crafting/WBP_CraftingMenu`)**

- Binds novos (criados pelo MCP): `Btn_CountMinus`, `Btn_CountPlus` (em `HorizontalBox_127`, com `Text_CraftCount`). O `Slider_CraftCount` foi removido; se voltar a existir no WBP, continua funcionando junto.
- `Box_RecipeList` e `Box_Ingredienthints` são montados pelo C++: não ponha nada dentro.
- O combo mostra rótulos em pt-BR; o C++ converte para a chave do servidor (`material`, `consumable`, `equipment`).

**Refinamento (`/Game/Widgets/UI/Refinement/WBP_RefinementWindow`)**

- Bind novo: `VB_StatsCompare` (VerticalBox em `VerticalBox_2`). Com ele, `CurrentStatsText`/`NextLevelStatsText` ficam escondidos; não os apague (são `BindWidget` obrigatórios).
- `ItemIcon`/`MaterialIcon` foram removidos do WBP (eram o fallback dos quadrados brancos); os `InventorySlot_*` mostram o ícone.

**Encantamento (`/Game/Widgets/UI/Enchant/WBP_ItemEnchant`)**

- `CancelButton` agora é **Limpar** (ligado por nome no C++). `VerticalBox_6` (extrator) é escondido pelo C++ quando o equipamento não tem afixo: não renomeie.
- Status só em `Text_Info` (rodapé); `Enchant_Status` fica escondido quando `Text_Info` existe.

## 2.5) Livro de skills (J9)

**Livro (`/Game/Widgets/UI/Skills/WBP_SkillBook`)**

- O Event Graph ficou vazio (106 nós apagados). As funções de Blueprint antigas (`PopulateSkillList`, `UpdateDetailPanel`, `UpdateHeader`) saíram no J10.
- A tecla K abre este livro pelo C++: `BP_ThirdPersonCharacter` → **Class Defaults → Open Skill Book Action** = `IA_OpenPlayerSkillBook` (J10). Antes, um Graph no personagem abria o `WBP_SkillBook1` antigo.
- Binds (nomes exatos, já existentes): `Title_Class`, `SkillPoints_Text`, `Skills_VBox`, `Skill_Name_Text_1`, `Skill_Icon_Large_1`, `Skill_Type_Text_1`, `Skill_Element_Text_1`, `SkillBook_Description`, `SkillBook_Effects`, `Power_Text_1`, `BonusPower_Text_1`, `Cost_Text_1`, `Duration_Text_1`, `Range_Text_1`, `Cooldown_Text_1`, `Radius_Text_1`, `Cast_Text_1`, `CanCrit_Text_1`, `Reqtarget_Text_1`, `MovCast_text_1`, `PVPModifier_Text_1`, `Scaling_Text_1`, `CurrentRank_Text_1`, `Action_Info_1`, `RankStars_HBox_1`, `Action_Button_1`, `Text_Action_Button_1`.
- Cada linha de stat é `Border > HB (ícone + texto)`. O C++ esconde o **Border** inteiro quando o valor não se aplica, então não tire o texto de dentro do HB.
- `Skills_VBox` é montado pelo C++: não ponha nada dentro.
- A classe da linha vem de **Class Defaults → SkillBook → Entry Class** (padrão `WBP_SkillBookEntry`).
- `Skill_Description_Text` (texto cru, duplicado) e `RankStars_HBox_1` (estrelas) são escondidos pelo C++.
- O X é o `Btn_Close`, ligado pela base `UmbraWindowWidget`.

**Linha (`WBP_SkillBookEntry`)**: Graph vazio. Usa `BTN_SkillSelect`, `Skill_Icon`, `Skill_Name_Text`, `Skill_Type_Text`, `Level_Required_Text` e `Rank_Text`. As estrelas e os ícones de status são escondidos; a situação vira uma tag de texto.

## 2.6) Morte, desconexão, missões e limpeza final (J10)

### Passo depois do rebuild (obrigatório)

As classes `UmbraDeathScreenWidget` e `UmbraServerDisconnectWidget` só existem depois do rebuild. Em seguida (o MCP faz, se pedir):

1. `/Game/Widgets/UI/DeathScreen/WBP_DeathScreen` → **Class Settings → Parent Class** = `UmbraDeathScreenWidget` → **Compile** → **Save**.
2. `/Game/Widgets/HUD/WBP_ServerDisconnect` → **Parent Class** = `UmbraServerDisconnectWidget` → **Compile** → **Save**.

Sem o reparent, o botão **Renascer** não faz nada (o Graph antigo já foi apagado) e a desconexão abre o diálogo sem texto.

### Tela de morte (`WBP_DeathScreen`)

- Binds: `Text_DeathTitle`, `Text_KilledBy`, `BTN_Respawn`, `Text_Respawn`. Graph vazio.
- Fundo `Border_0` em tela cheia, vermelho-escuro com 78% de opacidade.
- **Class Defaults → Umbra|Death**: `Respawn Zone Id` (1), `Respawn Spawn Key` (`Lvl_Tutorial_`, o mesmo do Graph antigo; sem linha com essa chave em `spawn_points`, o servidor usa o `is_default` da zona), `Respawn Retry Seconds` (5) e `Title Font Size` (44).

### Desconexão (`WBP_ServerDisconnect`)

- Binds: `Text_Title`, `Text_Message`, `BTN_OK`, `Text_OK`. Graph, função `SetupMessage` e variáveis apagados.
- Quem abre é o C++ (`ShowServerDisconnectDialog` virou `BlueprintNativeEvent`). O evento do `BP_UmbraGameInstance` foi apagado; se voltar a existir lá, ele substitui o C++.
- Para trocar o WBP: `BP_UmbraGameInstance` → **Class Defaults → Connection|UI → Server Disconnect Widget Class**.

### Diário de missões (`/Game/Widgets/UI/Quest/WBP_QuestJournal`)

- Abas no topo da lista: `BTN_Active`/`Text_TabActive` e `BTN_Completed`/`Text_TabCompleted`. O C++ escreve "Ativas (N)" e "Concluídas (N)".
- Botões no detalhe: `BTN_TurnIn`/`Text_TurnIn` (novo, **Entregar**) e `BTN_Abandon`/`Text_Abandon` (**Abandonar**, em vermelho, com confirmação).
- O X fecha pelo C++ (o `Remove from Parent` do Graph foi apagado). A janela tem 900x700.
- **Tecla J** abre e fecha o diário: `/Game/Input/Actions/IA_OpenQuestJournal` (novo), mapeado em `IMC_Default`, e `BP_ThirdPersonCharacter` → **Class Defaults → Open Quest Journal Action**. O C++ alterna (`ToggleQuestJournal`); antes do rebuild a J só abre.
- Linha (`WBP_QuestEntry`): `SB_Row` (280 de largura) → `Btn_Select` → `VB_EntryText` → `Text_Title` + `Text_Objective`. A linha inteira é o botão.

### Escolha de recompensa (`/Game/Widgets/UI/Quest/WBP_QuestRewardPicker`)

- Montado do zero (o WBP estava vazio). Binds: `Border_Window`, `Border_TitleBar`, `Text_WindowTitle`, `Btn_WindowClose`, `Text_Title`, `VBox_Choices` (dentro de `SB_Choices` → `Scroll_Choices`, altura máxima 360), `Text_SelectedItem`, `BTN_ConfirmReward` (**Receber recompensa**).
- `VBox_Choices` é montado pelo C++: não ponha nada dentro.

### Limpeza

- Apagados: `WBP_OptionsPopup`, `WBP_TradeGoldAmount`, `WBP_TradeRemoveGold`, `WBP_GoldDepositManager`, `WBP_GoldWithdrawManager` e `WBP_SplitInput`. O diálogo padrão (`WBP_AmountDialog`) cobre tudo.
- Apagados também `WBP_SkillBook1` e `WBP_SkillDetails` (livro antigo, sem referência e sem compilar). No `WBP_SkillBookEntry` saiu a função BP `SetSkillData` (120 nós, ninguém chamava; deixava o asset em erro de compilação) e as variáveis `SkillID`/`MyGameInstance`. Todos os 97 WBPs de `/Game/Widgets` compilam sem erro.
- `BP_ThirdPersonCharacter`: saiu a cadeia do F10, que abria o `WBP_OptionsPopup` junto com o menu C++.
- `WBP_InventorySlot` → função `OnDragDetected`: saiu o ramo do Shift (o C++ trata o Shift antes; o ramo antigo abria o `WBP_SplitInput` e escrevia "Item não pode ser dividido" na tela).
- Textos de designer em inglês ou "Bloco de Texto": 199 textos em 36 WBPs passaram para pt-BR (login, cadastro, seleção e criação de personagem, menus de jogador e amigo, chat, level up, chances de CC, livro de skills, troca, sussurro...). Nomes de classe (Barbarian, Templar...) ficaram como estão.

## 2.7) Correções do QA geral (ficha, excluir item, armazém, leilão, guilda e livro)

Saídas do primeiro QA geral (17/11/2026). As mudanças nos WBPs já foram feitas via MCP; a sua parte é o rebuild e um reparent.

### Passo depois do rebuild (obrigatório)

1. **Rebuild** do módulo (classe nova `UmbraDeleteItemWidget`; headers da ficha, do inventário e do armazém mudaram).
2. Abrir `/Game/Widgets/UI/Inventory/WBP_DeleteItem` → **File → Reparent Blueprint** → `UmbraDeleteItemWidget` → **Compile** → **Save**.
3. Reiniciar o editor antes do PIE (os WBPs foram editados via MCP; evita o crash de "world leak").

### Ficha (`/Game/Widgets/UI/CharacterInfo/WBP_CharacterInfo1`)

- **Valores sobrepostos:** a função BP **Update Character Info Display** reescrevia os textos no formato antigo ("Strength: 8/8", "Level: 4", "Class: DarkMage") depois de cada recarga (ao aplicar pontos, por exemplo). Ela ficou só com a chamada a `UpdateEquipmentSlots`; os textos são do C++.
- **Segurar + / −:** clicar soma 1 ponto; **segurando**, repete depois de 0,35 s, a cada 0,06 s, e depois de 20 repetições soma 5 por vez. Para ao soltar, ao sair do botão ou quando os pontos livres acabam. Ajustável em **Class Defaults → Umbra|CharacterInfo** (`Stat Repeat Delay`, `Stat Repeat Interval`).
- Não existe limite de 10 pontos por envio (nem no C++ nem no PHP); o "+" só desliga quando os pontos livres acabam.

### Excluir item (`WBP_DeleteItem`)

- Toda a lógica saiu do Graph (63 nós, funções `CreateDeleteSlot`, `OnDrop`, `ResetTrashSlot` e variáveis) e foi para o C++ (`UUmbraDeleteItemWidget`).
- Binds (nomes já existentes no WBP): `Text_Item_Name`, `Text_Info`, `Grid_DeleteSlots`, `BTN_Delete_Item`, `BTN_Cancel_Delete`, `Border_TitleBar`.
- `Grid_DeleteSlots` é montado pelo C++ (quadro com ícone e quantidade): não ponha nada dentro.
- `WBP_TrashSlot` ficou sem uso (nenhuma referência). Pode ser apagado quando quiser.

### Armazém (`WBP_Storage`)

- O texto de ouro mostra o **ouro guardado** ("N ouro guardado"), e "Itens: N/100" conta os itens do armazém. Os 3 nós `Set Text` do Graph que escreviam no `Text_Gold_Info` foram removidos.

### Leilão, guilda e livro (layout via MCP)

- `WBP_AuctionHouse`: o preço do slot da aba **Vender** fica escondido (o preço está no campo "Preço de venda"); `SizeBox_0` com 72×72.
- `WBP_GuildMemberRow`: `Border_40` passou de 850×100 fixo para a largura da lista (âncora esticada, 44 px de altura); nome em Fill e cortado se não couber; Nv, classe e cargo sem quebra.
- `WBP_SkillBookEntry`: textos à esquerda ocupando a largura; `Rank_HBox`, `Status_HBox_1` e os spacers ficaram **Collapsed**.
- `WBP_SkillBook`: coluna de detalhe (`Border_13`) em Fill 0,85 (antes estreita); textos dos stats e a escala quebram dentro da célula; nome da skill centralizado com quebra.
- Arrastar skill do livro não mostra mais as estrelas e os ícones de status antigos (o pass-through do drag só mexe no que está visível).

### Atalhos da barra de skills (`FUmbraSkillbarInputProcessor`)

- **Bug:** depois de Alt+Tab (ou de clicar em outro programa) nenhum atalho funcionava mais. O processador guarda as teclas apertadas e só dispara quando o conjunto bate exatamente com o bind; o KeyUp do Alt nunca chegava, o conjunto ficava `{Alt}` e "1" virava `{Alt, 1}` para sempre.
- **Correção:** o conjunto é limpo quando o app perde o foco; a cada tecla, Alt/Ctrl/Shift são conferidos com o estado real do teclado; tecla comum sem KeyUp e sem repetição há mais de 1,2 s sai do conjunto.
- **PIE com 2 clients:** cada client só processa as teclas da própria janela (antes a tecla apertada na janela do P2 também disparava a skill do P1).
- Segurar a tecla continua repetindo a skill, como antes.

### Contador de itens do inventário (`WBP_Inventory`)

- "Itens: N/50" (`Text_ItemCount`) agora é do C++: atualiza ao abrir, a cada carga do inventário (loot, compra, troca, exclusão, criação...) e ao mover ou empilhar. A função BP `UpdateItemCount` ficou vazia (antes só rodava ao mover). Itens equipados não contam. Capacidade em **Class Defaults → Umbra|Inventory → Inventory Capacity**.

## 3) O que o C++ já faz

- Títulos de todas as janelas `UmbraWindowWidget`: fonte Cinzel (quando importada) e "..." quando o título não cabe.
- Valores de BD em inglês nos tooltips e no livro de skills (`ACTIVE`, `ENEMY`, `FIRE`, `MANA`, `INT`...) aparecem em pt-BR (`UmbraUI::SkillTermLabel`); valor desconhecido aparece como veio.
- Livro de skills (J9):
  - monta a lista (ataque básico primeiro, depois por nível);
  - preenche o detalhe em pt-BR e escolhe o botão (Aprender/Melhorar/Rank máximo) com o motivo quando está desligado;
  - envia `RequestLearnOrUpgradeSkill`, trava o botão até a resposta e mostra o erro do servidor em vermelho;
  - recarrega tudo quando os pontos, o nível ou a lista de skills mudam.
- Rótulos novos em `NSLOCTEXT` (prontos para tradução).
- Tela de morte (J10): "Você morreu" em Cinzel vermelho; "Morto por X" com o autor do último dano recebido (opcode 92: jogador pelo cache de nomes, NPC pelo nome do actor; se os dois existirem com o mesmo id, vale o mais perto); sem autor (ambiente, dano em si mesmo) a linha some. **Renascer** envia o opcode 90, mostra "Renascendo..." e libera de novo depois de 5 s sem resposta; a tela fecha no respawn (opcode 91).
- Desconexão (J10): título pelo motivo ("Conexão perdida", "Sessão aberta em outro lugar", "Sessão expirada", "Desconectado pelo servidor"), mensagem em pt-BR (o erro cru da engine fica só no log), input só na UI com cursor, **Voltar ao login** ou Enter chama `ReturnToLoginLevel`.
- Diário (J10): abre já com a primeira missão selecionada; linha selecionada em dourado; "Pronta para entregar" em verde na lista; objetivos "• texto 3/5" e "✓ texto 5/5" em verde; cabeçalhos "Objetivos" e "Recompensas"; recompensa "N ouro", "Item ×N" e a classe só quando o item é restrito; **Entregar** só com a missão pronta (com escolha, pede uma por grupo); **Abandonar** pede confirmação; os dois travam até a resposta e mostram o erro do servidor em vermelho.
- Escolha de recompensa (J10): linhas no cartão escuro, a escolhida em dourado; item de outra classe em vermelho com "(sua classe não pode)"; "Escolha uma (1 de 2)" só quando há mais de um grupo; **Receber recompensa** só liga com todas as escolhas.
- Log de combate (J10): "Você recebeu N de dano", "Você causou N de dano em Fulano"; motivos em pt-BR; combate entre outros jogadores não entra.
- Ficha (2.7): só o C++ escreve os valores; rótulo corta em vez de passar por baixo de um valor grande; segurar + / − repete e acelera.
- Excluir item (2.7): aceita item do inventário solto em qualquer ponto da janela; recusa item equipado ou do armazém com aviso vermelho; ícone e "Nome +N ×N" na cor da raridade; **Excluir** só liga com item; raro ou refinado pede confirmação vermelha; chama `RemoveItem`, mostra "Excluindo...", depois "X excluído." em verde ou o erro do servidor em vermelho.
- Armazém (2.7): "N ouro guardado" e "Itens: N/100" atualizados a cada carga do armazém e a cada depósito/saque de ouro.

## 4) QA geral (PIE, 2 players)

Roteiro único para depois do rebuild que junta J5 a J10 (lote A e lote B incluídos). J2 e J4 já passaram no QA; as linhas deles ficam como regressão.

**Antes de começar**

1. **Rebuild** do módulo `UmbraEternumUE`. O J9 e o J10 mexeram em headers (`UmbraSkillBookWidget.h`, `UmbraUIStyle.h`, `UmbraGameInstance.h`, classes novas), então Live Coding não basta. Depois, o reparent da seção 2.6.
2. PHP do lote A já sincronizado no WAMP (`enchant_helper.php`).
3. Abrir o PIE com 2 clients, P1 e P2 logados no mesmo mapa.
4. Deixar o **Output Log** filtrado por `Umbra` e anotar qualquer `Warning` ou `Error` de widget (por exemplo, "EntryClass não carregou").

Marque cada linha com ✅ ou ❌. Quando der ❌, anote a janela e um print.

### 4.1) Tooltips e janelas (base)

| # | Teste | Esperado |
|---|-------|----------|
| 1 | Hover em skill simples na barra | Cabeçalho com ícone, grade só com o que existe, sem espaço vazio |
| 2 | Hover em skill com muitos efeitos | Tudo dentro da moldura; o tooltip cresce para baixo e não sai da tela |
| 3 | Skill na barra antes de abrir o livro | Nome, custo e recarga + aviso "Detalhes indisponíveis..." |
| 4 | Item comum sem stats | Só nome, meta, descrição e rodapé |
| 5 | Equipamento com muitos stats e cristais | Uma linha por stat ≠ 0; cristais com ícone |
| 6 | Item de outra classe | Linha de classes em vermelho com "(sua classe não pode)" |
| 7 | Item refinado | Nome com +N e stats somados ao refino |
| 8 | Abrir o "Item Info" (clique) e clicar no X | Fecha |
| 9 | Janela com título longo | Título termina em "..." sem cobrir o X |

### 4.2) Ficha e diálogos (regressão J2/J4)

| # | Teste | Esperado |
|---|-------|----------|
| 10 | Abrir a ficha (C) | Rótulos em pt-BR à esquerda e valores à direita; "Nv N · Classe"; uma linha de EXP com % |
| 11 | Ficha: + em Força → Aplicar / Redefinir | Valor verde e somado; "Pontos livres" diminui; Aplicar salva; Redefinir volta |
| 12 | Ficha: X com outra janela aberta | Fecha só a ficha; o cursor continua |
| 13 | Ficha no nível máximo | "EXP N · nível máximo", barra cheia; terceira coluna visível |
| 14 | Inventário: Shift + arrastar pilha de 10 | "Dividir pilha", começa em 5; Confirmar cria a nova pilha |
| 15 | Diálogo aberto: Enter / ESC / X | Confirma / cancela / cancela |

### 4.3) Troca, lojas, inventário e armazém (J5/J6)

| # | Teste | Esperado |
|---|-------|----------|
| 16 | Troca entre P1 e P2 | Ofertas lado a lado; quem aceitou vira "Pronto" (verde) nas duas telas; "N ouro"; "Itens N/10" |
| 17 | Troca → **+ Ouro** acima do que tem / **− Ouro** | Campo trava no máximo; P2 vê o ouro; − Ouro limitado ao oferecido |
| 18 | Loja pessoal (vendedor) | "Nome da loja" fora da barra de título; Definir preço abre o diálogo novo; mensagens com "ouro" |
| 19 | Vendedor NPC | Seções COMPRAR / VENDER; prévia "Item · Qtd N · Total N ouro" |
| 20 | Inventário | "Excluir itens" avermelhado; "Itens: N/50"; "N ouro" |
| 21 | Excluir item: arrastar → Excluir / Cancelar | Nome aparece; aviso vermelho; Excluir remove; Cancelar fecha sem excluir |
| 22 | Armazém | Botões em pt-BR; Depositar/Retirar tudo funcionam; depositar/sacar ouro pelo diálogo; "Itens: N/100" |
| 23 | Loot com ouro | "N ouro" no topo da janela de loot |

### 4.4) Correio e leilão (J6 + lote B)

| # | Teste | Esperado |
|---|-------|----------|
| 24 | Correio: lista | Remetente em cima, assunto embaixo, "N anexo(s)" à direita; não lida destacada; vazio = "Nenhuma carta." |
| 25 | Correio: abrir carta | Linha fica dourada; assunto como título, "De: X" em cinza, corpo com quebra de linha |
| 26 | Correio: Coletar / Coletar tudo / Excluir / Atualizar | Funcionam e a lista atualiza |
| 27 | Leilão: Explorar | Linhas em cartão escuro; nome na cor da raridade (+N, ×N); "Arma · Espada · Raro · Vendedor: X"; "N ouro" (vermelho se faltar); "Expira em …" |
| 28 | Leilão: filtros | Combos escuros (botão e lista aberta) com rótulos pt-BR; Filtrar devolve o resultado certo; busca com caixa e dica |
| 29 | Leilão: abas | Explorar / Vender / Meus anúncios trocam; a ativa fica clara; "Meus anúncios" vazio = "Você não tem anúncios ativos." |
| 30 | Leilão: anunciar (P1) → comprar (P2) → cancelar outro anúncio | Mensagens com "ouro" e sem "#id"; erro em vermelho; o ouro de P1 sobe |

### 4.5) Social (J7 + lote B)

| # | Teste | Esperado |
|---|-------|----------|
| 31 | Guilda sem guilda | Só "Criar guilda"; sem cabeçalho, barra e abas |
| 32 | Guilda com guilda | "[TAG] Nome", "Nv N", barra com "EXP a / b", "Membros N/M"; aba Membros ativa |
| 33 | Guilda: abas | Gerenciar mostra convite, EXP e sair; Membros volta a lista; a aba ativa fica clara |
| 34 | Guilda: Adicionar EXP 500 | EXP e barra sobem; ao passar do limite, o "Nv" sobe |
| 35 | Guilda: Sair / Debandar / Remover membro | Confirmação vermelha; Cancelar/ESC não faz nada; Confirmar/Enter executa |
| 36 | Guilda: lista de membros (líder e membro comum) | Uma linha por membro (nome, classe, "Nv N", cargo colorido); Promover/Rebaixar/Remover só para quem pode; Debandar só para o líder |
| 37 | Amigos (P1 amigo de P2) | "1 amigo · 1 online", bolinha verde; P2 desloga → cinza no próximo refresh (30 s) |
| 38 | Amigos: vazio / Convites (N) | "Nenhum amigo ainda."; o contador abre um popup por pedido, sem duplicar |
| 39 | Amigos: clicar em 2 linhas seguidas | Só uma janela de opções aberta |
| 40 | Bloqueados | "N jogadores bloqueados"; vazio = "Ninguém bloqueado."; desbloquear tira da lista |
| 41 | Sussurro P1 → P2 | Enter envia; nome de P1 dourado e de P2 azul; X fecha; reabrir funciona |
| 42 | Denúncia | "0/10 mínimo"; Enviar desligado até 10 caracteres (contador verde) |

### 4.6) Criação, refinamento e encantamento (J8 + lote A)

| # | Teste | Esperado |
|---|-------|----------|
| 43 | Criação: abrir | Categorias "Todas/Materiais/Consumíveis/Equipamentos"; combo escuro; "N receitas. Selecione na lista." |
| 44 | Criação: escolher receita | Linha dourada; "Resultado: X ×N"; ingredientes "tem / precisa" verdes/vermelhos; custo em ouro (vermelho se faltar) |
| 45 | Criação: − / + e categoria vazia | "×N" de 1 a 99, ingredientes e custo multiplicam; "Nenhuma receita nesta categoria." |
| 46 | Criação: soltar item errado no espaço | Status vermelho "Este espaço pede: X." |
| 47 | Refinamento: layout | Item/Material à esquerda; nome, "+N → +N+1" e material à direita, numa faixa só; sem faixa preta nem quadrados brancos |
| 48 | Refinamento: soltar equipamento | Tabela Atual → Próximo inteira, com "(+N)" verde; chance colorida; "Material: … · você tem N"; status abaixo da chance |
| 49 | Refinamento: Cancelar | Limpa e fecha |
| 50 | Encantamento: equipamento sem afixo | Encaixes "Vazio" cinza; Remover desligado; extrator escondido |
| 51 | Encantamento: aplicar cristal | "Afixo +N" verde; extrator aparece; Remover liga |
| 52 | Encantamento: Limpar | Esvazia equipamento, cristal e extrator; volta a mensagem inicial |
| 53 | Cristal: usar o último da bag e reabrir o encantamento | O encaixe continua com o ícone do cristal |
| 54 | Tooltip de item encantado sem cristal na bag / item que caiu encantado | Linha do afixo com o ícone do cristal do atributo (ou o genérico) |

### 4.7) Livro de skills (J9)

| # | Teste | Esperado |
|---|-------|----------|
| 55 | Abrir o livro de skills | Cabeçalho com a classe em pt-BR e "Pontos: N" (dourado se > 0); lista com o ataque básico primeiro; a primeira skill já aparece no detalhe |
| 56 | Linhas da lista | Ícone, nome, "Ativa · Fogo", "Rank N/M · Nv N" e uma tag: Aprender (dourado), Melhorar (verde), Máx., Aprendida, "Requer Nv N" (vermelho) |
| 57 | Clicar em outra skill | A linha fica dourada e só ela; o detalhe troca |
| 58 | Detalhe | Nome em título, ícone grande, "Tipo · Alvo: Inimigo", "Elemento: Fogo"; só aparecem os stats que se aplicam (sem "Duração" numa skill instantânea, por exemplo); "Escala: FOR 50% · …"; sem estrelas nem descrição duplicada |
| 59 | Skill não aprendida com ponto e nível | Botão "Aprender (1 pt)" ligado → clicar: "Enviando...", depois aprende; pontos diminuem; a linha vira "Aprendida"/"Melhorar"; a seleção continua na mesma skill |
| 60 | Melhorar até o máximo | "Melhorar (1 pt)" sobe o rank ("Rank N/M" dourado); no máximo o botão fica "Rank máximo" desligado |
| 61 | Sem pontos / nível baixo | Botão desligado e motivo em vermelho ("Pontos insuficientes..." / "Requer nível N.") |
| 62 | Ataque básico | Sem botão; "Ataque básico da classe: sempre disponível." |
| 63 | Erro do servidor (ex.: clicar duas vezes rápido) | Botão não dispara duas vezes; se o servidor recusar, a mensagem aparece em vermelho e o botão volta |
| 64 | Subir de nível com o livro aberto | Pontos e botões atualizam sozinhos |
| 65 | Arrastar skill aprendida para a barra | O drag continua funcionando; o livro segue clicável depois |
| 66 | X / reabrir o livro | Fecha; ao reabrir, a lista e o detalhe aparecem sem duplicar linhas |
| 67 | Tecla K | Abre o livro novo (o mesmo da HUD); K de novo fecha. Nunca abre dois livros |

### 4.8) Morte, desconexão, missões e limpeza (J10)

| # | Teste | Esperado |
|---|-------|----------|
| 68 | P2 mata P1 | Tela escura com "Você morreu" e "Morto por P2" |
| 69 | NPC mata P1 | "Morto por <nome do NPC>" |
| 70 | Morrer na lava / ambiente | Sem a linha "Morto por" |
| 71 | **Renascer** | "Renascendo...", P1 volta no ponto de spawn e a tela some; P2 vê P1 de pé |
| 72 | Renascer sem servidor (zone parada logo após morrer) | Depois de 5 s o botão volta a "Renascer" |
| 73 | Parar a zone com P1 em jogo | "Conexão perdida" + mensagem em pt-BR; cursor livre; o jogo não recebe teclas |
| 74 | Logar a mesma conta em outro client | No primeiro: "Sessão aberta em outro lugar" |
| 75 | Desconexão: **Voltar ao login** / Enter | Volta para a tela de login sem erro no log |
| 76 | F10 | Abre só o menu do jogo (sem o popup antigo de opções) |
| 77 | Abrir o diário (botão da HUD ou clique no rastreador) com missões ativas | Abas "Ativas (N)" / "Concluídas (N)" no topo; a primeira missão já aparece no detalhe, com a linha dourada |
| 78 | Diário: detalhe | Título em Cinzel; "Objetivos" com "• … 3/5" e "✓ …" verde; "Recompensas" com "N ouro", "N EXP", "Item ×N" |
| 79 | Diário: missão pronta | Na lista "Pronta para entregar" em verde; **Entregar** aparece; clicar entrega e a missão vai para Concluídas |
| 80 | Diário: missão com escolha | Linhas de escolha em cartão escuro; clicar marca em dourado; **Entregar** sem escolher mostra aviso vermelho |
| 81 | Diário: **Abandonar** | Confirmação vermelha; Cancelar não faz nada; Confirmar remove a missão da lista |
| 82 | Diário: aba Concluídas vazia | "Nenhuma missão concluída." |
| 83 | Entregar no NPC uma missão com escolha | Abre "Recompensa da missão"; **Receber recompensa** desligado até escolher; item de outra classe em vermelho |
| 84 | Escolha com 2 grupos | "Escolha uma (1 de 2)" e "(2 de 2)"; rodapé "Escolhidas: 1 de 2" até completar |
| 85 | Troca: **+ Ouro** | Abre o diálogo padrão (não existe mais modal antigo) |
| 86 | Inventário: arrastar pilha sem Shift / com Shift | Sem Shift arrasta normal; com Shift abre "Dividir pilha"; nenhuma mensagem azul na tela |
| 87 | Login, cadastro, seleção e criação de personagem | Todos os textos em pt-BR ("Entrar", "Criar conta", "Escolha o personagem", "Jogar"...) |
| 88 | Menu de contexto de jogador e opções de amigo | "Convidar para o grupo", "Negociar", "Sussurrar", "Seguir", "Inspecionar"... |
| 89 | Hover em buff | Rótulos curtos em pt-BR (FOR, DES, Atq. físico, Precisão...) |
| 90 | Log de combate | "Você recebeu N de dano" / "Você causou N de dano em Fulano"; sem ids nem "COMBAT" |
| 91 | Tecla J (depois do rebuild) | Abre o diário; J de novo fecha. Conferir também: digitando "j" no chat, o diário não deve abrir |

### 4.9) Correções do QA geral (seção 2.7)

| # | Teste | Esperado |
|---|-------|----------|
| 92 | Ficha: + em Força → **Aplicar** | Depois de salvar, os valores continuam só números à direita ("10", "380 / 380"); nada de "Strength: 8/8" sobre o rótulo |
| 93 | Ficha: **segurar** o + com 30+ pontos livres | Soma 1, espera um instante e começa a subir sozinho, cada vez mais rápido; para ao soltar e quando os pontos acabam |
| 94 | Ficha: segurar o − | Desce até 0 pendente e para |
| 95 | Ficha com atributo alto (ex.: 2.000 (+150)) | O valor aparece inteiro; o rótulo corta se faltar espaço, sem sobrepor |
| 96 | Excluir item: abrir sem item | "Arraste um item do inventário para cá." em cinza; **Excluir** desligado; aviso vermelho embaixo |
| 97 | Excluir item: soltar item comum e **Excluir** | Ícone e nome na cor da raridade (×N se pilha); "Excluindo..." → "X excluído." verde; o item some do inventário |
| 98 | Excluir item: soltar item raro ou +N | **Excluir** abre confirmação vermelha; Cancelar/ESC não exclui; Confirmar/Enter exclui |
| 99 | Excluir item: soltar item equipado / **Cancelar** | "Desequipe o item antes de excluir." em vermelho / fecha a janela sem excluir |
| 100 | Armazém: abrir com itens e ouro guardado | "Itens: N/100" com o número certo; "N ouro guardado" (não o ouro da bolsa); depositar/sacar atualiza os dois |
| 101 | Leilão → **Vender**: soltar item | Ícone limpo no slot, sem preço por cima; o preço só no campo "Preço de venda" |
| 102 | Guilda → Membros (líder vendo membro comum) | Linha cabe na janela: nome, "Nv 50", classe, cargo e os 3 botões visíveis, sem "Nv" quebrado |
| 103 | Livro de skills: lista, detalhe e drag | Linhas com textos à esquerda numa linha só; stats do detalhe inteiros ("Conjuração: instantânea"); arrastar skill não mostra estrelas nem ícones "CAN LEARN&MAX" |
| 104 | Atalho na barra → **Alt+Tab** para fora e volta → apertar o atalho | A skill sai normalmente (repetir 3 vezes, também segurando Alt ao sair) |
| 105 | Atalho com modificador (ex.: Shift+1) depois de Alt+Tab | Funciona; "1" sozinho não dispara o slot do Shift+1 |
| 106 | PIE 2 clients com o mesmo atalho: apertar na janela do P2 | Só o P2 usa a skill; o P1 não |
| 107 | Inventário: abrir, pegar loot, comprar no NPC, excluir item, equipar | "Itens: N/50" muda na hora em cada caso (equipado não conta), sem precisar mover item |

## 5) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Tooltip só com a moldura vazia | Build sem o C++ da fase 6 | Rebuild do módulo |
| Título continua em Roboto | `Font_Cinzel` ausente ou com outro nome | Seguir a seção 1 ou apontar no `DA_UmbraUIStyle` |
| Tooltip antigo (três colunas) | WBP sem `VB_Content` | Conferir o nome exato do VerticalBox |
| Livro de skills com a lista vazia e o log "EntryClass não carregou" | `WBP_SkillBookEntry` mudou de pasta ou de nome | **Class Defaults → SkillBook → Entry Class** apontando para a linha |
| Livro com textos em inglês ("Power:", "Learn (1pt)") | Build antigo ou nós do Graph voltaram | Rebuild; conferir que o Event Graph do `WBP_SkillBook` está vazio |
| Linha de stat some mas o ícone fica | Texto foi tirado do `HB_PhysAtk_*` | Manter `Border > HB > (Image, Text)` |
| **Renascer** não faz nada | `WBP_DeathScreen` ainda com pai `UserWidget` | Reparent para `UmbraDeathScreenWidget` (seção 2.6) |
| Tela de morte em inglês ("You Died!") | Build sem o C++ do J10 | Rebuild |
| Desconexão sem título/mensagem ou sem diálogo | `WBP_ServerDisconnect` sem reparent, ou evento do GameInstance recriado no BP | Reparent para `UmbraServerDisconnectWidget`; apagar o evento `ShowServerDisconnectDialog` do `BP_UmbraGameInstance` |
| "Morto por" nunca aparece | O último dano veio sem origem (ambiente) ou o atacante saiu do alcance | Esperado; o log `[UmbraGameInstance]` não mostra erro |
| Diário sem o botão Entregar | `BTN_TurnIn` renomeado ou apagado | Restaurar o nome exato |
| Escolha de recompensa abre vazia | `VBox_Choices` renomeado | Restaurar o nome exato |
| Tecla K abre um livro antigo | `Open Skill Book Action` vazio no `BP_ThirdPersonCharacter` | Apontar para `IA_OpenPlayerSkillBook` |
| Excluir item não aceita o item / botão nunca liga | `WBP_DeleteItem` ainda com pai `UmbraDraggableWindowWidget` | Reparent para `UmbraDeleteItemWidget` (seção 2.7) |
| Ficha volta a mostrar "Strength: 8/8" | Nós de texto voltaram na função **Update Character Info Display** | Deixar só entrada → `UpdateEquipmentSlots` |
| Segurar + soma só 1 | Build antigo | Rebuild |
| Linha da guilda some (largura 0) | Linha adicionada fora de um ScrollBox/VerticalBox em Fill | Manter `ScrollBox_Members` como pai das linhas |
| Atalhos param depois de Alt+Tab | Build antigo | Rebuild; o log `[UmbraSkillbarInput] App sem foco` deve aparecer ao sair da janela com tecla presa |
| Atalho não funciona com a janela do jogo em foco | A janela ativa não é a do viewport (ex.: popup do editor por cima) | Clicar no viewport do jogo |
| "Itens: N/50" travado | Nó de texto voltou na função BP `UpdateItemCount` ou build antigo | Deixar a função vazia; rebuild |
| Armazém mostra o ouro da bolsa | Nó `Set Text` do `Text_Gold_Info` voltou no Graph do `WBP_Storage` ou build antigo | Remover o nó; rebuild |
