# Procedimento Completo: Implementação WBP_SkillBar

## FASE 1: SETUP DE MATERIAIS

### Passo 1.1 - Criar Material de Cooldown Radial

1. **Abra o Content Browser**
2. **Navegue a:** `Content/Materials/UI/`
3. **Clique direito** → **Material**
4. **Nome:** `M_CooldownRadial`
5. **Duplo-clique para abrir**
6. **Na Details (direita):**
   - `Material → Blend Mode`: **Opaque**
   - `Material → Lighting Model`: **Unlit**
   - `Material → Material Domain`: **Surface**

7. **Criar nós (click direito no canvas):**
   - **ScalarParameter** (nome = `Progress`, default = 0.0, range 0-1)
   - **TextureCoordinate** (index = 0)
   - **Atan2**: pega as UVs e converte em ângulo
   - **Divide**: divide o resultado por 2π para normalizar (0-1)
   - **Compare**: compara `Progress` > `Angle`
   - **If**: retorna cor completa se `Progress` > `Angle`, senão cor escura
   - **Final Color Output** → conecta o If

8. **Setup dos nós:**
   ```
   TextureCoordinate (0,0)
     ├─ R (U) → Atan2 (B) 
     └─ G (V) → Atan2 (A)
   
   Atan2 → Divide(÷ 6.28) → Compare(> Progress)
   
   If(Progress > Angle)
     ├─ True: Vector(1,1,1,1) [branco]
     └─ False: Vector(0.2,0.2,0.2,1) [cinza]
   
   If → Base Color
   ```

9. **Conectar Opacity Mask (opcional):**
   - Criar **RadialGradientExponential** (center = 0.5, 0.5; radius = 0.5)
   - Conectar em **Opacity Mask**

10. **Salvar** (`Ctrl+S`)

### Passo 1.2 - Criar Material Instance

1. **Click direito no M_CooldownRadial**
2. **Create Material Instance**
3. **Nome:** `MI_CooldownRadial`
4. **Duplo-clique para abrir**
5. **Marcar "Progress" como parameter** (deve aparecer na lista)
6. **Salvar**

---

## FASE 2: CRIAR WIDGET WBP_SkillSlot

### Passo 2.1 - Criar Widget

1. **Content Browser** → **User Interface** → **Widget Blueprint**
2. **Nome:** `WBP_SkillSlot`
3. **Salvar**
4. **Duplo-clique para abrir no Designer**

### Passo 2.2 - Estrutura Visual

**Hierarchy (lado esquerdo) - criar nessa ordem:**

```
SizeBox (Root)
├── Overlay
    ├── Border_Background (Border)
    ├── Skill_Icon (Image)
    ├── Cooldown_Overlay (Canvas Panel)
    │   ├── Cooldown_Fill (Image com material)
    │   └── Cooldown_Text (Text Block)
    ├── Resource_Warning (Image - vermelho, piscar)
    ├── Keybind_Text (Text Block - canto inferior direito)
    ├── Rank_Indicator (Horizontal Box)
    └── Unusable_Overlay (Canvas Panel - vermelho "X")
```

### Passo 2.3 - Configurar Root (SizeBox)

1. **Click em SizeBox** (root)
2. **Details (direita):**
   - **Size X**: 64
   - **Size Y**: 64
   - **Visibility**: Visible

### Passo 2.4 - Configurar Overlay (filho de SizeBox)

1. **Add Child** → **Overlay**
2. **Details:**
   - **Size X**: Fill
   - **Size Y**: Fill

### Passo 2.5 - Configurar Border_Background

1. **Add Child ao Overlay** → **Border** (índice 0)
2. **Details:**
   - **Name**: `Border_Background`
   - **Size X**: Fill
   - **Size Y**: Fill
   - **Brush Color**: `#1A1A2E` (azul escuro) Alpha = 1.0
   - **Border**: Thickness = 1, Color = `#FFFFFF`

### Passo 2.6 - Configurar Skill_Icon (Image)

1. **Add Child ao Overlay** → **Image** (índice 1)
2. **Details:**
   - **Name**: `Skill_Icon`
   - **Size X**: Fill
   - **Size Y**: Fill
   - **Brush**: deixe vazio (será setado no Blueprint)
   - **Color and Opacity**: Branco (será atualizado via Blueprint)

### Passo 2.7 - Configurar Cooldown_Overlay (Canvas Panel)

