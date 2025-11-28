# 📋 GUIA COMPLETO: Implementação do Widget WBP_CharacterInfo

## 🎯 **OBJETIVO:**

Criar um widget completo de informações do personagem que será aberto ao pressionar a tecla **C**, exibindo todas as informações do personagem, stats e equipamentos.

---

## ✅ **PRÉ-REQUISITOS:**

1. ✅ C++ compilado com sucesso
2. ✅ API PHP funcionando
3. ✅ Estruturas C++ implementadas

---

## 🔧 **PARTE 1: Criar Widget Blueprint**

### **1.1 Criar o Widget**

1. **No Content Browser:**
   - Navegue até: `Content/Widgets/UI/Character/`
   - Clique com botão direito → **User Interface** → **Widget Blueprint**
   - Nome: `WBP_CharacterInfo`
   - Parent Class: **User Widget**

2. **Abrir o Widget:**
   - Dê duplo clique em `WBP_CharacterInfo`

---

## 🔧 **PARTE 2: Criar Layout do Widget**

### **2.1 Estrutura Base**

**NO DESIGNER (Visual Editor):**

1. **Root (Canvas Panel):**
   - Selecione o **Canvas Panel** (root)
   - **Anchors:** Preenche toda a tela
   - **Size:** 1920x1080 (ou ajuste conforme necessário)

2. **Background (Image):**
   - Adicione um **Image** como filho do Canvas Panel
   - Nome: `Image_Background`
   - **Anchors:** Preenche toda a tela
   - **Color and Opacity:** Preto com 80% de opacidade (para overlay escuro)

3. **Main Container (Vertical Box):**
   - Adicione um **Vertical Box** como filho do Canvas Panel
   - Nome: `VB_MainContainer`
   - **Anchors:** Centro da tela
   - **Size:** 1200x900 (ajuste conforme necessário)
   - **Alignment:** Centro (0.5, 0.5)

### **2.2 Header (Nome, Nível, EXP)**

**DENTRO DE `VB_MainContainer`:**

1. **Header Container (Horizontal Box):**
   - Adicione um **Horizontal Box** como primeiro filho
   - Nome: `HB_Header`
   - **Padding:** 20px em todos os lados

2. **Nome do Personagem (TextBlock):**
   - Adicione um **TextBlock** em `HB_Header`
   - Nome: `Text_CharacterName`
   - **Text:** "Nome do Personagem"
   - **Font Size:** 32
   - **Color:** Branco

3. **Nível (TextBlock):**
   - Adicione um **TextBlock** em `HB_Header`
   - Nome: `Text_Level`
   - **Text:** "Nível: 1"
   - **Font Size:** 24
   - **Color:** Amarelo

4. **EXP Container (Vertical Box):**
   - Adicione um **Vertical Box** em `HB_Header`
   - Nome: `VB_EXPContainer`
   - **Fill Width:** True

5. **Progress Bar EXP:**
   - Adicione um **Progress Bar** em `VB_EXPContainer`
   - Nome: `ProgressBar_EXP`
   - **Percent:** 0.0
   - **Fill Color:** Verde

6. **Text EXP:**
   - Adicione um **TextBlock** em `VB_EXPContainer`
   - Nome: `Text_EXP`
   - **Text:** "0 / 1000"
   - **Font Size:** 14
   - **Color:** Branco

### **2.3 Container Principal (Horizontal Box)**

**DENTRO DE `VB_MainContainer` (após Header):**

1. **Main Content (Horizontal Box):**
   - Adicione um **Horizontal Box** como segundo filho
   - Nome: `HB_MainContent`
   - **Fill Height:** True

### **2.4 Seção Esquerda - Stats Base**

**DENTRO DE `HB_MainContent`:**

1. **Stats Container (Vertical Box):**
   - Adicione um **Vertical Box** como primeiro filho
   - Nome: `VB_StatsContainer`
   - **Size:** 300px de largura
   - **Padding:** 20px

2. **Título Stats (TextBlock):**
   - Adicione um **TextBlock** em `VB_StatsContainer`
   - Nome: `Text_StatsTitle`
   - **Text:** "Atributos"
   - **Font Size:** 20
   - **Color:** Branco

