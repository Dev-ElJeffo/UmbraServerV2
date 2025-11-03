# 🔍 **ANÁLISE: Log e XML - PlayerID Corrompido**

## 📋 **PROBLEMA IDENTIFICADO:**

Nos logs, vemos:
```
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] X=-320.0Y=550.0Z=92.0Yaw=0.0PlayerID=1  ✅ CORRETO
LogBlueprintUserMessages: [BP_NetMovementClient_C_1] X=-320.0Y=550.0Z=92.0Yaw=0.0PlayerID=-1610612668  ❌ CORRUPTO!
```

**Análise:**
- ✅ **Primeiro frame**: Valores corretos (`X=-320.0, Y=550.0, Z=92.0, PlayerID=1`)
- ❌ **Segundo frame**: **PlayerID corrompido** (`-1610612668`), mas posição correta

**Observação importante:** O `Print String` está sendo executado **DEPOIS** da validação do `ReturnValue`, o que significa que o frame foi considerado válido (`ReturnValue = true`). O problema é que o **PlayerID está sendo lido incorretamente**, mesmo que os outros campos (X, Y, Z, Yaw) estejam corretos.

---

## 🔍 **CAUSA RAIZ PROVÁVEL:**

### **Hipótese 1: Frame Parcial ou Buffer Mal Alinhado**

O `ProcessBinaryBuffer` pode estar extraindo um frame que **não está alinhado corretamente** com o início dos dados. Se os dados estão desalinhados por alguns bytes, a leitura do `PlayerID` (que está no offset 5 do frame) pode ler bytes do final do frame anterior ou do início do próximo frame.

**Estrutura do StateUpdateFrame (29 bytes):**
```
Offset | Tipo    | Valor
-------|---------|------------------
0      | uint8   | Type = 2
1-4    | uint32  | PlayerID (little-endian)
5-8    | float   | X
9-12   | float   | Y
13-16  | float   | Z
17-20  | float   | Yaw
21-24  | uint32  | Timestamp (little-endian)
```

Se o frame estiver desalinhado (por exemplo, começando no offset 1 em vez de 0), a leitura ficaria:
```
Offset | Esperado | Lido
-------|----------|------
1      | Type=2   | PlayerID byte 0
2-5    | PlayerID | PlayerID bytes 1-3 + X byte 0
...
```

### **Hipótese 2: Conversão uint32 → int32 Incorreta**

O valor `-1610612668` em hexadecimal é `0xA0000004` (aproximadamente). Se isso for lido como um `uint32` e depois convertido para `int32` usando `static_cast<int32>(PlayerIdU32)`, pode resultar em um valor negativo se o bit mais significativo estiver setado.

**Análise do valor:**
- `-1610612668` (decimal) = `0xA0000004` (hexadecimal)
- Se lido como `uint32`: `2684354564`
- Se lido como `int32`: `-1610612668` (bit 31 = 1 = sinal negativo)

---

## ✅ **SOLUÇÕES:**

### **SOLUÇÃO 1: Verificar Alinhamento do Buffer**

O problema pode estar no `ProcessBinaryBuffer` que não garante que o frame comece exatamente no offset 0. Vamos adicionar uma verificação adicional no `ParseStateUpdateFrame` para garantir que o primeiro byte seja exatamente `2`:

**Não é necessário modificar C++**, mas podemos adicionar uma verificação no Blueprint:

1. **Após `ParseStateUpdateFrame`**, antes de usar `OutPlayerId`:
   - Adicionar um `Branch` que verifica se `OutPlayerId > 0` e `OutPlayerId < 1000000` (ou um valor máximo razoável)
   - Se o `PlayerID` estiver fora dessa faixa, **rejeitar o frame**

### **SOLUÇÃO 2: Adicionar Validação de PlayerID no Blueprint**

No Blueprint `ProcessNextFrame`, após o `ParseStateUpdateFrame`, adicione uma validação:

