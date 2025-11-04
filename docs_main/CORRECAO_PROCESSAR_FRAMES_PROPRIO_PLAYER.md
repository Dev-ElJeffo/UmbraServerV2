# 🔧 Correção: Cliente Processando Frames do Próprio Player

## 📋 Problema Identificado

Os logs mostram que o cliente está processando frames com `PlayerID=14`, que é o **próprio player**. Isso causa:
- ❌ Múltiplos spawns do mesmo player
- ❌ Personagem não consegue se mover (está sendo atualizado pelos frames próprios)
- ❌ Comportamento estranho no spawn
- ❌ `SpawnActor failed because of collision at the spawn location [X=0.000 Y=0.000 Z=0.000]`

### Logs Evidência Atuais:
```
LogTemp: Warning: [ProcessBinaryBuffer] Frame aceito - PlayerID: 14, Location: (-320.000000, 550.000000, 92.000000)
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] ProcessNextFrame called!
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] X=-320.0Y=550.0Z=92.0Yaw=0.0PlayerID=14  ← PRÓPRIO PLAYER SENDO PROCESSADO!
LogTemp: Warning: [ProcessBinaryBuffer] Frame aceito - PlayerID: 4, Location: (-320.000000, 550.000000, 92.000000)
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] X=-320.0Y=550.0Z=92.0Yaw=90.175003PlayerID=4  ← OUTRO PLAYER (CORRETO!)
LogSpawn: Warning: SpawnActor failed because of collision at the spawn location [X=0.000 Y=0.000 Z=0.000]
```

**Análise**: 
- ✅ O cliente está parseando corretamente `PlayerID=14` (próprio) e `PlayerID=4` (outro player)
- ❌ **O problema**: O cliente está processando frames do próprio player (`PlayerID=14`), causando tentativas de spawnar/atualizar a si mesmo
- ❌ O erro `SpawnActor failed at [X=0.000 Y=0.000 Z=0.000]` indica que algum frame está sendo processado com coordenadas zeradas, possivelmente do próprio player ou de um frame corrompido

## ✅ Solução

### No Blueprint `BP_NetMovementClient`:

No Custom Event `ProcessNextFrame`, **ADICIONAR UMA VERIFICAÇÃO** logo após o `ParseStateUpdateFrame`:

1. **Após `ParseStateUpdateFrame`**, conectar o `ReturnValue` a um `Branch` (já existe)
2. **No lado `True` do Branch**, adicionar **OUTRO `Branch`** que compara:
   - `OutPlayerId != MyPlayerId`
   - Se `OutPlayerId == MyPlayerId` (é o próprio player): **não fazer nada**, apenas continuar para o próximo frame
   - Se `OutPlayerId != MyPlayerId` (é outro player): processar normalmente (spawn/atualizar)

## 📝 Implementação no Blueprint

### Fluxo Corrigido:

```
ProcessNextFrame (Custom Event)
  ↓
Get OutFrame (from ProcessBinaryBuffer)
  ↓
Break BinaryFrame → Get Data
  ↓
ParseStateUpdateFrame
  ↓
[Branch: ParseStateUpdateFrame.ReturnValue?]
  ├─ False → Continue (próximo frame ou parar)
  └─ True → [NOVO: Branch: OutPlayerId != MyPlayerId?]
              ├─ False (OutPlayerId == MyPlayerId) → IGNORAR, continuar para próximo frame
              └─ True (OutPlayerId != MyPlayerId) → Processar:
                  ├─ GetOrCreatePlayerState
                  ├─ SetActorLocation
                  ├─ SetActorRotation
                  └─ etc.
```

### Detalhes da Nova Verificação:

1. **Nó `Branch`** (VERIFICAÇÃO CRÍTICA):
   - **Nome sugerido**: `K2Node_IfThenElse_PlayerIdFilter` (ou similar)
   - **Condition**: `OutPlayerId != MyPlayerId`
   - **Como criar**:
     - Adicione um nó `Not Equal (Integer)`
     - Conecte `OutPlayerId` (do `ParseStateUpdateFrame`) ao primeiro input
     - Conecte `MyPlayerId` (variável do Blueprint) ao segundo input
     - Conecte o output do `Not Equal` ao `Condition` do novo `Branch`
   - **True Pin** (OutPlayerId != MyPlayerId): Processar o frame (spawn/atualizar outro player)
   - **False Pin** (OutPlayerId == MyPlayerId): **IGNORAR**, continuar para o próximo frame

