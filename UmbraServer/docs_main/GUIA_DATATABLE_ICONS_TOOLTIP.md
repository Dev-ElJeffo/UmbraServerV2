# Guia - DataTable de Ícones e Sistema de Tooltip

## Visão Geral

Este guia cobre a criação e uso do DataTable de ícones de skills e o sistema de tooltip para exibir informações detalhadas.

---

## Estrutura de Pastas para Ícones

```
Content/
└── UI/
    └── Skills/
        ├── Icons/
        │   ├── Barbarian/
        │   │   ├── T_Ruin_Strike.png
        │   │   ├── T_Hardened_Skin.png
        │   │   └── ...
        │   ├── DarkMage/
        │   │   ├── T_Crimson_Mist.png
        │   │   └── ...
        │   ├── Monk/
        │   ├── Templar/
        │   ├── Alchemist/
        │   ├── Assassin/
        │   └── Common/
        │       ├── T_Locked_Skill.png
        │       └── T_Empty_Slot.png
        ├── IconsLarge/
        │   └── (mesma estrutura, resolução maior)
        └── DataTables/
            ├── DT_Skills.uasset
            └── DT_SkillIcons.uasset
```

---

## Especificações de Ícones

### Tamanhos Recomendados

| Uso | Tamanho | Formato |
|-----|---------|---------|
| Slot da Skillbar | 64x64 | PNG 32-bit |
| SkillBook Entry | 48x48 | PNG 32-bit |
| Tooltip | 64x64 | PNG 32-bit |
| Detail Panel (Large) | 128x128 | PNG 32-bit |
| Buff Bar | 32x32 | PNG 32-bit |

### Estilo Visual

- **Background**: Transparente ou com borda consistente
- **Cores por Classe**:
  - Barbarian: Tons de vermelho/laranja
  - Dark Mage: Tons de roxo/carmesim
  - Monk: Tons de cinza/azul
  - Templar: Tons de dourado/branco
  - Alchemist: Tons de verde/azul
  - Assassin: Tons de preto/vermelho escuro

- **Indicadores de Tipo** (opcional no canto):
  - Active: Seta para cima
  - Passive: Círculo
  - Ultimate: Estrela
  - Buff: Seta verde
  - Debuff: Seta vermelha

---

## Criando o DataTable de Ícones

### 1. Criar a Struct (já fornecida em UmbraSkillDataStructures.h)

```cpp
USTRUCT(BlueprintType)
struct FUmbraSkillIconRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconLarge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TooltipText;
};
```

### 2. Criar DataTable no Editor

1. Content Browser → Add → Miscellaneous → Data Table
2. Selecionar Row Structure: `UmbraSkillIconRow`
3. Nomear: `DT_SkillIcons`
4. Salvar em `Content/UI/Skills/DataTables/`

### 3. Popular o DataTable

Para cada skill, adicionar uma row:

| Row Name | SkillKey | Icon | IconLarge | TooltipText |
|----------|----------|------|-----------|-------------|
| BARB_RUIN_STRIKE | BARB_RUIN_STRIKE | T_Ruin_Strike | T_Ruin_Strike_L | Golpe da Ruína |
| BARB_HARDENED_SKIN | BARB_HARDENED_SKIN | T_Hardened_Skin | ... | Pele Endurecida |
| ... | ... | ... | ... | ... |

### 4. Script para Auto-Popular (Editor Utility Widget)

```cpp
// Editor Utility Widget Blueprint
void PopulateIconDataTable()
{
    UDataTable* DT = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Skills/DataTables/DT_SkillIcons"));
    UDataTable* SkillsDT = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Skills/DataTables/DT_Skills"));
    
    TArray<FUmbraSkillData*> AllSkills;
    SkillsDT->GetAllRows<FUmbraSkillData>(TEXT(""), AllSkills);
    
    for (FUmbraSkillData* Skill : AllSkills)
    {
        FUmbraSkillIconRow NewRow;
        NewRow.SkillKey = Skill->SkillKey;
        
        // Construir caminho do ícone
        FString IconPath = FString::Printf(TEXT("/Game/UI/Skills/Icons/%s/%s"), 
            *Skill->ClassName, *Skill->IconPath);
        
        NewRow.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));
        NewRow.IconLarge = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath + TEXT("_L")));
        NewRow.TooltipText = Skill->SkillName;
        
        DT->AddRow(FName(*Skill->SkillKey), NewRow);
    }
}
```

