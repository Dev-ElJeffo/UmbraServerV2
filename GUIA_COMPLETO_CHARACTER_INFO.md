# 📋 GUIA COMPLETO: Widget de Informações do Personagem (Character Info)

## 🎯 **OBJETIVO:**

Criar um widget completo de informações do personagem que será aberto ao pressionar a tecla **C**, exibindo:
- Informações básicas (Nome, Nível, EXP)
- Stats base e totais (Força, Agilidade, Inteligência, Constituição, Sorte, etc.)
- Vida, Mana, Stamina
- Stats de combate (Ataque, Defesa, Crítico, etc.)
- Slots de equipamento com itens equipados

---

## ✅ **O QUE JÁ ESTÁ PRONTO:**

1. ✅ **API PHP:** `www/umbra_api/api/character/get_character_info.php`
2. ✅ **Estrutura C++:** `FUmbraCharacterInfo` (com todos os stats)
3. ✅ **Enum expandido:** `EUmbraEquipmentSlot` (incluindo Earring, Bracelet, Mount)
4. ✅ **FUmbraPlayerData expandido:** (com Agility, Constitution, Luck)
5. ✅ **Função C++:** `LoadCharacterInfo()` implementada
6. ✅ **Callbacks C++:** `OnLoadCharacterInfoComplete` e `OnLoadCharacterInfoFail` implementados
7. ✅ **Delegates:** `OnCharacterInfoLoaded` e `OnCharacterInfoFailed` adicionados
8. ✅ **ParseEquipmentSlot:** Atualizado para incluir Earring, Bracelet, Mount

---

## 🔧 **PARTE 1: Implementar C++ - UmbraGameInstance**

### **1.1 Adicionar Variável e Delegates**

**NO `UmbraGameInstance.h`:**

**ADICIONAR APÓS `TMap<EUmbraEquipmentSlot, FUmbraInventorySlot> EquippedItems;`:**

```cpp
// Informações completas do personagem
UPROPERTY(BlueprintReadOnly, Category = "Character")
FUmbraCharacterInfo CurrentCharacterInfo;
```

**ADICIONAR DELEGATES (após outros delegates de Character):**

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterInfoLoaded, const FUmbraCharacterInfo&, CharacterInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterInfoFailed, const FString&, ErrorMessage);
```

**ADICIONAR PROPRIEDADES DOS DELEGATES:**

```cpp
UPROPERTY(BlueprintAssignable, Category = "Events|Character")
FOnCharacterInfoLoaded OnCharacterInfoLoaded;

UPROPERTY(BlueprintAssignable, Category = "Events|Character")
FOnCharacterInfoFailed OnCharacterInfoFailed;
```

### **1.2 Adicionar Função LoadCharacterInfo**

**ADICIONAR APÓS `LoadCharacterList()`:**

```cpp
/**
 * Carregar informações completas do personagem ativo
 * Inclui stats calculados e equipamentos
 */
UFUNCTION(BlueprintCallable, Category = "Character")
void LoadCharacterInfo();
```

### **1.3 Adicionar Callbacks**

**ADICIONAR NA SEÇÃO PROTECTED:**

```cpp
UFUNCTION()
void OnLoadCharacterInfoComplete(UVaRestRequestJSON* Request);

