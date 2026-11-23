# GUIA — Companheiros etapa 2: consumíveis pela zone + elemental (UE 5.6.1)

Esta etapa faz duas coisas:
- o uso de poção (e de qualquer consumível) passa a ser **decidido pela zone**;
- entra o **elemental**, um companheiro que usa poções sozinho seguindo regras do jogador.

O **C++ já resolve** a rede, o cooldown, a bolsa, a aplicação da vida, da mana e do buff, as regras e a janela. No Editor você só cria a linha de regra e o painel de regras na janela de companheiros. Se você pedir, eu monto pelo MCP depois que o UE estiver compilado.

## Objetivo

- Poção pela bolsa ou pela barra: o cliente manda o **127**, a zone confere se você está vivo, o cooldown do subtipo e a bolsa. Depois desconta 1 unidade, aplica o efeito e responde com o **128**.
- A vida e a mana chegam pelo **87**. O número flutuante vai para o grupo e para quem está perto pelo **95**, e o buff pelo **85**.
- **Elemental:** com o "Espírito da Fonte" invocado, cada regra é um gatilho + % + item:
  - **Vida abaixo de X%** → poção de vida;
  - **Mana abaixo de X%** → poção de mana;
  - **Buff ausente** → elixir.
- A zone confere as regras a cada `auto_check_ms` e usa **uma poção por checagem**, na ordem das regras. No chat aparece: *"Espírito da Fonte usou Poção de Vida Menor (+150 vida)"*.
- Sem o item na bolsa, o elemental avisa *"Espírito da Fonte: Sem Poção de Vida Menor na bolsa."* e só tenta aquela regra de novo depois de 30 s.
- **Mudanças de segurança:**
  - `use_item.php` recusa poções (409 `use_via_zone`); só o selo de companheiro continua no PHP;
  - o opcode **86** não altera mais a vida e a mana atuais. Se o cliente manda um valor diferente do da zone, ela ignora e devolve o 87 real. Isso fecha o "curar abrindo a ficha" (o PHP devolvia a vida antiga do banco).
- Opcodes **novos**: 127, 128, 129, 130. O 94 continua existindo, mas o cliente novo não manda mais nas poções.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `companion_templates.max_rules` | Manager → **Companheiros** → "Regras que o jogador pode configurar" | 1 a 4 regras por elemental |
| `companion_templates.auto_check_ms` | Manager → Companheiros → "Intervalo de checagem" | Tempo de reação do elemental (250 a 5000 ms) |
| `player_companions.auto_rules_json` | (jogo; gravado pela zone no 129) | `[{"trigger":"hp_below","threshold":40,"item_template_id":7}]` |
| `item_templates.stats_json` / `use_cooldown_ms` | Manager → **Itens** | Efeito e cooldown que a zone usa (`health_restore`, `mana_restore`, `duration` + `*_buff`) |

Depois de mudar poções na aba Itens, use **Companheiros → Recarregar consumíveis** (`reload_consumables`).

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_companion_elemental_rules.sql
```

Ele cria o elemental de teste `ELEM_SPRING` ("Espírito da Fonte", 2 regras, 500 ms; id 2 no dev) e o **Selo do Espírito da Fonte** (item 137 no dev).

---

## 0) Pré-requisitos

1. `zone_server` recompilado (já feito) e reiniciado.
2. Manager recompilado (já feito): fechar e abrir de novo.
3. Rebuild do módulo `UmbraEternumUE` (classe nova `UmbraElementalRuleRowWidget`; mudanças no GameInstance e na janela).
4. PHP já sincronizado com o WAMP.
5. **Parar o PIE** antes de editar o Designer.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_ElementalRuleRow` (novo) | `/Game/Widgets/UI/Companions/WBP_ElementalRuleRow` | `UmbraElementalRuleRowWidget` |
| `WBP_Companions` (painel novo) | `/Game/Widgets/UI/Companions/WBP_Companions` | `UmbraCompanionWindowWidget` |

---

## 1) Designer — `WBP_ElementalRuleRow`

```
[HorizontalBox — raiz]
  Combo_Trigger      ← NOVO (ComboBoxString, sem opções: o C++ preenche)
  Combo_Threshold    ← NOVO (ComboBoxString; some em "Buff ausente")
  Combo_Item         ← NOVO (ComboBoxString, Fill)
  Btn_RemoveRule     ← NOVO (Button) + Text "X"
```

| Nome do bind | Tipo | Função |
|--------------|------|--------|
| `Combo_Trigger` | `ComboBoxString` | Vida abaixo de / Mana abaixo de / Buff ausente |
| `Combo_Threshold` | `ComboBoxString` | 10% … 90% |
| `Combo_Item` | `ComboBoxString` | Consumíveis da bolsa que servem para o gatilho (`health_potion`, `mana_potion`, `buff_potion`). Item gravado que acabou aparece como "(sem na bolsa)". |
| `Btn_RemoveRule` | `Button` | Remove a linha (precisa **Salvar regras** depois) |

