# Guia rápido — editar skills e efeitos por rank (UmbraManager)

## Objetivo

Fortalecer skills por rank (1–5) e liberar CC (SILENCE/STUN/…) em ranks altos sem restart longo (hot-reload na zone).

Seed de QA (`BARB_RUIN_STRIKE`): **rank 1 = STUN 2000 ms** (teste imediato), rank 3 = SILENCE, rank 5 = STUN extra. STUN e SILENCE não se apagam mais (chave inclui `target_stat` / `effect_type`).

## Passos

1. Abrir UmbraManager → aba **Skills**.
2. Filtrar/buscar → **Editar** a skill.
3. Ajustar `power_coef`, custos, CD, `effects_json` base.
4. Na grade **Rank scaling**:
   - `Power+` / `Cost+` / `CD-` / `Dur+` por rank
   - **StunRes+ / SilenceRes+ / RootRes+ / SlowRes+**: % de resistência de CC no **caster** (grava `BUFF_STAT` em `extra_effects_json`).
   - `extra_effects_json` exemplo rank 3:
     ```json
     [{"type":"SILENCE","duration_ms":1500,"chance_percent":100,"resist_penetration":0}]
     ```
   - `resist_penetration` (0–100): ignora essa parcela da resist do **alvo** no roll de CC.
   - rank 5: `STUN` 1000 ms
   - rank 1 (QA): `STUN` 2000 ms — ver seed SQL
5. **Salvar skill** (grava `skills` + upsert de todos os ranks da grade).
6. **Recarregar no Zone** (comando admin TCP `reload_skills` em cada zone autenticada).
7. No jogo: subir `player_skills.current_rank` e testar cast.

Fórmula do CC no combate: `chance = clamp(chance_percent + caster_cc_chance − max(0, resist_alvo − resist_penetration), 0, 100)`, depois `rand() % 100`.

- `chance_percent` — efeito da skill (JSON)
- `caster_cc_chance` — `stun_chance` / `silence_chance` / … do caster (cristais/afixos em **acessórios**)
- Painel CcChances no cliente mostra **só** o bônus do personagem, não skill + bônus.

Ícones de CC no mundo: [GUIA_BP_CC_NAMEPLATE_UE561.md](GUIA_BP_CC_NAMEPLATE_UE561.md).

## Tooltip (cliente)

Power efetivo: `round(base * (1 + 0.1*(rank-1))) + Power+`. No tooltip: **180 (252)** = base (rank).

`FormatSkillTooltipText` preenche só o **template** (`Causa {damage}…`). A lista de efeitos **não** é colada na descrição.

- `{damage}` / `{value}` / `{attack}` — `180` ou `180 (252)`
- `{rank}` — rank
- `{effects}` / `{cc}` — só se o template tiver o token (CC também em `Tooltip_Effects`)

`Tooltip_Effects` / `SkillBook_Effects`: Dano, Atordoado/Silenciado/…, Chance de Double Attack, Resist.

Livro: `SkillBook_Description` + `SkillBook_Effects`. Painel filho: reparent `WBP_SkillBookDetails` → `UmbraSkillBookDetailsWidget`, instância nomeada `SkillBook_Details`.

Após upgrade de rank: **Load Available Skills** de novo (tooltip vem da API PHP). Combate: recarregar skills no Zone.

## Seed SQL

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/seed_skill_rank_scaling_defaults.sql
```

## QA mínimo

| Caso | Esperado |
|------|----------|
| Sem row scaling, rank 3 | power ≈ base×1.2 |
| Row com Power+ 0 no rank 5 | power ≈ base×1.4 (fallback 10%/rank) |
| Row com power_coef_bonus | dano = fallback + bonus |
| Rank 1 + STUN | alvo stunned (Golpe da Ruína sem upgrade) |
| Rank 3 + SILENCE | alvo silenciado |
| Rank 5 + STUN | alvo stunned **e** silenciado (ícones distintos) |
| StunRes+ 15 no caster | CC inimigo falha mais contra esse player |
| `resist_penetration` 100 vs resist 50 | chance ≈ `chance_percent` |
| Reload após edit | efeito no próximo cast sem restart |
| Tooltip rank 5 Ruína | descrição sem bullets; Effects com Dano 180 (252) + CC |
| Tooltip rank 1 Ruína | dano base + linha Atordoado 2,0 s |