UFUNCTION()
void OnLoadCharacterInfoFail(UVaRestRequestJSON* Request);
```

---

## 🔧 **PARTE 2: Implementar C++ - UmbraGameInstance.cpp**

### **2.1 Implementar LoadCharacterInfo**

**ADICIONAR APÓS `LoadCharacterList()`:**

```cpp
void UUmbraGameInstance::LoadCharacterInfo()
{
	if (!bIsAuthenticated || CurrentToken.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UmbraGameInstance] ❌ Tentativa de carregar informações do personagem sem autenticação"));
		OnCharacterInfoFailed.Broadcast(TEXT("Não autenticado"));
		return;
	}

	if (!HasActiveCharacter())
	{
		UE_LOG(LogTemp, Error, TEXT("[UmbraGameInstance] ❌ Tentativa de carregar informações sem personagem ativo"));
		OnCharacterInfoFailed.Broadcast(TEXT("Nenhum personagem selecionado"));
		return;
	}

	UVaRestRequestJSON* Request = CreateRequest(TEXT("/api/character/get_character_info.php"), TEXT("POST"));
	if (!Request)
	{
		OnCharacterInfoFailed.Broadcast(TEXT("Erro ao criar requisição"));
		return;
	}

	UVaRestSubsystem* VaRest = GEngine->GetEngineSubsystem<UVaRestSubsystem>();
	if (!VaRest)
	{
		OnCharacterInfoFailed.Broadcast(TEXT("VaRest Subsystem não encontrado"));
		return;
	}

	UVaRestJsonObject* JsonObject = VaRest->ConstructVaRestJsonObject();
	JsonObject->SetStringField(TEXT("token"), CurrentToken);

	Request->SetRequestObject(JsonObject);
	Request->OnRequestComplete.AddDynamic(this, &UUmbraGameInstance::OnLoadCharacterInfoComplete);
	Request->OnRequestFail.AddDynamic(this, &UUmbraGameInstance::OnLoadCharacterInfoFail);

	UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] 📡 Carregando informações do personagem..."));
	Request->ExecuteProcessRequest();
}
```

### **2.2 Implementar OnLoadCharacterInfoComplete**

**ADICIONAR:**

```cpp
void UUmbraGameInstance::OnLoadCharacterInfoComplete(UVaRestRequestJSON* Request)
{
	UVaRestJsonObject* ResponseObject = Request->GetResponseObject();

	if (!ResponseObject || !ResponseObject->HasField(TEXT("success")))
	{
		OnCharacterInfoFailed.Broadcast(TEXT("Resposta inválida do servidor"));
		return;
	}

	bool bSuccess = ResponseObject->GetBoolField(TEXT("success"));
	if (!bSuccess)
	{
		FString Message = ResponseObject->GetStringField(TEXT("message"));
		UE_LOG(LogTemp, Warning, TEXT("[UmbraGameInstance] ❌ Erro ao carregar informações: %s"), *Message);
		OnCharacterInfoFailed.Broadcast(Message);
		return;
	}

	UVaRestJsonObject* CharacterObj = ResponseObject->GetObjectField(TEXT("character"));
	if (!CharacterObj)
	{
		OnCharacterInfoFailed.Broadcast(TEXT("Dados do personagem não encontrados"));
		return;
	}

	// Parsear informações básicas
	FUmbraCharacterInfo CharacterInfo;
	CharacterInfo.PlayerID = CharacterObj->GetIntegerField(TEXT("player_id"));
	CharacterInfo.CharacterName = CharacterObj->GetStringField(TEXT("character_name"));
	CharacterInfo.Level = CharacterObj->GetIntegerField(TEXT("level"));
	CharacterInfo.Experience = CharacterObj->GetIntegerField(TEXT("experience"));
	CharacterInfo.ExpForNextLevel = CharacterObj->GetIntegerField(TEXT("exp_for_next_level"));
	CharacterInfo.ExpProgressPercent = CharacterObj->GetNumberField(TEXT("exp_progress_percent"));

	// Parsear stats base
	UVaRestJsonObject* StatsObj = CharacterObj->GetObjectField(TEXT("stats"));
	if (StatsObj)
	{
		UVaRestJsonObject* BaseStatsObj = StatsObj->GetObjectField(TEXT("base"));
		if (BaseStatsObj)
		{
			CharacterInfo.BaseStrength = BaseStatsObj->GetIntegerField(TEXT("strength"));
			CharacterInfo.BaseDexterity = BaseStatsObj->GetIntegerField(TEXT("dexterity"));
			CharacterInfo.BaseIntelligence = BaseStatsObj->GetIntegerField(TEXT("intelligence"));
			CharacterInfo.BaseVitality = BaseStatsObj->GetIntegerField(TEXT("vitality"));
			CharacterInfo.BaseAgility = BaseStatsObj->GetIntegerField(TEXT("agility"));
			CharacterInfo.BaseConstitution = BaseStatsObj->GetIntegerField(TEXT("constitution"));
			CharacterInfo.BaseLuck = BaseStatsObj->GetIntegerField(TEXT("luck"));
		}

		// Parsear stats totais
		UVaRestJsonObject* TotalStatsObj = StatsObj->GetObjectField(TEXT("total"));
		if (TotalStatsObj)
		{
			CharacterInfo.TotalStrength = TotalStatsObj->GetIntegerField(TEXT("strength"));
			CharacterInfo.TotalDexterity = TotalStatsObj->GetIntegerField(TEXT("dexterity"));
			CharacterInfo.TotalIntelligence = TotalStatsObj->GetIntegerField(TEXT("intelligence"));
			CharacterInfo.TotalVitality = TotalStatsObj->GetIntegerField(TEXT("vitality"));
			CharacterInfo.TotalAgility = TotalStatsObj->GetIntegerField(TEXT("agility"));
			CharacterInfo.TotalConstitution = TotalStatsObj->GetIntegerField(TEXT("constitution"));
			CharacterInfo.TotalLuck = TotalStatsObj->GetIntegerField(TEXT("luck"));
		}

		// Parsear Health
		UVaRestJsonObject* HealthObj = StatsObj->GetObjectField(TEXT("health"));
		if (HealthObj)
		{
			CharacterInfo.CurrentHealth = HealthObj->GetIntegerField(TEXT("current"));
			CharacterInfo.MaxHealthBase = HealthObj->GetIntegerField(TEXT("max_base"));
			CharacterInfo.MaxHealthTotal = HealthObj->GetIntegerField(TEXT("max_total"));
			CharacterInfo.HealthBonus = HealthObj->GetIntegerField(TEXT("bonus"));
		}

		// Parsear Mana
		UVaRestJsonObject* ManaObj = StatsObj->GetObjectField(TEXT("mana"));
		if (ManaObj)
		{
			CharacterInfo.CurrentMana = ManaObj->GetIntegerField(TEXT("current"));
			CharacterInfo.MaxManaBase = ManaObj->GetIntegerField(TEXT("max_base"));
			CharacterInfo.MaxManaTotal = ManaObj->GetIntegerField(TEXT("max_total"));
			CharacterInfo.ManaBonus = ManaObj->GetIntegerField(TEXT("bonus"));
		}

		// Parsear Stamina
		UVaRestJsonObject* StaminaObj = StatsObj->GetObjectField(TEXT("stamina"));
		if (StaminaObj)
		{
			CharacterInfo.CurrentStamina = StaminaObj->GetIntegerField(TEXT("current"));
			CharacterInfo.MaxStamina = StaminaObj->GetIntegerField(TEXT("max"));
		}

		// Parsear stats de combate
		UVaRestJsonObject* CombatObj = StatsObj->GetObjectField(TEXT("combat"));
		if (CombatObj)
		{
			CharacterInfo.PhysicalAttack = CombatObj->GetIntegerField(TEXT("physical_attack"));
			CharacterInfo.MagicAttack = CombatObj->GetIntegerField(TEXT("magic_attack"));
			CharacterInfo.PhysicalDefense = CombatObj->GetIntegerField(TEXT("physical_defense"));
			CharacterInfo.MagicDefense = CombatObj->GetIntegerField(TEXT("magic_defense"));
			CharacterInfo.Accuracy = CombatObj->GetIntegerField(TEXT("accuracy"));
			CharacterInfo.Dodge = CombatObj->GetIntegerField(TEXT("dodge"));
			CharacterInfo.Critical = CombatObj->GetIntegerField(TEXT("critical"));
			CharacterInfo.Movement = CombatObj->GetIntegerField(TEXT("movement"));
			CharacterInfo.Resistance = CombatObj->GetIntegerField(TEXT("resistance"));
			CharacterInfo.DoubleAttackRate = CombatObj->GetIntegerField(TEXT("double_attack_rate"));
		}
	}

	// Parsear equipamentos
	UVaRestJsonObject* EquippedItemsObj = CharacterObj->GetObjectField(TEXT("equipped_items"));
	if (EquippedItemsObj)
	{
		CharacterInfo.EquippedItems.Empty();
		
		// Iterar sobre todos os campos do objeto (cada campo é um slot de equipamento)
		TArray<FString> FieldNames;
		EquippedItemsObj->GetFieldNames(FieldNames);
		
		for (const FString& SlotName : FieldNames)
		{
			UVaRestJsonObject* ItemObj = EquippedItemsObj->GetObjectField(SlotName);
			if (ItemObj)
			{
				EUmbraEquipmentSlot EquipmentSlot = ParseEquipmentSlot(SlotName);
				if (EquipmentSlot != EUmbraEquipmentSlot::None)
				{
					FUmbraInventorySlot EquippedSlot;
					EquippedSlot.InventoryID = ItemObj->GetIntegerField(TEXT("inventory_id"));
					EquippedSlot.ItemTemplateID = ItemObj->GetIntegerField(TEXT("item_template_id"));
					EquippedSlot.ItemTemplate.ItemName = ItemObj->GetStringField(TEXT("item_name"));
					EquippedSlot.ItemTemplate.ItemDescription = ItemObj->GetStringField(TEXT("item_description"));
					EquippedSlot.ItemTemplate.EquipmentSlot = EquipmentSlot;
					EquippedSlot.ItemTemplate.Rarity = ParseItemRarity(ItemObj->GetStringField(TEXT("rarity")));
					EquippedSlot.Durability = ItemObj->GetNumberField(TEXT("durability"));
					EquippedSlot.bIsEquipped = true;
					
					// Parsear stats do item
					UVaRestJsonObject* ItemStatsObj = ItemObj->GetObjectField(TEXT("stats"));
					if (ItemStatsObj)
					{
						// Parsear stats do equipamento (se necessário)
					}
					
					// Buscar ícone local
					EquippedSlot.ItemTemplate.ItemIcon = GetItemIconByID(EquippedSlot.ItemTemplateID);
					
					CharacterInfo.EquippedItems.Add(EquipmentSlot, EquippedSlot);
				}
			}
		}
	}

	CurrentCharacterInfo = CharacterInfo;
	OnCharacterInfoLoaded.Broadcast(CharacterInfo);

	UE_LOG(LogTemp, Log, TEXT("[UmbraGameInstance] ✅ Informações do personagem carregadas: %s (Nível %d)"), 
		*CharacterInfo.CharacterName, CharacterInfo.Level);
}

