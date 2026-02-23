# Guia de Implementação - WBP_SkillBar (HUD)

## Visão Geral

A Skill Bar é a interface principal de combate, exibindo skills equipadas com cooldowns visuais, custos de recursos e atalhos de teclado.

---

## Estrutura de Widgets

```
WBP_SkillBar (UserWidget)
├── Canvas Panel (Root)
│   ├── Background_Border (fundo semi-transparente)
│   │
│   ├── MainBar_HBox (slots 1-10)
│   │   └── [WBP_SkillSlot] x 10
│   │
│   ├── SecondaryBar_HBox (slots 11-20)
│   │   └── [WBP_SkillSlot] x 10
│   │
│   └── BuffBar_HBox (opcional - buffs ativos)
│       └── [WBP_BuffIcon] x N (dinâmico)
```

---

## WBP_SkillSlot (Slot Individual)

### Estrutura Visual

```
WBP_SkillSlot (UserWidget)
├── SizeBox (64x64 ou 48x48)
│   └── Overlay
│       ├── Border_Background
│       │
│       ├── Skill_Icon (Image)
│       │
│       ├── Cooldown_Overlay
│       │   ├── Cooldown_Fill (Material radial)
│       │   └── Cooldown_Text (segundos restantes)
│       │
│       ├── Resource_Warning (pisca se sem mana)
│       │
│       ├── Keybind_Text (canto inferior direito)
│       │
│       ├── Rank_Indicator (pequenas estrelas)
│       │
│       └── Unusable_Overlay (quando stunned/silenced)
```

### Variáveis do Slot

```cpp
UPROPERTY(BlueprintReadWrite)
int32 SlotIndex;

UPROPERTY(BlueprintReadWrite)
int32 SkillID;

UPROPERTY(BlueprintReadWrite)
FString SkillKey;

UPROPERTY(BlueprintReadWrite)
FString SkillName;

UPROPERTY(BlueprintReadWrite)
int32 CurrentRank;

UPROPERTY(BlueprintReadWrite)
FString Keybind;

UPROPERTY(BlueprintReadWrite)
int32 CooldownMs;

UPROPERTY(BlueprintReadWrite)
int32 ResourceCost;

UPROPERTY(BlueprintReadWrite)
FString ResourceType;

UPROPERTY(BlueprintReadWrite)
UTexture2D* SkillIcon;

// Estado runtime
UPROPERTY(BlueprintReadWrite)
bool bIsOnCooldown;

UPROPERTY(BlueprintReadWrite)
float CooldownRemainingSeconds;

UPROPERTY(BlueprintReadWrite)
float CooldownProgress; // 0-1

UPROPERTY(BlueprintReadWrite)
bool bHasEnoughResource;

UPROPERTY(BlueprintReadWrite)
bool bCanUse;
```

---

## Material de Cooldown (Radial)

### Criar Material M_CooldownRadial

1. **Criar Material** em Content/Materials/UI/
2. **Configurar como Unlit, Masked**
3. **Nós do Material**:

```
// Parâmetro de progresso
ScalarParameter "Progress" (0-1)
    ↓
[Atan2 de UVs] → Normalizar para 0-1
    ↓
[Compare] Progress > Angle
    ↓
[Lerp] Cor cheia / Cor escurecida
    ↓
Output: Emissive Color

// Máscara circular
[RadialGradientExponential]
    ↓
Output: Opacity Mask
```

4. **Criar Material Instance** MI_CooldownRadial
5. **Expor parâmetro** "Progress" para Blueprint

### Uso no Blueprint

```
UpdateCooldown(RemainingMs, TotalMs)
    ↓
[Calculate Progress] = 1 - (Remaining / Total)
    ↓
[Set Scalar Parameter] "Progress" on MI_CooldownRadial
    ↓
[Update Cooldown_Text] = Format("{0}s", Ceiling(Remaining / 1000))
    ↓
[Set Visibility] Cooldown_Overlay = (Remaining > 0)
```

---

## Sistema de Keybinds

### Configuração Padrão (20 slots)

| Slot | Keybind | Slot | Keybind |
|------|---------|------|---------|
| 0 | 1 | 10 | Ctrl+1 |
| 1 | 2 | 11 | Ctrl+2 |
| 2 | 3 | 12 | Ctrl+3 |
| 3 | 4 | 13 | Ctrl+4 |
| 4 | 5 | 14 | Ctrl+5 |
| 5 | Q | 15 | Ctrl+Q |
| 6 | E | 16 | Ctrl+E |
| 7 | R | 17 | Ctrl+R |
| 8 | F | 18 | Ctrl+F |
| 9 | T | 19 | Ctrl+T |