1. **Add Child ao Overlay** → **Canvas Panel** (índice 2)
2. **Details:**
   - **Name**: `Cooldown_Overlay`
   - **Visibility**: Collapsed (será mostrado quando houver cooldown)
   - **Canvas Slot:**
     - **Position X**: 0
     - **Position Y**: 0
     - **Size X**: 64
     - **Size Y**: 64

### Passo 2.8 - Configurar Cooldown_Fill (dentro de Cooldown_Overlay)

1. **Click em Cooldown_Overlay**
2. **Add Child** → **Image**
3. **Details:**
   - **Name**: `Cooldown_Fill`
   - **Size X**: Fill
   - **Size Y**: Fill
   - **Brush**: selecione `MI_CooldownRadial`
   - **Brush Size**: Fixed (64 × 64)

### Passo 2.9 - Configurar Cooldown_Text (dentro de Cooldown_Overlay)

1. **Add Child ao Cooldown_Overlay** → **Text Block**
2. **Details:**
   - **Name**: `Cooldown_Text`
   - **Text**: "0s"
   - **Font**: Roboto, Tamanho 16, Bold
   - **Color and Opacity**: Branco
   - **Canvas Slot:**
     - **Position X**: 16
     - **Position Y**: 24 (centralizado)
     - **Size X**: 32
     - **Size Y**: 16

### Passo 2.10 - Configurar Resource_Warning (Image)

1. **Add Child ao Overlay** → **Image** (índice 3)
2. **Details:**
   - **Name**: `Resource_Warning`
   - **Size X**: Fill
   - **Size Y**: Fill
   - **Brush**: cor vermelha `#FF0000` (ou imagem de warning)
   - **Color and Opacity**: Branco Alpha = 0.5
   - **Visibility**: Collapsed

### Passo 2.11 - Configurar Keybind_Text

1. **Add Child ao Overlay** → **Text Block** (índice 4)
2. **Details:**
   - **Name**: `Keybind_Text`
   - **Text**: "1"
   - **Font**: Roboto, Tamanho 10
   - **Color and Opacity**: Amarelo `#FFFF00`
   - **Canvas Slot:**
     - **Position X**: 44
     - **Position Y**: 50 (canto inferior direito)
     - **Size X**: 20
     - **Size Y**: 14

### Passo 2.12 - Configurar Rank_Indicator (Horizontal Box)

1. **Add Child ao Overlay** → **Horizontal Box** (índice 5)
2. **Details:**
   - **Name**: `Rank_Indicator`
   - **Size X**: Wrap
   - **Size Y**: Auto
   - **Canvas Slot:**
     - **Position X**: 4
     - **Position Y**: 4 (canto superior esquerdo)
3. **Adicionar 5 × Image (estrelas)** com tamanho 8×8 cada
   - Brush: Cor branca quando ativa, cinza quando inativa
   - Espaço entre elas: 1px

### Passo 2.13 - Configurar Unusable_Overlay (Canvas Panel)

1. **Add Child ao Overlay** → **Canvas Panel** (índice 6)
2. **Details:**
   - **Name**: `Unusable_Overlay`
   - **Visibility**: Collapsed
   - **Canvas Slot:** Fill (0, 0, 64, 64)
3. **Add Child** → **Image**
   - Brush: vermelho `#FF0000`
   - Opacity: 0.4
   - Color: overlay semitransparente
4. **Add Child** → **Text Block**
   - Text: "X"
   - Font: Tamanho 36, Bold, Branco
   - Posição: Centro (16, 16)

### Passo 2.14 - Ativar Desinger View para verificar

- Clique em **"Preview"** (botão no canto superior)
- Verifique que todos os elementos estão visíveis

---

## FASE 3: C++ - CRIAR CLASSE WBP_SkillSlot

### Passo 3.1 - Criar Header (UmbraSkillSlotWidget.h)

**Local:** `Source/UmbraEternumUE/UI/UmbraSkillSlotWidget.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Data/UmbraSkillDataStructures.h"
#include "UmbraSkillSlotWidget.generated.h"

UCLASS()
class UMBRAETERNUMUE_API UUmbraSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widgets
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Skill_Icon;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Cooldown_Text;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Cooldown_Fill;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> Cooldown_Overlay;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Resource_Warning;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Keybind_Text;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> Unusable_Overlay;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Background;

	// Dados do slot
	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	int32 SkillID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	FString SkillName;

	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	FString Keybind;

	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	int32 CurrentRank = 1;

	UPROPERTY(BlueprintReadWrite, Category = "SkillSlot")
	UTexture2D* SkillIcon = nullptr;

	// Runtime
	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	bool bIsOnCooldown = false;

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	float CooldownRemaining = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	int32 ResourceCost = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Runtime")
	FString ResourceType;

	// Funções
	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void SetSlotData(const FUmbraSkillbarSlot& SlotData);

	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void UpdateCooldownVisual(float RemainingSeconds, int32 TotalCooldownMs);

	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void ClearCooldown();

	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void SetEmpty();

	UFUNCTION(BlueprintCallable, Category = "SkillSlot")
	void UpdateResourceWarning(bool bHasEnoughResource);

protected:
	virtual void NativeConstruct() override;

private:
	// Material de cooldown para atualização programática
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicCooldownMaterial;
};
```

