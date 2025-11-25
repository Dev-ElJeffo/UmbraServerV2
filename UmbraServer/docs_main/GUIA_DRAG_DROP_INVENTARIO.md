# 🎯 GUIA COMPLETO: DRAG & DROP DO INVENTÁRIO

## 📋 **O QUE VAMOS CRIAR**

Um sistema completo de arrastar e soltar itens:
- 🖱️ Clicar e segurar para pegar um item
- 🎨 Visual feedback (item sendo arrastado)
- 📍 Drop em outro slot = trocar itens
- ❌ Drop fora do inventário = cancelar
- 🔄 Chamada à API para mover item no servidor

---

## 🏗️ **ARQUITETURA**

```
1. OnMouseButtonDown (WBP_InventorySlot)
   └─> Detecta clique no item

2. OnDragDetected
   └─> Cria DragDropOperation
       └─> Cria widget visual (WBP_DraggedItem)

3. OnDrop (WBP_InventorySlot destino)
   └─> Valida drop
       └─> Chama API MoveItem
           └─> Atualiza inventário
```

---

## 📝 **PASSO 1: CRIAR WIDGET VISUAL DO DRAG**

### **1.1 - Criar WBP_DraggedItem:**

1. **Content Browser** → Clique com botão direito
2. **User Interface** → **Widget Blueprint**
3. Nome: `WBP_DraggedItem`
4. Abra o widget

---

### **1.2 - Designer do WBP_DraggedItem:**

Estrutura simples:

```
Canvas Panel
└── Border (Semi-transparent background)
    └── Image (Image_DraggedIcon)
```

**Configurações:**
- **Border:**
  - **Brush Color:** `(R=1, G=1, B=1, A=0.7)` (semi-transparente)
  - **Padding:** `5, 5, 5, 5`
  - **Size:** `64x64` (mesmo tamanho do slot)

- **Image_DraggedIcon:**
  - **Size:** `64x64`
  - **Is Variable:** ✅ True

---

### **1.3 - Criar Função: SetDragIcon**

No **Graph** de `WBP_DraggedItem`:

**Function:** `SetDragIcon`

**Inputs:**
- `IconTexture` (type: `Texture 2D`)

**Lógica:**

```
[SetDragIcon]
    │
    └─> [Set Brush from Texture]
            ├─ Target: Image_DraggedIcon
            └─ Texture: IconTexture
```

---

## 🔧 **PASSO 2: CRIAR CLASSE DE DRAG DROP OPERATION**

### **2.1 - Criar Blueprint Drag Drop Operation:**

1. **Content Browser** → Clique com botão direito
2. **Blueprint Class** → Pesquise: `DragDropOperation`
3. Nome: `BP_InventoryDragDropOperation`
4. Abra

---

### **2.2 - Adicionar Variáveis:**

No `BP_InventoryDragDropOperation`:

1. **DraggedSlotData** (type: `UmbraInventorySlot`)
   - **Instance Editable:** ✅ True
   - **Expose on Spawn:** ✅ True

2. **SourceSlotIndex** (type: `Integer`)
   - **Instance Editable:** ✅ True
   - **Expose on Spawn:** ✅ True

3. **DraggedItemIcon** (type: `Texture 2D`)
   - **Instance Editable:** ✅ True
   - **Expose on Spawn:** ✅ True

---

## 🎮 **PASSO 3: IMPLEMENTAR DRAG NO WBP_InventorySlot**

### **3.1 - Override: OnMouseButtonDown**

Na aba **Graph** de `WBP_InventorySlot`:

1. **Override** → **On Mouse Button Down**

**Lógica:**

```
[OnMouseButtonDown]
    │
    ├─> [Branch] (Condition: InventoryID > 0 AND Mouse Button == Left)
    │       │
    │       └─ TRUE:
    │           │
    │           ├─> [Detect Drag if Pressed]
    │           │       ├─ Pointer Event: (from OnMouseButtonDown)
    │           │       ├─ Drag Key: Left Mouse Button
    │           │       └─ Output: Drag Detected Event
    │           │
    │           └─> [Return] (Handled = true)
    │
    └─> FALSE:
        └─> [Return] (Handled = false)
```

---

### **3.2 - Override: OnDragDetected**

