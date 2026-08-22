# Guia — Animações de NPC / player (UE 5.6.1)

## Dois pipelines

| Família | Como anima |
|---------|----------|
| **Griffon / pack sem ABP** | `PlayAnimation` de Sequences (idle/walk 1 eixo + attacks oneshot) |
| **Bípedes (mesmo skeleton)** | **Um** `ABP_Humanoid` + Animation Set (clips injetados no `UUmbraNpcAnimInstance`) |

Não use `PlayAnimation` para 4/8 direções em humanoide: isso mata o grafo. Locomoção fica no AnimBP; combate usa **montages em slots**.

```
spawn 100 / login classe
        │
        ▼
  anim_states_json  ──► UUmbraNpcAnimInstance (Idle, Walk/Run 4 dirs, GroundSpeed, MoveDirection)
        │
        ▼
  ABP_Humanoid (Editor, 1x por skeleton)
        ├─ blend Idle | Walk | Run × MoveDirection
        ├─ DefaultSlot  (hit, death)
        └─ UpperBody    (attack, cast, buff)
```

O C++ já preenche as UPROPERTYs no spawn. **Você precisa criar o AnimBP no Editor** e apontar o path no Manager. Sem isso, bípede continua no fallback Sequence (só idle/walk 1 eixo).

---

## JSON (`npc_templates.anim_states_json` e `classes.anim_set_json`)

```json
{
  "idle": "/Game/Characters/Humanoid/Anims/Idle.Idle",
  "walk": {
    "fwd": "/Game/Characters/Humanoid/Anims/Walk_F.Walk_F",
    "bwd": "/Game/Characters/Humanoid/Anims/Walk_B.Walk_B",
    "left": "/Game/Characters/Humanoid/Anims/Walk_L.Walk_L",
    "right": "/Game/Characters/Humanoid/Anims/Walk_R.Walk_R"
  },
  "run": { "fwd": "...", "bwd": "...", "left": "...", "right": "..." },
  "attacks": ["/Game/Characters/Humanoid/Montages/AM_Attack1.AM_Attack1"],
  "hits": ["/Game/Characters/Humanoid/Montages/AM_Hit.AM_Hit"],
  "death": "/Game/Characters/Humanoid/Montages/AM_Death.AM_Death",
  "casts": ["/Game/Characters/Humanoid/Montages/AM_Cast.AM_Cast"],
  "buffs": ["/Game/Characters/Humanoid/Montages/AM_Buff.AM_Buff"],
  "death_ms": 1500
}
```

Compat Griffon: `"walk": "/Game/.../FlyForward.FlyForward"` (string) continua 1 eixo.

Aliases de direção: `fwd`/`f`/`forward`, `bwd`/`b`/`back`, `left`/`l`, `right`/`r`.

Path de asset no UE: clique direito no Sequence/Montage → **Copy Reference** e use o formato `/Game/.../Nome.Nome` (sem aspas `AnimSequence'...'`).

---

## Passo a passo no Unreal 5.6.1 — ABP_Humanoid

Faça **uma vez** por skeleton (Manny/Quinn). Guerreiro, mago e guardas usam o **mesmo** ABP; só o JSON/set muda.

### 0) Compile o C++

1. Feche o Editor se estiver aberto (ou Live Coding).
2. Compile o módulo `UmbraEternumUE` (Visual Studio ou `UmbraEternumUE.sln`).
3. Abra o projeto **5.6.1**.
4. Confirme que a classe `UmbraNpcAnimInstance` aparece em **All Classes** (filtro do picker).

### 1) (Opcional) Montages a partir das Sequences

O combate prefere **Anim Montage**. Se o pack só tem Sequence:

1. Content Browser → pasta do pack → clique direito na Sequence (ex. `Attack_01`).
2. **Create** → **Create Anim Montage**.
3. Abra o Montage. Em **Slot Name** (painel Asset Details / Slot):
   - ataques, cast, buff → `UpperBody`
   - hit, death → `DefaultSlot`
4. **Looping**: off (exceto buff contínuo).
5. Salve como `AM_Attack1`, etc.

Se deixar Sequence no JSON, o C++ tenta `CreateSlotAnimationAsDynamicMontage` no slot certo quando o ABP está ativo.

### 2) Criar o Animation Blueprint

1. Content Browser → pasta ex. `/Game/Characters/Humanoid/`.
2. Clique direito → **Animation** → **Animation Blueprint**.
3. **Target Skeleton**: o **mesmo** do mesh do player (SK_Mannequin / SK_Quinn). Não use o skeleton do Griffon.
4. **Parent Class**: clique em **All Classes**, busque `UmbraNpcAnimInstance` (não `AnimInstance`).
5. Nome: `ABP_Humanoid`. Crie.
6. Abra o asset. Duas abas: **AnimGraph** e **Event Graph**. Só precisamos do **AnimGraph**.

Copie o path para o Manager: clique direito em `ABP_Humanoid` → **Copy Reference**.  
Exemplo: `/Game/Characters/Humanoid/ABP_Humanoid.ABP_Humanoid_C`  
(o `_C` é a classe gerada; o servidor/cliente aceitam com ou sem, o load C++ acrescenta `_C` se faltar).

