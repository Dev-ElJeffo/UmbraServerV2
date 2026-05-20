# Guia: Skillbar — quantidade, drag-out, hotkeys e cooldown

Complemento ao [GUIA_USO_CONSUMIVEIS_SKILLBAR.md](GUIA_USO_CONSUMIVEIS_SKILLBAR.md).

## Visão geral

| Recurso | Onde |
|---------|------|
| Quantidade do consumível no slot | `UUmbraSkillSlotWidget::Quantity_Text` |
| Botão para definir atalho | `Keybind_Button` → `WBP_HotkeyCapture` |
| Texto do atalho | `Keybind_Text` (já existia) |
| Drag skill/item para fora | `UUmbraSkillbarDragDropOperation` + `NativeOnDragCancelled` |
| Uso por tecla | `FUmbraSkillbarInputProcessor` |
| Cooldown consumível (barra) | `UpdateAllCooldowns` em `UUmbraSkillBarWidget` |
| Cooldown consumível (inventário) | `Cooldown_*_Inv` em `UUmbraInventorySlotWidget` |

---

## 1. WBP_SkillSlot (reparent → `UUmbraSkillSlotWidget`)

Adicione estes filhos no **Canvas** do slot (nomes exatos para `BindWidgetOptional`):

| Nome | Tipo | Posição sugerida |
|------|------|------------------|
| `Quantity_Text` | TextBlock | Canto inferior direito, fonte pequena, cor branca |
| `Keybind_Button` | Button | Canto superior direito, ~16×16, fundo semi-transparente |
| `Keybind_Text` | TextBlock | Canto superior esquerdo (atalho visível) |
| `Cooldown_Overlay` | CanvasPanel | Cobre o ícone (já existente) |
| `Cooldown_Fill` | Image | Material `MI_CooldownRadial` com parâmetro `Progress` |
| `Cooldown_Text` | TextBlock | Centro do overlay |

**Graph do BP:** vazio — toda a lógica está no C++.

### Comportamento

- **Quantidade:** soma de todas as pilhas do `item_template_id` no inventário (não equipadas).
- **Keybind_Button:** abre captura de teclas.
- **LMB drag:** skill ou consumível na barra (remove ao soltar fora).
- **RMB:** usar skill ou consumível.

---

## 2. WBP_HotkeyCapture (novo)

1. Criar widget `WBP_HotkeyCapture`.
2. **Reparent Class** → `UmbraHotkeyCaptureWidget`.
3. Filhos obrigatórios:
   - `Status_Text` (TextBlock): "Pressione 1 ou 2 teclas..."
   - `Cancel_Button` (Button): "Cancelar"
4. Layout: painel central semi-opaco (ZOrder alto).

### Configurar no GameInstance

No **BP_GameInstance** (ou Default do `UUmbraGameInstance`):

- **Hotkey Capture Widget Class** = `WBP_HotkeyCapture`

---

## 3. Formato do atalho (MySQL + cliente)

Coluna: `player_skillbar.keybind` VARCHAR(20)

| Formato | Exemplo |
|---------|---------|
| Uma tecla | `A`, `One`, `F1` |
| Duas teclas | `A+S`, `LeftShift+One` (ordenadas alfabeticamente no encode) |

Gravação só do atalho (sem alterar skill/item):

```json
POST /api/skills/set_skillbar.php
{
  "token": "...",
  "player_id": 1,
  "slot_index": 5,
  "keybind": "A+S",
  "keybind_only": true
}
```

---

## 4. WBP_InventorySlot — overlay de cooldown

Adicione no Canvas do slot (nomes exatos):

| Nome | Tipo |
|------|------|
| `Cooldown_Overlay_Inv` | CanvasPanel |
| `Cooldown_Fill_Inv` | Image (`MI_CooldownRadial`) |
| `Cooldown_Text_Inv` | TextBlock |

Mesmo material radial da skillbar (`Progress` 0→1).

---

## 5. API

### `use_item.php` — campo extra

```json
"cooldown_total_ms": 5000
```

Usado pelo cliente para o preenchimento radial do cooldown.

---

## 6. Teste de aceite

1. Poção na barra → `Quantity_Text` mostra total (ex.: 18).
2. Arrastar poção para fora da barra → slot vazio após soltar; relog confirma.
3. Clicar engrenagem → pressionar `A` depois `S` → `Keybind_Text` mostra `A+S`; relog mantém.
4. Pressionar `A+S` no jogo → mesmo efeito que RMB (cura + cooldown).
5. Após usar → overlay radial ~5s na barra e no inventário.
6. Com foco no chat, teclas não disparam atalhos da barra.

---

## 7. Arquivos C++ novos/alterados

**Novos**

- `UI/UmbraSkillbarDragDropOperation.h`
- `Core/UmbraSkillbarInputProcessor.h/.cpp`
- `UI/UmbraHotkeyCaptureWidget.h/.cpp`

**Alterados**

- `Core/UmbraGameInstance.h/.cpp`
- `UI/UmbraSkillSlotWidget.h/.cpp`
- `UI/UmbraSkillBarWidget.h/.cpp`
- `UI/UmbraInventorySlotWidget.h/.cpp`
- `www/umbra_api/api/inventory/use_item.php`
- `www/umbra_api/api/skills/set_skillbar.php`
