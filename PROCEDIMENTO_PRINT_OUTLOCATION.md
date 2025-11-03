# 🔍 **PROCEDIMENTO DETALHADO: Adicionar Print String para Debug de OutLocation**

## 📋 **OBJETIVO:**
Adicionar `Print String` após `ParseStateUpdateFrame` para verificar se `OutLocation` está vindo como `(0,0,0)` do servidor, o que está causando o erro de spawn.

---

## 🎯 **PASSO A PASSO:**

### **PASSO 1: Localizar o Event `ProcessNextFrame`**
1. Abra o Blueprint `BP_NetMovementClient` no Unreal Editor
2. No painel de **Graph** (lateral esquerda), localize a função `ProcessNextFrame`
3. Clique no nome da função para abrir seu grafo

---

### **PASSO 2: Localizar o nó `ParseStateUpdateFrame`**
1. No grafo do `ProcessNextFrame`, localize o nó `Parse State Update Frame`
2. Este nó tem os seguintes pinos de saída:
   - `OutPlayerId` (Int)
   - `OutLocation` (Vector)
   - `OutYawDegrees` (Float)
   - `OutTimestampMs` (Int)
   - `ReturnValue` (Bool) - indica se o parse foi bem-sucedido

---

### **PASSO 3: Verificar a Conexão Atual**
1. O `ReturnValue` do `ParseStateUpdateFrame` deve estar conectado a um `Branch` (ou `IfThenElse`)
2. O pino `then` (execução) do `ParseStateUpdateFrame` deve estar conectado a algo (provavelmente um `Knot` ou outro nó)

**⚠️ IMPORTANTE:** Não desconecte nenhuma conexão existente! Vamos apenas **adicionar** um `Print String` entre o `ParseStateUpdateFrame` e o próximo nó.

---

### **PASSO 4: Adicionar o Nó `Print String`**

#### **4.1: Criar o nó**
1. Clique com o botão direito no espaço vazio do grafo (logo após o `ParseStateUpdateFrame`)
2. No menu de busca, digite: `Print String`
3. Selecione **`Print String`** da categoria `Utilities > Debug`
4. O nó será criado no grafo

#### **4.2: Conectar a Execução**
1. Localize o pino `then` (saída de execução) do `ParseStateUpdateFrame`
2. **Se já houver uma conexão saindo de `then`:**
   - Clique e arraste o fio de `then` para desconectá-lo temporariamente
   - Conecte `then` (ParseStateUpdateFrame) → `execute` (Print String)
   - Conecte `then` (Print String) → o próximo nó que estava conectado antes
3. **Se NÃO houver conexão:**
   - Conecte `then` (ParseStateUpdateFrame) → `execute` (Print String)

---

### **PASSO 5: Configurar o Texto do `Print String`**

#### **5.1: Criar a String de Debug**
O `Print String` precisa mostrar:
- `OutLocation.X`
- `OutLocation.Y`
- `OutLocation.Z`
- `OutYawDegrees`
- `OutPlayerId`

**Para isso, vamos usar o nó `Append`:**
1. Clique com o botão direito no espaço vazio
2. Busque por: `Append`
3. Selecione **`Append`** (da categoria `String`)
4. Você precisará de **vários** nós `Append` para juntar todas as informações

#### **5.2: Estrutura Completa da String:**
```
"OutLocation: X=" + [OutLocation.X convertido para String] + " Y=" + [OutLocation.Y convertido para String] + " Z=" + [OutLocation.Z convertido para String] + " | Yaw=" + [OutYawDegrees convertido para String] + " | PlayerID=" + [OutPlayerId convertido para String]
```

---

### **PASSO 6: Construir a String Passo a Passo**

#### **6.1: Converter Valores para String**
Para cada valor numérico, você precisa converter para String:
1. Clique com o botão direito → Busque: `Convert Float to String` (para `OutLocation.X`, `OutLocation.Y`, `OutLocation.Z`, `OutYawDegrees`)
2. Clique com o botão direito → Busque: `Convert Integer to String` (para `OutPlayerId`)

