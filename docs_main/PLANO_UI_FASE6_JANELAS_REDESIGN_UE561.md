# PLANO — Fase 6 (parte 2): redesign das janelas e diálogos (UE 5.6.1)

Continua o [PLANO_UI_FASE6_POLIMENTO_UE561.md](PLANO_UI_FASE6_POLIMENTO_UE561.md) (F6.0 estilo central e F6.1 tooltips). Este documento substitui os lotes F6.2–F6.4 daquele plano por lotes por janela, no mesmo modelo dos tooltips: diagnóstico → layout proposto → implementação.

**Regras:**
- toda funcionalidade fica;
- todo bind usado pelo C++ mantém o nome;
- lógica que hoje está no Graph de um WBP passa para C++ antes de o layout mudar (o Graph é levantado com `inspect_graph` no começo de cada lote);
- textos visíveis em pt-BR, via NSLOCTEXT quando vêm do C++.

## 1) Diagnóstico (pré-visualização MCP em 16/11/2026, 1280x720)

### Janelas

| Janela | Tamanho (1080p) | Problemas |
|--------|-----------------|-----------|
| `WBP_CharacterInfo1` | 880x1010 | Quase a tela inteira; "Exp" duplicado; tudo em inglês (Name, Level, Equipped Itens, Character, Class, Faction, Unspent Points, Apply, Reset); botão "C.C Info" cinza fora do estilo; colunas de stats com alturas e larguras diferentes |
| `WBP_CcChances` | 330x290 | Rótulos em inglês (Stun Chance, Silence Resist...); valores e rótulos no mesmo texto |
| `WBP_GuildPanel` | 900x900 | Tudo em inglês; cabeçalho solto (Guild Name, Guild Tag, Level, EXP, Members) sem barra de EXP; campo de descrição branco; quatro botões largos empilhados. O C++ já alterna criar/gerenciar conforme o jogador tenha guilda |
| `WBP_Trade` | 545x720 | "My Offer"/"Received Offer"/"Accept Trade"/"Decline Trade"/"Add Gold" em inglês; ofertas empilhadas (difícil comparar); metade de baixo vazia; botões pequenos demais para a ação principal |
| `WBP_Personal_Player_Store` | 500x500 | Placeholder "Insert Store Name" dentro da barra de título; sem preço visível por item nem total; sem botão claro de abrir/fechar loja no layout |
| `WBP_PersonalShopBuyer` | 500x500 | Título duplicado ("Loja do jogador" + "Loja de Fulano"); sem resumo do item selecionado, preço e seu ouro antes de "Comprar" |
| `WBP_NpcVendor` | 600x800 | Compra e venda empilhadas; "Vendor Title", "Buy Item", "Sell Item", "Cancel Offer", "amount" em inglês; slider vermelho padrão; "Item, amount, total price" como texto fixo |
| `WBP_Storage` | 820x800 | Botões em inglês (Withdraw/Deposit All Items, Deposit/Withdraw Gold); título sem centralizar |
| `WBP_Inventory` | 500x800 | "Delete Items", "Gold" em inglês |
| `WBP_AuctionHouse` | 900x900 | Abas, filtros e rodapé em inglês; combos com estilo padrão (barra branca) |
| `WBP_Mail` | 730x500 | "Claim"/"Delete" sem estilo e em inglês; lista sem estado vazio; falta "Coletar tudo" visível |
| `WBP_CraftingMenu` | 900x900 | Category/Action/Refresh/Learn/Craft em inglês; combo branco; slider branco solto no meio do painel; "Status" fixo |
| `WBP_RefinementWindow` | 700x900 | Inglês (Item Name, Req. Material, Quant, Current Stats, Next Level Stats, Refine, Cancel); quadrados brancos nos slots; "Taxa de sucesso" e "Refinement Result" sobrepostos; faixa preta grande sem uso |
| `WBP_ItemEnchant` | 700x900 | Mistura pt/en (Aplicar Cristal / Cancel / Remove Slot 1); rodapé vazio; área preta |
| `WBP_QuestJournal` | 800x700 | Abas "Active Quests"/"Completed" no rodapé da lista; "Abandon Quest" em inglês; detalhe sem seções (objetivos, recompensas) |
| `WBP_SkillBook1` | 600x700 | Sem moldura/barra do padrão; "Avaliable Points"; X vermelho padrão |
| `WBP_FriendList` / `WBP_BlockedPlayersList` | 300x500 | "XXXX friends"/"blocked" em inglês; "Convites Pendentes" estoura o botão; sem estado vazio nem indicador online |
| `WBP_Whisper` | 600x500 | "Message Here"; botão "Fechar" redundante com o X; botões pequenos |
| `WBP_ReportPlayer` | 500x400 | Botões cinza padrão enormes; fundo preto chapado; "Report XXXX" em inglês |
| `WBP_DeathScreen` | — | "You Died!"/"Respawn" em inglês; caixa minúscula no meio de uma tela escura |

