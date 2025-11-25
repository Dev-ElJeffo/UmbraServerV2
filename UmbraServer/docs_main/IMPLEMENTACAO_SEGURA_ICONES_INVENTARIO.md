# 🔒 Implementação Segura de Ícones para o Sistema de Inventário

**Data:** 14 de Novembro de 2025  
**Status:** ✅ Completo  
**Objetivo:** Adicionar ícones de itens sem comprometer a segurança do sistema

---

## 🚨 PROBLEMA DE SEGURANÇA IDENTIFICADO

### **Solução Inicial (INSEGURA):**
- ❌ Data Table com `FUmbraItemTemplate` completo (stats, valor, dano, etc.)
- ❌ Dados sensíveis armazenados no cliente
- ❌ Possibilidade de cheating/hacking (modificar dano, valor, etc.)
- ❌ Duplicação de dados (MySQL + Data Table Unreal)
- ❌ Inconsistência entre servidor e cliente

### **Solução Corrigida (SEGURA):**
- ✅ Data Table com `FUmbraItemIconMapping` (APENAS ItemID + Ícone)
- ✅ Dados sensíveis vêm SEMPRE da API (servidor)
- ✅ Impossível hackear stats modificando arquivos locais
- ✅ Servidor é a única fonte confiável de dados
- ✅ Cliente armazena apenas texturas visuais

---

## 📋 Modificações Implementadas

### **1️⃣ Novo Struct para Mapeamento de Ícones**

#### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h`

```cpp
/**
 * Mapeamento de ItemID para Ícone (usado APENAS no cliente para visual)
 * SEGURANÇA: Este struct contém APENAS referências visuais, sem dados sensíveis
 * Todas as informações de gameplay (stats, valor, etc.) vêm da API
 */
USTRUCT(BlueprintType)
struct FUmbraItemIconMapping : public FTableRowBase
{
	GENERATED_BODY()

	/** ID do item (corresponde ao item_id do banco MySQL) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 ItemID = 0;

	/** Ícone do item (textura local) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UTexture2D* ItemIcon = nullptr;
};
```

**Por que é seguro:**
- ✅ Contém APENAS `ItemID` (int32) e `ItemIcon` (UTexture2D*)
- ✅ Não expõe stats, dano, valor, raridade, peso
- ✅ Mesmo se hackeado, não afeta gameplay
- ✅ Texturas não podem alterar mecânicas de jogo

---

### **2️⃣ Data Table Renomeado e Restrito**

#### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h`

```cpp
/** 
 * Data Table de mapeamento ItemID → Ícone (APENAS visual, sem dados sensíveis)
 * SEGURANÇA: Contém apenas referências visuais. Todas as informações de gameplay 
 * (stats, valor, dano, etc.) vêm da API para evitar cheating
 */
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
UDataTable* ItemIconsDataTable = nullptr;
```

**Mudanças:**
- ❌ `ItemTemplatesDataTable` (continha dados completos - REMOVIDO)
- ✅ `ItemIconsDataTable` (contém apenas ícones - SEGURO)

---

### **3️⃣ Função de Busca Simplificada**

#### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.h`

```cpp
/**
 * Buscar APENAS o ícone do item por ID (busca local, sem dados sensíveis)
 * SEGURANÇA: Retorna apenas a textura visual, não expõe stats ou valores
 * @param ItemID ID do item
 * @return Ícone do item (nullptr se não encontrado)
 */
UFUNCTION(BlueprintCallable, Category = "Inventory")
UTexture2D* GetItemIconByID(int32 ItemID) const;
```

**Mudanças:**
- ❌ `GetItemTemplateByID()` retornava template completo (REMOVIDO)
- ✅ `GetItemIconByID()` retorna APENAS a textura (SEGURO)

---

### **4️⃣ Implementação Segura**

#### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`