### 3) Variáveis (já existem — não recrie)

No **My Blueprint** → **Variables** você deve ver (vindas do C++):

| Variável | Tipo | Uso no grafo |
|---------|------|----------------|
| `GroundSpeed` | float | velocidade XY |
| `MoveDirection` | float | −180..180 (0 = frente) |
| `bIsMoving` | bool | `GroundSpeed > WalkSpeedThreshold` |
| `bIsRunning` | bool | `GroundSpeed > RunSpeedThreshold` |
| `WalkSpeedThreshold` / `RunSpeedThreshold` | float | defaults 20 / 400 |
| `IdleSequence` | Anim Sequence | idle |
| `WalkSequence` | Anim Sequence | fallback 1 eixo |
| `WalkFwdSequence` … `WalkRightSequence` | Anim Sequence | walk 4 dirs |
| `RunFwdSequence` … `RunRightSequence` | Anim Sequence | run 4 dirs |

**Não** crie duplicatas no Blueprint. O C++ preenche no spawn; no Editor, arraste essas variáveis para o grafo.

### 4) AnimGraph — locomoção 4 direções

Objetivo: Idle quando parado; Walk/Run blendados por direção.

**4.1 — Walk (4 dirs)**

1. Clique direito no AnimGraph → **Blend Poses by Enum** **não**. Use **Blend Poses by Bool** em cadeia **ou** um **Blendspace** feito à mão. O caminho mais simples **sem BlendSpace asset**:

   **Opção A (recomendada, só Sequences):** nó **Blend Angles** / **Directional Blend** não existe nativo. Use:

   - Nó **Blend Poses by Int** **não**.
   - Monte um **2D Blend** caseiro com **Blend Overlay** **não**.

   **Opção prática no 5.6:** criar um **Blend Space 1D** *não é obrigatório*. Use **Layered blend per bone** só para combate.

   Para 4 dirs no grafo:

   1. Arraste `WalkFwdSequence` → **Play Sequence** (ou o pino rosa **Sequence Player** ao arrastar a variável). Marque **Loop**.
   2. Repita para Bwd, Left, Right.
   3. Clique direito → **Blend Poses by Bool** (três vezes) **ou** use **Blend** (alpha 0–1).

   **Blend por `MoveDirection` (mais limpo):**

   1. Clique direito → **Blendspace** não. Use o nó **Calculate Direction** **já está no C++** (`MoveDirection`).
   2. Clique direito → busque **Blend Poses by Int** — skip.
   3. Use **Map Range Clamped** + **Blend** para eixos:

   **Grafo sugerido (cardinal):**

   ```
   MoveDirection
        │
        ├─ abs > 135  → WalkBwd          (costas)
        ├─ > 45       → WalkRight        (strafe D)
        ├─ < -45      → WalkLeft         (strafe E)
        └─ else       → WalkFwd          (frente)
   ```

   Nós:

   1. `MoveDirection` → **Absolute** (para costas: `Abs(MoveDirection) > 135`).
   2. **Branch** / **Blend Poses by Bool**:
      - `Abs(MoveDirection) > 135` → pose Bwd.
      - Senão: `MoveDirection > 45` → Right.
      - Senão: `MoveDirection < -45` → Left.
      - Senão → Fwd.
   3. Marque **Looping** em todos os Sequence Players de walk/run/idle.

   **Se uma direção vier vazia no JSON:** use `WalkFwdSequence` (ou `WalkSequence`) como fallback no próprio grafo com **Is Valid** no object pin da Sequence. Se inválido, plugue `WalkSequence` / `IdleSequence`.

**4.2 — Run (4 dirs)**

Copie o mesmo arranjo com `RunFwdSequence` … `RunRightSequence`.

**4.3 — Idle | Walk | Run**

1. Arraste `IdleSequence` → Sequence Player, **Loop**.
2. Clique direito → **Blend Poses by Bool**:
   - `bIsRunning` → saída do blend **Run 4 dirs**.
   - Senão `bIsMoving` → blend **Walk 4 dirs**.
   - Senão → Idle.
3. Encadeie: `Blend(bIsRunning, Run, Blend(bIsMoving, Walk, Idle))`.

Nomes dos nós no 5.6: **Blend Poses by Bool** (True / False pose pins).

### 5) AnimGraph — slots de combate (obrigatório)

Sem slots, `Montage_Play` não aparece por cima da loco.

1. Na pose final de locomoção (saída do blend Idle/Walk/Run), clique direito → **DefaultSlot** (categoria **Slot**).  
   Slot Name: `DefaultSlot` (exato).
