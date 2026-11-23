# GUIA — Padrão de descrição das skills (UE 5.6.1)

Continuação de `GUIA_SKILLS_PROGRESSAO_UE561.md` (R1–R6).

## Objetivo

Antes, o tooltip mostrava textos escritos à mão que não batiam com a skill: placeholders que liam o `secondary_coef` em vez do efeito real, e textos de outra skill. A lista de efeitos saía crua ("Buff · physical_defense 15 50").

Agora o tooltip segue um padrão fixo, e os números saem sempre dos dados, já com o rank.

## Dados / Manager

| Bloco | Origem | Regra |
|-------|--------|-------|
| **Resumo** | `skills.description` | Uma frase, até ~90 caracteres, **sem números**, dizendo o que a skill faz |
| **Efeitos** | `skills.effects_json` (gerado pelo UE) | Uma frase por efeito: ação + valor + em quem + condição + duração/chance |
| **Nota** (opcional) | `skills.tooltip_template` | Só para regra que os efeitos não explicam; aparece abaixo do resumo |

Exemplo (Golpe da Ruína, rank 1):

```
Golpe brutal que fere o alvo e acelera seus próximos ataques.
• Causa 180% do ataque físico ao alvo.
• +25% de chance de ataque duplo em você por 5 s.
```

## 0) Pré-requisitos

| Passo | Comando / ação |
|-------|----------------|
| SQL | `www/umbra_api/scripts/fix_skills_descricoes.sql`. **Já rodei no banco de dev**; em outro banco: `mysql -u root -p umbra_eternum < www\umbra_api\scripts\fix_skills_descricoes.sql` |
| PHP | Sync do WAMP (`scripts_main\sync_umbra_api_to_wamp.bat`); já sincronizado |
| UE | Compilar: `Core/UmbraGameInstance.{h,cpp}`, `Data/UmbraSkillDataStructures.h`, `UI/UmbraBuffLabelHelper.{h,cpp}` |
| Manager | Recompilar com o Manager fechado |

O SQL guarda o texto antigo em `skills_desc_backup`. Para desfazer:

```sql
UPDATE skills s JOIN skills_desc_backup b ON b.skill_id = s.skill_id
   SET s.description = b.description, s.tooltip_template = b.tooltip_template;
```

## 1) Designer

Nada a fazer. O `WBP_SkillTooltip` e o `WBP_SkillBook` usam os mesmos binds (`VB_Content`, `Tooltip_Description`, `Tooltip_Effects`, `SkillBook_Description`, `SkillBook_Effects`).

## 2) O que o C++ já faz (não precisa de Graph)

- `UmbraSkillEffectsParse::FormatOneEffect` monta a frase de cada efeito:
  - **em quem:** a mesma regra da zone (`resolveEffectTarget`): `target_override` vence; um efeito benéfico numa skill ofensiva vai para "você";
  - **área:** "nos inimigos ao seu redor (3 m)" (`range_max` = 0) ou "nos inimigos em 3 m";
  - **dano:** "do ataque físico" ou "do ataque mágico", conforme o `damage_type` (a API passou a mandar esse campo e o `include_caster`);
  - **stats** em pt-BR (`FUmbraBuffLabelHelper::GetStatPhraseLabel`, espelho de `StatKeyMapping.hpp`);
  - **condições:** gatilhos (`on_crit_received`, `on_dodge`, `on_attack_received`, `ally_damaged`), `health_below_percent`, `guaranteed_crit` e `crit_if_stealthed`.
- **Valores:** saem do `effects_json` que o PHP já escala pelo rank. Isso também corrige o dano que aparecia escalado duas vezes nos ranks altos.
- **Ordem:** a lista segue a ordem do `effects_json`; o CC não é mais jogado para o topo.
- **Texto do tooltip:** `FormatSkillTooltipText` = resumo + nota, com os placeholders `{eN}` expandidos (`ExpandSkillEffectPlaceholders`).
- **"★ Novo:" da R6:** usa as mesmas frases.

### Frases por tipo de efeito

