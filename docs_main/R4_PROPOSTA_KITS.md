# R4 — Proposta de kits por classe (para revisão)

Revisão das 90 skills (15 por classe, fora o ataque básico) dentro do `PLANO_SKILLS_PROGRESSAO.md`. **Aprovada e aplicada** (`www/umbra_api/scripts/fix_skills_r4_kits.sql`; QA no `GUIA_SKILLS_PROGRESSAO_UE561.md` §8).

## Como ler

- **Status:** `=` mantida, `~` ajustada, `★` nova (reaproveita o slot/linha de outra skill; o ID e o nível continuam os mesmos, então a skillbar e os pontos dos personagens não quebram).
- **Rank 1:** valores-base. Todos crescem pela fórmula da R1 (dano/cura +12,5%/rank, buff/debuff +10%/rank, controle +10% de duração e +5 de chance por rank).
- **Rank 3 / Rank 5:** marcos (`extra_effects_json`). Ultimates têm 3 ranks: o marco fica no rank 3.
- Valores de dano em % do ATK/MATK (`value_percent` do DAMAGE). CD em segundos, custo em mana.
- Skill de área sem alvo centraliza no próprio personagem (já suportado); essas ficam com `requires_target = 0`.

## Meta por classe

| Classe | Dano | Controle | Suporte/buff | Passivas | Reação | Ultimate |
|--------|------|----------|--------------|----------|--------|----------|
| Barbarian | 6 | 4 | 3 (+1 cura própria) | 4 | 1 | 1 |
| Templar | 4 | 4 (taunt incluso) | 7 | 2 | 1 | 1 |
| DarkMage | 8 | 5 | 2 | 3 | — | 1 |
| Alchemist | 4 | 2 | 10 | 1 | — | 1 |
| Assassin | 7 | 3 | 3 (stealth incluso) | 3 | 1 | 1 |
| Martial | 6 | 4 | 4 (+1 cura própria) | 3 | 1 | 1 |

(Uma skill pode contar em mais de uma coluna: ex. dano + controle.)

---

## Barbarian — lutador corpo a corpo, controle por impacto

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | Golpe da Ruína | ~ | Ativa / inimigo | Dano 180% + double attack +25% em si 5s | −0,5s de CD | Stun 1s (30%) | 3 / 15 |
| 4 | Pele Endurecida | ~ | Passiva | DEF +50 e +15%; **absorve Ossos de Ferro**: resist. crítico +30 e +10% | — | — | — |
| 7 | Investida Selvagem | ~ | Ativa / inimigo | Dano 140% + velocidade +30% em si 3s + interrupt 50% | Root 1s | Stun 1,2s | 8 / 25 |
| 10 | Sangue Quente | = | Passiva | Crítico +25% com vida < 30% | — | — | — |
| 14 | Rugido de Guerra | ~ | Buff / **área aliada** (500) | ATK +20% 60s no grupo por perto | resist. stun +15 | duração +20s | 90 / 40 |
| 18 | **Pisotear** | ★ (Ossos de Ferro) | Ativa / área ao redor (300) | Dano 90% + slow −30% 3s + interrupt 40% | −2s de CD | Root 1s | 14 / 35 |
| 22 | Corte Dilacerante | ~ | Ativa / inimigo | Dano 130% + DOT 40% 8s | −1s de CD | DEF −15% 6s no alvo | 6 / 30 |
| 26 | **Arremesso de Corrente** | ★ (Passo da Caçadora) | Ativa / inimigo (800) | Dano 80% + **puxa** 500uu + slow −40% 2s | Root 1s | Stun 1s | 15 / 30 |
| 30 | Recuperação Instintiva | ~ | Ativa / si | Cura 25% da vida | + HOT 10% 6s | Cleanse | 45 / 50 |
| 34 | Retaliação Primitiva | = | Reação | Ao levar crítico: dano 120% | — | — | 10 / — |
| 38 | Frenesi Carmesim | ~ | Buff / si | Double attack +35% e DEF −20% 12s | — | Lifesteal +10% 12s | 25 / 35 |
| 42 | Resistência Brutal | = | Passiva | Vida +200 e +12% | — | — | — |
| 45 | Impacto Esmagador | ~ | Ativa / inimigo | Dano 150% + esquiva −30% 6s + interrupt 60% (silêncio 1,5s) | −2s de CD | Stun 1,5s (50%) | 12 / 40 |
| 48 | Caçada Implacável | ~ | Passiva | Precisão +40 e +10%; **absorve Passo da Caçadora**: esquiva +15 | — | — | — |
| 50 | Avatar da Ruína | ~ | Ultimate / si | ATK +40%, velocidade +25%, double attack +30% 20s | (rank 3) resist. stun +30 | — | 180 / 100 |

