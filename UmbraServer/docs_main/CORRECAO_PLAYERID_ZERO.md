# ⚠️ Problema Crítico: Player ID = 0

## 🐛 Problema Identificado

**Sintoma**: Logs do servidor mostram:
```
[DEBUG] Received MoveUpdate from client 1: player_id=0, pos=(...), yaw=...
```

**Causa**: O cliente está enviando `player_id=0` em vez do `ActivePlayerID` do personagem selecionado.

**Impacto**:
- Todos os clientes enviando `player_id=0` → conflito de IDs
- Servidor não consegue distinguir diferentes players
- Personagens não spawnam corretamente

## ✅ Solução

### Verificação no Blueprint

**1. Verificar se `MyPlayerId` está sendo configurado no `BeginPlay`:**

No `BeginPlay` do `BP_NetMovementClient`, deve ter:

```
BeginPlay
  ↓
[Lógica de verificação/obtenção do GameInstance]
  ↓
Get Active Player ID (do UmbraGameInstance)
  ↓
Set MyPlayerId = [Return Value do Get Active Player ID]
```

**2. Verificar se `SendMoveUpdate` está usando `MyPlayerId`:**

No `SendMoveUpdate`, o `BuildMoveUpdateFrame` deve receber:

```
BuildMoveUpdateFrame
  - PlayerId: Get MyPlayerId (variável Integer)
  - Location: ...
  - YawDegrees: ...
  - TimestampMs: ...
```

**NÃO DEVE SER:**
- `PlayerId: 0` (constante)
- `PlayerId: LocalPlayerId` (se não estiver configurado)
- `PlayerId: [vazio ou não conectado]`

### Passos para Corrigir

#### PASSO 1: Verificar Variável `MyPlayerId`

1. Abra o `BP_NetMovementClient` no Editor
2. No painel **"Meu Blueprint"** → **"Variáveis"**
3. Procure pela variável `MyPlayerId`
   - **Tipo**: `Integer`
   - **Valor Padrão**: Deve ser `0` (inicial, será configurado no BeginPlay)

#### PASSO 2: Verificar BeginPlay

1. Abra o gráfico **"EventGraph"** do `BP_NetMovementClient`
2. Procure pelo evento **"Event BeginPlay"**
3. Verifique se há a seguinte sequência:

```
Event BeginPlay
  ↓
Get Game Instance
  ↓
Cast to Umbra Game Instance
  ↓ (Branch True)
Get Active Player ID
  ↓
Set MyPlayerId = [Return Value do Get Active Player ID]
```

**Se NÃO encontrar essa sequência**, adicione conforme a seção **FASE 2: Passo 6** do documento `CORRECAO_CONEXAO_WEBSOCKET_APOS_SELECTCHARACTER.md`.

#### PASSO 3: Verificar SendMoveUpdate

1. Abra a função **"SendMoveUpdate"**
2. Procure pelo nó **"BuildMoveUpdateFrame"**
3. Verifique o input **"PlayerId"**:
   - **DEVE SER**: `Get MyPlayerId` (variável)
   - **NÃO DEVE SER**: Constante `0` ou vazio

**Conexão Correta**:
```
Get MyPlayerId → Return Value (Integer) → BuildMoveUpdateFrame.PlayerId
```

#### PASSO 4: Adicionar Logs de Debug (Opcional)

Para verificar se está configurado corretamente:

1. No `BeginPlay`, após `Set MyPlayerId`:
   - Adicione `Print String`: `"MyPlayerId configurado: [MyPlayerId]"`

2. No `SendMoveUpdate`, antes de `BuildMoveUpdateFrame`:
   - Adicione `Print String`: `"Enviando MoveUpdate com PlayerId: [MyPlayerId]"`

**Logs Esperados**:
- No BeginPlay: `"MyPlayerId configurado: 1"` (ou o ID correto do personagem)
- No SendMoveUpdate: `"Enviando MoveUpdate com PlayerId: 1"` (deve corresponder)

### Problema Comum: ActivePlayerID não está disponível no BeginPlay

**Sintoma**: `Get Active Player ID` retorna `0` ou não está disponível no momento do `BeginPlay`.

**Causa**: O `BeginPlay` pode executar **antes** do personagem ser selecionado.

**Solução Alternativa**: Usar um evento customizado que é chamado após `SelectCharacter`:

1. No `UmbraGameInstance`, após `SelectCharacter`, despache um evento/delegate
2. No `BP_NetMovementClient`, bind ao evento no `BeginPlay`
3. Quando o evento for disparado, configure `MyPlayerId`

**OU** verificar se `ActivePlayerID` está disponível antes de usar:

```
BeginPlay
  ↓
Get Game Instance → Cast to Umbra Game Instance
  ↓
Branch: Get Active Player ID > 0?
  ├─ True → Set MyPlayerId = Get Active Player ID
  └─ False → Print "⚠️ ActivePlayerID ainda não disponível"
             → Retry após delay (Timer ou aguardar evento)
```

### Verificação Final

Após as correções, os logs do servidor devem mostrar:

```
[DEBUG] Received MoveUpdate from client 1: player_id=1, pos=(...), yaw=...  ← ID correto!
[DEBUG] Received MoveUpdate from client 2: player_id=4, pos=(...), yaw=...  ← IDs diferentes!
```

**NÃO deve mais aparecer**:
```
[DEBUG] Received MoveUpdate from client X: player_id=0, ...
```

---

**Status**: ⚠️ **VERIFICAÇÃO NECESSÁRIA NO BLUEPRINT**

**Próximos Passos**:
1. Verificar se `MyPlayerId` está configurado no `BeginPlay`
2. Verificar se `SendMoveUpdate` usa `MyPlayerId` (não constante 0)
3. Adicionar logs de debug se necessário
4. Testar novamente e verificar logs do servidor