---

## Sistema de Tooltip

### WBP_SkillTooltip - Estrutura

```
WBP_SkillTooltip (UserWidget)
├── SizeBox (MinWidth: 300, MaxWidth: 400)
│   └── Border (Background com cantos arredondados)
│       └── VerticalBox
│           │
│           ├── Header_HBox
│           │   ├── Icon_Image (48x48)
│           │   ├── VBox_Title
│           │   │   ├── Name_Text (Bold, 16pt)
│           │   │   └── Type_Text (12pt, cor por tipo)
│           │   └── Level_Badge
│           │
│           ├── Separator_Image
│           │
│           ├── Element_HBox
│           │   ├── Element_Icon (16x16)
│           │   └── Element_Text
│           │
│           ├── Stats_Grid (2 colunas)
│           │   ├── "Custo:" | Cost_Text
│           │   ├── "Cooldown:" | CD_Text
│           │   ├── "Alcance:" | Range_Text
│           │   ├── "Duração:" | Duration_Text
│           │   └── "Scaling:" | Scaling_Text
│           │
│           ├── Separator_Image
│           │
│           ├── Description_RichText (Wrap)
│           │
│           ├── Separator_Image (se tiver rank info)
│           │
│           ├── Rank_HBox
│           │   ├── "Rank:" 
│           │   ├── Current_Text
│           │   ├── "/"
│           │   └── Max_Text
│           │
│           └── Requirement_Box (se bloqueado)
│               └── Requirement_Text (vermelho)
```

### Variáveis do Tooltip

```cpp
// Em WBP_SkillTooltip
UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UImage* Icon_Image;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Name_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Type_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Element_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Cost_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* CD_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Range_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
URichTextBlock* Description_RichText;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Rank_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* Requirement_Text;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UVerticalBox* Requirement_Box;
```

### Função: SetSkillData

