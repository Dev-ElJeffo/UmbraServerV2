# 🎮 **GUIA COMPLETO: Seleção de Personagem em Level**

## 📋 **DECISÃO: Mesmo Nível ou Novo?**

### ✅ **RECOMENDAÇÃO: Criar um Novo Nível**

**Por quê?**
- **Separação de responsabilidades**: Criação e seleção são fluxos diferentes
- **Facilita manutenção**: Mudanças em um não afetam o outro
- **Melhor organização**: Cada level tem seu propósito claro
- **Reutilização**: Você pode usar o level de criação em outros contextos

**Níveis necessários:**
1. `Lvl_CharacterCreation` - Criação de personagem (já existe)
2. `Lvl_CharacterSelection` - Seleção de personagem (NOVO)
3. `Lvl_Tutorial` - Jogo principal (já existe)

---

## 🎯 **OBJETIVO**

Criar um level onde:
1. **Ao abrir** (após login): Mostra visualização 3D de **TODOS os personagens da conta** (similar ao `Lvl_CharacterCreation`)
2. **O widget `WBP_CharacterSelection` abre IMEDIATAMENTE** (mas VBox vazio)
3. **Cada personagem spawnado** tem um botão "SELECIONAR" (via Widget Component 3D)
4. **Ao clicar "SELECIONAR" em um personagem**:
   - A câmera se move para mostrar o personagem selecionado (similar ao creator)
   - Outros personagens são **despawnados**
   - O `WBP_CharacterItem` é criado e adicionado ao VBox do `WBP_CharacterSelection`
   - Exibe as informações específicas desse personagem no `WBP_CharacterItem`
5. **Botão "Play"** (no `WBP_CharacterItem`): Entra no jogo com o personagem selecionado (spawna no `Lvl_Tutorial`)
6. **Botão "Retornar"** (no `WBP_CharacterItem`): Remove o item do VBox e respawna todos os personagens

**Similar ao sistema de criação de personagem, mas para seleção!**

---

## 🔄 **FLUXO COMPLETO EXPLICADO**

### **Passo a Passo:**

```
1. Login → Abre Lvl_CharacterSelection automaticamente
   ↓
2. Level Blueprint:
   - Spawna BP_CharacterSelectionManager
   - Chama Initialize(PlayerController) no Manager
   - Cria WBP_CharacterSelection IMEDIATAMENTE
   - Adiciona WBP_CharacterSelection ao viewport
   ↓
3. Manager.Initialize:
   - Chama SpawnAllCharacterPreviews()
   ↓
4. Manager.SpawnAllCharacterPreviews:
   - Pega Current Players do GameInstance
   - Para cada personagem:
     * Spawna BP_CharacterPreview na posição calculada
     * Chama SetCharacterData no preview
     * Chama SetManagerReference no preview
     * Adiciona ao array CharacterPreviews
   ↓
5. Cada BP_CharacterPreview:
   - Tem um Widget Component (WBP_CharacterPreviewWidget)
   - O widget mostra o nome do personagem e botão "SELECIONAR"
   ↓
6. Usuário vê:
   - TODOS os personagens na tela (3D) com botões "SELECIONAR"
   - WBP_CharacterSelection aberto (mas VBox vazio, sem WBP_CharacterItem ainda)
   ↓
7. Usuário clica no botão "SELECIONAR" de um personagem
   ↓
8. Widget 3D chama Manager.SelectCharacterPreview(PlayerID)
   ↓
9. Manager.SelectCharacterPreview:
    - Encontra o preview pelo PlayerID
    - Despawne outros personagens
    - Move câmera para o selecionado
    - Chama SelectCharacter no GameInstance
    - Carrega informações completas
    - Chama AddCharacterItemToWidget(PlayerID)
    ↓
10. Manager.AddCharacterItemToWidget:
    - Cria WBP_CharacterItem
    - Chama SetCharacterData no WBP_CharacterItem
    - Chama SetSelectionContext no WBP_CharacterItem
    - Limpa VBox_CharacterList (remove itens anteriores)
    - Adiciona WBP_CharacterItem ao VBox do WBP_CharacterSelection
    ↓
11. WBP_CharacterItem aparece no VBox do WBP_CharacterSelection com:
    - Informações do personagem selecionado
    - Botão "Play"
    - Botão "Retornar"
    ↓
12. Usuário pode:
    - Clicar "Play" (no WBP_CharacterItem) → Abre Lvl_Tutorial e spawna o player
    - Clicar "Retornar" (no WBP_CharacterItem) → Remove WBP_CharacterItem e respawna todos os personagens
```

### **Pontos Importantes:**

- ✅ **Level Blueprint cria WBP_CharacterSelection IMEDIATAMENTE** ao abrir o level
- ✅ **Manager spawna TODOS os personagens** via `Get Current Players` do GameInstance
- ✅ **Cada preview tem Widget Component** com botão "SELECIONAR"
- ✅ **Ao clicar "SELECIONAR", WBP_CharacterItem é adicionado ao VBox** do WBP_CharacterSelection
- ✅ **Ao selecionar, outros personagens são despawnados** (não apenas escondidos)

---

## ⚠️ **IMPORTANTE: Modificar WBP_Login2 PRIMEIRO!**

**ANTES de começar a implementar o level, você DEVE modificar o `WBP_Login2` para abrir o level em vez de criar o widget diretamente!**

### **O QUE FAZER:**

1. **Abrir `WBP_Login2` → Event Graph**
2. **Localizar o Custom Event `OnCharacterListLoaded_Event`**
3. **REMOVER todo o código antigo:**
   - ❌ `Remove from Parent` Self
   - ❌ `Create Widget` WBP_CharacterSelection
   - ❌ `Add to Viewport`
4. **SUBSTITUIR por:**
   - ✅ `Open Level` → Level Name: `"Lvl_CharacterSelection"` → bAbsolute: `TRUE`

**Graph Visual:**
```
[OnCharacterListLoaded_Event]
    ↓
[Print String] "Lista carregada, abrindo level..."
    ↓
[Open Level]
    • Level Name: "Lvl_CharacterSelection"
    • bAbsolute: TRUE
```

**Se você não fizer isso, o sistema antigo continuará sendo usado e o level nunca será aberto!**

**Veja o guia completo em: `CORRECAO_LOGIN_ABRIR_LEVEL.md`**

---

## 📝 **FASE 1: Criar Level de Seleção**

### **1.1 - Criar Novo Level**

1. **File → New Level → Empty Level**
2. **Save As**: `Lvl_CharacterSelection`
3. Adicionar:
   - **Camera Actor** (nome: `CameraActor`) - Posição inicial da câmera
   - **Player Start** (posição: 0, 0, 0)
   - **Directional Light**
   - **Sky Sphere** (opcional)

### **1.2 - Criar Actor para Visualização 3D (Similar ao BP_Class_Placeholder)**

1. **Content Browser** → Botão direito → **Blueprint Class** → **Actor**
2. Nome: `BP_CharacterPreview`
3. Abrir o Blueprint

**Componentes:**
- **Scene Component** (nome: `Root` - raiz)
- **Skeletal Mesh Component** (nome: `CharacterMesh`)
- **Widget Component** (nome: `CharacterWidget`) - Widget com nome do personagem (opcional)

**Configuração:**
- `CharacterMesh`: 
  - Skeletal Mesh: (placeholder por enquanto, será atualizado baseado em ClassID/Hair/Head)
  - Position: (0, 0, 0)

**Variáveis:**
- `PlayerID` (int32) - ID do personagem
- `CharacterData` (UmbraPlayerData) - Dados do personagem
- `Manager` (BP_CharacterSelectionManager) - Referência ao manager

**Funções (criar depois):**
- `SetCharacterData(UmbraPlayerData)` - Define dados do personagem
- `UpdateCharacterMesh(int32 ClassID, int32 Hair, int32 Head)` - Atualiza mesh baseado em dados

**Salvar** o Blueprint.

### **1.3 - Criar Blueprint Manager (Similar ao BP_CharacterCreationManager)**

**IMPORTANTE: ANTES DE CRIAR O BLUEPRINT:**

1. **Compile o C++** primeiro (as classes `UmbraCharacterSelectionManager.h` e `.cpp` foram criadas)
2. **Feche o Unreal Editor** se estiver aberto
3. **Reabra o Unreal Editor** para que as classes C++ sejam reconhecidas

**AO CRIAR O BLUEPRINT:**

1. **Content Browser** → Botão direito → **Blueprint Class**
2. **Parent Class**: `UmbraCharacterSelectionManager` (NÃO Actor ou UmbraCharacterCreationManager!)
3. Nome: `BP_CharacterSelectionManager`
4. Abrir o Blueprint

**IMPORTANTE**: O Blueprint DEVE herdar de `UmbraCharacterSelectionManager` (classe C++) para ter acesso às funções `SelectCharacterAndMoveCamera` e `SetCharacterPreviewLocation`!

**Variáveis:**
- `CharacterPreviews` (Array of BP_CharacterPreview) - Array de todos os previews spawnados
- `SelectedPreview` (BP_CharacterPreview) - Preview atualmente selecionado

**Funções (criar depois):**
- `SpawnAllCharacterPreviews()` - Spawna previews de todos os personagens
- `SelectCharacterPreview(int32 PlayerID)` - Seleciona um preview e move câmera
- `DespawnOtherPreviews(int32 SelectedPlayerID)` - Despawne outros previews
- `GetPreviewByPlayerID(int32 PlayerID)` - Retorna preview pelo PlayerID

**Salvar** o Blueprint.

---

## 📝 **FASE 2: Criar Widget de Seleção (Similar ao WBP_CreateCharacter)**

### **2.1 - Criar Widget**

1. **Content Browser** → **User Interface → Widget Blueprint**
2. Nome: `WBP_CharacterSelection`
3. Abrir o Widget

### **2.2 - Layout (Designer)**

**IMPORTANTE**: Este widget abre IMEDIATAMENTE ao abrir o level! O VBox começa vazio e será populado quando o usuário selecionar um personagem.

```
Canvas Panel
├── Horizontal Box (preencher tela)
│   ├── Vertical Box (Lado Esquerdo - 30% da tela)
│   │   ├── Text Block: "SELECIONE SEU PERSONAGEM"
│   │   ├── Separator
│   │   ├── Scroll Box: "ScrollBox_CharacterList"
│   │   │   └── Vertical Box: "VBox_CharacterList"
│   │   │       └── [SERÁ POPULADO com botões "Select" para cada personagem no Event Construct]
│   │   ├── Separator
│   │   └── Vertical Box: "VBox_SelectedCharacter"
│   │       └── [SERÁ POPULADO com WBP_CharacterItem quando usuário clicar "Select" em um personagem]
│   │
│   ├── Vertical Box (Centro - 40% da tela)
│   │   └── [ESPAÇO PARA VISUALIZAÇÃO 3D - será renderizado pelo level]
│   │
│   └── Vertical Box (Lado Direito - 30% da tela)
│       └── [VAZIO inicialmente - pode adicionar instruções ou deixar vazio]
```

### **2.3 - Variáveis do Widget**

**No "My Blueprint" → Variables:**

```
- MyGameInstance (UmbraGameInstance) - Object Reference
- SelectionManager (BP_CharacterSelectionManager) - Object Reference
```

**Marcar todas como "Is Variable" = TRUE**

**No Designer, marcar como "Is Variable":**
- `ScrollBox_CharacterList` (Scroll Box)
- `VBox_CharacterList` (Vertical Box) - Lista de botões "Select" para cada personagem
- `VBox_SelectedCharacter` (Vertical Box) - Recebe WBP_CharacterItem quando um personagem é selecionado (fora do ScrollBox)

---

## 📝 **FASE 3: Atualizar WBP_CharacterItem**

### **3.1 - IMPORTANTE: WBP_CharacterItem é usado APENAS dentro do VBox**

**O `WBP_CharacterItem` só aparece quando o usuário clica "SELECIONAR" em um personagem no level. Ele deve ter apenas o botão "Play" que abre `Lvl_Tutorial`.**

### **3.2 - Modificar Botão "Play"**

**No `WBP_CharacterItem` → Event Graph:**