### Diálogos abertos por outras janelas

| Diálogo | Aberto por | Problemas |
|---------|-----------|-----------|
| `WBP_SplitInput` | Inventário (dividir pilha) | Fundo preto chapado; slider branco padrão; valor aparece como `""`; botões cinza "Confirm"/"Cancel"; sem mín/máx |
| `WBP_DeleteItem` | Inventário | "Delete Item"/"Deleted INFO" em inglês; não destaca que é irreversível; nome sem cor da raridade |
| `WBP_GoldDepositManager` / `WBP_GoldWithdrawManager` | Armazém | "Amount" em inglês; campo sem limite visível (quanto você tem) |
| `WBP_TradeGoldAmount` / `WBP_TradeRemoveGold` | Troca | Fundo preto, botões cinza, "Amount"/"Confirm"/"Cancel" |
| `WBP_SetPriceItem` | Loja pessoal | Já em pt-BR, mas visual igual ao Trade (botões cinza, fundo preto) |
| `WBP_FriendRemoveConfirm` | Lista de amigos (Graph) | "Friend Removal", "Remove X from friendlist?", "Accept" em inglês; X vermelho padrão |
| `WBP_ConfirmDialog` | (sem referência no C++; conferir Graphs) | Só um botão vermelho "CONFIRMA EXCLUIR?" |
| `WBP_OptionsPopup` | (sem referência no C++; conferir Graphs) | Antigo menu ESC em inglês; provável sobra do `WBP_GameMenu` |
| `WBP_PlayerContextMenu` | Clique direito em jogador | Inglês (Follow, Invite to Party, ADD Friend, Duel, Report, Block, Whisper); dez botões-banner iguais; ações perigosas sem destaque |
| `WBP_FriendOptions` / `WBP_BlockedPlayerOptions` | Linha da lista | Mesmo problema do menu de contexto; botão "Close" redundante |

## 2) Design comum (todas as janelas)

### 2.1 Anatomia de janela

```
┌─ moldura ─────────────────────────────────────┐
│ ▓▓▓▓▓▓▓▓  TÍTULO (Cinzel)  ▓▓▓▓▓▓▓▓▓▓▓▓  [X]  │  barra de título (banner) — arrasta
│                                               │
│  SEÇÃO (Cinzel 12, dourado)                   │  cabeçalho de seção
│  ───────────────────────────────────────────  │  separador fino
│  Rótulo .......................... Valor      │  linhas rótulo/valor
│                                               │
│  [conteúdo: grade de slots / lista / form]    │
│                                               │
│ ───────────────────────────────────────────── │
│  ◈ 12.500 ouro           [Secundário] [PRIMÁRIO]│ rodapé: info à esquerda, ações à direita
└───────────────────────────────────────────────┘
```

- **Padding:** 16 na moldura; espaço de 8 entre seções.
- **Rodapé fixo:** informação (ouro, slots, status) à esquerda e ações à direita. A ação principal fica sempre por último, à direita.
- **Estado vazio:** toda lista vazia mostra uma frase em cinza ("Nenhum item à venda.", "Nenhum amigo ainda.").

### 2.2 Botões (variantes no estilo central)

| Variante | Uso | Visual |
|----------|-----|--------|
| Primário | Confirmar, Comprar, Aceitar, Criar | Banner vermelho atual |
| Secundário | Cancelar, Atualizar, abas inativas | Banner neutro (cinza) |
| Perigo | Excluir, Debandar, Abandonar, Bloquear | Banner vermelho escuro com texto claro e confirmação obrigatória |
| Aba ativa | Aba selecionada | Banner dourado com texto escuro |

Implementação: `FButtonStyle` por variante no `UUmbraUIStyle` + `UmbraUI::ApplyButtonVariant(UButton*, EUmbraButtonVariant)`. O WBP marca a variante pelo **prefixo do nome** (`BTN_`/`Btn_` = primário por padrão; lista `DangerButtons`/`SecondaryButtons` nos Class Defaults da janela) para não exigir Graph.

### 2.3 Diálogo padrão (novo `UUmbraDialogWidget`)

Uma base C++ para todos os diálogos pequenos, em dois modos:

```
Confirmação                         Quantidade
┌───────────────────────────┐       ┌───────────────────────────┐
│ ▓▓ Excluir item ▓▓    [X] │       │ ▓▓ Dividir pilha ▓▓   [X] │
│                           │       │ [ícone] Poção de Vida     │
│ Excluir  Espada Sombria+7 │       │                           │
│ (cor da raridade)?        │       │  [−]  [   25   ]  [+]     │
│ Esta ação não pode ser    │       │  ├────────●──────────┤    │
│ desfeita.  (vermelho)     │       │  1                  49    │
│                           │       │  [Mín] [Metade] [Máx]     │
│     [Cancelar] [EXCLUIR]  │       │     [Cancelar] [CONFIRMAR]│
└───────────────────────────┘       └───────────────────────────┘
```