#### **6.2: Criar a Primeira Parte da String**
1. Crie um `Append` (será o nó final que conecta tudo)
2. No primeiro `Append`:
   - Pino `A`: Digite manualmente `"OutLocation: X="` (texto literal)
   - Pino `B`: Conecte o `ReturnValue` de `Convert Float to String` (que recebe `OutLocation.X` do `ParseStateUpdateFrame`)

#### **6.3: Adicionar Y**
1. Crie outro `Append`
2. Conecte o `ReturnValue` do primeiro `Append` ao pino `A` do segundo `Append`
3. No pino `B` do segundo `Append`: Digite `" Y="`
4. Crie outro `Append` e conecte o segundo `Append` ao pino `A`, e `OutLocation.Y` (convertido para String) ao pino `B`

#### **6.4: Adicionar Z**
Continue a cadeia:
1. Mais um `Append`:
   - Pino `A`: Conecta do `Append` anterior
   - Pino `B`: `" Z="`
2. Mais um `Append`:
   - Pino `A`: Do anterior
   - Pino `B`: `OutLocation.Z` (convertido para String)

#### **6.5: Adicionar Yaw**
1. Mais um `Append`:
   - Pino `A`: Do anterior
   - Pino `B`: `" | Yaw="`
2. Mais um `Append`:
   - Pino `A`: Do anterior
   - Pino `B`: `OutYawDegrees` (convertido para String)

#### **6.6: Adicionar PlayerID**
1. Mais um `Append`:
   - Pino `A`: Do anterior
   - Pino `B`: `" | PlayerID="`
2. Mais um `Append`:
   - Pino `A`: Do anterior
   - Pino `B`: `OutPlayerId` (convertido para String)

---

### **PASSO 7: Conectar Tudo ao `Print String`**
1. Conecte o `ReturnValue` do **último** `Append` ao pino `InString` do `Print String`
2. Configure o `Print String`:
   - `bPrintToScreen`: `true` ✅
   - `bPrintToLog`: `true` ✅
   - `Duration`: `5.0` (segundos)

---

## 📊 **ESTRUTURA VISUAL ESPERADA:**

```
ParseStateUpdateFrame
  ↓ (then - execução)
Print String (execute)
  ↓ (then - execução) 
[Próximo nó que estava conectado antes]

ParseStateUpdateFrame.OutLocation
  ↓
Break Vector (ou use os pinos X, Y, Z diretamente)
  ↓
Convert Float to String (X)
  ↓
Append ("OutLocation: X=", [X String])
  ↓
Append (..., " Y=")
  ↓
Convert Float to String (Y)
  ↓
Append (..., [Y String])
  ↓
... (Z, Yaw, PlayerID)
  ↓
Print String.InString
```

---

## ⚠️ **MÉTODO ALTERNATIVO MAIS SIMPLES:**

Se a estrutura acima for muito complexa, você pode usar **3 Print Strings separados**:

### **Print String 1: OutLocation**
1. Crie `Break Vector` (conecta `OutLocation`)
2. Crie `Append` três vezes para juntar "X=" + X + " Y=" + Y + " Z=" + Z
3. Conecte ao `InString` do primeiro `Print String`

### **Print String 2: Yaw**
1. Converta `OutYawDegrees` para String
2. Use `Append` para criar "Yaw: " + [Yaw String]
3. Conecte ao segundo `Print String`

### **Print String 3: PlayerID**
1. Converta `OutPlayerId` para String
2. Use `Append` para criar "PlayerID: " + [PlayerID String]
3. Conecte ao terceiro `Print String`

**Conecte todos os três `Print String` na mesma linha de execução (um após o outro).**

---

## 🔍 **VERIFICAÇÃO APÓS IMPLEMENTAÇÃO:**

1. Compile o Blueprint (sem erros)
2. Execute o jogo (PIE)
3. Observe os logs ou a tela
4. Procure por mensagens como:
   - `"OutLocation: X=0.000 Y=0.000 Z=0.000"`
   - `"OutLocation: X=-320.000 Y=550.000 Z=92.000"` (exemplo válido)

