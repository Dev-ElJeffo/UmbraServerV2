## Correção WBP_Storage — evitar limpar slots após o reload

### Sintoma
- Após `OnStorageLoaded`, o `WBP_Storage` chama `GetAllStorageSlots` e executa um loop que limpa (`ClearSlot`) praticamente todos os widgets (52–149), apagando a UI logo depois do carregamento.
- Logs mostram `SetSlotData` em poucos slots e, na sequência, dezenas de `ClearSlot`.

### Objetivo
- Atualizar slots somente com base nos dados retornados.
- Não limpar todos os widgets depois de carregar; apenas limpar slots sem item.
- Evitar chamadas repetitivas a `GetAllStorageSlots` (sem tick/polling).

### Passo a passo no Blueprint (`WBP_Storage`)
1) **Entrada de dados**
   - Garanta que o evento que recebe `OnStorageLoaded` (delegate do `GameInstance`) seja o ponto de entrada único para atualizar a UI.
   - Não chame `GetAllStorageSlots` em tick ou em loops redundantes; use apenas quando o delegate disparar.

2) **Loop de atualização**
   - No evento `OnStorageLoaded`, faça um `ForEach` sobre o array retornado (ex.: `CurrentStorage` ou `GetAllStorageSlots`).
   - Para cada elemento (slotData), pegue `slotData.SlotIndex` (0–99).
   - Use `Get SlotWidgets` (Array de `WBP_InventorySlot`) e `Get (by index)` com `SlotIndex`.
   - Se `SlotWidgets`[SlotIndex] **é válido**:
     - Se `slotData.InventoryID > 0` **E** `slotData.Quantity > 0`:
       - Chame `SetSlotData(slotData)`.
       - Em seguida, chame `UpdateSlotVisual`.
     - Caso contrário (slot vazio):
       - Chame `ClearSlot()`.
       - Em seguida, chame `UpdateSlotVisual`.

3) **Remover/evitar clear em massa**
   - Remova qualquer loop separado que faça `ClearSlot` em todos os 100 widgets antes ou depois de aplicar dados.
   - A limpeza deve ser somente por slot vazio, conforme acima.

4) **Garantir tamanho do array**
   - Certifique-se de que o array `SlotWidgets` tenha 100 entradas (0–99) e que o índice recebido é 0–99 (já normalizado pelo C++).

5) **Verificação**
   - Após aplicar, teste: mover item storage→inventário (slot livre) e storage→storage. A UI deve refletir sem sumir. Os logs não devem mostrar `ClearSlot` em cascata após o carregamento.

### Observação
- O servidor já deduplica e envia 12 itens; o problema restante é apenas no fluxo do Blueprint que limpa todos os slots. Evitar o clear global resolve o desaparecimento visual.

