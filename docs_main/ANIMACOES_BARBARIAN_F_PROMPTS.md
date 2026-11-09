# Animações — Barbarian (feminina) — prompts por habilidade

Base: `SKM_Quinn_Simple` (esqueleto UE5 padrão) no Blender 5.2, 30 fps.
Fonte dos dados: `www/umbra_api/scripts/insert_all_skills.sql` (seed das 15 skills) e
`www/umbra_api/scripts/add_basic_attack_skills.sql` (ataque básico). Se o Manager alterou
tempos/efeitos no banco, os números abaixo precisam ser revisados.

## Convenções

| Item | Regra |
|------|-------|
| Arma | **Machado de duas mãos** (`subtype axe`). Mão direita em cima (perto da lâmina), esquerda na base do cabo, ~22 cm entre as mãos. |
| Pose de entrada/saída | Guarda de duas mãos: pés afastados com a esquerda à frente, joelhos flexionados, machado em diagonal na frente do corpo (lâmina para cima-direita). Toda animação começa e termina nela. |
| Impacto | O frame de impacto bate com o `cast_time_ms` (30 fps → `frame = 1 + cast_ms × 0,03`). Quando o cast é 0, usar o `hit_window_ms`. |
| Deslocamento | Em princípio, **in place**. Os casos com deslocamento (Investida, Impacto Esmagador) serão definidos em conjunto antes de animar. |
| Nomes | Sequence `A_Barb_F_<Nome>`; montage `AM_Barb_F_<Nome>` (gravar em `skills.cast_anim_path`). |
| Passivas | Sem animação de cast (só stat no servidor). |

## Modo combate (idle relaxada/combate, sacar, guardar)

Ver [GUIA_MODO_COMBATE_UE561.md](GUIA_MODO_COMBATE_UE561.md).

| Anim | Frames | Detalhe |
|---|---|---|
| `A_Barb_F_Idle_Relaxed` | 60 (loop, export do f1) | Mãos livres e machado nas costas. Respiração (peito/clavículas), troca de peso e olhar para o lado. |
| `A_Barb_F_Idle_Combat` | 60 (loop, export do f1) | Guarda de duas mãos com respiração e leve balanço do machado. |
| `A_Barb_F_DrawWeapon` | 30 | Mão direita no cabo atrás do ombro. **f8 = 267 ms (`draw_swap_ms`):** arma vai para a mão. Ergue por cima da cabeça (f12), traz à frente com uma mão (f16), a mão esquerda encaixa (f20) e termina na guarda (f30). |
| `A_Barb_F_SheatheWeapon` | 30 | Solta a mão esquerda (f5), ergue por cima da cabeça (f9). **f13 = 433 ms (`sheathe_swap_ms`):** encaixa nas costas. A mão desce e ela volta à postura relaxada (f30). |

- **Machado nas costas:** invertido. Preview `PREVIEW_AxeBack` no `spine_05`. A pegada fica em `c(18, -14, 148)` (direita/frente/cima, repouso), com o cabo descendo em diagonal para o quadril esquerdo.
- **Pegada no frame da troca:** a mão direita coincide com o cabo das costas (0 cm) no f8 do Draw e no f13 do Sheathe.

## Prompts

### 0. Ataque Básico — `A_Barb_F_BasicAttack_A` / `_B` (2 variações)

**v2 (2026-11-09):** mais flexão de pernas, mais rotação do corpo e mais impacto.
- **Duração:** 30 f (1,0 s), com impacto no **f10 (333 ms)**.
- **Manager:** `hit_window_ms` = **333** e cooldown do ataque básico = **1000 ms**. Na `basic_attacks` o cooldown é carregado na subida da zone, então é preciso reiniciá-la.
- **Pernas:** IK com os pés plantados, sem deslizar. O pé de trás pivota na ponta (o calcanhar sobe e gira) quando o quadril gira.
- **Tronco:** o giro total chega a ~±48°, com 45% no quadril e 55% na coluna. A cabeça compensa para continuar olhando o alvo.

**Variação A (v2):** golpe horizontal da direita para a esquerda.
- **f6 (wind-up):** tronco enrolado 45° para a direita, joelhos a ~50°/60° e machado sobre o ombro direito. O calcanhar esquerdo sobe.
- **f10 (impacto):** à frente, na altura do peito, com as mãos lideradas pela esquerda. Investida funda (joelho esquerdo a ~76°) e pé direito pivotando (calcanhar 30°).
- **Depois do impacto:** passa do ponto no f12, segue o giro até o f16 (~48°) e volta à guarda no f30.

