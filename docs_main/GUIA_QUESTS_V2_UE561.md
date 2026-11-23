# GUIA — Quests v2: objetivos e recompensas novos, repetição e restrições (UE 5.6.1)

Esta etapa (3a) reorganiza o sistema de quests:
- o progresso passa a ser **decidido pela zone**, com um cache em memória dos objetivos ativos de cada jogador;
- entram **8 objetivos novos** e **4 recompensas novas**, repetição diária/semanal, nível máximo, classe, categoria e tempo limite.

O **C++ e o PHP já fazem tudo**. Não há WBP novo: o diário mostra categoria, repetição e prazo no título, e o HUD já tem os textos dos tipos novos. Você só cadastra as quests no Manager e testa.

## Objetivo

- **Objetivos novos:**

  | Tipo | O que conta | Quem credita |
  |---|---|---|
  | `kill` (estendido) | Vários alvos para o mesmo objetivo (`npc_template_ids`) | zone |
  | `kill_any` | Qualquer mob numa faixa de nível (e zona, opcional) | zone |
  | `use_skill` | Cast aceito (mana e cooldown pagos); `skill_id` 0 = qualquer skill | zone |
  | `use_consumable` | Uso de consumível pela zone (jogador **ou elemental**), por item ou subtipo | zone |
  | `earn_gold` | Ouro saqueado (janela de loot ou pet); a meta é o `amount` | zone |
  | `reach_level` | Atingir o nível; já conta ao aceitar se o jogador tiver o nível | zone + PHP |
  | `craft_item` | Craft concluído (por item, ou 0 = qualquer); um lote de N conta N | PHP (crafting) |
  | `refine_item` | Refino bem-sucedido que chega a +N | PHP (refino) |

- **Mudanças de segurança:**
  - `reach_area` passa a ser conferido **pela zone com a posição real** a cada 1 s;
  - `use_item_at` também: no local com o item na bolsa, a zone conta sozinha (e consome o item se `consume` estiver marcado). Há ainda o uso explícito pelo **opcode 132**;
  - o cliente não reporta mais posição: `report_quest_progress.php` responde 409.
- **Recompensas novas:**
  - `skill_points`: vão para `bonus_points` e **não somem no level up**;
  - `stat_points`;
  - `companion`: `amount` = `companion_id`; se o jogador já tiver o companheiro, não duplica;
  - `guild_xp`: vai para a guilda do jogador e para a contribuição dele; sem guilda, ignora.
- **Repetição:** `none` (uma vez), `always` (repete na hora), `daily` (1x por dia, reinicia à 00h do servidor), `weekly` (1x por semana, reinicia na segunda). As quests antigas com `repeatable=1` viraram `always`.
- **Restrições:** `max_level` (0 = livre), `class_ids` (vazio = todas), `time_limit_minutes`. Ao estourar o tempo, a quest vira `failed`, sai do diário e pode ser aceita de novo.
- **Categoria:** `main` / `side` / `daily` / `event`, mostrada no título do diário.
- **Protocolo:** **131** (`QuestStateChanged`) e **132** (`QuestInteract`). O **116** ganhou a flag bit2 = "usar item no local" recusado.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `quests.repeat_type` | Manager → NPCs → Quests → "Repetição" | none / always / daily / weekly |
| `quests.quest_category` | "Categoria" | main / side / daily / event |
| `quests.max_level` | "Max Lv (0=livre)" | Bloqueia acima do nível |
| `quests.class_ids` | "Classes (ex.: 1,3)" | Vazio = todas |
| `quests.time_limit_minutes` | "Tempo (min, 0=livre)" | Prazo desde o aceite |
| Objetivos | Grade de objetivos + campos do objetivo selecionado | Veja a tabela abaixo |
| Recompensas | Grade de recompensas (tipo novo) | `companion` usa Amount = id do companheiro |

**Campos por objetivo (Manager):**

