# GUIA — Indicador de alvo e auto-ataque (UE 5.6.1)

O alvo selecionado (por clique ou Tab) ganha um **anel colorido no chão**. Com alvo selecionado, o ataque básico faz o personagem **andar até o alcance e atacar**. Com o **auto-ataque** ligado, ele continua atacando a cada cooldown. Um botão no HUD liga e desliga o auto-ataque. Toda a lógica está em C++; no Editor entram só o material do anel e o botão do HUD.

## Objetivo

- **Anel de alvo:** vermelho em NPC atacável, azul em jogador. Aparece e some junto com a seleção.
- **Aproximação:** fora do alcance, o personagem anda até 85% do alcance do ataque básico da classe e ataca.
- **Auto-ataque ON:** continua atacando o alvo engajado. **OFF:** um golpe por clique (com aproximação).
- **Cancela com:** WASD, pulo, troca ou perda do alvo, alvo morto, morte local, alvo a mais de 30 m. Stun e root pausam sem cancelar. Skills não cancelam.
- **Dois gatilhos:**
  - a **skill de ataque básico na skillbar** (`is_basic_attack`, via `UseSkillFromSlot`). Hoje é o caminho usado, porque o `BasicAttackAction` do `BP_ThirdPersonCharacter` está vazio. O golpe continua sendo o cast da skill, com o mesmo range/reject do servidor;
  - o `IA_BasicAttack` do personagem, se for mapeado no futuro (golpe pelo opcode **98**).
- **Sem opcode novo.** O movimento é o normal do pawn (frames 25B/34B).
- **Bloqueio durante cast:** enquanto uma skill com `cast_time_ms > 0` está carregando, não sai outra skill nem ataque básico.
  - Cliente: `UUmbraGameInstance::IsLocalPlayerCasting()` barra o `UseSkill` ("Conjurando habilidade") e o `TryBasicAttack`. O auto-ataque pausa (não anda nem golpeia) e retoma ao fim do cast. O estado é limpo no reject 105 da skill em cast e na morte.
  - Servidor: `CombatCoreEngine::isPlayerCasting()` (hit pendente vencendo em mais de 150ms) rejeita a skill com reason **4** "Conjurando outra habilidade" antes de cobrar mana/CD, e descarta o ataque básico (opcode 98).
  - Consumíveis da skillbar continuam liberados durante o cast.
- **Parada borda-a-borda:** o personagem para a `(alcance + raio da cápsula do alvo) × 0.85`. No alcance, só volta a andar se o alvo abrir mais 60u (histerese), sem passar do alcance real do golpe (vs player: só o alcance).
- **IA do NPC (servidor, `NpcAiSystem.cpp`):**
  - mob melee (`combat_stop_range = 0`) **não recua** quando o player chega perto; só a separação por corpo (`bodyMinDist`) continua;
  - mob ranged (`combat_stop_range > 0`) tenta voltar para a distância dele, **exceto** quando tomou dano nos últimos **2 s**: nesse caso ele segura a posição e ataca (`lastDamagedAt`, marcado no `NpcManager::applyDamage`).
- O alcance e o cooldown vêm da skill `is_basic_attack` da classe, que tem os mesmos valores de `basic_attacks` no servidor. Assim o cliente não envia golpes que o servidor descartaria.

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (componente novo `UmbraAutoAttackComponent`).
2. **Parar o PIE** antes de editar os assets.

**Assets:**

| Asset | Path | Tipo / Parent |
|-------|------|---------------|
| M_TargetRing | `/Game/UI/Targeting/M_TargetRing` | Material, **Material Domain = Deferred Decal** (**NOVO**) |
| WBP_PlayerHUD | (o WBP do HUD atual) | Parent `UmbraPlayerHudWidget` (já existe) |

---

> **Status (2026-11-10):** o `M_TargetRing` (§1) e os binds do HUD (§2) já foram criados no projeto pelo MCP. As seções abaixo ficam como referência para recriar ou ajustar à mão. No MCP, o anel foi feito com um nó **Custom** (HLSL) equivalente ao grafo da §1.

## 1) Material do anel — `M_TargetRing`

1. Content Browser → criar a pasta `/Game/UI/Targeting` → botão direito → **Material** → nome **exatamente** `M_TargetRing`.
2. No nó principal (Details):
   - **Material Domain** = **Deferred Decal**
   - **Blend Mode** = **Translucent**
   - **Decal Blend Mode** = **Translucent** (DBuffer não é necessário)