2. Da saída do DefaultSlot → clique direito → **Slot** de novo → Slot Name: `UpperBody`.
3. **UpperBody precisa ser layered**, senão o ataque para as pernas:

   - Clique direito → **Layered blend per bone**.
   - **Base Pose**: saída da loco (depois do DefaultSlot **ou** a loco pura).
   - **Blend Poses 0**: saída de um **Slot** `UpperBody`.
   - **Layer Setup** (Details do nó):
     - Add element → **Branch Filter**
     - Bone Name: `spine_01` (Manny) ou `Spine` (Quinn). Confira no Skeleton.
     - **Blend Depth**: 0 (da spine para cima).
   - **Blend Weights[0]**: 1.0 (ou curve se quiser fade).

   Ordem recomendada:

   ```
   [Idle/Walk/Run blend]
        │
        ▼
   Slot DefaultSlot          ← hit / death (corpo inteiro)
        │
        ▼
   Layered blend per bone
        ├─ Base: loco+DefaultSlot
        └─ Blend Pose 0: Slot UpperBody   ← attack / cast / buff
        │
        ▼
   Output Pose
   ```

   Se o Layered blend ficar estranho, versão mínima (ataque para o corpo todo):

   ```
   loco → Slot DefaultSlot → Slot UpperBody → Output Pose
   ```

   O C++ usa exatamente os nomes **`DefaultSlot`** e **`UpperBody`**.

4. Compile o AnimBP (botão **Compile**). Corrija pinos vermelhos (Sequence nula no preview é normal até o spawn).

### 6) Preview no Editor

1. Aba **Preview Scene Settings** → mesh humanoide do mesmo skeleton.
2. No **Anim Preview Editor** (ou **My Blueprint** → defaults): arraste Sequences de teste em `IdleSequence` / `WalkFwdSequence` para ver o blend.
3. No **AnimGraph**, com o preview rodando, altere **GroundSpeed** e **MoveDirection** nos defaults (lupa **Class Defaults** / painel Preview):
   - Speed 0 → idle
   - Speed 150, Direction 0 → walk fwd
   - Direction 90 → strafe
   - Speed 500 → run
4. Salve.

O preview **não** injeta JSON do servidor. Paths reais entram só no Play (opcode 100).

### 7) Ligar no jogo (Manager + mesh)

1. Template NPC bípede no UmbraManager:
   - **Anim Blueprint path**: `/Game/Characters/Humanoid/ABP_Humanoid.ABP_Humanoid_C`
   - Idle + Walk/Run 4 dirs + attacks (montages de preferência).
2. **Skeletal mesh path**: mesh **desse** skeleton. Mesh de Griffon + ABP_Humanoid = T-pose / crash de bone.
3. Salve o template, recarregue a zone (ou respawn).
4. PIE: o NPC deve andar nas 4 dirs; basic ataca no UpperBody.

Player (fase seguinte):

1. Abra o Blueprint do pawn (`BP_ThirdPersonCharacter` / o que o GameMode usa).
2. Mesh → **Anim Class** = `ABP_Humanoid`.
3. Rode o SQL `www/umbra_api/scripts/add_classes_anim_set_json.sql`.
4. Preencha `classes.anim_set_json` (mesmo JSON). No BeginPlay o C++ chama `ApplyClassAnimSetToPawn`.

### 8) DataAsset opcional (`UUmbraAnimSet`)

Só para preview/local, **não substitui** o JSON do servidor.

1. Clique direito → **Miscellaneous** → **Data Asset**.
2. Classe: `UmbraAnimSet`.
3. Preencha Idle / Walk / Run / Montages.
4. O spawn **sempre** sobrescreve com paths do opcode 100.

---

## Combate (NPC com ABP)

| Ação | Slot | Fonte |
|------|------|--------|
| Basic | UpperBody | `attacks[i]` + `animIndex` (opcode 99) |
| Skill | UpperBody | `casts[]` / `skill` / `attacks[0]` |
| Buff | UpperBody | `buffs[]` |
| Hit | DefaultSlot | `hits[]` + 103 |
| Death | DefaultSlot | `death` + 102 Dying |

Sem ABP (Griffon): Sequence `PlayAnimation` como antes.

---

## Checklist se não animar

| Sintoma | Causa típica |
|--------|----------------|
| T-pose / idle preso | AnimBP path é pasta, não `ABP_*_C`; ou Parent Class não é `UmbraNpcAnimInstance` |
| Anda só 1 eixo | JSON `walk` ainda é string; preencha `walk.fwd`… |
| Ataque não toca | Slot no grafo ≠ `UpperBody`; ou path não é Montage/Sequence do **mesmo** skeleton |
| Ataque para as pernas | Falta Layered blend per bone na spine |
| Player ignora o set | Mesh ainda no `ABP_Unarmed` (Cast para `UUmbraNpcAnimInstance` falha) |
| Griffon quebrou | Você apontou `ABP_Humanoid` no template do Griffon — deixe AnimBP **vazio** |

---

## Opcodes 100

Além de idle/walk: 8 paths dir + listas `casts`/`buffs` (compat se o packet antigo acabar no `death_ms`).

## Teste

1. Griffon: AnimBP vazio, walk string — fly loop intacto.
2. Humanoide com `ABP_Humanoid`: walk 4 dirs + run; basic no UpperBody; hit não substitui o ciclo de loco.
3. Player com o mesmo ABP + `anim_set_json` da classe: strafe/corrida iguais ao NPC da classe.
