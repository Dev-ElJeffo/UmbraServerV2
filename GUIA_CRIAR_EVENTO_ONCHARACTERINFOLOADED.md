# 🔧 GUIA: Criar e Conectar OnCharacterInfoLoaded_Event

## 🎯 **PROBLEMA:**

O evento `OnCharacterInfoLoaded_Event` não está aparecendo ou não está sendo chamado, então os dados não aparecem no widget.

---

## ✅ **SOLUÇÃO: Usar "Assign" para Criar o Evento Automaticamente**

### **PASSO 1: Criar o Evento Automaticamente (Método Correto)**

**No Event Graph do `WBP_CharacterInfo`:**

1. **No Event Construct:**
   - Após fazer `Cast to Umbra Game Instance`
   - **Arraste do pino azul** do `As Umbra Game Instance` (Cast)
   - Procure por: **"Assign On Character Info Loaded"**
   - Isso criará **automaticamente** um Custom Event chamado `On Character Info Loaded`

2. **Renomear o Evento (Opcional):**
   - O evento criado terá o nome padrão: `On Character Info Loaded`
   - Você pode renomeá-lo para `OnCharacterInfoLoaded_Event` se preferir
   - Clique no nó do evento → No painel "Details", altere o nome

3. **Verificar Input do Evento:**
   - O evento criado automaticamente já terá o input correto: `Character Info` (FUmbraCharacterInfo)
   - Não é necessário adicionar manualmente

---

## ✅ **PASSO 2: Conectar ao Delegate no Event Construct**

### **2.1 No Event Graph:**

1. **Criar Event Construct:**
   - Se não existir, clique com botão direito → **"Event Construct"**

2. **Obter Game Instance:**
   ```
   [Event Construct]
     ↓
   [Get Game Instance]
     ↓
   [Cast to Umbra Game Instance]
   ```

   **NÓS NECESSÁRIOS:**
   - **Get Game Instance** (clique direito → "Get Game Instance")
   - **Cast to Umbra Game Instance** (clique direito → "Cast to Umbra Game Instance")
     - **Object:** Return Value (Get Game Instance)
     - **As Umbra Game Instance:** (saída do Cast)

3. **Assign On Character Info Loaded (Cria o Evento Automaticamente):**
   ```
   [Cast to Umbra Game Instance]
     ↓
   [Assign On Character Info Loaded]
     ├─ Target: As Umbra Game Instance (Cast)
     └─ Event: On Character Info Loaded (criado automaticamente)
   ```

   **NÓS NECESSÁRIOS:**
   - **Assign On Character Info Loaded**
     - Arraste do pino azul do `As Umbra Game Instance` (Cast)
     - Procure por **"Assign On Character Info Loaded"**
     - Isso criará automaticamente um Custom Event chamado `On Character Info Loaded`
     - O evento já terá o input `Character Info` (FUmbraCharacterInfo)

4. **Chamar LoadCharacterInfo:**
   ```
   [Bind Event to OnCharacterInfoLoaded]
     ↓
   [Load Character Info]
     └─ Target: As Umbra Game Instance (Cast)
   ```

   **NÓS NECESSÁRIOS:**
   - **Load Character Info**
     - Clique direito → Procure por **"Load Character Info"**
     - **Target:** As Umbra Game Instance (Cast)

---

## ✅ **PASSO 3: Implementar OnCharacterInfoLoaded_Event**

### **3.1 No Event Graph:**

1. **Localizar o Evento:**
   - No Event Graph, você deve ver o nó `OnCharacterInfoLoaded_Event`
   - Ele deve ter um input: `Character Info` (FUmbraCharacterInfo)

2. **Conectar à Função Update Character Info Display:**
   ```
   [OnCharacterInfoLoaded_Event]
     └─ Character Info: (FUmbraCharacterInfo)
     ↓
   [Update Character Info Display]
     └─ Character Info: Character Info (input do evento)
   ```

   **NÓS NECESSÁRIOS:**
   - **Update Character Info Display** (função que você criou)
   - Conecte **Character Info** (input do evento) → **Character Info** (input da função)

---

## 🔍 **VERIFICAÇÃO: Como Saber se Está Correto**

### **1. Verificar se o Evento Existe:**
- No painel **"My Blueprint"**, procure por `OnCharacterInfoLoaded_Event`
- Deve aparecer na seção **"Event Dispatchers"** ou **"Custom Events"**

