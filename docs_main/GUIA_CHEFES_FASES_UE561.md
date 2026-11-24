# GUIA — Chefes com fases (UE 5.6.1)

A etapa 5b faz o chefe mudar de comportamento ao perder vida (fases), chamar ajudantes (adds), se enfurecer se a luta demorar (enrage) e se reiniciar se o grupo morrer ou fugir (reset). A configuração é **por template de NPC**, no Manager. Plano: `PLANO_ETAPA5B_CHEFES_FASES_UE561.md`.

O **C++ e o PHP já fazem tudo**. No Editor você só cria `WBP_BossBar`. Se você pedir, eu monto pelo MCP depois de compilar.

## Objetivo

- **Barra do chefe:** com o chefe em combate, aparece no topo da tela:
  - nome, vida em %, "Fase 2/3 — Fúria";
  - as marcas dos limiares na barra;
  - "Enfurece em 2:30", "ENFURECIDO" ou "Imune".
- **Troca de fase:** ao cruzar o limiar de vida, o chefe:
  - fala no centro da tela;
  - toca a animação e o VFX de transição;
  - fica imune por N ms (opcional);
  - troca para as skills da fase e passa a usar os multiplicadores dela (dano, velocidade, intervalo de ataque);
  - chama os adds da fase.
  - Um golpe que pula fases aplica todas de uma vez: os adds de todas, a fala e a animação só da última.
- **Enrage:** depois de N s em combate, o dano é multiplicado e aparece o aviso ("Fulano se enfurece!" ou o texto do Manager).
- **Reset:** se o chefe perde o alvo e volta para casa (wipe ou fuga):
  - volta à fase 1 com vida cheia;
  - os adds somem;
  - o enrage zera.
- **Morte do chefe:** a barra some e os adds dele somem junto.
- **Adds:**
  - só nascem **dentro de instância**;
  - já chegam atacando quem bateu no chefe;
  - **não contam** em "Inimigos" da dungeon.
- **Snapshot:** quem entra na cópia no meio da luta recebe o estado atual do chefe.

**Protocolo:** **140** `BossState` (novo). A vida continua vindo pelo **102**.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `npc_boss_configs.*` | Manager → **NPCs e mundo** → sub-aba **Chefe** (template selecionado em **Templates**) | "É chefe", enrage (segundos, × dano, aviso), reiniciar ao perder o alvo, barra no topo |
| `npc_boss_phases.*` | grade **Fases** | Nome, **Vida ≤ %**, × Dano, × Velocidade, × Intervalo de ataque (< 1 = mais rápido), imunidade (ms) |
| fala / animação / VFX | painel da fase selecionada | Texto e soft paths (`/Game/.../Asset.Asset`) |
| `npc_boss_phase_skills.*` | grade **Skills da fase** | Skill ID, rank, peso, CD (0 = da skill). Vazio = skills do template |
| `npc_boss_phase_adds.*` | grade **Adds ao entrar na fase** | Template do add, quantidade, raio |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_boss_phases.sql
```

Sem seed: cadastre pelo Manager.

---

## 0) Pré-requisitos

1. `zone_server` recompilado e **reiniciado**. O link falhou porque a zone estava aberta: feche-a e rode `cmake --build . --config Release --target zone_server`.
2. Manager recompilado: feche o aberto e rode `dotnet build tools/UmbraManagerWpf/UmbraManager.sln`.
3. Rebuild do módulo `UmbraEternumUE` (classe nova `UmbraBossBarWidget`; `UmbraGameInstanceBoss.cpp`).
4. PHP já sincronizado com o WAMP.

**Asset:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_BossBar` | `/Game/Widgets/UI/Instance/WBP_BossBar` | `UmbraBossBarWidget` |

## 1) Designer — `WBP_BossBar`

Faixa larga no topo central, abaixo do HUD da instância (ex.: 640 × 70, Anchor topo-centro, Y ≈ 110).