| Tipo | Alvo ID | Qtd | Campos extras |
|---|---|---|---|
| `kill` | NPC template | quantidade | "alvos extras (6,9)" |
| `kill_any` | — | quantidade | nível mín./máx., `zone_id` (0 = qualquer) |
| `use_skill` | skill (0 = qualquer) | quantidade | — |
| `use_consumable` | item (0 = qualquer) | quantidade | subtipo (ex.: `health_potion`) |
| `earn_gold` | — | **ouro** | — |
| `reach_level` | — | **nível** | — |
| `craft_item` | item (0 = qualquer) | quantidade | — |
| `refine_item` | — | quantidade | refino mín. (+N) |
| `use_item_at` | — | quantidade | item, zone/pos/raio, **consome o item** |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_quests_v2.sql
```

O script também recria o gatilho `trg_update_skill_points_on_level` para somar `bonus_points`.

---

## 0) Pré-requisitos

1. `zone_server` recompilado (já feito) e **reiniciado**.
2. Manager: fechar e rodar `dotnet build tools/UmbraManagerWpf/UmbraManager.sln`.
3. Rebuild do módulo `UmbraEternumUE`.
4. PHP já sincronizado com o WAMP.

Nenhum asset novo no Editor.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `QuestProgressService` | `src/zone/QuestProgressService.cpp` | Thread própria com fila. Cache dos objetivos ativos por jogador (TTL 60 s, recarregado na hora pelo 131), crédito por evento, varredura de área e de "usar item no local" a cada 1 s, 132, respeita o tempo limite; grava só quando um objetivo casa e notifica o 116 |
| Fontes de evento | `CombatCoreEngine.cpp` (kill + nível do NPC, skill), `ConsumableService.cpp` (consumível), `LootService.cpp` (ouro), `ExperienceService.cpp` (level up) | Só enfileiram |
| PHP | `helpers/quest_helper.php` | Repetição, classe e nível máx., tempo limite (`failed`), recompensas novas, `questCreditEvent` (craft/refino), `reach_level` no aceite |
| UE | `Core/UmbraGameInstance.cpp` | Parse dos tipos novos, 131 depois de aceitar/abandonar/entregar, `ReportQuestProgress` → 132, aviso quando o 116 traz a flag de recusa |
| UE | `UI/UmbraQuestJournalWidget.cpp`, `UI/UmbraQuestHudTrackerWidget.cpp`, `UI/UmbraQuestWidgetLayout.cpp` | Tags no título (Principal/Evento, Diária/Semanal/Repetível, "expira em N min"), textos dos objetivos e recompensas novos |
| UE | `Components/UmbraQuestAreaTrackerComponent.cpp` | Não reporta mais área sozinho (a zone faz) |

---

## 4) Protocolo

### 131 `QuestStateChanged` (cliente → zone)
`[questId:u32]`, 5 bytes. É enviado depois de aceitar, abandonar ou entregar pelo HTTP; a zone recarrega o cache de objetivos do jogador.

### 132 `QuestInteract` (cliente → zone)
`[questId:u32][itemTemplateId:u32]`, 9 bytes. A zone confere a posição real e o item na bolsa (sem equipado nem leilão); se o objetivo tiver `consume`, desconta 1.

### 116 `QuestProgressNotify` (existente)
`[playerId:u32][questId:u32][status:u8][flags:u8]`, com as flags:
- bit0 = progresso;
- bit1 = ficou pronta;
- **bit2 = usar item no local recusado** (o cliente mostra "Vá até o local indicado com o item da missão.").

---

## 6) QA (PIE)

PIE com **2 jogadores**. Para cada caso, cadastrar uma quest de teste no Manager num NPC.

| # | Cenário | Esperado |
|---|---------|----------|
| Q1 | `kill` com 2 alvos (6,9), Qtd 3 | Matar qualquer um dos dois conta; quem está no grupo e perto também conta |
| Q2 | `kill_any` nível 1–5, Qtd 5 | Só mobs na faixa contam |
| Q3 | `use_skill` (0), Qtd 5 | Cada cast aceito conta; cast recusado (sem mana) não |
| Q4 | `use_consumable` subtipo `health_potion`, Qtd 2 | Poção de vida conta; poção de mana não; **elemental usando a poção também conta** |
| Q5 | `earn_gold` 100 | Ouro pela janela de loot e pelo pet soma até 100 |
| Q6 | `reach_level` 2 com personagem nível 1 | Ao subir de nível a quest fica pronta. Aceitar já no nível 2 = pronta na hora |
| Q7 | `craft_item` (0), Qtd 2 | Cada craft conta; craft de lote ×2 completa |
| Q8 | `refine_item` +3, Qtd 1 | Refino que chega a +3 conta; falha de refino não |
| Q9 | `reach_area` | Andar até o local completa sozinho (sem HTTP); ficar longe não completa |
| Q10 | `use_item_at` com "consome" | No local com o item: completa e o item some. Sem o item: nada |
| Q11 | Repetição `daily` | Entregar → some do NPC até a 00h; `always` reaparece na hora; `none` nunca mais |
| Q12 | `max_level` 1 com personagem nível 2 | A quest não aparece no NPC |
| Q13 | Classes `1` com personagem de outra classe | Não aparece |
| Q14 | Tempo limite 1 min | Título mostra "expira em 1 min"; depois de 1 min a quest some do diário e pode ser aceita de novo |
| Q15 | Recompensas `skill_points` 2 e `stat_points` 5 | Pontos aparecem; **subir de nível mantém os 2 pontos de skill** |
| Q16 | Recompensa `companion` (id 1) | Jogador ganha a Fada Coletora; de novo não duplica |
| Q17 | Recompensa `guild_xp` 500 (com guilda) | EXP da guilda sobe; sem guilda a quest completa normal |
| Q18 | Chamar `report_quest_progress.php` (curl) | 409 `use_via_zone` |

Logs esperados na zone:
- `"[QuestProgressService] player X quest Y objetivo Z → N (ativa|pronta)"`;
- `"usar item no local recusado player X quest Y: fora do local"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Objetivo novo não avança | Zone antiga, ou o cache não recarregou (cliente antigo não manda o 131) | Reiniciar a zone nova; recompilar o UE; o TTL de 60 s recarrega sozinho |
| "Erro ao salvar quest: objective_type inválido" | PHP/SQL antigos | Rodar `add_quests_v2.sql` e sincronizar o PHP |
| `reach_area` não completa | `zone_id` do objetivo diferente da zone atual ou raio pequeno | Conferir `zone_id`, pos e raio (+50 uu de folga) |
| `craft_item`/`refine_item` não contam | Craft/refino feitos antes de aceitar a quest | Só contam depois do aceite |
| Pontos de skill de quest somem | Banco sem o gatilho novo | Rodar `add_quests_v2.sql` de novo (recria o gatilho) |