**Variação B (v2):** golpe de volta, subindo na diagonal da esquerda baixa para a direita alta.
- **f4–6 (wind-up):** levanta o machado à frente e carrega baixo, atrás do quadril esquerdo. O tronco enrola 45° para a esquerda e o joelho esquerdo dobra a ~68°.
- **f10 (impacto):** cruza à frente do peito subindo. O corpo se estende e o pé esquerdo pivota.
- **Fim:** termina alto sobre o ombro direito no f16 e volta à guarda no f30.

**QA (A e B):**
- pulso ≤ 66° em todos os frames;
- antebraços ≥ 13 cm do eixo do tronco nas passagens e ≥ 16 cm nas poses-chave;
- mãos sempre no cabo;
- deslize dos pés ≤ 1,9 cm.

**Combo A/B no cliente** (`UUmbraCombatComponent::PlayBasicAttackFromPathList`):
- **Configuração:** o `cast_anim_path` do ataque básico aceita vários paths separados por `,` ou `;`. Exemplo: `/Game/Animations/Combat/Barbarian/AM_Barb_F_BasicAttack_A,/Game/Animations/Combat/Barbarian/AM_Barb_F_BasicAttack_B`.
- **Alternância:** A → B → A a cada ataque. Volta para a A depois de `BasicAttackComboResetSeconds` (1,5 s) sem atacar.
- **Eco do servidor:** o opcode 99 também chega para quem atacou. Se a predição local tocou há ≤ `BasicAttackEchoIgnoreSeconds` (0,6 s), o eco é ignorado (sem animação dupla e sem pular variação).
- **Os dois caminhos tratam a lista:** o ataque básico pode chegar pelo opcode 99 (`HandleBasicAttackBroadcast`) ou como cast da skill `is_basic_attack` (`HandleSkillCastBroadcast`). Este segundo caminho é o que o PIE usou em 2026-11-09.
- **Compatibilidade:** com um path só, o comportamento é o de antes.
- **Limite:** 255 caracteres (protocolo e coluna). Dois paths de ~55 caracteres cabem com folga.

**Prompt original:**
- **Dados:** 85% de dano físico, cooldown 800 ms, `hit_window_ms` 300.
- **Tempo:** ~0,8 s (24 f), impacto no f10.
- **Variação A:** golpe horizontal da direita para a esquerda, na altura do peito. O quadril e os ombros giram juntos e o machado termina à esquerda do corpo.
- **Variação B:** volta da esquerda para a direita, num diagonal ascendente curto. Junto com a A, forma um combo alternado.
- **Ritmo:** recuperação rápida para a guarda, para o ataque poder emendar no próximo.

### 1. Golpe da Ruína — `A_Barb_F_RuinStrike` ✅ feita
- **Dados:** 180% de dano físico, stun 2 s, cast 500 ms, chance de Double Attack.
- **Tempo:** 46 f (1,53 s), impacto no **f15**.
- **f1–4 (antecipação):** agacha e traz o machado para o ombro direito.
- **f4–13 (wind-up pesado):** a subida desacelera no topo e o machado "flutua" acima da cabeça (f10–13), com a lâmina armada para trás e o tronco arqueado.
- **f13–15 (descida):** aceleração forte. Os braços passam estendidos à frente e acima, com os cotovelos para fora. O tronco puxa o golpe antes dos braços (sobreposição), a perna esquerda avança e o quadril despenca ~22 cm.
- **Mãos deslizando:** a mão direita desliza pelo cabo até encontrar a esquerda (de 22 cm para 6 cm), como num corte de machado real.
- **f15 (impacto):** corte baixo que atravessa o alvo. O machado fica quase horizontal, com a lâmina perto do chão, pulsos retos e os nós dos dedos alinhados ao antebraço.
- **f16:** o golpe passa do ponto e a lâmina raspa o chão.
- **f18:** quique de retorno.
- **f20–28:** segura o peso.
- **f28–46 (recuperação lenta):** levanta o machado como algo pesado, as mãos voltam a se afastar no cabo e ela retorna à guarda.
- **Regras de QA usadas:**
  - pulso (antebraço × nós dos dedos) ≤ ~65° nos frames de destaque;
  - antebraços a ≥ 16 cm do eixo do tronco;
  - as duas mãos sempre no cabo.
- **VFX/SFX:** `VFX_Heavy_Slash` e `SFX_Heavy_Impact` no f15.