| Bind | Tipo | Função |
|---|---|---|
| `Text_BossName` | TextBlock | Nome do chefe |
| `ProgressBar_BossHealth` | ProgressBar | Vida (lida do ator, que o 102 atualiza) |
| `Canvas_BossMarkers` | CanvasPanel | **Por cima da barra, com o mesmo tamanho** (Overlay com a barra embaixo). O C++ desenha uma marca por limiar |
| `Text_BossHealthPct` | TextBlock | "72%" |
| `Text_BossPhase` | TextBlock | "Fase 2/3 — Fúria" |
| `Text_BossStatus` | TextBlock | "Enfurece em 2:30" / "ENFURECIDO" (vermelho) / "Imune" (dourado) |

Class Defaults: **Marker Color** e **Marker Width** (padrão dourado, 2 px).

No `BP_UmbraGameInstance` (opcional): **Boss Bar Widget Class** = `WBP_BossBar`. Vazio = path padrão.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `BossController` | `src/zone/BossController.*` | Configs em memória (`reload_boss_configs`); troca de fase, morte, enrage, reset e entrada em combate, sempre **no combat worker**; adds; snapshot do 140 |
| `NpcManager` | `src/zone/NpcManager.*` | Estado de chefe no NPC (fase, multiplicadores, imunidade, skills base); **protótipo por template** para os adds (query separada; o SELECT posicional não muda); `spawnTemplateClones`/`removeBossAdds`; o respawn volta à fase 1 |
| Combate | `CombatCoreEngine.cpp`, `CharacterStateLoader.cpp`, `NpcAiSystem.cpp` | Dano → fase; morte → barra/adds; × dano no atacante; × velocidade e × intervalo na IA; imunidade ignora o dano |
| Dungeon | `InstanceManager.cpp` | Adds (papel 4) não contam |
| UE | `Core/UmbraGameInstanceBoss.cpp` | 140 → cache por NPC, fala (`ShowCombatFeedback`), animação (`PlayNpcAnimFromPath`), VFX, barra |
| UE | `UI/UmbraBossBarWidget.*` | Vida, %, fase, marcas, enrage/imune; some com o chefe morto |

## 3) Protocolo

| Op | Direção | Formato |
|----|---------|---------|
| **140** `BossState` | S→C (camada do chefe) | `[140][npcId:u32][phase:u8 (1-based)][phaseCount:u8][flags:u8 bit0 enfurecido, bit1 imune, bit2 em combate, bit3 mostrar barra][enrageInSec:u16][invulnerableMs:u16][phaseName:u16+utf8≤64][announce:u16+utf8≤200][animPath:u16+utf8≤255][vfxPath:u16+utf8≤255][n:u8]{pct:u8}` |

Quando é enviado:
- na entrada em combate (por dano ou aggro);
- na troca de fase, com `announce`, `animPath` e `vfxPath`;
- no enrage, com `announce`;
- no reset e na morte (sem o bit2: a barra some);
- no snapshot de quem entra na camada.

## 4) Manager

1. **NPCs e mundo** → **Templates** → selecione o template do chefe → sub-aba **Chefe**. A config carrega sozinha.
2. Marque **É chefe** e clique **+ Fase**:
   - a fase 1 vale do início;
   - a 2ª e as seguintes já vêm com limiar 30% abaixo da anterior, ×1,2 de dano e 2 s de imunidade.
3. Selecione uma fase e preencha a fala, a animação e o VFX (soft path completo). Em **+ Skill** e **+ Add**, coloque os IDs.
4. Enrage: segundos (0 = sem), × dano e o aviso.
5. **Salvar chefe**. O Manager manda o `reload_boss_configs` às zones.
6. Para a dungeon contar o chefe, ele também precisa do **Papel na dungeon** (`boss`/`final_boss`) na aba **Instâncias no mundo** (etapa 5).

Regras validadas no PHP:
- limiares estritamente decrescentes;
- até 10 fases;
- o chefe não pode ser add dele mesmo;
- skills e templates precisam existir;
- soft path no formato `/Game/...Asset.Asset`.

---

## 5) QA (PIE)

PIE com **2 jogadores** no mesmo grupo dentro de uma dungeon. Use um chefe com 3 fases (100 / 70 / 40), fase 2 com 2 adds e 2 s de imunidade, e enrage de 60 s.

