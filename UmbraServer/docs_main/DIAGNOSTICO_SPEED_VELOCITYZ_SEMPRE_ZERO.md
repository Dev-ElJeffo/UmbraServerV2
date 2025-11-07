# 🔍 **DIAGNÓSTICO: Speed e VelocityZ Sempre em 0**

## 📋 **PROBLEMA:**

`Speed` e `VelocityZ` estão sempre em **0**, mesmo quando o personagem está se movendo.

**Logs observados:**
- `Speed: 0` sempre
- `VelocityZ: 0` sempre
- `IsInAir: false` sempre (mesmo quando pulando)

---

## 🔍 **CAUSA PROVÁVEL:**

O problema está no método usado para obter a velocidade do player local em `SendMoveUpdate`. 

**Método atual (do guia):**
```
Get First Player Controller → Get Pawn → Get Movement Base Actor → Get Velocity
```

**Problema:** `Get Movement Base Actor` pode retornar `None` ou um objeto que não expõe `Get Velocity` corretamente, especialmente se o Pawn não está caindo/pulando sobre outro objeto.

---

## ✅ **SOLUÇÃO: Usar `Cast to Character` → `Get Character Movement` → `Get Velocity`**

### **POR QUE FUNCIONA:**

1. **`Cast to Character`**: Garante que estamos trabalhando com um `Character` (que sempre tem `CharacterMovementComponent`)
2. **`Get Character Movement`**: Obtém diretamente o `CharacterMovementComponent`, que sempre expõe `Get Velocity` corretamente
3. **`Get Velocity`**: Retorna a velocidade atual do Character, mesmo quando parado ou movendo

**Fluxo correto:**
```
Get First Player Controller → Get Pawn → Cast to Character → Get Character Movement → Get Velocity
```

---

## 🔧 **CORREÇÃO PASSO A PASSO:**

### **ETAPA 1: Substituir `Get Movement Base Actor` por `Cast to Character`**

**ANTES (não funciona):**
```
Get First Player Controller
  ↓
Get Pawn
  ↓
Get Movement Base Actor  ← Pode retornar None ou objeto sem Velocity
  ↓
Get Velocity
```

**DEPOIS (funciona):**
```
Get First Player Controller
  ↓
Get Pawn
  ↓
Cast to Character  ← Garante que é um Character
  ↓ (Saída de execução `D` = sucesso)
Get Character Movement  ← Obtém CharacterMovementComponent diretamente
  ↓
Get Velocity
```

**⚠️ IMPORTANTE:** `Cast To Character` **NÃO** tem uma saída booleana `Success`. O fluxo funciona através das saídas de execução:
- Se o cast for bem-sucedido, o fluxo continua pela saída `D` principal
- Se falhar, o fluxo vai para a saída `Cast Failed`

---

### **ETAPA 2: Implementação Detalhada no Blueprint**

1. **Localize o nó `Get Movement Base Actor` em `SendMoveUpdate`**

2. **Delete o nó `Get Movement Base Actor`**

3. **Adicione `Cast to Character`:**
   - Clique direito → Busque: `Cast to Character`
   - Selecione: `Cast to Character`
   - **Object:** Conecte ao `Return Value` do `Get Pawn`
   - **Saída de execução `D`:** Fluxo continua aqui se o cast for bem-sucedido
   - **Saída de execução `Cast Failed`:** Fluxo dispara aqui se o cast falhar
   - **As Character:** Retorna o Character castado (válido apenas se o cast for bem-sucedido)

   **⚠️ IMPORTANTE:** `Cast To Character` **NÃO** tem uma saída booleana `Success`. O sucesso/falha é determinado pelas saídas de execução.

4. **Conecte a saída de execução `D` do `Cast to Character` ao `Get Character Movement`:**
   - Esta saída só é executada se o cast for bem-sucedido
   - Conecte o pin de execução `D` do `Cast` ao pin de execução do `Get Character Movement`
   
5. **Adicione `Get Character Movement`:**
   - Clique direito → Busque: `Get Character Movement`
   - Selecione: `Get Character Movement` (função do Character)
   - **Target:** Conecte ao `As Character` do `Cast to Character`
   - **Return Value:** CharacterMovementComponent

6. **Adicione `Get Velocity`:**
   - Clique direito → Busque: `Get Velocity`
   - Selecione: `Get Velocity` (função do CharacterMovementComponent)
   - **Fluxo de execução:** Conecte a saída de execução do `Get Character Movement` ao pin de execução do `Get Velocity`
   - **Target:** Conecte ao `Return Value` do `Get Character Movement`
   - **Return Value:** FVector (velocidade completa)

