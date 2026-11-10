# GUIA — VFX das skills da Barbarian (UE 5.6.1)

Efeitos Niagara das skills da Barbarian, adaptados dos pacotes do projeto (`Free_Magic`, `Free_Spells`, `sA_Megapack_v1`, `Niagara_Effects`) para a paleta de fúria (vermelho/laranja), faísca de metal e poeira. O cliente já toca os três campos de VFX da skill; não há Blueprint novo.

## Objetivo

- Cada skill ativa da Barbarian tem efeito de impacto e, quando é buff, uma aura em loop.
- O `hit_vfx_path` sai no alvo depois de `cast_time_ms`, que nas animações da Barbarian é o frame do impacto (ver `ANIMACOES_BARBARIAN_F_PROMPTS.md`).
- Opcodes: **97** (cast de skill, já leva `vfx_path`/`hit_vfx_path`). Nada muda no protocolo.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `skills.vfx_path` | Manager → **Skills** → VFX | Em quem lança, no início do cast |
| `skills.hit_vfx_path` | Manager → **Skills** → Hit VFX | No alvo, após `cast_time_ms` (alvo 0 = em quem lançou) |
| `skills.buff_vfx_path` | Manager → **Skills** → Buff VFX | Loop preso ao corpo enquanto o buff durar |

Sem SQL: as colunas já existem.

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (mudou `Core/UmbraGameInstance.cpp`).
2. Assets em `/Game/VFX/Barbarian/` (criados pelo MCP; se houver redirectors das renomeações, **botão direito na pasta → Fix Up Redirectors**).

---

## 1) Paths por skill

| Skill | `vfx_path` | `hit_vfx_path` | `buff_vfx_path` |
|---|---|---|---|
| Ataque básico | — | `/Game/VFX/Barbarian/NS_Barb_HitSpark.NS_Barb_HitSpark` | — |
| Golpe da Ruína | — | `/Game/VFX/Barbarian/NS_Barb_HeavyImpact_Ground.NS_Barb_HeavyImpact_Ground` | — |
| Investida Selvagem | `/Game/VFX/Barbarian/NS_Barb_ChargeDust_Ground.NS_Barb_ChargeDust_Ground` | `/Game/VFX/Barbarian/NS_Barb_HitSpark_Big.NS_Barb_HitSpark_Big` | `/Game/VFX/Barbarian/NS_Barb_ChargeTrail.NS_Barb_ChargeTrail` |
| Rugido de Guerra | — | `/Game/VFX/Barbarian/NS_Barb_WarCry_Ground.NS_Barb_WarCry_Ground` | `/Game/VFX/Barbarian/NS_Barb_WarCryBuff.NS_Barb_WarCryBuff` |
| Corte Dilacerante | — | `/Game/VFX/Barbarian/NS_Barb_BleedSlash.NS_Barb_BleedSlash` | — |
| Recuperação Instintiva | — | `/Game/VFX/Barbarian/NS_Barb_SelfHeal.NS_Barb_SelfHeal` | — |
| Retaliação Primitiva | — | `/Game/VFX/Barbarian/NS_Barb_CounterHit.NS_Barb_CounterHit` | — |
| Frenesi Carmesim | — | `/Game/VFX/Barbarian/NS_Barb_FrenzyBurst.NS_Barb_FrenzyBurst` | `/Game/VFX/Barbarian/NS_Barb_FrenzyAura.NS_Barb_FrenzyAura` |
| Impacto Esmagador | — | `/Game/VFX/Barbarian/NS_Barb_GroundSlam_Ground.NS_Barb_GroundSlam_Ground` | — |
| Avatar da Ruína | `/Game/VFX/Barbarian/NS_Barb_AvatarCharge_Ground.NS_Barb_AvatarCharge_Ground` | `/Game/VFX/Barbarian/NS_Barb_AvatarBurst_Ground.NS_Barb_AvatarBurst_Ground` | `/Game/VFX/Barbarian/NS_Barb_AvatarAura.NS_Barb_AvatarAura` |

Passivas: sem VFX. O ataque básico usa o `hit_vfx_path` da skill `is_basic_attack` (ou o da `basic_attacks`).

### Origem de cada sistema