### 3. Investida Selvagem — `A_Barb_F_WildCharge` (deslocamento a definir)
- **Dados:** 140% de dano, alcance 300, +30% de velocidade de movimento por 3 s, cast 0.
- **Estrutura:** 3 partes (início, loop e golpe), conforme o deslocamento que definirmos.
- **Início (~0,3 s):** agacha, inclina o tronco à frente e leva o machado baixo para trás, à direita.
- **Loop de corrida:** corrida curvada com o ombro esquerdo à frente e o machado baixo, arrastando atrás com as duas mãos.
- **Golpe de chegada:** golpe ascendente de baixo para cima (uppercut com o machado), com impacto ~0,25 s depois de chegar.

### 5. Rugido de Guerra — `A_Barb_F_WarCry` ✅ feita
**Como ficou ("estandarte"):**
- **Duração:** 54 f (1,8 s), com o **rugido no f30 (1000 ms = `cast_time_ms`)**.
- **f4–7:** pisa firme, afastando o pé esquerdo (o pé sai do chão), e entra numa base larga com o machado à frente.
- **f10–14:** ergue o machado com o cabo atravessado acima da cabeça.
- **f20–25:** puxa o ar com o cabo na altura do rosto. Ombros sobem e o peito abre.
- **f30 (rugido):** braços estendidos para cima, machado ao céu, corpo arqueado para trás e cabeça jogada para trás.
- **f30–38:** tremor de tensão (micro-oscilações no tronco, nos ombros e na cabeça).
- **f42–54:** desce o machado, recolhe o pé e volta à guarda.
- **QA:** só o f16 fica com pulso a 68°, com os braços longe do corpo.

**Prompt original:**
- **Dados:** buff de party, +20% de ataque físico por 15 s, raio 500, cast 1000 ms.
- **Tempo:** ~1,6 s (48 f), pico do grito no **f31**.
- **Preparação:** finca a base com os pés bem afastados, puxa o machado para cima com as duas mãos e o ergue acima da cabeça, com o cabo na horizontal.
- **Grito (f31):** peito para frente, cabeça para trás, boca/maxilar abertos e tremor leve no tronco por ~10 f.
- **Saída:** desce o machado de volta para a guarda.
- **VFX/SFX:** `VFX_War_Shout` e `SFX_Roar` no f31.

### 7. Corte Dilacerante — `A_Barb_F_RendingCut` ✅ feita
**Como ficou (v3, "carniceira"):** a v1 era parecida demais com o ataque básico. Na v3 são duas machadadas curtas, em ritmo duplo.
- **Duração:** 32 f, com a **1ª machadada no f9 (300 ms = `cast_time_ms`)** e a 2ª no f15, mais funda.
- **f4:** machado ao ombro direito.
- **f7–9:** desce pelo caminho alto à frente, mãos juntas no cabo, fio para baixo e tronco inclinado.
- **f10–12:** volta ao ombro pelo mesmo caminho.
- **f13–15:** 2ª machadada, com o quadril mais baixo e o calcanhar de trás subindo.
- **f18:** arrasta a lâmina, rasgando. Retorno nos f24–32.
- **QA (régua pela malha, calibrada no Golpe da Ruína: ≥ 1,4 cm):** só o retorno à guarda fica em 0,9 cm.
- **Descartados:**
  - "fisgar e arrancar" e "enxada": o fio voltado para ela exige o pulso a ~100°;
  - "estripar" de baixo para cima: o fio subindo com os braços à frente exige o pulso a ~80°.
  - Motivo: neste rig, o fio do machado acompanha os nós dos dedos (pegada real de machado).

**Prompt original:**
- **Dados:** 130% de dano + Sangramento (40%/tick por 8 s), cast 300 ms.
- **Tempo:** ~1 s (30 f), impacto no **f10**.
- **Movimento:** corte diagonal rápido da direita-alta para a esquerda-baixa. No fim, as duas mãos "puxam" o machado para trás, como quem arranca a lâmina da carne.
- **Fechamento:** tronco torcido para a esquerda e leve giro do pulso.
- **VFX:** `VFX_Slash_Bleed` acompanha o arco.

### 9. Recuperação Instintiva — `A_Barb_F_InstinctRecovery`
- **Dados:** cura 25% do HP, cast 1500 ms.
- **Tempo:** ~2 s (60 f), cura no **f46**.
- **Preparação:** apoia a cabeça do machado no chão à frente, as duas mãos no topo do cabo, e curva o corpo apoiada nele.
- **Respiração:** inspiração longa nos f15–40 (peito e ombros sobem) e expiração forte no f46, endireitando a coluna "ignorando a dor".
- **Saída:** volta à guarda.
- **VFX/SFX:** `VFX_Self_Heal` e `SFX_Breath`.