2. **Conexão no Fluxo**:
   ```
   ParseStateUpdateFrame
     ↓ (ReturnValue == true)
   Branch [K2Node_IfThenElse_0] (Parse OK?)
     ↓ (then = true)
   [NOVO] Not Equal (Integer): OutPlayerId != MyPlayerId
     ↓ (output)
   Branch [NOVO: K2Node_IfThenElse_PlayerIdFilter]
     ├─ True (OutPlayerId != MyPlayerId) → VALIDAÇÃO ADICIONAL DE LOCALIZAÇÃO
     │   ├─ [OPCIONAL] Break Vector (OutLocation → X, Y, Z)
     │   ├─ [OPCIONAL] Branch: (X != 0.0 OR Y != 0.0 OR Z != 0.0)
     │   │   ├─ True (Location válida) → TODA A LÓGICA DE SPAWN/ATUALIZAÇÃO
     │   │   │   ├─ GetOrCreatePlayerState
     │   │   │   ├─ SetActorLocation
     │   │   │   ├─ SetActorRotation
     │   │   │   └─ etc.
     │   │   └─ False (Location = 0,0,0) → IGNORAR FRAME (continuar para próximo)
     │   └─ [OU SIMPLESMENTE] → TODA A LÓGICA DE SPAWN/ATUALIZAÇÃO (sem validação extra)
     └─ False (OutPlayerId == MyPlayerId) → IGNORAR E CONTINUAR:
         ├─ Make Array (vazio, 0 elementos)
         ├─ ProcessBinaryBuffer (NewData = array vazio)
         ├─ Branch (ReturnValue?)
         │   ├─ True → Call ProcessNextFrame (recursivo)
         │   └─ False → STOP (não há mais frames)
   ```

   **Nota sobre validação de Location (0,0,0):**
   - O erro `SpawnActor failed at [X=0.000 Y=0.000 Z=0.000]` pode ocorrer se frames com coordenadas zeradas forem processados
   - Recomenda-se adicionar uma validação opcional após `OutPlayerId != MyPlayerId` para verificar se `OutLocation` não é (0,0,0)
   - Isso pode acontecer se o servidor enviar frames com coordenadas zeradas ou se houver corrupção no buffer

3. **Observações Importantes**:
   - ⚠️ **NUNCA** processe frames onde `OutPlayerId == MyPlayerId`
   - ✅ O próprio player só **ENVIA** dados via `SendMoveUpdate`, nunca recebe atualizações de si mesmo
   - ✅ Apenas frames de outros players devem ser processados para spawn/atualização
   - ⚠️ **Validação adicional recomendada**: Verificar se `OutLocation != (0,0,0)` antes de spawnar para evitar o erro `SpawnActor failed at [X=0.000 Y=0.000 Z=0.000]`
     - Isso pode ser feito com um `Break Vector` em `OutLocation` e verificando se pelo menos uma coordenada é diferente de zero
     - Ou simplesmente verificando `OutLocation.Size() > 0.01f` (distância mínima)

## 🎯 Por Que Isso Acontece?

### Cenário 1: Servidor Enviando Frames do Próprio Player (Mais Provável)
- O servidor está fazendo `broadcastBinary` que envia frames para **TODOS** os clientes, incluindo o próprio emissor
- Isso é comum em sistemas de broadcast, mas o cliente precisa filtrar frames próprios

### Cenário 2: Cliente Recebendo Frames de Outros Players com Mesmo ID (Improvável)
- Outro player teria o mesmo PlayerID=14, o que seria um bug crítico no banco de dados
- Os logs mostram que o PlayerID está sendo parseado corretamente, então isso é improvável

## 🔬 Debug Adicional

Se após implementar a verificação o problema persistir:

1. **Verificar `MyPlayerId` no Blueprint**:
   - Adicionar um `Print String` em `BeginPlay` para confirmar que `MyPlayerId` está sendo setado corretamente
   - Verificar se está usando `UmbraGameInstance::GetActivePlayerID()`

2. **Verificar lógica de spawn**:
   - Certificar-se de que `GetOrCreatePlayerState` verifica se o player já existe antes de spawnar
   - O erro `SpawnActor failed because of collision at [X=0.000 Y=0.000 Z=0.000]` indica que pode haver um spawn com coordenadas zero ainda

3. **Verificar servidor**:
   - O servidor pode estar enviando frames do próprio player para o cliente
   - Isso é normal em broadcast, mas o cliente precisa ignorar

## ⚠️ Nota Importante

**O PlayerID está CORRETO agora!** (`PlayerID=14` e `PlayerID=4` estão sendo parseados corretamente). O problema é que o cliente está processando frames do próprio player quando não deveria.

Após implementar a verificação `OutPlayerId != MyPlayerId`, o cliente só processará frames de outros players, e o movimento do próprio player funcionará normalmente (ele só envia dados, não recebe atualizações de si mesmo).

## 🔗 **DOCUMENTAÇÃO RELACIONADA:**

Para uma solução completa que também aborda:
- **Múltiplos spawns do mesmo player remoto** (verificar se actor já existe)
- **Spawn em (0,0,0)** (validação de Location)
- **Processamento repetido do mesmo PlayerID**

Consulte: **`CORRECAO_COMPLETA_SPAWN_DUPLICADO.md`**

## 🎯 **RESUMO DA CORREÇÃO:**

### **O que fazer:**
1. No `ProcessNextFrame`, após validar `ParseStateUpdateFrame.ReturnValue == true`
2. Adicionar um `Not Equal (Integer)` comparando `OutPlayerId` com `MyPlayerId`
3. Adicionar um `Branch` com condition = `OutPlayerId != MyPlayerId`
4. **True** (outro player): Processar normalmente (spawn/atualizar)
5. **False** (próprio player): Ignorar e continuar para o próximo frame

### **Resultado esperado:**
- ✅ Apenas frames de outros players serão processados
- ✅ O próprio player não será spawnado/atualizado por frames recebidos
- ✅ O movimento do próprio player funcionará normalmente (controlado localmente)
- ✅ Múltiplos players aparecerão corretamente no mundo

