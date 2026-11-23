# PLANO — Progressão e revisão do sistema de habilidades

Continuação da revisão de skills (`GUIA_SKILLS_REVISAO_UE561.md`, fases A/B/C). Objetivo: o rank tornar a skill progressivamente mais forte, a economia de pontos forçar escolhas, o controle de combate ser usado e justo, e cada classe ter um kit coerente.

| Fase | Conteúdo | Status |
|------|----------|--------|
| A/B/C | Motor de efeitos, move cast/interrupt, dados e UX do Manager | QA feito; pendência: TELEPORT (Rift Step) |
| **R1** | Motor de rank (C++ zone + PHP) + correção do TELEPORT | QA ok; Rift Step com varredura de parede aguardando QA. Pendente: passiva com limiar de vida reavaliada em tempo real |
| R2 | Economia de pontos (PHP + SQL + UE) | QA ok. Reset no livro de skills (não em NPC) |
| R3 | Controle de combate: retorno decrescente, imunidade, teto de esquiva | Implementado, falta QA (guia §7). `cc_immune` e resistências de NPC no Manager ficam para a R5 |
| R4 | Rebalanceamento dos kits (todas as skills revisadas) | QA ok (furtividade corrigida no §8.5) |
| R5 | Manager: progressão por rank com pré-visualização e validação; CC de NPC | QA ok |
| R6 | UE: tooltip e livro "rank atual → próximo" | Implementado, falta QA (guia §10) |

Decisão: testar a fórmula de crescimento proposta e reajustar depois do QA.

---

## Diagnóstico (2026-11-22)

### Rank

- 61 das 90 skills (fora o ataque básico) **não mudam nada** ao subir de rank.
- Dano: `getEffectivePowerCoef` soma +10%/rank implícito **e** o `power_coef_bonus` de `skill_rank_scaling`. O resultado é desigual: o Golpe da Ruína vai de 180% para 324% (+80%); quem não tem linhas fica em +40%.
- Buff, debuff, escudo, DOT/HOT e CC ignoram o rank.
- `duration_bonus_ms` soma na duração da skill, mas a duração do efeito tem prioridade, então nunca vale (bug).
- Passivas ignoram o rank (`CharacterStateLoader` e `character_info_helper.php`).
- Passiva com `health_below_percent` só é avaliada na carga do estado (fica desatualizada).
- `extra_effects_json` (marcos) é usado em só 6 skills.

### Economia

- 3 pontos/nível = 150 no nível 50. Maximizar as 15 skills = 75 pontos, alcançados por volta do nível 25. Sem escolhas de build.

### Kits (15 skills por classe, fora o ataque básico)

| Classe | Dano | Cura/suporte | Buff próprio | Debuff | Passiva | Reação | Controle |
|---|---|---|---|---|---|---|---|
| Barbarian | 4 | 1 (própria) | 3 | 0 | 6 | 1 | silence/stun só em ranks do Golpe da Ruína, interrupt |
| Templar | 3 | 4 | 4 | 1 | 2 | 1 | interrupt; tank sem taunt |
| DarkMage | 6 | 0 | 2 | 2 | 4 | 0 | interrupt; sem root/slow |
| Alchemist | 1 | 4 + cleanse | 8 (aliado) | 0 | 1 | 0 | interrupt |
| Assassin | 7 | 0 | 4 | 0 | 3 | 1 | slow, interrupt |
| Martial | 4 | 1 (própria) | 6 | 0 | 3 | 1 | stun, interrupt |

- ROOT, KNOCKBACK e TAUNT estão implementados e nenhuma skill usa.
- Cinco buffs grandes de dodge (+30 a +60) sem teto.
- Alchemist sem dano e com 5 elixires quase iguais.

---

## R1 — Motor de rank