### Passo 3.2 - Criar Implementation (UmbraSkillSlotWidget.cpp)

**Local:** `Source/UmbraEternumUE/UI/UmbraSkillSlotWidget.cpp`

```cpp
#include "UmbraSkillSlotWidget.h"
#include "Components/Image.h"
#include "Material/MaterialInstanceDynamic.h"

void UUmbraSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Criar material dinâmico para cooldown
	if (Cooldown_Fill)
	{
		DynamicCooldownMaterial = Cooldown_Fill->GetDynamicMaterial();
		if (!DynamicCooldownMaterial && Cooldown_Fill->Brush.GetResourceObject())
		{
			DynamicCooldownMaterial = UMaterialInstanceDynamic::Create(
				Cast<UMaterialInterface>(Cooldown_Fill->Brush.GetResourceObject()),
				this
			);
			Cooldown_Fill->SetBrushFromMaterial(DynamicCooldownMaterial);
		}
	}

	// Inicializar estado
	SetEmpty();
}

void UUmbraSkillSlotWidget::SetSlotData(const FUmbraSkillbarSlot& SlotData)
{
	SlotIndex = SlotData.SlotIndex;
	SkillID = SlotData.SkillID;
	SkillName = SlotData.SkillName;
	Keybind = SlotData.Keybind;
	CurrentRank = SlotData.CurrentRank;
	ResourceCost = SlotData.ResourceCost;
	ResourceType = SlotData.ResourceType;

	// Atualizar ícone
	if (Skill_Icon && SlotData.IconPath.Len() > 0)
	{
		UTexture2D* LoadedIcon = LoadObject<UTexture2D>(nullptr, *SlotData.IconPath);
		if (LoadedIcon)
		{
			Skill_Icon->SetBrushFromTexture(LoadedIcon);
			SkillIcon = LoadedIcon;
		}
	}

	// Atualizar keybind
	if (Keybind_Text)
	{
		Keybind_Text->SetText(FText::FromString(SlotData.Keybind));
	}

	// Atualizar ranks (visual)
	// TODO: Implementar indicador de rank se necessário

	UE_LOG(LogTemp, Log, TEXT("[SkillSlot %d] Setado com skill %d (%s)"), 
		SlotIndex, SkillID, *SkillName);
}

void UUmbraSkillSlotWidget::UpdateCooldownVisual(float RemainingSeconds, int32 TotalCooldownMs)
{
	if (!Cooldown_Overlay || !Cooldown_Text)
		return;

	bIsOnCooldown = RemainingSeconds > 0.0f;
	CooldownRemaining = RemainingSeconds;

	if (bIsOnCooldown)
	{
		// Mostrar overlay
		Cooldown_Overlay->SetVisibility(ESlateVisibility::Visible);

		// Atualizar texto
		int32 SecondsDisplay = FMath::CeilToInt(RemainingSeconds);
		Cooldown_Text->SetText(FText::AsNumber(SecondsDisplay));

		// Atualizar material (progress = 1 - (remaining / total))
		if (DynamicCooldownMaterial && TotalCooldownMs > 0)
		{
			float Progress = 1.0f - (RemainingSeconds / (TotalCooldownMs / 1000.0f));
			Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
			DynamicCooldownMaterial->SetScalarParameterValue(TEXT("Progress"), Progress);
		}
	}
	else
	{
		ClearCooldown();
	}
}

void UUmbraSkillSlotWidget::ClearCooldown()
{
	if (Cooldown_Overlay)
	{
		Cooldown_Overlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	bIsOnCooldown = false;
	CooldownRemaining = 0.0f;
}

void UUmbraSkillSlotWidget::SetEmpty()
{
	SkillID = 0;
	SkillName = TEXT("");

	if (Skill_Icon)
	{
		Skill_Icon->SetBrushFromTexture(nullptr);
	}

	if (Border_Background)
	{
		Border_Background->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.0f));
	}

	ClearCooldown();
}

void UUmbraSkillSlotWidget::UpdateResourceWarning(bool bHasEnoughResource)
{
	if (Resource_Warning)
	{
		Resource_Warning->SetVisibility(bHasEnoughResource ? 
			ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}
```

