# Guia: Sistema de Opções UE 5.6.1 (gráficos, áudio, resolução)

**Engine:** Unreal Engine **5.6.1**  
**Regra do projeto:** lógica em **C++**, Blueprint só para **layout**. Graph dos WBPs de opções/menu pode ficar **vazio**.

Guia nó a nó para criar `WBP_Options`, `WBP_GameMenu` e vincular login / seleção / ESC.

---

## 1. Pré-requisitos

| Passo | Ação |
|-------|------|
| Compilar C++ | Rebuild / Live Coding do `UmbraEternumUE` até aparecerem `UmbraOptionsWidget` e `UmbraGameMenuWidget` no picker de Parent Class |
| Pasta Content | Criar `/Game/Widgets/UI/Settings/` (botão direito no Content Browser → New Folder) |
| Persistência | Áudio/gameplay → SaveGame `UmbraClientSettings`; gráficos → `GameUserSettings.ini` |
| ESC | Já bindado em C++ no PlayerController (`EKeys::Escape`). Não precisa de Blueprint para abrir o menu in-game |

Verificação rápida: no Content Browser, botão direito → **User Interface → Widget Blueprint** → em Parent Class digite `UmbraOptions` — deve listar `UmbraOptionsWidget`.

---

## 2. Criar WBP_Options (passo a passo)

### 2.1 Criar o asset

1. Content Browser → abra `/Game/Widgets/UI/Settings/`.
2. Botão direito → **User Interface → Widget Blueprint**.
3. Nome: `WBP_Options` (exato).
4. Abra o asset.
5. Toolbar → **Class Settings** (ou Details com o root selecionado) → **Parent Class** → escolha `UmbraOptionsWidget`.
6. **Compile** e **Save**.

> Se a Parent Class não aparecer: o editor ainda não recarregou o módulo. Feche o WBP, compile o C++ de novo e reabra.

### 2.2 Como montar as 3 páginas no mesmo widget (WidgetSwitcher — recomendado)

Você **não** cria 3 Widget Blueprints. Tudo fica **dentro de um único** `WBP_Options`.

**Padrão do projeto** (igual ao leilão / `Switcher_Main`): um **Widget Switcher** com 3 filhos. O C++ chama `SetActiveWidgetIndex(0/1/2)` ao clicar nas abas.

```text
Mesmo painel
┌─────────────────────────────────────┐
│  [Tab Display] [Tab Audio] [Tab GP] │
├─────────────────────────────────────┤
│  Switcher_Pages                     │
│    ├─ [0] Page_Display   ← ativo    │
│    ├─ [1] Page_Audio                │
│    └─ [2] Page_Gameplay             │
├─────────────────────────────────────┤
│  [Aplicar] [Restaurar] [Fechar]     │
└─────────────────────────────────────┘
```

#### Passo a passo no Designer

1. Em `VB_Root`, depois de `HB_Tabs`, arraste **Widget Switcher** da Palette.
2. Details → **Name** = `Switcher_Pages`  
   (aceito também `Switcher_Main`, mesmo nome do Auction House.)
3. Com o Switcher selecionado, arraste **3× Size Box** (ou Border / Canvas) **para dentro** dele, **nessa ordem**:
   - índice **0** → Name `Page_Display`
   - índice **1** → Name `Page_Audio`
   - índice **2** → Name `Page_Gameplay`
4. A ordem na Hierarchy = índice. Arraste para reordenar se necessário (Display tem que ser o primeiro filho).
5. No Designer, Details do Switcher → **Active Widget Index** = `0` (só para editar a página Display com conforto).
6. Monte o conteúdo **dentro** de cada `Page_*` (seções 2.6–2.8).

#### O que o C++ faz (Graph vazio)

| Aba | Índice |
|-----|--------|
| `Tab_Display` | `0` |
| `Tab_Audio` | `1` |
| `Tab_Gameplay` | `2` |

#### Fallback Overlay (legado)

Se **não** houver `Switcher_Pages` / `Switcher_Main`, o C++ ainda usa Visible/Collapsed nos `Page_*` (Overlay). Prefira Switcher para ficar igual aos outros menus.

### 2.3 Hierarquia completa sugerida

No painel **Hierarchy** (à esquerda), parta do `CanvasPanel` raiz e monte:

```text
[CanvasPanel] Root
└── [Border] Border_Backdrop          (preto semi-transparente, ancora full screen)
    └── [Border] Border_Panel         (painel central ~900x650, ancorado no centro)
        └── [VerticalBox] VB_Root
            ├── [Text] Txt_Title      (texto "Opções" — só visual, sem bind C++)
            ├── [HorizontalBox] HB_Tabs
            │   ├── [Button] Tab_Display
            │   ├── [Button] Tab_Audio
            │   └── [Button] Tab_Gameplay
            ├── [WidgetSwitcher] Switcher_Pages   ★ padrão do projeto
            │   ├── [SizeBox] Page_Display        (índice 0)
            │   ├── [SizeBox] Page_Audio          (índice 1)
            │   └── [SizeBox] Page_Gameplay       (índice 2)
            └── [HorizontalBox] HB_Actions
                ├── [Button] BTN_Apply
                ├── [Button] BTN_ResetDefaults
                └── [Button] BTN_Close
```

**Dica:** selecione o Switcher → seta **Active Widget Index** no Details para 0, 1 ou 2 e edite só a página ativa no Designer.

### 2.4 Como renomear widgets (obrigatório)

1. Selecione o widget na Hierarchy.
2. No painel **Details** → topo → campo **Name** (não o texto do botão).
3. Digite o nome **exatamente** como na tabela (case-sensitive, com `_`).
4. Pressione Enter.

Exemplo errado: `TabDisplay`, `tab_display`, `BTN_Apply_1`.  
Exemplo certo: `Tab_Display`, `BTN_Apply`.

### 2.5 Texto visível nos botões (não é o Name)

1. Expanda o Button na Hierarchy → selecione o `Text` filho interno.
2. Details → **Text** → digite o rótulo (ex.: `Display`, `Áudio`, `Aplicar`).
3. Isso **não** altera o Name do Button usado pelo C++.

### 2.6 Montar Page_Display (gráficos)

Dentro de `Page_Display`, use um **Vertical Box** com linhas `HorizontalBox` (label + controle):

```text
Page_Display
└── VB_Display
    ├── HB_Res
    │   ├── Text "Resolução"
    │   └── [ComboBoxString] Combo_Resolution
    ├── HB_Window
    │   ├── Text "Modo de janela"
    │   └── [ComboBoxString] Combo_WindowMode
    ├── HB_VSync
    │   ├── Text "VSync"
    │   └── [CheckBox] Check_VSync
    ├── HB_FPS
    │   ├── Text "Limite de FPS"
    │   ├── [Slider] Slider_FrameRate
    │   └── [Text] Txt_FrameRate
    └── HB_Quality
        ├── Text "Qualidade geral"
        └── [ComboBoxString] Combo_OverallQuality
```

**Configurar `Slider_FrameRate` (Details):**

| Propriedade | Valor |
|-------------|-------|
| Value | `60` |
| Min Value | `0` |
| Max Value | `240` |
| Step Size | `1` |

(0 = ilimitado; o C++ mostra "Ilimitado" em `Txt_FrameRate`.)

**Combo boxes:** deixe Options vazio no Designer — o C++ preenche em `PopulateFromCurrentSettings`.

### 2.7 Montar Page_Audio

```text
Page_Audio
└── VB_Audio
    ├── HB_Master  → Text "Master"  + [Slider] Slider_Master  + [Text] Txt_Master
    ├── HB_Music   → Text "Música"  + [Slider] Slider_Music   + [Text] Txt_Music
    ├── HB_Sfx     → Text "SFX"     + [Slider] Slider_Sfx     + [Text] Txt_Sfx
    └── HB_Ui      → Text "UI"      + [Slider] Slider_Ui      + [Text] Txt_Ui
```

**Cada slider de volume (Details):**

| Propriedade | Valor |
|-------------|-------|
| Min Value | `0.0` |
| Max Value | `1.0` |
| Step Size | `0.01` |
| Value | `1.0` (ou `0.8` em Music) |

### 2.8 Montar Page_Gameplay

```text
Page_Gameplay
└── VB_Gameplay
    ├── HB_Sens → Text "Sensibilidade" + [Slider] Slider_MouseSens + [Text] Txt_MouseSens
    ├── HB_Inv  → Text "Inverter eixo Y" + [CheckBox] Check_InvertY
    ├── HB_Dmg  → Text "Números de dano" + [CheckBox] Check_DamageNumbers
    ├── HB_Name → Text "Nameplates" + [CheckBox] Check_Nameplates
    └── HB_Zoom → Text "Vel. zoom câmera" + [Slider] Slider_CameraZoom + [Text] Txt_CameraZoom
```