### 10. Retaliação Primitiva — `A_Barb_F_PrimalRetaliation` ✅ feita
**Como ficou (v2, "encontrão com o cabo"):** a v1 reaproveitava o ataque básico. A v2 é o único golpe que não usa a lâmina.
- **Duração:** 30 f, impacto no **f9 (300 ms = `hit_window_ms`)**.
- **f3 (bloqueio):** cabo atravessado na altura do rosto, corpo e cabeça jogados para trás.
- **f6 (carga):** agacha (quadril −18 cm), com o cabo junto ao peito.
- **f9 (encontrão):** passo à frente com o pé esquerdo (14 cm) e o calcanhar de trás a 40°. Braços estendidos empurrando o cabo horizontal, de frente para o alvo.
- **f11–15:** estende mais e segura.
- **f19 (rugido):** tronco para trás e cabeça para cima.
- **f24–28:** recolhe o pé, que sai do chão em vez de arrastar.
- **QA:** pulsos entre 2° e 23° nas poses-chave. Só o retorno fica no nível da guarda.

**Prompt original:**
- **Dados:** reação a crítico recebido, 120% de dano, cast 0.
- **Tempo:** ~0,9 s (27 f), impacto no **f12**.
- **Movimento:** no f1–5 o tronco é jogado para trás, como se tivesse sido atingida, e o machado vai para o ombro direito. Sem pausa, sai um golpe horizontal de revide da direita para a esquerda.
- **Saída:** recuperação curta.
- **VFX/SFX:** `VFX_Counter` e `SFX_Counter_Hit` no f12.

### 11. Frenesi Carmesim — `A_Barb_F_CrimsonFrenzy` ✅ feita
**Como ficou ("transe do berserker"):**
- **Duração:** 45 f (1,5 s), com a **entrada no transe no f15 (500 ms = `cast_time_ms`)**.
- **f4–11:** duas pancadas do cabo atravessado no próprio peito (f6 e f11). A cabeça baixa e os ombros curvam a cada pancada.
- **f15:** cai numa postura curvada (quadril −20 cm), com a cabeça projetada à frente e o machado pendurado baixo no centro, de cabeça para baixo, quase arrastando.
- **f19–38:** balanço instável do tronco e da cabeça (respiração pesada).
- **f42–45:** recolhe o pé e volta à guarda.
- **QA:** pulsos ≤ 36° na postura. A régua pela malha marca 0,3–0,4 cm nos cotovelos rente às costelas e nas pancadas (contato proposital do cabo com o peito). Visualmente os braços ficam à frente do corpo.

**Prompt original:**
- **Dados:** +35% de Double Attack e −20% de defesa por 12 s, cast 500 ms.
- **Tempo:** ~1,2 s (36 f), pico no **f16**.
- **Movimento:** bate o cabo do machado no próprio peito duas vezes (f6 e f12). No f16 abre os braços com o machado numa mão só, inclina-se à frente e rosna.
- **Fechamento:** termina numa guarda mais baixa e instável, com a defesa aberta.
- **VFX/SFX:** `VFX_Frenzy` e `SFX_Rage`.

### 13. Impacto Esmagador — `A_Barb_F_CrushingImpact` (deslocamento a definir)
- **Dados:** 150% de dano, −30% de Dodge do alvo por 6 s, cast 800 ms.
- **Tempo:** ~1,6 s (48 f), impacto no **f25**.
- **Movimento:** salto curto (ou passo largo, a definir) com o machado acima da cabeça. Golpe vertical no chão com as duas mãos, ajoelhando sobre a perna de trás.
- **Pós-impacto:** segura 6 f e se levanta.
- **VFX:** `VFX_Ground_Slam` (rachadura no chão) no f25.

### 15. Avatar da Ruína (ultimate) — `A_Barb_F_AvatarRuin`
- **Dados:** +40% de ataque físico, +25% de velocidade e +30% de Double Attack por 20 s, cast 2000 ms.
- **Tempo:** ~2,5 s (75 f), transformação no **f61**.
- **f1–20:** crava a lâmina no chão à frente e curva o corpo sobre o cabo.
- **f20–55:** ergue-se devagar com os punhos fechados e os braços se abrindo, tremendo de tensão.
- **f61:** grito com os braços abertos e o peito para cima.
- **f61–75:** arranca o machado do chão e o leva para a guarda.
- **VFX/SFX:** `VFX_Ultimate_Barbarian` e `SFX_Ultimate_Roar` no f61.

