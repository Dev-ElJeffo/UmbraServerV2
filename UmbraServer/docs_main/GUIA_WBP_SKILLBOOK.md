# Guia de Implementação - WBP_SkillBook

## Visão Geral

O Skill Book é a interface onde o jogador visualiza todas as skills disponíveis para sua classe, aprende novas skills usando pontos de skill e melhora skills existentes.

**Procedimento passo a passo no Editor:** use **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md** para criar os widgets com tipos e nomes exatos, e para ligar eventos e funções.

---

## Estrutura de Widgets (tipos e conteúdo)

Cada nó indica o **tipo de widget** do Unreal e o que colocar dentro. Os **nomes na Hierarchy devem ser exatos** (ver tabela no procedimento).

### Raiz e fundo

| Widget | Tipo no Unreal | O que é / O que fazer |
|--------|-----------------|------------------------|
| **CanvasPanel_Root** | **Canvas Panel** | Raiz do widget. Mantenha como root. Permite posicionar filhos por Anchors/Offsets. |
| **Background_Image** | **Image** | Filho do Canvas. Fundo do painel. Anchors = preencher tela; Color & Opacity = preto/cinza com Alpha ~0,85. Opcional: Brush com textura de painel. |

### Header_Panel (cabeçalho)

| Widget | Tipo no Unreal | O que é / O que fazer |
|--------|----------------|------------------------|
| **Header_Panel** | **Border** | Filho do Canvas. Container do cabeçalho com fundo e padding. Anchors = Top Center; Size Y ≈ 60; Padding (L,R,T,B); Brush Color = escuro (#1A1A2E). |
| **Header_HBox** | **Horizontal Box** | Único filho do Header_Panel. Organiza título, classe, pontos e botão em uma linha. |
| **Title_Text** | **Text Block** | Filho de Header_HBox. Texto fixo: "Livro de Skills". Fonte 24. |
| **Class_Text** | **Text Block** | Filho de Header_HBox. Nome da classe do personagem (preenchido por Blueprint). Fonte 18. |
| **SkillPoints_Text** | **Text Block** | Filho de Header_HBox. "Pontos: X" (Blueprint atualiza com SkillPoints.Available). |
| **Close_Button** | **Button** | Filho de Header_HBox. Texto "X" ou "Fechar". On Clicked → fechar o widget (Remove From Parent ou função Close). |

Entre os elementos do Header use **Spacer** (Width 20) para separar; um Spacer com **Size = Fill** antes do Close_Button empurra o botão para a direita.

### Content_Panel (área central: lista + detalhes)

| Widget | Tipo no Unreal | O que é / O que fazer |
|--------|----------------|------------------------|
| **Content_Panel** | **Horizontal Box** | Filho do Canvas. Divide a tela em duas colunas: esquerda = lista de skills; direita = detalhes. Anchors/Position para ficar abaixo do header. |
| **List_Column_VBox** | **Vertical Box** | Primeiro filho de Content_Panel. Coluna da esquerda. Slot: Size X 380–450. |
| **Filter_Panel** | **Horizontal Box** | Primeiro filho de List_Column_VBox. Contém os botões de filtro (Todos, Ativas, Passivas, Aprendidas). |
| **Filter_All_Button** | **Button** | Texto "Todos". |
| **Filter_Active_Button** | **Button** | Texto "Ativas". |
| **Filter_Passive_Button** | **Button** | Texto "Passivas". |
| **Filter_Learned_Button** | **Button** | Texto "Aprendidas". |
| **Skills_ScrollBox** | **Scroll Box** | Segundo filho de List_Column_VBox. Lista rolável. Em runtime, adicionar aqui os **WBP_SkillBookEntry** (Create Widget + Add Child). Size = Fill. |
| **Detail_Panel** | **Border** | Segundo filho de Content_Panel. Painel de detalhes à direita. Brush Color diferente (ex.: #252535). Padding 16. Slot: Size = Fill. |
| **Detail_VBox** | **Vertical Box** | Único filho de Detail_Panel. Empilha ícone, nome, tipo, descrição, stats, scaling, rank e botão de ação. |

### Detail_Panel – conteúdo (detalhes da skill)

Todos são filhos de **Detail_VBox**. Use **Spacer** (Height 8–20) entre blocos para espaçamento.

| Widget | Tipo no Unreal | O que é / O que fazer |
|--------|----------------|------------------------|
| **Skill_Icon_Large** | **Image** | Ícone grande (96×96 ou 128×128). Brush setado por Blueprint (Get Skill Icon com bLarge = true). |
| **Skill_Name_Text** | **Text Block** | Nome da skill. Fonte 22, negrito. |
| **Skill_Type_Text** | **Text Block** | Ex.: "Ativa • Físico". Fonte 14. |
| **Skill_Description_Text** | **Text Block** | Descrição longa. Auto Wrap Text = true. |
| **Skill_Stats_Panel** | **Horizontal Box** ou **Wrap Box** | Container em linha para: Cost_Text, Cooldown_Text, Range_Text, Duration_Text. |
| **Cost_Text** | **Text Block** | "Custo: 10 Mana". |
| **Cooldown_Text** | **Text Block** | "CD: 5s". |
| **Range_Text** | **Text Block** | "Alcance: 100". |
| **Duration_Text** | **Text Block** | "Duração: 10s". |
| **Scaling_Text** | **Text Block** | "Escala: STR 60%, VIT 30%" (montado no Blueprint). |
| **Rank_Panel** | **Horizontal Box** | Contém CurrentRank_Text + RankStars_HBox. |
| **CurrentRank_Text** | **Text Block** | "Rank 2/5". |
| **RankStars_HBox** | **Horizontal Box** | 5× Image (estrela). Ativar/desativar por CurrentRank no Blueprint. |
| **Action_Button** | **Button** | Texto dinâmico: "Aprender (1 pt)" / "Melhorar (1 pt)" / "Rank Máximo" / "Requer Nível X". On Clicked → Learn Skill ou Upgrade Skill no Game Instance. |

### Árvore resumida (tipos)

```
WBP_SkillBook (UserWidget)
└── Canvas Panel (Root)
    ├── Image (Background_Image)
    ├── Border (Header_Panel)
    │   └── Horizontal Box (Header_HBox)
    │       ├── Text Block (Title_Text)
    │       ├── Spacer, Text Block (Class_Text), Spacer, Text Block (SkillPoints_Text), Spacer [Fill], Button (Close_Button)
    │
    └── Horizontal Box (Content_Panel)
        ├── Vertical Box (List_Column_VBox)
        │   ├── Horizontal Box (Filter_Panel) → Buttons: Filter_All, Filter_Active, Filter_Passive, Filter_Learned
        │   └── Scroll Box (Skills_ScrollBox)  ← [WBP_SkillBookEntry] adicionados em runtime
        │
        └── Border (Detail_Panel)
            └── Vertical Box (Detail_VBox)
                ├── Image (Skill_Icon_Large)
                ├── Text Block (Skill_Name_Text), Skill_Type_Text, Skill_Description_Text
                ├── Horizontal Box (Skill_Stats_Panel) → Cost_Text, Cooldown_Text, Range_Text, Duration_Text
                ├── Text Block (Scaling_Text)
                ├── Horizontal Box (Rank_Panel) → CurrentRank_Text, RankStars_HBox
                └── Button (Action_Button)
```

---

## WBP_SkillBookEntry (Entrada Individual)

Cada linha da lista de skills é uma instância deste widget. Adicionado em runtime como filho de **Skills_ScrollBox**.

### Tipos de widget e conteúdo

| Widget | Tipo no Unreal | O que é / O que fazer |
|--------|----------------|------------------------|
| **Entry_HBox** | **Horizontal Box** | Raiz recomendada. Uma linha horizontal. Padding 4–8; Height 72–80. |
| **Skill_Icon** | **Image** | Ícone 64×64. Brush setado por Blueprint (Get Skill Icon do Game Instance). |
| **Info_VBox** | **Vertical Box** | Agrupa nome, tipo e nível em coluna. Slot = Fill. |
| **Skill_Name_Text** | **Text Block** | Nome da skill. Fonte 16. |
| **Skill_Type_Text** | **Text Block** | "Ativa", "Passiva", etc. Fonte 12, cor cinza. |
| **Level_Required_Text** | **Text Block** | "Nível 10" ou "Requer Nível 10". Fonte 12. |
| **Rank_HBox** | **Horizontal Box** | 5× Image (Star_1 a Star_5), 16×16. Ativar N estrelas conforme CurrentRank. |
| **Status_Overlay** | **Overlay** (opcional) | Por cima da entrada: ícone de cadeado ou check. Visibility no Blueprint. |
| **Locked_Icon** | **Image** | Ícone de cadeado. Visible quando !bIsUnlocked. |
| **Learned_Check** | **Image** | Ícone de check. Visible quando bIsLearned. |

Entre Skill_Icon e Info_VBox use **Spacer** (Width 12); outro Spacer entre Info_VBox e Rank_HBox.

### Árvore resumida (tipos)

```
WBP_SkillBookEntry (UserWidget)
└── Horizontal Box (Entry_HBox)
    ├── Image (Skill_Icon) 64×64
    ├── Spacer (Width 12)
    ├── Vertical Box (Info_VBox)
    │   ├── Text Block (Skill_Name_Text)
    │   ├── Text Block (Skill_Type_Text)
    │   └── Text Block (Level_Required_Text)
    ├── Spacer (Width 12)
    ├── Horizontal Box (Rank_HBox) → Star_1 … Star_5 (Image)
    └── Overlay (Status_Overlay) [opcional]
        ├── Image (Locked_Icon)
        └── Image (Learned_Check)
```

### Variáveis (Blueprint)

```cpp
// Dados da skill
UPROPERTY(BlueprintReadWrite)
int32 SkillID;

UPROPERTY(BlueprintReadWrite)
FString SkillKey;

UPROPERTY(BlueprintReadWrite)
FString SkillName;

UPROPERTY(BlueprintReadWrite)
int32 RequiredLevel;

UPROPERTY(BlueprintReadWrite)
int32 CurrentRank;

UPROPERTY(BlueprintReadWrite)
int32 MaxRank;

// Status
UPROPERTY(BlueprintReadWrite)
bool bIsUnlocked;

UPROPERTY(BlueprintReadWrite)
bool bIsLearned;

UPROPERTY(BlueprintReadWrite)
bool bCanLearn;

UPROPERTY(BlueprintReadWrite)
bool bCanUpgrade;

// Visual
UPROPERTY(BlueprintReadWrite)
UTexture2D* SkillIcon;

UPROPERTY(BlueprintReadWrite)
FLinearColor ElementColor;
```

---

## Implementação em Blueprint

### 1. Event Graph - WBP_SkillBook

```
Event Construct
    ↓
[Get Game Instance] → Cast to UmbraGameInstance
    ↓
[Store Reference] → GameInstanceRef
    ↓
[Bind to OnSkillsLoaded Delegate]
    ↓
[Call LoadAvailableSkills on GameInstance]
```

### 2. Função: PopulateSkillList

```
PopulateSkillList (SkillsArray)
    ↓
[Clear Children] → Skills_ScrollBox
    ↓
[For Each Skill in SkillsArray]
    ↓
    [Create Widget] → WBP_SkillBookEntry
        ↓
    [Set Skill Data] → Entry Widget
        ↓
    [Bind OnClicked] → SelectSkill(SkillID)
        ↓
    [Add Child] → Skills_ScrollBox
```

### 3. Função: SelectSkill

```
SelectSkill (SkillID)
    ↓
[Find Skill in Array by ID]
    ↓
[Update Detail Panel]
    - Set Skill_Icon_Large
    - Set Skill_Name_Text
    - Set Skill_Type_Text
    - Set Skill_Description_Text
    - Set Cost/Cooldown/Range texts
    - Update RankStars
    ↓
[Update Action Button]
    - If not learned AND can learn → "Aprender (X pts)"
    - If learned AND can upgrade → "Melhorar (X pts)"
    - If learned AND max rank → "Rank Máximo"
    - If locked → "Requer Nível X"
```

### 4. Função: OnActionButtonClicked

```
OnActionButtonClicked
    ↓
[Branch: Is Skill Learned?]
    ↓
[No] → [Call LearnSkill on GameInstance]
        ↓
    [Wait for Response]
        ↓
    [If Success] → [Refresh Skill List]
                   [Show Success Message]
        ↓
    [If Fail] → [Show Error Message]
    
[Yes] → [Call UpgradeSkill on GameInstance]
        ↓
    [Same response handling]
```

---

## Integração com GameInstance

### Funções Disponíveis no GameInstance (já implementadas)

O GameInstance já possui toda a integração necessária para o sistema de skills:

```cpp
// ========== FUNÇÕES DISPONÍVEIS ==========

// Carregar skills disponíveis da classe
UFUNCTION(BlueprintCallable, Category = "Skills")
void LoadAvailableSkills();

// Carregar apenas skills aprendidas
UFUNCTION(BlueprintCallable, Category = "Skills")
void LoadPlayerSkills();

// Aprender skill
UFUNCTION(BlueprintCallable, Category = "Skills")
void LearnSkill(int32 SkillID);

// Melhorar skill
UFUNCTION(BlueprintCallable, Category = "Skills")
void UpgradeSkill(int32 SkillID);

// Verificar se aprendeu skill
UFUNCTION(BlueprintPure, Category = "Skills")
bool HasLearnedSkill(int32 SkillID) const;

// Obter dados de skill
UFUNCTION(BlueprintPure, Category = "Skills")
bool GetSkillDataByID(int32 SkillID, FUmbraSkillData& OutSkillData) const;

// Obter ícone de skill
UFUNCTION(BlueprintCallable, Category = "Skills")
UTexture2D* GetSkillIcon(const FString& SkillKey, bool bLarge = false) const;

// ========== DELEGATES DISPONÍVEIS ==========

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnAvailableSkillsLoaded OnAvailableSkillsLoaded;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnAvailableSkillsLoadFailed OnAvailableSkillsLoadFailed;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillLearned OnSkillLearned;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillLearnFailed OnSkillLearnFailed;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillUpgraded OnSkillUpgraded;

UPROPERTY(BlueprintAssignable, Category = "Events|Skills")
FOnSkillUpgradeFailed OnSkillUpgradeFailed;

// ========== DADOS ARMAZENADOS ==========

// Skills disponíveis para a classe
UPROPERTY(BlueprintReadOnly, Category = "Skills")
TArray<FUmbraSkillData> AvailableSkills;

// Skills aprendidas pelo jogador
UPROPERTY(BlueprintReadOnly, Category = "Skills")
TArray<FUmbraPlayerSkill> PlayerSkills;

// Pontos de skill
UPROPERTY(BlueprintReadOnly, Category = "Skills")
FUmbraSkillPoints SkillPoints;

// DataTable de ícones (configurar no Editor)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
UDataTable* SkillIconsDataTable;
```

### Uso no Blueprint - WBP_SkillBook

```
Event Construct
    ↓
[Get Game Instance] → Cast to UmbraGameInstance
    ↓
[Store Reference] → GameInstanceRef
    ↓
[Bind to OnAvailableSkillsLoaded]
    → Callback: PopulateSkillList
    ↓
[Bind to OnSkillLearned]
    → Callback: RefreshSkillList + ShowSuccessMessage
    ↓
[Bind to OnSkillUpgraded]
    → Callback: RefreshSkillList + ShowSuccessMessage
    ↓
[Call LoadAvailableSkills]
```

### Acessando Dados no Blueprint

**Importante:** **Available Skills** não é uma função — é uma **variável (propriedade)** do Umbra Game Instance. No Blueprint você usa o **getter** da variável.

- Obter o array de skills:
  1. **Get Game Instance** → **Cast to Umbra Game Instance**.
  2. Arraste o **pin de saída (azul)** do Cast para o canvas e solte.
  3. No menu, procure **"Available Skills"** ou **"AvailableSkills"**. O nó que aparece é o getter (ex.: **Get Available Skills**), que retorna **Array of Umbra Skill Data**.
  4. Se não achar: botão direito no canvas → **Search** → digite **Available** ou **Skills**; ou use o **GameInstanceRef** (variável do tipo Umbra Game Instance) e arraste dela para buscar **Available Skills**.

```
// Obter skills disponíveis (é variável, não função)
[Cast to Umbra Game Instance] → Get Available Skills
    → Returns: Array of FUmbraSkillData (Umbra Skill Data)

// Obter pontos de skill
[Get Game Instance] → Get Skill Points → Break Struct
    → TotalEarned
    → Spent
    → Available

// Verificar se skill foi aprendida
[Get Game Instance] → Has Learned Skill (SkillID)
    → Returns: bool

// Obter ícone da skill
[Get Game Instance] → Get Skill Icon (SkillKey, bLarge)
    → Returns: UTexture2D*
```

### Exemplo de PopulateSkillList (Blueprint)

```
PopulateSkillList
    ↓
[Get Game Instance] → Get AvailableSkills
    ↓
[For Each Skill in Array]
    ↓
    [Create Widget] → WBP_SkillBookEntry
        ↓
    [Call SetSkillData on Entry]
        - SkillID
        - SkillKey
        - SkillName
        - RequiredLevel
        - SkillType
        - Element
        - Description
        - IconPath (usar GetSkillIcon)
        ↓
    [Bind OnClicked] → SelectSkill(SkillID)
        ↓
    [Add Child] → Skills_ScrollBox
```

### Exemplo de LearnSkill (Blueprint)

```
OnActionButtonClicked
    ↓
[Get Selected SkillID]
    ↓
[Get Game Instance] → Has Learned Skill (SkillID)
    ↓
[Branch]
    |
    [No] → [Get Game Instance] → Learn Skill (SkillID)
    |
    [Yes] → [Get Game Instance] → Upgrade Skill (SkillID)
```

---

## Visual Design

### Cores por Tipo de Skill

| Tipo | Cor (Hex) | Uso |
|------|-----------|-----|
| ACTIVE | #4A90D9 | Borda/Destaque azul |
| PASSIVE | #8B8B8B | Borda/Destaque cinza |
| BUFF | #4CAF50 | Borda/Destaque verde |
| DEBUFF | #9C27B0 | Borda/Destaque roxo |
| ULTIMATE | #FFD700 | Borda/Destaque dourado |
| REACTION | #FF9800 | Borda/Destaque laranja |

### Cores por Elemento

| Elemento | Cor (Hex) |
|----------|-----------|
| PHYSICAL | #C0C0C0 |
| SHADOW | #4B0082 |
| FIRE | #FF4500 |
| HOLY | #FFD700 |
| POISON | #32CD32 |
| ICE | #00BFFF |
| LIGHTNING | #FFFF00 |
| ARCANE | #9400D3 |

### Estados Visuais

1. **Bloqueado** (nível insuficiente)
   - Ícone em grayscale
   - Overlay escuro 50%
   - Ícone de cadeado

2. **Disponível** (pode aprender)
   - Ícone colorido
   - Borda brilhante
   - Botão "Aprender" ativo

3. **Aprendido** (possui a skill)
   - Ícone colorido
   - Check verde no canto
   - Estrelas de rank visíveis

4. **Rank Máximo**
   - Todas as estrelas douradas
   - Efeito de brilho sutil

---

## DataTable de Ícones

### Criar DT_SkillIcons

1. Criar DataTable com struct `FSkillIconRow`:

```cpp
USTRUCT(BlueprintType)
struct FSkillIconRow : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SkillKey;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Icon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconLarge;
};
```

2. Popular com as 90 skills
3. Usar Soft References para carregar sob demanda

### Função: GetSkillIcon

```cpp
UTexture2D* GetSkillIcon(const FString& SkillKey, bool bLarge = false)
{
    // Lookup no DataTable
    FSkillIconRow* Row = DT_SkillIcons->FindRow<FSkillIconRow>(*SkillKey, TEXT(""));
    if (Row)
    {
        TSoftObjectPtr<UTexture2D>& IconRef = bLarge ? Row->IconLarge : Row->Icon;
        if (!IconRef.IsNull())
        {
            return IconRef.LoadSynchronous();
        }
    }
    // Retornar ícone padrão
    return DefaultSkillIcon;
}
```

---

## Tooltips

### WBP_SkillTooltip

```
WBP_SkillTooltip (UserWidget)
├── Border (background com sombra)
│   └── VBox
│       ├── Header_HBox
│       │   ├── Icon (32x32)
│       │   ├── Name_Text
│       │   └── Type_Badge
│       ├── Separator
│       ├── Stats_Grid
│       │   ├── "Custo:" / Cost_Value
│       │   ├── "Cooldown:" / CD_Value
│       │   ├── "Alcance:" / Range_Value
│       │   └── "Duração:" / Duration_Value
│       ├── Separator
│       ├── Description_RichText
│       ├── Separator
│       └── Requirement_Text (se bloqueado)
```

### Mostrar Tooltip

```
OnSkillEntryHovered
    ↓
[Create Widget] → WBP_SkillTooltip
    ↓
[Set Skill Data]
    ↓
[Add to Viewport]
    ↓
[Position near cursor]
```

---

## Checklist de Implementação

Para passos detalhados no Editor (tipos de painel, nomes exatos, Event Graph), use **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md**.

### Widgets
- [ ] WBP_SkillBook criado (Canvas Panel → Background_Image, Header_Panel Border, Content_Panel Horizontal Box)
- [ ] WBP_SkillBookEntry criado (Horizontal Box → Skill_Icon, Info_VBox, Rank_HBox, Status_Overlay)
- [ ] WBP_SkillTooltip criado (opcional)
- [ ] Layouts e nomes conforme procedimento
- [ ] Bindings de eventos (Construct, On Available Skills Loaded, On Clicked dos botões)

### Integração
- [ ] Funções no GameInstance
- [ ] Delegates configurados
- [ ] Requests HTTP funcionando
- [ ] Parsing de JSON

### Visual
- [ ] Ícones importados
- [ ] DT_SkillIcons populado
- [ ] Cores por tipo/elemento
- [ ] Estados visuais (locked/learned/max)

### Funcionalidades
- [ ] Listar skills
- [ ] Filtrar skills
- [ ] Selecionar skill
- [ ] Ver detalhes
- [ ] Aprender skill
- [ ] Melhorar skill
- [ ] Tooltip funcional