3. Grafo (anel procedural, sem textura):

```
[TexCoord]                         (UV 0..1 do decal)
  → [Subtract] B = (0.5, 0.5)      → centro
  → [Length]                       → d (0 no centro, 0.5 na borda)
  → [Subtract] A = d, B = 0.42     → distância até o raio do anel
  → [Abs]
  → [Divide] B = 0.06              → espessura
  → [OneMinus] → [Saturate]        → Ring (1 no traço, 0 fora)

[VectorParameter "Color"] (1, 0.12, 0.08)
  → Emissive Color (Multiply por 3 para brilhar)
  → Base Color

Ring → Opacity
```

4. **Apply** + **Save**.

> O nome do parâmetro tem que ser **`Color`** (VectorParameter). O C++ cria um `MaterialInstanceDynamic` e troca a cor (vermelho/azul) conforme o alvo.
> Sem o material, nada quebra: só aparece uma vez no log `Material do anel de alvo ausente`.

---

## 2) HUD — botão do auto-ataque (`WBP_PlayerHUD`)

### 2.1 Hierarquia

```
[Barra inferior do HUD]
  BTN_Skillbook ...        ← já existem
  BTN_AutoAttack           ← NOVO (Button)
    TXT_AutoAttack         ← NOVO (TextBlock, filho do botão)
```

### 2.2 Criar

1. Hierarchy → na barra de botões do HUD → **Button** → renomear **exatamente** `BTN_AutoAttack`.
2. Dentro dele → **Text** → renomear **exatamente** `TXT_AutoAttack` (texto inicial qualquer; o C++ troca).
3. Nos dois: **Is Variable** = ligado.
4. **Compile** + **Save**.

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `BTN_AutoAttack` | `UButton` | Não (`BindWidgetOptional`) | Clique alterna o auto-ataque; cor de fundo verde (ON) / cinza (OFF) e tooltip pelo C++ |
| `TXT_AutoAttack` | `UTextBlock` | Não (`BindWidgetOptional`) | "Auto ON" / "Auto OFF" |

As cores podem ser trocadas em **Class Defaults** → `Umbra|HUD|AutoAttack` (`AutoAttackOnColor` / `AutoAttackOffColor`).

---

## 3) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraAutoAttackComponent` | `Source/UmbraEternumUE/Components/UmbraAutoAttackComponent.cpp` | Engaja o alvo, aproxima com `AddMovementInput`, ataca no alcance, mantém o loop com auto ON e cancela pelas regras acima |
| `UUmbraGameInstance::UseSkillFromSlot` | `Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` | Slot com skill `is_basic_attack` e alvo selecionado → `EngageCurrentTarget(SkillID)`; o golpe sai por `UseSkill` quando sai do cooldown |
| `AUmbraEternumUECharacter::BasicAttackPressed` | `Source/UmbraEternumUE/UmbraEternumUECharacter.cpp` | Com alvo, chama `EngageCurrentTarget()` (golpe pelo opcode 98); sem alvo, cai no `TryBasicAttack` antigo. Só dispara se `BasicAttackAction` estiver preenchido |
| `AUmbraEternumUECharacter::Move` / `DoJumpStart` | idem | WASD e pulo cancelam o engajamento |
| `UUmbraBasicAttackComponent` | `Source/UmbraEternumUE/Components/UmbraBasicAttackComponent.cpp` | Cooldown e alcance da skill `is_basic_attack` (piso 0,8 s; fallback de alcance 200 u); `IsReady()` |
| `UUmbraPlayerSelectionComponent::ShowTargetRing` | `Source/UmbraEternumUE/Components/UmbraPlayerSelectionComponent.cpp` | Decal no chão do alvo (raio = cápsula × 1,6), cor por tipo; some na desseleção ou com o actor |
| `UUmbraGameInstance::Is/SetAutoAttackEnabled` | `Source/UmbraEternumUE/Core/UmbraGameInstanceClientSettings.cpp` | Estado salvo no `UmbraClientSettings` (padrão ON) + delegate `OnAutoAttackToggled` |
| `UUmbraPlayerHudWidget` | `Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` | Bind do `BTN_AutoAttack`, texto, cor e tooltip |

**Ajustes no Details:**

