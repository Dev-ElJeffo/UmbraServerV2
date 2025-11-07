# 🔧 **GUIA PRÁTICO: Corrigir Speed e VelocityZ Sempre em 0**

## 📋 **OBJETIVO:**

Corrigir o cálculo de `Speed` e `VelocityZ` em `SendMoveUpdate` substituindo `Get Movement Base Actor` por `Cast to Character` → `Get Character Movement` → `Get Velocity`.

---

## 🔍 **DIAGNÓSTICO:**

O problema é que `Get Movement Base Actor` pode retornar `None` ou um objeto que não expõe `Get Velocity` corretamente. A solução é usar `Cast to Character` para garantir que estamos trabalhando com um `Character` que sempre tem `CharacterMovementComponent`.

---

## 📝 **PASSO A PASSO DETALHADO:**

### **PASSO 1: Localizar o Nó `Get Movement Base Actor`**

1. Abra `BP_NetMovementClient` no Blueprint Editor
2. Encontre a função `SendMoveUpdate`
3. Localize o nó `Get Movement Base Actor` (geralmente conectado após `Get Pawn`)

---

### **PASSO 2: Excluir o Nó `Get Movement Base Actor`**

1. Clique no nó `Get Movement Base Actor`
2. Pressione `Delete` ou clique direito → `Delete`
3. **IMPORTANTE:** Anote todas as conexões que estavam conectadas a este nó:
   - Saída `Return Value` estava conectada a algum nó? (provavelmente `Get Velocity`)
   - Anote qual nó estava conectado

---

### **PASSO 3: Adicionar `Cast to Character`**

1. **Posicione o cursor** onde estava o `Get Movement Base Actor` (ou próximo ao `Get Pawn`)

2. **Clique direito** → Busque: `Cast to Character`
   - Digite: `cast to character`
   - Selecione: `Cast to Character` (da categoria `Utilities`)

3. **Conecte o `Return Value` do `Get Pawn` ao pin `Object` do `Cast to Character`:**
   - Arraste do `Return Value` do `Get Pawn`
   - Solte no pin `Object` do `Cast to Character`

4. **Verifique os pins do `Cast to Character`:**
   - **Object:** Conectado ao `Return Value` do `Get Pawn` ✅
   - **Saída de execução `D`:** Fluxo continua aqui se o cast for bem-sucedido
   - **Saída de execução `Cast Failed`:** Fluxo dispara aqui se o cast falhar
   - **As Character:** Saída Character (Character castado, válido apenas se o cast for bem-sucedido)

   **⚠️ IMPORTANTE:** `Cast To Character` **NÃO** tem uma saída booleana `Success`. O sucesso/falha é determinado pelas saídas de execução:
   - Se o cast for bem-sucedido, o fluxo continua pela saída `D` principal
   - Se falhar, o fluxo vai para a saída `Cast Failed`

---

### **PASSO 4: Conectar o Fluxo de Execução**

1. **Conecte a saída de execução `D` do `Cast to Character` ao próximo passo:**
   - Esta saída só é executada se o cast for bem-sucedido
   - Se o cast falhar, o fluxo vai para `Cast Failed` (você pode conectar um log de erro lá se quiser)

   **⚠️ IMPORTANTE:** Não é necessário um `Branch` separado. O próprio `Cast To Character` já funciona como uma ramificação através das saídas de execução.

---

### **PASSO 5: Adicionar `Get Character Movement`**

1. **Adicione um nó `Get Character Movement`:**
   - Clique direito → Busque: `Get Character Movement`
   - Selecione: `Get Character Movement` (função do Character)

2. **Conecte a saída de execução `D` do `Cast to Character` ao pin de execução do `Get Character Movement`**

3. **Conecte o pin `As Character` do `Cast to Character` ao pin `Target` do `Get Character Movement`:**
   - Arraste do `As Character` do `Cast to Character`
   - Solte no pin `Target` do `Get Character Movement`

