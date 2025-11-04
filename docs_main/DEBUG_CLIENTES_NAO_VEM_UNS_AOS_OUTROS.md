# 🔍 Debug: Clientes Não Vem Uns Aos Outros

## ✅ Status Atual

**Servidor:**
- ✅ Dois clientes conectados (WS client 1 e WS client 2)
- ✅ `StateUpdate` sendo enviados corretamente para ambos
- ✅ Movimentos sendo processados e broadcastados

**Problema:**
- ❌ Clientes não estão visualizando os players remotos na tela

## 🔍 Checklist de Debug (Verificar no Blueprint)

### 1. Cliente Está Recebendo Mensagens Binárias?

**Adicionar Log no `OnWSBinaryMessage`:**

1. Abra o Blueprint `BP_NetMovementClient`
2. No evento `OnWSBinaryMessage`:
   - Adicione um **Print String** logo no início:
     - Texto: `"📥 Received binary message, size: {Data.Num()}"`
     - Na concatenação, use: `Length` do Array `Data` (Get → Length)
   - **Exemplo de concatenação:**
     ```
     "📥 Received binary message, size: " + (String from Integer: Data.Length)
     ```

3. **Teste:**
   - Conecte dois clientes
   - **Se você ver os prints**: Cliente está recebendo mensagens ✅
   - **Se NÃO ver prints**: WebSocket não está enviando mensagens ou evento não está conectado ❌

### 2. ParseStateUpdateFrame Está Funcionando?

**Adicionar Log Após Parse:**

1. Após o nó `ParseStateUpdateFrame`, adicione um **Print String**:
   - **Branch** após o `ParseStateUpdateFrame`
   - **True**: Print → `"✅ Parsed StateUpdate: PlayerID={OutPlayerId}, Pos=({X}, {Y}, {Z}), Yaw={Yaw}"`
   - **False**: Print → `"❌ Failed to parse StateUpdate"`

2. **Como fazer a concatenação:**
   ```
   "✅ Parsed StateUpdate: PlayerID=" + (String from Integer: OutPlayerId) + 
   ", Pos=(" + (String from Float: OutLocation.X) + ", " + 
   (String from Float: OutLocation.Y) + ", " + 
   (String from Float: OutLocation.Z) + "), Yaw=" + 
   (String from Float: OutYawDegrees)
   ```

3. **Teste:**
   - **Se ver prints com dados corretos**: Parse funcionando ✅
   - **Se ver "Failed to parse"**: Formato de dados incorreto ❌

### 3. Verificação de Player ID Está Correta?

**Adicionar Log para Verificar Comparação:**

1. Após obter `ActivePlayerID`, adicione prints:
   ```
   Print: "MyPlayerId: " + (String from Integer: MyPlayerId)
   Print: "Received PlayerId: " + (String from Integer: OutPlayerId)
   ```

2. No Branch `OutPlayerId != MyPlayerId`:
   - **True**: Print → `"✅ É player remoto, processando..."`
   - **False**: Print → `"⏭️ Ignorando (é próprio player)"`

3. **Teste:**
   - **Se sempre mostra "Ignorando"**: Player ID está errado ou igual ❌
   - **Se mostra "É player remoto"**: Comparação OK ✅

### 4. GetOrCreatePlayerState Está Funcionando?

**Adicionar Log:**

1. Após `GetOrCreatePlayerState`, adicione:
   ```
   Print: "📊 GetOrCreatePlayerState: PlayerId=" + (String from Integer: OutPlayerId) + 
          ", Entry.PlayerId=" + (String from Integer: ReturnValue.PlayerId)
   ```

2. **Teste:**
   - Verifique se o PlayerId está correto

### 5. Set Element Está Atualizando o Array?

**Adicionar Log Após Set Element:**

1. Após `Set Element`, adicione:
   ```
   Print: "💾 Set Element: Index=" + (String from Integer: Index do FindPlayerStateIndex) + 
          ", RemoteStates size=" + (String from Integer: Get RemoteStates.Length)
   ```

### 6. Actor Remoto Está Sendo Spawnado?

**Verificação CRÍTICA - Esta é Provavelmente o Problema:**

1. **No Passo 7 (Criar Actor Remoto):**
   - Adicione um **Print String** quando spawnar:
     ```
     "🎭 Spawned remote actor for PlayerID=" + (String from Integer: OutPlayerId) + 
     " at " + (String from Vector: OutLocation)
     ```

2. **No Branch `FoundIndex >= 0`:**
   - **False (vai spawnar)**: Adicione Print → `"🎭 Spawning new remote actor..."`
   - **True (já existe)**: Adicione Print → `"✅ Remote actor already exists"`

3. **Após `Spawn Actor from Class`:**
   - Adicione **IsValid** no `NewActorRef`
   - **True**: Print → `"✅ Actor spawned successfully"`
   - **False**: Print → `"❌ FAILED to spawn actor!"`

4. **Teste:**
   - **Se você NÃO vê "Spawning new remote actor"**: O código nunca chega no spawn ❌
   - **Se vê "FAILED to spawn actor"**: Classe do Actor ou Transform está incorreto ❌
   - **Se vê "Actor spawned successfully"**: Spawn OK, problema pode ser na interpolação ✅

