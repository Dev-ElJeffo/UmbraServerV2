# 📊 DIAGRAMA VISUAL: OnDrop - Fluxo Completo

## 🎯 **OBJETIVO:**

Este documento mostra o **fluxo visual completo** do `OnDrop`, indicando claramente:
- **Quais nós conectar em cada caminho**
- **De onde vem cada conexão**
- **Para onde vai cada conexão**

---

## 📊 **DIAGRAMA COMPLETO:**

```
┌─────────────────────────────────────────────────────────────────┐
│ OnDrop (Override Function)                                      │
│ Inputs: MyGeometry, PointerEvent, Operation                     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│ Cast to Umbra Item Drag Drop Operation                          │
│ Object: Operation (do OnDrop)                                   │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      CastFailed
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Get Source Slot Widget                                           │
│ Target: As Umbra Item Drag Drop Operation (do Cast)             │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (azul)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Is Valid? (Source Slot Widget)                                   │
│ Object: Return Value (do Get Source Slot Widget)                │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (bool)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Branch (Is Valid?)                                               │
│ Condition: Return Value (do Is Valid?)                          │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
                 TRUE                FALSE
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Get Slot Data (Source)                                           │
│ Target: Return Value (do Get Source Slot Widget)                │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (azul)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Break Umbra Inventory Slot (Source)                             │
│ Umbra Inventory Slot: Return Value (do Get Slot Data)           │
│ Outputs: Inventory ID, Slot Index, etc.                         │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Inventory ID (int)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ > (Integer > Integer)                                            │
│ A: Inventory ID (do Break)                                       │
│ B: 0 (Make Literal Int)                                          │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              Return Value (bool)
                    │
                    ▼
┌─────────────────────────────────────────────────────────────────┐
│ Branch (Inventory ID > 0?)                                        │
│ Condition: Return Value (do >)                                   │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
                 TRUE                FALSE
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Get Parent Storage Widget (Source)                               │
│ Target: Return Value (do Get Source Slot Widget)                 │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (azul)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Is Valid? (Parent Storage Widget da Origem)                     │
│ Object: Return Value (do Get Parent Storage Widget Source)       │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (bool)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Branch (Origem é Storage?)                                        │
│ Condition: Return Value (do Is Valid? Origem)                    │
│                                                                   │
│ TRUE = Origem É Storage                                           │
│ FALSE = Origem NÃO É Storage (é Inventário)                       │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
                 TRUE                FALSE
    (Origem = Storage)        (Origem = Inventário)
                    │                   │
                    ▼                   ▼
        ┌───────────────────┐  ┌───────────────────┐
        │ CENÁRIO 1:        │  │ CENÁRIO 2:        │
        │ Storage → ?       │  │ Inventário → ?    │
        └───────────────────┘  └───────────────────┘
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Get Parent Storage Widget (Destino)                              │
│ Target: Self (do WBP_InventorySlot)                              │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (azul)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Is Valid? (Parent Storage Widget do Destino)                     │
│ Object: Return Value (do Get Parent Storage Widget Destino)      │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
              then (branco)      Return Value (bool)
                    │                   │
                    ▼                   ▼
┌─────────────────────────────────────────────────────────────────┐
│ Branch (Destino é Storage?)                                       │
│ Condition: Return Value (do Is Valid? Destino)                    │
│                                                                   │
│ TRUE = Destino É Storage                                          │
│ FALSE = Destino NÃO É Storage (é Inventário)                      │
└─────────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    │                   │
                 TRUE                FALSE
    (Destino = Storage)       (Destino = Inventário)
                    │                   │
                    ▼                   ▼
        ┌───────────────────┐  ┌───────────────────┐
        │ CENÁRIO 1A:        │  │ CENÁRIO 1B:       │
        │ Storage → Storage  │  │ Storage → Invent.│
        │ (Não implementar)  │  │ (IMPLEMENTAR)    │
        └───────────────────┘  └───────────────────┘
                    │                   │
                    ▼                   ▼
        ┌───────────────────┐  ┌───────────────────┐
        │ CENÁRIO 2A:        │  │ CENÁRIO 2B:       │
        │ Invent. → Storage  │  │ Invent. → Invent. │
        │ (IMPLEMENTAR)      │  │ (Já funciona)     │
        └───────────────────┘  └───────────────────┘
```

---

## 🔧 **CENÁRIO 1B: Storage → Inventário**

### **Fluxo Completo:**