3. **Verifique os pins:**
   - **Target:** Conectado ao `As Character` do `Cast to Character` ✅
   - **Return Value:** CharacterMovementComponent (este será usado no próximo passo)

---

### **PASSO 6: Adicionar `Get Velocity`**

1. **Adicione um nó `Get Velocity`:**
   - Clique direito → Busque: `Get Velocity`
   - Selecione: `Get Velocity` (função do CharacterMovementComponent)

2. **Conecte a saída de execução do `Get Character Movement` ao pin de execução do `Get Velocity`**

3. **Conecte o pin `Return Value` do `Get Character Movement` ao pin `Target` do `Get Velocity`:**
   - Arraste do `Return Value` do `Get Character Movement`
   - Solte no pin `Target` do `Get Velocity`

3. **Verifique os pins:**
   - **Target:** Conectado ao `Return Value` do `Get Character Movement` ✅
   - **Return Value:** FVector (velocidade completa) ← Este será usado no próximo passo

---

### **PASSO 7: Reconectar `Break Vector`**

1. **Localize o nó `Break Vector` que estava conectado ao `Get Velocity` anterior**

2. **Conecte o pin `Return Value` do novo `Get Velocity` ao pin `Input` do `Break Vector`:**
   - Arraste do `Return Value` do novo `Get Velocity`
   - Solte no pin `Input` do `Break Vector`

3. **Verifique se o `Break Vector` está funcionando:**
   - **Input:** Conectado ao `Return Value` do `Get Velocity` ✅
   - **X, Y, Z:** Saídas Float (devem estar conectadas aos próximos passos)

---

### **PASSO 8: Verificar o Resto do Fluxo**

O restante do fluxo deve permanecer igual:

1. **Speed:**
   - `Make Vector (X, Y, 0)` → Conecte `X` e `Y` do `Break Vector`
   - `Vector Length` → Conecte ao `Return Value` do `Make Vector`
   - Conecte o `Return Value` do `Vector Length` ao pin `Speed` do `BuildMoveUpdateFrameWithAnimation`

2. **VelocityZ:**
   - Conecte o `Z` do `Break Vector` diretamente ao pin `VelocityZ` do `BuildMoveUpdateFrameWithAnimation`

3. **IsInAir:**
   - `Abs` → Conecte o `Z` do `Break Vector` ao `Input` do `Abs`
   - `Greater` → Conecte o `Return Value` do `Abs` ao pin `A` do `Greater`
   - Conecte uma constante `0.1` ao pin `B` do `Greater`
   - Conecte o `Return Value` do `Greater` ao pin `IsInAir` do `BuildMoveUpdateFrameWithAnimation`

---

### **PASSO 9: Adicionar Logs Temporários para Verificação**

Adicione logs temporários para verificar se a velocidade está sendo obtida corretamente:

1. **Após `Get Velocity`:**
   ```
   Break Vector → X, Y, Z
     ↓
   Format Text: "[SendMoveUpdate] Velocity: X={0}, Y={1}, Z={2}"
     ├─ {0}: Break Vector → X
     ├─ {1}: Break Vector → Y
     └─ {2}: Break Vector → Z
     ↓
   Print String
   ```

2. **Após `Vector Length` (Speed):**
   ```
   Vector Length → Return Value
     ↓
   Format Text: "[SendMoveUpdate] Speed: {0}"
     ├─ {0}: Vector Length → Return Value
     ↓
   Print String
   ```

3. **Após `Break Vector Z` (VelocityZ):**
   ```
   Break Vector → Z
     ↓
   Format Text: "[SendMoveUpdate] VelocityZ: {0}"
     ├─ {0}: Break Vector → Z
     ↓
   Print String
   ```

---

### **PASSO 10: Compilar e Testar**

1. **Compile o Blueprint:**
   - Clique no botão "Compile" no topo do Blueprint Editor
   - Verifique se não há erros de compilação

