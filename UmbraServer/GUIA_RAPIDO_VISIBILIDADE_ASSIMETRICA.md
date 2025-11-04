# ⚡ **GUIA RÁPIDO: Corrigir Visibilidade Assimétrica**

## 🔴 **PROBLEMA:**
- Standalone 0 vê Standalone 1 ✅
- Standalone 1 **NÃO** vê Standalone 0 ❌

---

## ✅ **SOLUÇÃO RÁPIDA:**

### **NO BLUEPRINT `BP_NetMovementClient` - `ProcessNextFrame`:**

**SUBSTITUIR:**
```
Not Equal: OutPlayerId != MyPlayerId  ← ERRADO (pode estar desatualizado)
```

**POR:**
```
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Not Equal: OutPlayerId != [Return Value do Get Active Player ID]  ← CORRETO
```

---

## 📝 **PASSO A PASSO:**

1. **Localize** o `Not Equal` que compara `OutPlayerId != MyPlayerId`

2. **Antes do `Not Equal`, adicione:**
   - `Get Game Instance`
   - `Cast to Umbra Game Instance`
   - `Get Active Player ID`

3. **Conecte:**
   - `Get Active Player ID` → `Return Value` → `Not Equal` → `Input B`
   - **Remova** a conexão de `MyPlayerId` → `Not Equal` → `Input B`

4. **Resultado:**
   - ✅ Filtro sempre usa valor atualizado
   - ✅ Ambos os clientes veem uns aos outros

---

## 🎯 **RESULTADO:**

- ✅ Standalone 0 vê Standalone 1
- ✅ Standalone 1 vê Standalone 0
- ✅ Visibilidade bidirecional funcionando

