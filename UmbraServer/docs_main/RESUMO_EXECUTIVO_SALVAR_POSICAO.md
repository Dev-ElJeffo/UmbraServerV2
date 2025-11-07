# 🎯 **RESUMO EXECUTIVO: Sistema de Salvar Posição**

## 📋 **O QUE FAZER:**

### **1. Criar API PHP** ⚠️ **PRIORIDADE ALTA**

**Arquivo:** `www/umbra_api/api/character/update_position.php`

**Função:** Receber `player_id`, `pos_x`, `pos_y`, `pos_z` e atualizar no banco.

---

### **2. Adicionar Função C++** ⚠️ **PRIORIDADE ALTA**

**Arquivo:** `UmbraGameInstance.h` e `UmbraGameInstance.cpp`

**Função:** `SavePlayerPosition(int32 PlayerID, FVector Position, FString Zone)`

**Função:** Chamar API PHP para salvar posição.

---

### **3. Modificar Spawn Inicial** ⚠️ **PRIORIDADE ALTA**

**Blueprint:** Character principal (ex: `BP_ThirdPersonCharacter`)

**O que fazer:**
- No `BeginPlay`, obter `Position` do `Active Character` (do `UmbraGameInstance`)
- Se `Position != (0,0,0)`, usar esta posição no spawn
- Se `Position == (0,0,0)`, usar posição padrão (PlayerStart)

---

### **4. Salvar Posição Periodicamente** ⚠️ **PRIORIDADE MÉDIA**

**Blueprint:** Character principal

**O que fazer:**
- Adicionar `Timer` no `BeginPlay` (executar a cada 5 segundos)
- Criar Custom Event `SavePositionTimer`
- Chamar `Save Player Position` com posição atual

---

### **5. Salvar ao Desconectar** ⚠️ **PRIORIDADE MÉDIA**

**Blueprint:** `BP_NetMovementClient`

**O que fazer:**
- No `Event EndPlay`, antes de fechar WebSocket
- Chamar `Save Player Position` com posição atual

---

## 🔄 **FLUXO:**

```
LOGIN → Parse pos_x, pos_y, pos_z → Salvar em UmbraGameInstance
  ↓
SPAWN → Usar Position do banco (se válida)
  ↓
JOGO → Timer salva posição a cada 5 segundos
  ↓
DESCONECTAR → Salvar última posição
  ↓
PRÓXIMO LOGIN → Spawnar na última posição salva ✅
```

---

## ✅ **CHECKLIST RÁPIDO:**

- [ ] API PHP criada
- [ ] Função C++ adicionada e compilada
- [ ] Spawn inicial modificado
- [ ] Timer de salvamento implementado
- [ ] Salvamento ao desconectar implementado

---

**Veja `SISTEMA_SALVAR_POSICAO_PLAYER_BANCO.md` para detalhes completos!**

