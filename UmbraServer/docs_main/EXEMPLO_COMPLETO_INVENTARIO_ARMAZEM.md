# 🎮 EXEMPLO COMPLETO: INVENTÁRIO + ARMAZÉM

## 🎯 **CENÁRIO:**

Um jogador entra no jogo, abre seu inventário, organiza itens, e depois visita um armazém para guardar itens.

---

## 📋 **PASSO A PASSO COMPLETO:**

### **1. COMPILAR C++ (1x apenas):**

```powershell
cd D:\UmbraServerV2\UmbraEternumUE
.\compile_drag_drop.bat
```

✅ Aguarde a compilação finalizar.

---

### **2. CONFIGURAR INVENTÁRIO:**

**A. Re-parent WBP_Inventory:**
1. Abra `WBP_Inventory` no UE5
2. File → Reparent Blueprint
3. Escolha: `UmbraInventoryWidget`
4. Compile & Save

**B. Re-parent WBP_InventorySlot:**
1. Abra `WBP_InventorySlot` no UE5
2. File → Reparent Blueprint
3. Escolha: `UmbraInventorySlotWidget`
4. Compile & Save

✅ **PRONTO!** Inventário agora é arrastável e tem drag & drop de itens!

---

### **3. CRIAR SISTEMA DE ARMAZÉM:**

**A. Criar Widget do Armazém:**

1. Content Browser → Right Click → Widget Blueprint
2. Nome: `WBP_Storage`
3. Parent Class: `UmbraStorageWidget`

**Designer (WBP_Storage):**

```
Canvas Panel
├── Border_Background (preto semi-transparente)
│   └── Vertical Box
│       ├── Horizontal Box (Header)
│       │   ├── Text_Title ("Armazém do Jogador")
│       │   ├── Spacer
│       │   ├── Text_Capacity ("0/100")
│       │   └── Button_Close ("X")
│       │
│       ├── Uniform Grid (10x10 = 100 slots)
│       │   └── [100x WBP_InventorySlot]
│       │
│       └── Horizontal Box (Footer)
│           ├── Button_DepositAll ("Depositar Tudo")
│           └── Button_WithdrawAll ("Retirar Tudo")
```

**Event Graph (WBP_Storage):**

```blueprint
[Event Construct]
    │
    ├─> [Load Storage] ◄─── FUNÇÃO C++
    │       │
    │       └─> [For Loop: 0 to 99]
    │               │
    │               ├─> [Create WBP_InventorySlot]
    │               │       │
    │               │       └─> [Add Child to Uniform Grid]
    │               │               │ Row: Index / 10
    │               │               │ Column: Index % 10
    │               │
    │               └─> [Add to Array: SlotWidgets]
    │
    └─> [Bind Events]

[Button_Close: OnClicked]
    │
    └─> [Remove From Parent]
        └─> [Get Owning Player]
            └─> [Set Input Mode Game Only]
                └─> [Set Show Mouse Cursor: False]

[Button_DepositAll: OnClicked]
    │
    └─> [Deposit All] ◄─── FUNÇÃO C++
            │ Return: ItemsDeposited (int32)
            │
            └─> [Branch: ItemsDeposited > 0]
                    │
                    ├─ TRUE ──> [Show Message]
                    │           │ Text: "Depositados {ItemsDeposited} itens"
                    │           │
                    │           └─> [Play Sound: Deposit]
                    │
                    └─ FALSE ─> [Show Message]
                                │ Text: "Nenhum item para depositar"

[Button_WithdrawAll: OnClicked]
    │
    └─> [Withdraw All] ◄─── FUNÇÃO C++
            │ Return: ItemsWithdrawn (int32)
            │
            └─> [Branch: ItemsWithdrawn > 0]
                    │
                    ├─ TRUE ──> [Show Message]
                    │           │ Text: "Retirados {ItemsWithdrawn} itens"
                    │           │
                    │           └─> [Play Sound: Withdraw]
                    │
                    └─ FALSE ─> [Show Message]
                                │ Text: "Nenhum item no armazém"

[Event On Storage Loaded]
    │ Used Slots (int32)
    │ Total Slots (int32)
    │
    └─> [Set Text (Text_Capacity)]
            │ InText: "{UsedSlots}/{TotalSlots}"
```

