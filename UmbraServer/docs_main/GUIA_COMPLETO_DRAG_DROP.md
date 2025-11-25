# 🎮 **GUIA COMPLETO: DRAG & DROP DE ITENS**

## 📋 **VISÃO GERAL:**

Vamos implementar drag & drop para **arrastar itens entre slots** do inventário!

**Fluxo:**
1. Clicar e segurar em um item
2. Arrastar o item (visual do item segue o mouse)
3. Soltar em outro slot (move o item)

---

## 🏗️ **ARQUITETURA:**

```
WBP_InventorySlot (origem)
    │
    ├─ OnMouseButtonDown → Detecta clique
    ├─ OnDragDetected → Inicia drag
    │   └─ Cria BP_ItemDragDropOperation
    │       └─ Cria WBP_DraggedItem (visual)
    │
WBP_InventorySlot (destino)
    │
    ├─ OnDragEnter → Mouse entra no slot (highlight)
    ├─ OnDragLeave → Mouse sai do slot
    └─ OnDrop → Solta o item (troca slots)
        └─ Chama API MoveItem
```

---

## 🎨 **PASSO 1: CRIAR WBP_DraggedItem**

Widget visual do item sendo arrastado.

### **1.1 Criar o Widget:**

1. **Content Browser** → `Content/Widgets/UI/Inventory/`
2. Clique direito → **User Interface** → **Widget Blueprint**
3. Nome: `WBP_DraggedItem`

### **1.2 Designer do WBP_DraggedItem:**

**Hierarquia:**
```
Canvas Panel
└─ Overlay
    ├─ Image_Icon
    └─ Text_Quantity (opcional)
```

**Configuração:**

1. **Overlay:**
   - Size: `64x64` (mesmo tamanho do slot)

2. **Image_Icon:**
   - Variable: **Marque "Is Variable"**
   - Size to Content: TRUE
   - Render Opacity: `0.7` (semi-transparente)

3. **Text_Quantity (opcional):**
   - Anchors: Bottom Right
   - Font Size: 12
   - Color: Branco
   - Outline: Preto

### **1.3 Event Graph do WBP_DraggedItem:**

Adicione uma função para configurar o visual:

**Função: SetDraggedItemVisual**

- Input: `ItemIcon` (Texture 2D - Object Reference)
- Input: `Quantity` (Integer)

```blueprint
[Function Entry] SetDraggedItemVisual
    │ ItemIcon (Texture2D)
    │ Quantity (int)
    │
    ▼
[Set Brush from Texture]
    │ Target: Image_Icon
    │ Texture: ItemIcon
    │
    ▼
[To Text (int)]
    │ In Int: Quantity
    │
    ▼
[Set Text]
    │ Target: Text_Quantity
    │ In Text: (resultado do To Text)
```

---

## 🎯 **PASSO 2: CRIAR BP_ItemDragDropOperation**

Blueprint que armazena os dados do drag.

### **2.1 Criar o Blueprint:**

1. **Content Browser** → `Content/Blueprints/`
2. Clique direito → **Blueprint Class**
3. Parent Class: **DragDropOperation**
4. Nome: `BP_ItemDragDropOperation`

### **2.2 Adicionar Variáveis:**

No **Class Defaults**, adicione:

| Nome | Tipo | Descrição |
|------|------|-----------|
| `OriginSlot` | WBP_InventorySlot (Object Reference) | Slot de origem |
| `SlotData` | UmbraInventorySlot (Struct) | Dados do item |
| `DraggedVisual` | WBP_DraggedItem (Object Reference) | Widget visual |

**Configure como:**
- Instance Editable: ✅
- Expose on Spawn: ✅

---

## 🖱️ **PASSO 3: IMPLEMENTAR NO WBP_InventorySlot**

### **3.1 Override OnMouseButtonDown:**

1. No `WBP_InventorySlot`, Event Graph
2. Clique direito → **DESMARQUE** "Context Sensitive"
3. Procure: `OnMouseButtonDown`

```blueprint
[Event On Mouse Button Down]
    │ My Geometry
    │ Mouse Event
    │
    ▼
[Branch] (Tem item neste slot?)
    │ Condition: SlotData.InventoryID > 0
    │
    ├─ True ──> [Detect Drag]
    │               │ Drag Key: Left Mouse Button
    │               │ Pointer Event: Mouse Event
    │               │
    │               └─> Return Value (Event Reply)
    │                       │
    │                       └─> [Return Node]
    │
    └─ False ─> [Unhandled]
                    │
                    └─> [Return Node]
```