void UUmbraGameInstance::OnLoadCharacterInfoFail(UVaRestRequestJSON* Request)
{
	LogRequestError(TEXT("LoadCharacterInfo"), Request);
	OnCharacterInfoFailed.Broadcast(TEXT("Erro de conexão ao carregar informações do personagem"));
}
```

### **2.3 Atualizar ParseEquipmentSlot**

**ADICIONAR NO `ParseEquipmentSlot`:**

```cpp
else if (SlotString.Equals(TEXT("earring"), ESearchCase::IgnoreCase))
{
	return EUmbraEquipmentSlot::Earring;
}
else if (SlotString.Equals(TEXT("bracelet"), ESearchCase::IgnoreCase))
{
	return EUmbraEquipmentSlot::Bracelet;
}
else if (SlotString.Equals(TEXT("mount"), ESearchCase::IgnoreCase))
{
	return EUmbraEquipmentSlot::Mount;
}
```

---

## 🔧 **PARTE 3: Criar Widget Blueprint WBP_CharacterInfo**

### **3.1 Estrutura do Widget**

**CRIAR WIDGET BLUEPRINT:**
- **Nome:** `WBP_CharacterInfo`
- **Parent Class:** `User Widget`

**COMPONENTES NECESSÁRIOS:**

1. **Painel Principal (Canvas Panel ou Vertical Box)**
2. **Header:**
   - TextBlock: Nome do Personagem
   - TextBlock: Nível
   - Progress Bar: EXP
   - TextBlock: EXP atual / EXP necessário

3. **Seção de Stats (Left Side):**
   - TextBlock: Força (Base / Total)
   - TextBlock: Agilidade (Base / Total)
   - TextBlock: Inteligência (Base / Total)
   - TextBlock: Constituição (Base / Total)
   - TextBlock: Sorte (Base / Total)
   - Progress Bar: Vida (Current / Max)
   - Progress Bar: Mana (Current / Max)
   - Progress Bar: Stamina (Current / Max)

4. **Seção de Stats de Combate (Right Side):**
   - TextBlock: Ataque Físico
   - TextBlock: Ataque Mágico
   - TextBlock: Defesa Física
   - TextBlock: Defesa Mágica
   - TextBlock: Acerto
   - TextBlock: Esquiva
   - TextBlock: Crítico
   - TextBlock: Movimento
   - TextBlock: Resistência
   - TextBlock: Taxa de Ataque Duplo

5. **Slots de Equipamento:**
   - **Esquerda (Vertical):**
     - Slot: Cabeça (Head)
     - Slot: Armadura (Chest)
     - Slot: Luvas (Hands)
     - Slot: Botas (Feet)
   
   - **Direita (Vertical):**
     - Slot: Anel (Ring)
     - Slot: Brinco (Earring)
     - Slot: Bracelete (Bracelet)
     - Slot: Colar (Amulet)
   
   - **Centro:**
     - Slot: Mão Principal (MainHand)
     - Slot: Mão Secundária (OffHand)
     - Slot: Montaria (Mount)

6. **Botão Fechar:**
   - Button: Fechar Widget

---

## 🔧 **PARTE 4: Implementar Input C**

### **4.1 Criar Input Action**

**NO PROJECT SETTINGS → INPUT → ACTION MAPPINGS:**
- **Action Name:** `OpenCharacterInfo`
- **Key:** `C`

### **4.2 Implementar no Player Controller ou Character**

**CRIAR FUNÇÃO NO BLUEPRINT DO PERSONAGEM:**

```
[Event BeginPlay]
  ↓