### Passo 3.3 - Reparentar WBP_SkillSlot

1. **Abra WBP_SkillSlot no Blueprint Editor**
2. **Clique em "File"** → **Reparent Blueprint**
3. **Search:** `UmbraSkillSlotWidget`
4. **Selecione** `UmbraSkillSlotWidget`
5. **Salvar**

---

## FASE 4: CRIAR WIDGET WBP_SkillBar

### Passo 4.1 - Criar Widget

1. **Content Browser** → **User Interface** → **Widget Blueprint**
2. **Nome:** `WBP_SkillBar`
3. **Salvar**
4. **Duplo-clique para abrir no Designer**

### Passo 4.2 - Estrutura Visual (Designer)

**Hierarchy:**

```
Canvas Panel (Root)
├── Background_Border (Border - fundo semitransparente)
├── MainBar_HBox (Horizontal Box - slots 1-10)
├── SecondaryBar_HBox (Horizontal Box - slots 11-20)
└── BuffBar_HBox (Horizontal Box - buffs)
```

### Passo 4.3 - Configurar Root (Canvas Panel)

1. **Root já é Canvas Panel**
2. **Details:**
   - **Anchor:** Bottom Center (0.5, 1.0)
   - **Position:** (0, -20)
   - **Size X:** 600
   - **Size Y:** 120

### Passo 4.4 - Configurar Background_Border

1. **Add Child** → **Border**
2. **Details:**
   - **Name**: `Background_Border`
   - **Size X**: Fill
   - **Size Y**: Fill
   - **Brush Color**: `#1A1A2E` Alpha = 0.8
   - **Padding**: 8

### Passo 4.5 - Configurar MainBar_HBox (slots 1-10)

1. **Add Child** → **Horizontal Box**
2. **Details:**
   - **Name**: `MainBar_HBox`
   - **Size X**: Fill
   - **Size Y**: Auto
   - **Spacing**: 4
   - **Canvas Slot:**
     - **Position X**: 8
     - **Position Y**: 8
     - **Size X**: 584
     - **Size Y**: 64

3. **Add 10 × WBP_SkillSlot** como filhos
   - Cada um tem slot slot 0-9
   - Size: 64×64
   - Spacing: 4

### Passo 4.6 - Configurar SecondaryBar_HBox (slots 11-20)

1. **Add Child** → **Horizontal Box**
2. **Details:**
   - **Name**: `SecondaryBar_HBox`
   - **Size X**: Fill
   - **Size Y**: Auto
   - **Spacing**: 4
   - **Canvas Slot:**
     - **Position X**: 8
     - **Position Y**: 80
     - **Size X**: 584
     - **Size Y**: 64

3. **Add 10 × WBP_SkillSlot** como filhos
   - Cada um tem slot 10-19
   - Size: 64×64
   - Spacing: 4

### Passo 4.7 - Configurar BuffBar_HBox

1. **Add Child** → **Horizontal Box**
2. **Details:**
   - **Name**: `BuffBar_HBox`
   - **Size X**: Auto
   - **Size Y**: Auto
   - **Spacing**: 2
   - **Canvas Slot:**
     - **Position X**: 8
     - **Position Y**: -40 (acima da skillbar)
     - **Size X**: 300
     - **Size Y**: 32

---

## FASE 5: C++ - CRIAR CLASSE WBP_SkillBar

### Passo 5.1 - Criar Header (UmbraSkillBarWidget.h)

