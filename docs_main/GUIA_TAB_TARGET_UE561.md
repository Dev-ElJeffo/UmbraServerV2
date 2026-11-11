# GUIA — Tab target (UE 5.6.1)

O **Tab** passa a alternar entre os alvos próximos (NPCs atacáveis vivos e jogadores remotos), do mais próximo para o mais distante. A troca do modo de câmera vai para o **V**. Toda a lógica está em C++. No Editor, só é preciso criar um Input Action e ajustar dois mapeamentos do `IMC_Default`.

## Objetivo

- **Tab:** seleciona o próximo alvo dentro de 25 m. Usa a mesma seleção do clique: destaque, painel de alvo, `FollowTargetNpcId` e o personagem vira para o alvo. Skill e ataque básico já miram o alvo escolhido.
- **V:** alterna o modo de câmera (antes no Tab).
- **Limite de alvos:** a configuração `TabTargetMaxCount` restringe o ciclo aos N mais próximos (**0 = livre**, 5, 10...). Ela fica salva no `UmbraClientSettings`, pronta para um seletor futuro na tela de Opções.
- **Sem opcode novo e sem mudança no servidor.** A seleção é local, como no clique.

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE`.
2. **Parar o PIE** antes de editar os assets.

**Assets:**

| Asset | Path | Tipo |
|-------|------|------|
| IA_TabTarget | `/Game/Input/Actions/IA_TabTarget` | Input Action (**NOVO**) |
| IA_ToggleCameraMode | `/Game/Input/Actions/IA_ToggleCameraMode` | Input Action (já existe) |
| IMC_Default | `/Game/Input/IMC_Default` | Input Mapping Context (já existe) |

---

## 1) Editor — Input

### 1.1 Criar `IA_TabTarget`

1. Content Browser → `/Game/Input/Actions` → botão direito → **Input** → **Input Action**.
2. Nome **exatamente** `IA_TabTarget`.
3. Abrir e conferir **Value Type** = **Digital (bool)**.
4. **Save**.

> O C++ carrega `/Game/Input/Actions/IA_TabTarget` sozinho quando o campo **Tab Target Action** do `BP_ThirdPersonCharacter` está vazio. Não é preciso preencher o BP.

### 1.2 Ajustar o `IMC_Default`

1. Abrir `/Game/Input/IMC_Default`.
2. No mapeamento de **IA_ToggleCameraMode**, trocar a tecla **Tab** por **V**.
3. Clicar em **+** em **Mappings**, escolher **IA_TabTarget** e a tecla **Tab**.
4. **Save**.

Resultado esperado:

| Tecla | Action |
|-------|--------|
| `Tab` | `IA_TabTarget` |
| `V` | `IA_ToggleCameraMode` |

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `AUmbraEternumUECharacter::TabTargetPressed` | `Source/UmbraEternumUE/UmbraEternumUECharacter.cpp` | Bind do `IA_TabTarget` (Started), com fallback por path; chama `CycleTabTarget()` no PlayerController local |
| `UUmbraPlayerSelectionComponent::CycleTabTarget` | `Source/UmbraEternumUE/Components/UmbraPlayerSelectionComponent.cpp` | Junta os candidatos no alcance, ordena por distância, aplica o limite N e seleciona via `SelectNpc` / `SelectPlayer` |
| `UUmbraGameInstance::Get/SetTabTargetMaxCount` | `Source/UmbraEternumUE/Core/UmbraGameInstanceClientSettings.cpp` | Lê e grava o limite (clamp 0..50) no slot `UmbraClientSettings` |
| `UUmbraGameInstance::GetAllRemotePlayerActors` | `Source/UmbraEternumUE/Core/UmbraGameInstance.cpp` | Lista os actors de jogadores remotos válidos |

**Regras do ciclo:**

- **Candidatos:** NPC com `IsAttackable`, HP > 0 e visível; jogador remoto que não seja você e não esteja morto no cache.
- O alvo atual conta como visitado, então o Tab sempre avança para outro.
- Cada Tab vai para o mais próximo **ainda não visitado**. Quando todos foram visitados, o ciclo recomeça do mais próximo.
- Depois de **3 s** sem Tab, os visitados são zerados e o próximo Tab volta ao mais próximo.
- Sem candidatos, aparece **"Nenhum alvo próximo"** no feedback de combate.
- Jogador selecionado pelo Tab abre o `WBP_SelectedPlayerInfo`, mas **não** abre o menu de contexto.

**Ajustes no Details do PlayerController (componente `PlayerSelectionComponent` → categoria `PlayerSelection|TabTarget`):**

| Propriedade | Padrão | Uso |
|-------------|--------|-----|
| `TabTargetRange` | 2500 | Alcance em unidades (2500 = 25 m) |
| `bTabTargetIncludeNpcs` | ligado | NPCs atacáveis no ciclo |
| `bTabTargetIncludePlayers` | ligado | Jogadores remotos no ciclo |
| `TabTargetResetSeconds` | 3.0 | Tempo sem Tab até recomeçar do mais próximo |

> **Não** adicione lógica no Event Graph. O seletor de nº de alvos na tela de Opções, quando for feito, só precisa chamar `SetTabTargetMaxCount` pelo C++ do `UmbraOptionsWidget`.

---

## 6) QA (PIE)

PIE com **Number of Players ≥ 2**, os dois clients perto de alguns NPCs.

| Cenário | Esperado |
|---------|----------|
| **V** | Alterna o modo de câmera (log `Modo de Câmera: COMBAT/DEFAULT`) |
| **Tab** sem alvo selecionado | Seleciona o alvo mais próximo (NPC ou o outro player) |
| **Tab** repetido | Passa por todos os alvos do mais próximo ao mais distante, sem repetir, e depois recomeça |
| **Tab** depois de 3 s parado | Volta ao mais próximo (pulando o atual) |
| **Tab** no outro player | Abre o painel do jogador, sem menu de contexto; skill de alvo vai nele |
| **Tab** num NPC e depois skill/ataque | Mira o NPC escolhido |
| NPC morto ou vendor/quest | Não entra no ciclo |
| Ninguém a 25 m | Mensagem "Nenhum alvo próximo" |
| `SetTabTargetMaxCount(5)` com 8 NPCs perto | O ciclo passa só pelos 5 mais próximos |
| Digitar no chat e apertar Tab | Não troca de alvo (foco no campo de texto) |

Logs esperados (Verbose): `"[UmbraPlayerSelection] Tab target -> <actor> (NPC|Player, <dist> u, <n> candidatos, limite=<N>)"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Log `IA_TabTarget ausente` | Asset não criado ou com outro nome/pasta | Criar `/Game/Input/Actions/IA_TabTarget` (§1.1) |
| Tab ainda troca a câmera | `IMC_Default` sem o ajuste | Refazer §1.2 e salvar |
| Tab não faz nada e não mostra mensagem | `Tab` não mapeado para `IA_TabTarget` | Conferir o `IMC_Default` (§1.2) |
| "Nenhum alvo próximo" com NPC perto | NPC não atacável (vendor/quest) ou fora de 25 m | Conferir `is_attackable` no Manager ou aumentar `TabTargetRange` |
| Player remoto nunca entra | `bTabTargetIncludePlayers` desligado no BP | Ligar no componente do PlayerController |

## 8) Referência de código

- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraPlayerSelectionComponent.cpp` (`CycleTabTarget`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/UmbraEternumUECharacter.cpp` (`TabTargetPressed`, bind em `SetupPlayerInputComponent`)
- Cliente: `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraClientSettings.h` (`TabTargetMaxCount`)

**Fim do guia.**