## Templar — tanque e suporte sagrado

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | Golpe Consagrado | = | Ativa / inimigo | Dano 155% + DOT 25% 5s + interrupt 40% + resist. stun em si (marcos já existentes) | (já existe) | Silêncio 1s (40%) | 4 / 18 |
| 4 | Escudo de Voto | ~ | Buff / si | DEF +30% e **MDEF +30%** 30s (absorve a Égide) | — | redução de dano +10% | 45 / 25 |
| 7 | **Desafio** | ★ (Égide Espiritual) | Ativa / área ao redor (400) | **Taunt** nos inimigos + redução de dano +15% em si 4s | ATK −10% 6s nos inimigos | redução de dano +25% | 15 / 25 |
| 10 | Luz das Cinzas | ~ | Ativa / inimigo | Dano 160% + MDEF −15% 6s (troca o bônus contra "criatura sombria", que não existe) | −1s de CD | Silêncio 1,5s (40%) | 6 / 30 |
| 14 | Aura do Guardião | = | Aura / grupo | Resist. crítico +20 e +10% 60s | — | — | 60 / 0 |
| 18 | Passo Guardião | ~ | Buff / si | Redução de dano 20% 6s | velocidade +20% | Cleanse de root/slow | 15 / 30 |
| 22 | Julgamento | ~ | Ativa / inimigo | Dano 200% + resist. silêncio em si (marcos já existentes) | (já existe) | Stun 1,5s | 10 / 35 |
| 26 | Barreira Rúnica | ~ | Buff / si | Escudo **400** + 25% da vida 60s (hoje 5000 fixo, alto demais) | (já existe) | (já existe) | 60 / 40 |
| 30 | Oração de Campo | ~ | Cura / aliado | Cura 25% (+40% MATK) | — | Cleanse | 12 / 45 |
| 34 | Marca do Juramento | = | Debuff / inimigo | ATK e MATK −20% 10s + DOT 10% (25%) | — | Root 1s | 15 / 30 |
| 38 | Resistência Devota | = | Passiva | Vida +250 e +15% | — | — | — |
| 42 | Interposição | = | Reação | Redireciona 50% do dano de um aliado | — | — | 30 / — |
| 45 | Chama do Voto | = | Buff / área aliada | ATK e MATK +30% 60s + resist. stun (marcos existentes) | (já existe) | (já existe) | 90 / 200 |
| 48 | Escudo Inquebrável | = | Passiva | Resist. double attack +30 e +12% | — | — | — |
| 50 | Milagre das Cinzas | = | Ultimate / área aliada | Cura 35% + DEF/MDEF +50% 35s | (rank 3) Cleanse no grupo | — | 180 / 120 |

## DarkMage — conjurador de dano e controle à distância

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | Rajada de Névoa Carmesim | ~ | Ativa / inimigo | Dano 170% (cast 0,8s) | −0,5s de CD | Slow −30% 3s | 4 / 20 |
| 4 | Marca Dimensional | = | Debuff / inimigo | MDEF −25% 10s | — | + DOT 15% 10s | 10 / 25 |
| 7 | **Correntes Sombrias** | ★ (Véu Sombrio) | Ativa / inimigo (1200) | Dano 60% + **root** 2s | −2s de CD | Silêncio 1s | 12 / 30 |
| 10 | Eco Arcano | = | Passiva | Double attack +15 | — | — | — |
| 14 | Selo de Ruptura | ~ | DOT / inimigo | DOT 80% 12s | — | Silêncio 1s | 8 / 35 |
| 18 | Mente Profunda | = | Passiva | Mana +150 e +15% | — | — | — |
| 22 | Orbe da Corrupção | ~ | Ativa / área (600) | Dano 145% em área (interrupt legado 50%) | — | Slow −30% 3s | 10 / 45 |
| 26 | Reflexo do Véu | = | Passiva | Resist. crítico +25 e +8% | — | — | — |
| 30 | Dreno Carmesim | = | Ativa / inimigo | Dano 120% + lifesteal 40% | — | — | 12 / 40 |
| 34 | Convergência Rubicina | = | Buff / si | MATK +30% 12s | — | CD −10s | 25 / 35 |
| 38 | Torção da Realidade | ~ | Debuff / inimigo | Precisão −25% 8s + interrupt 50% (silêncio 2s) + **slow −30% 4s** | — | — | 15 / 30 |
| 42 | Passo Entre Fendas | = | Mobilidade / si | Blink 5m | −3s de CD | — | 15 / 25 |
| 45 | Colapso Arcano | = | Ativa / inimigo | Dano 200% (cast 1,5s) | — | Stun 1s (30%) | 18 / 60 |
| 48 | **Onda do Vazio** | ★ (Afinidade da Névoa) | Ativa / área ao redor (400) | Dano 90% + **knockback** 400uu | −3s de CD | Slow −30% 3s | 18 / 40 |
| 50 | Abertura do Portal Menor | ~ | Ultimate / área | Dano 250% em área | (rank 3) Stun 1s (50%) | — | 90 / 120 |

