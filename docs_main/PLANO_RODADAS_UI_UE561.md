# Plano — próximas rodadas de correção (cliente UE 5.6.1)

Origem: análise feita depois do ciclo "HUD e widgets em C++" (branch `fix/hud-widgets-cpp`). Fontes:
- `Saved/Logs/UmbraEternumUE.log` de uma sessão de PIE com 2 players;
- varredura dos BPs via MCP EngineWire;
- leitura do C++ e do PHP.

Cada rodada termina com guia `docs_main/GUIA_*_UE561.md`, QA no PIE com 2 players, `/validar` e `/commit-umbra`.

| Rodada | Tema | Itens | Status |
|---|---|---|---|
| 1 | Limpeza de log/tela, party e fechamento do ciclo | R1.1–R1.4 | **Feita, QA ok** (guia `GUIA_LIMPEZA_LOG_PARTY_UE561.md`) |
| 2 | Trade, denúncia, deletar item e party na zone | R2.1, R2.2, R2.4, R2.5 | **Feita, QA ok** (guia `GUIA_TRADE_REPORT_PARTY_UE561.md`); falta só o cenário de party com 3. R2.3 (drag&drop) adiada, sob demanda |
| 3 | Conteúdo e mapa | R3.1–R3.2 | R3.1 feita (Necropolis restaurada só no disco, sem versionar); R3.2 (sons) vai para uma **rodada dedicada** mais tarde |
| 4 | Telas de entrada em C++ e dívida técnica | R4.1–R4.3 | **Em andamento.** Bloco 1 (R4.2 + R4.3 itens 1–2) **feito, QA ok** (guia `GUIA_JSON_DEPRECACOES_UE561.md`). R4.1: 4.1a/4.1b (cadastro e login) **feitas, QA ok** (guia `GUIA_TELAS_ENTRADA_UE561.md`); 4.1c (seleção) **feita, QA ok**; 4.1d (criação) a seguir. A limpeza de assets fica para o fim |

O merge do PR #25 fica para depois das 4 rodadas e do QA geral de regressão.

---

## Rodada 1 — detalhada

### R1.1 Log de yaw a cada frame

**Problema:** `UE_LOG(Warning)` de `[YawSend]`/`[YawRecv]` em `Network/WSBinaryBPFL.cpp` (linhas ~113, 163, 181, 255).
- Gerou ~111 mil das ~127 mil linhas de warning numa sessão curta.
- Custa desempenho no caminho do movimento e esconde os erros de verdade.

**Correção:** trocar por `VeryVerbose`, mantendo a mensagem para investigação com `log LogTemp VeryVerbose`.
- Melhor ainda: uma categoria própria `LogUmbraNetYaw` (`DECLARE_LOG_CATEGORY_EXTERN`), para ligar só ela.

**Arquivos:** `UmbraEternumUE/Source/UmbraEternumUE/Network/WSBinaryBPFL.cpp`.

### R1.2 `Print String` de debug (MCP)

**Problema:** 177 nós em 25 assets. Na tela aparecem, por exemplo, `SavePositionTimer - ...` e as linhas `WBP_STORAGE - UPDATE ALL SLOTS VISUAL` no log.

**Escopo desta rodada** (gameplay):

| Asset | Nós |
|---|---|
| `/Game/Blueprints/BP_NetMovementClient2` | 34 (conferir antes se ainda é usado; o Level BP usa `BP_NetMovementClient`) |
| `/Game/Blueprints/Player_BP/BP_ThirdPersonCharacter` | 22 |
| `/Game/Blueprints/Player_BP/BP_Player` | 11 |
| `/Game/Widgets/UI/Storage/WBP_Storage` | 9 |
| `/Game/Widgets/UI/PlayerInteraction/PlayerSelection/WBP_PlayerContextMenu` | 7 |
| `/Game/Widgets/UI/CharacterInfo/WBP_CharacterInfo1` | 6 |
| `WBP_Inventory`, `WBP_EquipmentSlot`, `WBP_InventorySlot` | 3 / 3 / 1 |
| `WBP_Trade`, `WBP_Levelup`, `WBP_SplitInput` | 2 / 1 / 1 |
| `WBP_SkillSlot`, `WBP_SkillSlot1`, `WBP_SkillBookEntry`, `WBP_SkillDetails`, `WBP_SkillBook` | 1 cada |

**Procedimento por asset** (o mesmo usado no `WBP_InventorySlot`):
1. `inspect_graph node followExec` para ver o que entra e o que sai de cada `Print String`.
2. Religar o exec: `in` → `then` (`connect_pins`).
3. Apagar o `Print String` e os nós que só alimentavam ele (`Format Text`, `To String`, `Append`), com `unusedOnly`.
4. Compilar e salvar; conferir `hasCompileErrors=false` e `warnings=[]`.