| # | Cenário | Esperado |
|---|---------|----------|
| B1 | Puxar o chefe | Barra no topo: nome, 100%, "Fase 1/3", 2 marcas (70 e 40), "Enfurece em 1:00" |
| B2 | Levar a 70% | Fala no centro; animação/VFX; "Imune" por 2 s (dano não entra); "Fase 2/3 — …"; 2 adds atacando; skills da fase |
| B3 | Golpe de 80% → 30% | Fases 2 e 3 de uma vez: adds das duas, fala da última, "Fase 3/3" |
| B4 | Esperar 60 s em combate | Aviso de enrage; "ENFURECIDO"; dano visivelmente maior |
| B5 | Fugir/morrer (o chefe volta para casa) | Barra some; adds somem; ao puxar de novo, fase 1, vida cheia, enrage zerado |
| B6 | Matar o chefe com adds vivos | Barra some; adds somem; a dungeon conclui se ele for `final_boss` (adds não contam) |
| B7 | B entra na cópia com a luta na fase 2 | B vê a barra na fase 2 (snapshot) |
| B8 | NPC sem config de chefe | Nada muda (sem barra, sem 140) |
| B9 | Chefe no mundo aberto com adds | Fases e enrage funcionam; adds **não** nascem (só em instância) |

Logs esperados:
- `[BossController] chefe N (tpl T) entrou na fase 2/3 (2 add(s))`;
- `… enfureceu`;
- `… reiniciado (wipe/leash)`.

## 6) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Sem barra | Template sem "É chefe", zone sem reload ou `WBP_BossBar` ausente | Salvar de novo no Manager; conferir o asset ou **Boss Bar Widget Class** |
| Fase não troca | Limiares fora de ordem ou chefe não tomou dano depois de salvar | Conferir a grade; bater de novo (a checagem é no dano) |
| Adds não aparecem | Chefe fora de instância, ou template do add inválido | Só dentro de instância; ver o log `[NpcManager] protótipo do template … inválido` |
| Chefe reseta sozinho | Leash curto: ele volta para casa durante a luta | Aumentar `leash_radius` do template ou da instância |
| Animação não toca | Soft path errado | Usar `/Game/.../Asset.Asset` de uma Montage/Sequence compatível com o esqueleto |
| Depois do wipe, o chefe alterna entre "em combate" e "fora de combate" | DOTs dos jogadores mortos continuavam batendo nele | Corrigido: o reset do chefe limpa DOTs e debuffs dos jogadores (log `efeito(s) hostil(is) removido(s) no reset`) |
| Personagem morto volta a andar | CC do golpe letal acabava e devolvia o movimento | Corrigido: o servidor não aplica CC/buff em alvo morto e o cliente não devolve o movimento antes do respawn |
| Duas barras de vida no topo | Quadro de alvo e barra do chefe mostravam o mesmo NPC | Corrigido: com a barra do chefe visível, o quadro de alvo do mesmo chefe fica invisível e volta quando a luta acaba |

## 7) Referência de código

- **Servidor:**
  - `src/zone/BossController.{hpp,cpp}`;
  - `src/zone/NpcManager.{hpp,cpp}`;
  - `src/zone/CombatCoreEngine.{hpp,cpp}`;
  - `src/zone/CharacterStateLoader.cpp`;
  - `src/zone/NpcAiSystem.cpp`;
  - `src/zone/InstanceManager.cpp`;
  - `src/zone/MovementProtocol.hpp` (140);
  - `src/zone/ZoneServer.{hpp,cpp}`;
  - `src/admin/ServiceAdminRegister.cpp` (`reload_boss_configs`);
  - `tests/test_boss_phases.cpp`.
- **PHP/SQL:**
  - `scripts/add_boss_phases.sql`;
  - `api/admin/get_boss_config.php`, `save_boss_config.php`.
- **Manager:**
  - `Models/BossPhaseRow.cs`, `BossPhaseSkillRow.cs`, `BossPhaseAddRow.cs`;
  - `ViewModels/MainViewModel.BossPhases.cs`;
  - `Views/Npcs/NpcEditorView.xaml` (sub-aba Chefe);
  - `Services/PhpAdminClient.cs`.
- **UE:**
  - `Data/UmbraBossTypes.h`;
  - `Core/UmbraGameInstanceBoss.cpp`, `Core/UmbraGameInstance.{h,cpp}`;
  - `Network/NetMovementClient.cpp`;
  - `UI/UmbraBossBarWidget.*`.

**Fim do guia.**