## Alchemist — suporte com dano alquímico

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | **Frasco Ácido** | ★ (Tônico de Campo vai para o Nv 7) | Ativa / inimigo (900) | Dano 120% + DOT 30% 6s | DEF −10% 6s | Slow −30% 3s | 4 / 15 |
| 4 | **Elixir Ofensivo** | ★ (Elixir de Força) | Buff / aliado | STR +15% e +30, INT +15% e +30, crítico +10, 60s | — | duração +30s | 20 / 25 |
| 7 | Tônico de Campo | ~ (vindo do Nv 1) | Cura / área aliada (400) | Cura 20% (+40% MATK) | — | + HOT 8% 6s | 6 / 20 |
| 10 | **Elixir Defensivo** | ★ (Composto Revigorante) | Buff / aliado | VIT +12% e +25, vida +100, DEF +10%, 60s | — | duração +30s | 20 / 25 |
| 14 | **Gás Paralisante** | ★ (Infusão Cognitiva) | Ativa / área (300) | Dano 50% + slow −40% 4s | Root 1,5s | Silêncio 1,5s | 14 / 35 |
| 18 | **Bomba Incendiária** | ★ (Destilado da Fortuna) | Ativa / área (250) | Dano 110% + DOT 35% 6s | −2s de CD | Interrupt 50% | 12 / 40 |
| 22 | Névoa Restauradora | = | Cura / área aliada | Cura 18% (+40% MATK) | — | + Cleanse | 18 / 50 |
| 26 | Revestimento Protetor | = | Buff / aliado | DEF e MDEF +20% 30s | — | redução de dano +10% | 22 / 35 |
| 30 | Estimulante de Combate | = | Buff / aliado | Velocidade +25% e double attack +20% 20s | — | — | 25 / 35 |
| 34 | Neutralizador | = | Cleanse / aliado | Remove 3 debuffs | — | + HOT 10% 6s | 15 / 40 |
| 38 | Ampola de Estabilização | = | Escudo / aliado | Escudo 350 + 25% 8s | — | — | 20 / 45 |
| 42 | Catalisador de Mana | = | HOT mana / aliado | 30% da mana em 10s | — | — | 30 / 30 |
| 45 | Bomba Disruptiva | = | Debuff / área (200) | Dano 60% + precisão −25% e crítico −20% 8s + interrupt 50% | — | Stun 1s (30%) | 20 / 45 |
| 48 | Preparação Antecipada | = | Passiva | Duração dos buffs +25% | — | — | — |
| 50 | Protocolo de Sobrevivência | = | Ultimate / grupo | Cura 30% + DEF/MDEF +25% e resist. +20 20s | (rank 3) Cleanse no grupo | — | 180 / 130 |

Saem: Soro de Precisão (DEX), Infusão Cognitiva (INT), Destilado da Fortuna (LUK) — fundidos nos dois elixires.

## Assassin — dano explosivo, furtividade

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | Golpe nas Sombras | ~ | Ativa / inimigo | Dano 160% + interrupt 50% (silêncio 1s); **crítico garantido saindo da furtividade** | −0,5s de CD | Silêncio 1,5s | 4 / 15 |
| 4 | Passo Invisível | ~ | Buff / si | **Furtividade** 5s + velocidade +20% (atacar/usar skill hostil quebra) | duração +2s | velocidade +35% | 15 / 20 |
| 7 | Lâmina Precisa | = | Ativa / inimigo | Dano 140% + precisão +30 em si 5s | — | Ignora 20% da esquiva 5s | 3,5 / 18 |
| 10 | Ataque Duplo | = | Ativa / inimigo | 2 × dano 110% | — | + 3º golpe 60% | 5 / 22 |
| 14 | Corte de Tendão | = | Ativa / inimigo | Dano 120% + slow −40% 6s | Root 1s | — | 10 / 25 |
| 18 | Sangramento Profundo | = | DOT / inimigo | DOT 70% 10s | — | DEF −10% 10s | 12 / 28 |
| 22 | Reflexos Mortais | = | Reação | Ao ser atacado: esquiva 100% por 0,5s | — | — | 15 / — |
| 26 | Caçador Noturno | = | Passiva | Crítico +25 e +10% | — | — | — |
| 30 | Recuperação Sombria | = | Passiva | Lifesteal 8% | — | — | — |
| 34 | **Bomba de Fumaça** | ★ (Véu de Fumaça) | Ativa / área ao redor (300) | **Silêncio 2s** + precisão −20% 4s nos inimigos | −5s de CD | Furtividade 2s em si | 25 / 30 |
| 38 | Execução | = | Ativa / inimigo | Dano 200% + execute 80% abaixo de 25% | — | — | 20 / 50 |
| 42 | Olho do Predador | = | Buff / si | Ignora 50% da esquiva 10s | — | crítico +15 10s | 30 / 35 |
| 45 | Passo Entre Golpes | = | Buff / si | Velocidade +35% e double attack +25% 8s | — | — | 25 / 35 |
| 48 | Nervos Frios | = | Passiva | Resist. crítico +30 e +10% | — | — | — |
| 50 | Assassinato Perfeito | = | Ultimate / inimigo | Dano 280% crítico garantido | (rank 3) + execute 50% abaixo de 30% | — | 180 / 100 |

