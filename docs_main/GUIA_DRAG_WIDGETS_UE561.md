# Guia: janelas arrastáveis (`UmbraDraggableWindowWidget`) — UE 5.6.1

Documentação **somente** do sistema de **arrastar janela** por uma barra (título). Não inclui alterações de `PlayerController` nem de modo de input ao fechar painéis.

## Classe C++

- **`UUmbraDraggableWindowWidget`** (`UI/UmbraDraggableWindowWidget.h/.cpp`) — base abstrata para qualquer `WBP` que deva ser arrastável.

## O que configurar no Widget Blueprint

1. **Class Settings** → **Parent class** → `UmbraDraggableWindowWidget`.
2. **Class Defaults** (ou variáveis):
   - **`bEnableWindowDrag`**: `true` para permitir arrastar.
   - **`DragMouseButton`**: em geral **Left Mouse Button**.
   - **`DragAreaWidget`**: referência ao painel da “barra” (ex.: `Border_TitleBar`). Se ficar **None**, o **drag da janela não inicia** (evita `CaptureMouse` em todo o painel — quebrava cursor com inventário + loja).
3. No **Event Construct** (ou C++), chame **`Set Drag Area Widget`** apontando para o border da barra.

## Inventário

- **`UUmbraInventoryWidget`** já herda `UmbraDraggableWindowWidget`. O armazém (`UUmbraStorageWidget`) herda o inventário — mesmo comportamento de janela.
- O inventário criado/mostrado por **`EnsureInventoryVisibleForPersonalShop`** (vendedor ou comprador) fica com **`bEnableWindowDrag = false`**: só arraste de **itens** (drag & drop dos slots), não da janela inteira.

## Comportamento técnico (resumo)

- **Mouse down** na área de drag → `CaptureMouse` no Slate.
- **Mouse up** → `ReleaseMouseCapture`.
- **`CancelWindowDrag()`** → zera estado interno e chama **`ReleaseAllPointerCapture()`** no `FSlateApplication` (útil ao fechar o widget no meio de um arraste).

## Blueprint: fechar com segurança

- **`UUmbraPlayerInfoBPFL::SafeRemoveWidgetFromParent`** (categoria Umbra|PlayerInfo):
  - Se o widget for `UmbraDraggableWindowWidget`, chama **`CancelWindowDrag()`**.
  - Chama **`ReleaseAllPointerCapture()`**.
  - Ajusta **`FInputModeGameAndUI`** no `OwningPlayer` (comportamento **legado** do projeto; não é o mesmo que “só gameplay”).
  - Por fim **`RemoveFromParent`**.

Use quando a referência ao widget puder ser **None**, para evitar acessos inválidos.

## Arquivos de referência

| Arquivo | Função |
|---------|--------|
| `UmbraDraggableWindowWidget.h/.cpp` | Lógica de drag |
| `UmbraInventoryWidget.h` | Herda a base arrastável |
| `UmbraPlayerInfoBPFL.cpp` | `SafeRemoveWidgetFromParent` + captura |

---

**Motor:** Unreal Engine **5.6.1** (alinhado a `AGENTS.md`).
