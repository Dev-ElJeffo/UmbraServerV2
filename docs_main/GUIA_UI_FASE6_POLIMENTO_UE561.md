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

- O Event Graph ficou vazio (106 nós apagados). As funções de Blueprint antigas (`PopulateSkillList`, `UpdateDetailPanel`, `UpdateHeader`) ficaram no asset, mas ninguém as chama; podem sair no J10.
- Binds (nomes exatos, já existentes): `Title_Class`, `SkillPoints_Text`, `Skills_VBox`, `Skill_Name_Text_1`, `Skill_Icon_Large_1`, `Skill_Type_Text_1`, `Skill_Element_Text_1`, `SkillBook_Description`, `SkillBook_Effects`, `Power_Text_1`, `BonusPower_Text_1`, `Cost_Text_1`, `Duration_Text_1`, `Range_Text_1`, `Cooldown_Text_1`, `Radius_Text_1`, `Cast_Text_1`, `CanCrit_Text_1`, `Reqtarget_Text_1`, `MovCast_text_1`, `PVPModifier_Text_1`, `Scaling_Text_1`, `CurrentRank_Text_1`, `Action_Info_1`, `RankStars_HBox_1`, `Action_Button_1`, `Text_Action_Button_1`.
- Cada linha de stat é `Border > HB (ícone + texto)`. O C++ esconde o **Border** inteiro quando o valor não se aplica, então não tire o texto de dentro do HB.
- `Skills_VBox` é montado pelo C++: não ponha nada dentro.
- A classe da linha vem de **Class Defaults → SkillBook → Entry Class** (padrão `WBP_SkillBookEntry`).
- `Skill_Description_Text` (texto cru, duplicado) e `RankStars_HBox_1` (estrelas) são escondidos pelo C++.
- O X é o `Btn_Close`, ligado pela base `UmbraWindowWidget`.

**Linha (`WBP_SkillBookEntry`)**: Graph vazio. Usa `BTN_SkillSelect`, `Skill_Icon`, `Skill_Name_Text`, `Skill_Type_Text`, `Level_Required_Text` e `Rank_Text`. As estrelas e os ícones de status são escondidos; a situação vira uma tag de texto.

## 3) O que o C++ já faz

- Títulos de todas as janelas `UmbraWindowWidget`: fonte Cinzel (quando importada) e "..." quando o título não cabe.
- Valores de BD em inglês nos tooltips e no livro de skills (`ACTIVE`, `ENEMY`, `FIRE`, `MANA`, `INT`...) aparecem em pt-BR (`UmbraUI::SkillTermLabel`); valor desconhecido aparece como veio.
- Livro de skills (J9):
  - monta a lista (ataque básico primeiro, depois por nível);
  - preenche o detalhe em pt-BR e escolhe o botão (Aprender/Melhorar/Rank máximo) com o motivo quando está desligado;
  - envia `RequestLearnOrUpgradeSkill`, trava o botão até a resposta e mostra o erro do servidor em vermelho;
  - recarrega tudo quando os pontos, o nível ou a lista de skills mudam.
- Rótulos novos em `NSLOCTEXT` (prontos para tradução).

## 4) QA geral (PIE, 2 players)

Roteiro único para depois do rebuild que junta J5 a J9 (lote A e lote B incluídos). J2 e J4 já passaram no QA; as linhas deles ficam como regressão.

**Antes de começar**

1. **Rebuild** do módulo `UmbraEternumUE`. O J9 mexeu em headers (`UmbraSkillBookWidget.h`, `UmbraSkillBookEntryWidget.h`, `UmbraUIStyle.h`), então Live Coding não basta.
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

## 5) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Tooltip só com a moldura vazia | Build sem o C++ da fase 6 | Rebuild do módulo |
| Título continua em Roboto | `Font_Cinzel` ausente ou com outro nome | Seguir a seção 1 ou apontar no `DA_UmbraUIStyle` |
| Tooltip antigo (três colunas) | WBP sem `VB_Content` | Conferir o nome exato do VerticalBox |
| Livro de skills com a lista vazia e o log "EntryClass não carregou" | `WBP_SkillBookEntry` mudou de pasta ou de nome | **Class Defaults → SkillBook → Entry Class** apontando para a linha |
| Livro com textos em inglês ("Power:", "Learn (1pt)") | Build antigo ou nós do Graph voltaram | Rebuild; conferir que o Event Graph do `WBP_SkillBook` está vazio |
| Linha de stat some mas o ícone fica | Texto foi tirado do `HB_PhysAtk_*` | Manter `Border > HB > (Image, Text)` |