## Martial — combo corpo a corpo e disciplina

| Nv | Skill | St | Tipo / alvo | Rank 1 | Rank 3 | Rank 5 | CD / MP |
|----|-------|----|-------------|--------|--------|--------|---------|
| 1 | Corte Técnico | = | Ativa / inimigo | Dano 150% + precisão +20 em si 5s + interrupt 40% | −0,5s de CD | — | 3 / 15 |
| 4 | Guarda Alta | = | Buff / si | DEF +25% 8s | — | redução de dano +10% | 15 / 20 |
| 7 | Sequência Disciplinada | = | Ativa / inimigo | Dano 130% + double attack +30% em si 5s | — | + 2º golpe 70% | 5 / 25 |
| 10 | Passo Tático | = | Buff / si | Velocidade +40% 4s | — | Cleanse de root/slow | 12 / 15 |
| 14 | Postura Precisa | = | Passiva | Crítico +20 e +8% | — | — | — |
| 18 | Disciplina de Ferro | = | Passiva | Resist. crítico +25 e +8% | — | — | — |
| 22 | Golpe Quebrador | = | Ativa / inimigo | Dano 160% + DEF −20% 6s + stun 1,5s | — | — | 8 / 30 |
| 26 | Ripostar | = | Reação | Ao esquivar: dano 110% | — | — | 8 / — |
| 30 | Respiração de Combate | = | Cura / si | Cura 20% | — | + Cleanse | 30 / 40 |
| 34 | **Chute Giratório** | ★ (Foco de Batalha) | Ativa / área ao redor (250) | Dano 120% + **knockback** 350uu + interrupt 40% | −3s de CD | Stun 1s (30%) | 15 / 30 |
| 38 | **Varredura** | ★ (Defesa Rotacional) | Ativa / inimigo | Dano 100% + **root** 1,5s | −2s de CD | Slow −30% 4s | 14 / 30 |
| 42 | Execução Marcial | ~ | Ativa / inimigo | Dano 180% + execute 20% abaixo de 30% + **lifesteal 20%** | — | — | 14 / 45 |
| 45 | Ritmo de Guerra | = | Buff / si | Double attack +25% e velocidade +20% 15s | — | — | 30 / 40 |
| 48 | Armadura Mental | = | Passiva | MDEF +35 e +10% | — | — | — |
| 50 | Forma do Veterano | = | Ultimate / si | ATK +30%, DEF +25%, precisão +40, esquiva +30 20s | (rank 3) resist. stun +30 | — | 180 / 100 |

---

## O que o motor precisa (R4a, C++ zone)

| Item | Para quem | Como |
|------|-----------|------|
| **Furtividade quebra ao agir** | Passo Invisível, Bomba de Fumaça | Usar skill hostil ou ataque básico remove o STEALTH do caster (opcode 104 remove) |
| **Crítico saindo da furtividade** | Golpe nas Sombras | Condição nova no DAMAGE: `"crit_if_stealthed": true` |
| **Puxar** | Arremesso de Corrente | KNOCKBACK com `value_flat` negativo puxa até ~1,5m do caster (NPC e player, com a varredura de parede) |
| **Área ao redor sem alvo** | Pisotear, Desafio, Onda do Vazio, Chute Giratório, Bomba de Fumaça | Conferir o range/alvo no `processSkillCast` com `requires_target = 0` |
| Taunt em área | Desafio | Já despacha por alvo; conferir |

## Assets novos (sua parte no Editor)

Skills ★ reaproveitam ícone/animação/VFX do slot antigo até você trocar no Manager: Pisotear, Arremesso de Corrente, Desafio, Correntes Sombrias, Onda do Vazio, Frasco Ácido, Elixir Ofensivo, Elixir Defensivo, Gás Paralisante, Bomba Incendiária, Bomba de Fumaça, Chute Giratório, Varredura.

## Perguntas

1. Aprova os kits como estão, ou quer trocar alguma skill/valor?
2. Alchemist: aceita perder Soro de Precisão, Infusão Cognitiva e Destilado da Fortuna (fundidos nos dois elixires)?
3. Barreira Rúnica: escudo de 5000 fixo vira 400 + 25% da vida. Ok?