### 7. Event Tick Está Interpolando?

**Verificação no Event Tick:**

1. Adicione um contador de iterações:
   - Crie uma variável `TickCounter` (Integer)
   - No início do Tick: `Set TickCounter = TickCounter + 1`
   - A cada 60 frames (1 segundo): Print → `"🔄 Tick: Interpolating {RemoteStates.Length} remote players"`

2. **Dentro do Loop do Tick:**
   - Após obter `RemoteActorRef`, adicione Print a cada 60 iterações:
     ```
     "📍 Tick: PlayerID=" + (String from Integer: PlayerId) + 
     ", HasStateA=" + (String from Bool: Entry.HasStateA) + 
     ", HasStateB=" + (String from Bool: Entry.HasStateB)
     ```

3. **Teste:**
   - **Se não vê prints**: Tick não está executando ou Array está vazio ❌
   - **Se vê "HasStateA=false, HasStateB=false"**: Estados não estão sendo salvos ❌
   - **Se vê "HasStateA=true, HasStateB=true"**: Estados OK, verificar interpolação ✅

## 🚨 Problemas Comuns e Soluções

### Problema 1: "Nenhum print de OnWSBinaryMessage"

**Causa:** Evento `OnWSBinaryMessage` não está conectado ao delegate `OnRawMessage`

**Solução:**
1. No `BeginPlay` ou `OnWSConnected`:
   - Verifique se há `Bind Event to OnRawMessage` → `OnWSBinaryMessage`
   - Se não há, adicione o bind

### Problema 2: "Parse failed" sempre

**Causa:** Formato de dados incorreto ou tipo errado

**Solução:**
1. Verifique se `ParseStateUpdateFrame` está sendo usado (não `ParseMoveUpdateFrame`)
2. Verifique se o primeiro byte do array é `2` (StateUpdate)
3. Adicione print: `Print: "First byte: " + (String from Integer: Data[0])`

### Problema 3: "Sempre ignorando (é próprio player)"

**Causa:** `MyPlayerId` não está sendo configurado corretamente

**Solução:**
1. No `BeginPlay`:
   - Verifique se `MyPlayerId` está sendo setado com `Get Active Player ID`
   - Adicione Print: `"MyPlayerId set to: " + (String from Integer: MyPlayerId)`
   - Se estiver 0, o problema é na obtenção do ID

### Problema 4: "Actor spawned successfully" mas não aparece

**Causas Possíveis:**

1. **Actor spawnado fora da câmera:**
   - Verifique a posição `OutLocation` no log
   - Se estiver muito longe, pode estar fora da view frustum

2. **Classe do Actor não tem Mesh:**
   - Verifique se a classe usada no `Spawn Actor from Class` tem um componente visual (Skeletal Mesh, Static Mesh)
   - Se não tiver, o Actor existe mas é invisível

3. **Actor spawnado mas Tick não atualiza:**
   - Verifique se o `Event Tick` está interpolando
   - Se o Actor fica na posição inicial e não se move, problema na interpolação

### Problema 5: "HasStateA=false, HasStateB=false" sempre

**Causa:** `Set Element` não está sendo executado ou está usando índice errado

**Solução:**
1. Verifique se `Set Element` está sendo chamado após `UpdatePlayerStateBuffer`
2. Verifique se o `Index` do `FindPlayerStateIndex` é >= 0
3. Adicione Print após `Set Element` para confirmar que foi executado

## 📋 Checklist Completo para Debug

Execute estes prints em ordem e verifique cada um:

```
[ ] OnWSBinaryMessage recebe mensagens?
    → Print: "📥 Received binary message"

[ ] ParseStateUpdateFrame funciona?
    → Print: "✅ Parsed StateUpdate" ou "❌ Failed to parse"

[ ] Player ID está correto?
    → Print: "MyPlayerId: X"
    → Print: "Received PlayerId: Y"
    → Print: "✅ É player remoto" ou "⏭️ Ignorando"

[ ] GetOrCreatePlayerState funciona?
    → Print: "📊 GetOrCreatePlayerState: PlayerId=X"

[ ] UpdatePlayerStateBuffer e Set Element executam?
    → Print: "💾 Set Element: Index=X"

[ ] Actor remoto spawna?
    → Print: "🎭 Spawning new remote actor..."
    → Print: "✅ Actor spawned successfully"

[ ] Event Tick interpola?
    → Print: "🔄 Tick: Interpolating X remote players"
    → Print: "📍 Tick: PlayerID=X, HasStateA=true, HasStateB=true"
```

## 🎯 Próximos Passos

1. **Adicione os prints acima no Blueprint**
2. **Teste com dois clientes**
3. **Verifique os logs no Output Log do Unreal Editor**
4. **Identifique qual etapa está falhando**
5. **Reporte qual print aparece e qual não aparece**

Isso nos ajudará a identificar exatamente onde está o problema!

---

**Data**: 2025-11-01