| Slider | Min | Max | Step | Value inicial |
|--------|-----|-----|------|---------------|
| `Slider_MouseSens` | `0.1` | `5.0` | `0.05` | `1.0` |
| `Slider_CameraZoom` | `10` | `150` | `1` | `50` |

### 2.9 Tabela completa de nomes (checklist)

Marque na Hierarchy se cada Name existe:

**Abas / páginas**

| Name | Tipo UMG |
|------|----------|
| `Tab_Display` | Button |
| `Tab_Audio` | Button |
| `Tab_Gameplay` | Button |
| `Switcher_Pages` | Widget Switcher (ou `Switcher_Main`) |
| `Page_Display` | filho índice 0 do Switcher |
| `Page_Audio` | filho índice 1 do Switcher |
| `Page_Gameplay` | filho índice 2 do Switcher |

**Display**

| Name | Tipo |
|------|------|
| `Combo_Resolution` | Combo Box (String) |
| `Combo_WindowMode` | Combo Box (String) |
| `Check_VSync` | Check Box |
| `Slider_FrameRate` | Slider |
| `Txt_FrameRate` | Text |
| `Combo_OverallQuality` | Combo Box (String) |

**Áudio**

| Name | Tipo |
|------|------|
| `Slider_Master` | Slider |
| `Txt_Master` | Text |
| `Slider_Music` | Slider |
| `Txt_Music` | Text |
| `Slider_Sfx` | Slider |
| `Txt_Sfx` | Text |
| `Slider_Ui` | Slider |
| `Txt_Ui` | Text |

**Gameplay**

| Name | Tipo |
|------|------|
| `Slider_MouseSens` | Slider |
| `Txt_MouseSens` | Text |
| `Check_InvertY` | Check Box |
| `Check_DamageNumbers` | Check Box |
| `Check_Nameplates` | Check Box |
| `Slider_CameraZoom` | Slider |
| `Txt_CameraZoom` | Text |

**Ações**

| Name | Tipo | Texto sugerido |
|------|------|----------------|
| `BTN_Apply` | Button | Aplicar |
| `BTN_ResetDefaults` | Button | Restaurar padrões |
| `BTN_Close` | Button | Fechar |

### 2.10 Event Graph do WBP_Options

1. Abra a aba **Graph**.
2. **Não** conecte OnClicked / OnValueChanged.
3. Deixe o Graph vazio (só Event Construct padrão, se existir, sem nós extras).
4. Compile e Save.

O C++ faz todos os binds em `NativeConstruct`.

### 2.11 Preview rápido no Designer

1. Toolbar → **Preview** (ou Play no Designer).
2. Clique nas abas: só a página correspondente deve aparecer (após Play in Editor com C++ carregado).
3. No Designer puro, a troca de aba só funciona em PIE / Standalone, porque o bind é em C++.

---

## 3. Criar WBP_GameMenu (passo a passo)

### 3.1 Criar o asset

1. Pasta `/Game/Widgets/UI/Settings/`.
2. **User Interface → Widget Blueprint** → nome `WBP_GameMenu`.
3. **Class Settings → Parent Class** = `UmbraGameMenuWidget`.
4. Compile e Save.

### 3.2 Hierarquia sugerida

```text
[CanvasPanel] Root
└── [Border] Border_Dim                 (full screen, preto ~50% alpha)
    └── [VerticalBox] VB_Menu           (ancorado no centro, largura ~320)
        ├── [Text] Txt_MenuTitle        ("Menu" — só visual)
        ├── [Button] BTN_Options
        ├── [Button] BTN_Resume
        ├── [Button] BTN_Logout
        └── [Button] BTN_QuitGame
```

### 3.3 Nomes e textos

| Name (Details) | Tipo | Texto do Label |
|----------------|------|----------------|
| `BTN_Options` | Button | Opções |
| `BTN_Resume` | Button | Continuar |
| `BTN_Logout` | Button | Sair para login |
| `BTN_QuitGame` | Button | Sair do jogo |

### 3.4 Layout tipográfico (sugestão)

Para cada botão:
1. Size → **Fill** no Vertical Box Slot, ou altura fixa ~48.
2. Padding entre botões: 8–12.
3. `Border_Dim` → Brush Color alpha ~0.5 para escurecer o mundo atrás.