[Enable Input]
  ↓
[Get Player Controller]
  ↓
[Set Input Mode Game And UI]
```

**CRIAR FUNÇÃO `OpenCharacterInfo`:**
```
[OpenCharacterInfo (Action)]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ↓
[Load Character Info]
```

**CRIAR FUNÇÃO `CloseCharacterInfo`:**
```
[CloseCharacterInfo]
  ↓
[Remove from Parent] ← WBP_CharacterInfo
```

---

## 🔧 **PARTE 5: Conectar Delegates no WBP_CharacterInfo**

### **5.1 Event Construct**

**NO `WBP_CharacterInfo` → `Event Graph` → `Event Construct`:**

```
[Event Construct]
  ↓
[Get Game Instance]
  ↓
[Cast to Umbra Game Instance]
  ↓
[Bind Event to OnCharacterInfoLoaded]
  ├─ Target: Umbra Game Instance
  └─ Event: OnCharacterInfoLoaded_Event
  ↓
[Bind Event to OnCharacterInfoFailed]
  ├─ Target: Umbra Game Instance
  └─ Event: OnCharacterInfoFailed_Event
  ↓
[Load Character Info] ← Carregar informações imediatamente
  └─ Target: Umbra Game Instance
```

### **5.2 OnCharacterInfoLoaded_Event**

**CRIAR EVENTO CUSTOMIZADO:**

```
[OnCharacterInfoLoaded_Event]
  └─ Character Info: (FUmbraCharacterInfo)
  ↓
