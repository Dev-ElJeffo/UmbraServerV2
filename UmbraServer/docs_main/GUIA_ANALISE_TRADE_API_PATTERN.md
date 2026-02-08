# Análise: Padrão de APIs e Trade

Este documento analisa como as funções que comunicam com APIs funcionam no projeto e como o sistema de Trade deve seguir o **mesmo método**, sem que o Blueprint chame VaRest diretamente.

---

## 1. Padrão das APIs no UmbraGameInstance

Todas as funções que usam APIs seguem o mesmo fluxo:

### 1.1 Fluxo C++ (interno)

1. **CreateRequest(Endpoint, Verb)** – cria `UVaRestRequestJSON` via `VaRest->ConstructVaRestRequestExt`
2. **Request->GetRequestObject()** – obtém/cria o corpo JSON
3. **SetStringField / SetNumberField / SetBoolField** – preenche o corpo
4. **Request->OnRequestComplete.AddDynamic(this, &Callback)**
5. **Request->OnRequestFail.AddDynamic(this, &FailCallback)**
6. **Request->ExecuteProcessRequest()**

### 1.2 Callback de sucesso

- Usa **Request->GetResponseObject()** para obter `UVaRestJsonObject*`
- Faz o **parse em C++** (GetArrayField, GetObjectField, GetIntegerField, etc.)
- **Armazena** em membros (ex: `CurrentInventory`) ou **broadcast** com struct (ex: `OnCharacterInfoLoaded.Broadcast(CharacterInfo)`)

### 1.3 Blueprint – NUNCA chama VaRest

O Blueprint:
- **Get Game Instance** → **Cast to UmbraGameInstance**
- Chama funções: `LoadInventory`, `AddItem`, `GetInventorySlotByIndex`, etc.
- Usa delegates: `OnInventoryLoaded`, `OnCharacterInfoLoaded`, etc.
- Os delegates passam **structs** ou **tipos simples** (nunca JSON em string)

---

## 2. Exemplos de funções analisadas (sem modificação)

| Função | Parse em C++ | O que passa ao Blueprint |
|--------|--------------|---------------------------|
| LoadInventory | Sim – `ParseInventorySlot`, `CurrentInventory` | `OnInventoryLoaded` (sem parâmetro). Dados em `CurrentInventory` via `GetInventorySlotByIndex` |
| LoadCharacterInfo | Sim – `FUmbraCharacterInfo` | `OnCharacterInfoLoaded.Broadcast(CharacterInfo)` |
| LoadGold | Sim – integers | `OnGoldLoaded.Broadcast(PlayerGold, StoredGold)` |
| AddItem | Sim – `ParseInventorySlot` | `OnItemAdded.Broadcast(InventoryID, TempSlot)` |
| GetClasses | Sim – array de structs | `OnClassesLoaded` (dados em `CurrentClasses`) |

---

## 3. Problema atual do Trade

O `LoadTradeState` atualmente:
- Faz parse em C++ **parcial** (valida success)
- Faz **OnTradeStateLoaded.Broadcast(JsonStr)** – passa **string JSON bruta**
- O Blueprint precisaria usar **VaRest Subsystem → Decode Json Object** para parsear

Isso viola o padrão: **Blueprint chamaria VaRest diretamente**.

---

## 4. Solução: mesmo método das outras APIs

O C++ deve:
1. **Parsear** o JSON em `OnLoadTradeStateComplete` (como `OnLoadInventoryRequestComplete`)
2. **Armazenar** em `CurrentTradeState` (FUmbraTradeState)
3. **Broadcast** `OnTradeStateLoaded.Broadcast(TradeState)` com struct
4. Oferecer **GetCurrentTradeState()** para o Blueprint acessar quando quiser

Assim, o Blueprint **nunca** usa VaRest – apenas `Get Game Instance` → `LoadTradeState` e o delegate com struct.

---

## 5. Structs necessárias

Ver código em `UmbraDataStructures.h` e implementação em `UmbraGameInstance.cpp` conforme seção seguinte.

---

## 6. Atualização do GUIA_WBP_TRADE_PASSO_A_PASSO.md

O Passo 2.4 (OnTradeStateLoaded_Custom) e Passo 4 (UpdateTradeUI) devem usar **FUmbraTradeState** recebido no delegate, sem Parse JSON nem VaRest no Blueprint.