**SUBSTITUIR a lógica atual completamente:**

```
[BTN_Play OnClicked]
    ↓
[Get] MyGameInstance
    ↓
[Is Valid?]
    ├─→ [FALSE] → [Print String] "GameInstance None" → STOP
    └─→ [TRUE] →
        ↓
        [Break Struct] CharacterData
        ↓
        [SelectCharacter]  ← Garantir que está selecionado
        • PlayerID: (ID)
        ↓
        [Delay] 0.5s  ← Aguardar seleção completar
        ↓
        [Open Level]
        • Level Name: "Lvl_Tutorial"
        • bAbsolute: TRUE
        ↓
        [Set Show Mouse Cursor]
        • Target: (Get Player Controller)
        • Show Mouse Cursor: FALSE
```

**NOTA**: O botão "Select" está no `WBP_CharacterSelection` (ver Fase 2), não no `WBP_CharacterItem`!

---

## 📝 **FASE 4: Level Blueprint - Lvl_CharacterSelection**

### **4.1 - Abrir Level Blueprint**

1. Com `Lvl_CharacterSelection` aberto
2. **Blueprints → Open Level Blueprint**

### **4.2 - Event BeginPlay (CRIAR WIDGET E MANAGER)**

**IMPORTANTE**: O widget e o manager são criados IMEDIATAMENTE, mas o spawn dos personagens só acontece quando o evento `OnCharacterListLoaded` disparar (quando os dados chegarem do servidor).

```
[Event BeginPlay]
    ↓
[Delay] 0.2s
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Is Valid?]
    ├─→ [FALSE] → [Print String] "Erro: Player Controller None" → STOP
    └─→ [TRUE] →
        ↓
        [Find and Set Level Camera Actor]
        • Blend Time: 0.0
        ↓
        [Spawn Actor] BP_CharacterSelectionManager
        • Class: BP_CharacterSelectionManager
        • Location: (0, 0, 0)
        ↓
        [Is Valid?]
        ├─→ [FALSE] → [Print String] "Erro ao spawnar manager" → STOP
        └─→ [TRUE] →
            ↓
            [Create Widget] WBP_CharacterSelection
            • Owning Player: (Player Controller)
            ↓
            [Is Valid?]
            ├─→ [FALSE] → [Print String] "Erro ao criar widget" → STOP
            └─→ [TRUE] →
                ↓
                [Set SelectionManager] (no widget)
                • Manager: (Manager spawnado)
                ↓
                [Add to Viewport]
                ↓
                [Set Input Mode Game And UI]
                • Player Controller: (Player Controller)
                • Widget to Focus: (Widget criado)
                ↓
                [Set Show Mouse Cursor]
                • Target: (Player Controller)
                • Show Mouse Cursor: TRUE
                ↓
                [Get Game Instance]
                ↓
                [Cast to UmbraGameInstance]
                ↓
                [Is Valid?]
                ├─→ [FALSE] → [Print String] "GameInstance None" → STOP
                └─→ [TRUE] →
                    ↓
                    [Bind Event to OnCharacterListLoaded]
                    • Target: (UmbraGameInstance)
                    • Event: [Create Custom Event] "OnCharacterListLoaded_Event"
```

**COMO FAZER NO BLUEPRINT:**

1. **No Level Blueprint → Event Graph:**
   - Arraste do **exec pin** de **Event BeginPlay** → Adicione um **Delay** de `0.2` segundos
   - Conecte o **exec pin** de **Delay** → Digite "Get Player Controller" → Selecione **Get Player Controller** (Index: 0)