```
Branch (Origem é Storage?) [TRUE]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Destino)
          └─ Target: Self
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget do Destino)
                  └─ Object: Return Value (do Get Parent Storage Widget Destino)
                  └─ then (branco)
                      └─ Branch (Destino é Storage?)
                          └─ Condition: Return Value (do Is Valid? Destino)
                          └─ FALSE (Destino = Inventário)
                              └─ execute (branco)
                                  └─ Get Parent Storage Widget (Source) [Para obter WBP_Storage]
                                      └─ Target: Return Value (do Get Source Slot Widget)
                                      └─ then (branco)
                                          └─ Is Valid? (WBP_Storage)
                                              └─ Object: Return Value (do Get Parent Storage Widget Source)
                                              └─ then (branco)
                                                  └─ Get Game Instance
                                                      └─ then (branco)
                                                          └─ Cast to Umbra Game Instance
                                                              └─ Object: Return Value (do Get Game Instance)
                                                              └─ then (branco)
                                                                  └─ Get Storage ID By Inventory ID
                                                                      └─ Target: As Umbra Game Instance
                                                                      └─ Inventory ID: Inventory ID (do Break Umbra Inventory Slot Source)
                                                                      └─ then (branco)
                                                                          └─ Branch (Storage ID > 0?)
                                                                              └─ Condition: Return Value > 0 (do Get Storage ID By Inventory ID)
                                                                              └─ TRUE
                                                                                  └─ execute (branco)
                                                                                      └─ Get Slot Index (Destino)
                                                                                          └─ Target: Self
                                                                                          └─ then (branco)
                                                                                              └─ Move Item From Storage
                                                                                                  └─ Target: Return Value (do Get Parent Storage Widget Source)
                                                                                                  └─ Storage Item ID: Return Value (do Get Storage ID By Inventory ID)
                                                                                                  └─ Target Slot Index: Return Value (do Get Slot Index Destino)
                                                                                                  └─ then (branco)
                                                                                                      └─ Branch (Return Value)
                                                                                                          └─ Condition: Return Value (do Move Item From Storage)
                                                                                                          └─ TRUE
                                                                                                              └─ Return Handled
                                                                                                          └─ FALSE
                                                                                                              └─ Return Unhandled
```

---

## 🔧 **CENÁRIO 2A: Inventário → Storage**

### **Fluxo Completo:**

```
Branch (Origem é Storage?) [FALSE]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Destino)
          └─ Target: Self
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget do Destino)
                  └─ Object: Return Value (do Get Parent Storage Widget Destino)
                  └─ then (branco)
                      └─ Branch (Destino é Storage?)
                          └─ Condition: Return Value (do Is Valid? Destino)
                          └─ TRUE (Destino = Storage)
                              └─ execute (branco)
                                  └─ Get Parent Storage Widget (Destino) [Para obter WBP_Storage]
                                      └─ Target: Self
                                      └─ then (branco)
                                          └─ Is Valid? (WBP_Storage)
                                              └─ Object: Return Value (do Get Parent Storage Widget Destino)
                                              └─ then (branco)
                                                  └─ Get Slot Index (Destino)
                                                      └─ Target: Self
                                                      └─ then (branco)
                                                          └─ Move Item To Storage
                                                              └─ Target: Return Value (do Get Parent Storage Widget Destino)
                                                              └─ Inventory Item ID: Inventory ID (do Break Umbra Inventory Slot Source)
                                                              └─ Target Slot Index: Return Value (do Get Slot Index Destino)
                                                              └─ then (branco)
                                                                  └─ Branch (Return Value)
                                                                      └─ Condition: Return Value (do Move Item To Storage)
                                                                      └─ TRUE
                                                                          └─ Return Handled
                                                                      └─ FALSE
                                                                          └─ Return Unhandled
```

---

## 🔧 **CENÁRIO 2B: Inventário → Inventário**

### **Fluxo Completo:**

```
Branch (Origem é Storage?) [FALSE]
  └─ execute (branco)
      └─ Get Parent Storage Widget (Destino)
          └─ Target: Self
          └─ then (branco)
              └─ Is Valid? (Parent Storage Widget do Destino)
                  └─ Object: Return Value (do Get Parent Storage Widget Destino)
                  └─ then (branco)
                      └─ Branch (Destino é Storage?)
                          └─ Condition: Return Value (do Is Valid? Destino)
                          └─ FALSE (Destino = Inventário)
                              └─ execute (branco)
                                  └─ Process Item Drop
                                      └─ Target: Self
                                      └─ Dragged Slot Widget: Return Value (do Get Source Slot Widget)
                                      └─ then (branco)
                                          └─ Return Handled
```

---

