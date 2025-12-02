# Correção: Adicionar Necklace em SetSlotType (WBP_EquipmentSlot)

## Problema
A função `SetSlotType` em `WBP_EquipmentSlot` não está tratando o caso `Necklace`, causando mapeamento incorreto para `Bracelet`.

## Solução

### 1. Abrir WBP_EquipmentSlot
- Abra o Blueprint `WBP_EquipmentSlot` no Editor do Unreal.

### 2. Localizar a função SetSlotType
- No painel de funções, encontre `SetSlotType`.
- Abra a função para edição.

### 3. Verificar o Switch on EUmbraEquipmentSlot
- Deve haver um nó `Switch on EUmbraEquipmentSlot` que recebe o parâmetro de entrada `EquipmentSlot`.
- Verifique se existe um caso para `Necklace`.

### 4. Adicionar o caso Necklace (se não existir)
- No `Switch on EUmbraEquipmentSlot`, clique com o botão direito e selecione **"Add Pin"** ou **"Add Case"**.
- Selecione `Necklace` da lista de opções.
- Conecte o pin `Necklace` a um nó `Set EquipmentSlot` (ou similar) que define a variável `EquipmentSlot` do widget como `Necklace`.

### 5. Verificar a ordem dos casos
A ordem dos casos no `Switch` deve ser:
1. None
2. Head
3. Chest
4. Legs
5. Feet
6. Hands
7. MainHand
8. OffHand
9. Ring
10. Amulet
11. **Necklace** ← Deve estar aqui, entre Amulet e Earring
12. Earring
13. Bracelet
14. Mount

### 6. Estrutura esperada do SetSlotType

```
[Function Entry: SetSlotType]
  └─ Input: EquipmentSlot (EUmbraEquipmentSlot)
       ↓
[Switch on EUmbraEquipmentSlot]
  ├─ None → [Set EquipmentSlot] → EquipmentSlot = None
  ├─ Head → [Set EquipmentSlot] → EquipmentSlot = Head
  ├─ Chest → [Set EquipmentSlot] → EquipmentSlot = Chest
  ├─ Legs → [Set EquipmentSlot] → EquipmentSlot = Legs
  ├─ Feet → [Set EquipmentSlot] → EquipmentSlot = Feet
  ├─ Hands → [Set EquipmentSlot] → EquipmentSlot = Hands
  ├─ MainHand → [Set EquipmentSlot] → EquipmentSlot = MainHand
  ├─ OffHand → [Set EquipmentSlot] → EquipmentSlot = OffHand
  ├─ Ring → [Set EquipmentSlot] → EquipmentSlot = Ring
  ├─ Amulet → [Set EquipmentSlot] → EquipmentSlot = Amulet
  ├─ Necklace → [Set EquipmentSlot] → EquipmentSlot = Necklace  ← ADICIONAR ESTE CASO
  ├─ Earring → [Set EquipmentSlot] → EquipmentSlot = Earring
  ├─ Bracelet → [Set EquipmentSlot] → EquipmentSlot = Bracelet
  └─ Mount → [Set EquipmentSlot] → EquipmentSlot = Mount
```

### 7. Verificar se a variável EquipmentSlot existe
- No painel de variáveis de `WBP_EquipmentSlot`, verifique se existe uma variável do tipo `EUmbraEquipmentSlot` chamada `EquipmentSlot`.
- Se não existir, crie-a:
  - Tipo: `EUmbraEquipmentSlot` (Enum)
  - Nome: `EquipmentSlot`
  - Visibilidade: `Private` ou `Public`

### 8. Compilar e testar
- Compile o Blueprint.
- Teste equipando um item do tipo `Necklace`.
- Verifique se o slot correto é atribuído.

## Nota Importante
Se o `Switch` não tiver o caso `Necklace`, o item será tratado como `None` ou cairá no `Default` do `Switch`, o que pode causar comportamento inesperado.