---

### **3.2 Override OnDragDetected:**

```blueprint
[Event On Drag Detected]
    │ My Geometry
    │ Pointer Event
    │
    ▼
[Create Widget] (WBP_DraggedItem)
    │ Owning Player: Get Player Controller (0)
    │
    ▼
[Set Variable] (variável local: DragVisualWidget)
    │
    ▼
[Get SlotData] (variável do slot)
    │
    ▼
[Break UmbraInventorySlot]
    │ → ItemTemplate
    │
    ▼
[Break UmbraItemTemplate]
    │ → ItemIcon (se você tiver)
    │ → OU: Get Item Icon by ID
    │
    ▼
[SetDraggedItemVisual]
    │ Target: DragVisualWidget
    │ ItemIcon: (ícone do item)
    │ Quantity: SlotData.Quantity
    │
    ▼
[Construct Object from Class]
    │ Class: BP_ItemDragDropOperation
    │ Outer: self
    │
    ▼
[Set Origin Slot]
    │ Target: (BP_ItemDragDropOperation criado)
    │ Value: self (WBP_InventorySlot)
    │
    ▼
[Set Slot Data]
    │ Target: (BP_ItemDragDropOperation)
    │ Value: SlotData
    │
    ▼
[Set Dragged Visual]
    │ Target: (BP_ItemDragDropOperation)
    │ Value: DragVisualWidget
    │
    ▼
[Set Default Drag Visual]
    │ Target: (BP_ItemDragDropOperation)
    │ Value: DragVisualWidget
    │
    ▼
[Set Pivot]
    │ Target: (BP_ItemDragDropOperation)
    │ Value: Mouse Down (0.5, 0.5)
    │
    ▼
[Return Node]
    │ Return Value: (BP_ItemDragDropOperation)
```

---

### **3.3 Override OnDragEnter (visual feedback):**

```blueprint
[Event On Drag Enter]
    │ My Geometry
    │ Pointer Event
    │ Operation
    │
    ▼
[Cast to BP_ItemDragDropOperation]
    │ Object: Operation
    │
    ├─ Cast Succeeded ──> [Set Color and Opacity]
    │                         │ Target: Border_BG (do slot)
    │                         │ Color: (R=0, G=1, B=0, A=0.5) - Verde
    │
    └─ Cast Failed ──> [Nada]
```

---

### **3.4 Override OnDragLeave:**

```blueprint
[Event On Drag Leave]
    │ My Geometry
    │ Pointer Event
    │ Operation
    │
    ▼
[Set Color and Opacity]
    │ Target: Border_BG
    │ Color: (R=1, G=1, B=1, A=1) - Branco (normal)
```

---

### **3.5 Override OnDrop (A LÓGICA PRINCIPAL!):**

```blueprint
[Event On Drop]
    │ My Geometry
    │ Pointer Event
    │ Operation
    │
    ▼
[Cast to BP_ItemDragDropOperation]
    │ Object: Operation
    │
    ├─ Cast Succeeded ──> [Get Origin Slot]
    │                         │
    │                         ▼
    │                     [Branch] (Não é o mesmo slot?)
    │                         │ Condition: Origin Slot != self
    │                         │
    │                         ├─ True ──> [Get Slot Data] (do Operation)
    │                         │               │
    │                         │               ▼
    │                         │           [Get SlotData] (deste slot - destino)
    │                         │               │
    │                         │               ▼
    │                         │           [Branch] (Slot destino está vazio?)
    │                         │               │ Condition: InventoryID <= 0
    │                         │               │
    │                         │               ├─ True ──> [Move Item API]
    │                         │               │
    │                         │               └─ False ─> [Swap Items API]
    │                         │
    │                         └─ False ─> [Handled] (retorna TRUE)
    │
    └─ Cast Failed ──> [Handled] (retorna FALSE)
```

---

### **3.6 Chamar API Move Item:**

Crie uma função no `WBP_InventorySlot`: **MoveItemToSlot**

