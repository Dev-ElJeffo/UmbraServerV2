# 📝 **COMO USAR PRINT STRING COM MÚLTIPLOS VALORES**

## 🎯 **MÉTODO 1: Format Text (RECOMENDADO)**

### **Passo a Passo:**

1. **Adicione `Format Text`** (não `Print String` diretamente)
2. **Configure o `Format Pattern`:**
   - Digite: `"[Event EndPlay] EVENTO DISPARADO! EndPlayReason: {0}"`
   - O `{0}` é um placeholder que será substituído pelo primeiro valor
   - Para mais valores, use `{1}`, `{2}`, etc.

3. **Conecte os valores:**
   - `Format Text` tem pins `In Fmt` (Format Pattern) e `In Args` (Array de valores)
   - **OU** use os pins individuais `Arg0`, `Arg1`, `Arg2`, etc.

4. **Conecte `Format Text` ao `Print String`:**
   - O `Return Value` de `Format Text` vai para o `In String` de `Print String`

### **Exemplo Visual:**

```
Event EndPlay
  ↓
Format Text
  - Format Pattern: "[Event EndPlay] EVENTO DISPARADO! EndPlayReason: {0}"
  - Arg0: EndPlayReason (do evento)
  → Return Value
  ↓
Print String
  - In String: Return Value (do Format Text)
```

---

## 🎯 **MÉTODO 2: Append String (Para Concatenar)**

### **Passo a Passo:**

1. **Adicione `Append String`** (ou múltiplos `Append String` em cadeia)
2. **Primeiro `Append String`:**
   - `A`: Texto fixo (ex: `"[Event EndPlay] EVENTO DISPARADO! EndPlayReason: "`)
   - `B`: Converter valor para string (ex: `ToString(EndPlayReason)`)
   - `Return Value`: String concatenada

3. **Se precisar de mais valores, adicione outro `Append String`:**
   - `A`: Return Value do primeiro `Append String`
   - `B`: Próximo valor convertido para string
   - `Return Value`: Nova string concatenada

4. **Conecte ao `Print String`:**
   - O `Return Value` final vai para o `In String` de `Print String`

### **Exemplo Visual:**

```
Event EndPlay
  ↓
ToString (EndPlayReason) ← Converter EndPlayReason para string
  ↓
Append String
  - A: "[Event EndPlay] EVENTO DISPARADO! EndPlayReason: "
  - B: Return Value (do ToString)
  → Return Value
  ↓
Print String
  - In String: Return Value (do Append String)
```

---

## 🎯 **MÉTODO 3: Format Text com Múltiplos Argumentos**

### **Para 2 ou mais valores:**

```
Event EndPlay
  ↓
Format Text
  - Format Pattern: "[Event EndPlay] PlayerID: {0}, EndPlayReason: {1}"
  - Arg0: Get Active Player ID (ou variável PlayerID)
  - Arg1: EndPlayReason
  → Return Value
  ↓
Print String
  - In String: Return Value
```

**OU usando Array:**

```
Event EndPlay
  ↓
Make Array (String)
  - [0]: ToString(Get Active Player ID)
  - [1]: ToString(EndPlayReason)
  ↓
Format Text
  - Format Pattern: "[Event EndPlay] PlayerID: {0}, EndPlayReason: {1}"
  - In Args: Array (do Make Array)
  → Return Value
  ↓
Print String
  - In String: Return Value
```

---

## 📋 **EXEMPLO ESPECÍFICO: Log do Event EndPlay**

### **Usando Format Text (Mais Simples):**

```
Event EndPlay
  ↓
Format Text
  - Format Pattern: "🔴 [Event EndPlay] EVENTO DISPARADO! EndPlayReason: {0}"
  - Arg0: EndPlayReason
  → Return Value
  ↓
Print String
  - In String: Return Value
```

### **Usando Append String:**

```
Event EndPlay
  ↓
ToString (EndPlayReason)
  → Return Value
  ↓
Append String
  - A: "🔴 [Event EndPlay] EVENTO DISPARADO! EndPlayReason: "
  - B: Return Value (do ToString)
  → Return Value
  ↓
Print String
  - In String: Return Value (do Append String)
```

---

## 🎯 **EXEMPLO: Log com RemoteActors.Num()**

### **Usando Format Text:**

```
Get Array Length (RemoteActors)
  → Return Value
  ↓
Format Text
  - Format Pattern: "🔵 [CleanupRemoteActors] RemoteActors.Num(): {0}"
  - Arg0: Return Value (do Get Array Length)
  → Return Value
  ↓
Print String
  - In String: Return Value
```

### **Usando Append String:**

```
Get Array Length (RemoteActors)
  → Return Value
  ↓
ToString (Return Value) ← Converter int para string
  → Return Value
  ↓
Append String
  - A: "🔵 [CleanupRemoteActors] RemoteActors.Num(): "
  - B: Return Value (do ToString)
  → Return Value
  ↓
Print String
  - In String: Return Value
```

---

## ⚠️ **IMPORTANTE:**

- **Format Text** é mais fácil para múltiplos valores
- **Append String** é mais simples para 1-2 valores
- **Sempre converta valores numéricos para string** usando `ToString` antes de concatenar
- **EndPlayReason** é um `byte` (enum), então use `ToString` para converter

---

## 🎯 **RECOMENDAÇÃO:**

**Use `Format Text`** - é mais limpo e fácil de ler quando você tem múltiplos valores.