### 3.5 Event Graph

Graph **vazio**. Clicks tratados no C++:
- Options → abre `WBP_Options` (Z 500)
- Resume → fecha menu + restaura input
- Logout → `Logout()` + `ReturnToLoginLevel()`
- Quit → `QuitGame`

Compile e Save.

---

## 4. Class Defaults (vínculos)

### 4.1 BP_UmbraGameInstance

1. Abra `/Game/Blueprints/BP_UmbraGameInstance` (ou o path do GI do projeto).
2. Class Defaults (toolbar).
3. Category **Settings | UI**:
   - **Options Widget Class** → `WBP_Options`
4. (Opcional) **Settings | Audio** — só se já criou Sound Mix/Classes (seção 8).
5. Compile e Save.

### 4.2 Blueprint do Player Controller

1. Abra o BP do Player Controller usado pelo pawn (ex.: o parent de `BP_ThirdPersonCharacter` / PC do Game Mode).
2. Class Defaults → **UI | Settings**:
   - **Game Menu Widget Class** → `WBP_GameMenu`
   - **Options Widget Class** → `WBP_Options` (fallback)
3. Compile e Save.

> Sem esses Class Defaults, o C++ ainda tenta paths de fallback (`/Game/Widgets/UI/Settings/WBP_*`). Prefira configurar explicitamente.

---

## 5. Input (opcional Enhanced Input)

ESC já abre/fecha o menu via `BindKey` no PlayerController.

Se quiser `IA_OpenGameMenu` também:

1. Content Browser → `/Game/Input/Actions/` → **Input → Input Action**.
2. Nome: `IA_OpenGameMenu`.
3. Details → **Value Type** = `Digital (bool)`.
4. Abra `IMC_Default` → **Mappings** → Add → Action = `IA_OpenGameMenu`, Key = **Escape**.
5. No Character Blueprint (Class Defaults) → **Input** → **Open Game Menu Action** = `IA_OpenGameMenu`.
6. Compile e Save.

---

## 6. Botão Options no login (`WBP_Login2`) — nó a nó

### 6.1 Designer

1. Abra `WBP_Login2`.
2. Adicione um **Button** (ex.: canto inferior ou ao lado de Login).
3. Name do Button: pode ser qualquer um no login (ex.: `BTN_Options`) — este Name **não** precisa bater com o C++ do Options; o Graph chama a função do GI.
4. Texto do label: `Opções`.

### 6.2 Event Graph (nó a nó)

1. Selecione `BTN_Options` → Details → Events → **+ On Clicked** (cria o nó no Graph).
2. Arraste do pino de execução:

| Ordem | Nó | Configuração |
|-------|-----|--------------|
| 1 | `Get Game Instance` | — |
| 2 | `Cast To UmbraGameInstance` (ou `BP_UmbraGameInstance`) | Object = saída do Get Game Instance |
| 3 | `Open Options From UI` | Target = Cast As…; **ZOrder** = `800` |

Esquema:

```text
BTN_Options.OnClicked
  → Get Game Instance
  → Cast To UmbraGameInstance
  → Open Options From UI (ZOrder = 800)
```

3. **Não** chame isso no mesmo fluxo de `LoginUser` / `OnLoginCompleted`.
4. Compile e Save.

---

## 7. Botão Options na seleção (`WBP_CharacterSelection`) — nó a nó

1. Abra `WBP_CharacterSelection`.
2. Adicione Button `BTN_Options` (texto "Opções").
3. Graph idêntico ao login:

```text
BTN_Options.OnClicked
  → Get Game Instance
  → Cast To UmbraGameInstance
  → Open Options From UI (ZOrder = 800)
```

4. Compile e Save.

---

## 8. Sound Classes (áudio por canal)

Sem Sound Classes, o **Master** ainda funciona via `SetTransientPrimaryVolume` (UE 5.6).

Para Music / SFX / UI separados:

### 8.1 Criar assets

1. Pasta `/Game/Audio/`.
2. Botão direito → **Sounds → Sound Class** → crie:
   - `SC_Master`
   - `SC_Music`
   - `SC_SFX`
   - `SC_UI`
3. Botão direito → **Sounds → Sound Mix** → `SM_ClientSettings`.

### 8.2 Atribuir no GI

