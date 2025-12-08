# 🚨 **URGENTE: Restaurar UmbraDataStructures.h**

## ❌ **PROBLEMA**

O arquivo `UmbraDataStructures.h` foi **SOBRESCRITO** e agora tem apenas 25 linhas quando deveria ter mais de 1000.

**Erro de compilação:**
```
D:\UmbraServerV2\UmbraEternumUE\Source\UmbraEternumUE\Data\UmbraDataStructures.h(11): error : Invalid use of keyword 'UPROPERTY'.  It may only appear in Class, and Struct scopes
```

---

## ✅ **SOLUÇÕES (TENTE NA ORDEM)**

### **1. Visual Studio - Undo (MAIS RÁPIDO)**

1. **Abra o arquivo** `UmbraDataStructures.h` no Visual Studio
2. **Pressione Ctrl+Z várias vezes** até restaurar o conteúdo original
3. **Salve o arquivo** (Ctrl+S)

**⚠️ Se o Visual Studio não tiver histórico suficiente, tente a Opção 2.**

---

### **2. Verificar Backup Automático do Visual Studio**

1. **No Visual Studio:**
   - Menu **File** → **Recent Files and Projects**
   - Ou procure por **"Backup"** no menu
   - Ou navegue até: `%LOCALAPPDATA%\Microsoft\VisualStudio\Backup\`

2. **Procure por:** `UmbraDataStructures.h` com data/hora anterior

---

### **3. Verificar Histórico Local do Git (se houver)**

```powershell
# Verificar se há commits locais não enviados
git reflog

# Ver o conteúdo de um commit anterior
git show HEAD@{1}:UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h > UmbraDataStructures_backup.h
```

---

### **4. Verificar Backup do Unreal Engine**

1. **Navegue até:** `UmbraEternumUE/Saved/Autosaves/`
2. **Procure por:** arquivos com nome similar a `UmbraDataStructures`

---

### **5. ÚLTIMA OPÇÃO: Recriar Manualmente**

Se nenhuma das opções acima funcionar, o arquivo precisa ser recriado manualmente. **⚠️ ISSO É MUITO TRABALHOSO!**

**Estruturas que DEVEM estar no arquivo:**
- `FUmbraAccountData`
- `FUmbraPlayerData` (com todos os campos)
- `FUmbraClassData`
- `FUmbraCharacterInfo` (com todos os campos)
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

**Referências para reconstruir:**
- Veja `UmbraGameInstance.cpp` linha 687+ para ver como `FUmbraCharacterInfo` é usado
- Veja `UmbraGameInstance.h` linha 238 para ver a declaração
- Veja outros arquivos que incluem `UmbraDataStructures.h`

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

Após restaurar:

1. **Compile o projeto** (Build → Compile)
2. **Verifique se não há erros** de compilação
3. **No Blueprint**, você deve conseguir criar uma variável do tipo `Placeholder Spawn Config`

---

**⚠️ RECOMENDAÇÃO:** Tente a Opção 1 (Ctrl+Z no Visual Studio) PRIMEIRO - é a mais rápida e geralmente funciona!

---

**Fim do Guia**

