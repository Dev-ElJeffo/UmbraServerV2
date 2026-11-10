# GUIA — Modo combate / relaxado (sacar e guardar a arma) (UE 5.6.1)

Uma tecla alterna a personagem entre **relaxada** (arma nas costas, idle relaxada) e **combate** (arma nas mãos, idle de combate), com animações de **sacar** e **guardar**. Atacar ou usar skill relaxada **saca automático**. O estado é **replicado** (opcode **119**). Quase tudo é C++: no Editor você cria o AnimBP do player, o socket nas costas e a Input Action.

## Objetivo

- **Relaxado:**
  - arma no socket `WeaponBack`;
  - idle/locomoção do conjunto normal (`idle`, `walk`, `run` do `anim_set_json` da classe).
- **Combate:**
  - arma no `HandGrip_R`;
  - conjunto de combate (`combat_idle`, `combat_walk`, `combat_run`). Slot vazio usa o normal.
- **Tecla (padrão sugerido Z):**
  - entra em combate com a anim `draw` (a arma vai para a mão no `draw_swap_ms`);
  - sai com `sheathe` (a arma vai para as costas no `sheathe_swap_ms`).
- **Ataque básico / skill estando relaxado:** entra em combate na hora, sem anim de sacar. O golpe cobre a transição.
- **Rede:**
  - opcode **119** `CombatStanceState` (cliente → zone → todos);
  - quem entra no alcance recebe o estado no snapshot inicial;
  - ao logar, todos começam relaxados.
- **O que não muda:** opcodes de combate (96–99), movimento (25B/34B), PHP e SQL. As chaves novas vão no `anim_set_json` que já existe.

## Dados / Manager

| Chave no `classes.anim_set_json` | Manager → Classes → "Modo combate" | Uso |
|---|---|---|
| `idle` (já existia) | "Idle" (Animation Set) | Idle **relaxada** |
| `combat_idle` | "Idle de combate" | Idle em combate |
| `draw` | "Sacar a arma" | Anim de sacar (Sequence ou Montage) |
| `draw_swap_ms` | "Sacar: ms em que a arma vai p/ a mão" | Ms em que a arma troca de socket costas → mão |
| `sheathe` | "Guardar a arma" | Anim de guardar |
| `sheathe_swap_ms` | "Guardar: ms em que a arma vai p/ as costas" | Ms em que a arma troca de socket mão → costas |
| `combat_walk.fwd/bwd/left/right` | "Combate: walk …" | Andar em combate (vazio = andar normal) |
| `combat_run.fwd/bwd/left/right` | "Combate: run …" | Correr em combate (vazio = correr normal) |

**Sem SQL e sem PHP novos:** o `anim_set_json` é gravado como JSON livre (`class_admin_helpers.php`).

**Valores da Barbarian (animações prontas):**

| Campo | Valor |
|---|---|
| Idle | `/Game/Game/Animations/Combat/Barbarian/A_Barb_F_Idle_Relaxed` |
| Idle de combate | `/Game/Game/Animations/Combat/Barbarian/A_Barb_F_Idle_Combat` |
| Sacar | `/Game/Game/Animations/Combat/Barbarian/A_Barb_F_DrawWeapon` |
| Sacar ms | **267** |
| Guardar | `/Game/Game/Animations/Combat/Barbarian/A_Barb_F_SheatheWeapon` |
| Guardar ms | **433** |
| Walk fwd / bwd / left / right | `/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd` / `_Bwd` / `_Left` / `_Right` |
| Run fwd / bwd / left / right | `/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd` / `_Bwd` / `_Left` / `_Right` |

> Walk/Run são as mesmas animações do `BS_Idle_Walk_Run` do `ABP_Unarmed`. Sem elas o player fica em idle ao andar (ou em T-pose sem a idle).

### Já feito no Editor via MCP (2026-11-09)

| Item | Estado |
|---|---|
| Socket `WeaponBack` no `SK_Mannequin` (`spine_05`) | Valor **atual (ajuste visual do usuário)**: Location (-7.42133, -5.14999, -125.569), Rotation P/Y/R (-8.2758, -51.5534, 14.66331), Scale 1.75. Valor calculado original (pegada das anims Draw f8 / Sheathe f13): Location (6.372, -89.639, -98.455), Rotation (-33.584, -35.354, 42.163). Se a arma "pular" na troca, as anims Draw/Sheathe precisam ser refeitas para a pegada nova. |
| `IA_ToggleCombat` (Boolean) + tecla **Z** no `/Game/Input/IMC_Default` | Criados e salvos |
| 4 animações em `/Game/Game/Animations/Combat/Barbarian/` | Importadas no `SK_Mannequin`. Idles com **Loop** ligado. |
| `/Game/Characters/Mannequins/Anims/ABP_PlayerHumanoid` (parent `UmbraNpcAnimInstance`) | AnimGraph montado: `IdleSequence`/`WalkFwdSequence`/`RunFwdSequence` → Blend por `bIsRunning` → Blend por `bIsMoving` → Slot `DefaultSlot` → Output. Compila. |
| `BP_ThirdPersonCharacter` → Mesh → **Anim Class** | `ABP_PlayerHumanoid_C` (antes `ABP_Unarmed_C`) |