| Tipo | Frase |
|------|-------|
| DAMAGE | Causa **X%** do ataque físico/mágico ao alvo / nos inimigos em N m (+ "sempre crítico" / "crítico garantido saindo da furtividade") |
| DOT | Dano contínuo no alvo: **X%** do ataque físico a cada T s por D s |
| HOT | Cura / Restaura **X%** da vida / mana máxima \<onde\> a cada T s por D s |
| HEAL / RESOURCE_RESTORE | Cura **X%** da vida máxima \<onde\> (+ **Y%** do ataque mágico, numa skill sem dano) |
| SHIELD | Escudo de **N + X%** da vida máxima \<onde\> por D s |
| BUFF_STAT / DEBUFF_STAT | **+N e +X%** / **−X%** de \<stat\> \<onde\> por D s (passiva: sem "onde" e sem duração) |
| STUN / SILENCE / ROOT | Atordoa / Silencia / Enraíza o alvo por D s (X% de chance) |
| SLOW | Lentidão de **X%** \<onde\> por D s |
| INTERRUPT | X% de chance de interromper a conjuração do alvo (e silenciar por D s) |
| KNOCKBACK | Empurra o alvo N m / Puxa o alvo até você (`value_flat` negativo) |
| TAUNT | Provoca os inimigos por D s: eles passam a atacar você |
| STEALTH / INVULNERABLE | Fica furtivo por D s; atacar revela você / Fica invulnerável por D s |
| REFLECT | Reflete **X%** do dano recebido |
| LIFESTEAL / MANASTEAL | Recupera **X%** do dano causado como vida / mana |
| EXECUTE | Com o alvo abaixo de N% de vida: **+X%** de dano no golpe |
| TELEPORT / CLEANSE | Teleporta você até N m à frente / Remove os efeitos negativos \<onde\> |
| Gatilho | "Ao receber um crítico: …", "Ao esquivar: …", "Ao ser atacado (uma vez): …", "Quando um aliado leva dano: …" |
| Condição de vida | "Com sua vida abaixo de 30%: …" |

## 3) Fluxo de nós

Não se aplica.

## 4) Protocolo

Nenhum opcode novo. `get_available_skills.php` ganhou dois campos por skill: `damage_type` e `include_caster`.

## 5) Manager — passo a passo

1. **Skills → Editar** a skill.
2. **Resumo (description):** uma frase, sem números. Bons exemplos:
   - "Arremessa uma corrente que fere, puxa o inimigo até você e o desacelera.";
   - "Reação: ao se esquivar, contra-ataca na mesma hora.".
3. **Nota (tooltip_template):** deixe vazia. Use só para regra extra, por exemplo "Não acumula com outras auras.".
   - Placeholders: `{e1}` (frase inteira do efeito 1), `{e1.value}`, `{e1.flat}`, `{e1.duration}` (segundos) e `{e1.chance}`, numerados na ordem dos efeitos.
4. **Calcular prévia:** novos alertas aparecem quando o resumo está vazio, tem números ou passa de 120 caracteres, ou quando a nota tem placeholder antigo (`{value}`, `{debuff}`...).
5. **Salvar skill → Recarregar no Zone** e reabrir o livro de skills no PIE.

## 6) QA (PIE)

| # | Passo | Esperado |
|---|-------|----------|
| D1 | Hover no Golpe da Ruína (rank 1) | Resumo de uma linha; "Causa 180% do ataque físico ao alvo."; "+25% de chance de ataque duplo em você por 5 s." |
| D2 | Mesma skill no rank 5 | Valores do rank 5, sem escalar duas vezes |
| D3 | Pele Endurecida (passiva) | "+50 e +15% de defesa física." e "+30 e +10% de resistência a crítico." (sem duração) |
| D4 | Pisotear | "nos inimigos ao seu redor (3 m)", "Lentidão de 30%… por 3 s", "40% de chance de interromper a conjuração dos inimigos" |
| D5 | Arremesso de Corrente | "Puxa o alvo até você." |
| D6 | Retaliação Primitiva / Ripostar | "Ao receber um crítico: causa…" / "Ao esquivar: causa…" |
| D7 | Golpe nas Sombras | "(crítico garantido saindo da furtividade)" |
| D8 | Sangue Quente | "Com sua vida abaixo de 30%: +25% de chance de crítico." |
| D9 | Elixir Ofensivo com Incluir caster | "no aliado e em você" (se `include_caster` ligado) |
| D10 | R6 numa skill com marco | "★ Novo:" com a frase nova |
| D11 | Manager: escrever "Causa 200% de dano" no resumo | Alerta "Resumo com números" |