7. **Continue com o resto do fluxo:**
   - `Break Vector` → X, Y, Z
   - `Make Vector (X, Y, 0)` → `Vector Length` → Speed
   - `Z` → VelocityZ
   - `Abs (VelocityZ)` → `Greater (0.1)` → IsInAir

---

### **ETAPA 3: Verificação com Logs**

Adicione logs temporários para verificar se a velocidade está sendo obtida corretamente:

1. **Após `Get Velocity`:**
   ```
   Format Text: "[SendMoveUpdate] Get Velocity: X={0}, Y={1}, Z={2}"
     ├─ {0}: Break Vector → X
     ├─ {1}: Break Vector → Y
     └─ {2}: Break Vector → Z
     ↓
   Print String
   ```

2. **Após `Vector Length` (Speed):**
   ```
   Format Text: "[SendMoveUpdate] Speed calculado: {0}"
     ├─ {0}: Vector Length → Return Value
     ↓
   Print String
   ```

3. **Após `Break Vector Z` (VelocityZ):**
   ```
   Format Text: "[SendMoveUpdate] VelocityZ: {0}"
     ├─ {0}: Break Vector → Z
     ↓
   Print String
   ```

**Execute e verifique os logs:**
- Se `Get Velocity` retorna `(0, 0, 0)` mesmo quando se move → Problema no Character Movement Component
- Se `Get Velocity` retorna valores válidos mas `Speed` ainda é 0 → Problema no cálculo de `Vector Length`
- Se `Speed` e `VelocityZ` estão corretos → Problema resolvido!

---

## 📊 **FLUXO CORRETO COMPLETO:**

```
SendMoveUpdate (Custom Event)
  ↓
[Lógica existente: Location, Yaw, Timestamp]
  ↓
Get First Player Controller
  ↓
Get Pawn
  ↓
Cast to Character
  ↓ (Branch True)
Get Character Movement
  ↓
Get Velocity
  ↓
Break Vector → X, Y, Z
  ↓
[PARALELO:]
  ├─ Make Vector (X, Y, 0) → Vector Length → Speed
  ├─ Z → VelocityZ
  └─ Abs (VelocityZ) → Greater (0.1) → IsInAir
  ↓
BuildMoveUpdateFrameWithAnimation
  ├─ PlayerId: [Valor existente]
  ├─ Location: [Valor existente]
  ├─ YawDegrees: [Valor existente]
  ├─ Speed: [Do Vector Length] ← Deve ser > 0 quando movendo
  ├─ VelocityZ: [Do Break Vector Z] ← Deve ser ≠ 0 quando pulando/caindo
  ├─ IsInAir: [Do Greater (Abs(VelocityZ) > 0.1)] ← Deve ser true quando no ar
  └─ TimestampMs: [Valor existente]
  ↓
Send Bytes (WebSocket)
```

---

## ⚠️ **NOTAS IMPORTANTES:**

1. **`Cast to Character` falha (fluxo vai para `Cast Failed`):**
   - Se o Pawn não for um Character, o fluxo vai para a saída `Cast Failed`
   - Nesse caso, conecte a saída `Cast Failed` a valores padrão (Speed=0, VelocityZ=0, IsInAir=false)

2. **`Get Character Movement` pode retornar `None`:**
   - Adicione um `Is Valid` após `Get Character Movement`
   - Se inválido, use valores padrão

3. **Velocidade pode ser 0 mesmo quando movendo:**
   - Isso pode acontecer se o Character está sendo teleportado (Set Actor Location) em vez de se mover naturalmente
   - Verifique se o movimento está sendo feito via `CharacterMovementComponent` (Add Movement Input, etc.)

---

## ✅ **CHECKLIST DE VERIFICAÇÃO:**

- [ ] `Get Movement Base Actor` foi removido
- [ ] `Cast to Character` foi adicionado após `Get Pawn`
- [ ] `Branch` foi adicionado para verificar `Success` do Cast
- [ ] `Get Character Movement` foi adicionado no caminho `True` do Branch
- [ ] `Get Velocity` foi conectado ao `CharacterMovementComponent`
- [ ] Logs temporários foram adicionados para verificar valores
- [ ] `Speed` e `VelocityZ` estão sendo calculados corretamente quando o personagem se move
- [ ] `IsInAir` está correto quando o personagem pula/cai

---

## 🔄 **ALTERNATIVA (SE `Cast to Character` FALHAR):**

Se o Pawn não for um Character, use `Get Velocity` diretamente do Pawn:

```
Get First Player Controller
  ↓
Get Pawn
  ↓
Get Velocity (função do Pawn) ← Tentar isso primeiro
  ↓
Break Vector → X, Y, Z
```

**Nota:** Nem todos os Pawns têm `Get Velocity` diretamente. Se não estiver disponível, você precisará usar `Cast to Character`.

---

**Fim do Documento**

