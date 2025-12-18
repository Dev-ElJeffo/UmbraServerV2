# 🔗 **GUIA: Conectar Delegate OnCharacterInfoLoaded ao HUD**

## 🎯 **OBJETIVO**

Conectar o delegate `OnCharacterInfoLoaded` do `UmbraGameInstance` ao widget `WBP_PlayerHUD` para que as barras de HP/MP sejam atualizadas automaticamente sempre que o `CurrentCharacterInfo` for atualizado.

---

## ✅ **PASSO 1: Criar Função de Atualização no HUD**

### **1.1. No `WBP_PlayerHUD`, crie uma função:**

**Nome:** `UpdateStatsFromCharacterInfo`

**Inputs:**
- `CharacterInfo` (FUmbraCharacterInfo)

**Lógica:**
```
[UpdateStatsFromCharacterInfo]
    Input: CharacterInfo
    ↓
[UpdateHPBar]
    CurrentHP: CharacterInfo.CurrentHealth
    MaxHP: CharacterInfo.MaxHealthTotal
    ↓
[UpdateMPBar]
    CurrentMP: CharacterInfo.CurrentMana
    MaxMP: CharacterInfo.MaxManaTotal
```

---

## ✅ **PASSO 2: Criar Custom Event para Receber o Delegate**

### **2.1. No `WBP_PlayerHUD`, crie um Custom Event:**

**Nome:** `OnCharacterInfoUpdated`

**Inputs:**
- `CharacterInfo` (FUmbraCharacterInfo)

**Lógica:**
```
[OnCharacterInfoUpdated]
    Input: CharacterInfo
    ↓
[UpdateStatsFromCharacterInfo]
    CharacterInfo: Input CharacterInfo
```

---

## ✅ **PASSO 3: Conectar o Delegate no Event Construct**

### **3.1. No `Event Construct` do `WBP_PlayerHUD`:**

**Estrutura completa:**

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get OnCharacterInfoLoaded] (delegate do GameInstance)
    ↓
[Assign] (não Call!)
    ↓
[OnCharacterInfoUpdated] (seu Custom Event)
```

### **3.2. Passo a Passo no Blueprint:**

1. **No `Event Construct`:**
   - Adicione `Get Game Instance`
   - Conecte ao `Cast to Umbra Game Instance`

2. **Obter o Delegate:**
   - **Arraste o pin `OnCharacterInfoLoaded`** do `Umbra Game Instance`
   - **IMPORTANTE:** Você verá duas opções:
     - **`Call OnCharacterInfoLoaded`** ❌ (NÃO use isso)
     - **`Assign OnCharacterInfoLoaded`** ✅ (USE ESTE!)

3. **Selecionar `Assign`:**
   - Isso abrirá um menu de contexto
   - **Selecione `OnCharacterInfoUpdated`** (seu Custom Event criado no Passo 2)
   - OU crie um novo Custom Event diretamente neste menu

4. **Resultado:**
   - O delegate `OnCharacterInfoLoaded` do GameInstance agora está conectado ao seu Custom Event `OnCharacterInfoUpdated`
   - Sempre que o GameInstance atualizar `CurrentCharacterInfo` e chamar `OnCharacterInfoLoaded.Broadcast()`, seu Custom Event será executado automaticamente

---

## ✅ **PASSO 4: Atualização Inicial (Opcional mas Recomendado)**

### **4.1. No `Event Construct`, após conectar o delegate:**

**Adicione uma atualização inicial para garantir que o HUD seja atualizado mesmo se o `CurrentCharacterInfo` já estiver carregado:**

```
[Event Construct]
    ↓
[... conectar delegate ...]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓
[Get Current Character Info]
    ↓
[UpdateStatsFromCharacterInfo]
    CharacterInfo: CurrentCharacterInfo obtido
```

**Isso garante que:**
- Se o `CurrentCharacterInfo` já estiver carregado quando o widget for criado, o HUD será atualizado imediatamente
- O delegate cuidará das atualizações futuras

---

## 🔍 **DETALHES TÉCNICOS**

### **Por que usar `Assign` e não `Call`?**

- **`Call`**: Executa o delegate imediatamente (não é o que queremos)
- **`Assign`**: Conecta seu Custom Event ao delegate, fazendo com que ele seja chamado automaticamente sempre que o delegate for disparado

### **Como funciona o Delegate?**

1. O `UmbraGameInstance` tem um delegate `OnCharacterInfoLoaded` do tipo `FOnCharacterInfoLoaded`
2. Quando `CurrentCharacterInfo` é atualizado (ex: após carregar do servidor), o GameInstance chama `OnCharacterInfoLoaded.Broadcast(CharacterInfo)`
3. Todos os widgets/objetos que se conectaram a este delegate recebem automaticamente a chamada com o `CharacterInfo` atualizado
4. Seu Custom Event `OnCharacterInfoUpdated` é executado e atualiza as barras de HP/MP

---

## ✅ **VERIFICAÇÃO**

Após implementar:

1. ✅ O HUD deve ser atualizado automaticamente quando o `CurrentCharacterInfo` for atualizado
2. ✅ Não é necessário usar Timers ou chamadas manuais
3. ✅ O sistema é reativo e eficiente

---

## ⚠️ **NOTAS IMPORTANTES**

- **Sempre use `Assign` para conectar delegates**, nunca `Call`
- O delegate só funciona se o `WBP_PlayerHUD` estiver criado e no viewport
- Se você criar o widget depois que o `CurrentCharacterInfo` já foi carregado, use a atualização inicial (Passo 4) para garantir que o HUD mostre os valores corretos

---

**FIM DO GUIA**