| Sistema | Base | Ajustes |
|---|---|---|
| `NS_Barb_HitSpark` / `_Big` | `NS_BasicHit` | bola de fogo desligada; faísca amarelo-metal; flash 190 no `_Big` |
| `NS_Barb_HeavyImpact_Ground` | `NS_AOE_Attack_1` + emissor `Spiral1` do `NS_Free_Magic_Slash` | arco de corte vermelho sobre a rachadura |
| `NS_Barb_GroundSlam_Ground` | `NS_AOE_Attack_1` | rachadura 1,7×, vida 2,5 s |
| `NS_Barb_WarCry_Ground` | `NS_Free_Spells_Shockwave` | vermelho; runas desligadas; escala 1,3 |
| `NS_Barb_WarCryBuff` | `NS_Free_Spells_Buff_Fire` | loop infinito; onda desligada; escala 0,7 |
| `NS_Barb_ChargeDust_Ground` | `NS_Free_Magic_Area1` | tons de poeira |
| `NS_Barb_ChargeTrail` | `NS_Free_Spells_Buff_Air` | loop infinito; laranja; onda desligada |
| `NS_Barb_BleedSlash` | `NS_Free_Magic_Slash2` | cor de sangue; 0,5 s; raio desligado |
| `NS_Barb_SelfHeal` | `NS_Healing_1` | âmbar; 1,5 s |
| `NS_Barb_CounterHit` | `NS_BasicHit_2` | vermelho-alaranjado |
| `NS_Barb_FrenzyBurst` | `NS_Free_Magic_Hit2` | vermelho |
| `NS_Barb_FrenzyAura` | `NS_RedAura1` | partículas vermelhas saindo da malha do corpo |
| `NS_Barb_AvatarCharge_Ground` | `NS_Free_Magic_Circle1` | círculo vermelho |
| `NS_Barb_AvatarBurst_Ground` | `NS_Free_Spells_Explosion` | vermelho; escala 1,2 |
| `NS_Barb_AvatarAura` | `NS_Free_Spells_Aura_Fire` | vermelho profundo |

Cores e escalas ficam nos **User Parameters** de cada sistema (`User.Color_*`, `User.Scale_All`): abra o sistema → painel **User Parameters** para retocar sem mexer nos emissores.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraGameInstance::PlaySkillCastVfx` | `Core/UmbraGameInstance.cpp` | `vfx_path` em quem lança; `hit_vfx_path` no alvo após `cast_time_ms`. **Alvo 0 → em quem lançou** (buffs/cura). |
| `UUmbraGameInstance::SpawnSkillNiagaraAtActor` | idem | Nasce na posição do ator (centro da cápsula). **Sistema com nome terminando em `_Ground` nasce nos pés** (centro − meia altura da cápsula). |
| `UUmbraPersistentVfxComponent` | `Components/UmbraPersistentVfxComponent.cpp` | `buff_vfx_path` em loop preso à malha do corpo enquanto o buff está ativo. |

**Convenção:** efeito de chão (rachadura, onda, círculo, poeira) → nome termina em `_Ground`. Efeito de corpo (faísca, corte, flash) → sem sufixo.

---

## 5) Manager — passo a passo

1. **UmbraManager** → **Skills** → filtre a classe Barbarian.
2. Para cada skill, cole os paths da tabela da §1 nos campos de VFX.
3. **Salvar** (o Manager envia o reload das skills para a zone).

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**.

| Cenário | Esperado |
|---------|----------|
| Ataque básico em NPC | Faísca no alvo junto do impacto do machado |
| Golpe da Ruína | Rachadura **no chão** sob o alvo + arco vermelho |
| Rugido de Guerra | Onda de choque no chão sob ela; brasas em loop por 15 s |
| Recuperação / Frenesi / Avatar (sem alvo) | Efeito de impacto aparece **nela** |
| Corte Dilacerante | Corte vermelho curto no alvo (não fica 4 s) |
| Outro jogador lança | O remoto vê os mesmos efeitos |

Logs: `"[UmbraGI] Niagara skill nao carregou: <path>"` indica path errado (copie a referência do Content Browser).

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Rachadura flutuando na cintura | Nome sem `_Ground` ou módulo UE antigo | Conferir o nome do sistema; recompilar o módulo |
| Efeito de buff some logo | Emissor em `Loop Behavior = Once` | Sistema → emissor → **Emitter State → Loop Behavior = Infinite** |
| Nada aparece em skill sem alvo | Módulo UE antigo | Recompilar (`PlaySkillCastVfx` com fallback para quem lançou) |
| Efeito no lugar errado do corpo | Efeito de corpo nasce no centro da cápsula | Ajustar `Position Offset` no emissor, ou usar um sistema `_Ground` |
| Path com redirector | Renomeação recente | **Fix Up Redirectors** na pasta `/Game/VFX/Barbarian` |

## 8) Referência de código

- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` (`PlaySkillCastVfx`, `SpawnSkillNiagaraAtActor`)
- Servidor: `src/zone/CombatCoreEngine.cpp` (`loadSkillAnimPaths`, `broadcastSkillCast`)
- Animações e tempos de impacto: `docs_main/ANIMACOES_BARBARIAN_F_PROMPTS.md`

**Fim do guia.**