3. **Força (Horizontal Box):**
   - Adicione um **Horizontal Box** em `VB_StatsContainer`
   - Nome: `HB_Strength`
   - **Padding:** 5px

4. **Text Força:**
   - Adicione um **TextBlock** em `HB_Strength`
   - Nome: `Text_Strength`
   - **Text:** "Força: 10 / 10"
   - **Font Size:** 16
   - **Color:** Branco

**REPITA PARA OS OUTROS ATRIBUTOS:**
- `HB_Agility` → `Text_Agility` ("Agilidade: 10 / 10")
- `HB_Intelligence` → `Text_Intelligence` ("Inteligência: 10 / 10")
- `HB_Constitution` → `Text_Constitution` ("Constituição: 10 / 10")
- `HB_Luck` → `Text_Luck` ("Sorte: 10 / 10")
- `HB_Dexterity` → `Text_Dexterity` ("Destreza: 10 / 10")
- `HB_Vitality` → `Text_Vitality` ("Vitalidade: 10 / 10")

4. **Separador (Spacer):**
   - Adicione um **Spacer** em `VB_StatsContainer`
   - **Size:** 20px de altura

5. **Vida (Vertical Box):**
   - Adicione um **Vertical Box** em `VB_StatsContainer`
   - Nome: `VB_Health`
   - **Padding:** 5px

6. **Text Vida:**
   - Adicione um **TextBlock** em `VB_Health`
   - Nome: `Text_Health`
   - **Text:** "Vida: 100 / 100"
   - **Font Size:** 16
   - **Color:** Vermelho

7. **Progress Bar Vida:**
   - Adicione um **Progress Bar** em `VB_Health`
   - Nome: `ProgressBar_Health`
   - **Percent:** 1.0
   - **Fill Color:** Vermelho

**REPITA PARA MANA E STAMINA:**
- `VB_Mana` → `Text_Mana` + `ProgressBar_Mana` (Azul)
- `VB_Stamina` → `Text_Stamina` + `ProgressBar_Stamina` (Amarelo)

### **2.5 Seção Central - Equipamentos**

**DENTRO DE `HB_MainContent` (segundo filho):**

1. **Equipment Container (Horizontal Box):**
   - Adicione um **Horizontal Box` como segundo filho
   - Nome: `HB_EquipmentContainer`
   - **Fill Width:** True
   - **Padding:** 20px

2. **Coluna Esquerda (Vertical Box):**
   - Adicione um **Vertical Box** em `HB_EquipmentContainer`
   - Nome: `VB_EquipmentLeft`
   - **Size:** 100px de largura

3. **Slots de Equipamento Esquerda:**
   - Adicione **Image** widgets para cada slot:
     - `Image_Slot_Head` (Cabeça)
     - `Image_Slot_Chest` (Armadura)
     - `Image_Slot_Hands` (Luvas)
     - `Image_Slot_Feet` (Botas)
   - **Size:** 80x80px cada
   - **Padding:** 5px entre eles

4. **Coluna Direita (Vertical Box):**
   - Adicione um **Vertical Box** em `HB_EquipmentContainer`
   - Nome: `VB_EquipmentRight`
   - **Size:** 100px de largura

5. **Slots de Equipamento Direita:**
   - Adicione **Image** widgets:
     - `Image_Slot_Ring` (Anel)
     - `Image_Slot_Earring` (Brinco)
     - `Image_Slot_Bracelet` (Bracelete)
     - `Image_Slot_Amulet` (Colar)

6. **Coluna Centro (Vertical Box):**
   - Adicione um **Vertical Box** em `HB_EquipmentContainer`
   - Nome: `VB_EquipmentCenter`
   - **Size:** 150px de largura

7. **Slots de Equipamento Centro:**
   - Adicione **Image** widgets:
     - `Image_Slot_MainHand` (Mão Principal)
     - `Image_Slot_OffHand` (Mão Secundária)
     - `Image_Slot_Mount` (Montaria)

### **2.6 Seção Direita - Stats de Combate**

**DENTRO DE `HB_MainContent` (terceiro filho):**

1. **Combat Stats Container (Vertical Box):**
   - Adicione um **Vertical Box` como terceiro filho
   - Nome: `VB_CombatStatsContainer`
   - **Size:** 300px de largura
   - **Padding:** 20px