```
[OnDragDetected]
    │
    ├─> [Create Widget] (Class: WBP_DraggedItem)
    │       └─> DragVisual
    │
    ├─> [DragVisual] → [SetDragIcon]
    │       └─ IconTexture: [Get SlotData] → [Break] → ItemTemplate → ItemIcon
    │
    ├─> [Create Drag Drop Operation] (Class: BP_InventoryDragDropOperation)
    │       ├─ Default Drag Visual: DragVisual
    │       ├─ Pivot: (0.5, 0.5)
    │       ├─ Offset: (0, 0)
    │       │
    │       ├─ DraggedSlotData: SlotData
    │       ├─ SourceSlotIndex: SlotData.SlotIndex
    │       └─ DraggedItemIcon: ItemTemplate.ItemIcon
    │       │
    │       └─> DragDropOperation
    │
    ├─> [Print String] (DEBUG) "🔵 Drag iniciado - Slot: {SourceSlotIndex}"
    │
    └─> [Return]
            ├─ Operation: DragDropOperation
            └─ Widget: DragVisual
```

---

### **3.3 - Override: OnDragEnter** (Visual Feedback)

**Purpose:** Destacar o slot quando o item arrastado passa sobre ele.

```
[OnDragEnter]
    │
    ├─> [Cast to BP_InventoryDragDropOperation] ← Operation
    │       │
    │       └─ Success:
    │           │
    │           └─> [Set Brush Color] (Self)
    │                   └─ Color: (R=0.5, G=0.8, B=1, A=1) (azul claro)
    │
    └─> [End]
```

---

### **3.4 - Override: OnDragLeave**

```
[OnDragLeave]
    │
    └─> [Set Brush Color] (Self)
            └─ Color: (R=1, G=1, B=1, A=1) (branco - cor normal)
```

---

### **3.5 - Override: OnDrop** ⭐ **MAIS IMPORTANTE**

```
[OnDrop]
    │
    ├─> [Cast to BP_InventoryDragDropOperation] ← Operation
    │       │
    │       └─ Success:
    │           │
    │           ├─> [Get DraggedSlotData] from Operation → SourceSlot
    │           ├─> [Get SourceSlotIndex] from Operation → SourceIndex
    │           ├─> [Get SlotData] (self) → TargetSlot
    │           ├─> [SlotData.SlotIndex] → TargetIndex
    │           │
    │           ├─> [Print String] (DEBUG)
    │           │     "🟢 Drop: Slot {SourceIndex} → Slot {TargetIndex}"
    │           │
    │           ├─> [Branch] (Condition: SourceIndex != TargetIndex)
    │           │       │
    │           │       └─ TRUE:
    │           │           │
    │           │           ├─> [Get Game Instance] → Cast to UmbraGameInstance
    │           │           │       │
    │           │           │       └─> [MoveItem]
    │           │           │               ├─ SourceSlotIndex: SourceIndex
    │           │           │               └─ TargetSlotIndex: TargetIndex
    │           │           │
    │           │           └─> [Set Brush Color] (Self) → Cor normal
    │           │
    │           └─> FALSE: (mesmo slot, cancelar)
    │               └─> [Print String] "⚠️ Drop cancelado (mesmo slot)"
    │
    └─> [Return] (Handled = true)
```

---

## 🔌 **PASSO 4: IMPLEMENTAR MoveItem NO C++**

Já temos a função `MoveItem` no `UmbraGameInstance.cpp`, mas vamos revisar:

### **4.1 - Verificar UmbraGameInstance.h:**

```cpp
// No arquivo: UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h

UFUNCTION(BlueprintCallable, Category = "Umbra|Inventory")
void MoveItem(int32 SourceSlotIndex, int32 TargetSlotIndex);
```

---

### **4.2 - Implementação UmbraGameInstance.cpp:**

