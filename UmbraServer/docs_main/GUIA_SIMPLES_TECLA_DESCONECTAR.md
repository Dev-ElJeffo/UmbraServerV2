# 🔧 **GUIA SIMPLES: Tecla para Desconectar WebSocket (Mais Rápido)**

## 🎯 **OBJETIVO:**

Adicionar uma tecla de atalho (ex: `F9`) que desconecta o WebSocket, sem precisar criar widgets.

---

## ⚡ **SOLUÇÃO: Input Action no Blueprint**

### **PASSO 1: Adicionar Input Action (Opcional)**

**Se você já tem Input Actions configuradas:**

1. **Edit → Project Settings → Input → Action Mappings**
2. **Adicione:**
   - **Action Name:** `DisconnectWebSocket`
   - **Key:** `F9` (ou qualquer tecla)

**OU use diretamente no Blueprint (sem Input Action):**

### **PASSO 2: Adicionar Lógica no BP_NetMovementClient**

**No `BP_NetMovementClient`, Event Graph:**

**Adicione um Custom Event:**

```
[Custom Event: DisconnectWebSocket]
  ↓
[Print String: "🔴 Desconectando WebSocket..."]
  ↓
[Branch: Is Valid (WebSocketRef)?]
  ├─ True:
  │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   └─ [Print String: "✅ WebSocket fechado!"]
  └─ False:
      └─ [Print String: "⚠️ WebSocketRef inválido!"]
```

**Conecte ao Input (Event Tick ou Input Action):**

**OPÇÃO A: Usar Input Action (Recomendado):**

```
[Input Action: DisconnectWebSocket] (ou qualquer tecla)
  ↓
[Custom Event: DisconnectWebSocket]
```

**OPÇÃO B: Usar Event Tick (Verificar tecla pressionada):**

```
[Event Tick]
  ↓
[Get Player Controller]
  ↓
[Was Input Key Just Pressed] (Key: F9)
  ↓
[Branch: Key Pressed?]
  ├─ True:
  │   └─ [Custom Event: DisconnectWebSocket]
  └─ False:
      └─ (nada)
```

**OPÇÃO C: Usar Console Command (Mais Simples):**

Adicione uma função C++ que pode ser chamada via console.

---

## 🛠️ **IMPLEMENTAÇÃO: Console Command (Mais Simples)**

### **PASSO 1: Adicionar Função em UmbraGameInstance**

**No arquivo `UmbraGameInstance.h`:**

```cpp
// Adicione na seção pública:
UFUNCTION(Exec, Category="Umbra|Net|WS")
void DisconnectWebSocket();
```

**No arquivo `UmbraGameInstance.cpp`:**

```cpp
// Adicione a implementação:
void UUmbraGameInstance::DisconnectWebSocket()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ [DisconnectWebSocket] World inválido!"));
        return;
    }

    // Buscar BP_NetMovementClient no nível
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    // Filtrar por nome da classe (já que não temos acesso direto ao tipo)
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("BP_NetMovementClient")))
        {
            // Tentar acessar WebSocketRef via Blueprint
            // Como não temos acesso direto, vamos usar uma abordagem diferente
            UE_LOG(LogTemp, Warning, TEXT("🔴 [DisconnectWebSocket] Encontrado BP_NetMovementClient: %s"), *Actor->GetName());
            
            // Chamar função Blueprint se disponível
            // Você precisará criar uma função pública no BP_NetMovementClient
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("⚠️ [DisconnectWebSocket] Use a função Blueprint diretamente!"));
}
```

**NOTA:** A abordagem C++ é mais complexa. Recomendamos usar a **OPÇÃO A ou B** (Input Action ou Event Tick).

---

## 🛠️ **IMPLEMENTAÇÃO: Função Pública no BP_NetMovementClient (Recomendado)**

### **PASSO 1: Criar Função Pública**

**No `BP_NetMovementClient`, crie uma nova função:**

**Function Name:** `DisconnectWebSocket`

**Function Body:**

```
[Function Entry: DisconnectWebSocket]
  ↓
[Print String: "🔴 [DisconnectWebSocket] Desconectando WebSocket..."]
  ↓
[Branch: Is Valid (WebSocketRef)?]
  ├─ True:
  │   ├─ [Call Function: Close] (Target: WebSocketRef)
  │   └─ [Print String: "✅ [DisconnectWebSocket] WebSocket fechado!"]
  └─ False:
      └─ [Print String: "⚠️ [DisconnectWebSocket] WebSocketRef inválido!"]
```

**Configurar Função:**
- **Access:** Public
- **Call In Editor:** FALSE
- **Pure:** FALSE

### **PASSO 2: Chamar Função via Input**

**No `BP_NetMovementClient`, Event Graph:**

**Adicione Input Action ou Event Tick:**

```
[Input Action: DisconnectWebSocket] (ou Event Tick com Was Input Key Just Pressed)
  ↓
[Call Function: DisconnectWebSocket] (Self)
```

---

## 🧪 **TESTE RÁPIDO:**

1. **Compile e execute o projeto**
2. **Inicie 2 clientes PIE**
3. **No Cliente 1:**
   - Pressione `F9` (ou a tecla configurada)
   - Verifique os logs: `"🔴 [DisconnectWebSocket] Desconectando WebSocket..."`
   - Verifique os logs: `"✅ [DisconnectWebSocket] WebSocket fechado!"`
4. **No Cliente 2:**
   - O actor remoto do Cliente 1 deve desaparecer
   - Verifique os logs: `"PlayerDisconnected processado"`

---

## ✅ **CHECKLIST MÍNIMO:**

- [ ] Função `DisconnectWebSocket` criada no `BP_NetMovementClient`
- [ ] Função chama `Close()` no `WebSocketRef`
- [ ] Input Action configurada (ou Event Tick com verificação de tecla)
- [ ] Input Action conectada à função `DisconnectWebSocket`
- [ ] Testado: Pressionar tecla desconecta o WebSocket
- [ ] Testado: Actors remotos são destruídos em outros clientes

---

## 📝 **NOTAS:**

- **Esta solução é mais simples** que criar um widget, mas requer configurar Input Actions.
- **O `OnWSClosed` deve estar conectado ao `CleanupRemoteActors`** no `BP_NetMovementClient`.
- **O servidor deve estar enviando `PlayerDisconnected`** quando um cliente desconecta.

---

## 🔄 **ALTERNATIVA: Usar Widget (Mais Visual)**

Se preferir um botão visual, use o guia `GUIA_RAPIDO_BOTAO_DESCONECTAR.md`.

---

**✅ Guia simples para adicionar tecla de desconexão!**