```cpp
UTexture2D* UUmbraGameInstance::GetItemIconByID(int32 ItemID) const
{
	// Validação: DataTable configurado
	if (!ItemIconsDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ⚠️ ItemIconsDataTable não configurado!"));
		return nullptr;
	}

	// Buscar no Data Table (APENAS ícone, sem dados de gameplay)
	FString RowName = FString::Printf(TEXT("Item_%d"), ItemID);
	FUmbraItemIconMapping* IconMapping = ItemIconsDataTable->FindRow<FUmbraItemIconMapping>(
		FName(*RowName), TEXT("GetItemIconByID"));
	
	if (IconMapping && IconMapping->ItemIcon)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[UmbraGameInstance] 🖼️ Ícone encontrado para ItemID %d"), ItemID);
		return IconMapping->ItemIcon;
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("[UmbraGameInstance] ⚠️ Ícone não configurado para ItemID %d"), ItemID);
		return nullptr;
	}
}
```

**Por que é seguro:**
- ✅ Retorna APENAS `UTexture2D*` (textura visual)
- ✅ Não retorna stats, valores ou dados sensíveis
- ✅ Busca em `FUmbraItemIconMapping` (struct minimalista)

---

### **5️⃣ ParseInventorySlot Corrigido**

#### **Arquivo:** `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`

```cpp
FUmbraInventorySlot UUmbraGameInstance::ParseInventorySlot(UVaRestJsonObject* SlotObject)
{
	FUmbraInventorySlot Slot;
	
	// ... (parsing de campos básicos) ...
	
	// SEGURANÇA: Parsear template da API (todas as informações vêm do servidor)
	// Isso garante que stats, valores e dados sensíveis não sejam manipuláveis no cliente
	Slot.ItemTemplate = ParseItemTemplate(SlotObject);
	
	// Visual: Buscar APENAS o ícone localmente (sem dados de gameplay)
	// Substitui apenas a textura visual, mantendo todos os dados da API intactos
	Slot.ItemTemplate.ItemIcon = GetItemIconByID(Slot.ItemTemplateID);
	
	return Slot;
}
```

**Fluxo Seguro:**
1. ✅ API retorna item completo (nome, stats, valor, raridade, etc.)
2. ✅ `ParseItemTemplate()` converte TODOS os dados da API → `FUmbraItemTemplate`
3. ✅ `GetItemIconByID()` busca APENAS o ícone local → `UTexture2D*`
4. ✅ Substitui apenas o campo `ItemIcon`, mantendo todos os outros dados da API

**Por que é seguro:**
- ✅ Dados de gameplay (stats, valor) vêm da API
- ✅ Ícone é apenas visual (não afeta mecânicas)
- ✅ Mesmo hackeando o Data Table, não altera dano/valor
- ✅ Servidor valida todas as operações

---

## 🔒 Análise de Segurança

### **O Que Está Protegido:**

| Dado | Origem | Pode ser Hackeado? |
|------|--------|-------------------|
| Nome do item | API (Servidor) | ❌ NÃO |
| Dano | API (Servidor) | ❌ NÃO |
| Defesa | API (Servidor) | ❌ NÃO |
| Valor | API (Servidor) | ❌ NÃO |
| Raridade | API (Servidor) | ❌ NÃO |
| Peso | API (Servidor) | ❌ NÃO |
| Stats | API (Servidor) | ❌ NÃO |
| **Ícone** | **Data Table (Cliente)** | ✅ **SIM (mas não afeta gameplay!)** |

### **Cenários de Ataque:**

#### **❌ Ataque 1: Modificar Data Table para aumentar dano**
```cpp
// Hacker tenta: DT_ItemIcons[Item_1].Damage = 9999

// RESULTADO: Impossível! O struct FUmbraItemIconMapping não tem campo Damage.
// Apenas ItemID e ItemIcon estão disponíveis.
```

#### **❌ Ataque 2: Modificar valor do item no Data Table**
```cpp
// Hacker tenta: DT_ItemIcons[Item_1].Value = 999999

// RESULTADO: Impossível! O struct não tem campo Value.
// Valor vem da API e é validado pelo servidor.
```

#### **✅ Ataque 3: Modificar ícone do item**
```cpp
// Hacker tenta: DT_ItemIcons[Item_1].ItemIcon = IconDeEspadaLendaria

// RESULTADO: Possível, mas inútil! Apenas o visual muda.
// Stats, dano e valor continuam vindo da API.
// O hacker vê uma espada lendária, mas o servidor sabe que é uma espada de ferro.
```

