# PLANO — Fase 6: polimento de widgets, tooltips e textos (UE 5.6.1)

Continua o [PLANO_UI_JANELAS_TOOLTIPS_UE561.md](PLANO_UI_JANELAS_TOOLTIPS_UE561.md) (fases 1, 2 e 5 concluídas). Esta fase não muda regra de jogo nem protocolo: só layout, estilo, textos e o C++ que preenche os widgets.

**Regra da fase:** manter toda funcionalidade e todo nome de bind existente. Um bind novo sempre tem fallback para o antigo, para que um WBP ainda não migrado continue funcionando.

## 1) Diagnóstico (pré-visualização MCP, 1280x720 = escala de 1080p ÷ 1,5)

### Tooltips

| Widget | Problemas |
|--------|-----------|
| `WBP_SkillTooltip` (320x485) | Tipo e Elemento lado a lado se sobrepõem; rótulos em inglês ("Target", "Scalling", "Cooldown", "Power"); todos os campos centralizados, sem hierarquia; o painel tem metade vazia quando a skill é simples e transborda quando é complexa; o fallback da barra diz "Full data unavailable..." em inglês |
| `WBP_ItemTooltipRefined` | Três colunas fixas de stats, quase todas vazias na maioria dos itens; metadados (Type, SubType, EquipSlot, Tradeable, Max. Stack, Req. Level) espalhados em duas colunas pequenas; "Bloco de Texto" nos cristais; quadrado branco no lugar do ícone do cristal; rótulos em inglês; descrição vem depois dos metadados, numa faixa vermelha |
| `WBP_BuffTooltip` | Ícone quadrado branco quando vazio; quatro faixas com moldura para quatro linhas curtas; tudo centralizado |
| `WBP_StatTooltip` | Título, descrição e fórmula com "Bloco de Texto" no Designer (o C++ preenche); usa tooltip nativo |

A correção da fase 5b (`FitTooltipToContent`) já impede o texto de sair da borda. O que falta é o layout.

### Janelas

| Janela | Tamanho aprox. (1080p) | Problemas |
|--------|------------------------|-----------|
| `WBP_CharacterInfo1` | 880x1010 | Quase a altura da tela inteira; "Exp" duplicado; rótulos em inglês (Name, Level, Equipped Itens, Class, Faction, Unspent Points, Apply, Reset); botão "C.C Info" cinza fora do estilo; colunas de stats com alturas diferentes |
| `WBP_SkillBook1` | 600x700 | Sem moldura nem barra de título no padrão (fundo preto chapado, X vermelho padrão); "Avaliable Points" com erro de grafia |
| `WBP_Inventory` | 500x800 | Bom; "Delete Items" e "Gold" em inglês |
| `WBP_AuctionHouse` | 900x900 | Abas e filtros em inglês (Register Item, My Listings, Item Type, Apply Filter, Refresh, Search, Auction Info); combos com estilo padrão (barra branca) |
| `WBP_GuildPanel` | 900x900 | Tudo em inglês; criar guilda e gerenciar guilda aparecem juntos, mesmo para quem já tem guilda; "Add Guild EXP" é botão de debug à vista; campo de descrição branco |
| `WBP_Mail` | 730x500 | "Claim"/"Delete" em inglês e botões sem estilo; lista sem estado vazio |
| `WBP_NpcVendor` | 600x800 | "Vendor Title", "Buy Item", "Sell Item", "Cancel Offer", "amount", "Item, amount, total price" em inglês; slider vermelho padrão |

As demais janelas migradas nos lotes 1 a 5 seguem a mesma revisão no lote F6.3.

## 2) Design dos tooltips (mesmas informações, mais limpo)

Princípios:
- uma coluna, largura fixa de 300, altura pelo conteúdo;
- rótulo à esquerda em cinza e valor à direita em branco;
- só aparece o que tem valor (nada de "0" nem "-");
- seções separadas por linha fina, sem moldura por linha;
- cor só onde tem significado: raridade, bônus em verde, restrição em vermelho e ouro no valor.

### Skill

```
┌──────────────────────────────────────┐
│ [ícone 40]  Golpe Sombrio            │  nome (dourado, 16)
│             Rank 3/5 · Ativa · Trevas│  linha de meta (cinza, 11)
├──────────────────────────────────────┤
│ Custo           20 Mana  │ Recarga 8,0 s │  grade 2x2 só com o que existe
│ Conjuração      1,2 s    │ Alcance 0–600 │
│ Duração         6,0 s    │ Alvo  Inimigo │
├──────────────────────────────────────┤
│ Descrição formatada (quebra de linha,│  branco, 12
│ valores do rank já resolvidos)       │
├──────────────────────────────────────┤
│ • Efeito 1                           │  verde-claro, 11
│ • Efeito 2                           │
├──────────────────────────────────────┤
│ Poder 120 → 150 · Escala: INT        │  rodapé cinza, 10
└──────────────────────────────────────┘
```

