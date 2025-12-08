# 🎮 **GUIA COMPLETO: Aprimoramento do Sistema de Criação de Personagem**

## 📋 **ÍNDICE**

1. [Visão Geral](#visão-geral)
2. [Pré-requisitos](#pré-requisitos)
3. [Passo 1: Atualizar Banco de Dados](#passo-1-atualizar-banco-de-dados)
4. [Passo 2: Recompilar Projeto C++](#passo-2-recompilar-projeto-c)
5. [Passo 3: Implementar no Blueprint](#passo-3-implementar-no-blueprint)
6. [Passo 4: Testar Implementação](#passo-4-testar-implementação)
7. [Troubleshooting](#troubleshooting)

---

## 🎯 **VISÃO GERAL**

Este guia detalha a implementação completa do sistema aprimorado de criação de personagem, que agora inclui:

- ✅ Seleção de **Classe** (com stats baseados na classe escolhida)
- ✅ Seleção de **Hair** (cabelo do personagem)
- ✅ Seleção de **Head** (cabeça do personagem)
- ✅ Validação completa de todos os campos
- ✅ Criação de personagem com stats baseados na classe selecionada

---

## ✅ **PRÉ-REQUISITOS**

Antes de começar, certifique-se de que:

- ✅ MySQL/MariaDB está rodando
- ✅ Banco de dados `umbra_eternum` existe
- ✅ Tabela `classes` existe e está populada (execute `create_character_info_tables.sql` se necessário)
- ✅ Unreal Engine 5 está instalado e configurado
- ✅ Projeto `UmbraEternumUE` está aberto no Unreal Editor

---

## 📊 **PASSO 1: ATUALIZAR BANCO DE DADOS**

### **1.1. Executar Script SQL**

1. Abra o **MySQL Workbench** ou **phpMyAdmin**
2. Conecte-se ao banco de dados `umbra_eternum`
3. Abra o arquivo: `www/umbra_api/scripts/add_hair_head_columns.sql`
4. Execute o script completo

**O que o script faz:**
```sql
ALTER TABLE players
ADD COLUMN IF NOT EXISTS hair INT UNSIGNED DEFAULT 0 AFTER vitality,
ADD COLUMN IF NOT EXISTS head INT UNSIGNED DEFAULT 0 AFTER hair;
```

### **1.2. Verificar Execução**

Execute a seguinte query para verificar se os campos foram adicionados:

```sql
DESCRIBE players;
```

Você deve ver os campos `hair` e `head` na lista de colunas.

### **1.3. Verificar Tabela Classes**

Certifique-se de que a tabela `classes` existe e está populada:

```sql
SELECT COUNT(*) as total_classes FROM classes;
```

Se retornar `0`, execute o script `www/umbra_api/scripts/create_character_info_tables.sql`.

---

## 🔨 **PASSO 2: RECOMPILAR PROJETO C++**

### **2.1. Fechar Unreal Editor**

⚠️ **IMPORTANTE:** Feche completamente o Unreal Editor antes de recompilar.

### **2.2. Limpar Arquivos Intermediários (Opcional, mas Recomendado)**

No Windows Explorer, navegue até:
```
D:\UmbraServerV2\UmbraEternumUE
```

Delete as pastas:
- `Intermediate/`
- `Binaries/` (opcional, mas recomendado para uma compilação limpa)

### **2.3. Regenerar Arquivos do Projeto**

1. Clique com o botão direito no arquivo `UmbraEternumUE.uproject`
2. Selecione **"Generate Visual Studio project files"** (ou equivalente)
3. Aguarde a conclusão

### **2.4. Compilar o Projeto**

**Opção A: Via Visual Studio**
1. Abra `UmbraEternumUE.sln` no Visual Studio
2. Selecione a configuração: **Development Editor** ou **Development**
3. Clique em **Build > Build Solution** (ou pressione `Ctrl+Shift+B`)
4. Aguarde a compilação concluir

**Opção B: Via Unreal Editor**
1. Abra o projeto no Unreal Editor
2. Vá em **Tools > Compile** (ou pressione `Ctrl+Alt+F11`)
3. Aguarde a compilação concluir

### **2.5. Verificar Compilação**

Certifique-se de que não há erros de compilação. Se houver erros, verifique:
- ✅ Todos os arquivos foram salvos
- ✅ Includes estão corretos
- ✅ Sintaxe está correta

---

## 🎨 **PASSO 3: IMPLEMENTAR NO BLUEPRINT**

### **3.1. Abrir Widget de Criação de Personagem**

1. No **Content Browser**, navegue até: `Content/Widgets/UI/Character/`
2. Abra o widget `WBP_CreateCharacter` (ou crie um novo se não existir)

### **3.2. Verificar/Criar Estrutura do Widget no Designer**

⚠️ **IMPORTANTE:** Antes de continuar, verifique a estrutura do widget no **Designer**.

**Estrutura Recomendada:**

```
Canvas Panel (Root)
└── [Seu Container Principal - Border, Vertical Box, etc.]
    └── Scroll Box: "ScrollBox_Classes" ⭐ ESSENCIAL
        └── Vertical Box: "VBox_ClassList" ⭐ DEVE ESTAR DENTRO DO SCROLL BOX
```

**Se você NÃO tem um `ScrollBox_Classes`:**

1. **No Designer do `WBP_CreateCharacter`:**
   - Selecione o `VBox_ClassList`
   - **Corte** (Ctrl+X)
   - Adicione um **Scroll Box** onde o `VBox_ClassList` estava
   - Nomeie como: `ScrollBox_Classes`
   - **Marque como variável:** ✅ Is Variable = TRUE
   - **Cole** (Ctrl+V) o `VBox_ClassList` **DENTRO** do `ScrollBox_Classes`

2. **Configure o ScrollBox_Classes:**
   - **Size:** Fill (Width e Height)
   - **Always Show Scrollbar:** FALSE
   - **Scrollbar Visibility:** As Needed

3. **Configure o VBox_ClassList:**
   - **Size:** Fill (Width e Height)
   - **Slot → Fill:** 1.0

**Por que usar ScrollBox?**
- Permite scroll quando há muitos itens
- Garante que todos os itens fiquem dentro da área visível
- Evita que itens apareçam "fora" do widget

### **3.3. Adicionar Variáveis ao Widget**

No **My Blueprint** panel, adicione as seguintes variáveis:

#### **Variáveis de Referência:**
- **`MyGameInstance`** (tipo: `Umbra Game Instance`, Category: "References")
  - ✅ Marcar como **Instance Editable** (se necessário)

#### **Variáveis de Dados:**
- **`SelectedClassID`** (tipo: `Integer`, Category: "Character Creation", Default: `0`)
- **`SelectedHair`** (tipo: `Integer`, Category: "Character Creation", Default: `0`)
- **`SelectedHead`** (tipo: `Integer`, Category: "Character Creation", Default: `0`)
- **`CharacterName`** (tipo: `String`, Category: "Character Creation", Default: `""`)

#### **Variáveis de UI (se necessário):**
- **`ClassesArray`** (tipo: `Array of Umbra Class Data`, Category: "Data", Default: `[]`)

### **3.4. Implementar Event Construct**

No **Event Graph**, encontre ou crie o **Event Construct**:

```
[Event Construct]
    ↓
[Get Game Instance]
    ↓
[Cast to Umbra Game Instance]
    ↓ (Success)
[SET] MyGameInstance = (cast result)
    ↓
[Load Classes]
    • Target: MyGameInstance
    ↓
[Set Input Mode UI Only]
    • Widget to Focus: (seu campo de nome)
    ↓
[Set Show Mouse Cursor] TRUE
```

### **3.5. Conectar Delegate OnClassesLoaded**

1. No **My Blueprint** panel, encontre **Event Dispatchers**
2. Adicione um **Custom Event** chamado `OnClassesLoadedEvent` (ou use o delegate diretamente)
3. No **Event Graph**, conecte:

```
[OnClassesLoaded] (delegate do MyGameInstance)
    ↓
[GET] MyGameInstance
    ↓
[Get Available Classes]
    • Target: MyGameInstance
    ↓
[SET] ClassesArray = (resultado)
    ↓
[Call Function: PopulateClassSelector]
    • Target: (Self)
```

### **3.6. Criar Função PopulateClassSelector**

Crie uma função **Custom Function** chamada `PopulateClassSelector`:

**Inputs:**
- Nenhum (usa `ClassesArray` da instância)

**Lógica Completa e Corrigida:**
```
[PopulateClassSelector]
    ↓
[Get] VBox_ClassList
    ↓
[Clear Children]
    • Target: VBox_ClassList ← ⭐ ADICIONE ESTE NÓ (limpa widgets antigos)
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ Loop Body
    • Array Element: (saída automática do For Each Loop - tipo: Umbra Class Data)
    • Array Index: (índice atual, opcional)
    ↓
[Break Umbra Class Data]
    • Input: Array Element (conecte o pin "Array Element" do For Each Loop)
    ↓
[Format Text]
    • Format: "Classe: {0}"
    • {0}: (Break result) ClassName
    ↓
[Print String]
    • In String: (resultado do Format Text)
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Create Widget]
    • Class: WBP_ClassSelectorItem
    • Owning Player: [Get Player Controller result]
    ↓
[Call Function: SetClassData]
    • Target: [Widget criado]
    • New Class Data: Array Element (conecte diretamente o pin "Array Element" do For Each Loop)
    ↓
[Get] VBox_ClassList ← ⭐ ESSENCIAL - ADICIONE ESTE NÓ
    ↓
[Add Child to Vertical Box]
    • Target: [VBox_ClassList] ← ⭐ CONECTE AQUI (não deixe desconectado!)
    • Content: [Widget criado]
    ↓ (Loop continua para próximo item)
[Completed]
    ↓
[Print String] "Classes populadas com sucesso!"
```

**⚠️ PONTOS CRÍTICOS PARA CORRIGIR:**

1. **`Clear Children` ANTES do loop:**
   - Limpa widgets antigos antes de adicionar novos
   - Evita duplicação de itens
   - **Como adicionar:** Clique direito → `Clear Children` → Conecte `Get VBox_ClassList` → `Clear Children` (Target)

2. **`Get VBox_ClassList` ANTES de `AddChildToVerticalBox`:**
   - **OBRIGATÓRIO** - O Target deve estar conectado
   - Sem isso, você verá o erro: "Este Blueprint (próprio) não é VerticalBox"
   - **Como adicionar:** Arraste `VBox_ClassList` do painel de variáveis para o gráfico

3. **Target do `AddChildToVerticalBox`:**
   - **DEVE** ser o `VBox_ClassList` (conectado via `Get VBox_ClassList`)
   - **NÃO** pode estar desconectado
   - **NÃO** pode ser `Self` (o widget principal)

**📌 IMPORTANTE: Como obter o Element no Blueprint:**

1. **Adicione o nó `For Each Loop`** no Event Graph
2. **Conecte o Array:** `ClassesArray` → pin `Array` do `For Each Loop`
3. **O `For Each Loop` automaticamente cria um pin de saída chamado `Array Element`**
   - Este pin contém o item atual do array (tipo: `Umbra Class Data`)
   - Este é o `Element` que você precisa usar
4. **Para acessar propriedades do Element:**
   - **Opção A:** Use `Break Umbra Class Data` e conecte `Array Element` → `Break Umbra Class Data`
   - **Opção B:** Conecte `Array Element` diretamente onde precisar (ex: passar para função que aceita `Umbra Class Data`)

**Exemplo Visual no Blueprint:**
```
[For Each Loop]
    Array: [ClassesArray] ──┐
                            │
    Array Element: [Umbra Class Data] ──┐
                                        │
    Array Index: [Integer]              │
                                        │
    Loop Body ──────────────────────────┘
                                        │
    [Break Umbra Class Data]            │
        Input: [Array Element] ─────────┘
        Outputs: ClassID, ClassName, ClassDescription, etc.
```

### **3.5.1. Passo a Passo Detalhado: For Each Loop no Blueprint**

**Como adicionar e configurar o For Each Loop:**

1. **No Event Graph, clique com botão direito e procure por:**
   - Digite: `For Each Loop`
   - Selecione: **"For Each Loop"** (não "For Each Loop with Break")

2. **Configure o Array:**
   - Conecte `ClassesArray` (variável do widget) → pin `Array` do `For Each Loop`
   - O tipo do array será detectado automaticamente como `Array of Umbra Class Data`

3. **O `For Each Loop` terá os seguintes pins:**
   - **`Array`** (Input): Conecte `ClassesArray`
   - **`Loop Body`** (Output - Exec): Execução para cada item
   - **`Array Element`** (Output - Data): O item atual (tipo: `Umbra Class Data`) ⭐ **ESTE É O ELEMENT**
   - **`Array Index`** (Output - Data): Índice atual (0, 1, 2, etc.)
   - **`Completed`** (Output - Exec): Executado quando o loop termina

4. **Para usar o Element:**
   - **Conecte o pin `Array Element`** onde você precisar
   - Exemplo: `Array Element` → `Break Umbra Class Data` → `ClassName`

5. **Exemplo completo de conexão:**
   ```
   [For Each Loop]
       Array: [ClassesArray] ──────────────────────────────┐
                                                           │
       Loop Body ─────────────────────────────────────────┼──┐
                                                           │  │
       Array Element: [Umbra Class Data] ─────────────────┘  │
                                                              │
       [Break Umbra Class Data]                               │
           Input: [Array Element] ────────────────────────────┘
           Outputs:
               - ClassID
               - ClassName ───→ [Format Text] ───→ [Print String]
               - ClassDescription
               - BaseStrength
               - etc.
   ```

### **3.6. Criar Widget de Item de Classe (Opcional)**

⚠️ **IMPORTANTE:** Se você criar um widget separado para cada item de classe (`WBP_ClassSelectorItem`), você precisa criar a função `SetClassData` manualmente. Ela não existe por padrão!

#### **3.6.1. Criar o Widget WBP_ClassSelectorItem (se ainda não existe)**

1. No **Content Browser**, clique com botão direito
2. Selecione **User Interface > Widget Blueprint**
3. Nomeie como `WBP_ClassSelectorItem`
4. Abra o widget

#### **3.6.2. Adicionar Variável ClassData**

1. No **My Blueprint** panel, clique no botão **+ Variable**
2. Nomeie como: `ClassData`
3. Configure:
   - **Variable Type:** `Umbra Class Data` (procure na lista)
   - **Category:** "Data"
   - **Instance Editable:** ✅ (opcional, mas útil para debug)
   - **Expose on Spawn:** ❌

#### **3.6.3. Criar a Função SetClassData (PASSO A PASSO)**

1. **No My Blueprint panel, clique em "Functions"**
2. **Clique no botão "+ Function"**
3. **Nomeie como:** `SetClassData`
4. **Configure a função:**
   - Clique na função `SetClassData` que você acabou de criar
   - No painel de detalhes, você verá **"Inputs"**
   - Clique em **"+ Input"**
   - Nomeie como: `NewClassData`
   - Tipo: `Umbra Class Data`
   - ✅ Marque como **"Required"**

5. **Agora vá para o Event Graph da função:**
   - A função `SetClassData` terá um nó de entrada automático
   - Você verá um pin de entrada chamado `NewClassData` (tipo: `Umbra Class Data`)

6. **Implemente a lógica:**
   ```
   [SetClassData] (nó de entrada da função)
       Input: NewClassData (Umbra Class Data)
       ↓
   [SET] ClassData = NewClassData
       • Target: (Self)
       • ClassData: NewClassData (conecte o pin de entrada)
       ↓
   [Break Umbra Class Data]
       • Input: NewClassData (ou ClassData após o SET)
       ↓
   [Set Text] TXT_ClassName
       • Target: (Self)
       • Text: (Break result) ClassName
       ↓
   [Set Text] TXT_ClassDescription
       • Target: (Self)
       • Text: (Break result) ClassDescription
       ↓
   [Format Text]
       • Format: "Str: {0} | Dex: {1} | Int: {2} | Vit: {3} | Luck: {4}"
       • {0}: (Break result) BaseStrength
       • {1}: (Break result) BaseDexterity
       • {2}: (Break result) BaseIntelligence
       • {3}: (Break result) BaseVitality
       • {4}: (Break result) BaseLuck
       ↓
   [Set Text] TXT_BaseStats
       • Target: (Self)
       • Text: (resultado do Format Text)
       ↓
   [Format Text]
       • Format: "HP: {0} | MP: {1} | Stamina: {2}"
       • {0}: (Break result) BaseHealth
       • {1}: (Break result) BaseMana
       • {2}: (Break result) BaseStamina
       ↓
   [Set Text] TXT_BaseResources
       • Target: (Self)
       • Text: (resultado do Format Text)
   ```

**📌 Nota:** Se você preferir fazer tudo no widget principal (`WBP_CreateCharacter`) sem criar um widget separado, você pode pular esta seção e implementar diretamente no `PopulateClassSelector` usando `Break Umbra Class Data` e criando os elementos de UI diretamente.

**OnClicked do Botão de Seleção:**
```
[OnClicked] BTN_SelectClass
    ↓
[GET] ClassData
    ↓
[Get Parent Widget]
    ↓
[Cast to WBP_CreateCharacter]
    ↓ (Success)
[Call Function: SelectClass]
    • Target: (cast result)
    • Class ID: ClassData.ClassID
```

### **3.6.4. Alternativa Simples: Implementar Diretamente no Widget Principal**

Se você **NÃO** quiser criar um widget separado (`WBP_ClassSelectorItem`), você pode implementar tudo diretamente no `WBP_CreateCharacter`:

**Modificar a função `PopulateClassSelector` para implementar diretamente:**

```
[PopulateClassSelector]
    ↓
[Clear Children]
    • Target: VBox_ClassList (ou seu container)
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ Loop Body
    • Array Element: (Umbra Class Data)
    ↓
[Create Widget]
    • Class: WBP_ClassButton (ou Button simples)
    • Owning Player: Get Player Controller (0)
    ↓
[Break Umbra Class Data]
    • Input: Array Element
    ↓
[Set Text] TXT_ButtonText (no widget criado)
    • Target: (widget criado)
    • Text: (Break result) ClassName
    ↓
[Add Child to Vertical Box]
    • Target: VBox_ClassList
    • Content: (widget criado)
    ↓
[Bind Event to OnClicked] (do botão criado)
    • Target: (widget criado)
    • Event: (criar evento customizado)
    ↓
[OnButtonClicked] (evento customizado)
    ↓
[Call Function: SelectClass]
    • Target: (Self)
    • Class ID: (Break result) ClassID (você precisa salvar isso em uma variável local)
```

**Ou ainda mais simples - usar um List View ou Scroll Box:**

1. **Adicione um `ListView` ou `ScrollBox` ao seu widget**
2. **No `PopulateClassSelector`, use:**

```
[PopulateClassSelector]
    ↓
[Clear Children]
    • Target: ScrollBox_Classes (ou ListView_Classes)
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ Loop Body
    • Array Element: (Umbra Class Data)
    ↓
[Create Widget]
    • Class: WBP_ClassButton (Button simples com Text Block)
    • Owning Player: Get Player Controller (0)
    ↓
[Break Umbra Class Data]
    • Input: Array Element
    ↓
[Set Text] TXT_ClassName
    • Target: (widget criado)
    • Text: (Break result) ClassName
    ↓
[Add Child to Scroll Box]
    • Target: ScrollBox_Classes
    • Content: (widget criado)
    ↓
[Bind Event] (OnClicked do botão)
    • Target: (widget criado)
    ↓
[OnClicked Handler]
    ↓
[Call Function: SelectClass]
    • Target: (Self)
    • Class ID: (você precisa passar o ClassID - veja nota abaixo)
```

**📌 IMPORTANTE - Como passar o ClassID no evento:**

Quando você criar o widget no loop, você precisa salvar o `ClassID` de alguma forma. Opções:

**Opção 1: Usar variável do widget criado**
- No `WBP_ClassButton`, adicione uma variável `ClassID` (Integer)
- Quando criar o widget, use `Set ClassID` para definir o valor
- No `OnClicked`, use `Get ClassID` para obter o valor

**Opção 2: Usar Tag do widget**
- Use `Set Tag` no widget criado com o ClassID como string
- No `OnClicked`, use `Get Tag` para obter o valor

**Opção 3: Usar um Map**
- Crie um `Map` no `WBP_CreateCharacter`: `ClassWidgetMap` (tipo: `Map<Widget, Integer>`)
- Quando criar o widget, adicione ao map: `ClassWidgetMap.Add(WidgetCriado, ClassID)`
- No `OnClicked`, use `Find` no map para obter o ClassID

### **3.6.5. ⚠️ CORREÇÃO: Classes Aparecendo Fora do Widget**

**PROBLEMA:** A primeira classe aparece, mas a segunda e seguintes aparecem fora do widget.

**CAUSAS POSSÍVEIS:**
1. ❌ `AddChildToVerticalBox` sem Target conectado corretamente
2. ❌ `VBox_ClassList` não está dentro de um `ScrollBox`
3. ❌ Tamanho do container não está configurado corretamente
4. ❌ Widgets filhos não têm tamanho adequado

**SOLUÇÃO COMPLETA:**

#### **Passo 1: Verificar Estrutura do Widget no Designer**

No **Designer** do `WBP_CreateCharacter`, verifique a hierarquia:

```
Canvas Panel (Root)
└── [Seu Container Principal]
    └── Scroll Box: "ScrollBox_Classes" ⭐ NOVO - ADICIONE SE NÃO EXISTIR
        └── Vertical Box: "VBox_ClassList" ⭐ DEVE ESTAR DENTRO DO SCROLL BOX
```

**Se o `VBox_ClassList` NÃO está dentro de um `ScrollBox`:**

1. **No Designer:**
   - Selecione o `VBox_ClassList`
   - **Corte** (Ctrl+X) o `VBox_ClassList`
   - Adicione um **Scroll Box** onde o `VBox_ClassList` estava
   - Nomeie como: `ScrollBox_Classes`
   - **Cole** (Ctrl+V) o `VBox_ClassList` **DENTRO** do `ScrollBox_Classes`

2. **Configure o ScrollBox_Classes:**
   - **Is Variable:** ✅ TRUE
   - **Size:** Fill (Width e Height)
   - **Always Show Scrollbar:** FALSE (ou TRUE se preferir)
   - **Scrollbar Visibility:** As Needed

3. **Configure o VBox_ClassList:**
   - **Is Variable:** ✅ TRUE
   - **Size:** Fill (Width e Height)
   - **Slot → Fill:** 1.0 (se estiver dentro do ScrollBox)

#### **Passo 2: Corrigir o AddChildToVerticalBox no Blueprint**

**No Event Graph da função `PopulateClassSelector`:**

**ERRO COMUM:**
```
[Add Child to Vertical Box]
    Target: [DESCONECTADO ou ERRADO] ❌
    Content: (widget criado)
```

**CORREÇÃO:**
```
[Get] VBox_ClassList
    ↓
[Add Child to Vertical Box]
    Target: [VBox_ClassList] ← DEVE ESTAR CONECTADO AQUI
    Content: (widget criado)
```

**Passo a Passo no Blueprint:**

1. **Localize o nó `AddChildToVerticalBox`** na função `PopulateClassSelector`
2. **Verifique o pin `Target` (self):**
   - Se estiver **desconectado** ou com **erro vermelho**, você precisa conectar
   - O pin `Target` deve receber o `VBox_ClassList`

3. **Como conectar corretamente:**
   - **Adicione um nó `Get VBox_ClassList`** (ou use `Variable Get` → `VBox_ClassList`)
   - **Conecte** a saída do `Get VBox_ClassList` → pin `Target` (self) do `AddChildToVerticalBox`

**Exemplo Visual Correto:**
```
[Create Widget]
    ReturnValue: [Widget criado]
    ↓
[Call Function: SetClassData]
    Target: [Widget criado]
    ↓
[Get] VBox_ClassList ← ADICIONE ESTE NÓ
    ↓
[Add Child to Vertical Box]
    Target: [VBox_ClassList] ← CONECTE AQUI
    Content: [Widget criado]
```

#### **Passo 3: Garantir que o VBox_ClassList está Limpo Antes de Popular**

**No início da função `PopulateClassSelector`, adicione:**

```
[PopulateClassSelector]
    ↓
[Clear Children]
    Target: VBox_ClassList ← ADICIONE ESTE NÓ
    ↓
[For Each Loop]
    Array: ClassesArray
    ↓
    ... (resto do código)
```

**Como adicionar `Clear Children`:**

1. **No Event Graph da função `PopulateClassSelector`**
2. **Clique com botão direito** → procure por: `Clear Children`
3. **Selecione:** "Clear Children" (não "Remove All Children")
4. **Conecte:**
   - `PopulateClassSelector` (exec) → `Clear Children` (exec)
   - `Get VBox_ClassList` → `Clear Children` (Target)
   - `Clear Children` (exec) → `For Each Loop` (exec)

#### **Passo 4: Verificar Tamanho dos Widgets Filhos**

**No widget `WBP_ClassSelectorItem`:**

1. **No Designer, selecione o Root (Canvas Panel ou Border)**
2. **Verifique o Size:**
   - **Size To Content:** ✅ TRUE (recomendado)
   - **OU** **Size:** Fixed Height (ex: 200)

3. **Se usar Size To Content:**
   - Certifique-se de que todos os elementos internos têm tamanho definido
   - O widget vai crescer automaticamente conforme o conteúdo

4. **Se usar Fixed Height:**
   - Defina uma altura adequada (ex: 200 pixels)
   - Isso garante que todos os widgets tenham o mesmo tamanho

#### **Passo 5: Verificar Layout do Container Principal**

**Se o `VBox_ClassList` está dentro de um `ScrollBox`:**

1. **No Designer, selecione o `ScrollBox_Classes`**
2. **Verifique:**
   - **Size:** Fill (Width e Height)
   - **Anchors:** Preenche o espaço disponível
   - **Slot → Fill:** 1.0 (se estiver dentro de outro container)

3. **Se o `ScrollBox_Classes` não preenche o espaço:**
   - Ajuste os **Anchors** para preencher
   - Ajuste o **Size** para Fill

#### **Passo 6: Teste Completo**

Após fazer todas as correções:

1. **Compile o Blueprint** (F7)
2. **Teste no jogo:**
   - Abra o widget de criação de personagem
   - Verifique se todas as classes aparecem
   - Verifique se é possível fazer scroll (se necessário)
   - Verifique se todas as classes estão dentro do widget

**Se ainda houver problemas:**

- Verifique os **logs do Unreal Editor** para erros
- Verifique se `ClassesArray` tem mais de um elemento
- Adicione um `Print String` no loop para verificar quantas iterações ocorrem

### **3.7. Criar Função SelectClass no WBP_CreateCharacter**

```
[SelectClass]
    • Input: Class ID (Integer)
    ↓
[SET] SelectedClassID = (input)
    ↓
[For Each Loop]
    • Array: ClassesArray
    ↓ Loop Body
[Branch] Element.ClassID == SelectedClassID?
    • Condition: (comparison result)
    ↓ TRUE
[Call Function: HighlightClass]
    • Target: (Self)
    • Class Data: (loop element)
    ↓
[Print String] "Classe selecionada: {Element.ClassName}"
```

### **3.8. Criar Seletores de Hair e Head**

Adicione **Combo Box** ou **Spin Box** para Hair e Head:

**Para Hair:**
```
[Spin Box Value Changed] SPIN_Hair
    ↓
[SET] SelectedHair = (new value)
    ↓
[Clamp] SelectedHair
    • Min: 0
    • Max: (valor máximo, ex: 10)
    ↓
[SET] SelectedHair = (clamped value)
```

**Para Head:**
```
[Spin Box Value Changed] SPIN_Head
    ↓
[SET] SelectedHead = (new value)
    ↓
[Clamp] SelectedHead
    • Min: 0
    • Max: (valor máximo, ex: 10)
    ↓
[SET] SelectedHead = (clamped value)
```

### **3.9. Implementar Validação Completa**

Crie uma função **Custom Function** chamada `ValidateCharacterCreation`:

**Outputs:**
- **`IsValid`** (tipo: `Boolean`)
- **`ErrorMessage`** (tipo: `String`)

**Lógica:**
```
[ValidateCharacterCreation]
    ↓
[Branch] CharacterName Is Empty?
    • String: CharacterName
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Nome é obrigatório"
    ↓ FALSE
[Branch] CharacterName Length < 3?
    • String: CharacterName
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Nome deve ter no mínimo 3 caracteres"
    ↓ FALSE
[Branch] CharacterName Length > 20?
    • String: CharacterName
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Nome deve ter no máximo 20 caracteres"
    ↓ FALSE
[Branch] SelectedClassID <= 0?
    • Condition: SelectedClassID <= 0
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Selecione uma classe"
    ↓ FALSE
[Branch] SelectedHair < 0?
    • Condition: SelectedHair < 0
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Hair inválido"
    ↓ FALSE
[Branch] SelectedHead < 0?
    • Condition: SelectedHead < 0
    ↓ TRUE
[Return] IsValid = FALSE, ErrorMessage = "Head inválido"
    ↓ FALSE
[Return] IsValid = TRUE, ErrorMessage = ""
```

### **3.10. Implementar Botão de Criar Personagem**

No **OnClicked** do botão **BTN_Create**:

```
[OnClicked] BTN_Create
    ↓
[Call Function: ValidateCharacterCreation]
    • Target: (Self)
    ↓
[Branch] IsValid?
    • Condition: (resultado da validação)
    ↓ FALSE
[Set Text] TXT_Validation
    • Text: ErrorMessage
    • Color: RED
    ↓
[Set Enabled] BTN_Create
    • Enabled: TRUE
    ↓ (End)
    ↓ TRUE
[Set Enabled] BTN_Create
    • Enabled: FALSE
    ↓
[Set Text] TXT_Validation
    • Text: "Criando personagem..."
    • Color: CYAN
    ↓
[GET] MyGameInstance
    ↓
[Create Character]
    • Target: MyGameInstance
    • Character Name: CharacterName
    • Class ID: SelectedClassID
    • Hair: SelectedHair
    • Head: SelectedHead
    ↓
[Delay] 2.0 segundos
    ↓
[Set Enabled] BTN_Create
    • Enabled: TRUE
```

### **3.11. Conectar Delegate OnCharacterCreated**

```
[OnCharacterCreated] (delegate do MyGameInstance)
    ↓
[Print String] "Personagem criado com sucesso!"
    ↓
[Set Text] TXT_Validation
    • Text: "Personagem criado com sucesso!"
    • Color: GREEN
    ↓
[Remove from Parent]
    • Target: (Self)
    ↓
[Call Function: RefreshCharacterList]
    • Target: (widget pai, se houver)
```

### **3.12. Conectar Delegate OnCharacterCreateFailed**

```
[OnCharacterCreateFailed] (delegate do MyGameInstance)
    ↓
[Set Text] TXT_Validation
    • Text: (ErrorMessage do delegate)
    • Color: RED
    ↓
[Set Enabled] BTN_Create
    • Enabled: TRUE
```

---

## 🧪 **PASSO 4: TESTAR IMPLEMENTAÇÃO**

### **4.1. Testar API get_classes.php**

1. Abra o navegador
2. Acesse: `http://localhost/umbra_api/api/character/get_classes.php`
3. Você deve ver um JSON com todas as classes disponíveis

**Resposta esperada:**
```json
{
  "success": true,
  "message": "Classes carregadas com sucesso",
  "classes": [
    {
      "class_id": 1,
      "class_name": "Barbarian",
      "class_description": "...",
      "base_stats": { ... },
      "base_resources": { ... },
      "base_combat": { ... }
    },
    ...
  ],
  "total": 6
}
```

### **4.2. Testar API create_character.php**

Use um cliente HTTP (Postman, Insomnia, ou curl) para testar:

**Request:**
```json
POST http://localhost/umbra_api/api/character/create_character.php
Content-Type: application/json

{
  "token": "SEU_JWT_TOKEN_AQUI",
  "character_name": "TestePersonagem",
  "class_id": 1,
  "hair": 0,
  "head": 0
}
```

**Resposta esperada (sucesso):**
```json
{
  "success": true,
  "message": "Personagem criado com sucesso!",
  "player": {
    "player_id": 123,
    "character_name": "TestePersonagem",
    "class_id": 1,
    "hair": 0,
    "head": 0,
    "stats": { ... }
  }
}
```

### **4.3. Testar no Unreal Editor**

1. **Compile o Blueprint:**
   - Pressione `F7` ou clique em **Compile** no editor de Blueprint

2. **Teste o Widget:**
   - Crie um widget de teste ou adicione ao menu principal
   - Abra o widget de criação de personagem
   - Verifique se as classes são carregadas
   - Selecione uma classe
   - Defina Hair e Head
   - Digite um nome
   - Clique em Criar

3. **Verificar Logs:**
   - Abra a janela **Output Log** no Unreal Editor
   - Procure por mensagens como:
     - `[UmbraGameInstance] Carregando classes disponíveis`
     - `[UmbraGameInstance] ✅ Classes carregadas: X`
     - `[UmbraGameInstance] Criando personagem: ...`

### **4.4. Verificar Banco de Dados**

Execute a query para verificar se o personagem foi criado corretamente:

```sql
SELECT 
    id,
    character_name,
    class_id,
    hair,
    head,
    strength,
    dexterity,
    intelligence,
    vitality,
    luck,
    health,
    max_health,
    mana,
    max_mana
FROM players
WHERE character_name = 'TestePersonagem';
```

Verifique se:
- ✅ `class_id` está correto
- ✅ `hair` e `head` estão corretos
- ✅ Stats correspondem aos stats base da classe selecionada

---

## 🔧 **TROUBLESHOOTING**

### **Problema Específico: "Este Blueprint (próprio) não é VerticalBox, por isso Target deve ter uma conexão"**

**Erro no Blueprint:**
```
ErrorType=1
ErrorMsg="Este Blueprint (próprio) não é VerticalBox, por isso \" Target \" deve ter uma conexão."
```

**Causa:**
O nó `AddChildToVerticalBox` está tentando usar `Self` como Target, mas o widget atual (`WBP_CreateCharacter`) não é um `VerticalBox`. O Target deve ser o `VBox_ClassList`.

**Solução Rápida:**

1. **Localize o nó `AddChildToVerticalBox`** na função `PopulateClassSelector`
2. **Veja o pin `Target` (self):**
   - Se estiver com **erro vermelho** ou **desconectado**, você precisa conectar
3. **Adicione um nó `Get VBox_ClassList`:**
   - Clique com botão direito → `Get VBox_ClassList`
   - OU arraste `VBox_ClassList` do painel de variáveis para o gráfico
4. **Conecte:**
   - `Get VBox_ClassList` (output) → `AddChildToVerticalBox` (Target/self)

**Código Correto:**
```
[Create Widget]
    ReturnValue: [Widget criado]
    ↓
[Call Function: SetClassData]
    Target: [Widget criado]
    ↓
[Get] VBox_ClassList ← ADICIONE ESTE
    ↓
[Add Child to Vertical Box]
    Target: [VBox_ClassList] ← CONECTE AQUI (não deixe desconectado!)
    Content: [Widget criado]
```

**📌 Diagrama Visual da Correção:**

**ANTES (ERRADO):**
```
[For Each Loop]
    Loop Body
    ↓
[Create Widget]
    ReturnValue ──┐
                  │
[Call Function: SetClassData]
    Target: [ReturnValue]
    ↓
[Add Child to Vertical Box]
    Target: [ERRO - DESCONECTADO] ❌
    Content: [ReturnValue]
```

**DEPOIS (CORRETO):**
```
[For Each Loop]
    Loop Body
    ↓
[Create Widget]
    ReturnValue ──┐
                  │
[Call Function: SetClassData]    [Get VBox_ClassList] ← ADICIONE
    Target: [ReturnValue]              │
    ↓                                   │
[Add Child to Vertical Box]             │
    Target: [VBox_ClassList] ←──────────┘ ✅
    Content: [ReturnValue]
```

**Passo a Passo Visual no Blueprint:**

1. **Encontre o nó `AddChildToVerticalBox`** no seu gráfico
2. **Veja o pin `Target` (ou `self`):**
   - Se estiver **vermelho** ou **desconectado**, precisa corrigir
3. **No painel "My Blueprint" (esquerda), encontre a variável `VBox_ClassList`**
4. **Arraste `VBox_ClassList` para o gráfico** (isso cria um `Get VBox_ClassList`)
5. **Conecte:**
   - `Get VBox_ClassList` (output) → `AddChildToVerticalBox` (Target/self)
6. **O erro vermelho deve desaparecer!**

### **Problema: Segunda classe aparece fora do widget**

**Causas:**
1. `VBox_ClassList` não está dentro de um `ScrollBox`
2. Tamanho do container não está configurado
3. Widgets filhos não têm tamanho adequado

**Solução:**
Siga os passos da seção **3.6.5. ⚠️ CORREÇÃO: Classes Aparecendo Fora do Widget** acima.

## 🔧 **TROUBLESHOOTING (Continuação)**

### **Problema: Classes não carregam**

**Soluções:**
1. Verifique se a API `get_classes.php` está acessível
2. Verifique se a tabela `classes` existe e está populada
3. Verifique os logs do Unreal Editor para erros
4. Verifique se o delegate `OnClassesLoaded` está conectado corretamente

### **Problema: Erro ao criar personagem - "class_id é obrigatório"**

**Soluções:**
1. Verifique se `SelectedClassID` está sendo setado corretamente
2. Verifique se o valor está sendo passado para `CreateCharacter`
3. Verifique os logs do servidor PHP para mais detalhes

### **Problema: Personagem criado sem stats da classe**

**Soluções:**
1. Verifique se a classe existe no banco de dados
2. Verifique se a API está buscando os stats corretamente
3. Verifique os logs do servidor PHP

### **Problema: Campos hair e head não aparecem no banco**

**Soluções:**
1. Verifique se o script SQL foi executado corretamente
2. Execute manualmente: `ALTER TABLE players ADD COLUMN hair INT UNSIGNED DEFAULT 0;`
3. Execute manualmente: `ALTER TABLE players ADD COLUMN head INT UNSIGNED DEFAULT 0;`

### **Problema: Erro de compilação C++**

**Soluções:**
1. Verifique se todos os arquivos foram salvos
2. Limpe `Intermediate/` e `Binaries/`
3. Regenerar arquivos do projeto
4. Recompilar do zero

---

## ✅ **CHECKLIST FINAL**

Antes de considerar a implementação completa, verifique:

- [ ] Script SQL executado com sucesso
- [ ] Campos `hair` e `head` existem na tabela `players`
- [ ] Projeto C++ compilado sem erros
- [ ] API `get_classes.php` retorna dados corretos
- [ ] API `create_character.php` aceita novos parâmetros
- [ ] Blueprint compila sem erros
- [ ] Classes são carregadas no widget
- [ ] Seleção de classe funciona
- [ ] Seletores de Hair e Head funcionam
- [ ] Validação funciona corretamente
- [ ] Personagem é criado com stats da classe
- [ ] Personagem é criado com hair e head corretos
- [ ] Dados aparecem corretamente no banco de dados

---

## 📝 **NOTAS ADICIONAIS**

### **Valores Válidos para Hair e Head**

Por enquanto, os valores são numéricos simples (0, 1, 2, etc.). No futuro, você pode:
- Criar uma tabela `hair_options` e `head_options` no banco
- Carregar essas opções via API
- Mostrar previews visuais no Blueprint

### **Expansões Futuras**

- Adicionar preview visual da classe selecionada
- Adicionar preview visual do personagem com hair/head selecionados
- Adicionar validação de nome em tempo real
- Adicionar animações de transição
- Adicionar som ao selecionar classe

---

## 🎉 **CONCLUSÃO**

Após seguir todos os passos deste guia, você terá um sistema completo de criação de personagem com:
- ✅ Seleção de classe com informações detalhadas
- ✅ Customização visual (hair e head)
- ✅ Validação completa
- ✅ Criação de personagem com stats baseados na classe

**Boa sorte com a implementação!** 🚀