[Update Character Info Display]
  └─ Character Info: Character Info
```

### **5.3 Função Update Character Info Display**

**CRIAR FUNÇÃO NO BLUEPRINT:**

```
[Update Character Info Display]
  ├─ Character Info: (FUmbraCharacterInfo)
  ↓
[Break Umbra Character Info]
  └─ Character Info: Character Info
  ↓
[Set Text] ← Nome do Personagem
  └─ Text: Character Name
  ↓
[Set Text] ← Nível
  └─ Text: "Nível: [Level]"
  ↓
[Set Percent] ← Progress Bar EXP
  └─ Percent: Exp Progress Percent / 100.0
  ↓
[Set Text] ← EXP
  └─ Text: "[Experience] / [ExpForNextLevel]"
  ↓
[Update Stats Display] ← Função separada
  └─ Character Info: Character Info
  ↓
[Update Equipment Slots] ← Função separada
  └─ Character Info: Character Info
```

---

## 🔧 **PARTE 6: Atualizar Slots de Equipamento**

### **6.1 Criar Widget de Slot de Equipamento**

**CRIAR WIDGET BLUEPRINT:**
- **Nome:** `WBP_EquipmentSlot`
- **Parent Class:** `User Widget`

**COMPONENTES:**
- Image: Ícone do Item
- Border: Borda (cor baseada na raridade)
- TextBlock: Nome do Item (opcional, tooltip)

### **6.2 Função Update Equipment Slots**

**NO `WBP_CharacterInfo`:**

```
[Update Equipment Slots]
  ├─ Character Info: (FUmbraCharacterInfo)
  ↓