---

## ✅ **RESULTADO ESPERADO:**

Se você ver `"OutLocation: X=0.000 Y=0.000 Z=0.000"` nos logs, isso confirma que:
- ❌ O servidor está enviando posição `(0,0,0)` (problema no servidor)
- ✅ O `ParseStateUpdateFrame` está funcionando corretamente (o problema não está no Blueprint)

Se você ver valores diferentes de zero (ex: `X=-320.000 Y=550.000 Z=92.000`), então:
- ✅ O servidor está enviando posição válida
- ⚠️ O problema está na conexão do `Make Transform` ao `SpawnActorFromClass`

---

## 🎯 **PRÓXIMOS PASSOS (DEPENDENDO DO RESULTADO):**

### **Caso 1: `OutLocation = (0,0,0)`**
- O problema está no **servidor C++** (`MovementServer`)
- Verifique como o servidor está enviando a posição inicial
- Verifique o `sendInitialSnapshot` e `sendFullSnapshotToAll`

### **Caso 2: `OutLocation` tem valores válidos**
- O problema está na conexão do `Make Transform` ao `SpawnActorFromClass`
- Verifique se `SpawnTransform` está realmente conectado ao `ReturnValue` do `Make Transform`
- Verifique se `CollisionHandlingOverride` está configurado como `Always Spawn`

---

## 📝 **NOTAS IMPORTANTES:**

1. **Não remova conexões existentes** - apenas adicione o `Print String` na linha de execução
2. **Use `Break Vector`** se o `ParseStateUpdateFrame` não expor os pinos `X`, `Y`, `Z` separadamente
3. **Mantenha a ordem de execução:** `ParseStateUpdateFrame` → `Print String` → [próximo nó]
4. **Teste com um único cliente primeiro** para não poluir os logs com múltiplas mensagens

---

## 🚀 **MÉTODO MAIS SIMPLES (RECOMENDADO):**

### **Usar `ToString` diretamente do Vector:**
1. Após `ParseStateUpdateFrame`, crie um `Print String`
2. Conecte `ParseStateUpdateFrame.then` → `Print String.execute`
3. Conecte `ParseStateUpdateFrame.then` (novamente, se necessário) → próximo nó
4. No `Print String.InString`:
   - Conecte diretamente o `OutLocation` do `ParseStateUpdateFrame`
   - O Unreal Engine automaticamente converterá o Vector para String no formato `(X=..., Y=..., Z=...)`

**Isso é MUITO mais simples!** O Vector já tem um método `ToString` implícito, então você pode conectar diretamente.

---

## 🔧 **COMO SEPARAR X, Y, Z DE UM VECTOR PARA FAZER APPEND:**

### **PASSO 1: Criar o Nó `Break Vector`**
1. Clique com o botão direito no espaço vazio do grafo
2. No menu de busca, digite: `Break Vector`
3. Selecione **`Break Vector`** (categoria `Math > Vector`)
4. O nó terá os seguintes pinos:
   - **Entrada (Input):** `Vector` (conecte aqui o `OutLocation`)
   - **Saídas (Outputs):** `X` (Float), `Y` (Float), `Z` (Float)

### **PASSO 2: Conectar `OutLocation` ao `Break Vector`**
1. Localize o pino `OutLocation` do `ParseStateUpdateFrame`
2. Conecte `OutLocation` → `Vector` (pino de entrada do `Break Vector`)

### **PASSO 3: Converter X, Y, Z para String**
Para cada componente (X, Y, Z), você precisa converter Float para String:

1. **Para X:**
   - Clique com o botão direito → Busque: `To String (Float)`
   - Conecte `Break Vector.X` → `To String (Float).Value`

2. **Para Y:**
   - Crie outro `To String (Float)`
   - Conecte `Break Vector.Y` → `To String (Float).Value`

3. **Para Z:**
   - Crie outro `To String (Float)`
   - Conecte `Break Vector.Z` → `To String (Float).Value`

### **PASSO 4: Criar a String com `Append`**