### Item

```
┌──────────────────────────────────────┐
│ Espada Sombria +7                    │  nome na cor da raridade, 16
│ Épico · Arma · Espada de 2 mãos      │  raridade · tipo · subtipo (11)
│ Nível 30 · Mão principal             │  vermelho se o nível não basta
│ Classes: Bárbaro, Templário          │  vermelho + "(sua classe não pode)"
├──────────────────────────────────────┤
│ Ataque físico                 +120   │  só stats ≠ 0, uma linha cada
│ Força                          +15   │  base, combate e bônus juntos,
│ HP                            +200   │  bônus em verde
├──────────────────────────────────────┤
│ ◆ Cristal de Fogo +3                 │  cristais com ícone
├──────────────────────────────────────┤
│ "Descrição do item em itálico."      │  cinza claro, 11
├──────────────────────────────────────┤
│ 1.250 ouro   Durab. 87%   2/20       │  rodapé
│ [Negociável] [Refinável]             │  tags pequenas, só se verdadeiras
└──────────────────────────────────────┘
```

O modo clique (tooltip fixo com X) continua: `Btn_Close` aparece só nesse modo, na linha do nome.

### Buff e stat

- **Buff:** ícone de 32 ao lado do nome, descrição, linha de bônus em verde e tempo restante no rodapé. Sem as quatro molduras.
- **Stat:** título, descrição e fórmula numa caixa cinza; mesmo estilo de seções.

### Implementação

- **Stats do item:** o C++ passa a montar as linhas dentro de um `VB_StatList` (VerticalBox novo, criado via `WidgetTree`), com rótulos pt-BR em NSLOCTEXT. Os 20 TextBlocks antigos continuam como fallback: se o WBP não tiver `VB_StatList`, o caminho atual roda.
- **Grade da skill:** a grade de custo, recarga etc. vira `GP_SkillFacts` (UniformGridPanel), preenchido em C++. Os binds antigos (`Tooltip_Cost` e afins) viram fallback.
- **Layout no WBP:** montado pelo MCP (`add_panel_widget` com `widget_tree`) e substituindo a árvore antiga. Os nomes de bind que continuam sendo usados ficam iguais.
- **Textos:** todo rótulo dos tooltips passa a NSLOCTEXT pt-BR, inclusive o fallback "dados completos indisponíveis".

## 3) Base comum (antes dos lotes)

| Item | O que |
|------|-------|
| F6.0a — Estilo central | `UUmbraUIStyle` (DataAsset C++ + `DA_UmbraUIStyle`): cores (texto, cinza, dourado, raridades, bônus, erro), tamanhos de fonte (título 16, corpo 12, meta 11, rodapé 10), brushes (banner, X, separador, fundo de painel). É a fase 3 do plano anterior |
| F6.0b — Helpers C++ | `UmbraUI::MakeSectionSeparator`, `MakeLabelValueRow(Label, Value, Color)` e `ApplyTextRole(TextBlock, ETextRole)`, todos lendo o estilo central; usados pelos tooltips e pelas linhas dinâmicas das janelas |
| F6.0c — Proteção contra texto saindo | Na base `UUmbraWindowWidget` (uma vez no `NativeOnInitialized`): título com `TextOverflowPolicy::Ellipsis`; TextBlocks marcados como descrição com quebra de linha. Também um tamanho máximo da janela de 90% do viewport, com clamp, para janelas como a ficha do personagem não passarem da tela em 1280x720 |

## 4) Lotes

