# ✅ **CORREÇÃO: Is Empty Não Conectado Corretamente**

## 🎯 **PROBLEMA**

O log mostra `CharacterName = ElJeffo` (NÃO está vazio), mas o `UpdateNameplateFromCharacterInfo` não está sendo chamado.

## ✅ **SOLUÇÃO**

**O problema está na conexão do `Is Empty` com o `Branch`:**

### **VERIFICAÇÃO:**

1. **O pino `InString` do `Is Empty`** deve estar conectado ao pino `Character Name` do `Break Umbra Character Info`
2. **O pino `ReturnValue` (bool) do `Is Empty`** deve estar conectado ao pino `Condition` do `Branch`
3. **O pino `False` do `Branch`** (quando CharacterName NÃO está vazio) deve estar conectado ao `UpdateNameplateFromCharacterInfo`

### **LÓGICA:**

- `Is Empty` retorna `False` quando a string **NÃO** está vazia
- `Branch` executa o pino `False` quando a condição é `False`
- **Portanto:** Quando `CharacterName` NÃO está vazio → `Is Empty` retorna `False` → `Branch` executa o pino `False` → Chama `UpdateNameplateFromCharacterInfo`

---

## 📋 **VERIFICAÇÃO RÁPIDA**

**No seu código, verifique:**

1. **`K2Node_CallFunction_40` (Is Empty):**
   - `InString` está conectado ao `Character Name` do `Break Umbra Character Info`? ✅ (parece estar)
   - `ReturnValue` está conectado ao `Condition` do `Branch`? ✅ (parece estar)

2. **`K2Node_IfThenElse_6` (Branch):**
   - `Condition` está conectado ao `ReturnValue` do `Is Empty`? ✅ (parece estar)
   - `else` (False) está conectado ao `K2Node_CallFunction_45` (log) e depois ao `K2Node_CallFunction_41` (UpdateNameplateFromCharacterInfo)? ✅ (parece estar)

**Se tudo está conectado corretamente, o problema está DENTRO do `UpdateNameplateFromCharacterInfo`.**

---

## 🔍 **PRÓXIMO PASSO**

**Adicione um `Print String` no INÍCIO do `UpdateNameplateFromCharacterInfo` (Custom Event):**

- Logo após o `then` do evento
- Mensagem: `"🟢 UpdateNameplateFromCharacterInfo CHAMADO!"`

**Se este log NÃO aparecer:** O problema está na conexão do `Branch`.

**Se este log APARECER:** O problema está dentro do `UpdateNameplateFromCharacterInfo` (Widget Component, Cast, etc.).

---

**FIM DA CORREÇÃO**