2. **Título Combat Stats (TextBlock):**
   - Adicione um **TextBlock** em `VB_CombatStatsContainer`
   - Nome: `Text_CombatStatsTitle`
   - **Text:** "Stats de Combate"
   - **Font Size:** 20
   - **Color:** Branco

3. **Stats de Combate (TextBlocks):**
   - Adicione **TextBlock** para cada stat:
     - `Text_PhysicalAttack` ("Ataque Físico: 0")
     - `Text_MagicAttack` ("Ataque Mágico: 0")
     - `Text_PhysicalDefense` ("Defesa Física: 0")
     - `Text_MagicDefense` ("Defesa Mágica: 0")
     - `Text_Accuracy` ("Acerto: 0")
     - `Text_Dodge` ("Esquiva: 0")
     - `Text_Critical` ("Crítico: 0")
     - `Text_Movement` ("Movimento: 0")
     - `Text_Resistance` ("Resistência: 0")
     - `Text_DoubleAttackRate` ("Taxa Ataque Duplo: 0")

### **2.7 Botão Fechar**

**DENTRO DE `VB_MainContainer` (último filho):**

1. **Button Fechar:**
   - Adicione um **Button** como último filho
   - Nome: `Button_Close`
   - **Text:** "Fechar (C)"
   - **Size:** 150x40px
   - **Alignment:** Centro horizontal

---

## 🔧 **PARTE 3: Implementar Funções Blueprint**

### **3.1 Event Construct**

**NO EVENT GRAPH:**

1. **Event Construct:**
   - Arraste **Event Construct** para o gráfico

2. **Get Game Instance:**
   - Adicione **Get Game Instance**
   - Conecte **Event Construct** → **Get Game Instance**

3. **Cast to Umbra Game Instance:**
   - Adicione **Cast to Umbra Game Instance**
   - Conecte **Return Value** (Get Game Instance) → **Object** (Cast)
   - Conecte **Get Game Instance** → **Cast to Umbra Game Instance**

4. **Bind Event to OnCharacterInfoLoaded:**
   - Adicione **Bind Event to OnCharacterInfoLoaded**
   - **Target:** Cast to Umbra Game Instance (Return Value)
   - Conecte **Cast to Umbra Game Instance** → **Bind Event**
   - **Event:** Clique no dropdown e selecione **Create Event** → **OnCharacterInfoLoaded_Event**

