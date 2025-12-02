# ⚡ RESUMO RÁPIDO: Conectar Delegates OnItemEquipped/Unequipped

## 🎯 **OBJETIVO:**

Conectar os delegates do Game Instance aos eventos do `WBP_CharacterInfo` para atualizar automaticamente os slots quando itens forem equipados/desequipados.

---

## ✅ **CHECKLIST RÁPIDO:**

1. ✅ Criar evento `OnItemEquipped_Event` (parâmetros: `Inventory ID`, `Equipment Slot`)
2. ✅ Criar evento `OnItemUnequipped_Event` (parâmetros: `Inventory ID`, `Equipment Slot`)
3. ✅ Implementar lógica: `Get Game Instance` → `Cast` → `Load Character Info`
4. ✅ No `Event Construct`: `Get Game Instance` → `Cast` → `Assign On Item Equipped` → `Assign On Item Unequipped`
5. ✅ Verificar se `Update Equipment Slots` é chamado no `OnCharacterInfoLoaded_Event`

---

## 🔧 **ESTRUTURA BÁSICA:**

### **Eventos:**
```
[OnItemEquipped_Event]
  └─ Inventory ID, Equipment Slot
       │
       ▼
[Get Game Instance] → [Cast] → [Load Character Info]

[OnItemUnequipped_Event]
  └─ Inventory ID, Equipment Slot
       │
       ▼
[Get Game Instance] → [Cast] → [Load Character Info]
```

### **Event Construct:**
```
[Event Construct]
  ↓
[Get Game Instance] → [Cast to Umbra Game Instance]
  └─ Success:
       ├─ [Assign On Item Equipped] → OnItemEquipped_Event
       └─ [Assign On Item Unequipped] → OnItemUnequipped_Event
```

---

## 📘 **REFERÊNCIAS:**

- **Guia Completo:** `GUIA_CONECTAR_DELEGATES_EQUIPAR_ITENS.md`
- **Diagrama Visual:** `DIAGRAMA_VISUAL_CONECTAR_DELEGATES.txt`

---

## 🎯 **PRONTO!**

Siga o guia completo para implementação detalhada! 🎉

