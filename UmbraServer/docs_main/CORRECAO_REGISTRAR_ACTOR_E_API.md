# 🔴 CORREÇÃO: Registrar Actor e API InspectPlayer

## 🎯 PROBLEMAS IDENTIFICADOS NOS LOGS

1. **`RegisterRemotePlayerActor` não está sendo chamado:**
   ```
   LogTemp: Warning: [UmbraPlayerSelection] ⚠️ Jogador BP_RemotePlayer_C_0 não está no cache e PlayerID não encontrado no Map.
   ```

2. **API `get_public_info.php` está retornando erro:**
   ```
   LogTemp: Error: [UmbraGameInstance] ❌ Erro ao inspecionar jogador:
   ```

---

## ✅ SOLUÇÃO 1: Adicionar RegisterRemotePlayerActor no Blueprint

### **ONDE ADICIONAR:**

No `BP_NetMovementClient`, na função `ProcessNextFrame`, **APÓS** spawnar o actor:

```
[ProcessNextFrame]
    │
    ├──► [ParseStateUpdateFrame] → OutPlayerId, OutLocation, OutYawDegrees
    │
    ├──► [Array_Find (RemoteActorIds, OutPlayerId)] → FoundIndex
    │
    ├──► [Branch: FoundIndex >= 0?]
    │       │
    │       ├──► True: [Atualiza actor existente]
    │       │
    │       └──► False: [Novo actor - SPAWN]
    │               │
    │               ├──► [SpawnActorFromClass] → SpawnedActor
    │               ├──► [Set Variable: RemoteActorRef = SpawnedActor]
    │               ├──► [Array_Add (RemoteActorIds, OutPlayerId)]
    │               ├──► [Array_Add (RemoteActors, SpawnedActor)]
    │               │
    │               └──► ⭐ [AQUI ADICIONE O CÓDIGO DE REGISTRO]
    │                       │
    │                       ├──► [Get Game Instance]
    │                       ├──► [Cast to UmbraGameInstance]
    │                       ├──► [Register Remote Player Actor]
    │                       │       PlayerID: OutPlayerId
    │                       │       RemoteActor: SpawnedActor
    │                       │
    │                       └──► [Inspect Player]
    │                               Target Player ID: OutPlayerId
```

### **PASSO A PASSO:**

1. **Após `Array_Add (RemoteActors, SpawnedActor)`, adicione:**

   ```
   [Get Game Instance]
       ↓
   [Cast to UmbraGameInstance]
       ↓
   [Register Remote Player Actor]
       PlayerID: (OutPlayerId do ParseStateUpdateFrame)
       RemoteActor: (SpawnedActor)
       ↓
   [Inspect Player]
       Target Player ID: (OutPlayerId do ParseStateUpdateFrame)
   ```

2. **Conecte os pinos:**
   - `PlayerID` → `OutPlayerId` (do `ParseStateUpdateFrame`)
   - `RemoteActor` → `SpawnedActor` (do `SpawnActorFromClass`)

3. **Salve e compile o Blueprint**

---

## ✅ SOLUÇÃO 2: Verificar Erro da API

### **O que foi corrigido no C++:**

1. ✅ `CreateRequest` agora suporta GET corretamente
2. ✅ Token é enviado apenas no header (não no body para GET)
3. ✅ Logs de erro melhorados para mostrar detalhes completos

### **Verificar no Output Log:**

Após compilar e testar, você verá logs mais detalhados:

```
[UmbraGameInstance] 🔍 Inspecionando jogador 1...
[UmbraGameInstance] 🌐 Request criada: GET http://localhost/umbra_api/api/character/get_public_info.php?player_id=1
[UmbraGameInstance] 📤 Executando requisição InspectPlayer para PlayerID 1
```

**Se der erro, você verá:**
```
[UmbraGameInstance] ❌ OnInspectPlayerFail: Requisição falhou!
[UmbraGameInstance] ❌ Erro em InspectPlayer:
   Status Code: XXX
   Response: {...}
[UmbraGameInstance] ❌ InspectPlayer falhou - Status: XXX, Mensagem: ...
```

### **Possíveis causas do erro da API:**

1. **Token inválido/expirado:**
   - Verifique se está logado corretamente
   - Verifique se o token está sendo enviado no header

2. **PlayerID não existe no banco:**
   - Verifique se o PlayerID existe na tabela `players`

3. **Erro no banco de dados:**
   - Verifique a conexão com o MySQL
   - Verifique se as tabelas existem

4. **Endpoint não acessível:**
   - Verifique se o servidor web está rodando
   - Verifique se a URL está correta (`http://localhost/umbra_api/...`)

---

## 📋 CHECKLIST

- [ ] Adicionar `Register Remote Player Actor` no `ProcessNextFrame` após spawnar actor
- [ ] Adicionar `Inspect Player` logo após `Register Remote Player Actor`
- [ ] Compilar o projeto C++
- [ ] Compilar o Blueprint `BP_NetMovementClient`
- [ ] Testar e verificar logs no Output Log
- [ ] Verificar se `RegisterRemotePlayerActor` está sendo chamado (deve aparecer log)
- [ ] Verificar se a API está retornando sucesso (deve aparecer log com dados)

---

## 🔍 LOGS ESPERADOS APÓS CORREÇÃO

**Quando spawnar um novo remote actor:**
```
[UmbraGameInstance] ✅ Actor remoto registrado: PlayerID 1, Actor: BP_RemotePlayer_C_1
[UmbraGameInstance] 🔍 Inspecionando jogador 1...
[UmbraGameInstance] 🌐 Request criada: GET http://localhost/umbra_api/api/character/get_public_info.php?player_id=1
[UmbraGameInstance] 📤 Executando requisição InspectPlayer para PlayerID 1
```

**Quando a API retornar com sucesso:**
```
[UmbraGameInstance] 📥 Dados recebidos da API - PlayerID: 1, Nome: NomeDoPlayer, HP: 100/100, MP: 50/50, Actor: BP_RemotePlayer_C_1
[UmbraGameInstance] ✅ HandlePlayerInspectedInternal: Jogador registrado automaticamente! PlayerID: 1, Nome: NomeDoPlayer
[UmbraPlayerSelection] 🔍 UpdateRemotePlayerInfo: Jogador selecionado detectado! PlayerID: 1
[UmbraPlayerSelection] 📊 Dados recebidos - Nome: 'NomeDoPlayer', HP: 100/100, MP: 50/50
[UmbraPlayerSelection] ✅ Dados REAIS confirmados! Atualizando SelectedPlayer e fazendo broadcast...
[UmbraPlayerSelection] 🔄 Jogador selecionado atualizado: NomeDoPlayer (ID: 1, HP: 100/100, MP: 50/50)
[UmbraPlayerSelection] 📡 Broadcast OnSelectedPlayerInfoUpdated DISPARADO!
```

---

**Após implementar essas correções, o sistema deve funcionar corretamente!**