```cpp
void UUmbraGameInstance::MoveItem(int32 SourceSlotIndex, int32 TargetSlotIndex)
{
    UE_LOG(LogTemp, Log, TEXT("[MoveItem] Movendo item: Slot %d -> Slot %d"), 
           SourceSlotIndex, TargetSlotIndex);

    if (!IsPlayerLoggedIn())
    {
        UE_LOG(LogTemp, Error, TEXT("[MoveItem] Jogador não está logado!"));
        return;
    }

    // Criar requisição VaRest
    UVaRestRequestJSON* Request = UVaRestRequestJSON::ConstructRequestExt(
        this, 
        ERequestVerb::POST, 
        ERequestContentType::json
    );

    if (!Request)
    {
        UE_LOG(LogTemp, Error, TEXT("[MoveItem] Falha ao criar VaRest request"));
        return;
    }

    // URL da API
    FString URL = FString::Printf(TEXT("%s/move_item.php"), *ApiBaseURL);
    Request->SetURL(URL);

    // Body JSON
    TSharedPtr<FJsonObject> RequestBody = MakeShareable(new FJsonObject);
    RequestBody->SetStringField(TEXT("token"), CurrentAuthToken);
    RequestBody->SetNumberField(TEXT("source_slot_index"), SourceSlotIndex);
    RequestBody->SetNumberField(TEXT("target_slot_index"), TargetSlotIndex);

    Request->SetRequestObject(RequestBody);

    // Callbacks
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnMoveItemResponse);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnMoveItemFailed);

    // Executar
    Request->ProcessRequest();
}

void UUmbraGameInstance::OnMoveItemResponse(UVaRestRequestJSON* Request)
{
    UE_LOG(LogTemp, Log, TEXT("[OnMoveItemResponse] Resposta recebida"));

    TSharedPtr<FJsonObject> ResponseObj = Request->GetResponseObject();
    if (!ResponseObj.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[OnMoveItemResponse] Resposta JSON inválida"));
        return;
    }

    bool bSuccess = ResponseObj->GetBoolField(TEXT("success"));
    FString Message = ResponseObj->GetStringField(TEXT("message"));

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("[OnMoveItemResponse] ✅ Item movido: %s"), *Message);
        
        // Recarregar inventário
        LoadInventory();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[OnMoveItemResponse] ❌ Erro: %s"), *Message);
    }
}

void UUmbraGameInstance::OnMoveItemFailed(UVaRestRequestJSON* Request)
{
    UE_LOG(LogTemp, Error, TEXT("[OnMoveItemFailed] Falha na requisição HTTP"));
}
```

---

## 🌐 **PASSO 5: VERIFICAR API PHP (move_item.php)**

O endpoint já deve existir em `www/umbra_api/inventory/move_item.php`:

```php
<?php
require_once '../config/database.php';
require_once '../helpers/jwt_helper.php';

header('Content-Type: application/json');

// Validar JWT
$data = json_decode(file_get_contents('php://input'), true);
$user = validateJWTRequest($data);

if (!$user) {
    http_response_code(401);
    echo json_encode(['success' => false, 'message' => 'Token inválido']);
    exit;
}

$player_id = $user['player_id'];
$source_slot = $data['source_slot_index'] ?? null;
$target_slot = $data['target_slot_index'] ?? null;

if ($source_slot === null || $target_slot === null) {
    http_response_code(400);
    echo json_encode(['success' => false, 'message' => 'Slots não fornecidos']);
    exit;
}

// Conectar ao banco
$conn = getDBConnection();

// Trocar os itens
$query = "
    UPDATE player_inventory 
    SET slot_index = CASE 
        WHEN slot_index = ? THEN ?
        WHEN slot_index = ? THEN ?
    END
    WHERE player_id = ? AND slot_index IN (?, ?)
";

$stmt = $conn->prepare($query);
$stmt->bind_param(
    'iiiiiii', 
    $source_slot, $target_slot,
    $target_slot, $source_slot,
    $player_id, $source_slot, $target_slot
);

if ($stmt->execute()) {
    echo json_encode([
        'success' => true, 
        'message' => 'Item movido com sucesso',
        'source_slot' => $source_slot,
        'target_slot' => $target_slot
    ]);
} else {
    http_response_code(500);
    echo json_encode(['success' => false, 'message' => 'Erro ao mover item']);
}

$stmt->close();
$conn->close();
?>
```

---

## 🎨 **PASSO 6: MELHORIAS VISUAIS (OPCIONAL)**

### **6.1 - Adicionar Animação no Drop:**

No `OnDrop` de `WBP_InventorySlot`, após chamar `MoveItem`:

```
[Play Animation] (DropAnimation)
    ├─ Animation: Flash (piscar)
    └─ Play Mode: Forward
```

Criar a animação `DropAnimation` na aba **Animations**:
- **Track:** Border → Brush Color
- **0.0s:** Color = White
- **0.1s:** Color = Green
- **0.2s:** Color = White

---

### **6.2 - Som de Drop:**