## 8) Referência de código

- Servidor:
  - `src/zone/QuestProgressService.{hpp,cpp}`;
  - `src/zone/MovementProtocol.hpp` (131/132);
  - `src/zone/MovementServer.hpp`, `ZoneServer.cpp`;
  - `ExperienceService.cpp` (`bonus_points` e callback de level up);
  - `tests/test_quest_events.cpp`.
- PHP/SQL:
  - `scripts/add_quests_v2.sql`;
  - `helpers/quest_helper.php`, `helpers/experience_helper.php`;
  - `api/admin/quest_admin_helpers.php`, `quest_schema_helpers.php`, `create_quest.php`, `update_quest.php`, `get_quest_admin.php`;
  - `api/crafting/craft.php`, `api/refinement/refine_item.php`, `api/quest/report_quest_progress.php` (desativado).
- Manager: `Models/QuestObjectiveRow.cs`, `ViewModels/MainViewModel.NpcVendorQuest.cs`, `Views/Npcs/NpcEditorView.xaml`.
- UE:
  - `Data/UmbraDataStructures.h`;
  - `Core/UmbraGameInstance.{h,cpp}`;
  - `UI/UmbraQuestJournalWidget.cpp`, `UI/UmbraQuestHudTrackerWidget.cpp`, `UI/UmbraQuestWidgetLayout.cpp`;
  - `Components/UmbraQuestAreaTrackerComponent.cpp`.

**Fim do guia.**