5. **Bind Event to OnCharacterInfoFailed:**
   - Adicione **Bind Event to OnCharacterInfoFailed`
   - **Target:** Cast to Umbra Game Instance (Return Value)
   - Conecte **Cast to Umbra Game Instance** → **Bind Event**
   - **Event:** Clique no dropdown e selecione **Create Event** → **OnCharacterInfoFailed_Event**

6. **Load Character Info:**
   - Adicione **Load Character Info`
   - **Target:** Cast to Umbra Game Instance (Return Value)
   - Conecte **Cast to Umbra Game Instance** → **Load Character Info**
   - Conecte **Bind Event to OnCharacterInfoFailed** → **Load Character Info`

### **3.2 OnCharacterInfoLoaded_Event**

**CRIAR EVENTO CUSTOMIZADO:**

1. **OnCharacterInfoLoaded_Event:**
   - No **Event Graph**, clique com botão direito
   - **Add Custom Event**
   - Nome: `OnCharacterInfoLoaded_Event`
   - Adicione **Input Pin:** `Character Info` (tipo: **Umbra Character Info**)

2. **Update Character Info Display:**
   - Adicione **Update Character Info Display** (função que criaremos)
   - Conecte **Character Info** (input) → **Character Info** (função)
   - Conecte **OnCharacterInfoLoaded_Event** → **Update Character Info Display**

### **3.3 OnCharacterInfoFailed_Event**

**CRIAR EVENTO CUSTOMIZADO:**

1. **OnCharacterInfoFailed_Event:**
   - Clique com botão direito → **Add Custom Event**
   - Nome: `OnCharacterInfoFailed_Event`
   - Adicione **Input Pin:** `ErrorMessage` (tipo: **String**)

2. **Print String (opcional, para debug):**
   - Adicione **Print String**
   - Conecte **ErrorMessage** → **In String**
   - Conecte **OnCharacterInfoFailed_Event** → **Print String**

### **3.4 Função: Update Character Info Display**

**CRIAR FUNÇÃO:**

1. **Criar Função:**
   - No **Functions** tab, clique em **+ Function**
   - Nome: `Update Character Info Display`
   - Adicione **Input:** `Character Info` (tipo: **Umbra Character Info**)

2. **Break Umbra Character Info:**
   - Adicione **Break Umbra Character Info**
   - Conecte **Character Info** (input) → **Character Info** (Break)

3. **Atualizar Nome:**
   - Adicione **Set Text**
   - **Target:** `Text_CharacterName`
   - Conecte **Character Name** (Break) → **Text** (Set Text)
   - Conecte **Entry** (função) → **Set Text**

4. **Atualizar Nível:**
   - Adicione **Set Text**
   - **Target:** `Text_Level`
   - Adicione **Format Text**
   - **Format:** "Nível: {Level}"
   - Conecte **Level** (Break) → **Level** (Format Text)
   - Conecte **Return Value** (Format Text) → **Text** (Set Text)

5. **Atualizar EXP:**
   - Adicione **Set Percent**
   - **Target:** `ProgressBar_EXP`
   - Conecte **Exp Progress Percent** (Break) → **Percent** (Set Percent)
   - Adicione **Format Text**
   - **Format:** "{Experience} / {ExpForNextLevel}"
   - Conecte **Experience** (Break) → **Experience** (Format Text)
   - Conecte **Exp For Next Level** (Break) → **ExpForNextLevel** (Format Text)
   - Conecte **Return Value** (Format Text) → **Text** (Set Text para `Text_EXP`)

6. **Atualizar Stats Base:**
   - Para cada atributo (Strength, Agility, etc.):
     - Adicione **Format Text**
     - **Format:** "{StatName}: {Base} / {Total}"
     - Conecte **Base Strength** (Break) → **Base** (Format Text)
     - Conecte **Total Strength** (Break) → **Total** (Format Text)
     - Conecte **Return Value** → **Text** (Set Text para `Text_Strength`)

7. **Atualizar Vida:**
   - Adicione **Format Text**
   - **Format:** "Vida: {Current} / {Max}"
   - Conecte **Current Health** (Break) → **Current** (Format Text)
   - Conecte **Max Health Total** (Break) → **Max** (Format Text)
   - Conecte **Return Value** → **Text** (Set Text para `Text_Health`)
   - Adicione **Set Percent**
   - **Target:** `ProgressBar_Health`
   - Adicione **Divide** (Float)
   - Conecte **Current Health** → **A** (Divide)
   - Conecte **Max Health Total** → **B** (Divide)
   - Conecte **Return Value** (Divide) → **Percent** (Set Percent)

**REPITA PARA MANA E STAMINA**

8. **Atualizar Stats de Combate:**
   - Para cada stat de combate:
     - Adicione **Format Text**
     - **Format:** "{StatName}: {Value}"
     - Conecte o valor correspondente (Break) → **Value** (Format Text)
     - Conecte **Return Value** → **Text** (Set Text para o TextBlock correspondente)

9. **Update Equipment Slots:**
   - Adicione **Update Equipment Slots** (função que criaremos)
   - Conecte **Character Info** (input) → **Character Info** (função)
   - Conecte após todas as atualizações de stats

### **3.5 Função: Update Equipment Slots**

**CRIAR FUNÇÃO:**

1. **Criar Função:**
   - Nome: `Update Equipment Slots`
   - Adicione **Input:** `Character Info` (tipo: **Umbra Character Info**)

2. **Break Umbra Character Info:**
   - Adicione **Break Umbra Character Info**
   - Conecte **Character Info** (input) → **Character Info** (Break)

3. **Get Equipped Items:**
   - Adicione **Get Equipped Items`
   - Conecte **Character Info** (input) → **Character Info** (Get)
   - Isso retorna um **TMap** de `EUmbraEquipmentSlot` → `FUmbraInventorySlot`