```
[Play Sound 2D]
    └─ Sound: DropItemSound
```

---

### **6.3 - Cancelar Drag com ESC:**

No `WBP_Inventory`, adicionar:

```
[Event OnKeyDown] (Key: Escape)
    │
    └─> [Cancel Drag Drop]
```

---

## 🧪 **PASSO 7: TESTAR**

### **Checklist:**

1. ✅ Compile `WBP_DraggedItem`
2. ✅ Compile `BP_InventoryDragDropOperation`
3. ✅ Compile `WBP_InventorySlot`
4. ✅ Compile C++ (`UmbraGameInstance`)
5. ✅ Abra o jogo
6. ✅ Abra o inventário
7. ✅ **Clique e segure** em um item
8. ✅ **Arraste** para outro slot
9. ✅ **Solte** o mouse

**Resultado esperado:**
- Item é arrastado visualmente
- Slot destino fica destacado
- Ao soltar, item troca de posição
- Inventário é recarregado
- API registra a mudança

---

## 🚨 **PROBLEMAS COMUNS**

### **Problema 1: Drag não inicia**

**Causa:** `OnMouseButtonDown` não retorna `Handled = true`

**Solução:**
```
[OnMouseButtonDown] → [Return]
    └─ Reply → Handled()
```

---

### **Problema 2: Widget arrastado não aparece**

**Causa:** `WBP_DraggedItem` não foi criado corretamente

**Solução:**
1. Verifique se `Create Widget` está antes de `Create Drag Drop Operation`
2. Confirme que `Default Drag Visual` está conectado

---

### **Problema 3: Drop não funciona**

**Causa:** `OnDrop` não está sendo chamado

**Solução:**
1. Verifique se o slot destino tem **Visibility = Visible**
2. Certifique-se que `OnDragEnter` está funcionando (slot fica azul)

---

### **Problema 4: API retorna erro 401**

**Causa:** Token JWT não está sendo enviado

**Solução:**
1. Verifique se `CurrentAuthToken` está preenchido
2. Adicione log em `MoveItem`:
   ```cpp
   UE_LOG(LogTemp, Log, TEXT("Token: %s"), *CurrentAuthToken);
   ```

---

## 📊 **FLUXO COMPLETO**

```
1. Usuário clica e segura no Item A (Slot 0)
   └─> OnMouseButtonDown (Slot 0)
       └─> Detect Drag if Pressed

2. Usuário move o mouse
   └─> OnDragDetected (Slot 0)
       ├─> Cria WBP_DraggedItem (visual)
       ├─> Cria BP_InventoryDragDropOperation
       └─> Cursor agora arrasta o item

3. Usuário passa sobre Slot 5
   └─> OnDragEnter (Slot 5)
       └─> Slot 5 fica azul (feedback)

4. Usuário solta o mouse no Slot 5
   └─> OnDrop (Slot 5)
       ├─> Cast para BP_InventoryDragDropOperation
       ├─> Obtém SourceSlotIndex = 0
       ├─> Obtém TargetSlotIndex = 5
       ├─> Chama UmbraGameInstance->MoveItem(0, 5)
       │   └─> HTTP POST para move_item.php
       │       ├─> UPDATE player_inventory
       │       └─> Response: success = true
       ├─> LoadInventory() (recarrega)
       └─> OnInventoryLoaded event
           └─> UI atualiza (Item A agora está no Slot 5)
```

---

## 📝 **RESUMO**

✅ **Criado:** `WBP_DraggedItem` (widget visual do drag)
✅ **Criado:** `BP_InventoryDragDropOperation` (classe de drag drop)
✅ **Implementado:** `OnMouseButtonDown`, `OnDragDetected`, `OnDragEnter`, `OnDragLeave`, `OnDrop`
✅ **API:** `MoveItem()` no C++ + `move_item.php`
✅ **Feedback:** Visual (cor azul) e logs de debug

---

## 🎯 **FUNCIONALIDADES EXTRAS (FUTURO)**

- [ ] **Drag para equipar:** Arrastar item para slot de equipamento
- [ ] **Drag para deletar:** Arrastar item para fora do inventário
- [ ] **Stack merge:** Arrastar item stackable para outro do mesmo tipo
- [ ] **Swap animation:** Animação suave ao trocar itens
- [ ] **Confirmação:** Popup antes de mover itens raros

---

**Boa implementação do Drag & Drop! 🎮**

