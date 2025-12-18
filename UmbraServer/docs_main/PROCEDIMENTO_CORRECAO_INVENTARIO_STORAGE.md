# Procedimento de Correção – Inventário, Storage e Visual (UMG)

Objetivo: eliminar spam de logs, evitar auto-equipar involuntário, impedir duplicações por reprocessamento de slots e garantir que slots vazios não gerem erro/ruído visual.

## 1) Blueprint `WBP_InventorySlot` – `UpdateSlotVisual` (passo a passo explícito)
### Como ligar os nós
1. Depois do `Break UmbraInventorySlot`, crie um `Branch`. Ligue o exec de entrada no `Branch`. Ligue `InventoryID` (saída do Break) na condição do Branch.
2. Ramo **False** (InventoryID == 0):
   - Set `bIsEmpty = true`.
   - `Image_ItemIcon -> SetBrushFromTexture` (Texture = None, bMatchSize=false) → depois `SetVisibility = Collapsed`.
   - `Text_Quantity -> SetVisibility = Collapsed`.
   - `ProgressBar_Durability -> SetVisibility = Collapsed`.
   - `Border_SlotBackground -> SetVisibility = Collapsed` (ou Hidden).
   - Retorne (não prossegue mais exec). **Não** deixe nenhum `PrintString` aqui.
3. Ramo **True** (InventoryID > 0):
   - Set `bIsEmpty = false`.
   - Ícone:
     - `IsValid(ItemIcon)`? Se true: `Image_ItemIcon -> SetBrushFromTexture(ItemIcon)` e `SetVisibility = Visible`; se false: `SetVisibility = Collapsed`.
   - Quantidade:
     - Branch `Quantity > 1`: se true, `Text_Quantity = Visible` e `SetText(Quantity)`; se false, `Text_Quantity = Collapsed`.
   - Durabilidade:
     - Branch `EquipmentSlot != None AND Durability > 0`: se true, `ProgressBar_Durability = Visible` e `SetPercent = Durability/100`; se false, `ProgressBar_Durability = Collapsed`.
   - `Border_SlotBackground`: se quiser mostrar fundo quando há item, `SetVisibility = Visible`; caso contrário, manter como está.
4. Logs:
   - Remova/desative todos os `PrintString` no `UpdateSlotVisual` (INICIO, Inventory ID = 0, ItemIconTemplate, etc.).
5. Não use `ClearSlot` aqui:
   - `UpdateSlotVisual` só lê `SlotData` e atualiza a UI.
6. Opcional (anti-churn):
   - Se `bIsEmpty` já era true e `InventoryID == 0`, pode retornar logo após o Branch para não redesenhar.

## 2) Blueprint `WBP_Storage` – `UpdateAllSlotsVisual` (passo a passo explícito)
1. Tenha um array/lista dos 100 widgets de slot de storage.
2. Faça um `ForLoop` de 0 a 99:
   - `Get` o slot widget no índice do loop; se inválido, continue.
   - Chame `UpdateSlotVisual` **uma vez** nesse widget.
3. Não chamar `ClearSlot` antes de `UpdateSlotVisual`.
4. Remover prints por slot (deixe no máximo um log geral em Verbose, se precisar).

## 3) Servidor (PHP) – status atual
Já aplicado:
- `move_item.php` e `move_from_storage.php`:
  - Sempre setam `is_equipped = 0` em qualquer movimento (swap ou slot vazio).
  - Transferem propriedade para o `player_id` atual ao tirar do storage.
  - Usam slot temporário `-1` nos swaps para evitar violação de constraint.

## 4) C++ – pontos a observar
- `UUmbraInventorySlotWidget::ClearSlot` já está com log em `Verbose`.
- `UUmbraStorageWidget`: garantir que cada operação faça apenas **um** reload de inventário e **um** de storage (no callback de sucesso). Evitar reload duplo via `OnItemMoved` se já houver reload no callback.

## 5) Testes recomendados
1. Abrir inventário + storage:
   - Sem spam “Inventory ID = 0”.
   - Slots vazios colapsados, sem prints.
2. Mover item storage → inventário:
   - Slot vazio: item aparece, não equipa.
   - Swap: sem duplicação, sem auto-equip; item de destino vai para o storage; item vindo do storage vem sem `is_equipped`.
   - Propriedade: item passa a pertencer ao player atual.
3. Mover item inventário ↔ inventário:
   - Sem duplicação; sem auto-equip; sem spam.
4. Visual:
   - Quantidade só aparece se `Quantity > 1`.
   - Barra de durabilidade só aparece para equipáveis com `Durability > 0`.
   - Ícone some (Collapsed) se não houver item/ícone inválido.

## 6) Resumo rápido (para aplicar no Editor)
- `Branch InventoryID > 0` no início do `UpdateSlotVisual`.
- `False`: colapsar tudo, `bIsEmpty = true`, return, sem logs.
- `True`: `bIsEmpty = false`; ícone se válido; quantidade se >1; durabilidade se equipável e >0; sem logs.
- `UpdateAllSlotsVisual (storage)`: loop 0–99, um `UpdateSlotVisual` por slot, sem clears e sem prints por slot.