4. **Switch on EUmbraEquipmentSlot:**
   - Adicione **Switch on EUmbraEquipmentSlot**
   - Conecte **Get Equipped Items** → **Switch**

5. **Para cada Slot (Head, Chest, etc.):**
   - No **Switch**, adicione um **Case** para cada slot
   - Exemplo para **Head:**
     - **Case:** `Head`
     - Adicione **Get** no TMap (retorna `FUmbraInventorySlot`)
     - Adicione **Break Umbra Inventory Slot**
     - Conecte o **Value** do Get → **Break**
     - Adicione **Set Brush from Texture**
     - **Target:** `Image_Slot_Head`
     - Conecte **Item Icon** (Break) → **Texture** (Set Brush)
     - Adicione **Set Brush Color** (opcional, para raridade)
     - Conecte após **Set Brush from Texture**

6. **Para Slots Vazios:**
   - Adicione **Set Brush** (sem textura) para slots que não têm item
   - Ou use uma textura padrão de slot vazio

---

## 🔧 **PARTE 4: Implementar Input C**

### **4.1 Criar Input Action**

1. **Project Settings:**
   - **Edit** → **Project Settings**
   - **Engine** → **Input**
   - **Action Mappings:**
     - Clique em **+**
     - **Action Name:** `OpenCharacterInfo`
     - **Key:** `C`

### **4.2 Implementar no Character/Player Controller**

**NO BLUEPRINT DO PERSONAGEM OU PLAYER CONTROLLER:**

1. **Event BeginPlay:**
   - Adicione **Event BeginPlay**
   - Adicione **Enable Input**
   - Conecte **Event BeginPlay** → **Enable Input**
   - Adicione **Get Player Controller**
   - Adicione **Set Input Mode Game And UI**
   - Conecte **Get Player Controller** → **Player Controller** (Set Input Mode)
   - Conecte **Enable Input** → **Set Input Mode Game And UI**

2. **OpenCharacterInfo (Action):**
   - Adicione **OpenCharacterInfo** (Action Event)
   - Adicione **Get Game Instance**
   - Adicione **Cast to Umbra Game Instance**
   - Conecte **Get Game Instance** → **Cast**
   - Adicione **Load Character Info**
   - **Target:** Cast to Umbra Game Instance (Return Value)
   - Conecte **OpenCharacterInfo** → **Load Character Info**

3. **Criar Widget:**
   - Adicione **Create Widget**
   - **Class:** `WBP_CharacterInfo`
   - Adicione **Add to Viewport**
   - Conecte **Return Value** (Create Widget) → **Target** (Add to Viewport)
   - Conecte **Load Character Info** → **Add to Viewport**

4. **Variável para Widget:**
   - Crie uma **Variable** no Character/Player Controller
   - Nome: `CharacterInfoWidget`
   - Tipo: `WBP Character Info` (Object Reference)
   - **Instance Editable:** False

5. **Verificar se Widget já está aberto:**
   - Adicione **Is Valid**
   - Conecte **CharacterInfoWidget** → **Is Valid**
   - Adicione **Branch**
   - Conecte **Is Valid** → **Condition** (Branch)
   - **True:** **Remove from Parent** (CharacterInfoWidget)
   - **False:** **Create Widget** + **Add to Viewport** (como acima)
   - Conecte **Return Value** (Create Widget) → **Set** (CharacterInfoWidget)

### **4.3 Fechar Widget**

**NO WBP_CharacterInfo:**

1. **Button_Close OnClicked:**
   - Selecione `Button_Close` no Designer
   - No **Details**, encontre **Events** → **On Clicked**
   - Clique no **+** para criar evento