- **Teclado:** Enter confirma e ESC cancela. Camada Modal, centralizada, com fundo escurecido atrás.
- **Modo quantidade:** campo numérico validado entre mín e máx (o valor é limitado automaticamente), slider, botões −/+ e atalhos Mín/Metade/Máx.
- **Ouro:** mostra "Você tem X ouro" e formata com separador de milhar.
- **Migração:**
  - modo quantidade: `WBP_SplitInput`, `WBP_GoldDepositManager`, `WBP_GoldWithdrawManager`, `WBP_TradeGoldAmount`, `WBP_TradeRemoveGold`, `WBP_SetPriceItem`;
  - modo confirmação: `WBP_DeleteItem`, `WBP_FriendRemoveConfirm`, `WBP_ConfirmDialog`, além de debandar guilda e abandonar missão.
- O ponto que abre cada diálogo continua igual. O C++ chama `UUmbraDialogWidget::OpenConfirm(...)` / `OpenAmount(...)` com um delegate de resultado. Onde hoje o Graph abre o diálogo, a abertura passa para o C++ do dono.

### 2.4 Menu de contexto padrão (novo `UUmbraContextMenuWidget`)

```
┌────────────────────┐
│ Fulano  Nv 32      │  nome (Cinzel) + nível
│────────────────────│
│ Sussurrar          │  itens de texto (hover = faixa dourada),
│ Seguir             │  não banners
│ Convidar p/ grupo  │
│ Trocar             │
│ Inspecionar        │
│ Adicionar amigo    │
│ Convidar p/ guilda │
│ Duelar             │
│────────────────────│
│ Bloquear           │  ações sensíveis separadas, em vermelho
│ Denunciar          │
└────────────────────┘
```

- **Montagem:** os itens são montados em C++ a partir de uma lista, conforme o contexto: amigo, bloqueado, já em grupo etc.
- **Comportamento:** fecha ao clicar fora ou com ESC, e não passa da borda da tela.
- **Migração:** substitui `WBP_PlayerContextMenu`, `WBP_FriendOptions` e `WBP_BlockedPlayerOptions`. Isso também resolve a pendência de levar a lógica desses Graphs para C++.

## 3) Layout proposto por janela

### Personagem (`WBP_CharacterInfo1`, 880x1010 → ~760x820)

```
┌ Personagem ─────────────────────────────────── [X]┐
│ Fulano               Nv 32 · Bárbaro              │
│ ████████████░░░░░░  EXP 62,4%  (12.400 / 19.870)  │
├───────────────────┬───────────────────────────────┤
│  EQUIPAMENTO      │  PERFIL                       │
│  [paper-doll de   │  Facção ........ Ordem        │
│   slots, igual    │  Guilda ........ [TAG] Nome   │
│   ao atual]       │  Título ........ —            │
│                   │  PvP 3 · Caos 0 · Honra 120   │
├───────────────────┴───────────────────────────────┤
│ ATRIBUTOS              Pontos livres: 5           │
│ Força ......... 25 [−][+]   Vitalidade .. 18 [−][+]│
│ Destreza ...... 12 [−][+]   Sorte ....... 10 [−][+]│
│ Inteligência ..  8 [−][+]       [Redefinir][APLICAR]│
├───────────────────────────────────────────────────┤
│ COMBATE                                           │
│ Vida ...... 1.200/1.200   Mana ........ 300/300   │
│ Atq. físico ...... 145    Atq. mágico ...... 20   │
│ Def. física ....... 80    Def. mágica ...... 40   │
│ Crítico ........... 12%   Resist. crítico .. 5%   │
│ Atq. duplo ......... 8%   Resist. duplo .... 3%   │
│ Precisão .......... 95    Esquiva ......... 30    │
│ Velocidade ....... 450              [Chances de CC]│
└───────────────────────────────────────────────────┘
```

- **Binds:** os 44 binds atuais continuam. O C++ passa a escrever só o **valor** em cada `Text_*`, e o rótulo vira um TextBlock fixo no WBP. Assim acaba o "Strength 10" com rótulo e valor juntos e mal alinhados.
- **Tooltips de stat:** os tooltips de stat (já ajustados) seguem nos mesmos Borders.
- **CC (`WBP_CcChances`):** vira uma tabela de 3 colunas (Efeito | Chance | Resistência): Atordoar, Silenciar, Enraizar e Lentidão.

### Guilda (`WBP_GuildPanel`)

```
Sem guilda                               Com guilda
┌ Guilda ──────────────── [X]┐   ┌ Guilda ─────────────────────── [X]┐
│ Você não está em uma guilda│   │ [TAG] Nome da Guilda   Nv 4       │
│ ─────────────────────────  │   │ ███████░░░  EXP 1.200 / 3.000     │
│ CRIAR GUILDA               │   │ Membros 18/30                     │
│ Nome ....... [__________]  │   │ [Membros] [Gerenciar]   (abas)    │
│ Tag (2-5) .. [____]        │   │ ● Fulano    Líder     Nv 40       │
│ Descrição .. [__________]  │   │ ● Ciclano   Oficial   Nv 33       │
│              [__________]  │   │ ○ Beltrano  Membro    Nv 12  2d   │
│                  [CRIAR]   │   │ ...                               │
└────────────────────────────┘   └───────────────────────────────────┘
```