#### **4.1: Primeira Parte - "OutLocation: X="**
1. Crie um nó `Append` (busque: `Append` na categoria `String`)
2. No pino `A` do `Append`: Digite manualmente o texto `"OutLocation: X="` (sem aspas no Blueprint, apenas digite o texto)
3. No pino `B` do `Append`: Conecte o `ReturnValue` do `To String (Float)` que está conectado ao `Break Vector.X`

#### **4.2: Segunda Parte - " Y="**
1. Crie outro nó `Append`
2. Conecte o `ReturnValue` do primeiro `Append` → pino `A` do segundo `Append`
3. No pino `B` do segundo `Append`: Digite `" Y="`
4. Crie mais um `Append`
5. Conecte o `ReturnValue` do segundo `Append` → pino `A` do terceiro `Append`
6. No pino `B` do terceiro `Append`: Conecte o `ReturnValue` do `To String (Float)` que está conectado ao `Break Vector.Y`

#### **4.3: Terceira Parte - " Z="**
1. Crie outro nó `Append`
2. Conecte o `ReturnValue` do terceiro `Append` → pino `A` do quarto `Append`
3. No pino `B` do quarto `Append`: Digite `" Z="`
4. Crie mais um `Append`
5. Conecte o `ReturnValue` do quarto `Append` → pino `A` do quinto `Append`
6. No pino `B` do quinto `Append`: Conecte o `ReturnValue` do `To String (Float)` que está conectado ao `Break Vector.Z`

### **PASSO 5: Conectar ao `Print String`**
1. Conecte o `ReturnValue` do **último** `Append` (o quinto) → `InString` do `Print String`

---

## 📊 **ESTRUTURA VISUAL COMPLETA:**

```
ParseStateUpdateFrame.OutLocation
  ↓
Break Vector
  ├─ X (Float)
  ├─ Y (Float)
  └─ Z (Float)

Break Vector.X
  ↓
To String (Float)
  ↓ (ReturnValue)
Append ("OutLocation: X=", [X String])
  ↓ (ReturnValue)
Append (..., " Y=")
  ↓ (ReturnValue)
Append (..., [Y String])
  ↓ (ReturnValue)
Append (..., " Z=")
  ↓ (ReturnValue)
Append (..., [Z String])
  ↓ (ReturnValue)
Print String.InString
```

---

## ⚠️ **OBSERVAÇÕES IMPORTANTES:**

1. **Nomes dos Nós:**
   - O nó pode aparecer como `To String (Float)` ou apenas `To String`
   - Se não encontrar, busque por `Convert Float to String`

2. **Ordem dos `Append`:**
   - Cada `Append` junta duas strings
   - O primeiro `Append` recebe o texto inicial e o primeiro valor
   - Os `Append` seguintes recebem o resultado do anterior (pino `A`) e a nova parte (pino `B`)

3. **Espaços e Formatação:**
   - Use espaços nos textos literais: `"OutLocation: X="` (tem espaço antes do X e depois do =)
   - Use `" Y="` (tem espaço antes do Y)

---

## 🎯 **VERSÃO SIMPLIFICADA (SE NÃO QUISER APPEND COMPLEXO):**

Se a estrutura acima for muito trabalhosa, você pode usar **3 Print Strings separados**:

1. **Print String 1:**
   - `InString`: `"OutLocation X: "` + `[Break Vector.X convertido para String]`

2. **Print String 2:**
   - `InString`: `"OutLocation Y: "` + `[Break Vector.Y convertido para String]`

3. **Print String 3:**
   - `InString`: `"OutLocation Z: "` + `[Break Vector.Z convertido para String]`

**Conecte todos os três `Print String` na mesma linha de execução (um após o outro).**

---

### **Para Yaw e PlayerID:**
1. Crie um segundo `Print String` logo após o primeiro
2. Para Yaw: `"Yaw: " + [OutYawDegrees convertido para String]` (use `To String (Float)`)
3. Para PlayerID: `"PlayerID: " + [OutPlayerId convertido para String]` (use `To String (Integer)`)
4. Conecte ambos na mesma linha de execução (um após o outro)