1. **Crescimento único:** `valor(r) = base × (1 + crescimento × (r − 1))` em todos os números de efeito.

   | Tipo de efeito | Crescimento padrão por rank | Rank 5 vs rank 1 |
   |---|---|---|
   | DAMAGE, HEAL, SHIELD, DOT, HOT, RESOURCE_RESTORE, EXECUTE | 12,5% | +50% |
   | BUFF_STAT, DEBUFF_STAT, SLOW, REFLECT, passivas | 10% | +40% |
   | STUN, ROOT, SILENCE, KNOCKBACK, TAUNT, INTERRUPT | duração +10%/rank, chance +5 pontos/rank | +40% duração, +20 pontos |

   - `skills.rank_growth_percent` (NULL = padrão do tipo; 0 = não escala).
   - `rank_growth` por efeito no JSON sobrescreve a skill.
   - Valores negativos (debuff) crescem em módulo.
2. **Sem contagem dupla:** o crescimento substitui o +10% implícito do `power_coef`; `power_coef_bonus` continua como ajuste fino aditivo (migration zera os valores atuais).
3. **`duration_bonus_ms`** soma na duração de cada efeito que tem duração.
4. **Passivas** usam o rank do jogador (servidor e ficha PHP).
5. **Marcos:** rank 3 = utilidade, rank 5 = efeito assinatura, via `extra_effects_json` (dados na R4).
6. **TELEPORT (Rift Step):** corrigir junto (falhou no QA da fase A).

## R2 — Economia de pontos

- Rank N custa N pontos (15 por skill; 225 para tudo contra 150 disponíveis).
- Ultimates: `max_rank` 3, custo 3/4/5.
- Rank N exige nível `required_level + 4 × (N − 1)`.
- Reset de pontos pago em gold (NPC).
- Livro de skills (UE) mostra custo e nível do próximo rank.

## R3 — Controle de combate

- Retorno decrescente por categoria (stun; silence/interrupt; root/slow): 2º CC em 15s = 50% da duração; 3º = imune por 15s (texto "Imune").
- `cc_immune` nos templates de NPC (chefes).
- Teto de esquiva: dodge efetivo ≤ 60%; soma de buffs de dodge ≤ +50.
- Validar TAUNT, ROOT e KNOCKBACK em NPC e player, com visual pelo opcode 118.

## R4 — Kits (revisar todas as skills)

| Classe | Propostas |
|---|---|
| Alchemist | 5 elixires → 2 exclusivos (Ofensivo e Defensivo); novas: Frasco Ácido (dano + DOT), Gás Paralisante (área slow + root), Bomba Incendiária (área) |
| Templar | Desafio (TAUNT em área) no lugar da Égide Espiritual; Julgamento com stun no rank 5; revisar Interposição |
| DarkMage | Correntes Sombrias (ROOT), Onda do Vazio (KNOCKBACK em área); Torção da Realidade com slow; Portal Menor com stun no rank 3 |
| Barbarian | 6 passivas → 3; novas: Pisotear (área slow + interrupt), Arremesso de Corrente (root); Investida Selvagem com stun no rank 5 |
| Assassin | Passo Invisível vira STEALTH; Golpe nas Sombras com crit ao sair do stealth; Bomba de Fumaça (silence em área); cortar um buff de dodge |
| Martial | 6 → 4 buffs; novas: Chute Giratório (KNOCKBACK em área), Varredura (root curto); Execução Marcial cura ao matar |

Meta por classe: 5–6 dano, 2–3 controle, 2–3 suporte/buff, 2–3 passivas, 1 reação, 1 ultimate. Cada skill com marco no rank 3 e no rank 5.

## R5 — Manager

- Aba "Ranks": grade rank 1–5 com valores calculados, crescimento editável e marcos.
- Validação ao salvar: stat desconhecido, buff sem duração, override suspeito, skill sem progressão.

## R6 — UE

- Tooltip e livro de skills com "rank atual → próximo", valores do PHP (mesma fórmula do servidor) e marco destacado.
