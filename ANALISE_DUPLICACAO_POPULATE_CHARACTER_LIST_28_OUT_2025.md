# Análise: Duplicação de PopulateCharacterList - 28 de Outubro de 2025

## 🎯 Problema Identificado

### Evidência dos Logs

**Sequência do Log (Segunda Conta):**
```
[1640] Login successful, loading characters
[1641] Conectando ao Gateway Server
[1642] ⚠️ Debug: PopulateCharacterList chamada  ← PRIMEIRA CHAMADA (ANTES DOS DADOS)
[1643] ⚠️ 0  ← Array vazio
[1644] ⚠️ [WBP_CharacterItem_C_1] 0  ← ID errado
[1645] Response do servidor chega
[1652] Parsed Player ID: 10  ← ID correto
[1657] ✅ DEBUG: Lista carregada  ← Delegate disparado
```

---

## 🔍 Análise da Estrutura Atual

### XML do Event Graph (Construct)

**Estrutura Correta:**
1. Event Construct
2. Get Game Instance
3. DynamicCast → UmbraGameInstance
4. Set MyGameInstance
5. IsValid (Macro)
6. **Add Delegate** `OnCharacterListLoaded` → Custom Event
7. Custom Event `OnCharacterListLoaded_Event`:
   - Delay 0.2
   - Print "DEBUG: Lista carregada"
   - Call `PopulateCharacterList`

**Esta estrutura está CORRETA!**

---

## 🐛 Problema: Dupla Execução

### Primeira Execução (Incorreta)
```
Event: Algo que chama PopulateCharacterList
  → PopulateCharacterList (array ainda vazio)
```

### Segunda Execução (Correta)
```
Event: OnCharacterListLoaded_Event
  → Delay 0.2
  → PopulateCharacterList (array já populado)
```

---

## 🔎 Investigação Necessária

### Possíveis Causas

#### 1. `OnLoginSuccess` também está chamando `PopulateCharacterList`

**Verificar se há uma conexão no Event Graph:**
```
Event: OnLoginSuccess (delegate do UmbraGameInstance)
  → PopulateCharacterList  ← PROBLEMA
```

#### 2. Um botão ou outro evento está chamando `PopulateCharacterList`

**Verificar Event Graph completo para:**
- Event Construct ✅ (correto - apenas bind de delegate)
- Event BeginPlay (verificar)
- Event OnLoginSuccess (verificar)
- Botões ou outros eventos (verificar)

#### 3. Widget anterior ainda existe e está chamando novamente

Quando um novo widget é criado, o antigo pode ainda estar ativo e executando eventos.

---

## 🛠️ Como Verificar e Corrigir

### Passo 1: Adicionar Print de Identificação

**No Event Graph do `WBP_CharacterSelection`:**

Adicionar **prints com identificação** em TODOS os lugares que chamam `PopulateCharacterList`:

```blueprint
Event: Qualquer Event
  → Print String "DEBUG: PopulateCharacterList via Event: XXXX"
  → Call PopulateCharacterList
```

### Passo 2: Verificar TODAS as Conexões

**Procurar no Event Graph:**

1. **Right-click** → **Search in Blueprint**
2. **Pesquisar**: `PopulateCharacterList`
3. **Verificar TODAS as conexões** que chamam esta função
4. **Adicionar prints** em cada uma

### Passo 3: Verificar OnLoginSuccess

**Verificar se existe conexão:**

```
MyGameInstance → OnLoginSuccess
  → PopulateCharacterList  ← VERIFICAR SE EXISTE
```

**Se existir, REMOVER esta conexão!**

---

## 📝 Estrutura Esperada (Correta)

### Event Graph do WBP_CharacterSelection

```
Event: Construct
  → GetGameInstance
  → Cast to UmbraGameInstance
  → Set MyGameInstance
  → IsValid (if valid)
    → Add Delegate: OnCharacterListLoaded
    (else)
      → (nada)

Custom Event: OnCharacterListLoaded_Event
  → Print "DEBUG: Lista carregada"
  → Delay 0.2
  → Call PopulateCharacterList  ✅ ÚNICO LUGAR

Custom Event: OnLoginSuccess (se existir)
  → NADA relacionado a PopulateCharacterList  ✅
```

---

## ✅ Solução

### 1. Remover Conexões Incorretas

**No Unreal Editor:**

1. Abrir `WBP_CharacterSelection`
2. Ir ao **Event Graph**
3. **Right-click** → **Search in Blueprint**
4. **Buscar**: `PopulateCharacterList`
5. **Verificar TODAS as chamadas**:
   - Se houver chamada em `Event OnLoginSuccess` → **REMOVER**
   - Se houver chamada em `Event BeginPlay` → **REMOVER**
   - Se houver chamada em `Event Construct` → **REMOVER** (exceto delegate)
6. **Confirmar**: Apenas `OnCharacterListLoaded_Event` pode chamar `PopulateCharacterList`

### 2. Adicionar Print de Identificação

**No `OnCharacterListLoaded_Event`:**
```
Print String: "OnCharacterListLoaded_Event - Chamando PopulateCharacterList"
  → Delay 0.2
  → Call PopulateCharacterList
```

**Em QUALQUER OUTRO LUGAR que chama `PopulateCharacterList`:**
```
Print String: "CHAMADA INCORRETA DE PopulateCharacterList via [NOME_DO_EVENTO]"
  → Call PopulateCharacterList
```

### 3. Testar

**Executar o jogo e verificar logs:**

```
[Timestamp] OnCharacterListLoaded_Event - Chamando PopulateCharacterList  ← DEVE APARECER
[Timestamp] PopulateCharacterList chamada
[Timestamp] [N]  ← ID correto

NÃO deve aparecer:
[Timestamp] CHAMADA INCORRETA DE PopulateCharacterList via [...]
```

---

## 🎯 Resumo

**Problema**: `PopulateCharacterList` sendo chamado DUAS VEZES
- Uma vez ANTES dos dados chegarem (INCORRETO)
- Uma vez DEPOIS (CORRETO via delegate)

**Causa**: Algum evento no Event Graph está chamando `PopulateCharacterList` prematuramente

**Solução**: 
1. Verificar TODOS os eventos que chamam `PopulateCharacterList`
2. Remover conexões incorretas
3. Manter APENAS a conexão via `OnCharacterListLoaded_Event`

**Como encontrar**: Buscar `PopulateCharacterList` no Blueprint e verificar TODAS as conexões

---

**Documento criado em**: 28 de Outubro de 2025 (00:45)  
**Última atualização**: 28 de Outubro de 2025