Pendências do ABP (podem ser feitas depois):
- **locomoção em 4 direções:** hoje usa só os clips *Fwd* para qualquer direção (ver [GUIA_NPC_ANIM_STATES_UE561.md](GUIA_NPC_ANIM_STATES_UE561.md) §4);
- **pulo:** o `bIsInAir` ainda não está ligado.

FBX (Blender → UE): `Downloads\UmbraAnimExport\` (`A_Barb_F_Idle_Relaxed.fbx`, `A_Barb_F_Idle_Combat.fbx`, `A_Barb_F_DrawWeapon.fbx` e `A_Barb_F_SheatheWeapon.fbx`). Import igual aos golpes: Skeleton `SK_Mannequin`, sem mesh, *Exported Time*, escala 1. Nas duas idles, marque **Loop** na AnimSequence.

---

## 0) Pré-requisitos

1. **Rebuild** do módulo `UmbraEternumUE`. O header do personagem e do AnimInstance mudou, então Live Coding não basta.
2. `zone_server` recompilado e **reiniciado** (opcode 119).
3. **Parar o PIE** antes de mexer em assets.

**Assets:**

| Asset | Path | Parent Class / tipo |
|---|---|---|
| `ABP_PlayerHumanoid` | `/Game/Characters/Mannequins/Anims/ABP_PlayerHumanoid` | `UmbraNpcAnimInstance` |
| `IA_ToggleCombat` | `/Game/Input/Actions/IA_ToggleCombat` | Input Action (Digital/bool) |
| Socket `WeaponBack` | `SK_Mannequin`, filho do osso `spine_05` | Socket |

---

## 1) AnimBP do player (obrigatório)

Hoje o `BP_ThirdPersonCharacter` usa o `ABP_Unarmed` do template, que **não lê** o `anim_set_json`. Sem trocar o AnimBP, nem as idles nem o modo combate aparecem.

1. **Content Browser** → `/Game/Characters/Mannequins/Anims/` → botão direito → **Animation → Animation Blueprint**.
2. Skeleton: **SK_Mannequin**. Parent Class: clique em **All Classes** → `UmbraNpcAnimInstance`. Nome: `ABP_PlayerHumanoid`.
3. Monte o **AnimGraph** exatamente como no [GUIA_NPC_ANIM_STATES_UE561.md](GUIA_NPC_ANIM_STATES_UE561.md):
   - §4: locomoção `IdleSequence` / `Walk*Sequence` / `Run*Sequence`, com `bIsMoving`, `bIsRunning` e `MoveDirection`;
   - §5: **Slot `DefaultSlot`** na saída da locomoção (obrigatório para sacar/guardar e golpes).
4. **Pulo (opcional, recomendado):** antes do `DefaultSlot`, um **Blend Poses by Bool** com `bIsInAir` (True = a anim de pulo/queda que o `ABP_Unarmed` usava, False = a locomoção).
5. **Compile** + **Save**. Não use o Event Graph: as variáveis são preenchidas pelo C++.
6. Abra `BP_ThirdPersonCharacter` → componente **Mesh** → **Anim Class** = `ABP_PlayerHumanoid_C`. **Compile** + **Save**.
7. No Manager → Classes → Barbarian, deixe **anim_blueprint_path** vazio ou com `/Game/Characters/Mannequins/Anims/ABP_PlayerHumanoid.ABP_PlayerHumanoid_C`. O valor atual (`ABP_Unarmed` sem `_C`) falha ao carregar.

> As sequences ativas trocam sozinhas quando o estado muda. O C++ troca o que está em `IdleSequence`, `Walk*Sequence` e `Run*Sequence`, então o AnimGraph não precisa saber do modo combate. Se quiser, `bCombatStance` também está disponível (só leitura).

## 2) Socket `WeaponBack` (arma nas costas)

O machado vai **invertido nas costas**: cabo para cima, saindo por trás do ombro direito, e lâmina embaixo, perto do quadril esquerdo. As animações foram feitas para a mão direita pegar o cabo **exatamente** no ponto de pegada, então a troca de socket não dá "pulo".

1. Abra **SK_Mannequin** (Skeleton). Na árvore: botão direito em **spine_05** → **Add Socket** → renomeie para `WeaponBack`.
2. Botão direito no socket `WeaponBack` → **Add Preview Asset** → a malha do machado. Faça o mesmo no `HandGrip_R`.
3. No painel de animação do Skeleton, escolha **A_Barb_F_DrawWeapon** e pause em **0,267 s** (f8): é o instante em que a mão segura o cabo nas costas.
4. Ajuste **Relative Location/Rotation** do `WeaponBack` até o machado do `WeaponBack` **coincidir** com o do `HandGrip_R` nesse frame.
   - Referência: o ponto de pegada fica ~18 cm à direita, ~14 cm atrás e ~8 cm acima da cabeça do osso `spine_05`.
   - O cabo desce em diagonal para o quadril esquerdo.
5. Confira também em **A_Barb_F_SheatheWeapon** em **0,433 s** (f13). Deve coincidir do mesmo jeito.
6. **Save**. Remova os preview assets se quiser (não afetam o jogo).

> Só armas **StaticMesh** trocam de socket. Arma SkeletalMesh (leader pose) segue o esqueleto e fica na mão. Sem o socket `WeaponBack`, a arma continua na mão e o log avisa `[Appearance] socket WeaponBack ausente`.

## 3) Input — `IA_ToggleCombat`

1. `/Game/Input/Actions/` → botão direito → **Input → Input Action** → `IA_ToggleCombat` (Value Type **Digital (bool)**).
2. Abra o **Input Mapping Context** do jogador (o mesmo do `IA_BasicAttack`) → **+** → `IA_ToggleCombat` → tecla **Z** (ou a que preferir). **Save**.
3. Não precisa ligar nada no BP. O C++ carrega `/Game/Input/Actions/IA_ToggleCombat` se o campo **Toggle Combat Stance Action** do `BP_ThirdPersonCharacter` estiver vazio.

---

## 4) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|---|---|---|
| `UUmbraCombatStanceComponent` | `Components/UmbraCombatStanceComponent.cpp` | Máquina Relaxado ↔ Combate:<br>• toca `draw`/`sheathe` (via `UmbraCombatComponent`);<br>• timer no `*_swap_ms` troca a arma de socket;<br>• `EnsureCombatForAttack()` para ataque/skill;<br>• envia o 119 e aplica o 119 dos remotos (auto = sem anim);<br>• cooldown de 0,5 s entre teclas. |
| `UUmbraNpcAnimInstance` | `Animation/UmbraNpcAnimInstance.cpp` | Conjuntos relaxado e combate. `SetCombatStance()` troca `Idle/Walk/Run*Sequence` (slot vazio usa o normal). `bIsInAir` para o pulo. `ParseStanceAnims()` lê `draw`/`sheathe`/`*_swap_ms`. |
| `UUmbraPlayerAppearanceComponent` | `Components/UmbraPlayerAppearanceComponent.cpp` | Arma da mão principal nasce em `WeaponBack` (relaxado). `SetMainHandWeaponSheathed()` reanexa em `WeaponBack` ↔ `HandGrip_R`. |
| `AUmbraEternumUECharacter` | `UmbraEternumUECharacter.cpp` | Cria o componente `CombatStance`; bind de `IA_ToggleCombat` → `ToggleStance()`. |
| `UUmbraBasicAttackComponent` | `Components/UmbraBasicAttackComponent.cpp` | Ataque básico relaxado → `EnsureCombatForAttack()` antes do envio. |
| `UUmbraGameInstance` | `Core/UmbraGameInstance.cpp` | `SendCombatStanceViaWebSocket`; handler do 119 → `OnCombatStanceChanged` (eco do próprio player ignorado); cast de skill relaxado → `EnsureCombatForAttack()`. |
| `NetMovementClient` | `Network/NetMovementClient.cpp` | Encaminha o 119 ao GameInstance. |

> **Não** adicione lógica no Event Graph.

---

## 5) Protocolo

### Opcode 119 (`CombatStanceState`)

`[119][playerId u32 LE][stance u8][flags u8]` — cliente → zone → **todos** (broadcast) e snapshot inicial para quem conecta.

| Campo | Tipo | Observação |
|---|---|---|
| `playerId` | `u32` | Precisa ser o player do `cid`. Outro id é ignorado e gera log `CombatStanceState ignorado`. |
| `stance` | `u8` | 0 = relaxado, 1 = combate |
| `flags` | `u8` | bit0 = **auto** (entrou atacando / snapshot) → remoto aplica **sem anim**. **Trailing**: frame de 6 bytes → 0. |

- **Servidor:**
  - `src/zone/MovementProtocol.hpp` (`encodeCombatStanceState`/`decodeCombatStanceState`);
  - `src/zone/MovementServer.hpp`: handler sem DB, `PlayerStateNet::combatStance` e snapshot. Sem mudança, não retransmite.
- **Cliente:** `ProcessSocialWebSocketMessage` (`MsgType == 119`) em `UmbraGameInstance.cpp`.
- **Teste:** `tests/test_zone.cpp` (`CombatStanceProtocolTest`).

## 6) Manager — passo a passo

1. **UmbraManager** → **Classes** → selecione **Barbarian**.
2. Em **Animation Set**: preencha **Idle** (relaxada) e Walk/Run normais.
3. Em **Modo combate**: Idle de combate, Sacar (267), Guardar (433). Combate walk/run por enquanto vazios.
4. **Salvar**. O cliente lê as classes no login: **relogue** para valer.

---

## 7) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---|---|
| Logar | Idle relaxada, machado nas costas (nos dois clientes). |
| Apertar Z | Sacar: a mão busca o cabo atrás do ombro, a arma passa para a mão em ~0,27 s, termina na guarda e a idle vira a de combate. O outro cliente vê igual. |
| Apertar Z de novo | Guardar: a arma volta às costas em ~0,43 s, idle relaxada. O outro cliente vê igual. |
| Atacar relaxada (LMB ou skill) | Arma na mão na hora (sem anim de sacar), golpe normal, fica em combate. O outro cliente vê a arma na mão (sem anim de sacar). |
| Spam de Z | No máximo 1 troca a cada 0,5 s. Sem arma "presa" no lugar errado. |
| Cliente 2 entra depois, com o 1 em combate | O 2 já vê o 1 com a arma na mão (snapshot). |
| Frame 119 curto (6 bytes) | Sem crash. `flags` = 0. |

Logs (com `Log LogTemp Verbose`): `[UmbraCombatStance] ... stance=combate anim=1 auto=0`, `[UmbraGI] opcode 119 player=… stance=…`.

## 8) Troubleshooting

| Sintoma | Causa provável | Ação |
|---|---|---|
| Idle não muda / player fica em T-pose | Mesh ainda no `ABP_Unarmed` ou AnimGraph sem as sequences | §1: `ABP_PlayerHumanoid` com parent `UmbraNpcAnimInstance` no `BP_ThirdPersonCharacter`. |
| Ao andar fica parado na idle | `walk`/`run` normais vazios no Manager | Preencher Walk/Run em Classes → Animation Set. |
| Sacar/guardar não tocam | Sem slot `DefaultSlot` no AnimGraph ou path errado | §1 passo 3 (Slot). Conferir o path (`/Game/Game/...` se importou nessa pasta). |
| Arma "pula" na troca | Socket `WeaponBack` desalinhado | §2: alinhar com o `HandGrip_R` em 0,267 s do Draw. |
| Arma sempre na mão | Socket `WeaponBack` não existe ou arma é SkeletalMesh | §2. Ver o log `socket WeaponBack ausente`. |
| Remoto não guarda a arma (sacar funciona) | Zone antiga: o `MoveUpdate` recriava o estado do player e zerava o `combatStance`, então o "guardar" (0) era descartado como "sem mudança" | Corrigido em `MovementServer.hpp` (atualiza só os campos de movimento). Recompilar e reiniciar a `zone_server`. |
| Remoto volta "relaxado" após sair e voltar do alcance | Ator remoto recriado nascia relaxado | Corrigido: o GameInstance guarda o último 119 por player e reaplica sem anim ao registrar o ator. |
| Tecla não faz nada | `IA_ToggleCombat` não criado/mapeado | §3. Log `IA_ToggleCombat ausente`. |
| O outro cliente não vê | Zone antiga (sem 119) | Recompilar e reiniciar a zone. |

## 9) Referência de código

- Servidor: `src/zone/MovementProtocol.hpp`, `src/zone/MovementServer.hpp` (`CombatStanceState`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraCombatStanceComponent.{h,cpp}`, `Animation/UmbraNpcAnimInstance.{h,cpp}`, `Components/UmbraPlayerAppearanceComponent.{h,cpp}`, `UmbraEternumUECharacter.{h,cpp}`, `Components/UmbraBasicAttackComponent.cpp`, `Core/UmbraGameInstance.{h,cpp}`, `Network/NetMovementClient.cpp`
- Manager: `tools/UmbraManagerWpf/UmbraManager/ViewModels/MainViewModel.Classes.cs`, `Views/ClassesEditorView.xaml`
- Animações: `docs_main/ANIMACOES_BARBARIAN_F_PROMPTS.md` (seção "Modo combate")

**Fim do guia.**