## 1.1) Designer — painel de regras na `WBP_Companions`

Dentro de `VB_Detail`, depois de `Panel_LootFilter`:

```
Panel_ElementalRules      ← NOVO (VerticalBox; o C++ mostra só com elemental selecionado)
  Text_RulesHint          ← NOVO (TextBlock)
  VB_RuleList             ← NOVO (VerticalBox: linhas criadas em C++)
  HBox_RuleButtons        ← HorizontalBox
    Btn_AddRule + Text "Adicionar regra"     ← NOVO
    Btn_SaveRules + Text "Salvar regras"     ← NOVO
```

| Nome do bind | Tipo | Função |
|--------------|------|--------|
| `Panel_ElementalRules` | qualquer `Widget` | Contêiner do painel |
| `Text_RulesHint` | `TextBlock` | "Uso automático de poções (N de M regras)…" |
| `VB_RuleList` | `VerticalBox` | Recebe as linhas |
| `Btn_AddRule` | `Button` | Desabilita ao chegar em `max_rules` |
| `Btn_SaveRules` | `Button` | Habilita quando há mudança; envia o 129 |

Class Defaults da `WBP_Companions` (opcional): **Rule Row Class** = `WBP_ElementalRuleRow`. Se ficar vazio, o C++ carrega o path padrão.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `ConsumableService` | `src/zone/ConsumableService.cpp` | Thread própria: cache de consumíveis (`item_templates`), cooldown por subtipo e buffs de item (carregados do DB no 1º uso), bolsa (sem equipado nem leilão), desconto da pilha, gravação de cooldown e `player_item_buffs`; varredura do elemental a cada 100 ms usando só o cache; validação e gravação das regras (129 → 130) |
| `CombatCoreEngine` | `processConsumableEffect` | No combat worker: recarrega stats se houver buff, aplica vida e mana (clamp no máximo), grava em segundo plano e envia 87, 95 (grupo/AOI), 85 e o 128 ao dono |
| `CompanionService` | `summon` / `dismiss` | Elemental invocado → regras do DB vão para o `ConsumableService` (e o 130 ao dono); recolhido → para de usar |
| `MovementServer` | branch 127/129 e 86 | 127/129 só enfileiram; o 86 não altera vida e mana atuais (log `vitals 86 ignorado`) |
| GameInstance | `Core/UmbraGameInstanceCompanions.cpp` | `SendConsumableUseRequest` (bolsa e barra), handler do 128 (cooldown, inventário, buffs, número flutuante, quest "usar item", chat do elemental), `SetElementalRules` e handler do 130 |
| Janela | `UI/UmbraCompanionWindowWidget.cpp`, `UI/UmbraElementalRuleRowWidget.cpp` | Rascunho das regras, adicionar/remover/salvar, opções de item pela bolsa |

> **Não** adicione lógica no Event Graph.

---

## 4) Protocolo

### 127 `ConsumableUseRequest` (cliente → zone)
`[inventoryId:u32][itemTemplateId:u32]`. Um dos dois é 0; com `inventoryId` 0 vale a primeira pilha do template. São 9 bytes.

### 128 `ConsumableUseResult` (zone → dono)
`[status:u8][source:u8][itemTemplateId:u32][inventoryId:u32][qtyRemaining:u32][cdExpiresAt:i64][cdTotalMs:u32][hp:i32][mp:i32][buffValue:i32][buffDurationMs:u32][buffExpiresAt:i64][subtype][buffKey][message]`

- `status`: 0 ok, 1 cooldown, 2 sem item, 3 morto, 4 inválido, 5 erro.
- `source`: 0 jogador, 1 elemental.
- Nos casos de ok, `message` é o nome do item; nos de erro, é o motivo.
- As strings vão como `u16` + UTF-8 (subtype/buffKey até 64 bytes, message até 128).

### 129 `ElementalRulesSet` (cliente → zone)
`[companionId:u32][count:u8]` + count × `[trigger:u8][threshold:u8][itemTemplateId:u32]`, com no máximo 4 regras. `trigger`: 1 vida, 2 mana, 3 buff ausente.

### 130 `ElementalRulesState` (zone → dono)
`[status:u8][companionId:u32][maxRules:u8]` + o mesmo bloco de regras. `status`: 0 ok, 1 inválido (vem com as regras gravadas), 2 não possui, 3 erro. Também é enviado ao invocar o elemental.

## 5) Manager — passo a passo

1. **Companheiros** → **Atualizar** → editar `ELEM_SPRING`: mesh/animações (opcional), **Regras** e **Intervalo** → **Salvar**. O Manager manda o `reload_companions`.
2. Mudou alguma poção na aba **Itens**? → **Companheiros → Recarregar consumíveis**.

