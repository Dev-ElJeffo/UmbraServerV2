# Guia UE 5.6.1 — Flicker do `WBP_SelectedPlayerInfo` (duas barras de HP)

## Causa

Há **duas** criações do mesmo painel de alvo:

| Quem | Onde | Efeito |
|------|------|--------|
| C++ `UUmbraPlayerSelectionComponent::ShowTargetWidget` | `AddToViewport(100)` | Painel “oficial” (buffs + HP) |
| Blueprint **`WBP_PlayerHUD`** | `OnSelectedPlayerInfoUpdated` / `OnPlayerSelected` → `Create Widget` → **`Add To Viewport`** | Segunda cópia (muitas vezes **sem** buff icons) |

No hit (opcode 87) o C++ atualiza uma e o BP recria/atualiza a outra → flicker esquerda/direita.

O nameplate 3D (`WBP_PlayerNameplate`) **não** é esse bug.

---

## Correção no Editor (obrigatória)

1. Abra `/Game/Widgets/HUD/WBP_PlayerHUD`.
2. No **Event Graph**, ache binds a:
   - `On Selected Player Info Updated`
   - `On Player Selected`
3. **Apague** o fluxo que faz:
   - `Create Widget` (Class = `WBP_SelectedPlayerInfo`)
   - `Add To Viewport`
   - `Update Display` / `Remove From Parent` desse widget duplicado
4. Deixe a abertura do alvo **só** com o C++ (`PlayerSelection` no PlayerController).
5. Compile + Save o `WBP_PlayerHUD`.

Opcional em `WBP_SelectedPlayerInfo`: se o Graph chamar `Get Health Percent` → `Set Percent` em cima do C++ `UpdateDisplay`, pode remover — o C++ já preenche `Progress_HP` / `Progress_MP`.

---

## Defesa C++ (já aplicada)

- `ShowTargetWidget` / `RefreshTargetWidget` removem qualquer `UUmbraSelectedPlayerInfoWidget` no viewport que **não** seja `TargetWidgetInstance`.
- Log: `Removendo WBP_SelectedPlayerInfo órfão...`
- `UpdateDisplay` não força rebuild da buff bar a cada hit (só se a lista mudou).

---

## QA

1. Selecionar remoto → **uma** barra no topo da tela.
2. Hit → HP/MP atualizam **sem** segunda barra piscando.
3. Buffs aparecem numa só linha sob o painel.
4. Se o log de órfão ainda aparecer a cada hit → o `WBP_PlayerHUD` ainda tem `Create Widget` (passo 3).