## 7) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Resumo antigo no tooltip | Livro carregado antes do SQL | Reabrir o livro de skills (recarrega as skills) |
| "do ataque físico" numa skill mágica | `damage_type` da skill está PHYSICAL | Ajustar **Dano base** no Manager |
| Efeito aparece "em você" e devia ir no aliado | Efeito benéfico numa skill ENEMY/AREA vai para o caster (regra da zone) | Pôr `target_override` no efeito |
| Stat aparece em inglês com espaços | Stat fora da tabela | Corrigir o `target_stat` (o alerta "stat desconhecido" da prévia aponta) |
| Nota com `{value}` mostrando número errado | Placeholder antigo (lê `secondary_coef`) | Trocar por `{e1.value}` |

## 7.1) Correção de dados (`fix_skills_dados_v1.sql`)

A revisão das descrições achou dados que não batiam com a R4 (`R4_PROPOSTA_KITS.md`). **Já rodei no banco de dev.** O script faz backup antes, nas tabelas `skills_dados_backup_v1` e `skill_rank_scaling_backup_v1`; o comando para desfazer está no cabeçalho dele.

| Skill | Antes | Depois |
|-------|-------|--------|
| Golpe Consagrado | power 145, DAMAGE 155; DOT sem stat | power 155; DOT em `health` |
| Julgamento | power 170, DAMAGE 200 | power 200 |
| Dreno Carmesim | DAMAGE 135 | 120 (R4) |
| Abertura do Portal Menor | DAMAGE 300 | 250 (R4) |
| Oração de Campo | Cura só em você (`target_override` SELF), sem bônus de ataque mágico | Cura o aliado (sem alvo, você) 25% + 40% do ataque mágico (R4) |
| Milagre das Cinzas | Cura só em você | Cura os aliados na área, como os buffs de defesa |
| Escudo de Voto | skill 10s, efeitos 30s | skill 30s |
| Barreira Rúnica | skill 10s, escudo 60s | skill 60s (o marco de resist. silêncio também fica 60s) |
| Corte Técnico, Sequência Disciplinada, Lâmina Precisa | Buff sem duração (a zone já aplicava 5s por padrão) | 5s explícitos |
| Golpe Consagrado / Julgamento (marcos de resistência) | Sem duração | 5s explícitos |
| Dreno Carmesim, Neutralizador | `redirect_to_self` sem efeito (só vale em reação) | Removido |
| Interposição | Resumo dizia "devolve ao agressor" | "você assume parte dele no lugar" (o efeito redireciona o dano) |

O tooltip também passou a espelhar a zone: buff, debuff, escudo ou CC sem duração mostra **5 s**.

Depois do SQL: **Manager → Skills → Recarregar no Zone** (ou reiniciar a zone) e reabrir o livro no PIE.

QA extra:

| # | Passo | Esperado |
|---|-------|----------|
| D12 | Oração de Campo com um aliado selecionado | O **aliado** recebe a cura (não você) |
| D13 | Milagre das Cinzas com o grupo perto | Todos na área recebem a cura |
| D14 | Escudo de Voto | Buff de 30s; o tooltip mostra 30 s |
| D15 | Dreno Carmesim / Portal Menor | Tooltip e dano com 120% / 250% |
| D16 | Interposição | "Quando um aliado leva dano: você assume 50% do dano no lugar dele." |

## 8) Referência de código

| Arquivo | O que tem |
|---------|-----------|
| `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` | `UmbraSkillEffectsParse` (`FEffectPhraseContext`, `FormatOneEffect`), `FormatSkillEffectsDisplay`, `FormatSkillTooltipText`, `ExpandSkillEffectPlaceholders` |
| `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraBuffLabelHelper.cpp` | `GetStatPhraseLabel` |
| `www/umbra_api/api/skills/get_available_skills.php` | `damage_type`, `include_caster` |
| `www/umbra_api/api/admin/preview_skill_ranks.php` | Alertas do padrão |
| `www/umbra_api/scripts/fix_skills_descricoes.sql` | Resumos das 96 skills + backup |