```cpp
void UWBPSkillTooltip::SetSkillData(const FUmbraSkillData& Skill, int32 CurrentRank, bool bIsLocked, int32 PlayerLevel)
{
    // Nome
    Name_Text->SetText(FText::FromString(Skill.SkillName));
    
    // Tipo com cor
    Type_Text->SetText(FText::FromString(Skill.SkillType));
    Type_Text->SetColorAndOpacity(GetTypeColor(Skill.SkillType));
    
    // Elemento
    Element_Text->SetText(FText::FromString(Skill.Element));
    Element_Text->SetColorAndOpacity(FLinearColor(FColor::FromHex(Skill.ElementColor)));
    
    // Custo
    if (Skill.ResourceCost > 0)
    {
        Cost_Text->SetText(FText::Format(LOCTEXT("Cost", "{0} {1}"), 
            Skill.ResourceCost, FText::FromString(Skill.ResourceType)));
    }
    else
    {
        Cost_Text->SetText(LOCTEXT("NoCost", "Sem custo"));
    }
    
    // Cooldown
    float CDSeconds = Skill.GetCooldownSeconds();
    if (CDSeconds > 0)
    {
        CD_Text->SetText(FText::Format(LOCTEXT("Cooldown", "{0}s"), CDSeconds));
    }
    else
    {
        CD_Text->SetText(LOCTEXT("NoCooldown", "Sem cooldown"));
    }
    
    // Range
    if (Skill.RangeMax > 0)
    {
        Range_Text->SetText(FText::Format(LOCTEXT("Range", "{0}m"), Skill.RangeMax / 100.0f));
    }
    else
    {
        Range_Text->SetText(LOCTEXT("Melee", "Corpo a corpo"));
    }
    
    // Descrição com formatação
    FString FormattedDesc = FormatDescription(Skill.Description, Skill, CurrentRank);
    Description_RichText->SetText(FText::FromString(FormattedDesc));
    
    // Rank
    if (CurrentRank > 0)
    {
        Rank_Text->SetText(FText::Format(LOCTEXT("Rank", "Rank {0}/{1}"), CurrentRank, Skill.MaxRank));
        Rank_Text->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Rank_Text->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    // Requisito
    if (bIsLocked)
    {
        Requirement_Text->SetText(FText::Format(LOCTEXT("Req", "Requer Nível {0}"), Skill.RequiredLevel));
        Requirement_Box->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Requirement_Box->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    // Ícone
    SetIconFromDataTable(Skill.SkillKey);
}

FString UWBPSkillTooltip::FormatDescription(const FString& Template, const FUmbraSkillData& Skill, int32 Rank)
{
    FString Result = Template;
    
    // Calcular damage com rank
    float PowerMultiplier = 1.0f + ((Rank - 1) * 0.1f);
    int32 EffectivePower = FMath::RoundToInt(Skill.PowerCoef * PowerMultiplier);
    
    // Substituir placeholders
    Result = Result.Replace(TEXT("{damage}"), *FString::FromInt(EffectivePower));
    Result = Result.Replace(TEXT("{duration}"), *FString::SanitizeFloat(Skill.GetDurationSeconds()));
    Result = Result.Replace(TEXT("{secondary}"), *FString::FromInt(Skill.SecondaryCoef));
    
    // Destacar números com RichText
    // Ex: <damage>150</> vira texto colorido
    
    return Result;
}

FLinearColor UWBPSkillTooltip::GetTypeColor(const FString& Type)
{
    if (Type == TEXT("ACTIVE")) return FLinearColor(0.29f, 0.56f, 0.85f); // Azul
    if (Type == TEXT("PASSIVE")) return FLinearColor(0.54f, 0.54f, 0.54f); // Cinza
    if (Type == TEXT("BUFF")) return FLinearColor(0.30f, 0.69f, 0.31f); // Verde
    if (Type == TEXT("DEBUFF")) return FLinearColor(0.61f, 0.15f, 0.69f); // Roxo
    if (Type == TEXT("ULTIMATE")) return FLinearColor(1.0f, 0.84f, 0.0f); // Dourado
    if (Type == TEXT("REACTION")) return FLinearColor(1.0f, 0.60f, 0.0f); // Laranja
    if (Type == TEXT("DOT")) return FLinearColor(0.80f, 0.20f, 0.20f); // Vermelho
    if (Type == TEXT("HOT")) return FLinearColor(0.20f, 0.80f, 0.40f); // Verde claro
    return FLinearColor::White;
}
```

---

## Mostrando Tooltip

### No Widget de Skill (SkillBookEntry ou SkillSlot)

```cpp
// Event: OnMouseEnter
void ShowTooltip()
{
    if (!TooltipWidget)
    {
        TooltipWidget = CreateWidget<UWBPSkillTooltip>(GetWorld(), TooltipClass);
    }
    
    TooltipWidget->SetSkillData(CurrentSkillData, CurrentRank, bIsLocked, PlayerLevel);
    TooltipWidget->AddToViewport(100); // High Z-order
    
    // Posicionar próximo ao cursor
    PositionTooltipAtCursor();
}

// Event: OnMouseLeave
void HideTooltip()
{
    if (TooltipWidget && TooltipWidget->IsInViewport())
    {
        TooltipWidget->RemoveFromParent();
    }
}

void PositionTooltipAtCursor()
{
    FVector2D MousePos;
    GetOwningPlayer()->GetMousePosition(MousePos.X, MousePos.Y);
    
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    
    // Offset do cursor
    FVector2D TooltipPos = MousePos + FVector2D(20, 20);
    
    // Ajustar se sair da tela
    FVector2D TooltipSize = TooltipWidget->GetDesiredSize();
    
    if (TooltipPos.X + TooltipSize.X > ViewportSize.X)
    {
        TooltipPos.X = MousePos.X - TooltipSize.X - 10;
    }
    
    if (TooltipPos.Y + TooltipSize.Y > ViewportSize.Y)
    {
        TooltipPos.Y = MousePos.Y - TooltipSize.Y - 10;
    }
    
    TooltipWidget->SetPositionInViewport(TooltipPos);
}
```