**Local:** `Source/UmbraEternumUE/UI/UmbraSkillBarWidget.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Data/UmbraSkillDataStructures.h"
#include "UmbraSkillBarWidget.generated.h"

class UUmbraSkillSlotWidget;
class UUmbraGameInstance;

UCLASS()
class UMBRAETERNUMUE_API UUmbraSkillBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Widgets containers
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> MainBar_HBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> SecondaryBar_HBox;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> BuffBar_HBox;

	// Array de slots de skill
	UPROPERTY(BlueprintReadWrite, Category = "SkillBar")
	TArray<TObjectPtr<UUmbraSkillSlotWidget>> SkillSlots;

	// Referência ao GameInstance
	UPROPERTY(BlueprintReadWrite, Category = "SkillBar")
	TObjectPtr<UUmbraGameInstance> GameInstanceRef;

	// Funções
	UFUNCTION(BlueprintCallable, Category = "SkillBar")
	void PopulateSkillBar();

	UFUNCTION(BlueprintCallable, Category = "SkillBar")
	void UpdateAllCooldowns(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "SkillBar")
	void OnSkillValidated(int32 SkillID, int32 RequestID);

	UFUNCTION(BlueprintCallable, Category = "SkillBar")
	void OnSkillFailed(int32 SkillID, const FString& ErrorMessage);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	float TickTimer = 0.0f;
	float TickInterval = 0.1f; // Atualizar a cada 100ms
};
```

### Passo 5.2 - Criar Implementation (UmbraSkillBarWidget.cpp)

**Local:** `Source/UmbraEternumUE/UI/UmbraSkillBarWidget.cpp`

```cpp
#include "UmbraSkillBarWidget.h"
#include "UmbraSkillSlotWidget.h"
#include "Core/UmbraGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UUmbraSkillBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Obter GameInstance
	GameInstanceRef = Cast<UUmbraGameInstance>(GetGameInstance());
	if (!GameInstanceRef)
	{
		UE_LOG(LogTemp, Error, TEXT("[SkillBar] GameInstance não encontrado!"));
		return;
	}

	// Obter slots da HBox
	if (MainBar_HBox)
	{
		for (int32 i = 0; i < MainBar_HBox->GetChildrenCount(); ++i)
		{
			UUmbraSkillSlotWidget* Slot = Cast<UUmbraSkillSlotWidget>(MainBar_HBox->GetChildAt(i));
			if (Slot)
			{
				Slot->SlotIndex = i;
				SkillSlots.Add(Slot);
			}
		}
	}

	if (SecondaryBar_HBox)
	{
		for (int32 i = 0; i < SecondaryBar_HBox->GetChildrenCount(); ++i)
		{
			UUmbraSkillSlotWidget* Slot = Cast<UUmbraSkillSlotWidget>(SecondaryBar_HBox->GetChildAt(i));
			if (Slot)
			{
				Slot->SlotIndex = 10 + i;
				SkillSlots.Add(Slot);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[SkillBar] %d slots coletados"), SkillSlots.Num());

	// Bind events
	if (GameInstanceRef)
	{
		GameInstanceRef->OnSkillbarLoaded.AddDynamic(this, &UUmbraSkillBarWidget::PopulateSkillBar);
		GameInstanceRef->OnSkillUseValidated.AddDynamic(this, &UUmbraSkillBarWidget::OnSkillValidated);
		GameInstanceRef->OnSkillUseFailed.AddDynamic(this, &UUmbraSkillBarWidget::OnSkillFailed);

		// Carregar skillbar
		GameInstanceRef->LoadSkillbar();
	}
}

void UUmbraSkillBarWidget::PopulateSkillBar()
{
	if (!GameInstanceRef)
		return;

	UE_LOG(LogTemp, Log, TEXT("[SkillBar] Populando skillbar..."));

	// Atualizar cada slot
	for (UUmbraSkillSlotWidget* Slot : SkillSlots)
	{
		if (!Slot)
			continue;

		int32 SlotIndex = Slot->SlotIndex;
		FUmbraSkillbarSlot SkillbarSlot;

		if (GameInstanceRef->GetSkillbarSlot(SlotIndex, SkillbarSlot))
		{
			Slot->SetSlotData(SkillbarSlot);
		}
		else
		{
			Slot->SetEmpty();
		}
	}
}

void UUmbraSkillBarWidget::UpdateAllCooldowns(float DeltaTime)
{
	if (!GameInstanceRef)
		return;

	for (UUmbraSkillSlotWidget* Slot : SkillSlots)
	{
		if (!Slot || Slot->SkillID <= 0)
			continue;

		float RemainingSeconds = GameInstanceRef->GetSkillCooldownRemaining(Slot->SkillID);
		Slot->UpdateCooldownVisual(RemainingSeconds, 
			GameInstanceRef->SkillCooldownCache.FindRef(Slot->SkillID));
	}
}

void UUmbraSkillBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Atualizar cooldowns em intervals regulares
	TickTimer += InDeltaTime;
	if (TickTimer >= TickInterval)
	{
		UpdateAllCooldowns(InDeltaTime);
		TickTimer = 0.0f;
	}
}

void UUmbraSkillBarWidget::OnSkillValidated(int32 SkillID, int32 RequestID)
{
	UE_LOG(LogTemp, Log, TEXT("[SkillBar] Skill %d validada (RequestID: %d)"), 
		SkillID, RequestID);

	// TODO: Play animation no slot
	// Encontrar slot com este SkillID
	for (UUmbraSkillSlotWidget* Slot : SkillSlots)
	{
		if (Slot && Slot->SkillID == SkillID)
		{
			// Play press animation (scale 0.9 → 1.0)
			break;
		}
	}
}

void UUmbraSkillBarWidget::OnSkillFailed(int32 SkillID, const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("[SkillBar] Skill %d falhou: %s"), 
		SkillID, *ErrorMessage);
}
```