2. **Execute o jogo e mova o personagem:**
   - Pressione Play
   - Mova o personagem usando WASD
   - Pule usando Space

3. **Verifique os logs:**
   - Abra o Output Log (Window → Developer Tools → Output Log)
   - Procure por `[SendMoveUpdate]`
   - **Esperado:**
     - `Velocity: X=..., Y=..., Z=...` com valores diferentes de 0 quando se move
     - `Speed: ...` com valor > 0 quando se move
     - `VelocityZ: ...` com valor ≠ 0 quando pula/cai

4. **Se os valores ainda estiverem em 0:**
   - Verifique se o `Cast to Character` está retornando `Success = True`
   - Verifique se o `Get Character Movement` está retornando um componente válido
   - Verifique se o movimento está sendo feito via `CharacterMovementComponent` (não via `Set Actor Location`)

---

## 📊 **ESTRUTURA FINAL VISUAL:**

```
Get First Player Controller
  ↓
Get Pawn
  ↓
Cast to Character
  ├─ Saída de execução `D` (sucesso) →
  │   ↓
  │   Get Character Movement
  │   ↓
  │   Get Velocity
  │   ↓
  │   Break Vector → X, Y, Z
  │   └─ [Continuar com Speed, VelocityZ, IsInAir]
  │
  └─ Saída de execução `Cast Failed` → [OPCIONAL: Log de erro ou valores padrão]
```

**⚠️ NOTA:** O `Cast To Character` já funciona como uma ramificação através das saídas de execução. Não é necessário um `Branch` separado.

---

## ⚠️ **PROBLEMAS COMUNS:**

### **1. `Cast to Character` falha (fluxo vai para `Cast Failed`)**

**Causa:** O Pawn não é um Character (pode ser um Pawn customizado).

**Solução:** 
- Verifique se o Pawn usado no jogo é do tipo `Character`
- Se não for, você precisa usar outro método para obter a velocidade (ex: `Get Velocity` diretamente do Pawn, se disponível)
- Conecte a saída `Cast Failed` a um log de erro ou use valores padrão (Speed=0, VelocityZ=0, IsInAir=false)

### **2. `Get Character Movement` retorna `None`**

**Causa:** O Character não tem `CharacterMovementComponent` (improvável, mas possível).

**Solução:**
- Adicione um `Is Valid` após `Get Character Movement`
- Se inválido, use valores padrão (Speed=0, VelocityZ=0, IsInAir=false)

### **3. `Get Velocity` retorna `(0, 0, 0)` mesmo quando se move**

**Causa:** O movimento está sendo feito via `Set Actor Location` em vez de `CharacterMovementComponent`.

**Solução:**
- Verifique como o movimento está sendo feito
- Se estiver usando `Set Actor Location`, mude para `Add Movement Input` ou `Set Velocity` diretamente no `CharacterMovementComponent`

---

## ✅ **CHECKLIST FINAL:**

- [ ] `Get Movement Base Actor` foi removido
- [ ] `Cast to Character` foi adicionado após `Get Pawn`
- [ ] Saída de execução `D` do `Cast to Character` está conectada ao próximo passo
- [ ] `Get Character Movement` foi adicionado e conectado ao `As Character` do cast
- [ ] `Get Velocity` foi conectado ao `CharacterMovementComponent`
- [ ] `Break Vector` foi reconectado ao novo `Get Velocity`
- [ ] (Opcional) Saída `Cast Failed` conectada a log de erro ou valores padrão
- [ ] `Speed` está conectado ao `Vector Length` do `Make Vector (X, Y, 0)`
- [ ] `VelocityZ` está conectado ao `Z` do `Break Vector`
- [ ] `IsInAir` está conectado ao `Greater (Abs(VelocityZ) > 0.1)`
- [ ] Logs temporários foram adicionados
- [ ] Blueprint foi compilado sem erros
- [ ] Logs mostram valores corretos quando o personagem se move

---

**Fim do Guia**