### Input Handling

```cpp
// No PlayerController ou Character
void SetupSkillBarInput()
{
    // Slot 0 (tecla 1)
    InputComponent->BindAction("SkillSlot1", IE_Pressed, this, &AMyCharacter::UseSkillSlot<0>);
    // ... repetir para outros slots
}

template<int32 SlotIndex>
void UseSkillSlot()
{
    if (UUmbraGameInstance* GI = GetGameInstance<UUmbraGameInstance>())
    {
        GI->UseSkillFromSlot(SlotIndex);
    }
}
```

---

## Integração com GameInstance

### Funções Disponíveis no GameInstance (já implementadas)

O GameInstance possui toda a integração necessária para a Skillbar:

```cpp
// ========== FUNÇÕES DISPONÍVEIS ==========

// Carregar skillbar do servidor
UFUNCTION(BlueprintCallable, Category = "Skills")
void LoadSkillbar();

// Configurar skill em slot
UFUNCTION(BlueprintCallable, Category = "Skills")
void SetSkillbarSlot(int32 SlotIndex, int32 SkillID, const FString& Keybind = TEXT(""));

// Usar skill de um slot (com validação local)
UFUNCTION(BlueprintCallable, Category = "Skills")
void UseSkillFromSlot(int32 SlotIndex, int32 TargetID = 0);

// Usar skill diretamente
UFUNCTION(BlueprintCallable, Category = "Skills")
void UseSkill(int32 SkillID, int32 TargetID = 0);

// Carregar cooldowns do servidor
UFUNCTION(BlueprintCallable, Category = "Skills")
void LoadCooldowns();

// Carregar buffs ativos
UFUNCTION(BlueprintCallable, Category = "Skills")
void LoadActiveBuffs();

// Verificar cooldown local
UFUNCTION(BlueprintPure, Category = "Skills")
bool IsSkillOnCooldown(int32 SkillID) const;

// Obter tempo restante de cooldown
UFUNCTION(BlueprintPure, Category = "Skills")
float GetSkillCooldownRemaining(int32 SkillID) const;

// Obter slot da skillbar
UFUNCTION(BlueprintPure, Category = "Skills")
bool GetSkillbarSlot(int32 SlotIndex, FUmbraSkillbarSlot& OutSlot) const;

// Atualizar cooldowns locais (chamar no Tick)
UFUNCTION(BlueprintCallable, Category = "Skills")
void UpdateLocalCooldowns(float DeltaTime);

// ========== DELEGATES DISPONÍVEIS ==========

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillbarLoaded OnSkillbarLoaded;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillbarLoadFailed OnSkillbarLoadFailed;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillbarSlotUpdated OnSkillbarSlotUpdated;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillUseValidated OnSkillUseValidated;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillUseFailed OnSkillUseFailed;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnCooldownsLoaded OnCooldownsLoaded;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnActiveBuffsLoaded OnActiveBuffsLoaded;

// ========== DADOS ARMAZENADOS ==========

// Configuração da skillbar (20 slots)
UPROPERTY(BlueprintReadOnly, Category = "Skills")
TArray<FUmbraSkillbarSlot> SkillbarSlots;

// Cooldowns ativos
UPROPERTY(BlueprintReadOnly, Category = "Skills")
TArray<FUmbraActiveCooldown> ActiveCooldowns;

// Buffs ativos
UPROPERTY(BlueprintReadOnly, Category = "Skills")
TArray<FUmbraActiveBuff> ActiveBuffs;
```

### Struct FUmbraSkillbarSlot (definido em UmbraSkillDataStructures.h)

```cpp
USTRUCT(BlueprintType)
struct FUmbraSkillbarSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 SlotIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 SkillID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString SkillKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString Keybind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 CurrentRank = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 CooldownMs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 ResourceCost = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString IconPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsEmpty = true;

    // Runtime state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    bool bIsOnCooldown = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
    float CooldownRemaining = 0.0f;
};
```

### Uso no Blueprint - WBP_SkillBar