### **2. Verificar se Está Conectado:**
- No **Event Construct**, deve haver:
  - `Get Game Instance` → `Cast to Umbra Game Instance`
  - `Bind Event to OnCharacterInfoLoaded` (conectado ao Cast)
  - `Load Character Info` (conectado ao Cast)

### **3. Verificar se o Evento Tem Input:**
- Clique no nó `OnCharacterInfoLoaded_Event`
- No painel **"Details"**, deve haver um input chamado `Character Info` do tipo `FUmbraCharacterInfo`

### **4. Verificar se Está Conectado à Função:**
- O `OnCharacterInfoLoaded_Event` deve estar conectado a `Update Character Info Display`
- O input `Character Info` do evento deve estar conectado ao input `Character Info` da função

---

## 🐛 **TROUBLESHOOTING**

### **Problema 1: "Assign On Character Info Loaded não aparece"**

**Solução:**
- Certifique-se de que fez o **Cast to Umbra Game Instance** primeiro
- Arraste do pino azul do `As Umbra Game Instance` (Cast)
- Procure por **"Assign"** ou **"On Character Info Loaded"**
- Se não aparecer, compile o Blueprint primeiro e tente novamente

### **Problema 2: "O evento não foi criado automaticamente"**

**Solução:**
- Após usar **"Assign On Character Info Loaded"**, verifique se um novo Custom Event foi criado
- Procure no Event Graph por um nó chamado `On Character Info Loaded`
- Se não aparecer, tente compilar o Blueprint
- O evento deve aparecer automaticamente após usar o "Assign"

### **Problema 3: "Event não tem input Character Info"**

**Solução:**
- Clique no nó `OnCharacterInfoLoaded_Event`
- No painel **"Details"**, vá para **"Inputs"**
- Clique em **"+"** e adicione:
  - Nome: `Character Info`
  - Tipo: `Umbra Character Info`

### **Problema 4: "Dados não aparecem mesmo com tudo conectado"**

**Solução:**
- Verifique se `LoadCharacterInfo()` está sendo chamado no **Event Construct**
- Verifique se o widget está sendo criado e adicionado ao viewport
- Verifique se a API está retornando dados (teste no navegador)
- Adicione logs de debug no C++ para verificar se `OnCharacterInfoLoaded.Broadcast()` está sendo chamado

---

## 📋 **ESTRUTURA COMPLETA DO EVENT CONSTRUCT**

```
[Event Construct]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ├─ Object: Return Value (Get Game Instance)
  └─ As Umbra Game Instance: (saída)
        │
        ├─→ [Assign On Character Info Loaded]
        │   ├─ Target: As Umbra Game Instance
        │   └─ Event: On Character Info Loaded (criado automaticamente)
        │
        └─→ [Load Character Info]
            └─ Target: As Umbra Game Instance
```

---

## 📋 **ESTRUTURA COMPLETA DO ON CHARACTER INFO LOADED**

```
[On Character Info Loaded] (Custom Event criado automaticamente)
  └─ Character Info: (FUmbraCharacterInfo) [input automático]
        │
        └─→ [Update Character Info Display]
            └─ Character Info: Character Info (input do evento)
```

---

## ✅ **CHECKLIST:**

- [ ] Event Construct tem `Get Game Instance`
- [ ] Event Construct tem `Cast to Umbra Game Instance`
- [ ] Event Construct tem `Assign On Character Info Loaded` (cria evento automaticamente)
- [ ] Evento `On Character Info Loaded` foi criado automaticamente no Event Graph
- [ ] Evento tem input `Character Info` (FUmbraCharacterInfo) - automático
- [ ] Event Construct tem `Load Character Info`
- [ ] `On Character Info Loaded` está conectado a `Update Character Info Display`
- [ ] Input `Character Info` do evento está conectado ao input da função

---

## 🎯 **IMPORTANTE:**

1. **Use "Assign On Character Info Loaded"** - isso cria o evento automaticamente
2. **NÃO crie o evento manualmente** - o "Assign" faz isso por você
3. **O evento criado automaticamente já tem o input correto** `Character Info` (FUmbraCharacterInfo)
4. **LoadCharacterInfo() DEVE ser chamado** no Event Construct para carregar os dados
5. **O nome padrão do evento é "On Character Info Loaded"** - você pode renomeá-lo se preferir

---

## ✅ **PRONTO!**

Após seguir este guia, o evento será criado e conectado corretamente, e os dados aparecerão no widget quando a API retornar.