## 📝 **RESUMO DAS CONEXÕES POR CENÁRIO:**

### **CENÁRIO 1B: Storage → Inventário**

| Nó | Pino de Entrada | Origem da Conexão |
|---|---|---|
| Get Parent Storage Widget (Destino) | `execute` | `TRUE` do `Branch (Origem é Storage?)` |
| Get Parent Storage Widget (Destino) | `Target` | `Self` |
| Is Valid? (Destino) | `Object` | `Return Value` do `Get Parent Storage Widget (Destino)` |
| Branch (Destino é Storage?) | `Condition` | `Return Value` do `Is Valid? (Destino)` |
| Get Parent Storage Widget (Source) | `execute` | `FALSE` do `Branch (Destino é Storage?)` |
| Get Parent Storage Widget (Source) | `Target` | `Return Value` do `Get Source Slot Widget` |
| Get Storage ID By Inventory ID | `Inventory ID` | `Inventory ID` do `Break Umbra Inventory Slot (Source)` |
| Get Slot Index (Destino) | `Target` | `Self` |
| Move Item From Storage | `Target` | `Return Value` do `Get Parent Storage Widget (Source)` |
| Move Item From Storage | `Storage Item ID` | `Return Value` do `Get Storage ID By Inventory ID` |
| Move Item From Storage | `Target Slot Index` | `Return Value` do `Get Slot Index (Destino)` |

### **CENÁRIO 2A: Inventário → Storage**

| Nó | Pino de Entrada | Origem da Conexão |
|---|---|---|
| Get Parent Storage Widget (Destino) | `execute` | `FALSE` do `Branch (Origem é Storage?)` |
| Get Parent Storage Widget (Destino) | `Target` | `Self` |
| Is Valid? (Destino) | `Object` | `Return Value` do `Get Parent Storage Widget (Destino)` |
| Branch (Destino é Storage?) | `Condition` | `Return Value` do `Is Valid? (Destino)` |
| Get Parent Storage Widget (Destino) [2ª vez] | `execute` | `TRUE` do `Branch (Destino é Storage?)` |
| Get Parent Storage Widget (Destino) [2ª vez] | `Target` | `Self` |
| Get Slot Index (Destino) | `Target` | `Self` |
| Move Item To Storage | `Target` | `Return Value` do `Get Parent Storage Widget (Destino)` |
| Move Item To Storage | `Inventory Item ID` | `Inventory ID` do `Break Umbra Inventory Slot (Source)` |
| Move Item To Storage | `Target Slot Index` | `Return Value` do `Get Slot Index (Destino)` |

### **CENÁRIO 2B: Inventário → Inventário**

| Nó | Pino de Entrada | Origem da Conexão |
|---|---|---|
| Get Parent Storage Widget (Destino) | `execute` | `FALSE` do `Branch (Origem é Storage?)` |
| Get Parent Storage Widget (Destino) | `Target` | `Self` |
| Is Valid? (Destino) | `Object` | `Return Value` do `Get Parent Storage Widget (Destino)` |
| Branch (Destino é Storage?) | `Condition` | `Return Value` do `Is Valid? (Destino)` |
| Process Item Drop | `execute` | `FALSE` do `Branch (Destino é Storage?)` |
| Process Item Drop | `Target` | `Self` |
| Process Item Drop | `Dragged Slot Widget` | `Return Value` do `Get Source Slot Widget` |

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO:**

- [ ] Cast to Umbra Item Drag Drop Operation conectado
- [ ] Get Source Slot Widget conectado
- [ ] Is Valid? (Source Slot Widget) conectado
- [ ] Branch (Is Valid?) conectado
- [ ] Get Slot Data (Source) conectado
- [ ] Break Umbra Inventory Slot (Source) conectado
- [ ] Branch (Inventory ID > 0?) conectado
- [ ] Get Parent Storage Widget (Source) conectado
- [ ] Is Valid? (Parent Storage Widget da Origem) conectado
- [ ] Branch (Origem é Storage?) conectado
- [ ] Get Parent Storage Widget (Destino) conectado
- [ ] Is Valid? (Parent Storage Widget do Destino) conectado
- [ ] Branch (Destino é Storage?) conectado
- [ ] CENÁRIO 1B (Storage → Inventário) implementado
- [ ] CENÁRIO 2A (Inventário → Storage) implementado
- [ ] CENÁRIO 2B (Inventário → Inventário) implementado
- [ ] Todos os Return Handled/Unhandled conectados

---

**Com este diagrama visual, você deve conseguir implementar o OnDrop corretamente!** 🚀

