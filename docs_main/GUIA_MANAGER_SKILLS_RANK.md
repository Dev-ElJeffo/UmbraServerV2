# Guia rápido — editar skills e efeitos por rank (UmbraManager)

## Objetivo

Fortalecer skills por rank (1–5) e liberar CC (SILENCE/STUN/…) em ranks altos sem restart longo (hot-reload na zone).

## Passos

1. Abrir UmbraManager → aba **Skills**.
2. Filtrar/buscar → **Editar** a skill.
3. Ajustar `power_coef`, custos, CD, `effects_json` base.
4. Na grade **Rank scaling**:
   - `Power+` / `Cost+` / `CD-` / `Dur+` por rank
   - `extra_effects_json` exemplo rank 3:
     ```json
     [{"type":"SILENCE","duration_ms":1500,"chance_percent":100}]
     ```
   - rank 5: `STUN` 1000 ms
5. **Salvar skill** (grava `skills` + upsert de todos os ranks da grade).
6. **Recarregar no Zone** (comando admin TCP `reload_skills` em cada zone autenticada).
7. No jogo: subir `player_skills.current_rank` e testar cast.

## Seed SQL

```bash
mysql -u root -p umbra_eternum < www/umbra_api/scripts/seed_skill_rank_scaling_defaults.sql
```

## QA mínimo

| Caso | Esperado |
|------|----------|
| Sem row scaling, rank 3 | power ≈ base×1.2 |
| Row com power_coef_bonus | dano sobe pelo bonus |
| Rank 3 + SILENCE | alvo silenciado |
| Rank 5 + STUN | alvo stunned |
| Reload após edit | efeito no próximo cast sem restart |