### Blueprint: Mostrar Tooltip

```
Event OnMouseEnter
    ↓
[Branch: Has Skill Data?]
    ↓ Yes
[Create Widget] WBP_SkillTooltip
    ↓
[Call SetSkillData] with CurrentSkillData
    ↓
[Add to Viewport] Z-Order: 100
    ↓
[Get Mouse Position]
    ↓
[Set Position in Viewport] MousePos + (20, 20)
    ↓
[Store Reference] TooltipWidget

Event OnMouseLeave
    ↓
[Branch: TooltipWidget Valid?]
    ↓ Yes
[Remove from Parent]
```

---

## Carregando Ícones do DataTable

### Função: GetSkillIcon

```cpp
UTexture2D* USkillIconHelper::GetSkillIcon(const FString& SkillKey, bool bLarge)
{
    static UDataTable* IconTable = nullptr;
    
    if (!IconTable)
    {
        IconTable = LoadObject<UDataTable>(nullptr, 
            TEXT("/Game/UI/Skills/DataTables/DT_SkillIcons.DT_SkillIcons"));
    }
    
    if (!IconTable) return GetDefaultIcon();
    
    FUmbraSkillIconRow* Row = IconTable->FindRow<FUmbraSkillIconRow>(FName(*SkillKey), TEXT(""));
    
    if (Row)
    {
        TSoftObjectPtr<UTexture2D>& IconRef = bLarge ? Row->IconLarge : Row->Icon;
        if (!IconRef.IsNull())
        {
            return IconRef.LoadSynchronous();
        }
    }
    
    return GetDefaultIcon();
}

UTexture2D* USkillIconHelper::GetDefaultIcon()
{
    static UTexture2D* DefaultIcon = nullptr;
    
    if (!DefaultIcon)
    {
        DefaultIcon = LoadObject<UTexture2D>(nullptr,
            TEXT("/Game/UI/Skills/Icons/Common/T_Empty_Slot.T_Empty_Slot"));
    }
    
    return DefaultIcon;
}
```

### Blueprint Function Library

```
// BP_SkillIconLibrary

Function: GetSkillIcon
    Input: SkillKey (String), bLarge (Bool)
    Output: Texture2D
    
    [Get DataTable] DT_SkillIcons
        ↓
    [Find Row] SkillKey
        ↓
    [Branch: Row Found?]
        ↓ Yes
    [Load Asset] Icon or IconLarge
        ↓
    Return Texture
        
        ↓ No
    Return Default Icon
```

---

## Checklist de Implementação

### Ícones
- [ ] Criar pasta estruturada para ícones
- [ ] Criar/importar 90 ícones (1 por skill)
- [ ] Criar versões large (128x128)
- [ ] Criar ícones comuns (empty, locked)

### DataTables
- [ ] DT_Skills importado do CSV
- [ ] DT_SkillIcons criado
- [ ] DT_SkillIcons populado

### Tooltip
- [ ] WBP_SkillTooltip criado
- [ ] Layout configurado
- [ ] SetSkillData funcionando
- [ ] FormatDescription com placeholders
- [ ] Cores por tipo implementadas

### Integração
- [ ] Tooltip mostrando no hover
- [ ] Tooltip posicionado corretamente
- [ ] Ícones carregando do DataTable
- [ ] Performance OK (async loading)

---

## Dicas de Performance

1. **Soft References**: Use `TSoftObjectPtr` para ícones
2. **Async Loading**: Carregue ícones em background
3. **Caching**: Mantenha ícones frequentes em memória
4. **Pooling**: Reutilize widgets de tooltip
5. **Lazy Load**: Carregue DataTable apenas quando necessário
