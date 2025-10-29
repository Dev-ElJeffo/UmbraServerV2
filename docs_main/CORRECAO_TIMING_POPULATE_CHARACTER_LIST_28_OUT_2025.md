# Correção: Timing de PopulateCharacterList - 28 de Outubro de 2025

## 🎯 Problema Identificado

### Análise dos Logs

**Primeira Conta (Funcionando ✅)**:
```
[1516] Lista de personagens carregada: 1 personagens
[1517] CurrentPlayers Size After Load: 1
[1518] DEBUG: Lista carregada
[1519] PopulateCharacterList chamada
[1520] 1  ← ID correto
[1521] [WBP_CharacterItem_C_0] 1  ← ID correto
```

**Segunda Conta (Problema ❌)**:
```
[1642] PopulateCharacterList chamada  ← CHAMADO ANTES
[1643] 0  ← Array vazio
[1644] [WBP_CharacterItem_C_1] 0  ← ID errado
[1645] Response do servidor  ← CHEGA DEPOIS
[1652] Parsed Player ID: 10  ← ID correto do servidor
```

---

## 🔍 Causa Raiz

**`PopulateCharacterList` está sendo chamado ANTES de `OnCharacterListLoaded`.**

O Blueprint está chamando `PopulateCharacterList` no evento errado. Provavelmente está conectado a:
- ❌ `OnConstruct` ou `Event BeginPlay`
- ❌ Ou não está aguardando o evento `OnCharacterListLoaded`

**Deve ser conectado a**:
- ✅ `OnCharacterListLoaded` (delegate do GameInstance)

---

## 🛠️ Solução

### Passo 1: Verificar Conexões de Eventos no WBP_CharacterSelection

**Abrir o Blueprint `WBP_CharacterSelection` no Editor**:

#### ❌ INCORRETO (provável situação atual):
```
Event: Event Construct
  → PopulateCharacterList  ← PROBLEMA: Executado antes dos dados chegarem
```

```
Event: Event BeginPlay
  → PopulateCharacterList  ← PROBLEMA: Executado antes dos dados chegarem
```

#### ✅ CORRETO (deve ser):
```
Event: My Game Instance → On Character List Loaded
  → PopulateCharacterList  ← CORRETO: Executado após dados chegarem
```

---

## 📝 Verificar e Corrigir

### 1. Localizar Eventos no Blueprint

No Blueprint `WBP_CharacterSelection`:

1. **Verificar Event Graph**
2. **Localizar conexões de `PopulateCharacterList`**
3. **Verificar se está conectado a `OnCharacterListLoaded`**

### 2. Se não estiver conectado corretamente

**Corrigir a conexão:**

1. **Localizar o evento `MyGameInstance`**
2. **Expandi-lo para ver os delegates disponíveis**
3. **Conectar `On Character List Loaded` → `PopulateCharacterList`**
4. **Remover conexões incorretas** (como `OnConstruct` ou `Event BeginPlay`)

---

## 🎨 Estrutura Correta

### Event Graph do WBP_CharacterSelection

```
┌─────────────────────────────────────────────────┐
│ Event: Construct                                │
│   → Bind MyGameInstance (Get Game Instance)    │
│                                                 │
│ Event: My Game Instance → On Character List    │
│   Loaded                                        │
│   → PopulateCharacterList                      │
│                                                 │
│ Event: My Game Instance → On Login Success     │
│   → Fade In Widget (opcional)                  │
└─────────────────────────────────────────────────┘
```

---

## 🐛 Como Verificar

### Adicionar Prints Temporários

No Blueprint `WBP_CharacterSelection`:

**Event Construct:**
```
Print String: "OnConstruct - PopulateCharacterList NÃO deve ser chamado aqui"
```

**Event OnCharacterListLoaded:**
```
Print String: "OnCharacterListLoaded - PopulateCharacterList DEVE ser chamado aqui"
  → PopulateCharacterList
```

---

## ✅ Resultado Esperado Após Correção

### Sequência Correta nos Logs:

```
[2025.10.28-00.XX.XX:XXX] LogTemp: Carregando lista de personagens...
[2025.10.28-00.XX.XX:XXX] LogTemp: Request enviada
[2025.10.28-00.XX.XX:XXX] LogVaRest: Response (200)...
[2025.10.28-00.XX.XX:XXX] LogTemp: Parsed Player ID: 10
[2025.10.28-00.XX.XX:XXX] LogTemp: Lista carregada: 1 personagens  ← DADOS CHEGARAM
[2025.10.28-00.XX.XX:XXX] LogBlueprintUserMessages: DEBUG: Lista carregada
[2025.10.28-00.XX.XX:XXX] LogBlueprintUserMessages: PopulateCharacterList chamada  ← DEPOIS
[2025.10.28-00.XX.XX:XXX] LogBlueprintUserMessages: 10  ← ID CORRETO
[2025.10.28-00.XX.XX:XXX] LogBlueprintUserMessages: [WBP_CharacterItem_C_X] 10  ← ID CORRETO
```

---

## 🎯 Resumo da Correção

**Problema**: `PopulateCharacterList` sendo chamado antes de `OnCharacterListLoaded`

**Causa**: Conexão de evento incorreta no Blueprint

**Solução**: Conectar `OnCharacterListLoaded` → `PopulateCharacterList` (em vez de `OnConstruct` ou `BeginPlay`)

**Verificação**: Adicionar prints para confirmar ordem de execução

---

**Documento criado em**: 28 de Outubro de 2025 (00:30)  
**Última atualização**: 28 de Outubro de 2025