**Regra:** se um `Print String` for o **único** feedback ao jogador (mensagem de erro real), ele não é apagado. Fica listado no guia para virar texto de UI.

**Fora desta rodada:** `WBP_Login2`, `WBP_Register`, `WBP_CharacterSelection`, `WBP_CharacterItem`, `WBP_CharacterSelectButton`, `WBP_CreateCharacter`, `BP_CharacterSelectionManager`, `BP_Class_Placeholder`, `WBP_Dashboard` e `WBP_TestDisconnect`. Esses entram na R4.1.

### R1.3 Party: avisos para a zone inteira e grupo que não se desfaz

**Causa no servidor.** Os avisos de party vão para **todos os clientes da zone** (`ws_.broadcastBinary`):
- `PartyMemberJoined` (13), em `src/zone/MovementServer.hpp` ~551;
- `PartyStatsRefresh` (27), ~894;
- `PartyMemberLeft` (14), ~906 e no disconnect ~1899.

**Causa no cliente.** `UUmbraGameInstance` (`UmbraGameInstance.cpp` ~14855–14890) trata todo aviso como se fosse do próprio grupo:
- no 13, faz `CurrentPartyID = PartyID` de qualquer grupo e carrega esse estado;
- no 14 e no 27, chama `LoadPartyState(PartyID)` mesmo sem ser membro. Daí os `403 Você não pertence a este grupo`, inclusive para quem acabou de sair.

**Causa no PHP.** `leave_party.php` só apaga o grupo quando sobram 0 membros: quem fica sozinho continua num grupo de 1. O `catch` também devolve `$e->getMessage()` ao cliente, o que viola a regra do PHP.

**Correção (sem protocolo novo):**
- **Cliente:**
  - no 14 e no 27, ignorar se `PartyID != CurrentPartyID` ou se `CurrentPartyID == 0`;
  - no 13, não gravar `CurrentPartyID` a partir do broadcast:
    - se `CurrentPartyID == 0`, chamar `LoadPartyState(0)`. O `get_party_state.php` resolve o grupo do próprio jogador e devolve `in_party=false` sem 403 para quem não é membro;
    - se for o mesmo grupo, recarregar; se não, ignorar;
  - o `CurrentPartyID` passa a vir só da resposta do servidor;
  - `EnsurePartyPanelToViewport` só roda quando o aviso é do próprio grupo.
- **PHP** (`www/umbra_api/api/social/leave_party.php`):
  - quando sobrar 1 membro (ou menos), desfazer o grupo, apagando o membro restante e a linha em `parties`. O cliente que fica recebe o 14, recarrega e esconde o painel;
  - responder 500 com "Erro interno" e `error_log('[social/leave_party] ...')`, sem `getMessage()`.
- **Depois:** `/sync-wamp`.

**Melhoria futura (R2.5):** a zone mandar os avisos de party só para os membros, para não espalhar `party_id` pela zone.