### Passivas — sem animação
| # | Skill | Observação |
|---|-------|------------|
| 2 | Pele Endurecida | — |
| 4 | Sangue Quente | Opcional no futuro: idle ofegante quando o HP < 30%. |
| 6 | Ossos de Ferro | — |
| 8 | Passo da Caçadora | — |
| 12 | Resistência Brutal | — |
| 14 | Caçada Implacável | — |

## Status

| Animação | Status |
|----------|--------|
| A_Barb_F_RuinStrike | ✅ Aprovada, exportada e testada na UE (2026-11-09) |
| A_Barb_F_BasicAttack_A | Exportada (aguardando teste na UE) |
| A_Barb_F_BasicAttack_B | Exportada (aguardando teste na UE) |
| A_Barb_F_RendingCut | Exportada (aguardando teste na UE) |
| A_Barb_F_PrimalRetaliation | Exportada (aguardando teste na UE) |
| A_Barb_F_WarCry | Exportada (aguardando teste na UE) |
| A_Barb_F_CrimsonFrenzy | Exportada (aguardando teste na UE) |
| A_Barb_F_Idle_Relaxed / _Idle_Combat | Exportadas (aguardando teste na UE) |
| A_Barb_F_DrawWeapon / _SheatheWeapon | Exportadas (aguardando teste na UE) |
| Demais | Pendentes |

## Export → UE 5.6.1

**Blender (faço eu, via MCP):**
1. Bake dos ossos `ik_hand_gun`, `ik_hand_r`, `ik_hand_l`, `ik_foot_l` e `ik_foot_r` copiando mão e pé (erro 0).
2. Export FBX só do armature `root` (sem `PREVIEW_Axe`), com estas opções:
   - escala: Apply Unit; eixos padrão (−Z forward, Y up); bone axis Y/X;
   - sem leaf bones; bake com step 1 e simplify 0.
3. **Range exportado a partir do frame 0** (repete a guarda). Assim o impacto do f15 cai em exatamente 500 ms (= `cast_time_ms`).
4. Verificação de ida e volta: reimportar numa cena temporária e comparar as posições dos ossos. Precisa dar 0 cm.
- **Saída:** `Downloads\UmbraAnimExport\<Nome>.fbx`.

**Editor UE (usuário):**
1. Criar a pasta `/Game/Animations/Combat/Barbarian/`.
2. **Import** do FBX com estas opções:
   - **Skeleton** = `SK_Mannequin` (`/Game/Characters/Mannequins/Meshes/SK_Mannequin`);
   - **Import Mesh** desmarcado e **Import Animations** marcado;
   - **Animation Length** = *Exported Time*;
   - **Import Uniform Scale** = 1.
3. Conferir a AnimSequence no Persona: escala normal, sem tremer e com as mãos fechadas.
4. Botão direito na sequence → **Create → Create AnimMontage** → `AM_Barb_F_<Nome>`, no slot `DefaultSlot`.
5. **Manager → Classes → Barbarian → "Animações por habilidade (cast_anim_path)"**: na linha da skill, informar `/Game/Animations/Combat/Barbarian/AM_Barb_F_<Nome>` e salvar. A zone relê o banco a cada cast, sem reiniciar.
6. Testar no PIE com 2 clientes: usar a skill e ver a montage nos dois.

| Sintoma | Causa | Ação |
|---------|-------|------|
| Personagem gigante/minúsculo ou deitado | Escala/eixo do import | Conferir **Import Uniform Scale = 1** e reimportar. |
| Animação não toca, log `Montage_Play=0` | ABP sem slot `DefaultSlot` | Adicionar o nó **Slot 'DefaultSlot'** no AnimGraph do ABP do personagem. |
| Só a parte de cima anima | Montage em `UpperBody` | Usar `DefaultSlot` (golpe de corpo inteiro). |
| Toca no cliente local e não no outro | Path errado no banco | Conferir o `cast_anim_path` no Manager (sem `.uasset`). |

## Notas técnicas (Blender)

- O texto `umbra_anim_lib.py`, dentro do `.blend`, tem os helpers:
  - `pose_axe`: a pose define o machado e os dois braços são resolvidos por IK analítico;
  - `bake_axe_action`: grava frame a frame com as duas mãos sempre no cabo.
- `PREVIEW_Axe`, na coleção `Preview`, é só para visualização. **Não exportar.**
- Antes do export para a UE: bake dos ossos `ik_hand_*` / `ik_foot_*` copiando o FK, se o projeto os usar.