[Break Umbra Character Info]
  └─ Character Info: Character Info
  ↓
[Get Equipped Items] ← TMap
  └─ Character Info: Character Info
  ↓
[ForEach Equipped Items]
  ├─ Key: Equipment Slot
  ├─ Value: Inventory Slot
  ↓
[Switch on EUmbraEquipmentSlot]
  ├─ Head: [Set Slot Data] → Slot_Head
  ├─ Chest: [Set Slot Data] → Slot_Chest
  ├─ Hands: [Set Slot Data] → Slot_Hands
  ├─ Feet: [Set Slot Data] → Slot_Feet
  ├─ MainHand: [Set Slot Data] → Slot_MainHand
  ├─ OffHand: [Set Slot Data] → Slot_OffHand
  ├─ Ring: [Set Slot Data] → Slot_Ring
  ├─ Earring: [Set Slot Data] → Slot_Earring
  ├─ Bracelet: [Set Slot Data] → Slot_Bracelet
  ├─ Amulet: [Set Slot Data] → Slot_Amulet
  └─ Mount: [Set Slot Data] → Slot_Mount
```

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **C++:**
- [ ] Adicionar `FUmbraCharacterInfo CurrentCharacterInfo` no UmbraGameInstance.h
- [ ] Adicionar delegates `OnCharacterInfoLoaded` e `OnCharacterInfoFailed`
- [ ] Adicionar função `LoadCharacterInfo()`
- [ ] Adicionar callbacks `OnLoadCharacterInfoComplete` e `OnLoadCharacterInfoFail`
- [ ] Implementar `LoadCharacterInfo()` no .cpp
- [ ] Implementar parsing completo da resposta JSON
- [ ] Atualizar `ParseEquipmentSlot` para incluir Earring, Bracelet, Mount
- [ ] Expandir `FUmbraPlayerData` com Agility, Constitution, Luck
- [ ] Expandir `EUmbraEquipmentSlot` com Earring, Bracelet, Mount

### **Blueprint:**
- [ ] Criar `WBP_CharacterInfo`
- [ ] Criar `WBP_EquipmentSlot`
- [ ] Implementar layout completo do widget
- [ ] Conectar delegates no Event Construct
- [ ] Criar função `Update Character Info Display`
- [ ] Criar função `Update Equipment Slots`
- [ ] Criar função `Update Stats Display`
- [ ] Implementar input C para abrir/fechar widget
- [ ] Testar carregamento e exibição de dados

---

## 🎉 **PRONTO!**

Após implementar, o widget de informações do personagem estará completamente funcional!

**TESTE:**
1. Pressione C no jogo
2. Widget deve abrir com todas as informações
3. Stats devem ser exibidos corretamente
4. Equipamentos devem aparecer nos slots corretos

---

**DÚVIDAS?** Consulte os arquivos:
- `www/umbra_api/api/character/get_character_info.php` - API PHP
- `UmbraDataStructures.h` - Estruturas de dados
- `UmbraGameInstance.cpp` - Implementação C++