Em `BP_UmbraGameInstance` → Settings | Audio:

| Campo | Asset |
|-------|-------|
| Client Settings Sound Mix | `SM_ClientSettings` |
| Master Sound Class | `SC_Master` |
| Music Sound Class | `SC_Music` |
| Sfx Sound Class | `SC_SFX` |
| Ui Sound Class | `SC_UI` |

### 8.3 Sons do jogo

Nos assets de som (música, hit, UI click), Details → **Sound Class** = a classe correspondente. Sem isso, só o Master afeta o volume global.

---

## 9. Checklist de teste (PIE)

- [ ] Hierarchy do `WBP_Options`: todos os Names da seção 2.8 existem (sem sufixo `_C` / `_1`)
- [ ] Hierarchy do `WBP_GameMenu`: 4 botões com Names corretos
- [ ] GI e PC com Class Defaults apontando para os WBPs
- [ ] Login → Options → Master volume → Apply → reiniciar PIE → volume persiste
- [ ] Resolução + Window Mode → Apply → janela muda; após restart mantém
- [ ] Overall Quality Low vs Epic → diferença visual
- [ ] In-game **ESC** abre `WBP_GameMenu`
- [ ] Options no menu → Apply → Close → movimento volta
- [ ] ESC com Options aberto fecha Options; segundo ESC fecha GameMenu
- [ ] Logout do GameMenu → `Lvl_TestAuth`
- [ ] Sensibilidade / Invert Y / zoom afetam Look e scroll

---

## 10. Troubleshooting

| Sintoma | Causa / correção |
|---------|------------------|
| Parent Class não lista UmbraOptionsWidget | Rebuild C++; reinicie o Editor se necessário |
| Abas não trocam página | Ordem dos filhos do Switcher errada (Display=0, Audio=1, Gameplay=2); Name `Switcher_Pages` / `Tab_*` errado |
| Apply não faz nada | Name `BTN_Apply` errado; confira Hierarchy |
| Combos vazios | Normal até PIE; preenchidos no `NativeConstruct` |
| ESC não abre menu | `GameMenuWidgetClass` vazio e WBP fora dos paths de fallback |
| Options não abre no login | GI sem `OptionsWidgetClass`; Cast falhou no Graph; ZOrder OK |
| Volume Music/SFX sem efeito | Sound Classes não atribuídas ou sons sem Sound Class |
| Resolução não aplica | Clique **Aplicar** (não basta mover o combo) |
| Movimento preso após menu | `IgnoreMoveInput` antigo; agora usamos `ResetIgnoreMoveInput` + `GameAndUI` na abertura. Resume deve voltar `GameOnly` com movimento liberado |
| Warning BindWidget | Em C++ usamos `BindWidgetOptional` — widget ausente = null, não crash; funcionalidade parcial |

Paths de fallback do C++ (se Class Defaults vazios):

- `/Game/Widgets/UI/Settings/WBP_Options.WBP_Options_C`
- `/Game/UI/Settings/WBP_Options.WBP_Options_C`
- `/Game/Widgets/WBP_Options.WBP_Options_C`
- (idem para `WBP_GameMenu`)

---

## 11. API C++ útil

```cpp
// GameInstance
LoadClientSettings();
ApplyClientSettings();
SaveClientSettings();
OpenOptionsFromUI(800);
ShouldShowDamageNumbers();
ShouldShowNameplates();
GetMouseSensitivitySetting();

// PlayerController
ToggleGameMenu();
OpenOptionsMenu();
CloseGameMenu();

// Character
ApplyClientGameplaySettings();
```

---

## Arquivos C++

| Arquivo | Papel |
|---------|-------|
| `Core/UmbraClientSettings.*` | SaveGame áudio/gameplay |
| `Core/UmbraGameInstanceClientSettings.cpp` | Load/Save/Apply + OpenOptionsFromUI |
| `UI/UmbraOptionsWidget.*` | UI de opções |
| `UI/UmbraGameMenuWidget.*` | Menu ESC |
| `UmbraEternumUEPlayerController.*` | Toggle ESC |
| `UmbraEternumUECharacter.*` | Sensibilidade / invert / zoom |

---

## Fora do v1

- Remap geral de teclas (WASD, K, J) — skillbar já tem `WBP_HotkeyCapture`
- Persistência server-side de preferências
- `SetGamePaused` (MMO: simulação da zone continua)