- **Aba Gerenciar:**
  - **Convidar:** campo de nome + botão Convidar.
  - **Add Guild EXP:** mantido, campo + botão.
  - **Ações:** **Sair da guilda** (perigo) e **Debandar** (perigo, só líder); as duas pedem confirmação no diálogo padrão.

### Troca (`WBP_Trade`)

```
┌ Troca com Fulano ───────────────────────────── [X]┐
│ VOCÊ OFERECE            │ FULANO OFERECE          │
│ [□][□][□][□][□]         │ [□][□][□][□][□]         │
│ [□][□][□][□][□]         │ [□][□][□][□][□]         │
│ ◈ 1.500  [+ Ouro][− Ouro]│ ◈ 0                     │
│ ○ Aguardando            │ ✓ Pronto                │
├─────────────────────────┴─────────────────────────┤
│ Itens 3/10        [Cancelar]  [ACEITAR TROCA]     │
└───────────────────────────────────────────────────┘
```

- Lado a lado, para comparar as duas ofertas.
- O status "Pronto" de cada lado vem do estado que o C++ já recebe.
- Quando qualquer oferta muda, o C++ cancela o "Pronto" e mostra isso de forma visível (regra que já existe no servidor).

### Loja pessoal — vendedor (`WBP_Personal_Player_Store`)

```
┌ Minha loja ─────────────────────────────── [X]┐
│ Nome da loja [ Loja do Fulano______________ ] │
│ [□ 1.200][□ 300][□ 50 ][□    ][□    ]         │  preço sob cada slot
│ [□      ][□    ][□    ][□    ][□    ]         │
│ Arraste itens do inventário; clique para      │
│ definir o preço.                              │
├───────────────────────────────────────────────┤
│ Total: ◈ 1.550     [Fechar loja] [ABRIR LOJA] │  aberta = "Fechar loja" em perigo
└───────────────────────────────────────────────┘
```

### Loja pessoal — comprador (`WBP_PersonalShopBuyer`)

```
┌ Loja do Fulano ─────────────────────────── [X]┐
│ [□ 1.200][□ 300][□ 50 ] ...                   │
├───────────────────────────────────────────────┤
│ Selecionado: Espada Sombria +7                │
│ Preço ◈ 1.200        Seu ouro ◈ 5.300         │  vermelho se não tem
├───────────────────────────────────────────────┤
│                                    [COMPRAR]  │
└───────────────────────────────────────────────┘
```

### Vendedor NPC (`WBP_NpcVendor`)

```
┌ Ferreiro Gorn ──────────────────────────── [X]┐
│ [Comprar] [Vender]                    (abas)  │
│ Comprar: grade de itens com preço             │
│ Selecionado: Poção de Vida   ◈ 25 cada        │
│ Qtd [−][ 10 ][+]  ├──●────┤   Total ◈ 250     │
│ Vender: arraste do inventário → prévia        │
│ "Espada x1 → ◈ 120"   [Cancelar] [VENDER]     │
├───────────────────────────────────────────────┤
│ ◈ 5.300                           [COMPRAR]   │
└───────────────────────────────────────────────┘
```

### Armazém (`WBP_Storage`) e Inventário

- **Armazém:** rodapé em duas linhas, no padrão: "Itens 34/100 · [Retirar tudo] [Depositar tudo]" e "◈ 12.500 · [Depositar ouro] [Retirar ouro]". Os botões de ouro abrem o diálogo de quantidade.
- **Inventário:** "Excluir itens" com a variante perigo e "◈ ouro" no rodapé.

### Leilão (`WBP_AuctionHouse`)

- **Abas:** **Procurar | Vender | Meus anúncios**.
- **Filtros:** Tipo, Subtipo e Raridade em combos no estilo da UI, busca por nome, **Filtrar** e **Atualizar** (secundário).
- **Lista:** cada linha mostra ícone, nome na cor da raridade, nível, preço e tempo restante. Lista vazia mostra "Nenhum anúncio encontrado.".

### Correio (`WBP_Mail`)

```
┌ Correio ────────────────────────────────────── [X]┐
│ ● Fulano     Itens da troca   2d │ De: Fulano     │
│   Sistema    Recompensa       5d │ Assunto: ...   │
│                                  │ corpo...       │
│                                  │ ANEXOS [□][□]… │
├──────────────────────────────────┴────────────────┤
│ [Atualizar] [Novo]   [Excluir] [Coletar tudo] [COLETAR]│
└───────────────────────────────────────────────────┘
```

### Criação (`WBP_CraftingMenu`)