```
ParseStateUpdateFrame
  ├─ ReturnValue → Branch (validação principal)
  ├─ OutPlayerId → Branch (validação PlayerID)
  
Branch (ReturnValue = true)
  ├─ then → Branch (OutPlayerId > 0 AND OutPlayerId < 1000000)
  │           ├─ then → Processar frame normalmente
  │           └─ else → Print String "PlayerID inválido!" → NADA (ignorar frame)
  └─ else → Ignorar frame
```

### **SOLUÇÃO 3: Verificar ProcessBinaryBuffer**

O problema pode estar no `ProcessBinaryBuffer` que está extraindo frames de posições incorretas no buffer. Vamos adicionar uma verificação no C++ para garantir que o primeiro byte do frame seja `2`:

**Modificação no C++ (WSBinaryBPFL.cpp):**

```cpp
bool UWSBinaryBPFL::ProcessBinaryBuffer(TArray<uint8>& Buffer, const TArray<uint8>& NewData, FBinaryFrame& OutFrame)
{
    // Adicionar novos dados ao buffer
    Buffer.Append(NewData);
    
    // Constante: tamanho de um StateUpdateFrame = 29 bytes
    constexpr int32 FrameSize = 29;
    
    // Verificar se há um frame completo disponível
    if (Buffer.Num() >= FrameSize)
    {
        // VERIFICAÇÃO ADICIONAL: Garantir que o primeiro byte seja 2 (StateUpdate)
        // Isso garante que estamos começando no início correto de um frame
        while (Buffer.Num() >= FrameSize)
        {
            if (Buffer[0] == 2u) // Tipo StateUpdate
            {
                // Frame válido encontrado!
                break;
            }
            else
            {
                // Frame desalinhado, descartar o primeiro byte e tentar novamente
                Buffer.RemoveAt(0, 1, false);
            }
        }
        
        // Se ainda temos um frame completo após o alinhamento
        if (Buffer.Num() >= FrameSize)
        {
            // Extrair um frame completo
            OutFrame.Data.Reset();
            OutFrame.Data.SetNum(FrameSize);
            for (int32 i = 0; i < FrameSize; ++i)
            {
                OutFrame.Data[i] = Buffer[i];
            }
            
            // Remover frame processado do buffer
            Buffer.RemoveAt(0, FrameSize, false);
            
            return true;
        }
    }
    
    return false;
}
```

---

## 🎯 **PASSO A PASSO PARA CORRIGIR NO BLUEPRINT (SOLUÇÃO RÁPIDA):**

### **PASSO 1: Adicionar Validação de PlayerID**

1. **Localize o nó `ParseStateUpdateFrame`** no Blueprint `ProcessNextFrame`
2. **Encontre o pino `OutPlayerId`** deste nó
3. **Adicione um novo `Branch`** após o `K2Node_IfThenElse_0`:

**Estrutura:**
```
K2Node_IfThenElse_0 (ReturnValue = true)
  └─ then → K2Node_IfThenElse_PlayerID (NOVO)
              ├─ Condition: (OutPlayerId > 0) AND (OutPlayerId < 1000000)
              ├─ then → K2Node_IfThenElse_4 (verifica type == 2)
              └─ else → Print String "PlayerID inválido: [OutPlayerId]" → NADA
```

### **PASSO 2: Configurar o Branch de Validação PlayerID**

1. **Crie um novo `Branch`** (`K2Node_IfThenElse_PlayerID`)
2. **Conecte `OutPlayerId` a um `Integer > Integer`**:
   - `A = OutPlayerId`
   - `B = 0`
3. **Conecte `OutPlayerId` a um `Integer < Integer`**:
   - `A = OutPlayerId`
   - `B = 1000000`
4. **Use `AND (Boolean Boolean)`** para combinar as duas condições
5. **Conecte o resultado ao `Condition` do Branch**

### **PASSO 3: Configurar os Fluxos**

