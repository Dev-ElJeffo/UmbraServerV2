# GUIA — Fórmulas de combate (UE 5.6.1)

Revisão do cálculo de dano depois do QA do duelo. Os atributos (crítico, duplo, precisão, esquiva, defesa e resistências) são **pontos**, convertidos em % com retorno decrescente: `pontos / (pontos + K)`. Nenhum valor zera o outro, e todo ponto vale um pouco. **O C++ já resolve** o cálculo na zone. A ficha mostra o % efetivo ao lado dos pontos; o WBP não muda.

## Objetivo

O que estava errado (medido no duelo ElJeffo #1 × LoMartial #32, nível 50):

| Problema | Antes | Depois |
|---|---|---|
| Crítico nunca entrava | Crítico e resistência limitados a 100 **antes** da subtração: 509 − 204 virava 100 − 100 = 0% | `5% + 70% × C/(C+400)`, vezes `1 − R/(R+400)` → LoMartial ≈ 29%, ElJeffo ≈ 6% |
| Duplo quase sem efeito | O mesmo limite de 100 | `60% × D/(D+400)`, vezes `1 − R/(R+400)` |
| Dano sempre igual | Sem rolagem | Variação de ±7% em dano, cura e escudo |
| Defesa sempre no teto | `DEF/(DEF+100)` com teto de 90%: DEF 1900 = 90% | `DEF/(DEF + 500 + 50 × nível do atacante)`, teto de 75%: DEF 1900 contra nível 50 ≈ 39% |
| Atributo contava duas vezes | A STR soma no ATK **e** a skill multiplicava por ponto: +309% na skill 96 | `escala% × S/(S+300)` → +38% |
| Acerto sempre 95% | `80 + precisão − esquiva` com pontos na casa das centenas | `80% + 15% × A/(A+300) − 40% × E/(E+300)`, entre 5% e 95% |
| PvP com 0,7 fixo no código | — | `pvp_settings.pvp_damage_mult` (Manager → PvP) |
| Refino com ATK em dobro (zone) | `attack` e `physical_attack` no mesmo refino somavam os dois | Sinônimos contam uma vez (como no PHP) |

Nenhum opcode muda. Vale para jogadores **e** NPCs (mesma calculadora).

> **Impacto no balanceamento:** com a defesa fora do teto, o dano em geral **sobe** (antes quase tudo era reduzido em 90%). Reajuste HP/DEF de NPCs e `power_coef` das skills com os testes abaixo, ou aumente os K da defesa.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `combat_formula_settings.*` (21 colunas) | Manager → **Fórmulas** | Todas as constantes acima. K maior = cada ponto vale menos |
| `pvp_settings.pvp_damage_mult` | Manager → **PvP** → "Multiplicador de dano PvP" | 0,7 = 30% a menos entre jogadores |

SQL (já aplicado no banco de dev):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_combat_formula_settings.sql
```

---

## 0) Pré-requisitos

1. SQL acima aplicado.
2. PHP sincronizado com o WAMP: `get_combat_formulas.php`, `save_combat_formulas.php`, `helpers/combat_formula_helper.php`, `helpers/character_info_helper.php`, `get/save_pvp_settings.php`.
3. `zone_server` recompilado e reiniciado. No log deve aparecer `[CombatCoreEngine] fórmulas de combate: crit 5+70%/K400 ...`.
4. Rebuild do módulo `UmbraEternumUE` (ficha com %).

Nenhum asset novo; o `WBP_CharacterInfo1` continua igual.

---

## 1) Ficha do personagem

Os mesmos binds passam a mostrar `pontos (%)`, e o % é recalculado na prévia de pontos pendentes:

| Bind | Exemplo | % mostrado |
|------|---------|------------|
| `Text_CritRate` | `509 (44,2%)` | Chance de crítico contra alvo sem resistência |
| `Text_DoubleRate` | `392 (29,7%)` | Chance de duplo contra alvo sem resistência |
| `Text_CritRes` | `204 (33,8%)` | Quanto corta do crítico de quem bate |
| `Text_DoubleRes` | `53 (11,7%)` | Quanto corta do duplo de quem bate |
| `Text_Accuracy` | `475 (9,2%)` | Bônus de acerto sobre a base |
| `Text_Dodge` | `76 (8,1%)` | Quanto tira do acerto de quem bate |
| `Text_PhysDef` / `Text_MagDef` | `1900 (39,9%)` | Redução contra atacante do mesmo nível |

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Fórmulas | `src/services/CombatFormulas.hpp` | Funções puras + constantes em runtime (trocadas pelo reload) |
| Calculadora | `src/services/CombatCalculator.hpp` | Acerto, crítico, duplo, defesa por nível, escala de atributo, variação e PvP pelas fórmulas |
| Carga | `CombatCoreEngine::reloadCombatFormulas` | Lê a tabela no início e no `reload_combat_formulas` |
| PvP | `DuelService::reload` | `pvp_damage_mult` → `CombatFormulas::setPvpDamageMult` |
| Limite removido | `CombatEffectRuntime.cpp` (`clampStats`) | Crítico/duplo/resistências só com piso 0 (sem teto de 100) |
| Refino | `CharacterStateLoader.cpp` | Sinônimos do refino contam uma vez |
| Ficha | `UI/Character/UmbraStatCalculator.cpp`, `UmbraCharacterInfoWidget.cpp` | `pontos (%)` com as constantes de `stats.combat_formulas` |

---

## 4) Protocolo

Nenhuma mudança na zone. A ficha (`get_character_info`) ganhou dois campos em `stats`:
- `combat_effective`: os % efetivos já calculados;
- `combat_formulas`: as constantes, para o cliente recalcular a prévia.

## 5) Manager — passo a passo

1. **UmbraManager** → aba **Fórmulas** (perfil super).
2. Ajuste os K e tetos. **Preencher padrões** volta aos valores do SQL (precisa salvar).
3. **Salvar fórmulas**. A barra mostra "Salvo hh:mm:ss; recarregado em N/N zone(s)".
4. Multiplicador PvP: aba **PvP** → "Multiplicador de dano PvP" → **Salvar PvP**.

Guia rápido dos K:
- **Crítico raro demais:** baixe `crit_k` ou aumente `crit_bonus_max_pct`.
- **Resistência forte demais:** aumente `crit_resist_k`.
- **Defesa protegendo pouco:** baixe `defense_k_per_level`.
- **Dano muito alto em geral:** aumente `defense_k_base` ou reduza `power_coef` das skills.

---

## 6) QA (PIE)

| # | Cenário | Esperado |
|---|---------|----------|
| F1 | Bater várias vezes no mesmo alvo | Números variam ±7% (ex.: 430–500 onde antes era sempre 466) |
| F2 | Duelo LoMartial → ElJeffo | Críticos (texto de crítico) em ~1 de cada 3–4 golpes |
| F3 | Duelo ElJeffo → LoMartial | Crítico raro, mas existe |
| F4 | Tirar armadura do alvo | O dano recebido sobe na hora (a DEF não está mais no teto) |
| F5 | Tirar a arma | O dano causado cai |
| F6 | Ficha | `pontos (%)` em crítico/duplo/resistências/precisão/esquiva/defesa; mudam ao distribuir pontos |
| F7 | Manager → Fórmulas → variação 0 → Salvar | Golpe volta a ser fixo, sem reiniciar a zone |
| F8 | Manager → PvP → multiplicador 1,0 → Salvar | Dano entre jogadores sobe ~43% (de 0,7 para 1,0) |
| F9 | Matar NPCs comuns | Conferir se o TTK ficou aceitável; ajustar K ou NPCs se não |

Conferência no banco: `SELECT SUM(is_critical), COUNT(*) FROM combat_log WHERE timestamp > NOW() - INTERVAL 1 HOUR;`

## 7) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Zone avisa `combat_formula_settings ausente` | SQL não aplicado | Rodar o SQL e **Salvar fórmulas** (ou reiniciar a zone) |
| Ficha sem `%` | PHP antigo no WAMP ou UE sem rebuild | Sync do WAMP; rebuild do UE |
| Dano de NPC alto demais | A defesa deixou de reduzir 90% | Aumentar `defense_k_base`/`defense_k_per_level` ou a DEF dos NPCs |
| Crítico ainda 0 | Zone antiga rodando | Reiniciar a zone; conferir o log das fórmulas |
| "Salvar fórmulas" recusa | Valor fora da faixa | A mensagem diz o campo e a faixa |

## 8) Referência de código

- **Servidor:**
  - `src/services/CombatFormulas.hpp`;
  - `src/services/CombatCalculator.hpp`;
  - `src/zone/CombatEffectRuntime.cpp`;
  - `src/zone/CharacterStateLoader.cpp`;
  - `src/zone/CombatCoreEngine.{hpp,cpp}`;
  - `src/zone/DuelService.{hpp,cpp}`;
  - `src/admin/ServiceAdminRegister.cpp` (`reload_combat_formulas`);
  - `tests/test_combat_formulas.cpp`.
- **PHP/SQL:**
  - `scripts/add_combat_formula_settings.sql`;
  - `api/admin/get_combat_formulas.php`;
  - `api/admin/save_combat_formulas.php`;
  - `helpers/combat_formula_helper.php`;
  - `helpers/character_info_helper.php`;
  - `api/admin/get_pvp_settings.php`;
  - `api/admin/save_pvp_settings.php`.
- **Manager:**
  - `ViewModels/MainViewModel.CombatFormulas.cs`;
  - `Views/Admin/CombatFormulasView.xaml`;
  - `MainViewModel.Pvp.cs`;
  - `PvpView.xaml`;
  - `PhpAdminClient.cs`;
  - `MainWindow.xaml`;
  - `MainViewModel.cs`.
- **UE:**
  - `Data/UmbraDataStructures.h` (`FUmbraCombatFormulaParams`);
  - `Core/UmbraGameInstance.cpp` (parse);
  - `UI/Character/UmbraStatCalculator.*`;
  - `UI/Character/UmbraCharacterInfoWidget.cpp`.