---

**B. Criar Actor do Armazém:**

1. Content Browser → Right Click → Blueprint Class
2. Parent: `UmbraStorageActor`
3. Nome: `BP_StorageChest`

**Components (BP_StorageChest):**

```
BP_StorageChest
├── MeshComponent (Static Mesh)
│   └── Mesh: SM_Chest (ou qualquer mesh)
└── InteractionBox (Box Collision)
    └── Extent: 100, 100, 100
```

**Details (BP_StorageChest):**

| Property | Value |
|----------|-------|
| Storage Widget Class | WBP_Storage |
| Storage ID | 1 |
| Storage Name | "Banco do Jogador" |
| Max Interaction Distance | 200.0 |
| Requires Key | false |

**Event Graph (BP_StorageChest):**

```blueprint
[Event On Storage Opened]
    │ Player Controller
    │
    └─> [Play Animation: ChestOpen]
        │
        └─> [Play Sound: ChestCreak]
            │
            └─> [Print String: "Armazém aberto!"]

[Event On Storage Closed]
    │
    └─> [Play Animation: ChestClose]
        │
        └─> [Play Sound: ChestClose]

[Event On Access Denied]
    │ Reason (String)
    │
    └─> [Show Message: Reason]
        │
        └─> [Play Sound: Error]
```

---

**C. Sistema de Interação:**

**No PlayerController ou Character:**

```blueprint
[Input Action: Interact (E)]
    │
    ├─> [Line Trace by Channel]
    │       │ Start: Camera Location
    │       │ End: Camera Location + (Forward * 200)
    │       │ Channel: Visibility
    │       │
    │       └─> [Break Hit Result]
    │               │
    │               └─> [Cast to UmbraStorageActor]
    │                       │
    │                       └─> [Branch: Success]
    │                               │
    │                               ├─ TRUE ──> [Open Storage] ◄─── FUNÇÃO C++
    │                               │               │ Player Controller: self
    │                               │               │
    │                               │               └─> Success!
    │                               │
    │                               └─ FALSE ─> [Print String: "Nada para interagir"]
```

---

### **4. COLOCAR NO MUNDO:**

1. Arraste `BP_StorageChest` para o level
2. Posicione próximo ao jogador
3. Ajuste rotação e escala se necessário

---

### **5. TESTAR:**

**A. Testar Inventário:**

1. Play (PIE)
2. Pressione `I` para abrir inventário
3. **Testar Drag do Inventário:**
   - Clique e arraste o inventário pela tela
   - Deve mover suavemente mantendo o offset
4. **Testar Drag & Drop de Itens:**
   - Arraste um item para outro slot
   - Deve mover corretamente
5. **Testar Stacking:**
   - Arraste um item para um slot com o mesmo item
   - Deve empilhar automaticamente
6. **Testar Split:**
   - Segure Shift + Arraste um item com quantidade > 1
   - Deve dividir a pilha
7. Feche com o botão X

**B. Testar Armazém:**

1. Aproxime-se do `BP_StorageChest`
2. Pressione `E` para interagir
3. Armazém deve abrir
4. **Testar Drag & Drop entre Inventário e Armazém:**
   - Arraste itens do inventário para o armazém
   - Arraste itens do armazém para o inventário
   - Reorganize itens dentro do armazém
5. **Testar Depositar Tudo:**
   - Clique em "Depositar Tudo"
   - Todos os itens do inventário devem ir para o armazém
6. **Testar Retirar Tudo:**
   - Clique em "Retirar Tudo"
   - Todos os itens do armazém devem ir para o inventário
7. Feche o armazém
8. Afaste-se > 200 unidades
9. Armazém deve fechar automaticamente

---

## 🎯 **RESULTADO ESPERADO:**