| Lote | Escopo | Arquivos principais |
|------|--------|---------------------|
| **F6.0** | Estilo central e helpers (seção 3) | `UI/Style/UmbraUIStyle.{h,cpp}` (novo), `UI/Window/UmbraWindowWidget.cpp`, asset `DA_UmbraUIStyle` |
| **F6.1** | Tooltips de skill e item no design novo | `UmbraSkillTooltipWidget.{h,cpp}`, `UmbraItemTooltipWidget.{h,cpp}`, `WBP_SkillTooltip`, `WBP_ItemTooltipRefined` |
| **F6.2** | Tooltips de buff, stat e refino; tooltip da barra de skills sem dados completos | `UmbraBuffTooltipWidget.*`, `UmbraStatTooltipWidget.*`, `UmbraRefinementTooltipWidget.*`, WBPs correspondentes |
| **F6.3a** | Ficha do personagem: reduzir para cerca de 760x860, pt-BR, um só Exp, botão "Chances de CC" no estilo, colunas alinhadas | `WBP_CharacterInfo1`, `UmbraCharacterInfoWidget.cpp` (textos) |
| **F6.3b** | Skill Book: moldura e barra no padrão, "Pontos disponíveis", entradas com nome, rank e botão de aprender alinhados | `WBP_SkillBook1`, `WBP_SkillBookEntry`, `UmbraSkillBookWidget.cpp` |
| **F6.3c** | Economia: Inventário, Armazém, Vendedor, Loja/Comprador, Leilão (pt-BR, combos e slider no estilo, estado vazio "Nenhum item") | WBPs da pasta Inventory/Storage/Npc/PlayerStore/Auction e C++ das linhas |
| **F6.3d** | Social: Guilda (mostrar **Criar guilda** só sem guilda e **Gerenciar** só com; esconder "Add Guild EXP" fora de build de desenvolvimento), Correio, Amigos, Bloqueados, Grupo | WBPs Guild/Mail/PlayerInteraction, `UmbraGuildWidget.cpp`, `UmbraMailWidget.cpp` |
| **F6.3e** | Crafting, Refino, Encantamento, Missões (diário, interação, recompensa; inclui o layout do `WBP_QuestRewardPicker`) | WBPs Crafting/Refinement/Enchant/Quest |
| **F6.4** | Varredura final: busca por textos em inglês e "Bloco de Texto" em todos os WBPs (`get_widget_info` + textos padrão) e em literais de C++ visíveis ao jogador; troca por pt-BR | Todos os WBPs de `/Game/Widgets` e `UI/*.cpp` |

Cada lote: pré-visualização MCP antes e depois, compilar e salvar os WBPs, você faz o rebuild e o QA, e depois commit.

## 5) O que não muda

- Nomes de bind já usados pelo C++, delegates, opcodes e fluxo de abrir e fechar das janelas.
- Posições salvas das janelas (ids iguais).
- Graphs que ainda têm lógica (FriendList, BlockedList, menu de contexto) não são tocados nesta fase; continuam como pendência de migração para C++.

## 6) QA por lote (PIE com 2 players)

- Tooltip de skill simples e de skill complexa (muitos efeitos): nada sai da borda e não há espaço vazio sobrando.
- Item comum (sem stats), equipamento com muitos stats, item refinado com cristais e item de classe proibida.
- Janelas em 1280x720, 1920x1080 e 2560x1440: nenhuma passa da tela e todo texto está em pt-BR.
- Funcionalidade de cada janela revisada: comprar, vender, registrar leilão, criar guilda, convidar, coletar correio, aprender skill e distribuir pontos.

## 7) Decisões (16/11/2026)

1. **Fonte dos títulos:** Cinzel (SIL OFL 1.1, livre para jogo comercial), arquivos e licença em `UmbraEternumUE/ThirdParty/Fonts/Cinzel/`. Corpo continua em Roboto.
2. **Tooltips:** só texto por enquanto; ícones de custo e recarga ficam para depois.
3. **"Add Guild EXP":** fica visível; será usado depois. Sai do item F6.3d.

## 8) Andamento

| Lote | Status |
|------|--------|
| F6.0 | Feito em C++: `UUmbraUIStyle` (`UI/Style/UmbraUIStyle.{h,cpp}`) com papéis de texto, cores e helpers de linha, separador e tag. Base `UUmbraWindowWidget`: título com "..." e fonte de título do estilo (tamanho e cor do Designer). Limite automático de 90% da tela **adiado**: mexeria na matemática de posição e arraste já aprovada; janelas grandes são corrigidas no Designer (F6.3a) |
| F6.1 | Feito: `VB_Content` nos dois WBPs; o C++ monta tudo (`BuildModern`). Item: X do modo clique saiu do Graph para o C++ (`Btn_Close` em `HB_Header`); moldura com 340 de largura. Skill: moldura `Border_44` (320) mantida. Rótulos do layout antigo também traduzidos. Aguarda rebuild e QA |
| Fonte | `Cinzel-Bold` (Font Face) importada pelo MCP; o C++ monta a fonte de runtime com ela (o asset `Font_Cinzel` ficou vazio: Python não acessa o composite). `/Game/Widgets/Fonts` sempre cozida (DefaultGame.ini) |