---

## 6) QA (PIE)

PIE com **2 jogadores** (A e B), zone reiniciada, cliente recompilado.

| # | Cenário | Esperado |
|---|---------|----------|
| E1 | A usa Poção de Vida pela **bolsa** com vida baixa | Vida sobe (87); número flutuante em A **e** em B; poção −1; cooldown na barra |
| E2 | A usa a poção pela **barra** | Igual ao E1; a quantidade na barra atualiza |
| E3 | A usa de novo durante o cooldown | "Item em recarga" (cliente) ou "Em recarga. Aguarde Ns." (zone) |
| E4 | A usa um **Elixir** | Buff aparece em A e no ícone que B vê; stats sobem na ficha |
| E5 | A **abre a ficha (C)** logo depois de levar dano | A vida **não** volta ao valor antigo (log da zone: `vitals 86 ignorado`) |
| E6 | Chamar `use_item.php` com uma poção (Postman/curl) | 409 `use_via_zone`; nada muda no banco |
| E7 | A usa o **Selo do Espírito da Fonte**, abre P, seleciona e **Invoca** | Elemental aparece; painel de regras com "0 de 2 regras" |
| E8 | Adicionar "Vida abaixo de 50%" + Poção de Vida e **Salvar regras** | Status "Regras enviadas."; reabrir a janela mantém a regra |
| E9 | Levar dano até < 50% | Em ≤ 0,5 s o elemental usa a poção; chat "Espírito da Fonte usou…"; respeita o cooldown |
| E10 | Regra de **Buff ausente** com um elixir | Elemental usa o elixir quando o buff acaba (não antes) |
| E11 | Acabar a poção da regra | Aviso "Espírito da Fonte: Sem … na bolsa." no máximo a cada 30 s |
| E12 | Tentar 3 regras num elemental de 2 | **Adicionar regra** desabilita na 2ª |
| E13 | Recolher o elemental com vida baixa | Para de usar poções |
| E14 | A morre com a regra ativa | O elemental não usa nada enquanto A está morto |
| E15 | Relogar com o elemental invocado | Volta sozinho com as regras (130 ao entrar) |
| E16 | Pet + elemental invocados juntos | Os dois seguem A; pet coleta e elemental usa poções |

Logs esperados na zone:
- `"[ConsumableService] N consumíveis carregados"`;
- `"[ConsumableService] player X usou 7 (Poção de Vida Menor) fonte=1 restam=…"`;
- `"[ConsumableService] player X gravou 1 regra(s) no elemental 2"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| "Este item é usado pelo servidor do jogo. Atualize o cliente." | Cliente antigo chamando o PHP | Recompilar o UE |
| Poção não faz nada, sem 128 | Zone antiga ou o WS não autenticou (opcode 109) | Reiniciar a zone nova; ver `opcode 127` no Output Log |
| "Este item não pode ser usado." | `stats_json` sem `health_restore`/`mana_restore`/`*_buff`+`duration`, ou `item_type` ≠ consumable | Corrigir na aba Itens → **Recarregar consumíveis** |
| Regras recusadas ("Regras inválidas") | Item não combina com o gatilho (ex.: elixir em "Vida abaixo de") ou acima de `max_rules` | Escolher o item certo; conferir `max_rules` |
| Elemental não usa nada | Sem regras salvas, poção em cooldown, ou estado ainda não aquecido no combate | Salvar as regras; esperar o cooldown; levar/causar um hit |
| Vida "pula" de volta depois da poção | Cliente antigo aplicando vida localmente | Recompilar o UE (o 87 da zone manda) |

## 8) Referência de código

- Servidor:
  - `src/zone/ConsumableService.{hpp,cpp}`;
  - `src/zone/CombatCoreEngine.cpp` (`processConsumableEffect`);
  - `src/zone/MovementServer.hpp` (127/129, 86, `broadcastToPartyAndNearby`);
  - `src/zone/MovementProtocol.hpp` (127–130);
  - `tests/test_consumable_rules.cpp`.
- Cliente:
  - `Core/UmbraGameInstanceCompanions.cpp`;
  - `Core/UmbraGameInstance.cpp` (`UseInventoryItem`, `UseConsumableByTemplate`);
  - `UI/UmbraElementalRuleRowWidget.*`, `UI/UmbraCompanionWindowWidget.*`;
  - `Data/UmbraCompanionTypes.h`.
- PHP/SQL:
  - `scripts/add_companion_elemental_rules.sql`;
  - `api/inventory/use_item.php`;
  - `api/companion/get_player_companions.php`;
  - `api/admin/companion_admin_helpers.php`, `list_companion_templates.php`.
- Manager: `Views/CompanionsEditorView.xaml`, `ViewModels/MainViewModel.Companions.cs`.

**Fim do guia.**