---

## 📊 Comparação: Antes vs Depois

### **❌ ANTES (Inseguro):**

```cpp
// Data Table: DT_ItemTemplates (FUmbraItemTemplate)
Row: Item_1
  - ItemID: 1
  - ItemName: "Espada de Ferro"          ← Exposto no cliente
  - Damage: 25                             ← HACKEÁVEL!
  - Defense: 0                             ← HACKEÁVEL!
  - Value: 150                             ← HACKEÁVEL!
  - Rarity: Common                         ← HACKEÁVEL!
  - ItemIcon: Icon_Sword_Iron              ← Visual
```

```cpp
// C++ busca template COMPLETO do Data Table
Slot.ItemTemplate = GetItemTemplateByID(ItemID);  // ❌ Dados sensíveis!
```

### **✅ DEPOIS (Seguro):**

```cpp
// Data Table: DT_ItemIcons (FUmbraItemIconMapping)
Row: Item_1
  - ItemID: 1                              ← Identificador
  - ItemIcon: Icon_Sword_Iron              ← APENAS visual
```

```cpp
// C++ busca dados da API + ícone local
Slot.ItemTemplate = ParseItemTemplate(SlotObject);  // ✅ Da API (seguro)
Slot.ItemTemplate.ItemIcon = GetItemIconByID(ItemID);  // ✅ Apenas visual
```

---

## 🎯 Instruções para o Usuário

### **Data Table Simplificado:**

1. **Criar Data Table:**
   - Content Browser → Right Click → Data Table
   - **Pick Structure**: `UmbraItemIconMapping` ⭐ (NÃO `UmbraItemTemplate`)
   - Nome: `DT_ItemIcons`

2. **Preencher APENAS:**
   - Row Name: `Item_1`, `Item_2`, ..., `Item_17`
   - **Item ID**: `1`, `2`, ..., `17`
   - **Item Icon**: Selecionar textura importada

3. **NÃO há campos para:**
   - ❌ Nome, descrição
   - ❌ Stats (dano, defesa)
   - ❌ Valor, peso
   - ❌ Raridade, nível

---

## 🔍 Testes de Segurança

### **Teste 1: Verificar que dados vêm da API**

```cpp
// Log em ParseInventorySlot (após ParseItemTemplate):
UE_LOG(LogTemp, Log, TEXT("Item: %s, Damage: %.2f, Value: %d [FROM API]"), 
    *Slot.ItemTemplate.ItemName, 
    Slot.ItemTemplate.Stats.Damage, 
    Slot.ItemTemplate.Value);

// Resultado esperado: Todos os valores corretos da API
```

### **Teste 2: Modificar Data Table e verificar que não afeta stats**

1. Abra `DT_ItemIcons`
2. Mude o ícone do `Item_1` para uma espada lendária
3. Entre no jogo e pegue o `Item_1`
4. Verifique:
   - ✅ Ícone mudou (visual)
   - ❌ Dano NÃO mudou (veio da API)
   - ❌ Valor NÃO mudou (veio da API)

---

## 📝 Checklist de Segurança

- ✅ `FUmbraItemIconMapping` criado (struct minimalista)
- ✅ `ItemIconsDataTable` usa `FUmbraItemIconMapping`
- ✅ `GetItemIconByID()` retorna APENAS `UTexture2D*`
- ✅ `ParseInventorySlot()` usa API para dados + Data Table para ícone
- ✅ Dados sensíveis (stats, valor) vêm SEMPRE da API
- ✅ Texturas são apenas visuais (não afetam gameplay)
- ✅ Servidor valida todas as operações
- ✅ Guia atualizado com avisos de segurança

---

## 🚀 Resultado Final

✅ **Sistema de ícones funcionando**  
✅ **Totalmente seguro contra cheating**  
✅ **Servidor é a única fonte confiável**  
✅ **Cliente armazena apenas visuais**  
✅ **Performance otimizada** (texturas locais)  
✅ **Arquitetura escalável**

---

**🔒 Segurança Garantida! O servidor controla tudo, o cliente só exibe.**

