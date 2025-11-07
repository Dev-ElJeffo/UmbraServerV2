# ✅ **RESPOSTA DIRETA: Onde Adicionar a Lógica do OutIsInAir**

## 🎯 **RESPOSTA:**

A lógica do `OutIsInAir` que você criou (Branch → Set Movement Mode) deve ser adicionada em **AMBOS os caminhos**:

1. ✅ **Após `Set Velocity` no caminho de actor existente** (`FoundIndex >= 0`)
2. ✅ **Após `Set Velocity` no caminho de novo actor** (`FoundIndex < 0`)

---

## 📊 **VISUALIZAÇÃO:**

### **Caminho 1: Actor Existente (FoundIndex >= 0)**

```
Set RemoteActorRef
  ↓
Cast to Character
  ↓ (D - sucesso)
Get Character Movement
  ↓
Set Velocity
  ↓ (then) ← ADICIONE A LÓGICA AQUI
Branch (OutIsInAir)
  ├─ True → Set Movement Mode (Falling)
  └─ False → Set Movement Mode (Walking)
  ↓
Set Actor Location
```

### **Caminho 2: Novo Actor (FoundIndex < 0)**

```
SpawnActorFromClass
  ↓
Set RemoteActorRef
  ↓
Cast to Character
  ↓ (D - sucesso)
Get Character Movement
  ↓
Set Velocity
  ↓ (then) ← ADICIONE A LÓGICA AQUI
Branch (OutIsInAir)
  ├─ True → Set Movement Mode (Falling)
  └─ False → Set Movement Mode (Walking)
  ↓
Array_Add (RemoteActorIds)
Array_Add (RemoteActors)
  ↓
Set Actor Location
```

---

## 💡 **DICA PRÁTICA:**

Você pode criar a lógica uma vez e depois **copiar/colar** para o outro caminho:

1. **Crie a lógica completa** após o primeiro `Set Velocity` (no caminho que você preferir)
2. **Selecione todos os nós** da lógica (`Branch`, `Set Movement Mode (Falling)`, `Set Movement Mode (Walking)`)
3. **Copie** (Ctrl+C)
4. **Cole** após o segundo `Set Velocity`
5. **Ajuste as conexões:**
   - Conecte o pin `execute` do `Branch` ao pin `then` do `Set Velocity` correspondente
   - Conecte os pinos `then` dos `Set Movement Mode` ao próximo passo (Knot_133 ou Set Actor Location)

---

## ✅ **VERIFICAÇÃO:**

Após adicionar em ambos os caminhos, verifique:

- [ ] `Branch` com `OutIsInAir` está presente após `Set Velocity` (actor existente)?
- [ ] `Branch` com `OutIsInAir` está presente após `Set Velocity` (novo actor)?
- [ ] Ambos os `Branch` estão conectados ao mesmo `OutIsInAir` de `ParseStateUpdateFrameWithAnimation`?
- [ ] Ambos os caminhos convergem corretamente para `Set Actor Location`?

---

## 🎯 **RESULTADO ESPERADO:**

- ✅ **Actors existentes** recebem `Set Movement Mode` baseado em `OutIsInAir`
- ✅ **Novos actors** recebem `Set Movement Mode` baseado em `OutIsInAir`
- ✅ **Animações de queda/pulo** funcionam corretamente em ambos os casos

---

**A lógica que você criou está perfeita! Agora é só aplicá-la em ambos os caminhos após cada `Set Velocity`.**