2. **Validar Player Controller:**
   - Arraste do **exec pin** de **Get Player Controller** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Player Controller** (saída de **Get Player Controller**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Get Player Controller** ao **exec pin** de **Branch**

3. **Se FALSE (Player Controller inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Erro: Player Controller None"`

4. **Se TRUE (Player Controller válido) - Configurar Câmera:**
   - Arraste do **True** (saída do Branch) → Digite "Find and Set Level Camera Actor" → Selecione **Find and Set Level Camera Actor**
   - Conecte o **Player Controller** ao **Target** de **Find and Set Level Camera Actor**
   - No campo **Blend Time** de **Find and Set Level Camera Actor**, digite: `0.0`
   - Conecte o **exec pin** de **True** ao **exec pin** de **Find and Set Level Camera Actor**

5. **Spawnar Manager:**
   - Arraste do **exec pin** de **Find and Set Level Camera Actor** → Digite "Spawn Actor" → Selecione **Spawn Actor from Class**
   - No campo **Class** de **Spawn Actor**, selecione **BP_CharacterSelectionManager**
   - No campo **Location** de **Spawn Actor**, digite: `(0, 0, 0)`
   - Conecte o **exec pin** de **Find and Set Level Camera Actor** ao **exec pin** de **Spawn Actor**

6. **Validar Manager:**
   - Arraste do **exec pin** de **Spawn Actor** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Return Value** (saída de **Spawn Actor**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Spawn Actor** ao **exec pin** de **Branch**

7. **Se FALSE (Manager inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Erro ao spawnar manager"`

8. **Se TRUE (Manager válido) - Criar Widget:**
   - Arraste do **True** (saída do Branch) → Digite "Create Widget" → Selecione **Create Widget**
   - No campo **Class** de **Create Widget**, selecione **WBP_CharacterSelection**
   - Conecte o **Player Controller** ao **Owning Player** de **Create Widget**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Create Widget**

9. **Validar Widget:**
   - Arraste do **exec pin** de **Create Widget** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Return Value** (saída de **Create Widget**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Create Widget** ao **exec pin** de **Branch**

10. **Se FALSE (Widget inválido):**
    - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
    - No campo **In String** de **Print String**, digite: `"Erro ao criar widget"`

11. **Se TRUE (Widget válido) - Configurar Widget:**
    - Arraste do **True** (saída do Branch) → Digite "Set SelectionManager" → Selecione **Set SelectionManager** (função do WBP_CharacterSelection)
    - Conecte o **Return Value** (de **Create Widget**) ao **Target** de **Set SelectionManager**
    - Conecte o **Return Value** (de **Spawn Actor** - o manager) ao **Manager** de **Set SelectionManager**
    - Conecte o **exec pin** de **True** ao **exec pin** de **Set SelectionManager**

12. **Adicionar Widget ao Viewport:**
    - Arraste do **exec pin** de **Set SelectionManager** → Digite "Add to Viewport" → Selecione **Add to Viewport**
    - Conecte o **Return Value** (de **Create Widget**) ao **Target** de **Add to Viewport**
    - Conecte o **exec pin** de **Set SelectionManager** ao **exec pin** de **Add to Viewport**

13. **Configurar Input Mode:**
    - Arraste do **exec pin** de **Add to Viewport** → Digite "Set Input Mode Game And UI" → Selecione **Set Input Mode Game And UI**
    - Conecte o **Player Controller** ao **Player Controller** de **Set Input Mode Game And UI**
    - Conecte o **Return Value** (de **Create Widget**) ao **Widget to Focus** de **Set Input Mode Game And UI**
    - Conecte o **exec pin** de **Add to Viewport** ao **exec pin** de **Set Input Mode Game And UI**

14. **Mostrar Mouse Cursor:**
    - Arraste do **exec pin** de **Set Input Mode Game And UI** → Digite "Set Show Mouse Cursor" → Selecione **Set Show Mouse Cursor**
    - Conecte o **Player Controller** ao **Target** de **Set Show Mouse Cursor**
    - No campo **Show Mouse Cursor** de **Set Show Mouse Cursor**, marque como **TRUE**
    - Conecte o **exec pin** de **Set Input Mode Game And UI** ao **exec pin** de **Set Show Mouse Cursor**

15. **Conectar ao Event OnCharacterListLoaded:**
    - Arraste do **exec pin** de **Set Show Mouse Cursor** → Digite "Get Game Instance" → Selecione **Get Game Instance**
    - Conecte o **exec pin** de **Set Show Mouse Cursor** ao **exec pin** de **Get Game Instance**
    - Arraste do **exec pin** de **Get Game Instance** → Digite "Cast to UmbraGameInstance" → Selecione **Cast to UmbraGameInstance**
    - Conecte o **Return Value** (de **Get Game Instance**) ao **Object** de **Cast to UmbraGameInstance**
    - Conecte o **exec pin** de **Get Game Instance** ao **exec pin** de **Cast to UmbraGameInstance**

16. **Validar GameInstance:**
    - Arraste do **exec pin** de **Cast to UmbraGameInstance** → Digite "Is Valid" → Selecione **Is Valid?**
    - Conecte o **As Umbra Game Instance** (saída do Cast) ao **Object** de **Is Valid?**
    - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
    - Conecte o **exec pin** de **Cast to UmbraGameInstance** ao **exec pin** de **Branch**

17. **Se FALSE (GameInstance inválido):**
    - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
    - No campo **In String** de **Print String**, digite: `"GameInstance None"`

18. **Se TRUE (GameInstance válido) - Verificar se dados já chegaram:**
    - Arraste do **True** (saída do Branch) → Digite "Get Current Players" → Selecione **Get Current Players** (função do UmbraGameInstance)
    - Conecte o **As Umbra Game Instance** (do Cast) ao **Target** de **Get Current Players**
    - Arraste do **exec pin** de **True** → Digite "Get Array Length" → Selecione **Get Array Length**
    - Conecte o **Current Players** (saída de **Get Current Players**) ao **Array** de **Get Array Length**
    - Arraste do **exec pin** de **True** → Digite "Branch" → Selecione **Branch**
    - Conecte o **Length** (saída de **Get Array Length**) ao **Condition** de **Branch** (mas precisamos verificar se > 0)
    - Arraste do **exec pin** de **True** → Digite "Greater" → Selecione **Greater (Integer)**
    - Conecte o **Length** (de **Get Array Length**) ao **A** de **Greater**
    - No campo **B** de **Greater**, digite: `0`
    - Conecte o **Greater** (saída booleana) ao **Condition** de **Branch**
    - Conecte o **exec pin** de **True** ao **exec pin** de **Branch**

19. **Se TRUE (Array Length > 0 - Dados já chegaram):**
    - Arraste do **True** (saída do Branch) → Digite "Print String" → Selecione **Print String**
    - No campo **In String** de **Print String**, digite: `"Dados já chegaram, inicializando imediatamente..."`
    - Conecte o **exec pin** de **True** ao **exec pin** de **Print String**
    - Arraste do **exec pin** de **Print String** → Digite "Get Manager" → Selecione **Get Manager**
    - Arraste do **exec pin** de **Print String** → Digite "Get Widget" → Selecione **Get Widget**
    - Arraste do **exec pin** de **Print String** → Digite "Get Player Controller" → Selecione **Get Player Controller** (Index: 0)
    - Arraste do **exec pin** de **Print String** → Digite "Is Valid" → Selecione **Is Valid?**
    - Conecte o **Manager** (de **Get Manager**) ao **Object** de **Is Valid?**
    - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
    - Conecte o **exec pin** de **Print String** ao **exec pin** de **Branch**
    - **Se TRUE (Manager válido):**
      - Arraste do **True** (saída do Branch) → Digite "Initialize" → Selecione **Initialize**
      - Conecte o **Manager** (de **Get Manager**) ao **Target** de **Initialize**
      - Conecte o **Player Controller** (de **Get Player Controller**) ao **Player Controller** de **Initialize**
      - Conecte o **Widget** (de **Get Widget**) ao **Widget** de **Initialize**
      - Conecte o **exec pin** de **True** ao **exec pin** de **Initialize**
      - Arraste do **exec pin** de **Initialize** → Digite "Populate Character Select Buttons" → Selecione **PopulateCharacterSelectButtons**
      - Conecte o **Widget** (de **Get Widget**) ao **Target** de **PopulateCharacterSelectButtons**
      - Conecte o **exec pin** de **Initialize** ao **exec pin** de **PopulateCharacterSelectButtons**

20. **Se FALSE (Array Length = 0 - Dados ainda não chegaram):**
    - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
    - No campo **In String** de **Print String**, digite: `"Aguardando dados chegarem..."`
    - Conecte o **exec pin** de **False** ao **exec pin** de **Print String**
    - Arraste do **exec pin** de **Print String** → Digite "Bind Event to OnCharacterListLoaded" → Selecione **Bind Event to OnCharacterListLoaded**
    - Conecte o **As Umbra Game Instance** (do Cast) ao **Target** de **Bind Event to OnCharacterListLoaded**
    - Clique com botão direito no **Event** de **Bind Event to OnCharacterListLoaded** → **"Create Custom Event"**
    - Nome do Custom Event: `OnCharacterListLoaded_Event`
    - Conecte o **exec pin** de **Print String** ao **exec pin** de **Bind Event to OnCharacterListLoaded**

### **4.3 - Custom Event: OnCharacterListLoaded_Event**

**Este evento será disparado quando os dados dos personagens chegarem do servidor.**

```
[OnCharacterListLoaded_Event] (Custom Event)
    ↓
[Print String] "DEBUG: Lista de personagens carregada"
    ↓
[Is Valid?] Manager (variável do Level Blueprint)
    ├─→ [FALSE] → [Print String] "Manager None" → STOP
    └─→ [TRUE] →
        ↓
        [Is Valid?] Widget (variável do Level Blueprint)
        ├─→ [FALSE] → [Print String] "Widget None" → STOP
        └─→ [TRUE] →
            ↓
            [Get Player Controller] (Index: 0)
            ↓
            [Is Valid?]
            ├─→ [FALSE] → [Print String] "Player Controller None" → STOP
            └─→ [TRUE] →
                ↓
                [Call Function: Initialize] (no Manager)
                • Player Controller: (Player Controller)
                • Widget: (Widget criado)
                • Preview Class: BP_CharacterPreview
                • Spacing: 300.0
                ↓
                [Call Function: PopulateCharacterSelectButtons] (no Widget)
```

**COMO FAZER NO BLUEPRINT:**

1. **Criar Variáveis no Level Blueprint:**
   - No painel **"My Blueprint"** → **"Variables"** → **"+"**
   - Criar variável `Manager` (tipo: **BP_CharacterSelectionManager**)
   - Criar variável `Widget` (tipo: **WBP_CharacterSelection**)
   - Marcar ambas como **"Is Variable" = TRUE**

2. **Salvar Referências no Event BeginPlay:**
   - Após **Spawn Actor** (manager), adicione **"Set Manager"** → Conecte o **Return Value** ao **Manager**
   - Após **Create Widget**, adicione **"Set Widget"** → Conecte o **Return Value** ao **Widget**

3. **No Custom Event OnCharacterListLoaded_Event:**
   - Arraste do **exec pin** do evento → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"DEBUG: Lista de personagens carregada"`
   - Conecte o **exec pin** do evento ao **exec pin** de **Print String**

4. **Validar Manager:**
   - Arraste do **exec pin** de **Print String** → Digite "Get Manager" → Selecione **Get Manager** (variável do Level Blueprint)
   - Arraste do **exec pin** de **Print String** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Manager** (de **Get Manager**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Print String** ao **exec pin** de **Branch**

5. **Se FALSE (Manager inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Manager None"`

6. **Se TRUE (Manager válido) - Validar Widget:**
   - Arraste do **True** (saída do Branch) → Digite "Get Widget" → Selecione **Get Widget** (variável do Level Blueprint)
   - Arraste do **exec pin** de **True** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Widget** (de **Get Widget**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Branch**

7. **Se FALSE (Widget inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Widget None"`

8. **Se TRUE (Widget válido) - Inicializar:**
   - Arraste do **True** (saída do Branch) → Digite "Get Player Controller" → Selecione **Get Player Controller** (Index: 0)
   - Arraste do **exec pin** de **True** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Player Controller** (de **Get Player Controller**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Branch**

9. **Se FALSE (Player Controller inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Player Controller None"`

10. **Se TRUE (Player Controller válido) - Chamar Initialize:**
    - Arraste do **True** (saída do Branch) → Digite "Initialize" → Selecione **Initialize** (função do Manager)
    - Conecte o **Manager** (de **Get Manager**) ao **Target** de **Initialize**
    - Conecte o **Player Controller** (de **Get Player Controller**) ao **Player Controller** de **Initialize**
    - Conecte o **Widget** (de **Get Widget**) ao **Widget** de **Initialize**
    - **IMPORTANTE**: No campo **Preview Class** de **Initialize**, selecione **BP_CharacterPreview**
    - No campo **Spacing** de **Initialize**, digite: `300.0`
    - Conecte o **exec pin** de **True** ao **exec pin** de **Initialize**

11. **Chamar PopulateCharacterSelectButtons:**
    - Arraste do **exec pin** de **Initialize** → Digite "Populate Character Select Buttons" → Selecione **PopulateCharacterSelectButtons** (função do Widget)
    - Conecte o **Widget** (de **Get Widget**) ao **Target** de **PopulateCharacterSelectButtons**
    - Conecte o **exec pin** de **Initialize** ao **exec pin** de **PopulateCharacterSelectButtons**

**RESULTADO:**
- O widget e o manager são criados IMEDIATAMENTE ao abrir o level
- **IMPORTANTE**: O código verifica se os dados já chegaram (Length > 0)
  - **Se SIM**: Inicializa imediatamente (dados já estão no GameInstance)
  - **Se NÃO**: Conecta ao evento e aguarda os dados chegarem
- O manager é inicializado e spawna todos os personagens
- O widget popula a lista de botões "Select"

**⚠️ PROBLEMA COMUM:**
Se o level abre **DEPOIS** que o evento `OnCharacterListLoaded` já foi disparado, o evento nunca vai disparar novamente. Por isso é necessário verificar se os dados já chegaram e inicializar imediatamente se necessário.

**Veja o guia completo em: `CORRECAO_LEVEL_DADOS_JA_CHEGARAM.md`**

---

## 📝 **FASE 5: Blueprint BP_CharacterPreview**

### **5.1 - Função: SetCharacterData**

**No `BP_CharacterPreview` → Functions → + New Function**

Nome: `SetCharacterData`

**Inputs:**
- `Character` (UmbraPlayerData)

**Graph:**
```
[SetCharacterData]
    ↓
[Set] CharacterData (Character)
    ↓
[Break Struct] CharacterData
    ↓
[Set] PlayerID (ID)
    ↓
[Call Function: UpdateCharacterMesh]
    • ClassID: (ClassID)
    • Hair: (Hair)
    • Head: (Head)
```

### **5.2 - Função: UpdateCharacterMesh**

**No `BP_CharacterPreview` → Functions → + New Function**

Nome: `UpdateCharacterMesh`

**Inputs:**
- `ClassID` (int32)
- `Hair` (int32)
- `Head` (int32)

**Graph:**
```
[UpdateCharacterMesh]
    ↓
[Get] CharacterMesh (Skeletal Mesh Component)
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Switch on Int] ClassID
        ├─→ Case 1: [Set Skeletal Mesh] (Mesh da classe 1)
        ├─→ Case 2: [Set Skeletal Mesh] (Mesh da classe 2)
        ├─→ Case 3: [Set Skeletal Mesh] (Mesh da classe 3)
        └─→ Default: [Set Skeletal Mesh] (Mesh padrão)
        ↓
        [TODO: Aplicar Hair e Head ao mesh]
        (Isso depende do seu sistema de meshes)
```

### **5.3 - Função: SetManagerReference**

**No `BP_CharacterPreview` → Functions → + New Function**

Nome: `SetManagerReference`

**Inputs:**
- `ManagerRef` (BP_CharacterSelectionManager)

**Graph:**
```
[SetManagerReference]
    ↓
[Set] Manager (ManagerRef)
```

### **5.4 - Event BeginPlay (OPCIONAL - A função C++ já faz isso!)**

**IMPORTANTE**: A função C++ `SpawnAllCharacterPreviews` já chama `SetupPreviewWidget` automaticamente! Você NÃO precisa fazer nada no `Event BeginPlay` do `BP_CharacterPreview`.

**Se quiser fazer manualmente no Blueprint (não recomendado, mas possível):**

**No `BP_CharacterPreview` → Event Graph → Event BeginPlay:**

```
[Event BeginPlay]
    ↓
[Get] CharacterWidget (Widget Component)
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Get User Widget Object]
        • Target: CharacterWidget
        ↓
        [Cast to WBP_CharacterPreviewWidget]
        ↓
        [Is Valid?]
        ├─→ [FALSE] → STOP
        └─→ [TRUE] →
            ↓
            [Call Function: SetPreviewActor]
            • Target: (Cast result)
            • Actor: Self (BP_CharacterPreview)
```

**MAS**: A função C++ `SetupPreviewWidget` já faz isso automaticamente quando você spawna o preview via `SpawnAllCharacterPreviews`!

**Nota**: Por enquanto, você pode usar um mesh placeholder. A aplicação de Hair/Head será implementada quando você tiver o sistema de meshes pronto.

### **5.6 - Widget Component: WBP_CharacterPreviewWidget**

**No `BP_CharacterPreview` → Components → Widget Component:**

1. **Widget Class**: Criar novo widget `WBP_CharacterPreviewWidget`
2. **Space**: World
3. **Draw Size**: (400, 200)
4. **Widget Location**: (0, 0, 200) - Acima do personagem

**No Widget `WBP_CharacterPreviewWidget`:**

**Layout:**
```
Canvas Panel
├── Vertical Box
│   ├── Text Block: "TXT_CharacterName" (Nome do personagem)
│   └── Button: "BTN_Select"
│       └── Text: "SELECIONAR"
```

**Variáveis:**
- `PreviewActor` (BP_CharacterPreview) - Object Reference

**Função: SetPreviewActor**
- **Input:** `Actor` (BP_CharacterPreview)
- **Graph:**
```
[SetPreviewActor]
    ↓
[Set] PreviewActor (Actor)
    ↓
[Is Valid?] PreviewActor
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Get CharacterData] (do PreviewActor)
        • Target: PreviewActor
        ↓
        [Break Struct] CharacterData
        • Struct: (CharacterData obtido do PreviewActor)
        ↓
        [Set Text] TXT_CharacterName
        • Text: (CharacterName - do Break Struct)
```

**COMO FAZER NO BLUEPRINT:**

1. **Criar a função:**
   - Abra `WBP_CharacterPreviewWidget`
   - Vá em **Functions** → **+ New Function**
   - Nome: `SetPreviewActor`
   - Clique em **Compile**

2. **Adicionar Input:**
   - Na aba **Details** da função, em **Inputs**, clique em **+**
     - Nome: `Actor`
     - Tipo: **BP_CharacterPreview** (Object Reference)
   - Clique em **Compile** novamente

3. **No Graph da função:**

   **PASSO 1: Set PreviewActor**
   - Arraste do **exec pin** da função → Digite "Set PreviewActor" → Selecione **Set PreviewActor**
   - Conecte o **exec pin** da função ao **exec pin** de **Set PreviewActor**
   - Conecte o **Actor** (input pin da função) ao **PreviewActor** (entrada de **Set PreviewActor**)

   **PASSO 2: Validar PreviewActor**
   - Arraste do **exec pin** de **Set PreviewActor** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **PreviewActor** (variável do widget) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Set PreviewActor** ao **exec pin** de **Branch**

   **PASSO 3: Se FALSE (PreviewActor inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Return Node" → Selecione **Return Node**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Return Node**

   **PASSO 4: Se TRUE (PreviewActor válido) - Obter CharacterData**
   - Arraste do **True** (saída do Branch) → Digite "Get CharacterData" → Selecione **Get CharacterData** (variável do BP_CharacterPreview)
   - Conecte o **PreviewActor** (variável do widget) ao **Target** de **Get CharacterData**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Get CharacterData**

   **PASSO 5: Break Struct do CharacterData**
   - Arraste do **exec pin** de **Get CharacterData** → Digite "Break Struct" → Selecione **Break Struct**
   - No campo **Struct** de **Break Struct**, selecione **UmbraPlayerData**
   - Conecte o **CharacterData** (saída de **Get CharacterData**) ao **Struct** (entrada do **Break Struct**)

   **PASSO 6: Set Text do Nome**
   - Arraste do **exec pin** de **Break Struct** → Digite "Set Text" → Selecione **Set Text** (do TXT_CharacterName)
   - Conecte o **CharacterName** (saída do **Break Struct**) ao **Text** de **Set Text**
   - Conecte o **exec pin** de **Break Struct** ao **exec pin** de **Set Text**

**GRAPH VISUAL:**
```
[SetPreviewActor]
    • Actor: (Input)
    ↓
[Set] PreviewActor (Actor)
    ↓
[Is Valid?] PreviewActor
    ├─→ [FALSE] → [Return Node]
    └─→ [TRUE] →
        ↓
        [Get CharacterData] (do PreviewActor)
        • Target: PreviewActor
        • Return Value: CharacterData
        ↓
        [Break Struct] CharacterData
        • Struct: (CharacterData)
        • CharacterName: (saída do Break)
        ↓
        [Set Text] TXT_CharacterName
        • Text: (CharacterName)
```

**NOTA IMPORTANTE**: 
- A função C++ `SetupPreviewWidget` chama `SetPreviewActor` automaticamente
- O widget precisa obter o `CharacterData` do `PreviewActor` após setar o `PreviewActor`
- Use `Get CharacterData` (variável do BP_CharacterPreview) para obter o CharacterData

**Event Construct:**
```
[Event Construct]
    ↓
[Set Text] TXT_CharacterName
    • Text: "Carregando..."
```

**IMPORTANTE**: No `BP_CharacterPreview`, no `Event BeginPlay`, você deve:
1. Obter o Widget Component
2. Obter o User Widget Object do componente
3. Cast para `WBP_CharacterPreviewWidget`
4. Chamar `SetPreviewActor` passando `Self` (o próprio BP_CharacterPreview)

**Botão BTN_Select:**
```
[BTN_Select OnClicked]
    ↓
[Is Valid?] PreviewActor
    ├─→ [FALSE] → [Print String] "PreviewActor None" → STOP
    └─→ [TRUE] →
        ↓
        [Get] Manager (do PreviewActor)
        ↓
        [Is Valid?]
        ├─→ [FALSE] → [Print String] "Manager None" → STOP
        └─→ [TRUE] →
            ↓
            [Get] PlayerID (do PreviewActor)
            ↓
            [Call Function: SelectCharacterPreview] (no Manager)
            • PlayerID: (PlayerID)
```

---

## 📝 **FASE 6: Blueprint BP_CharacterSelectionManager**

### **6.1 - Variáveis do Manager**

**IMPORTANTE**: ❌ **NÃO CRIE ESSAS VARIÁVEIS NO BLUEPRINT!**

Essas variáveis **JÁ EXISTEM** na classe C++ pai (`AUmbraCharacterSelectionManager`) como `UPROPERTY(BlueprintReadWrite)`:

```cpp
// Já existem em UmbraCharacterSelectionManager.h (linhas 184-194)
UPROPERTY(BlueprintReadWrite, Category = "Character Selection|References")
TArray<AActor*> CharacterPreviews;

UPROPERTY(BlueprintReadWrite, Category = "Character Selection|References")
AActor* SelectedPreview;

UPROPERTY(BlueprintReadWrite, Category = "Character Selection|References")
UUserWidget* SelectionWidget;

UPROPERTY(BlueprintReadWrite, Category = "Character Selection|References")
APlayerController* PlayerControllerRef;
```

**O QUE FAZER:**

1. **Se você criou essas variáveis no Blueprint por engano:**
   - **DELETE-AS IMEDIATAMENTE!**
   - Elas causam conflito porque já existem na classe C++ pai
   - O erro "O nome já está em uso" aparece porque a classe C++ já tem essas variáveis

2. **Como usar as variáveis:**
   - As variáveis já estão disponíveis automaticamente no Blueprint
   - Use `Get Character Previews`, `Set Character Previews`, etc. diretamente
   - Não precisa criar nada - apenas use os nós Get/Set

3. **Verificar se há variáveis conflitantes:**
   - No painel **"My Blueprint"** → **"Variáveis"**
   - Se você ver `Character Previews`, `SelectedPreview`, `SelectionWidget`, ou `PlayerControllerRef` listadas como variáveis do Blueprint:
     - **Selecione cada uma**
     - **Pressione DELETE** ou clique com botão direito → **Delete**
   - Essas variáveis devem aparecer apenas como propriedades herdadas da classe C++ (não como variáveis do Blueprint)

**RESUMO:**
- ✅ As variáveis já existem na classe C++ e estão disponíveis no Blueprint
- ❌ NÃO crie essas variáveis no Blueprint
- 🗑️ Se criou por engano, DELETE-AS
- ✅ Use `Get Character Previews`, `Set Character Previews`, etc. diretamente nos graphs

### **6.2 - Funções C++ já existentes (NÃO CRIAR NO BLUEPRINT!)**

**IMPORTANTE**: ❌ **NÃO CRIE ESSAS FUNÇÕES NO BLUEPRINT!**

As seguintes funções **JÁ EXISTEM** na classe C++ pai (`AUmbraCharacterSelectionManager`) como `UFUNCTION(BlueprintCallable)`:

```cpp
// Já existem em UmbraCharacterSelectionManager.h
UFUNCTION(BlueprintCallable, Category = "Character Selection|Initialization")
void Initialize(APlayerController* PC, UUserWidget* Widget);

UFUNCTION(BlueprintCallable, Category = "Character Selection|Spawn")
int32 SpawnAllCharacterPreviews(TSubclassOf<AActor> PreviewClass, float Spacing = 300.0f);

UFUNCTION(BlueprintCallable, Category = "Character Selection|Widget")
bool AddCharacterItemToWidget(int32 PlayerID, TSubclassOf<UUserWidget> CharacterItemWidgetClass);

UFUNCTION(BlueprintCallable, Category = "Character Selection|Selection")
void ResetSelection(TSubclassOf<AActor> PreviewClass, float Spacing = 300.0f);

UFUNCTION(BlueprintCallable, Category = "Character Selection|Selection")
AActor* GetPreviewByPlayerID(int32 PlayerID);
```

**O QUE FAZER:**

1. **Se você criou essas funções no Blueprint por engano:**
   - **DELETE-AS IMEDIATAMENTE!**
   - Elas causam conflito porque já existem na classe C++ pai
   - O erro "O nome já está em uso" aparece porque a classe C++ já tem essas funções

2. **Como usar as funções:**
   - As funções já estão disponíveis automaticamente no Blueprint
   - Use `Call Function: Initialize`, `Call Function: SpawnAllCharacterPreviews`, etc. diretamente
   - Não precisa criar nada - apenas chame as funções C++ nos graphs

3. **Verificar se há funções conflitantes:**
   - No painel **"My Blueprint"** → **"Functions"**
   - Se você ver `Initialize`, `SpawnAllCharacterPreviews`, `AddCharacterItemToWidget`, `ResetSelection`, ou `GetPreviewByPlayerID` listadas como funções do Blueprint:
     - **Selecione cada uma**
     - **Pressione DELETE** ou clique com botão direito → **Delete**
   - Essas funções devem aparecer apenas como funções herdadas da classe C++ (não como funções do Blueprint)

**RESUMO:**
- ✅ As funções já existem na classe C++ e estão disponíveis no Blueprint
- ❌ NÃO crie essas funções no Blueprint
- 🗑️ Se criou por engano, DELETE-AS
- ✅ Use `Call Function: Initialize`, `Call Function: SpawnAllCharacterPreviews`, etc. diretamente nos graphs

**EXEMPLO DE USO:**

No Level Blueprint ou em outro lugar, você pode chamar diretamente:

```
[Event BeginPlay]
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Create Widget] WBP_CharacterSelection
    ↓
[Get Actor of Class] BP_CharacterSelectionManager
    ↓
[Call Function: Initialize]
    • Target: (BP_CharacterSelectionManager encontrado)
    • PC: (Player Controller)
    • Widget: (WBP_CharacterSelection criado)
```

A função C++ `Initialize` já faz tudo automaticamente:
- Seta `PlayerControllerRef` e `SelectionWidget`
- Chama `SpawnAllCharacterPreviews` internamente

### **6.4 - Função: SelectCharacterPreview (SIMPLIFICADO - USA C++)**

**No `BP_CharacterSelectionManager` → Functions → + New Function**

Nome: `SelectCharacterPreview`

**Inputs:**
- `PlayerID` (int32)

**COMO FAZER NO BLUEPRINT (SIMPLES - IGUAL AO CHARACTER CREATION):**

1. **Criar a função:**
   - Abra `BP_CharacterSelectionManager`
   - Vá em **Functions** → **+ New Function**
   - Nome: `SelectCharacterPreview`
   - Clique em **Compile**

2. **Adicionar Input:**
   - Na aba **Details** da função, em **Inputs**, clique em **+**
     - Nome: `PlayerID`
     - Tipo: **Integer** (int32)
   - Clique em **Compile** novamente

3. **No Graph da função (SIMPLES - IGUAL AO CHARACTER CREATION):**

   **PASSO 1: Chamar Função C++ SelectCharacterAndMoveCamera**
   - Arraste do **exec pin** da função → Digite "Select Character And Move Camera" → Selecione **SelectCharacterAndMoveCamera**
   - Conecte o **exec pin** da função ao **exec pin** de **SelectCharacterAndMoveCamera**
   - Conecte o **PlayerID** (input pin da função) ao **PlayerID** de **SelectCharacterAndMoveCamera**
   - **A função C++ faz TUDO automaticamente:**
     - Encontra o preview pelo PlayerID
     - Despawne outros previews
     - Move a câmera
     - Seleciona o character no GameInstance
   
   **PASSO 2: Validar Sucesso**
   - Arraste do **exec pin** de **SelectCharacterAndMoveCamera** → Digite "Branch" → Selecione **Branch**
   - Conecte o **Return Value** (boolean) de **SelectCharacterAndMoveCamera** ao **Condition** de **Branch**

   **PASSO 3: Se TRUE (Sucesso) - Carregar Info e Adicionar Widget**
   - Arraste do **True** (saída do Branch) → Digite "Get Game Instance" → Selecione **Get Game Instance**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Get Game Instance**
   - Arraste do **exec pin** de **Get Game Instance** → Digite "Cast to UmbraGameInstance" → Selecione **Cast to UmbraGameInstance**
   - Conecte o **Return Value** (de **Get Game Instance**) ao **Object** de **Cast to UmbraGameInstance**
   - Conecte o **exec pin** de **Get Game Instance** ao **exec pin** de **Cast to UmbraGameInstance**
   - Arraste do **exec pin** de **Cast to UmbraGameInstance** → Digite "Load Character Info" → Selecione **LoadCharacterInfo**
   - Conecte o **As Umbra Game Instance** (do Cast) ao **Target** de **LoadCharacterInfo**
   - Conecte o **exec pin** de **Cast to UmbraGameInstance** ao **exec pin** de **LoadCharacterInfo**
   - Arraste do **exec pin** de **LoadCharacterInfo** → Digite "Delay" → Selecione **Delay**
   - No campo **Duration** de **Delay**, digite: `0.5`
   - Conecte o **exec pin** de **LoadCharacterInfo** ao **exec pin** de **Delay**
   - Arraste do **exec pin** de **Delay** → Digite "Add Character Item To Widget" → Selecione **AddCharacterItemToWidget**
   - Conecte o **PlayerID** (input pin da função) ao **PlayerID** de **AddCharacterItemToWidget**
   - Conecte o **exec pin** de **Delay** ao **exec pin** de **AddCharacterItemToWidget**

   **PASSO 4: Se FALSE (Falha)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Erro ao selecionar personagem"`

**RESULTADO FINAL:**
- A função C++ `SelectCharacterAndMoveCamera` faz TUDO automaticamente:
  - Encontra o preview pelo PlayerID
  - Despawne outros previews
  - Move a câmera
  - Seleciona o character no GameInstance
- A função Blueprint apenas chama a C++ e depois carrega as informações e adiciona o widget

**GRAPH VISUAL (SIMPLIFICADO):**
```
[SelectCharacterPreview]
    ↓
[Call Function: SelectCharacterAndMoveCamera] (C++ - FAZ TUDO)
    • Target: (Self - BP_CharacterSelectionManager)
    • PlayerID: (PlayerID)
    • Target Location: (0, 0, 0) ← Usa offsets
    • Target Rotation: (0, 0, 0) ← Mantém rotação atual
    • Duration: 1.0
    • OffsetX: 200.0
    • OffsetY: 0.0
    • OffsetZ: 100.0
    ↓
[Branch] (Return Value)
    ├─→ [FALSE] → [Print String] "Erro ao selecionar personagem"
    └─→ [TRUE] →
        ↓
        [Get Game Instance]
        ↓
        [Cast to UmbraGameInstance]
        ↓
        [LoadCharacterInfo]
        ↓
        [Delay] 0.5s
        ↓
        [Call Function: AddCharacterItemToWidget]
        • PlayerID: (PlayerID)
```

**NOTA IMPORTANTE**: 
- A função C++ `SelectCharacterAndMoveCamera` faz EXATAMENTE o mesmo que `SelectClassAndMoveCamera` do Character Creation, mas para personagens!
- Você pode usar `TargetLocation` e `TargetRotation` para posição absoluta, ou deixar (0,0,0) para usar offsets relativos ao preview

### **6.5 - Função: AddCharacterItemToWidget (JÁ EXISTE NO C++!)**

**IMPORTANTE**: ❌ **NÃO CRIE ESTA FUNÇÃO NO BLUEPRINT!**

Esta função **JÁ EXISTE** na classe C++ como `UFUNCTION(BlueprintCallable)`.

**Como usar:**
- Apenas chame `Call Function: AddCharacterItemToWidget` diretamente nos graphs
- A função C++ já faz tudo automaticamente

**EXEMPLO DE USO (no SelectCharacterPreview ou em outro lugar):**

```
[Call Function: AddCharacterItemToWidget]
    • Target: (Self - BP_CharacterSelectionManager)
    • PlayerID: (PlayerID)
    • CharacterItemWidgetClass: WBP_CharacterItem
    • Return Value: (true/false)
```

**NOTA**: A função C++ já faz tudo:
- Valida `SelectionWidget` e `SelectedPreview`
- Obtém `CharacterData` do preview
- Cria o widget `WBP_CharacterItem`
- Configura o widget com os dados
- Adiciona ao `VBox_SelectedCharacter`

**SE VOCÊ CRIOU ESTA FUNÇÃO NO BLUEPRINT POR ENGANO:**
- **DELETE-A IMEDIATAMENTE!**
- Ela causa conflito porque já existe na classe C++ pai

---

**SEÇÃO ANTIGA (PARA REFERÊNCIA - NÃO IMPLEMENTAR):**

~~**No Graph da função, adicione os nós:**~~

   **PASSO 1: Validar SelectionWidget**
   - Arraste do **exec pin** da função → Digite "Is Valid" → Selecione **Is Valid?**
   - Arraste do **Target** (self) → Digite "Get SelectionWidget" → Selecione **Get SelectionWidget**
   - Conecte o **SelectionWidget** (saída de **Get SelectionWidget**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** da função ao **exec pin** de **Branch**

   **PASSO 2: Se FALSE (Widget inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"SelectionWidget None"`

   **PASSO 3: Se TRUE (Widget válido) - Validar SelectedPreview**
   - Arraste do **True** (saída do Branch) → Digite "Is Valid" → Selecione **Is Valid?**
   - Arraste do **Target** (self) → Digite "Get SelectedPreview" → Selecione **Get SelectedPreview**
   - Conecte o **SelectedPreview** (saída de **Get SelectedPreview**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Branch**

   **PASSO 4: Se FALSE (Preview inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"SelectedPreview None"`

   **PASSO 5: Se TRUE (Preview válido) - Obter CharacterData**
   - Arraste do **True** (saída do Branch) → Digite "Get CharacterData" → Selecione **Get CharacterData** (variável do BP_CharacterPreview)
   - Conecte o **SelectedPreview** (de **Get SelectedPreview**) ao **Target** de **Get CharacterData**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Get CharacterData**

   **PASSO 6: Validar PlayerControllerRef**
   - Arraste do **exec pin** de **Get CharacterData** → Digite "Is Valid" → Selecione **Is Valid?**
   - Arraste do **Target** (self) → Digite "Get PlayerControllerRef" → Selecione **Get PlayerControllerRef**
   - Conecte o **PlayerControllerRef** (saída de **Get PlayerControllerRef**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Get CharacterData** ao **exec pin** de **Branch**

   **PASSO 7: Se FALSE (Player Controller inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Player Controller None"`

   **PASSO 8: Se TRUE (Player Controller válido) - Criar Widget**
   - Arraste do **True** (saída do Branch) → Digite "Create Widget" → Selecione **Create Widget**
   - No campo **Class** de **Create Widget**, selecione **WBP_CharacterItem**
   - Conecte o **PlayerControllerRef** (de **Get PlayerControllerRef**) ao **Owning Player** de **Create Widget**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Create Widget**

   **PASSO 9: Validar Widget Criado**
   - Arraste do **exec pin** de **Create Widget** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Return Value** (saída de **Create Widget**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Create Widget** ao **exec pin** de **Branch**

   **PASSO 10: Se FALSE (Widget inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Erro ao criar WBP_CharacterItem"`

   **PASSO 11: Se TRUE (Widget válido) - Set CharacterData**
   - Arraste do **True** (saída do Branch) → Digite "Set Character Data" → Selecione **SetCharacterData** (função do WBP_CharacterItem)
   - Conecte o **Return Value** (de **Create Widget**) ao **Target** de **SetCharacterData**
   - Conecte o **CharacterData** (de **Get CharacterData**) ao **Character** de **SetCharacterData**
   - Conecte o **exec pin** de **True** ao **exec pin** de **SetCharacterData**

   **PASSO 12: Set SelectionContext**
   - Arraste do **exec pin** de **SetCharacterData** → Digite "Set Selection Context" → Selecione **SetSelectionContext** (função do WBP_CharacterItem)
   - Conecte o **Return Value** (de **Create Widget**) ao **Target** de **SetSelectionContext**
   - Arraste do **Target** (self) → Digite "Self" → Selecione **Self**
   - Conecte o **Self** ao **Manager** de **SetSelectionContext**
   - Conecte o **exec pin** de **SetCharacterData** ao **exec pin** de **SetSelectionContext**

   **PASSO 13: Clear Children do VBox**
   - Arraste do **exec pin** de **SetSelectionContext** → Digite "Clear Children" → Selecione **Clear Children**
   - Arraste do **Target** (self) → Digite "Get SelectionWidget" → Selecione **Get SelectionWidget**
   - Conecte o **SelectionWidget** (de **Get SelectionWidget**) ao **Target** de **Clear Children**
   - No campo **Content** de **Clear Children**, você precisa acessar o **VBox_SelectedCharacter** do widget
   - Para isso: Arraste do **SelectionWidget** → Digite "Get VBox Selected Character" → Selecione **Get VBox_SelectedCharacter** (variável do WBP_CharacterSelection)
   - Conecte o **VBox_SelectedCharacter** ao **Content** de **Clear Children**
   - Conecte o **exec pin** de **SetSelectionContext** ao **exec pin** de **Clear Children**

   **PASSO 14: Add Child to Vertical Box**
   - Arraste do **exec pin** de **Clear Children** → Digite "Add Child to Vertical Box" → Selecione **Add Child to Vertical Box**
   - Conecte o **VBox_SelectedCharacter** (de **Get VBox_SelectedCharacter**) ao **Target** de **Add Child to Vertical Box**
   - Conecte o **Return Value** (de **Create Widget**) ao **Content** de **Add Child to Vertical Box**
   - Conecte o **exec pin** de **Clear Children** ao **exec pin** de **Add Child to Vertical Box**

**RESULTADO FINAL:**
- A função cria um `WBP_CharacterItem` com os dados do personagem selecionado
- Adiciona o widget ao `VBox_SelectedCharacter` do `WBP_CharacterSelection`
- O widget aparece na tela com as informações do personagem e botão "Play"

**GRAPH VISUAL:**
```
[AddCharacterItemToWidget]
    ↓
[Is Valid?] SelectionWidget
    ├─→ [FALSE] → [Print String] "SelectionWidget None" → STOP
    └─→ [TRUE] →
        ↓
        [Is Valid?] SelectedPreview
        ├─→ [FALSE] → [Print String] "SelectedPreview None" → STOP
        └─→ [TRUE] →
            ↓
            [Get] CharacterData (do SelectedPreview)
            ↓
            [Is Valid?] PlayerControllerRef
            ├─→ [FALSE] → [Print String] "Player Controller None" → STOP
            └─→ [TRUE] →
                ↓
                [Create Widget] WBP_CharacterItem
                • Owning Player: (PlayerControllerRef)
                ↓
                [Is Valid?]
                ├─→ [FALSE] → [Print String] "Erro ao criar WBP_CharacterItem" → STOP
                └─→ [TRUE] →
                    ↓
                    [Call Function: SetCharacterData] (no WBP_CharacterItem)
                    • Character: (CharacterData)
                    ↓
                    [Call Function: SetSelectionContext] (no WBP_CharacterItem)
                    • Manager: (Self)
                    ↓
                    [Clear Children] VBox_SelectedCharacter (no SelectionWidget)
                    ↓
                    [Add Child to Vertical Box]
                    • Target: VBox_SelectedCharacter (do SelectionWidget)
                    • Content: (WBP_CharacterItem criado)
```

### **6.6 - Função: GetPreviewByPlayerID (JÁ EXISTE NO C++!)**

**IMPORTANTE**: ❌ **NÃO CRIE ESTA FUNÇÃO NO BLUEPRINT!**

Esta função **JÁ EXISTE** na classe C++ como `UFUNCTION(BlueprintCallable)`.

**NOTA IMPORTANTE**: 
- Esta função é usada **INTERNAMENTE** pela função C++ `SelectCharacterAndMoveCamera`
- Você **NÃO PRECISA** chamá-la diretamente na maioria dos casos
- A função `SelectCharacterAndMoveCamera` já faz tudo automaticamente

**ONDE USAR (SE NECESSÁRIO):**

**CENÁRIO 1: No Level Blueprint (se precisar encontrar um preview específico)**

**No `Lvl_CharacterSelection` → Level Blueprint → Event Graph:**

```
[Event BeginPlay]
    ↓
[Get Actor of Class] BP_CharacterSelectionManager
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: GetPreviewByPlayerID]
        • Target: (BP_CharacterSelectionManager encontrado)
        • PlayerID: (algum PlayerID específico)
        • Return Value: (Preview encontrado ou None)
        ↓
        [Is Valid?] (Return Value)
        ├─→ [FALSE] → [Print String] "Preview não encontrado"
        └─→ [TRUE] → [Fazer algo com o Preview]
```

**CENÁRIO 2: No WBP_CharacterSelection (se precisar acessar um preview específico)**

**No `WBP_CharacterSelection` → Event Graph ou em alguma função:**

```
[Algum Event ou Função]
    ↓
[Get] SelectionManager (variável do widget)
    ↓
[Is Valid?]
    ├─→ [FALSE] → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: GetPreviewByPlayerID]
        • Target: (SelectionManager)
        • PlayerID: (PlayerID do personagem)
        • Return Value: (Preview encontrado ou None)
```

**MAS NA PRÁTICA:**
- Você **NÃO PRECISA** usar `GetPreviewByPlayerID` diretamente
- Use `SelectCharacterAndMoveCamera` que já faz tudo:
  - Encontra o preview
  - Despawne outros
  - Move a câmera
  - Seleciona o character

**SE VOCÊ CRIOU ESTA FUNÇÃO NO BLUEPRINT POR ENGANO:**
- **DELETE-A IMEDIATAMENTE!**
- Ela causa conflito porque já existe na classe C++ pai

---

**SEÇÃO ANTIGA (PARA REFERÊNCIA - NÃO IMPLEMENTAR):**

~~**No Graph da função, adicione os nós:**~~

   **PASSO 1: For Each Loop**
   - Arraste do **exec pin** da função → Digite "For Each Loop" → Selecione **For Each Loop**
   - Conecte o **exec pin** da função ao **exec pin** de **For Each Loop**
   - No campo **Array** de **For Each Loop**, selecione **CharacterPreviews** (sua variável de array)

   **PASSO 2: Dentro do Loop - Validar Preview**
   - Arraste do **Loop Body** (exec pin do **For Each Loop**) → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Array Element** (saída do **For Each Loop**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Loop Body** ao **exec pin** de **Branch**

   **PASSO 3: Se FALSE (Preview inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Continue Loop" → Selecione **Continue Loop**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Continue Loop**

   **PASSO 4: Se TRUE (Preview válido) - Obter PlayerID do Preview**
   - Arraste do **True** (saída do Branch) → Digite "Get PlayerID" → Selecione **Get PlayerID** (variável do BP_CharacterPreview)
   - Conecte o **Array Element** (do **For Each Loop**) ao **Target** de **Get PlayerID**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Get PlayerID**

   **PASSO 5: Comparar PlayerIDs**
   - Arraste do **exec pin** de **Get PlayerID** → Digite "Equal" → Selecione **Equal (Integer)**
   - Conecte o **PlayerID** (saída de **Get PlayerID**) ao **A** de **Equal**
   - Conecte o **PlayerID** (input pin da função) ao **B** de **Equal**
   - Arraste do **Equal** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Get PlayerID** ao **exec pin** de **Branch**

   **PASSO 6: Se FALSE (PlayerIDs diferentes)**
   - Arraste do **False** (saída do Branch) → Digite "Continue Loop" → Selecione **Continue Loop**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Continue Loop**

   **PASSO 7: Se TRUE (PlayerIDs iguais) - Retornar Preview**
   - Arraste do **True** (saída do Branch) → Digite "Set Preview" → Selecione **Set Preview** (output pin da função)
   - Conecte o **Array Element** (do **For Each Loop**) ao **Preview** (entrada de **Set Preview**)
   - Arraste do **exec pin** de **True** → Digite "Break Loop" → Selecione **Break Loop**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Break Loop**

   **PASSO 8: Conectar ao Completed do Loop**
   - Arraste do **Completed** (exec pin de saída do **For Each Loop**) → Conecte ao **Return Node** (exec pin de saída da função)

**RESULTADO FINAL:**
- A função percorre o array `CharacterPreviews`
- Compara o `PlayerID` de cada preview com o `PlayerID` procurado
- Retorna o preview que corresponde ao `PlayerID`, ou `None` se não encontrar

**GRAPH VISUAL:**
```
[GetPreviewByPlayerID]
    ↓
[For Each Loop] CharacterPreviews
    • Array: CharacterPreviews
    • Loop Body: (Array Element = Preview)
        ↓
        [Is Valid?] Preview
        ├─→ [FALSE] → [Continue Loop]
        └─→ [TRUE] →
            ↓
            [Get] PlayerID (do Preview)
            ↓
            [Equal] (PlayerID do Preview == PlayerID procurado)
            ├─→ [FALSE] → [Continue Loop]
            └─→ [TRUE] →
                ↓
                [Set] Preview (return value)
                ↓
                [Break Loop]
    ↓
    RETURN Preview
```

### **6.7 - Função: DespawnOtherPreviews**

**No `BP_CharacterSelectionManager` → Functions → + New Function**

Nome: `DespawnOtherPreviews`

**COMO FAZER NO BLUEPRINT:**

1. **Criar a função:**
   - Abra `BP_CharacterSelectionManager`
   - Vá em **Functions** → **+ New Function**
   - Nome: `DespawnOtherPreviews`
   - Clique em **Compile**

2. **Adicionar Input:**
   - Na aba **Details** da função, em **Inputs**, clique em **+**
     - Nome: `SelectedPlayerID`
     - Tipo: **Integer** (int32)
   - Clique em **Compile** novamente

3. **No Graph da função, adicione os nós:**

   **PASSO 1: For Each Loop**
   - Arraste do **exec pin** da função → Digite "For Each Loop" → Selecione **For Each Loop**
   - Conecte o **exec pin** da função ao **exec pin** de **For Each Loop**
   - No campo **Array** de **For Each Loop**, selecione **CharacterPreviews**

   **PASSO 2: Dentro do Loop - Validar Preview**
   - Arraste do **Loop Body** (exec pin do **For Each Loop**) → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **Array Element** (saída do **For Each Loop**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Loop Body** ao **exec pin** de **Branch**

   **PASSO 3: Se FALSE (Preview inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Continue Loop" → Selecione **Continue Loop**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Continue Loop**

   **PASSO 4: Se TRUE (Preview válido) - CAST para BP_CharacterPreview**
   - Arraste do **True** (saída do Branch) → Digite "Cast to BP Character Preview" → Selecione **Cast to BP_CharacterPreview**
   - Conecte o **Array Element** (do **For Each Loop**) ao **Object** de **Cast to BP_CharacterPreview**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Cast to BP_CharacterPreview**

   **PASSO 5: Validar Cast (Is Valid?)**
   - Arraste do **exec pin** de **Cast to BP_CharacterPreview** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **As BP Character Preview** (saída do Cast) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Cast to BP_CharacterPreview** ao **exec pin** de **Branch**

   **PASSO 6: Se FALSE (Cast falhou)**
   - Arraste do **False** (saída do Branch) → Digite "Continue Loop" → Selecione **Continue Loop**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Continue Loop**

   **PASSO 7: Se TRUE (Cast bem-sucedido) - Obter PlayerID**
   - Arraste do **True** (saída do Branch) → Digite "Get PlayerID" → Selecione **Get PlayerID** (variável do BP_CharacterPreview)
   - Conecte o **As BP Character Preview** (do Cast) ao **Target** de **Get PlayerID**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Get PlayerID**

   **PASSO 8: Comparar PlayerIDs (Not Equal)**
   - Arraste do **exec pin** de **Get PlayerID** → Digite "Not Equal" → Selecione **Not Equal (Integer)**
   - Conecte o **PlayerID** (saída de **Get PlayerID**) ao **A** de **Not Equal**
   - Conecte o **SelectedPlayerID** (input pin da função) ao **B** de **Not Equal**
   - Arraste do **Not Equal** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Get PlayerID** ao **exec pin** de **Branch**

   **PASSO 9: Se FALSE (PlayerIDs iguais - não despawne)**
   - Arraste do **False** (saída do Branch) → Digite "Continue Loop" → Selecione **Continue Loop**
   - Conecte o **exec pin** de **False** ao **exec pin** de **Continue Loop**

   **PASSO 10: Se TRUE (PlayerIDs diferentes - despawne)**
   - Arraste do **True** (saída do Branch) → Digite "Destroy Actor" → Selecione **Destroy Actor**
   - Conecte o **As BP Character Preview** (do Cast) ao **Target** de **Destroy Actor** (ou use o Array Element se preferir)
   - Conecte o **exec pin** de **True** ao **exec pin** de **Destroy Actor**

   **PASSO 11: Remover do Array**
   - Arraste do **exec pin** de **Destroy Actor** → Digite "Remove from Array" → Selecione **Remove from Array**
   - No campo **Array** de **Remove from Array**, selecione **CharacterPreviews**
   - Conecte o **Array Element** (do **For Each Loop**) ao **Item to Remove** de **Remove from Array**
   - Conecte o **exec pin** de **Destroy Actor** ao **exec pin** de **Remove from Array**

   **PASSO 12: Conectar ao Completed do Loop**
   - Arraste do **exec pin** de **Remove from Array** → Conecte ao **Completed** (exec pin de saída do **For Each Loop**)

**RESULTADO FINAL:**
- A função percorre o array `CharacterPreviews`
- Destrói todos os previews que NÃO correspondem ao `SelectedPlayerID`
- Remove os previews destruídos do array
- Mantém apenas o preview selecionado

**GRAPH VISUAL:**
```
[DespawnOtherPreviews]
    ↓
[For Each Loop] CharacterPreviews
    • Array: CharacterPreviews
    • Loop Body: (Array Element = Actor)
        ↓
        [Is Valid?] Actor
        ├─→ [FALSE] → [Continue Loop]
        └─→ [TRUE] →
            ↓
            [Cast to BP_CharacterPreview]
            • Object: (Array Element)
            ↓
            [Is Valid?] (As BP Character Preview)
            ├─→ [FALSE] → [Continue Loop]
            └─→ [TRUE] →
                ↓
                [Get] PlayerID (do BP_CharacterPreview)
                • Target: (As BP Character Preview)
                ↓
                [Not Equal] (PlayerID != SelectedPlayerID)
                ├─→ [FALSE] → [Continue Loop]  ← Não despawne o selecionado
                └─→ [TRUE] →
                    ↓
                    [Destroy Actor] Preview
                    • Target: (As BP Character Preview)
                    ↓
                    [Remove from Array] CharacterPreviews
                    • Array: CharacterPreviews
                    • Item to Remove: (Array Element)
```

### **6.8 - Função: ResetSelection (JÁ EXISTE NO C++!)**

**IMPORTANTE**: ❌ **NÃO CRIE ESTA FUNÇÃO NO BLUEPRINT!**

Esta função **JÁ EXISTE** na classe C++ como `UFUNCTION(BlueprintCallable)`.

**Assinatura da função C++:**
```cpp
UFUNCTION(BlueprintCallable, Category = "Character Selection|Selection")
void ResetSelection(TSubclassOf<AActor> PreviewClass, float Spacing = 300.0f);
```

**O que a função faz:**
- Limpa o `VBox_SelectedCharacter` do widget
- Reseta a variável `SelectedPreview` para `None`
- Respawna todos os personagens na tela
- Volta ao estado inicial de visualização

**ONDE USAR:**

### **LOCAL 1: No WBP_CharacterItem → BTN_Return (PRINCIPAL)**

**No `WBP_CharacterItem` → Event Graph → BTN_Return → OnClicked:**

```
[BTN_Return OnClicked]
    ↓
[Is Valid?] SelectionManager
    ├─→ [FALSE] → [Print String] "Manager None" → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: ResetSelection] (no Manager)
        • Target: (SelectionManager)
        • Preview Class: BP_CharacterPreview
        • Spacing: 300.0
        ↓
        [Remove from Parent] (Self)  ← Remove este WBP_CharacterItem do VBox
```

**COMO FAZER NO BLUEPRINT:**

1. **No `WBP_CharacterItem` → Event Graph:**
   - Localize o botão **BTN_Return** (ou crie um se não existir)
   - Clique com botão direito no botão → **"On Clicked"**

2. **No Graph do OnClicked:**
   - Arraste do **exec pin** do evento → Digite "Is Valid" → Selecione **Is Valid?**
   - Arraste do **Target** (self) → Digite "Get SelectionManager" → Selecione **Get SelectionManager**
   - Conecte o **SelectionManager** (saída de **Get SelectionManager**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** do evento ao **exec pin** de **Branch**

3. **Se FALSE (Manager inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Manager None"`

4. **Se TRUE (Manager válido):**
   - Arraste do **True** (saída do Branch) → Digite "Reset Selection" → Selecione **ResetSelection**
   - Conecte o **SelectionManager** (de **Get SelectionManager**) ao **Target** de **ResetSelection**
   - No campo **Preview Class** de **ResetSelection**, selecione **BP_CharacterPreview**
   - No campo **Spacing** de **ResetSelection**, digite: `300.0`
   - Conecte o **exec pin** de **True** ao **exec pin** de **ResetSelection**

5. **Remover Widget do VBox:**
   - Arraste do **exec pin** de **ResetSelection** → Digite "Remove from Parent" → Selecione **Remove from Parent**
   - Conecte o **exec pin** de **ResetSelection** ao **exec pin** de **Remove from Parent**

**RESULTADO:**
- Quando o usuário clica "Return" no `WBP_CharacterItem`:
  - A função C++ `ResetSelection` é chamada
  - Limpa o `VBox_SelectedCharacter`
  - Respawna todos os personagens
  - O `WBP_CharacterItem` é removido do VBox
  - Volta ao estado inicial (todos os personagens visíveis)

**SE VOCÊ CRIOU ESTA FUNÇÃO NO BLUEPRINT POR ENGANO:**
- **DELETE-A IMEDIATAMENTE!**
- Ela causa conflito porque já existe na classe C++ pai

---

## 📍 **ONDE USAR ResetSelection - RESUMO COMPLETO**

### **LOCAL PRINCIPAL: WBP_CharacterItem → BTN_Return**

**Graph:** `WBP_CharacterItem` → **Event Graph** → **BTN_Return** → **OnClicked**

**Quando usar:**
- Quando o usuário clica no botão "Return" ou "Voltar" no `WBP_CharacterItem`
- Para voltar à visualização inicial (todos os personagens visíveis)

**Como chamar:**
```
[BTN_Return OnClicked]
    ↓
[Call Function: ResetSelection]
    • Target: (SelectionManager)
    • Preview Class: BP_CharacterPreview
    • Spacing: 300.0
```

### **OUTROS LOCAIS OPCIONAIS (se necessário):**

**1. No Level Blueprint (se quiser resetar ao abrir o level novamente):**
- Graph: `Lvl_CharacterSelection` → **Level Blueprint** → **Event BeginPlay**
- Quando: Ao abrir o level, garantir que está no estado inicial

**2. No WBP_CharacterSelection (se quiser um botão "Voltar" no widget principal):**
- Graph: `WBP_CharacterSelection` → **Event Graph** → **BTN_Back** → **OnClicked**
- Quando: Se você adicionar um botão "Voltar" no widget principal

**MAS:** O uso principal é no **BTN_Return do WBP_CharacterItem**!

---

## 📝 **FASE 6.5: Criar Widget WBP_CharacterSelectButton (Widget Simples para Botões)**

### **6.5.1 - Criar Widget**

1. **Content Browser** → **User Interface → Widget Blueprint**
2. Nome: `WBP_CharacterSelectButton`
3. Abrir o Widget

### **6.5.2 - Layout (Designer)**

```
Canvas Panel
└── Button: "BTN_Select"
    └── Text: "SELECIONAR"
```

### **6.5.3 - Variáveis**

```
- CharacterData (UmbraPlayerData) - Struct
- SelectionWidget (WBP_CharacterSelection) - Object Reference
```

**Marcar todas como "Is Variable" = TRUE**

### **6.5.4 - Funções**

**Função: SetCharacterData**
- Input: `Character` (UmbraPlayerData)
- Graph:
```
[SetCharacterData]
    ↓
[Set] CharacterData (Character)
    ↓
[Break Struct] CharacterData
    ↓
[Set Text] BTN_Select
    • Text: (CharacterName + " - SELECIONAR")
```

**Função: SetSelectionWidget**
- Input: `Widget` (WBP_CharacterSelection)
- Graph:
```
[SetSelectionWidget]
    ↓
[Set] SelectionWidget (Widget)
```

### **6.5.5 - Botão BTN_Select**

```
[BTN_Select OnClicked]
    ↓
[Is Valid?] SelectionWidget
    ├─→ [FALSE] → [Print String] "SelectionWidget None" → STOP
    └─→ [TRUE] →
        ↓
        [Break Struct] CharacterData
        ↓
        [Call Function: OnSelectCharacterClicked] (no SelectionWidget)
        • PlayerID: (ID)
```

---

## 📝 **FASE 7: Widget WBP_CharacterSelection**

### **7.1 - Event Construct**

**IMPORTANTE**: ❌ **NÃO CHAME `PopulateCharacterSelectButtons` NO EVENT CONSTRUCT!**

O `PopulateCharacterSelectButtons` deve ser chamado **APENAS** quando o evento `OnCharacterListLoaded` disparar (no Level Blueprint), **DEPOIS** que os dados chegarem do servidor.

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Is Valid?]
    ├─→ [FALSE] → [Print String] "GameInstance None" → STOP
    └─→ [TRUE] →
        ↓
        [Set] MyGameInstance
        ↓
        [Clear Children] VBox_SelectedCharacter
        ↓
        [Clear Children] VBox_CharacterList
```

**COMO FAZER NO BLUEPRINT:**

1. **No `WBP_CharacterSelection` → Event Graph:**
   - Localize o **Event Construct**
   - Arraste do **exec pin** do evento → Digite "Get Game Instance" → Selecione **Get Game Instance**

2. **Cast para UmbraGameInstance:**
   - Arraste do **exec pin** de **Get Game Instance** → Digite "Cast to UmbraGameInstance" → Selecione **Cast to UmbraGameInstance**
   - Conecte o **Return Value** (de **Get Game Instance**) ao **Object** de **Cast to UmbraGameInstance**
   - Conecte o **exec pin** de **Get Game Instance** ao **exec pin** de **Cast to UmbraGameInstance**

3. **Validar GameInstance:**
   - Arraste do **exec pin** de **Cast to UmbraGameInstance** → Digite "Is Valid" → Selecione **Is Valid?**
   - Conecte o **As Umbra Game Instance** (saída do Cast) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** de **Cast to UmbraGameInstance** ao **exec pin** de **Branch**

4. **Se FALSE (GameInstance inválido):**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"GameInstance None"`

5. **Se TRUE (GameInstance válido) - Configurar:**
   - Arraste do **True** (saída do Branch) → Digite "Set MyGameInstance" → Selecione **Set MyGameInstance**
   - Conecte o **As Umbra Game Instance** (do Cast) ao **MyGameInstance** de **Set MyGameInstance**
   - Conecte o **exec pin** de **True** ao **exec pin** de **Set MyGameInstance**

6. **Limpar VBoxes:**
   - Arraste do **exec pin** de **Set MyGameInstance** → Digite "Clear Children" → Selecione **Clear Children**
   - Conecte o **VBox_SelectedCharacter** (variável do widget) ao **Content** de **Clear Children**
   - Conecte o **exec pin** de **Set MyGameInstance** ao **exec pin** de **Clear Children**
   - Arraste do **exec pin** de **Clear Children** → Digite "Clear Children" → Selecione **Clear Children** (novo)
   - Conecte o **VBox_CharacterList** (variável do widget) ao **Content** de **Clear Children**
   - Conecte o **exec pin** do primeiro **Clear Children** ao **exec pin** do segundo **Clear Children**

**IMPORTANTE**: 
- ❌ **NÃO chame `PopulateCharacterSelectButtons` no Event Construct!**
- ✅ O `PopulateCharacterSelectButtons` será chamado pelo Level Blueprint quando o evento `OnCharacterListLoaded` disparar

### **7.2 - Função: PopulateCharacterSelectButtons**

**No `WBP_CharacterSelection` → Functions → + New Function**

Nome: `PopulateCharacterSelectButtons`

**Graph:**
```
[PopulateCharacterSelectButtons]
    ↓
[Clear Children] VBox_CharacterList
    ↓
[Get] MyGameInstance
    ↓
[Is Valid?]
    ├─→ [FALSE] → [Print String] "GameInstance None" → STOP
    └─→ [TRUE] →
        ↓
        [Get Current Players] (Array)
        ↓
        [For Each Loop] CurrentPlayers
        • Array: (CurrentPlayers)
        • Loop Body: (Array Element = CharacterData)
            ↓
            [Break Struct] CharacterData
            ↓
            [Create Widget] WBP_CharacterSelectButton
            • Owning Player: (Get Owning Player)
            ↓
            [Is Valid?]
            ├─→ [FALSE] → [Continue Loop]
            └─→ [TRUE] →
                ↓
                [Call Function: SetCharacterData] (no widget criado)
                • Character: (CharacterData)
                ↓
                [Call Function: SetSelectionWidget] (no widget criado)
                • Widget: (Self)
                ↓
                [Add Child to Vertical Box]
                • Target: VBox_CharacterList
                • Content: (Widget criado)
```


### **7.3 - Custom Event: OnSelectCharacterClicked**

**No `WBP_CharacterSelection` → Functions → + New Custom Event**

Nome: `OnSelectCharacterClicked`

**Inputs:**
- `PlayerID` (int32)

**Graph:**
```
[OnSelectCharacterClicked]
    ↓
[Is Valid?] SelectionManager
    ├─→ [FALSE] → [Print String] "Manager None" → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: SelectCharacterPreview] (no Manager)
        • PlayerID: (PlayerID)
```

**IMPORTANTE**: 
- Este widget abre IMEDIATAMENTE ao abrir o level!
- A função `PopulateCharacterSelectButtons` é chamada no Event Construct para popular o `VBox_CharacterList` com botões "Select" para cada personagem.
- O `VBox_SelectedCharacter` começa vazio e será populado com `WBP_CharacterItem` quando o usuário clicar "Select" em um personagem.
- **O `WBP_CharacterItem` NÃO tem botão "Select", apenas informações do personagem e botão "Play"!**

---

## 📝 **FASE 8: Integração com Lvl_Tutorial**

### **7.1 - Level Blueprint do Lvl_Tutorial**

**No `Lvl_Tutorial` → Level Blueprint:**

```
[Event BeginPlay]
    ↓
[Delay] 0.2s
    ↓
[Get Game Instance]
    ↓
[Cast to UmbraGameInstance]
    ↓
[Has Active Character?]
    ├─→ [FALSE] →
    │   ↓
    │   [Open Level] "Lvl_CharacterSelection"
    │   ↓
    │   STOP
    └─→ [TRUE] →
        ↓
        [Get Active Character]
        ↓
        [Break Struct] Position
        ↓
        [Find Player Start] (ou usar posição do personagem)
        ↓
        [Spawn Player Character]
        • Class: BP_ThirdPersonCharacter (ou seu character)
        • Location: (Position do personagem ou Player Start)
        • Rotation: (0, 0, 0)
        ↓
        [Possess]
        • Player Controller: (Get Player Controller)
        • Pawn: (Character spawnado)
        ↓
        [Set Show Mouse Cursor]
        • Target: (Get Player Controller)
        • Show Mouse Cursor: FALSE
        ↓
        [Set Input Mode Game Only]
        • Player Controller: (Get Player Controller)
```

---

## 📝 **FASE 9: WBP_CharacterItem (Usado APENAS dentro do VBox)**

### **9.1 - IMPORTANTE: WBP_CharacterItem é usado APENAS dentro do VBox**

**NÃO é usado na lista do Dashboard!** Ele só aparece quando o usuário clica "SELECIONAR" em um personagem no level.

### **9.2 - Adicionar Variável para Referência ao Manager**

**No `WBP_CharacterItem` → Variables:**

```
- SelectionManager (BP_CharacterSelectionManager) - Referência ao manager
```

**Marcar como "Is Variable" = TRUE**

### **9.3 - Função: SetSelectionContext**

**No `WBP_CharacterItem` → Functions → + New Function**

Nome: `SetSelectionContext`

**Inputs:**
- `Manager` (BP_CharacterSelectionManager) - Object Reference

**Graph:**
```
[SetSelectionContext]
    ↓
[Set] SelectionManager (Manager)
```

### **9.4 - Modificar BTN_Play**

**SUBSTITUIR a lógica atual completamente:**

```
[BTN_Play OnClicked]
    ↓
[Get] MyGameInstance
    ↓
[Is Valid?]
    ├─→ [FALSE] → [Print String] "GameInstance None" → STOP
    └─→ [TRUE] →
        ↓
        [Break Struct] CharacterData
        ↓
        [SelectCharacter]  ← Garantir que está selecionado
        • PlayerID: (ID)
        ↓
        [Delay] 0.5s  ← Aguardar seleção completar
        ↓
        [Open Level]
        • Level Name: "Lvl_Tutorial"
        • bAbsolute: TRUE
        ↓
        [Set Show Mouse Cursor]
        • Target: (Get Player Controller)
        • Show Mouse Cursor: FALSE
```

### **9.5 - Adicionar ou Modificar BTN_Return**

**Se o `WBP_CharacterItem` já tiver um botão "Retornar", ou adicionar um:**

**COMO FAZER NO BLUEPRINT:**

1. **No `WBP_CharacterItem` → Designer:**
   - Adicione um **Button** chamado **BTN_Return** (se não existir)
   - Adicione um **Text Block** dentro do botão com o texto "RETORNAR" ou "VOLTAR"

2. **No `WBP_CharacterItem` → Event Graph:**
   - Localize o botão **BTN_Return**
   - Clique com botão direito no botão → **"On Clicked"**

3. **No Graph do OnClicked:**

   **PASSO 1: Validar SelectionManager**
   - Arraste do **exec pin** do evento → Digite "Is Valid" → Selecione **Is Valid?**
   - Arraste do **Target** (self) → Digite "Get SelectionManager" → Selecione **Get SelectionManager**
   - Conecte o **SelectionManager** (saída de **Get SelectionManager**) ao **Object** de **Is Valid?**
   - Arraste do **Is Valid** (saída booleana) → Digite "Branch" → Selecione **Branch**
   - Conecte o **exec pin** do evento ao **exec pin** de **Branch**

   **PASSO 2: Se FALSE (Manager inválido)**
   - Arraste do **False** (saída do Branch) → Digite "Print String" → Selecione **Print String**
   - No campo **In String** de **Print String**, digite: `"Manager None"`

   **PASSO 3: Se TRUE (Manager válido) - Chamar ResetSelection**
   - Arraste do **True** (saída do Branch) → Digite "Reset Selection" → Selecione **ResetSelection**
   - Conecte o **SelectionManager** (de **Get SelectionManager**) ao **Target** de **ResetSelection**
   - No campo **Preview Class** de **ResetSelection**, selecione **BP_CharacterPreview**
   - No campo **Spacing** de **ResetSelection**, digite: `300.0`
   - Conecte o **exec pin** de **True** ao **exec pin** de **ResetSelection**

   **PASSO 4: Remover Widget do VBox**
   - Arraste do **exec pin** de **ResetSelection** → Digite "Remove from Parent" → Selecione **Remove from Parent**
   - Conecte o **exec pin** de **ResetSelection** ao **exec pin** de **Remove from Parent**

**GRAPH VISUAL:**
```
[BTN_Return OnClicked]
    ↓
[Is Valid?] SelectionManager
    ├─→ [FALSE] → [Print String] "Manager None" → STOP
    └─→ [TRUE] →
        ↓
        [Call Function: ResetSelection] (no Manager)
        • Target: (SelectionManager)
        • Preview Class: BP_CharacterPreview
        • Spacing: 300.0
        ↓
        [Remove from Parent] (Self)  ← Remove este WBP_CharacterItem do VBox
```

**RESULTADO:**
- Quando o usuário clica "Return":
  - A função C++ `ResetSelection` é chamada
  - Limpa o `VBox_SelectedCharacter`
  - Respawna todos os personagens
  - O `WBP_CharacterItem` é removido do VBox
  - Volta ao estado inicial (todos os personagens visíveis na tela)

---


---

## 📝 **FASE 10: Fluxo Completo**

### **10.1 - Fluxo do Usuário**

```
1. Login → Abre Lvl_CharacterSelection automaticamente
   ↓
2. Level Blueprint:
   - Spawna BP_CharacterSelectionManager
   - Chama Initialize no Manager
   ↓
3. Manager:
   - Pega Current Players do GameInstance
   - Spawna TODOS os BP_CharacterPreview (um para cada personagem)
   ↓
4. WBP_CharacterSelection:
   - Popula VBox_CharacterList com botões "Select" para cada personagem (via PopulateCharacterSelectButtons)
   ↓
5. Usuário vê:
   - TODOS os personagens na tela (3D)
   - WBP_CharacterSelection aberto com botões "Select" no VBox_CharacterList
   ↓
6. Usuário clica no botão "Select" (no WBP_CharacterSelection)
   ↓
7. Manager chama SelectCharacterPreview(PlayerID):
   - Despawne outros personagens
   - Move câmera para o personagem selecionado
   - Chama SelectCharacter no GameInstance
   - Carrega informações completas
   - Chama AddCharacterItemToWidget(PlayerID)
   ↓
8. Manager.AddCharacterItemToWidget:
    - Cria WBP_CharacterItem
    - Chama SetCharacterData no WBP_CharacterItem
    - Chama SetSelectionContext no WBP_CharacterItem
    - Limpa VBox_SelectedCharacter (remove itens anteriores)
    - Adiciona WBP_CharacterItem ao VBox_SelectedCharacter do WBP_CharacterSelection
   ↓
9. WBP_CharacterItem aparece no VBox_SelectedCharacter do WBP_CharacterSelection com:
    - Informações do personagem selecionado
    - Botão "Play"
   ↓
10. Usuário clica "Play" (no WBP_CharacterItem) → Abre Lvl_Tutorial e spawna o player
```

---

## ✅ **CHECKLIST DE IMPLEMENTAÇÃO**

### **Níveis:**
- [x ] `Lvl_CharacterSelection` criado
- [x ] `CameraActor` adicionado ao level
- [x ] Level Blueprint configurado
- [x ] Player Start adicionado
- [ x] Iluminação configurada

### **Blueprints:**
- [x ] `BP_CharacterPreview` criado
- [ x] Componentes configurados (Mesh, Widget Component)
- [ x] Funções implementadas:
  - [x ] `SetCharacterData`
  - [ x] `SetManagerReference`
  - [x ] `UpdateCharacterMesh`
- [ x] Variáveis `PlayerID`, `CharacterData`, `Manager` criadas
- [ x] `BP_CharacterSelectionManager` criado
- [ x] Variáveis do Manager criadas (CharacterPreviews, SelectedPreview, SelectionWidget, PlayerControllerRef)
- [x ] Funções do Manager implementadas:
  - [x ] `Initialize`
  - [x ] `SpawnAllCharacterPreviews`
  - [ x] `SelectCharacterPreview`
  - [ x] `AddCharacterItemToWidget`
  - [ x] `GetPreviewByPlayerID`
  - [x ] `DespawnOtherPreviews`
  - [ x] `ResetSelection`

### **Widgets:**
- [x ] `WBP_CharacterSelection` criado
- [ ] Layout configurado (ScrollBox + VBox - abre IMEDIATAMENTE)
- [ ] Variáveis criadas (SelectionManager, ScrollBox_CharacterList, VBox_CharacterList, VBox_SelectedCharacter)
- [ ] Função PopulateCharacterSelectButtons implementada
- [ ] PopulateCharacterSelectButtons chamada no OnCharacterListLoaded_Event (Level Blueprint) - ❌ NÃO no Event Construct!
- [ ] Custom Event OnSelectCharacterClicked implementado
- [ ] `WBP_CharacterSelectButton` criado (Widget simples para botões "Select" na lista)
- [ ] `WBP_CharacterPreviewWidget` criado (Widget 3D com botão "SELECIONAR")
- [ ] `WBP_CharacterItem` atualizado:
  - [ ] Variável `SelectionManager` adicionada
  - [ ] Função `SetSelectionContext` implementada
  - [ ] BTN_Play abre Lvl_Tutorial (dentro do VBox)
  - [ ] BTN_Return chama ResetSelection no Manager e remove do VBox

### **Integração:**
- [ ] `Lvl_Tutorial` verifica se tem personagem ativo
- [ ] Spawn do player no `Lvl_Tutorial` implementado
- [ ] Input mode configurado corretamente
- [ ] Câmera se move ao selecionar personagem (similar ao creator)

---

## 🔧 **NOTAS IMPORTANTES**

### **1. Visualização 3D de Todos os Personagens**

- Cada personagem da conta será spawnado como um `BP_CharacterPreview`
- Posicione-os em linha (similar ao creator de classes)
- Espaçamento recomendado: 300 unidades entre cada personagem
- Exemplo: Personagem 1 em (0, 0, 0), Personagem 2 em (300, 0, 0), etc.

### **2. Movimento de Câmera (Similar ao Creator)**

- Use a função `SelectClassAndMoveCamera` do `BP_CharacterCreationManager` (ou crie similar)
- Ao selecionar um personagem, a câmera se move suavemente para mostrar apenas ele
- Outros personagens podem ser despawnados ou escondidos
- Posição da câmera: Offset relativo ao personagem selecionado

### **3. Seleção de Personagem**

- Ao clicar "SELECIONAR" em um personagem, o `Manager` chama `SelectCharacterPreview`
- Isso move a câmera, despawne outros e carrega informações completas
- O Manager cria `WBP_CharacterItem` e adiciona ao `VBox_SelectedCharacter` do `WBP_CharacterSelection`
- **O `WBP_CharacterItem` contém APENAS informações do personagem e botão "Play" - NÃO tem botão "Select"!**
- O `WBP_CharacterItem` recebe o manager via `SetSelectionContext` para poder chamar `ResetSelection` no botão "Retornar"
- **IMPORTANTE**: `WBP_CharacterItem` é usado APENAS dentro do VBox, NÃO na lista do Dashboard!

### **4. Spawn do Player no Lvl_Tutorial**

O player deve ser spawnado na posição salva do personagem (`Position` do `CharacterData`). Se a posição for (0,0,0), use o `Player Start`.

### **5. Validações**

Sempre valide:
- ✅ GameInstance existe
- ✅ Player Controller existe
- ✅ Array de personagens não está vazio
- ✅ Manager spawnado com sucesso
- ✅ Previews spawnados com sucesso
- ✅ Widget criado com sucesso

---

## 🎯 **PRÓXIMOS PASSOS**

1. **Implementar visualização 3D completa** (quando tiver meshes)
2. **Adicionar animações** ao personagem preview
3. **Adicionar rotação** do personagem (mouse drag)
4. **Adicionar zoom** (mouse wheel)
5. **Melhorar UI** com ícones e estilos

---

## 📚 **RESUMO**

**Estrutura Final:**
```
Lvl_CharacterSelection (Novo Level)
├── CameraActor (Câmera principal do level)
├── BP_CharacterSelectionManager (Manager - spawna e gerencia previews)
│   └── CharacterPreviews[] (Array de BP_CharacterPreview)
│
├── BP_CharacterPreview (Actor 3D - um para cada personagem)
│   ├── CharacterMesh (Skeletal Mesh)
│   └── CharacterWidget (Widget Component - opcional)
│
└── WBP_CharacterSelection (Widget principal - abre IMEDIATAMENTE)
    ├── ScrollBox_CharacterList
    │   └── VBox_CharacterList (populado com botões "Select" para cada personagem)
    └── VBox_SelectedCharacter (recebe WBP_CharacterItem quando um personagem é selecionado)

WBP_CharacterItem (Usado dentro do VBox_SelectedCharacter)
├── Informações do personagem (nome, nível, stats, etc.)
└── BTN_Play → Abre Lvl_Tutorial
(NÃO tem botão "Select"!)

Lvl_Tutorial (Atualizado)
└── Spawna player na posição salva
```

**Fluxo:**
```
Login → Abre Lvl_CharacterSelection automaticamente
    ↓
Level Blueprint (Event BeginPlay):
  - Spawna Manager
  - Cria WBP_CharacterSelection IMEDIATAMENTE
  - Adiciona ao viewport
  - Conecta ao evento OnCharacterListLoaded
    ↓
GameInstance.LoadCharacterList() (chamado pelo sistema de login)
  - Faz request HTTP
    ↓
Quando dados chegam:
  - OnCharacterListLoaded dispara
    ↓
Level Blueprint (OnCharacterListLoaded_Event):
  - Chama Initialize no Manager
  - Chama PopulateCharacterSelectButtons no Widget
    ↓
Manager.Initialize:
  - Spawna TODOS os personagens (3D)
    ↓
Widget.PopulateCharacterSelectButtons:
  - Popula VBox_CharacterList com botões "Select"
    ↓
Usuário vê:
  - TODOS os personagens na tela (3D)
  - WBP_CharacterSelection aberto com botões "Select" no VBox_CharacterList
    ↓
Usuário clica "Select" em um botão (no WBP_CharacterSelection)
    ↓
Manager:
  - Move câmera
  - Despawne outros personagens
  - Cria WBP_CharacterItem
  - Adiciona WBP_CharacterItem ao VBox_SelectedCharacter do WBP_CharacterSelection
    ↓
WBP_CharacterItem aparece no VBox_SelectedCharacter com:
  - Informações do personagem selecionado
  - Botão "Play" (que abre Lvl_Tutorial)
    ↓
Usuário clica "Play" (no WBP_CharacterItem)
    ↓
Abre Lvl_Tutorial e spawna o player
```

**Similar ao sistema de criação de personagem, mas para seleção!**