| Onde | Propriedade | Padrão |
|------|-------------|--------|
| PlayerController → `PlayerSelectionComponent` → `PlayerSelection|TargetIndicator` | `bShowTargetRing`, `TargetRingMaterial`, `TargetRingEnemyColor`, `TargetRingPlayerColor`, `TargetRingRadiusScale` | ligado, `/Game/UI/Targeting/M_TargetRing`, vermelho, azul, 1.6 |
| Character → `AutoAttack` → `Umbra|Combat|AutoAttack` | `ApproachRangeFraction`, `MaxChaseDistance` | 0.85, 3000 |

> **Não** adicione lógica no Event Graph.

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---------|----------|
| Clicar/Tab num NPC | Anel **vermelho** sob o NPC |
| Clicar/Tab no outro player | Anel **azul** sob o player |
| Trocar de alvo / limpar seleção | Anel muda de alvo / some |
| Ataque básico com NPC longe (auto ON) | Anda até o NPC, para no alcance e ataca em loop |
| Mesmo cenário com auto OFF | Anda, dá **um** golpe e para |
| Durante a aproximação, apertar W | Para de seguir (cancelado) |
| Pular | Cancela |
| Tab para outro alvo durante o loop | Para; novo ataque básico engaja o novo alvo |
| NPC morre | Loop para |
| Skill com cast (barra) e apertar outra skill | Feedback "Conjurando habilidade"; nada é enviado |
| Auto-ataque ligado e soltar skill com cast | Golpes param durante a barra e voltam ao terminar |
| Cliente modificado mandando skill durante cast | Servidor responde 105 "Conjurando outra habilidade", sem cobrar mana/CD |
| Mob melee (Goblin) atacado em loop | Mob vem até o player e troca golpes; ninguém anda para trás |
| Mob ranged (Goblin Magician) atacado em loop | Segura a posição e ataca enquanto toma dano; 2 s sem dano, recua para a distância dele |
| Stun durante o loop | Pausa; volta a atacar quando o stun acaba |
| Clique em `BTN_AutoAttack` | Alterna "Auto ON/OFF" e a cor; estado persiste após reiniciar |
| Desligar o auto no meio do loop | Para depois do golpe atual |
| Client remoto vendo quem auto-ataca | Vê o personagem andar e as animações de golpe normalmente |

Logs (Verbose): `"[UmbraAutoAttack] Engajado alvo type=..."`, `"[UmbraAutoAttack] Engajamento encerrado (<motivo>) ..."`. Log (Log): `"[Settings] Auto-ataque ligado|desligado"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Sem anel e log `Material do anel de alvo ausente` | `M_TargetRing` não existe no path | Criar conforme §1 |
| Anel preto/sem cor | Parâmetro com outro nome | Renomear o VectorParameter para `Color` |
| Anel quadrado/cheio | Opacity sem a máscara do anel | Refazer o grafo da §1 (Ring → Opacity) |
| Personagem chega e não bate | Skill básica não carregada (alcance fallback 200) ou NPC não atacável | Conferir a skill `is_basic_attack` da classe no Manager |
| Golpes saem mas sem dano | Cooldown local menor que o do servidor | Conferir `cooldown_ms` da skill básica vs `basic_attacks` |
| Botão não reage | Nome do bind diferente | Renomear para `BTN_AutoAttack` / `TXT_AutoAttack` |
| Mob anda para trás enquanto o player segue atacando | `zone_server` antigo (recuo para todo mob) | Reiniciar a zone com o build novo; conferir `combat_stop_range = 0` nos melee |
| Mob ranged foge sem parar | Player parou de bater por mais de 2 s | Comportamento esperado: ele volta para a distância de range dele |
| "Atordoado" para sempre depois de morrer stunado (ícone 0:00 na buffbar) | `get_active_buffs.php` antigo devolvia `remaining_ms: 0` (PHP em UTC × MySQL -03:00) e a cópia do banco do STUN não expirava no cliente | PHP corrigido (tempos via `TIMESTAMPDIFF` no MySQL) e sincronizado no WAMP; cliente remove cópias de CC pelo efeito, ignora buff vencido do HTTP, recompõe CC no respawn e limpa tudo no logout |

## 8) Referência de código

- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraAutoAttackComponent.cpp` (`EngageCurrentTarget`, `StepEngagement`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraPlayerSelectionComponent.cpp` (`ShowTargetRing`, `HideTargetRing`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` (`OnAutoAttackClicked`, `RefreshAutoAttackButton`)
- Servidor (sem mudança): `src/zone/CombatCoreEngine.cpp` (`processBasicAttack`, `validateBasicAttackRange`)

**Fim do guia.**