2. **Remove from Parent:**
   - No **Event Graph**, adicione **Remove from Parent**
   - **Target:** `self`
   - Conecte **OnClicked** → **Remove from Parent**

---

## 🔧 **PARTE 5: Funções Auxiliares**

### **5.1 Função: Get Equipment Slot Image**

**CRIAR FUNÇÃO:**

1. **Criar Função:**
   - Nome: `Get Equipment Slot Image`
   - **Input:** `Equipment Slot` (tipo: **EUmbraEquipmentSlot**)
   - **Return:** `Image` (tipo: **Image**)

2. **Switch on EUmbraEquipmentSlot:**
   - Adicione **Switch on EUmbraEquipmentSlot**
   - Conecte **Equipment Slot** (input) → **Switch**

3. **Para cada Case:**
   - **Case Head:** Return `Image_Slot_Head`
   - **Case Chest:** Return `Image_Slot_Chest`
   - **Case Hands:** Return `Image_Slot_Hands`
   - **Case Feet:** Return `Image_Slot_Feet`
   - **Case MainHand:** Return `Image_Slot_MainHand`
   - **Case OffHand:** Return `Image_Slot_OffHand`
   - **Case Ring:** Return `Image_Slot_Ring`
   - **Case Earring:** Return `Image_Slot_Earring`
   - **Case Bracelet:** Return `Image_Slot_Bracelet`
   - **Case Amulet:** Return `Image_Slot_Amulet`
   - **Case Mount:** Return `Image_Slot_Mount`
   - **Default:** Return `None`

---

## 📋 **CHECKLIST DE IMPLEMENTAÇÃO:**

### **Layout:**
- [ ] Canvas Panel root criado
- [ ] Background overlay criado
- [ ] Header com nome, nível e EXP
- [ ] Seção de stats base (7 atributos)
- [ ] Progress bars para Vida, Mana, Stamina
- [ ] Slots de equipamento (11 slots)
- [ ] Seção de stats de combate (10 stats)
- [ ] Botão fechar

### **Funções:**
- [ ] Event Construct implementado
- [ ] Delegates conectados
- [ ] OnCharacterInfoLoaded_Event criado
- [ ] OnCharacterInfoFailed_Event criado
- [ ] Update Character Info Display implementado
- [ ] Update Equipment Slots implementado
- [ ] Get Equipment Slot Image implementado

### **Input:**
- [ ] Input Action "OpenCharacterInfo" criado
- [ ] Input C implementado no Character/Player Controller
- [ ] Widget abre/fecha corretamente
- [ ] Botão fechar funciona

### **Testes:**
- [ ] Widget abre ao pressionar C
- [ ] Informações são carregadas corretamente
- [ ] Stats são exibidos corretamente
- [ ] Equipamentos aparecem nos slots corretos
- [ ] Widget fecha ao pressionar C novamente ou botão fechar

---

## 🎉 **PRONTO!**

Após seguir este guia, o widget de informações do personagem estará completamente funcional!

**PRÓXIMOS PASSOS:**
1. Testar no jogo
2. Ajustar layout visual conforme necessário
3. Adicionar tooltips nos slots de equipamento (opcional)
4. Adicionar animações de abertura/fechamento (opcional)

---

## 📝 **NOTAS IMPORTANTES:**

1. **Slots de Equipamento:**
   - Use **Image** widgets para os slots
   - Configure **Size** e **Padding** adequadamente
   - Adicione bordas coloridas baseadas na raridade (opcional)

2. **Progress Bars:**
   - **Percent** deve ser um valor entre 0.0 e 1.0
   - Use **Divide** para calcular: `Current / Max`

3. **Format Text:**
   - Use para formatar números e strings
   - Exemplo: "Força: {Base} / {Total}"

4. **TMap:**
   - `Get Equipped Items` retorna um TMap
   - Use **Get** para acessar valores por chave
   - Use **Contains** para verificar se um slot tem item

---

**DÚVIDAS?** Consulte os arquivos:
- `UmbraGameInstance.cpp` - Implementação C++
- `UmbraDataStructures.h` - Estruturas de dados
- `get_character_info.php` - API PHP

