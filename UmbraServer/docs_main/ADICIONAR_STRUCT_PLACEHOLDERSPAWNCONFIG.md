# 🔧 **ADICIONAR STRUCT: FPlaceholderSpawnConfig**

## ⚠️ **IMPORTANTE**

O arquivo `UmbraDataStructures.h` precisa ter a struct `FPlaceholderSpawnConfig` adicionada manualmente.

---

## 📝 **PASSO A PASSO**

### **1. Abrir o Arquivo**

**Caminho:** `UmbraEternumUE/Source/UmbraEternumUE/Data/UmbraDataStructures.h`

### **2. Localizar o Final do Arquivo**

Procure pela última struct (provavelmente `FUmbraCharacterInfo`) que termina com:

```cpp
	UPROPERTY(BlueprintReadWrite, Category = "Stats|Movement")
	int32 Movement = 0;
};
```

### **3. Adicionar a Struct Após o Último `};`**

**Adicione o seguinte código:**

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

### **4. Verificar Includes**

Certifique-se de que o arquivo inclui:

```cpp
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "UmbraDataStructures.generated.h"
```

**Se `TSubclassOf` não for reconhecido, adicione:**

```cpp
#include "Engine/Actor.h"
```

---

## ✅ **VERIFICAÇÃO**

Após adicionar:

1. **Compile o projeto** (Build → Compile)
2. **Verifique se não há erros** de compilação
3. **No Blueprint**, você deve conseguir criar uma variável do tipo `Placeholder Spawn Config`

---

**Fim do Guia**