1. **`Branch.then` (True)**: Conecte ao `K2Node_IfThenElse_4` (verificação type == 2) - **fluxo normal**
2. **`Branch.else` (False)**: 
   - Conecte a um `Print String` com mensagem: `"PlayerID inválido ignorado: [OutPlayerId]"`
   - **NÃO conecte** nada após o Print String (o frame será ignorado)

---

## 📊 **ESTRUTURA VISUAL CORRETA:**

```
┌─────────────────────────────────┐
│  ParseStateUpdateFrame          │
│  ┌───────────────────────────┐ │
│  │ then (exec)                │─┼──┐
│  │ ReturnValue (bool)         │─┼──┼──┐
│  │ OutPlayerId (int)           │─┼──┼──┼──┐
│  └───────────────────────────┘ │  │  │  │
└─────────────────────────────────┘  │  │  │
                                      │  │  │
                                      ▼  ▼  │
                              ┌─────────────────┐
                              │ Branch (Return)  │
                              │ (IfThenElse_0)   │
                              │ ┌─────────────┐ │
                              │ │ execute     │◄┘
                              │ │ Condition   │◄┘
                              │ └─────────────┘ │
                              │                 │
                              │  then (True)    │──┐
                              │  else (False)   │──┼──┐ (não conectado)
                              └─────────────────┘  │  │
                                                 │  │
                                                 ▼  │
                                    ┌───────────────────┐
                                    │ Branch (PlayerID) │  ← NOVO!
                                    │ ┌───────────────┐ │
                                    │ │ Condition:    │ │
                                    │ │ OutPlayerId > 0│ │
                                    │ │ AND           │ │
                                    │ │ OutPlayerId < │ │
                                    │ │ 1000000       │ │
                                    │ └───────────────┘ │
                                    │                   │
                                    │  then (True)      │──┐
                                    │  else (False)     │──┼──┐
                                    └───────────────────┘  │  │
                                                         │  │
                                                         ▼  │
                                                ┌─────────────┐
                                                │ Print String│
                                                │ (valores)   │
                                                └─────────────┘
                                                         │
                                                         ▼
                                                [Resto do processamento]
```

---

## 🔍 **VERIFICAÇÕES ADICIONAIS:**

Após implementar a correção, verifique:

1. ✅ **Frames com PlayerID válido** (1-999999): Processados normalmente
2. ✅ **Frames com PlayerID inválido** (<=0 ou >=1000000): Ignorados com log de erro
3. ✅ **Logs limpos**: Apenas frames válidos aparecerão
4. ✅ **Sem PlayerID corrompido**: O valor `-1610612668` não deve mais aparecer

---

## ⚠️ **NOTA IMPORTANTE:**

O problema do PlayerID corrompido pode ser causado por:

1. **Fragmentação de mensagens WebSocket**: Mensagens chegando em pedaços menores que 29 bytes
2. **Buffer mal gerenciado**: O `ProcessBinaryBuffer` não está alinhando corretamente os frames
3. **Múltiplos frames concatenados**: O buffer pode conter múltiplos frames, mas a extração não está começando do byte correto

A **SOLUÇÃO 3 (modificar C++)** é a mais robusta, pois garante que o `ProcessBinaryBuffer` sempre comece a extrair frames do byte correto (onde `Type == 2`).

---

## 📝 **RESUMO:**

1. **Problema**: PlayerID corrompido (`-1610612668`) mesmo com posição correta
2. **Causa provável**: Frame desalinhado no buffer (não começa no offset 0 correto)
3. **Solução rápida**: Adicionar validação de PlayerID no Blueprint
4. **Solução robusta**: Modificar `ProcessBinaryBuffer` no C++ para garantir alinhamento
5. **Teste**: Verificar se frames com PlayerID inválido são rejeitados

---

## 🎯 **TESTE:**

Após implementar:
- ✅ Frames válidos: `X=-320.0 Y=550.0 Z=92.0 PlayerID=1`
- ❌ **NÃO deve mais ver**: `PlayerID=-1610612668` ou qualquer PlayerID negativo/fora de faixa

