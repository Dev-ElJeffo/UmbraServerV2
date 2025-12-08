# 🔧 **CORREÇÃO: UmbraDataStructures.h Corrompido**

## ❌ **ERRO**

```
D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Data\UmbraDataStructures.h(11): error : Invalid use of keyword 'UPROPERTY'.  It may only appear in Class, and Struct scopes
```

O arquivo `UmbraDataStructures.h` foi corrompido e tem apenas 35 linhas quando deveria ter mais de 1000.

---

## ✅ **SOLUÇÃO**

### **OPÇÃO 1: Restaurar do Backup do Unreal Engine**

1. **Feche o Unreal Engine**
2. **Navegue até:** `UmbraEternumUE/Intermediate/Backup/`
3. **Procure por:** `UmbraDataStructures.h.backup` ou similar
4. **Copie o backup** para `UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h`

### **OPÇÃO 2: Restaurar do Visual Studio**

1. **No Visual Studio:**
   - Clique com botão direito em `UmbraDataStructures.h`
   - Selecione **"Undo"** ou **Ctrl+Z** várias vezes até restaurar

### **OPÇÃO 3: Recriar Manualmente (ÚLTIMA OPÇÃO)**

Se não houver backup, você precisará recriar o arquivo. **⚠️ ISSO É MUITO TRABALHOSO!**

**Estruturas que devem estar no arquivo:**
- `FUmbraAccountData`
- `FUmbraPlayerData`
- `FUmbraClassData`
- `FUmbraCharacterInfo`
- `EUmbraConnectionStatus`
- `FUmbraItemIconMapping`
- `EUmbraItemType`
- `EUmbraItemRarity`
- `EUmbraEquipmentSlot`
- `FUmbraItemStatsBase`
- `FUmbraItemStatsCombat`
- `FUmbraItemStatsBonus`
- `FUmbraItemStatsLegacy`
- `FUmbraItemData`
- `FUmbraInventorySlot`
- E outras...

**⚠️ RECOMENDAÇÃO:** Tente as Opções 1 e 2 primeiro!

---

## 📝 **APÓS RESTAURAR: Adicionar Struct FPlaceholderSpawnConfig**

**No final do arquivo (após a última struct), adicione:**

```cpp
/**
 * Configuração para spawn de placeholder de classe
 */
USTRUCT(BlueprintType)
struct FPlaceholderSpawnConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Placeholder")
	int32 ClassID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Placeholder")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Placeholder")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Placeholder")
	TSubclassOf<AActor> PlaceholderClass = nullptr;
};
```

---

## ✅ **VERIFICAÇÃO**

Após restaurar e adicionar a struct:

1. **Compile o projeto** (Build → Compile)
2. **Verifique se não há erros** de compilação
3. **No Blueprint**, você deve conseguir criar uma variável do tipo `Placeholder Spawn Config`

---

**Fim do Guia**

