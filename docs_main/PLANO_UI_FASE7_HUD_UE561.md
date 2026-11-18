# PLANO — Fase 7: HUD, alvo, escala da interface e chat (UE 5.6.1)

Continua a fase 6 (janelas e tooltips, QA geral aprovado em 17/11/2026). Esta fase cuida do que fica na tela durante o jogo: HUD, quadros de alvo, chat e escala da interface. Também corrige os nomes que ainda aparecem como id.

**Regras (as mesmas da fase 6):**
- toda funcionalidade fica;
- todo bind usado pelo C++ mantém o nome;
- lógica que hoje está no Graph passa para C++ antes de o layout mudar;
- textos visíveis em pt-BR (NSLOCTEXT quando vêm do C++);
- cada lote tem guia em `docs_main/GUIA_UI_FASE7_HUD_UE561.md` e QA com PIE de 2 clients.

## 1) Diagnóstico (prints do usuário, 17/11/2026)

| Área | Problema | Causa |
|------|----------|-------|
| Pedidos de troca, grupo e amizade | "Negociar com Player_32?" | Opcodes 21/11/31 trazem só o id; o GI monta `Player_%d` |
| Remover amigo | "Remover Fulano da lista de amigos?" | O BP cria o `WBP_FriendRemoveConfirm` sem chamar `ShowConfirm` (texto de designer) |
| Quadros de alvo | Quadrado branco no lugar do ícone; HP cru ("499836280/500000000"); MP em NPC; quadro cobre os nameplates | Ícone sem textura; texto `HP %d/%d`; posição por `SetPositionInViewport` em pixels |
| Resolução | Chat, barra de skills, HP/MP e menu se sobrepõem em janela estreita | 4 widgets soltos no viewport, cada um ancorado sozinho; painéis de tamanho fixo (HP/MP 640×120 no centro, menu 650 de largura à direita) |
| Escala | Não existe ajuste de tamanho da interface | Sem curva de DPI configurada nem opção para o jogador |
| Chat | Todas as abas mostram as mesmas linhas; sem cores, horário nem limite de linhas; aba Guilda falsa; "Canal ativo: Combat/Guild" em inglês; fundo grande vazio | Feed único em `UmbraChatMainWidget`; o servidor não tem canal de guilda |
| HUD geral | Painel de HP/MP grande e vazio; sem barra de EXP visível; tracker de missões com botões soltos e objetivos em inglês vindos do banco; aviso de debug do texture streaming na tela | Layout antigo; textos de quest no banco; `r.Streaming.PoolSize` baixo em dev |

## 2) Decisões do usuário (17/11/2026)

- **Layout da HUD:** fixo e responsivo **e** um modo "Editar HUD" para arrastar cada elemento e salvar a posição.
- **Menu da HUD:** fileira de ícones com tooltip (nome + atalho). Os ícones ficam com o usuário no Editor.
- **Chat:** o canal de guilda real (servidor + UE) entra no K5.
- **Ordem:** K1 → K4 → K3 → K2 → K5 → K7 → K6 → K8 (K7, painel de grupo, entrou no QA do K2; K8, feedback de combate e broadcast, no QA do K6).

## 3) Lotes

| Lote | Conteúdo | Camadas | Status |
|------|----------|---------|--------|
| **K1** | Nome do remetente nos opcodes 11/21/31 (campo trailing) com remetente autoritativo pela conexão; remover amigo com confirmação em C++ | Servidor, UE, guia | **QA ok** (17/11/2026) |
| **K4** | Escala da interface: slider 75–150% nas Configurações (`UUserInterfaceSettings::ApplicationScale`), curva de DPI pelo lado menor da tela, salvo nas configurações do cliente, prévia ao vivo | UE, config, guia | **QA ok** (17/11/2026); faixa revista para 50–125% no K3 |
| **K3** | Layout da HUD por jogador local (`UUmbraHudLayoutSubsystem`): posições ancoradas, regras para tela estreita, modo "Editar HUD" com posições salvas, ícones no menu, escala 50–125%, correção do tracker encolhendo e do mapa sumindo. HP/MP compacto movido para o K6 | UE, WBP, guia | **QA ok** (17/11/2026) |
| **K2** | Quadros de alvo novos (`WBP_TargetFramePlayer`/`WBP_TargetFrameNpc`): retrato, nome pela relação, nível pela diferença, HP com % e números compactos, MP só para jogador, debuffs antes de buffs, "Morto"/"Fora de alcance", X/ESC limpa o alvo | UE, WBP, guia | **QA ok** (17/11/2026) |
| **K5** | Chat: aba Geral + filtros, cores por canal, nome clicável, `/l /g /p /w /r`, histórico ↑/↓, sussurros e sistema no chat, 200 linhas por canal, "novas mensagens", fundo que some ocioso, movível/redimensionável, canal de guilda (opcodes 120/121), log de combate no padrão novo, canal por combo | Servidor, UE, WBP, guia | **QA ok** (17/11/2026) |
| **K6** | Polimento: **barra de HP/MP do jogador** compacta e no padrão novo (cartão escuro, texto compacto "7.660 / 7.660 · 100%" por cima das barras, sem o painel grande vazio, alinhada à barra de skills), barra de EXP, tracker de missões, destaque falso do slot 1, aviso de streaming em dev | UE, WBP, config, guia | **QA ok** (18/11/2026) |
| **K7** | Painel de grupo redesenhado (pedido no QA do K2, 17/11/2026): mesmo visual do quadro de alvo (`WBP_TargetFrame*`), mais compacto (linha por membro: nome, "Nv", HP com texto por cima, MP fina), sem o widget antigo; mostra **todos** os buffs e debuffs de cada membro com ícones menores (debuffs primeiro); clique seleciona o membro; elemento "Grupo" no layout da HUD (movível no Editar HUD) | UE, WBP, guia | QA ok |
| **K8** | Redesign (pedido no QA do K6, 18/11/2026): `/Game/Widgets/UI/CombatFeedback/WBP_CombatFeedback`, `WBP_BroadcastToast` e `WBP_DamageNumber` no padrão novo da fase 7 (cartão escuro com borda dourada, fontes e cores do `UmbraUIStyle`, texto compacto, posição pelo layout da HUD quando fizer sentido); avisos de combate em pilha com contador, aviso de sistema com fila e linha no chat, números de dano pt-BR com contorno e pop | UE, WBP, guia | **QA ok** (18/11/2026) |

## 4) K1 — detalhes

- **Protocolo:** `PartyInviteReceived` (11), `TradeRequestReceived` (21) e `FriendRequestReceived` (31) ganham o campo trailing `[nameLen:u16 LE][name:utf8]` (até 32 bytes, sem cortar caractere UTF-8). Cliente antigo ignora os bytes a mais; cliente novo com servidor antigo cai no cache de nomes e depois em "Jogador #id".
- **Segurança:** o remetente passa a ser o player da conexão (`playerIdForClientUnlocked`), não o `fromId` do payload; divergência vira warning no log.
- **Remover amigo:** `UmbraFriendOptionsWidget` abre `UUmbraAmountDialogWidget::OpenConfirm` (vermelho) e chama `RemoveFriend`; o delegate `OnRemoveFriendRequested` não é mais disparado.