- **Esquerda:** receitas agrupadas por categoria (o combo vira filtro), com marca de "aprendida".
- **Direita:**
  - resultado com ícone e nome;
  - ingredientes "Ferro 3/5", em vermelho quando falta;
  - custo em ouro;
  - quantidade com −/campo/+ (o slider solto sai);
  - status em cinza;
  - botões **Aprender** (secundário) e **CRIAR**.

### Refinamento (`WBP_RefinementWindow`)

```
┌ Refinamento ────────────────────────────────── [X]┐
│  [Item]  →  +7 → +8        [Material] 3/5         │
│ ───────────────────────────────────────────────── │
│  Atributo        Atual     Próximo               │
│  Atq. físico      120   →   132  (+12)  verde     │
│  Força             15   →    16  (+1)             │
│ ───────────────────────────────────────────────── │
│  Chance de sucesso   65%   (cor por faixa)         │
│  Falha: o item pode perder 1 nível.               │
├───────────────────────────────────────────────────┤
│                       [Cancelar]  [REFINAR]       │
└───────────────────────────────────────────────────┘
```

### Encantamento (`WBP_ItemEnchant`)

- **Topo:** slot do item, slot do cristal e **APLICAR CRISTAL**.
- **Meio:** três slots de cristal em linha, cada um com ícone, efeito e um botão "Remover" pequeno. O extrator de cristal aparece só quando for usado.
- **Rodapé:** status em cinza no lugar da faixa vazia.

### Diário de missões (`WBP_QuestJournal`)

- **Abas no topo:** **Ativas | Concluídas**.
- **Detalhe:**
  - título;
  - descrição;
  - **Objetivos** com progresso "Lobos 3/10", em verde quando completo;
  - **Recompensas** com ícones, EXP e ouro;
  - **Abandonar** (perigo, com confirmação).

### Livro de skills (`WBP_SkillBook1`)