**Arquivos:**
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`;
- `www/umbra_api/api/social/leave_party.php`.

### R1.4 Fechar o ciclo anterior

- `/validar` na árvore atual.
- PR de `fix/hud-widgets-cpp` para `Development` e merge, se o usuário pedir.
- Decidir as alterações locais que não são da tarefa (decisão do usuário):
  - `BP_ThirdPersonCharacter`, `SK_Mannequin`, `WBP_CreateCharacter` e 43 `__ExternalActors__`;
  - o `BP_ThirdPersonCharacter` também é tocado pela R1.2.

### QA da Rodada 1 (PIE com 2 players, mais um 3º client para party)

| Cenário | Esperado |
|---|---|
| Andar 1 minuto | Output Log sem `[YawSend]`/`[YawRecv]` |
| Jogar normalmente | Nenhum texto de debug na tela |
| P1 convida P2, P2 aceita, P3 fora do grupo | P3 não mostra painel de party nem 403 no log |
| P2 sai do grupo | P2 sem 403; P1 fica sem grupo e o painel some |
| Grupo de 3, um sai | Os 2 restantes continuam no grupo e o painel atualiza |
| Desconectar P2 com grupo de 2 | P1 fica sem grupo |

---

## Rodada 2 — Trade e drag&drop em C++

### R2.1 `WBP_Trade` em C++ (`UUmbraTradeWidget`)

**Hoje:**
- polling em BP de `get_trade_state.php` (40 chamadas num trade curto);
- o HUD inicia o trade por reflexão (`UmbraHud::CallWidgetFunction("InitializeTrade")`).

**Plano:**
- classe C++ parent do `WBP_Trade` com binds `BindWidgetOptional` (slots de oferta, ouro, aceitar/recusar);
- polling por `FTimerHandle` só enquanto a janela estiver aberta, com intervalo configurável e parada no Destruct;
- `InitializeTrade(int32)` passa a ser `UFUNCTION` C++ e o HUD chama direto, sem reflexão;
- slots de troca como drop target C++ (`bIsTradeDropTarget`, no padrão de refino/enchant).

**Opcional:** opcode WS de "estado do trade mudou" para trocar o polling por push (skill `/novo-opcode`; o último em uso é 118).

### R2.2 `WBP_ReportPlayer` em C++

- `ShowReportDialog(int32, FString)` como `UFUNCTION` C++; tira a última ponte por reflexão do HUD.

### R2.3 Drag&drop do inventário em C++ — **adiada (sob demanda)**

> Decisão de 2026-11-08: o BP atual funciona e foi validado no QA. A migração não corrige bug visível e tem risco alto (inventário, equipamento, storage, loja, refino, enchant, crafting, split). Fazer quando houver motivo para mexer nos slots.

- Hoje o `OnDragDetected` está em BP no `WBP_InventorySlot` e no `WBP_EquipmentSlot` (com `Print String`).
- Passar a detecção para `NativeOnMouseButtonDown` (`DetectDrag`) + `NativeOnDragDetected`, usando o `CreateItemDragOperation` que já existe. Mesmo padrão do `UUmbraSkillSlotWidget`.
- Apagar os grafos de drag dos WBPs via MCP.
- QA amplo: inventário↔inventário, equipar/desequipar, storage, trade, refino, enchant, crafting, loja pessoal, NPC vendor, lixeira.

### R2.4 `WBP_DeleteItem` instância única

- O `WBP_Inventory` cria uma janela nova a cada clique.
- O C++ do `UUmbraInventoryWidget` passa a dono de uma instância única (abre e fecha), e o nó do graph é apagado.

### R2.5 Avisos de party só para os membros (servidor)

- A zone resolve os membros do grupo e manda com `sendToPlayerUnlocked`, sem broadcast.
- Exige consultar os membros fora do caminho de recv: worker/cache, nunca DB no callback do WebSocket (CLAUDE.md §5).

---

## Rodada 3 — Conteúdo e mapa

### R3.1 Restaurar `Content/Necropolis` (950 arquivos)

- Os arquivos sumiram do projeto e o `Lvl_Tutorial_` gera 46 erros de LevelInstance por PIE.
- Eles existem no commit `11ef18f` (branch `backup/mega-commit-11ef18f`); não estão no `develop`.
- **Feito na Rodada 1:** `git restore --source=11ef18f --worktree -- Content/Necropolis` (só disco, fora do index).
- **Não versionar:** são 2,19 GB sem LFS (passa do limite de 2 GB por push do GitHub), no mesmo padrão dos outros pacotes pesados.
- **Pendente:** decidir se o conteúdo pesado vai para Git LFS ou para outro armazenamento compartilhado.

### R3.2 Sons de editor em UI

- 38 referências a `/Engine/VREditor/...` em Login2, CreateCharacter, `WBP_SkillBookEntry`, Storage e slots.
- Trocar por SoundWaves do projeto em `/Game/Audio/UI/` (o usuário importa; Claude reaponta via MCP).

---

## Rodada 4 — Telas de entrada em C++ e dívida técnica

### R4.1 Login, cadastro, seleção e criação de personagem em C++

- Assets: `WBP_Login2`, `WBP_Register`, `WBP_CharacterSelection`, `WBP_CharacterItem`, `WBP_CharacterSelectButton`, `WBP_CreateCharacter` e `BP_CharacterSelectionManager`.
- Hoje a lógica está toda em BP, com 46 `Print String`.
- Fluxo protegido (AGENTS.md §3):
  1. login HTTP via VaRest;
  2. token/conta;
  3. personagens;
  4. TCP com delay.
- Migrar tela a tela, com QA de login completo a cada etapa.
- A ordem de autenticação não muda.

### R4.2 Warnings de parse JSON (VaRest)

- Cerca de 7 mil linhas "No field with name `zone_id`/`pos_x`/`radius`/`item_template_id`/`npc_template_id`".
- O parse lê campos opcionais com `GetNumberField` sem `HasField` (ex.: `UmbraGameInstance.cpp` ~790 e ~11794).
- Criar um helper `GetOptionalNumber(Obj, Field, Default)` e aplicar nos parsers.

### R4.3 Depreciações e limpeza

- **C4996 `UImage::Brush`:** usar `GetBrush()`/`SetBrush()` em `UmbraBuffIconWidget.cpp`, `UmbraInventorySlotWidget.cpp:50` e `UmbraSkillSlotWidget.cpp`.
- **`WBP_SkillBook` × `WBP_SkillBook1`:** avaliar a consolidação; hoje os dois são usados por funções diferentes do PlayerController.
- **`BP_NetMovementClient2`:** apagar se a R1.2 confirmar que não é usado.