### Passo 5.3 - Reparentar WBP_SkillBar

1. **Abra WBP_SkillBar no Blueprint Editor**
2. **File** → **Reparent Blueprint**
3. **Search:** `UmbraSkillBarWidget`
4. **Selecione** `UmbraSkillBarWidget`
5. **Salvar**

---

## FASE 6: INTEGRAÇÃO NO PLAYERCONTROLLER

### Passo 6.1 - Adicionar Input Binding

**Em UmbraEternumUEPlayerController::BeginPlay():**

```cpp
void AUmbraEternumUEPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Bind skill input
	if (UEnhancedInputComponent* EnhancedInputComponent = 
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Slot 0 (Tecla 1)
		EnhancedInputComponent->BindAction(InputActions[0], ETriggerEvent::Started, 
			this, &AUmbraEternumUEPlayerController::UseSkillSlot0);
		
		// ... repetir para slots 1-19
	}
}

void AUmbraEternumUEPlayerController::UseSkillSlot0()
{
	if (UUmbraGameInstance* GI = Cast<UUmbraGameInstance>(GetGameInstance()))
	{
		GI->UseSkillFromSlot(0, 0);
	}
}

// ... implementar UseSkillSlot1, UseSkillSlot2, etc.
```

### Passo 6.2 - Adicionar Widget à HUD

**No HUD (ou Player State):**

```cpp
void AHUD::BeginPlay()
{
	Super::BeginPlay();

	if (SkillBarWidgetClass)
	{
		SkillBarWidget = CreateWidget<UUmbraSkillBarWidget>(
			GetWorld()->GetFirstPlayerController(), SkillBarWidgetClass);
		if (SkillBarWidget)
		{
			SkillBarWidget->AddToViewport(10);
		}
	}
}
```

---

## FASE 7: TESTES

### Passo 7.1 - Verificar Visual

1. **Play in Editor (PIE)**
2. **Verificar se WBP_SkillBar aparece na tela** (bottom center)
3. **Verificar se os 20 slots estão visíveis**
4. **Verificar se keybinds estão corretos**

### Passo 7.2 - Testar Carregamento

1. **Log deve mostrar:**
   ```
   [SkillBar] 20 slots coletados
   [SkillBar] Populando skillbar...
   ```

2. **Cada slot deve mostrar:**
   - Ícone de skill
   - Nome do skill
   - Keybind

### Passo 7.3 - Testar Cooldown

1. **Usar uma skill (pressionar tecla)**
2. **Verificar se cooldown overlay aparece**
3. **Verificar se material radial atualiza**
4. **Verificar se texto de segundos conta regressiva**

### Passo 7.4 - Testar Recurso

1. **Gastar recurso (mana/vida)**
2. **Verificar se Resource_Warning ativa**
3. **Verificar se skill fica inutilizável visualmente**

---

## CHECKLIST FINAL

- [ ] Material M_CooldownRadial criado
- [ ] Material Instance MI_CooldownRadial criado
- [ ] WBP_SkillSlot estrutura visual completa
- [ ] UmbraSkillSlotWidget (C++) compilando
- [ ] WBP_SkillSlot reparentado para UmbraSkillSlotWidget
- [ ] WBP_SkillBar estrutura visual completa
- [ ] UmbraSkillBarWidget (C++) compilando
- [ ] WBP_SkillBar reparentado para UmbraSkillBarWidget
- [ ] Input actions configuradas (0-19)
- [ ] PlayerController com UseSkillSlot funções
- [ ] HUD adicionando WBP_SkillBar
- [ ] PIE test: Skills aparecem nos slots
- [ ] PIE test: Cooldown funciona
- [ ] PIE test: Keybinds funcionam
- [ ] PIE test: Recurso warning funciona
