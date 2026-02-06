# 🔴 CORREÇÃO: API HP/MP e Map de Actors

## 🎯 PROBLEMAS IDENTIFICADOS

### 1. **API retornando HP/MP iguais**
- A API estava usando `$player['current_health']` que não existe no array
- O campo correto é `$player['health']` (alias de `current_health` na query)

### 2. **Actor `BP_RemotePlayer_C_0` não encontrado no Map**
- Quando um actor é destruído e recriado, o Unreal Engine pode reutilizar o mesmo nome
- O Map não estava limpando actors inválidos
- O Map não estava substituindo actors quando o mesmo PlayerID era registrado novamente

---

## ✅ CORREÇÕES APLICADAS

### **1. API `get_public_info.php`**
- ✅ Corrigido `current_health` para usar `$player['health']`
- ✅ Corrigido `current_mana` para usar `$player['mana']`
- ✅ Adicionado log de debug para rastrear cálculo de HP/MP

### **2. `UmbraGameInstance::RegisterRemotePlayerActor`**
- ✅ Agora remove actors inválidos do Map antes de adicionar
- ✅ Detecta e loga quando substitui um actor existente
- ✅ Mostra total de actors no Map após registro

---

## 🔍 LOGS PARA VERIFICAR

### **Quando a API for chamada, verifique no log do PHP:**
```
[get_public_info] PlayerID X: Base HP=150, Level=1 (+20), Vit=18 (+30), Equip (+0) = Total 200
```

### **Quando um actor for registrado, verifique no log do Unreal:**
```
[UmbraGameInstance] ✅ Actor remoto registrado: PlayerID 1, Actor: BP_RemotePlayer_C_0 (Total no Map: 2)
```

### **Se um actor for substituído:**
```
[UmbraGameInstance] 🔄 Substituindo actor remoto: PlayerID 1, Actor antigo: BP_RemotePlayer_C_0, Actor novo: BP_RemotePlayer_C_1
```

---

## 📋 TESTES NECESSÁRIOS

1. **Teste HP/MP:**
   - Selecione um player e verifique se o HP/MP está correto
   - Verifique o log do PHP para ver o cálculo completo
   - O valor deve ser: `Base + (Level * 20) + (Vitality/10 * 30) + Equipamentos`

2. **Teste Map de Actors:**
   - Selecione um player várias vezes
   - Verifique se não aparece mais o erro `BP_RemotePlayer_C_0 não encontrado no Map`
   - Verifique se o total no Map está correto nos logs

3. **Teste com múltiplos players:**
   - Conecte 2+ clients
   - Selecione diferentes players
   - Verifique se todos os actors estão sendo registrados corretamente

---

## 🎯 PRÓXIMOS PASSOS

1. **Compile o projeto C++** com as correções
2. **Teste a seleção de players** e verifique os logs
3. **Verifique o log do PHP** para confirmar que o cálculo de HP/MP está correto
4. **Se o HP/MP ainda estiver incorreto**, verifique:
   - Os valores base no banco de dados (`players.max_health`, `players.max_mana`)
   - Se os bônus de equipamentos estão sendo calculados corretamente
   - Se os atributos (Vitality, Intelligence) estão corretos

---

**Após essas correções, o sistema deve funcionar corretamente!**