✅ Inventário abre e fecha suavemente
✅ Inventário é arrastável pela tela
✅ Itens podem ser arrastados entre slots
✅ Itens do mesmo tipo empilham automaticamente
✅ Shift + Drag divide pilhas
✅ Armazém abre ao interagir com o baú
✅ Drag & Drop funciona entre inventário e armazém
✅ Depositar/Retirar Tudo funciona
✅ UI mostra capacidade (ex: 25/100)
✅ Armazém fecha ao se afastar

---

## 🐛 **TROUBLESHOOTING:**

### **Inventário não é arrastável:**
- Verifique se `WBP_Inventory` tem Parent: `UmbraInventoryWidget`
- Recompile o Blueprint
- Verifique se o C++ foi compilado

### **Drag & Drop não funciona:**
- Verifique se `WBP_InventorySlot` tem Parent: `UmbraInventorySlotWidget`
- Recompile o Blueprint
- Verifique se o C++ foi compilado

### **Armazém não abre:**
- Verifique se `Storage Widget Class` está configurado em `BP_StorageChest`
- Verifique se o jogador está dentro de 200 unidades
- Verifique os logs: `LogTemp`

### **Stacking não funciona:**
- Verifique se os itens têm o mesmo `ItemTemplateID`
- Verifique se o `MaxStackSize` não foi atingido

### **Split não funciona:**
- Verifique se está segurando Shift
- Verifique se o item tem quantidade > 1

---

## 📊 **CAPACIDADES FINAIS:**

| Feature | Capacidade |
|---------|------------|
| Inventário | 30 slots (configurável) |
| Armazém | 100 slots |
| Max Stack (Consumíveis) | 99 |
| Max Stack (Recursos) | 999 |
| Max Stack (Equipamentos) | 1 |
| Distância Interação | 200 unidades |

---

## 🔥 **FEATURES AVANÇADAS PRONTAS:**

✅ **Stacking:** Empilhamento automático inteligente
✅ **Split:** Dividir pilhas com Shift + Drag
✅ **Validação:** Slots específicos por tipo
✅ **Armazém:** 100 slots + Actor interativo
✅ **Drag Universal:** Funciona em todos os contextos
✅ **Botões Especiais:** Depositar/Retirar Tudo
✅ **Auto-Close:** Fecha ao se afastar
✅ **Tooltips:** Informações detalhadas ao hover
✅ **Raridade:** Cores diferentes por raridade
✅ **Durabilidade:** Barra visual de durabilidade

---

## 🎮 **GAMEPLAY FINAL:**

**Jogador:**
1. Coleta itens no mundo
2. Abre inventário (I)
3. Organiza itens (drag & drop)
4. Empilha automaticamente (mesmo tipo)
5. Divide pilhas (Shift + Drag)
6. Fecha inventário (X)
7. Aproxima-se do armazém
8. Abre armazém (E)
9. Deposita itens (drag ou "Depositar Tudo")
10. Fecha armazém (X ou se afasta)

**Sistema gerencia:**
- ✅ Persistência no banco de dados
- ✅ Validações de tipo
- ✅ Limites de stack
- ✅ Chamadas API
- ✅ Feedback visual
- ✅ Sons e animações

---

## 🚀 **PRÓXIMOS PASSOS:**

1. ✅ Compile o C++
2. ✅ Configure os Blueprints
3. ✅ Crie o armazém
4. ✅ Teste tudo
5. 🎨 Adicione polish (sons, animações, VFX)
6. 🎮 **JOGUE!**

---

**SISTEMA COMPLETO E FUNCIONAL! 🎉🔥**

**Tempo total de implementação: ~30 minutos** ⚡

**Veja também:**
- `RESUMO_SOLUCAO_CPP_COMPLETA.md` - Visão geral
- `GUIA_FEATURES_AVANCADAS_INVENTARIO.md` - Detalhes técnicos
- `REFERENCIA_RAPIDA_ARMAZEM.md` - Quick start armazém
- `INDICE_COMPLETO_INVENTARIO.md` - Todos os guias

---

**BOM JOGO! 🎮✨**