- Input: `TargetSlotIndex` (int)
- Input: `OriginSlotData` (UmbraInventorySlot)

```blueprint
[Function Entry] MoveItemToSlot
    │ TargetSlotIndex
    │ OriginSlotData
    │
    ▼
[Get Game Instance]
    │
    ▼
[Cast to UmbraGameInstance]
    │
    ▼
[MoveItem] (função C++ que já existe!)
    │ Target: UmbraGameInstance
    │ InventoryID: OriginSlotData.InventoryID
    │ NewSlotIndex: TargetSlotIndex
```

---

## 🔄 **PASSO 4: IMPLEMENTAR MOVEITEM NO C++**

Se você ainda não tem, adicione no `UmbraGameInstance`:

**No UmbraGameInstance.h:**

```cpp
UFUNCTION(BlueprintCallable, Category = "Inventory")
void MoveItem(int32 InventoryID, int32 NewSlotIndex);
```

**No UmbraGameInstance.cpp:**

```cpp
void UUmbraGameInstance::MoveItem(int32 InventoryID, int32 NewSlotIndex)
{
    if (!IsLoggedIn())
    {
        UE_LOG(LogTemp, Error, TEXT("MoveItem: Usuário não está logado!"));
        return;
    }

    // Criar request
    UVaRestRequestJSON* Request = UVaRestSubsystem::Get()->ConstructVaRestRequest(this);
    
    // Configurar URL
    FString URL = FString::Printf(TEXT("%s/inventory/move_item.php"), *APIBaseURL);
    Request->SetURL(URL);
    Request->SetVerb(ERequestVerb::POST);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Criar JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("token"), AuthToken);
    JsonObject->SetNumberField(TEXT("inventory_id"), InventoryID);
    JsonObject->SetNumberField(TEXT("new_slot_index"), NewSlotIndex);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    Request->SetContentAsString(JsonString);

    // Bind callbacks
    Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnMoveItemResponse);
    Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnMoveItemFailed);

    // Enviar
    Request->ExecuteProcessRequest();
}

void UUmbraGameInstance::OnMoveItemResponse(UVaRestRequestJSON* Request)
{
    TSharedPtr<FJsonObject> JsonObject = Request->GetResponseObject();
    bool bSuccess = JsonObject->GetBoolField(TEXT("success"));

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Item movido com sucesso!"));
        LoadInventory(); // Recarrega o inventário
    }
    else
    {
        FString Message = JsonObject->GetStringField(TEXT("message"));
        UE_LOG(LogTemp, Error, TEXT("Erro ao mover item: %s"), *Message);
    }
}

void UUmbraGameInstance::OnMoveItemFailed(UVaRestRequestJSON* Request)
{
    UE_LOG(LogTemp, Error, TEXT("Falha na requisição de move item"));
}
```

**Não esqueça de declarar os callbacks no .h!**

---

## 🧪 **TESTE COMPLETO:**

### **Checklist:**

1. [ ] Clicar e segurar em um item
2. [ ] Arrastar o item (visual segue o mouse)
3. [ ] Passar sobre outro slot (slot fica verde)
4. [ ] Soltar o item (item se move)
5. [ ] Inventário atualiza automaticamente
6. [ ] API é chamada corretamente
7. [ ] Banco de dados atualizado

---

## 🎨 **MELHORIAS VISUAIS:**

### **Adicionar som de drag:**

No `OnDragDetected`:
```blueprint
[Play Sound 2D]
    │ Sound: (som de pegar item)
```

No `OnDrop`:
```blueprint
[Play Sound 2D]
    │ Sound: (som de soltar item)
```

### **Adicionar partículas:**

No `OnDrop` (se sucesso):
```blueprint
[Spawn Emitter at Location]
    │ Emitter Template: (partículas de sucesso)
    │ Location: (posição do slot)
```

---

## 📋 **RESUMO DE ARQUIVOS:**

| Arquivo | Descrição |
|---------|-----------|
| `WBP_DraggedItem` | Widget visual do item arrastado |
| `BP_ItemDragDropOperation` | Blueprint com dados do drag |
| `WBP_InventorySlot` | Implementa OnDragDetected, OnDrop, etc. |
| `UmbraGameInstance.cpp` | Implementa MoveItem (API call) |

---

**Sistema de drag & drop completo! 🎯✨**