- Moldura e barra no padrão, com "Pontos disponíveis: N" à direita do título.
- Cada entrada mostra ícone, nome, "Rank 2/5", "Requer Nv 20" (vermelho se faltar) e **Aprender**/**Melhorar**. Hover mostra o tooltip novo.

### Amigos e Bloqueados

- **Cabeçalho:** "12 amigos · 3 online".
- **Linhas:** bolinha verde ou cinza, nome, nível e clique direito para o menu padrão.
- **Convites:** o botão "Convites (2)" mostra um contador, e o texto não estoura mais.
- **Estado vazio:** "Nenhum amigo ainda." / "Ninguém bloqueado.".

### Sussurro, Denúncia, Tela de morte

- **Sussurro:**
  - conversa com o nome de cada lado em cores diferentes;
  - campo "Digite sua mensagem…";
  - Enter envia;
  - sai o botão "Fechar" (o X já fecha).
- **Denúncia:**
  - título "Denunciar Fulano";
  - campo com contador "0/10 mínimo";
  - **Enviar** (primário) e **Cancelar** (secundário) no padrão.
- **Tela de morte:**
  - vinheta escura em tela cheia;
  - "Você morreu" em Cinzel grande;
  - linha "Morto por X" se o dado existir;
  - **Renascer** com contagem se houver tempo de espera.

## 4) Lotes

| Lote | Escopo | Base C++ |
|------|--------|----------|
| **J1** | Variantes de botão, cabeçalho de seção, estado vazio e rodapé padrão no estilo central | `UmbraUIStyle` (ampliar) |
| **J2** | Diálogo padrão (confirmação e quantidade) e migração dos 9 diálogos da tabela | `UUmbraDialogWidget` (novo) |
| **J3** | Menu de contexto padrão (jogador, amigo, bloqueado); lógica dos Graphs para C++ | `UUmbraContextMenuWidget` (novo) |
| **J4** | Personagem + Chances de CC | `UmbraCharacterInfoWidget` (só valores nos `Text_*`) |
| **J5** | Troca, loja (vendedor, comprador, preço) e Vendedor NPC | `UmbraTradeWidget`, `UmbraPlayerStoreWidget`, `UmbraPersonalShopBuyerWidget`, `UmbraNpcVendorWidget` |
| **J6** | Inventário, Armazém, Leilão, Correio | widgets correspondentes |
| **J7** | Guilda, Amigos, Bloqueados, Sussurro, Denúncia | idem; FriendList/BlockedList saem do Graph |
| **J8** | Criação, Refinamento, Encantamento | idem |
| **J9** | Diário de missões, Interação e Recompensa (inclui o layout do `WBP_QuestRewardPicker`), Livro de skills | idem |
| **J10** | Tela de morte, desconexão, varredura final de inglês e "Bloco de Texto" | — |

Cada lote segue cinco passos:
1. Levantar o Graph e os binds.
2. Passar a lógica de BP para C++, quando houver.
3. Remontar o layout pelo MCP, com pré-visualização antes e depois.
4. Compilar e salvar os WBPs.
5. Fechar com o seu rebuild, o QA e o commit.

## 5) Perguntas antes do J1

1. `WBP_ConfirmDialog` e `WBP_OptionsPopup` não têm referência no C++. Se os Graphs também não usarem, posso **removê-los** (a sobra vira confusão) ou prefere manter?
2. Tela de morte: o `OnPlayerDeath` já traz `KillerId` e `Reason`, então dá para mostrar "Morto por X" resolvendo o nome pelo cache de NPC e jogador. Não existe tempo de espera para renascer, então o botão fica sem contagem. Ok assim?
3. Ordem dos lotes: começo pelos **diálogos (J2)**, que aparecem em muitas telas, ou pela **Personagem (J4)**?

## 6) Decisões (16/11/2026)

1. `WBP_ConfirmDialog`: sem referência nenhuma, **removido**. `WBP_OptionsPopup`: ainda referenciado pelo `BP_ThirdPersonCharacter`; avaliar no J10 (provável nó morto) antes de remover.
2. Tela de morte: "Você morreu" + "Morto por X" (pelo `KillerId`) e **Renascer** sem contagem.
3. Ordem: começar pelo **J4**.

## 7) Andamento

| Lote | Status |
|------|--------|
| J4 | Feito, aguarda rebuild e QA. C++: `UmbraUI::AttachRowLabel` cria o rótulo pt-BR ao lado de cada `Text_*` no `NativeOnInitialized` (antes do Slate montar) e o C++ escreve só o valor: atributos com total e "(+N)" do equipamento (verde com pontos pendentes), vida/mana "atual / máx", combate só o número, perfil com "—" quando vazio. Cabeçalho "Nv N · Classe" e uma linha "EXP 62,4% (atual / próximo)"; `Text_Total_EXP` escondido. WBP: textos fixos em pt-BR (Equipamento, Perfil, Aplicar, Redefinir, Chances de CC), botões no banner (Redefinir em secundário), janela 883x1015 → 800x940, stats em ScrollBox com Fill. Graph: removida a cadeia do X (Remove from Parent + Set Input Mode Game Only, que furava o gerenciador de janelas); a lógica de slots de equipamento (`RefreshEquipmentSlots` em BP) ficou intacta |
| J4 | **QA ok** (16/11/2026) |
| J2 (quantidade) | Feito, aguarda rebuild + reparent + QA. Um diálogo só no lugar de cinco: `UUmbraAmountDialogWidget` (`UI/Dialog/`) + `WBP_AmountDialog` (`/Game/Widgets/UI/Dialogs`). Finalidades: adicionar/remover ouro da troca, depositar/sacar ouro no armazém, dividir pilha; o C++ calcula o limite (ouro do jogador menos a oferta, oferta atual, ouro guardado, pilha − 1) e chama `AddTradeGold`/`RemoveTradeGold`/`DepositGold`/`WithdrawGold`/`RequestSplitItem`. Quem abre: Troca (`OpenGoldDialog`, com os modais antigos como alternativa), Armazém (`NativeConstruct` liga `BTN_Deposit_Gold`/`BTN_Withdraw_Gold`; nós do Graph removidos, junto com a cadeia do X que fazia Set Input Mode Game Only) e slot de inventário (`NativeOnDragDetected`: Shift + arrastar; sem Shift segue o BP de cada slot). `WBP_DeleteItem` não é diálogo de confirmação (é janela com slot de lixeira e feedback) e fica para o J6. Modais antigos (`WBP_TradeGoldAmount`, `WBP_TradeRemoveGold`, `WBP_Gold*Manager`, `WBP_SplitInput`) ficam até o QA e saem no J10 |
| J2 | **QA ok** (16/11/2026); `WBP_AmountDialog` com pai `UmbraAmountDialogWidget` |
| J4 (ajuste pós-QA) | Colunas de stats em Fill (1,4 / 1 / 1) para a terceira coluna não cortar; espaços menores; cabeçalhos 16; janela 820x960. C++: "Nv N · Classe" sem quebra; nível máximo mostra "EXP N · nível máximo" com barra cheia (antes "0,0% (N / 0)") |
| J5 | Feito, aguarda rebuild + QA. **Troca**: ofertas lado a lado (`HB_Offers`), janela AutoSize, "Sua oferta"/"Oferta recebida", "+ Ouro"/"- Ouro", "Aceitar troca"/"Cancelar", status por lado (`Text_MyStatus`/`Text_PartnerStatus`: Pronto verde / Aguardando); C++: "N ouro", "Itens N/10", "Troca concluída!", "Você aceitou. Aguardando o outro jogador...". **Loja (vendedor)**: campo do nome saiu da barra de título para a linha "Nome da loja" (dica "Digite o nome da loja"); barra com "Minha loja". **Preço**: moldura, barra, Confirmar primário/Cancelar secundário, AutoSize, dica "Preço em ouro". **Vendedor NPC**: cabeçalhos COMPRAR/VENDER, textos pt-BR, slider dourado, "N ouro"; C++ da prévia de venda em pt-BR. Comprador e lojas: "gold" → "ouro". Abas no vendedor NPC ficam para depois (mudam a lógica de seleção) |
| J6 | Feito, aguarda rebuild + QA. Textos estáticos em pt-BR via MCP. **Inventário**: "Excluir itens" (botão em tom de perigo), "Itens: 0/50", "0 ouro". **Armazém**: título "Armazém", "Depositar tudo"/"Retirar tudo", "Depositar ouro"/"Sacar ouro", "Itens: 0/100". **Excluir item**: "Excluir item", "Arraste um item para cá", aviso vermelho "Esta ação não pode ser desfeita.", "Excluir" (perigo) / "Cancelar". **Correio**: "Coletar", "Excluir", "Atualizar", "Coletar tudo". **Leilão**: abas "Explorar"/"Vender"/"Meus anúncios", filtros "Tipo"/"Subtipo"/"Raridade"/"Buscar:", "Filtrar"/"Atualizar", "Preço de venda:", "Anunciar"/"Limpar", dica de anúncio. C++: rótulos de fallback do correio em pt-BR; loot "N ouro" (antes "Gold: N"). Combos escuros ficam para o J1 (variantes de estilo) |
| J7 | Feito, aguarda QA. Rebuild feito; `WBP_FriendList`/`WBP_BlockedPlayersList` com pai `UmbraFriendListWidget`/`UmbraBlockedListWidget` e Graphs apagados (82 e 55 nós). **Guilda**: cabeçalho "[TAG] Nome", "Nv N", barra de EXP (`ProgressBar_GuildXP`, curva de `guild_helper.php`) com "EXP a / b", "Membros N/M"; abas Membros/Gerenciar (`Btn_TabMembers`/`Btn_TabManage`); convite e EXP em linha (`ETB_AddGuildXPAmount` criado, antes o botão nunca enviava); Sair, Debandar e Remover membro pedem confirmação; Debandar só aparece para o líder; cargos Fundador/Líder/Oficial/Recrutador/Membro; mensagens "guilda"/"ouro"; X do Graph removido. **Diálogo padrão**: modo confirmação (`UUmbraAmountDialogWidget::OpenConfirm`) no mesmo `WBP_AmountDialog` (esconde os controles de quantidade, Enter confirma, botão de perigo). **Amigos/Bloqueados**: classes novas `UUmbraFriendListWidget`/`UUmbraBlockedListWidget` (`UI/Social/`) com a lógica dos Graphs (83 e 56 nós): linhas online primeiro, "N amigos · N online", "Convites (N)" (pede ao servidor e o HUD abre os popups com dedupe), estado vazio, uma janela de opções por vez, auto-refresh liga/desliga; bolinha offline cinza. **Sussurro**: Enviar/Enter/Fechar em C++ (Graph vazio), botão "Fechar" removido, nome colorido por lado, "Conversa com X". **Denúncia**: contador "N/10 mínimo" (`Text_ReasonCount`), Enviar só com o mínimo, botões lado a lado no padrão. Opções de amigo/bloqueado (Graph) seguem para o J3 |
| J8 | Feito, aguarda rebuild + QA. **Criação**: categorias em pt-BR (Todas/Materiais/Consumíveis/Equipamentos → chaves do servidor); linhas de receita com estilo próprio (nome + meta "Aprender" dourado / "Não aprendida" / "Consome a receita" / "Nv N"), selecionada em dourado, estado vazio; ingredientes "Nome tem / precisa" (verde/vermelho, contando o inventário); "Resultado: X ×N"; "Custo: N ouro" vermelho se faltar; slider removido → `Btn_CountMinus`/`Text_CraftCount` "×N"/`Btn_CountPlus`; status cinza (erro vermelho); combo escuro; X do Graph removido. **Refinamento**: faixa preta removida (`Border_231`/`ItemSlotBorder` sem fundo), quadrados brancos removidos (`ItemIcon`/`MaterialIcon`), "+N → +N+1" dourado, tabela `VB_StatsCompare` "Atributo | Atual → Próximo (+N)" em pt-BR, "Chance de sucesso" colorida, "Material: N× X · você tem N", Cancelar/REFINAR. **Encantamento**: mesma faixa preta removida, rodapé vazio removido, textos pt-BR, encaixes "Vazio"/"Afixo +N" (verde), Remover só liga com afixo, extrator só aparece com afixo, status só no rodapé (antes repetido no topo), "Cancel" morto virou "Limpar" (C++). Falha "perde nível" não exibida: a config de refino não traz essa regra |
| J8 (ajuste pós-QA, lote A) | **Refinamento**: cabeçalho compacto `HB_RefineTop` (`VB_RefineSlots` com Item/Material à esquerda; `VB_RefineInfo` com nome, "+N → +N+1" e material à direita); status (`InfoMessageText`) no rodapé com a chance; tabela de comparação em Fill (antes cortada pelo painel de cima). **Ícone dos cristais**: o PHP (`enchant_parse_list`) manda `crystal_item_id` em cada afixo (template `enchant_crystal` com `enchant_force_stat` = `stat_key`; senão o cristal genérico); o cliente lê em `FUmbraItemEnchantment.CrystalItemId` e o tooltip e a janela de encantamento usam esse ID antes de procurar na bag (antes o ícone sumia quando não havia cristal na bag) |
| Lote B (padrão da criação) | **Estilo comum** (`UmbraUIStyle`): `MakeListRowStyle`/`ApplyListRowStyle` (linha de lista, selecionada dourada), `ApplyListRowBackground` (cartão escuro), `StatusColor`, `RarityColor`/`RarityLabel`, `ItemTermLabel` (weapon→Arma, sword→Espada, rare→Raro...), `GoldText`, `ApplyDarkComboItems`; a criação passou a usar esses helpers. **Leilão**: linha em cartão escuro, nome na cor da raridade com +N e ×N, "Arma · Espada · Raro · Vendedor: X", "N ouro" (vermelho se não dá para pagar), "Expira em 2h 15min", Comprar/Cancelar; combos escuros com rótulos pt-BR (a chave volta ao servidor), busca e preço com caixa e dica, abas com destaque, estado vazio, mensagens em ouro e sem "#id". **Correio**: linha com remetente/assunto e "N anexo(s)", não lida destacada, carta aberta dourada, "Nenhuma carta."; detalhe com assunto em título, "De: X" e corpo com quebra. **Guilda**: linha de membro em uma linha (nome, classe, Nv, cargo colorido) e ações pequenas só para quem pode gerenciar. **Livro de skills**: adiado (lista e detalhe montados por 106 nós + funções de Blueprint; precisa migrar a lógica para C++ antes) |
| J9 (livro de skills) | Feito, aguarda rebuild + QA geral. Lógica do Event Graph (106 nós) e das funções BP migrada para `UUmbraSkillBookWidget`: lista em C++ (`EntryClass` → `WBP_SkillBookEntry`; ataque básico primeiro, depois nível), linhas no padrão da criação (`ApplyEntryDisplay`: ícone, nome, "Ativa · Fogo", "Rank N/M · Nv N", tag Aprender/Melhorar/Máx./Aprendida/Requer Nv N; selecionada dourada), cabeçalho com classe e "Pontos: N", detalhe em pt-BR (stats só quando se aplicam; linha inteira escondida), "Escala: FOR …", botão Aprender/Melhorar/Rank máximo com motivo quando desligado, trava até a resposta e mostra erro do servidor (`OnSkillLearnFailed`/`OnSkillUpgradeFailed`). Termos do BD em `UmbraUI::SkillTermLabel` (tooltip e livro). WBP: Graphs do livro e da linha apagados, botão primário, textos pt-BR; estrelas e descrição duplicada escondidas pelo C++. Diário de missões e escolha de recompensa ficam para depois do QA geral |
| J10 | Feito, aguarda rebuild + reparent + QA geral. **Tela de morte**: `UUmbraDeathScreenWidget` ("Você morreu" Cinzel, "Morto por X" pela origem do último dano do opcode 92 resolvida em jogador/NPC, Renascer com trava de 5 s; Graph de 7 nós apagado; layout com vinheta). **Desconexão**: `UUmbraServerDisconnectWidget` + `ShowServerDisconnectDialog` virou `BlueprintNativeEvent` (C++ cria o widget); título por motivo, "Voltar ao login"; Graph, função e evento do `BP_UmbraGameInstance` apagados; erro cru da engine só no log. **Diário**: abas "Ativas (N)/Concluídas (N)" no topo, `BTN_TurnIn` novo (Entregar; o PHP aceita sem NPC), Abandonar com confirmação, trava até a resposta e erro em vermelho, primeira missão já selecionada, linha selecionada dourada, objetivos com •/✓, cabeçalhos de seção, X do Graph removido; `WBP_QuestEntry` refeito (linha = botão). **Escolha de recompensa**: `WBP_QuestRewardPicker` montado do zero (estava vazio), cartões no padrão de lista. **Limpeza**: apagados `WBP_OptionsPopup` e os 5 modais antigos; F10 do `BP_ThirdPersonCharacter` (abria o popup junto com o menu C++) e ramo Shift do `WBP_InventorySlot` removidos; funções mortas do `WBP_SkillBook` removidas; **tecla K** passou a abrir o `WBP_SkillBook` pelo C++ (o Graph do personagem abria o `WBP_SkillBook1` antigo, que não compila mais). **Varredura**: 199 textos de designer em 36 WBPs para pt-BR; C++: rótulos de buff, tooltip antigo, log de combate, nível "Nv N", login, ouro e missão |

| J10 | **QA ok** (17/11/2026) |
| J10 (extra) | Apagados `WBP_SkillBook1`/`WBP_SkillDetails`; `SetSkillData` morta removida do `WBP_SkillBookEntry` (estava em erro de compilação); tecla J (`IA_OpenQuestJournal` + `ToggleQuestJournal`), aguarda rebuild |