```
Event Construct
    ↓
[Get Game Instance] → Cast to UmbraGameInstance
    ↓
[Store Reference] → GameInstanceRef
    ↓
[Bind to OnSkillbarLoaded]
    → Callback: PopulateSkillbar
    ↓
[Bind to OnSkillUseValidated]
    → Callback: PlaySkillAnimation + StartCooldownVisual
    ↓
[Bind to OnSkillUseFailed]
    → Callback: ShowErrorMessage
    ↓
[Call LoadSkillbar]
```

### Atualizando Cooldowns (Event Tick)

```
Event Tick (DeltaTime)
    ↓
[Get Game Instance] → Update Local Cooldowns (DeltaTime)
    ↓
[For Each Slot in SlotWidgets]
    ↓
    [Get SkillID from Slot]
        ↓
    [Get Game Instance] → Is Skill On Cooldown (SkillID)
        ↓
    [Branch]
        |
        [True] → [Get Skill Cooldown Remaining (SkillID)]
                    → [Update Cooldown Visual]
        |
        [False] → [Hide Cooldown Overlay]
```

---

## Fluxo de Uso de Skill

### 1. Jogador Pressiona Tecla

```
[Input Event] → UseSkillFromSlot(SlotIndex, TargetID)
    ↓
[GameInstance Validates Locally]
    - Slot não vazio?
    - Não está em cooldown? (LocalCooldowns)
    - Tem recurso suficiente? (CurrentCharacterInfo)
    ↓
[If Valid] → [Send to Server via use_skill API]
    ↓
[Server Validates]
    - Cooldown real
    - Recursos reais
    - Range/Estado
    ↓
[Server Responds]
    ↓
[Success] → OnSkillUseValidated.Broadcast(SkillID, RequestID)
            [StartLocalCooldown automatically called]
            
[Fail] → OnSkillUseFailed.Broadcast(SkillID, ErrorMessage)
```

### 2. Implementação no Blueprint (PlayerController ou Character)

```
// Setup Input (BeginPlay)
[Bind Action "SkillSlot1"] → OnPressed: UseSlot0
[Bind Action "SkillSlot2"] → OnPressed: UseSlot1
... (repetir para slots 0-19)

// Função UseSlot0
UseSlot0
    ↓
[Get Game Instance] → Use Skill From Slot (0, GetTargetID())
```

### 3. Tratando Resposta no Blueprint

```
// Bind no WBP_SkillBar
[Get Game Instance] → Bind Event to OnSkillUseValidated
    ↓
Callback: OnSkillValidated(SkillID, RequestID)
    ↓
[Find Slot with SkillID]
    ↓
[Play Press Animation]
    - Scale down 0.9
    - Scale back 1.0
    ↓
[Play Border Flash (Element Color)]
    ↓
[Play Sound Effect]
```

### 4. Código Existente (UseSkillFromSlot - já implementado)

```cpp
void UUmbraGameInstance::UseSkillFromSlot(int32 SlotIndex, int32 TargetID)
{
    if (SlotIndex < 0 || SlotIndex >= SkillbarSlots.Num())
        return;

    const FUmbraSkillbarSlot& Slot = SkillbarSlots[SlotIndex];
    
    if (Slot.bIsEmpty || Slot.SkillID <= 0)
        return;

    // Verificar cooldown local
    if (IsSkillOnCooldown(Slot.SkillID))
    {
        float Remaining = GetSkillCooldownRemaining(Slot.SkillID);
        OnSkillUseFailed.Broadcast(Slot.SkillID, 
            FString::Printf(TEXT("Cooldown: %.1fs"), Remaining));
        return;
    }

    // Verificar recurso local
    bool bHasResource = true;
    if (Slot.ResourceType == TEXT("MANA"))
        bHasResource = CurrentCharacterInfo.Stats.CurrentMP >= Slot.ResourceCost;
    else if (Slot.ResourceType == TEXT("HEALTH"))
        bHasResource = CurrentCharacterInfo.Stats.CurrentHP > Slot.ResourceCost;

    if (!bHasResource)
    {
        OnSkillUseFailed.Broadcast(Slot.SkillID, TEXT("Recurso insuficiente"));
        return;
    }

    // Enviar para servidor
    UseSkill(Slot.SkillID, TargetID);
}
```

---

## Atualização de Cooldowns

### Tick do Widget

```
Event Tick (WBP_SkillBar)
    ↓
[For Each Slot with Cooldown]
    ↓
    [Decrease Remaining by DeltaTime]
        ↓
    [If Remaining <= 0]
        - Hide Cooldown Overlay
        - Remove from ActiveCooldowns
        ↓
    [Else]
        - Update Progress Material
        - Update Text
```

