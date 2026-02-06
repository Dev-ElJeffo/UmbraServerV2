# 🔧 **CORREÇÃO: Nameplates Duplicados e Terceiro Cliente Sem Nameplates**

## 🚨 **PROBLEMAS IDENTIFICADOS:**

1. **Nameplates duplicados/sobrepostos**: Nome e título de dois personagens aparecem no mesmo remote actor
2. **Terceiro cliente não vê nameplates**: O terceiro cliente a logar não vê nameplates de nenhum remote actor

---

## 🔍 **ANÁLISE:**

### **Problema 1: Nameplates Duplicados**

**Causa provável:**
- O widget `WBP_PlayerNameplate` não está limpando o texto anterior antes de atualizar
- Ou o `UpdateNameplate` está sendo chamado múltiplas vezes e acumulando texto
- Ou há múltiplos widgets sendo criados para o mesmo actor

**Verificação no Blueprint `WBP_PlayerNameplate`:**

A função `UpdateNameplate` **DEVE** limpar o texto ANTES de definir o novo:

```
[UpdateNameplate]
    Inputs: CharacterName, TitleName
    ↓
[Set Text] (TB_CharacterName)
    Text: "" (STRING VAZIA) ← LIMPAR PRIMEIRO!
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName ← DEPOIS DEFINIR O NOVO
    ↓
[Is Empty] (TitleName)
    ↓
[Branch]
    True → [Set Text] (TB_Title, "") ← LIMPAR
         → [Set Visibility] (TB_Title, Collapsed)
    False → [Set Text] (TB_Title, "") ← LIMPAR PRIMEIRO!
          → [Set Text] (TB_Title, TitleName) ← DEPOIS DEFINIR
          → [Set Visibility] (TB_Title, Visible)
```

**OU usar `Set Text` diretamente (substitui automaticamente):**

```
[UpdateNameplate]
    Inputs: CharacterName, TitleName
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName ← Substitui automaticamente
    ↓
[Is Empty] (TitleName)
    ↓
[Branch]
    True → [Set Text] (TB_Title, "") ← Limpar se vazio
         → [Set Visibility] (TB_Title, Collapsed)
    False → [Set Text] (TB_Title, TitleName) ← Substitui automaticamente
          → [Set Visibility] (TB_Title, Visible)
```

---

### **Problema 2: Terceiro Cliente Não Vê Nameplates**

**Causa provável:**
- O snapshot inicial (`sendInitialSnapshotLocked`) está enviando `PlayerInfoUpdate` corretamente
- Mas o terceiro cliente pode não estar processando os `PlayerInfoUpdate` recebidos
- Ou os dados estão sendo armazenados como pendentes mas não aplicados quando o actor é spawnado

**Verificação:**

1. **Verificar se o terceiro cliente está recebendo `PlayerInfoUpdate`:**
   - Adicione logs no `OnWSBinaryMessage` para verificar se mensagens tipo 4 estão chegando
   - Verifique se `ParsePlayerInfoUpdate` está sendo chamado

2. **Verificar se os dados pendentes estão sendo aplicados:**
   - Quando o actor é spawnado, verifique se há dados pendentes
   - Verifique se `RegisterRemotePlayerActor` está aplicando os dados pendentes

---

## ✅ **CORREÇÕES:**

### **1. Corrigir o Widget para Limpar Texto Antes de Atualizar**

**No `WBP_PlayerNameplate` → `UpdateNameplate`:**

**❌ ERRADO (acumula texto):**
```
[Set Text] (TB_CharacterName)
    Text: CharacterName ← Pode acumular se já tiver texto
```

**✅ CORRETO (substitui texto):**
```
[Set Text] (TB_CharacterName)
    Text: "" ← Limpar primeiro (opcional, Set Text já substitui)
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName ← Substitui o texto anterior
```

**OU simplesmente:**
```
[Set Text] (TB_CharacterName)
    Text: CharacterName ← Set Text já substitui automaticamente
```

---

### **2. Verificar se o Blueprint Está Atualizando Múltiplas Vezes**

**No `BP_NetMovementClient2` → `UpdateNameplateFromDelegate`:**

Verifique se o delegate está sendo conectado múltiplas vezes:

```
[Event BeginPlay]
    ↓
[Get Game Instance] → [Cast to Umbra Game Instance]
    ↓
[Assign OnRemotePlayerNameplateUpdated]
    Delegate: UpdateNameplateFromDelegate
```

**⚠️ IMPORTANTE:** O delegate deve ser conectado **APENAS UMA VEZ** no `BeginPlay`. Se estiver sendo conectado múltiplas vezes, cada conexão vai atualizar o widget.

**Verificação:**
- Adicione um log no início de `UpdateNameplateFromDelegate`:
  ```
  [Print String] "UpdateNameplateFromDelegate chamado: PlayerID=X, Name=Y"
  ```
- Se aparecer múltiplas vezes para o mesmo PlayerID, o delegate está sendo disparado múltiplas vezes

---

### **3. Adicionar Proteção Contra Atualizações Duplicadas**

**No `UpdateNameplateFromDelegate` (Blueprint):**

Adicione uma verificação para evitar atualizar se os dados são os mesmos:

```
[UpdateNameplateFromDelegate]
    Inputs: PlayerID, RemoteActor, CharacterName, CharacterTitle
    ↓
[Is Valid?] RemoteActor
    ↓
[Get Widget Component] (do RemoteActor)
    ↓
[Is Valid?] WidgetComponent
    ↓
[Get User Widget Object] (do WidgetComponent)
    ↓
[Is Valid?] UserWidget
    ↓
[Cast to WBP Player Nameplate]
    ↓
[Update Nameplate]
    CharacterName: CharacterName
    TitleName: CharacterTitle
```

**⚠️ IMPORTANTE:** Não adicione lógica de "verificar se já tem o mesmo texto" - o `Set Text` já substitui automaticamente. O problema pode ser que há múltiplos widgets ou o texto está sendo concatenado em vez de substituído.

---

### **4. Verificar se Há Múltiplos Widgets**

**No `BP_RemotePlayer`:**

Verifique se há apenas **UM** `WidgetComponent` configurado:

1. Abra `BP_RemotePlayer`
2. Vá em **Components**
3. Verifique se há apenas **UM** componente do tipo `Widget Component`
4. Se houver múltiplos, **DELETE os extras**

**Verificação no código C++:**

O código busca o widget usando `FindComponentByClass<UWidgetComponent>()`, que retorna o **primeiro** componente encontrado. Se houver múltiplos, pode estar atualizando o widget errado.

---

## 🧪 **TESTE:**

1. **Recompilar o código C++ e o Blueprint**
2. **Testar com 3 clientes:**
   - Cliente 1 loga primeiro
   - Cliente 2 loga depois
   - Cliente 3 loga por último
3. **Verificar:**
   - Cada remote actor deve ter **APENAS UM** nameplate (nome + título)
   - O terceiro cliente deve ver nameplates de todos os outros clientes
   - Não deve haver nameplates duplicados/sobrepostos

---

## 📝 **RESUMO:**

- ❌ **PROBLEMA 1:** Nameplates duplicados - widget pode estar acumulando texto ou há múltiplos widgets
- ✅ **SOLUÇÃO 1:** Verificar se `UpdateNameplate` limpa o texto antes de atualizar, e verificar se há apenas um WidgetComponent
- ❌ **PROBLEMA 2:** Terceiro cliente não vê nameplates - dados pendentes podem não estar sendo aplicados
- ✅ **SOLUÇÃO 2:** Verificar se `RegisterRemotePlayerActor` está aplicando dados pendentes corretamente
