# 🔍 **DIAGNÓSTICO: Nameplates Duplicados - Mesmo Actor com Múltiplos PlayerIDs**

## 🚨 **PROBLEMA IDENTIFICADO:**

O mesmo remote actor está exibindo nameplates de **dois players diferentes** (ex: "ElJeffo" e "TheKillZone" no mesmo actor).

**Causa Raiz:**
- O mesmo actor físico está sendo associado a múltiplos PlayerIDs no `RemotePlayerActorsMap`
- Ou há múltiplos WidgetComponents no mesmo actor
- Ou o delegate está criando múltiplos widgets

---

## ✅ **VERIFICAÇÕES ADICIONADAS NO C++:**

O código agora:
1. ✅ Detecta se o mesmo actor está registrado para múltiplos PlayerIDs
2. ✅ Remove associações antigas quando detecta duplicação
3. ✅ Limpa o nameplate do actor antigo quando um PlayerID é reassociado
4. ✅ Detecta múltiplos WidgetComponents no mesmo actor

---

## 🔍 **VERIFICAÇÃO NO BLUEPRINT:**

### **1. Verificar se há múltiplos WidgetComponents no BP_RemotePlayer:**

1. Abra `BP_RemotePlayer`
2. Vá em **Components**
3. **Conte quantos componentes do tipo `Widget Component` existem**
4. Se houver **MAIS DE UM**, **DELETE os extras** - deve haver apenas **UM**

---

### **2. Verificar se o delegate está sendo conectado múltiplas vezes:**

**No `BP_NetMovementClient2` → `Event BeginPlay`:**

Verifique se o delegate `OnRemotePlayerNameplateUpdated` está sendo conectado **APENAS UMA VEZ**:

```
[Event BeginPlay]
    ↓
[Get Game Instance] → [Cast to Umbra Game Instance]
    ↓
[Assign OnRemotePlayerNameplateUpdated]
    Delegate: UpdateNameplateFromDelegate
```

**⚠️ IMPORTANTE:** Se houver múltiplas conexões do mesmo delegate, cada uma vai atualizar o widget, causando nameplates duplicados.

**Verificação:**
- Adicione um log no início de `UpdateNameplateFromDelegate`:
  ```
  [Print String] "UpdateNameplateFromDelegate: PlayerID=X, Actor=Y"
  ```
- Se aparecer múltiplas vezes para o mesmo PlayerID/Actor, o delegate está sendo disparado múltiplas vezes

---

### **3. Verificar se o UpdateNameplate está criando múltiplos widgets:**

**No `WBP_PlayerNameplate` → `UpdateNameplate`:**

Verifique se a função está **SUBSTITUINDO** o texto (não concatenando):

```
[UpdateNameplate]
    Inputs: CharacterName, TitleName
    ↓
[Set Text] (TB_CharacterName)
    Text: CharacterName ← DEVE SUBSTITUIR, NÃO CONCATENAR
    ↓
[Is Empty] (TitleName)
    ↓
[Branch]
    True → [Set Text] (TB_Title, "") ← Limpar se vazio
         → [Set Visibility] (TB_Title, Collapsed)
    False → [Set Text] (TB_Title, TitleName) ← DEVE SUBSTITUIR
          → [Set Visibility] (TB_Title, Visible)
```

**⚠️ VERIFICAR:**
- Não use `Append Text` ou `Format Text` que concatena
- Use apenas `Set Text` que substitui

---

### **4. Verificar se há múltiplos widgets sendo criados no BeginPlay:**

**No `BP_RemotePlayer` → `Event BeginPlay`:**

Verifique se o widget está sendo criado **APENAS UMA VEZ**:

```
[Event BeginPlay]
    ↓
[Get Component By Class] (Widget Component)
    ↓
[Is Valid?] WidgetComponent
    ↓
    then → [Create Widget] (WBP_PlayerNameplate)
          → [Set Widget] (no WidgetComponent)
```

**⚠️ IMPORTANTE:** Se o widget estiver sendo criado múltiplas vezes, cada criação vai criar um novo widget, causando nameplates duplicados.

---

## 🧪 **TESTE COM LOGS:**

Adicione logs para diagnosticar:

**No `UpdateNameplateFromDelegate` (Blueprint):**

```
[UpdateNameplateFromDelegate]
    Inputs: PlayerID, RemoteActor, CharacterName, CharacterTitle
    ↓
[Print String] "UpdateNameplateFromDelegate: PlayerID=" + ToString(PlayerID) + ", Actor=" + GetName(RemoteActor)
    ↓
[Get All Components By Class] (Widget Component, do RemoteActor)
    ↓
[Length] (Array de WidgetComponents)
    ↓
[Print String] "WidgetComponents encontrados: " + ToString(Length)
    ↓
(se Length > 1, há problema!)
```

---

## 📝 **RESUMO:**

- ❌ **PROBLEMA:** Mesmo actor exibindo nameplates de múltiplos players
- ✅ **CAUSA:** Mesmo actor associado a múltiplos PlayerIDs, ou múltiplos WidgetComponents, ou delegate disparado múltiplas vezes
- 🔧 **CORREÇÃO C++:** Adicionada detecção e remoção de associações duplicadas
- 🔧 **CORREÇÃO BLUEPRINT:** Verificar se há apenas um WidgetComponent e se o delegate está conectado apenas uma vez