### Sincronização com Servidor

```
// A cada 5-10 segundos, ou ao reconectar
RefreshCooldownsFromServer()
    ↓
[Call get_cooldowns API]
    ↓
[For Each Cooldown in Response]
    - Ajustar timer local para valor do servidor
```

---

## Drag & Drop de Skills

### Do SkillBook para SkillBar

```
WBP_SkillBookEntry - OnDragDetected
    ↓
[Create DragDropOperation]
    - Payload = SkillID
    - DefaultDragVisual = Skill Icon
    ↓
Return Operation
```

```
WBP_SkillSlot - OnDrop
    ↓
[Get Payload (SkillID)]
    ↓
[Call SetSkillbarSlot(SlotIndex, SkillID)]
    ↓
[Update Slot Visual]
```

### Entre Slots da SkillBar

```
WBP_SkillSlot - OnDragDetected
    ↓
[If Has Skill]
    [Create DragDropOperation]
        - Payload = {SourceSlot, SkillID}
    ↓
Return Operation
```

```
WBP_SkillSlot - OnDrop
    ↓
[Get Payload]
    ↓
[If Same Slot] → Ignore
    ↓
[Swap Skills]
    - Call SetSkillbarSlot(TargetSlot, SourceSkillID)
    - Call SetSkillbarSlot(SourceSlot, TargetSkillID or null)
```

---

## Visual States

### Estados do Slot

| Estado | Visual |
|--------|--------|
| Empty | Borda escura, sem ícone |
| Ready | Ícone colorido, borda normal |
| On Cooldown | Ícone escurecido, overlay radial |
| No Resource | Ícone com tint vermelho, pisca |
| Unusable (Stunned) | Overlay vermelho, "X" |

### Feedback de Uso

```
OnSkillUsed(SkillID)
    ↓
[Find Slot with SkillID]
    ↓
[Play Press Animation]
    - Scale down 0.9
    - Scale back 1.0
    - Duration: 0.1s
    ↓
[Play Border Flash]
    - Color: Element Color
    - Duration: 0.2s
```

---

## Barra de Buffs (Opcional)

### WBP_BuffIcon

```
WBP_BuffIcon (UserWidget)
├── SizeBox (32x32)
│   └── Overlay
│       ├── Icon_Image
│       ├── Stacks_Text (se stackable)
│       ├── Duration_Progress (barra horizontal)
│       └── Type_Border (verde=buff, vermelho=debuff)
```

### Atualização de Buffs

```
// Chamar periodicamente ou quando server envia update
RefreshActiveBuffs()
    ↓
[Call get_active_buffs API]
    ↓
[Clear BuffBar]
    ↓
[For Each Buff]
    - Create WBP_BuffIcon
    - Set Icon, Duration, Stacks
    - Add to BuffBar_HBox
```

---

## Positioning na HUD

### Layout Recomendado

```
┌────────────────────────────────────────────┐
│                                            │
│   [Buffs: ▪▪▪▪▪]                          │
│                                            │
│                                            │
│                                            │
│                                            │
│                                            │
│                                            │
│   [HP Bar]        [MP Bar]                 │
│   [1][2][3][4][5][Q][E][R][F][T]          │
│   [Ctrl+1][Ctrl+2]...[Ctrl+T]             │
└────────────────────────────────────────────┘
```

### Ancoragem

```
WBP_SkillBar
    - Anchor: Bottom Center
    - Alignment: (0.5, 1.0)
    - Position: (0, -20) // 20px do bottom
```

---

## Checklist de Implementação

### Widgets
- [ ] WBP_SkillBar criado
- [ ] WBP_SkillSlot criado
- [ ] WBP_BuffIcon criado (opcional)
- [ ] M_CooldownRadial material
- [ ] MI_CooldownRadial instance

### Funcionalidades
- [ ] Carregar skillbar do servidor
- [ ] Exibir skills nos slots
- [ ] Cooldown visual funcional
- [ ] Indicador de recurso
- [ ] Keybinds funcionando
- [ ] Drag & drop entre slots
- [ ] Drag do SkillBook

### Integração
- [ ] Funções no GameInstance
- [ ] Input mapping configurado
- [ ] Requests HTTP funcionando
- [ ] Sincronização de cooldowns

### Polish
- [ ] Animações de uso
- [ ] Feedback visual
- [ ] Sons de UI
- [ ] Tooltips ao hover
