# GUIA — Warnings de JSON (VaRest), depreciações e log do token (UE 5.6.1)

Rodada 4, bloco 1 (R4.2 + R4.3 itens 1–2) do `docs_main/PLANO_RODADAS_UI_UE561.md`. É só C++, sem asset, sem Graph e sem protocolo.

## Objetivo

- **Output Log limpo:** sem as milhares de linhas `LogVaRest: Warning: No field with name X of type Y`. Num PIE de login → zone eram 2.913.
- **Build sem C4996:**
  - `UImage::Brush` no nosso código;
  - `RemoveAt`/`Pop` com `bool` no VaRest.
- **Segurança:** o JWT não é mais escrito no log. Só o tamanho aparece.
- Nenhum valor lido muda: com o campo presente e do tipo certo, o resultado é o mesmo de antes. Com o campo ausente ou `null`, o default é o mesmo que o VaRest já devolvia (0 / "" / false / nullptr). A única exceção são os itens com `max_stack_size` `null`, que agora caem no default 1 que o código já usava quando o campo faltava.
- **Protocolo:** nenhum opcode alterado.

---

## 0) Pré-requisitos

1. **Fechar o editor** e fazer **rebuild** de `UmbraEternumUEEditor` (Development Editor). O plugin VaRest também recompila.
2. Nada a fazer no Designer, no Manager ou no MySQL.

## 2) O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UmbraJson::GetInt/GetFloat/GetString/GetBool/GetObj/GetArray` | `Source/UmbraEternumUE/Core/UmbraJsonHelpers.h` | Checam `HasTypedField` (a mesma checagem do VaRest) antes de ler e devolvem o default sem logar. `null` também conta como ausente; por isso `HasField` sozinho não bastava (ex.: `tooltip_template: null`) |
| `UmbraJson::FieldType` | idem | Substitui o `GetFieldTypeString`, que loga `Field with name X type unknown` quando o campo não existe. Devolve `""` sem logar. Os 48 usos do GameInstance foram trocados (consumo de item/buff, keybind, ouro) |
| 2ª passada (QA) | `Core/UmbraGameInstance.cpp` | Paths de mesh opcionais das classes (`ParseOptionalPath`, `null` no banco), `equipped_items`, e `quest_id`/`needs_reward_choice`/`reward_choices`/`rewards_granted`/`reward_granted`/`detail` das respostas de quest |
| Parsers | `Core/UmbraGameInstance.cpp`, `Core/UmbraGameInstanceEquipmentVisual.cpp` | Os ~100 acessos dos campos que apareciam no log passaram a usar o helper. Exemplos: `params.zone_id/pos_*/radius/npc_template_id/item_template_id` dos objetivos de quest; `class_name`, `skill_cost`, `scaling_stat`, flags e resource das skills; `max_stack_size`, `stackable` e `icon_path` dos itens; os paths de mesh e `anim_set_json` das classes; os campos de `get_trade_state` com a sessão encerrada |
| Ícones/cooldown | `UI/UmbraBuffIconWidget.cpp`, `UI/UmbraInventorySlotWidget.cpp`, `UI/UmbraSkillSlotWidget.cpp` | `GetBrush()` no lugar do acesso direto a `Brush` |
| VaRest (patch local) | `Plugins/VaRest/Source/VaRest/Private/VaRestJsonParser.cpp` | `EAllowShrinking::No` no lugar de `false`. **Patch nosso no plugin:** se o VaRest for atualizado, reaplicar |
| Login | `Core/UmbraGameInstance.cpp` (`OnLoginRequestComplete`, `LoadAuthToken`) | O log mostra `Token: (N caracteres)` em vez do JWT |

> Campo novo em parser: usar `UmbraJson::Get*` quando o campo for opcional ou puder vir `null`. `Get*Field` direto só para campo obrigatório, em que o warning ajuda a achar resposta quebrada.

## 6) QA (PIE)

| Cenário | Esperado |
|---------|----------|
| Rebuild | Nenhum `warning C4996` de `UImage::Brush` nem de `VaRestJsonParser.cpp` |
| Login → seleção → zone, depois abrir inventário, skills (Skill Book e barra), quests, crafting, refino e storage | Tudo igual a antes: ícones, tooltips de skill, stacks, objetivos de quest com marcador |
| Contar no `Saved/Logs/UmbraEternumUE.log`: `grep -c "No field with name"` | Bem abaixo das 2.913 de antes; meta **< 50**. As que sobrarem vêm de BP (telas de entrada, R4.1) ou de campos fora da lista; mandar a contagem para o próximo ajuste |
| Login | O log mostra `Token: (N caracteres)`, sem o JWT |
| Cooldown de skill e de item, e duração de buff | O preenchimento radial continua funcionando |
| Trade concluído/recusado | Sem os warnings de `player1_id`/`trade_session_id` |

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| `No field with name X` continua para um campo | Campo lido em ponto que não estava no log original | Mandar o campo; troca-se o acesso por `UmbraJson::Get*` |
| `No field with name X` vindo de BP (sem acesso C++ correspondente) | Nó VaRest em Blueprint (telas de entrada) | Some com a R4.1 |
| Erro de compilação em `VaRestJsonParser.cpp` após atualizar o plugin | Patch local perdido ou API mudou | Reaplicar `EAllowShrinking::No` |

## 8) Referência de código

- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraJsonHelpers.h`
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`:
  - `ParseQuestObjective`;
  - parse de skill (~`:20989`);
  - templates de item (~`:7300` e `:7527`).
- `UmbraEternumUE/Plugins/VaRest/Source/VaRest/Private/VaRestJsonParser.cpp`

**Fim do guia.**
