# Procedimento Blueprint – WBP_SkillBook

Procedimento passo a passo para criar o Livro de Skills no Unreal Editor. Siga a ordem dos blocos. Os **nomes na Hierarchy devem ser exatos** para o Blueprint e para referência em código.

---

## 1. WBP_SkillBook – Estrutura no Designer

### 1.1 Root

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | Criar **User Widget** | Nome do Blueprint: **WBP_SkillBook**. Abra o Designer. |
| 2 | **Root** | Por padrão o root é **Canvas Panel**. Mantenha como root. Nome na Hierarchy: **CanvasPanel_Root** (ou deixe o nome padrão). Se quiser um fundo (Image), adicione como filho do Canvas; se esse fundo cobrir a lista, use **Visibility** = **Hit Test Invisible** nele para não bloquear cliques. |

---

### 1.2 Header_Panel (cabeçalho)

O cabeçalho é uma **Horizontal Box** dentro de um **Border** para ter fundo e padding.

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | Adicionar ao Canvas | Botão direito em **CanvasPanel_Root** → **Add Child** → **Border**. Nome: **Header_Panel**. |
| 2 | Configurar Header_Panel (Border) | **Anchors:** Top Center (0.5, 0). **Alignment:** (0.5, 0). **Position Y:** 20. **Size X:** 800–1000 (ou Fill). **Size Y:** 60. **Padding:** Left 20, Right 20, Top 10, Bottom 10. **Brush Color:** cor escura (ex.: #1A1A2E), Alpha 0,95. **Brush:** opcional – borda inferior para separar do conteúdo. |
| 3 | Conteúdo do Header | Com **Header_Panel** selecionado → **Add Child** → **Horizontal Box**. Nome: **Header_HBox**. (Tudo que for título, classe, pontos e botão ficará dentro deste HBox.) |
| 4 | Configurar Header_HBox | **Size:** Fill (expandir no Border). No **Horizontal Box** → **Slot (Horizontal Box Slot)** de cada filho: **Size** = Auto ou Fill conforme abaixo. |

**Filhos do Header_HBox (na ordem da esquerda para a direita):**

| Índice | Tipo de widget | Nome | Configuração do Slot |
|--------|----------------|------|----------------------|
| 0 | **Text Block** | **Title_Text** | Size = Auto. Texto: "Livro de Skills". Fonte tamanho 24, cor clara. |
| 1 | **Spacer** | **Header_Spacer1** | Add Child → **Spacer**. Width = 20 (espaço entre título e classe). |
| 2 | **Text Block** | **Class_Text** | Size = Auto. Texto: "" (será preenchido por Blueprint com nome da classe). Fonte 18. |
| 3 | **Spacer** | **Header_Spacer2** | Spacer, Width = 20. |
| 4 | **Text Block** | **SkillPoints_Text** | Size = Auto. Texto: "Pontos: 0". Será atualizado por Blueprint. |
| 5 | **Spacer** | **Header_Spacer_Fill** | Spacer, Size = Fill (empurra o botão para a direita). |
| 6 | **Button** | **Close_Button** | Size = Auto. Texto do botão: "X" ou "Fechar". Largura ~40–50. |

**Resumo Header:**  
`Header_Panel` (Border) → filho único `Header_HBox` (Horizontal Box) → filhos: Title_Text, Spacer, Class_Text, Spacer, SkillPoints_Text, Spacer (Fill), Close_Button.

---

### 1.3 Content_Panel (área central: lista + filtros)

É um **Horizontal Box** que divide a tela em “lista à esquerda” e “detalhes à direita”. A lista fica dentro de um **Scroll Box**.

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | Adicionar ao Canvas | **Add Child** → **Horizontal Box**. Nome: **Content_Panel**. |
| 2 | Anchors/Position Content_Panel | **Anchors:** Stretch horizontal, vertical entre header e rodapé. Ex.: Min (0.1, 0.1), Max (0.9, 0.9). **Position:** X 80, Y 90. **Size:** X 1100, Y 550 (ajuste ao layout). |
| 3 | Slot do Content_Panel | No **Canvas Slot**: Fill ou tamanho fixo conforme seu layout. |

**Filhos do Content_Panel (Horizontal Box) – dois filhos:**

- **Filho 0 – Coluna da lista (esquerda)**

| Passo | Ação | Detalhes |
|-------|------|----------|
| A | **Add Child** ao Content_Panel | **Vertical Box**. Nome: **List_Column_VBox**. |
| B | Slot (Horizontal Box Slot) | **Size X:** 380–450. **Padding:** Right 15. |
| C | Filho 0 de List_Column_VBox | **Horizontal Box** → Nome: **Filter_Panel**. Altura ~36. Contém: Filter_All_Button, Filter_Active_Button, Filter_Passive_Button, Filter_Learned_Button (todos **Button**). Textos: "Todos", "Ativas", "Passivas", "Aprendidas". |
| D | Filho 1 de List_Column_VBox | **Scroll Box** → Nome: **Skills_ScrollBox**. **Size:** Fill. Aqui serão adicionados em runtime os **WBP_SkillBookEntry**. |

- **Filho 1 – Coluna de detalhes (direita)**

| Passo | Ação | Detalhes |
|-------|------|----------|
| E | **Add Child** ao Content_Panel | **Border** (ou **Vertical Box**). Nome: **Detail_Panel**. |
| F | Slot (Horizontal Box Slot) | **Size X:** Fill (ou 400–500). **Padding:** Left 15. **Brush Color:** fundo levemente diferente (ex.: #252535), Alpha 0,9. **Padding** do Border: 16. |

---

### 1.4 Detail_Panel – Conteúdo (detalhes da skill selecionada)

Tudo abaixo fica **dentro de Detail_Panel**. Use um **Vertical Box** como único filho do **Detail_Panel** para empilhar os blocos.

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | Filho único de Detail_Panel | **Vertical Box**. Nome: **Detail_VBox**. |
| 2 | Configurar Detail_VBox | **Size:** Fill. **Padding:** 8. |

**Filhos de Detail_VBox (em ordem de cima para baixo):**

| Índice | Tipo | Nome | Configuração |
|--------|------|------|--------------|
| 0 | **Image** | **Skill_Icon_Large** | Size X/Y: 96 ou 128. Brush = ícone (setado por Blueprint). |
| 1 | **Spacer** | **Detail_Spacer1** | Height 8. |
| 2 | **Text Block** | **Skill_Name_Text** | Texto: "Nome da Skill". Fonte 22, negrito. |
| 3 | **Text Block** | **Skill_Type_Text** | Texto: "Ativa • Físico". Fonte 14, cor secundária. |
| 4 | **Spacer** | **Detail_Spacer2** | Height 12. |
| 5 | **Text Block** | **Skill_Description_Text** | Texto: "" (descrição longa). Auto Wrap Text = true. Tamanho 14. |
| 6 | **Spacer** | **Detail_Spacer3** | Height 12. |
| 7 | **Horizontal Box** | **Skill_Stats_Panel** | Contém 4 **Text Block** em linha: **Cost_Text** ("Custo: 10 Mana"), **Cooldown_Text** ("CD: 5s"), **Range_Text** ("Alcance: 100"), **Duration_Text** ("Duração: 10s"). Ou use **Wrap Box** se quiser quebra de linha. |
| 8 | **Spacer** | **Detail_Spacer4** | Height 8. |
| 9 | **Text Block** | **Scaling_Text** | Texto: "Escala: STR 60%, VIT 30%". Nome da variável no Blueprint: **Scaling_Text**. |
| 10 | **Horizontal Box** | **Rank_Panel** | **CurrentRank_Text** ("Rank 2/5") + **RankStars_HBox** (Horizontal Box com 5 **Image** de estrela, ativadas/desativadas por Blueprint). |
| 11 | **Spacer** | **Detail_Spacer5** | Height 20. |
| 12 | **Button** | **Action_Button** | Texto: "Aprender (1 pt)" / "Melhorar (1 pt)" / "Rank Máximo" / "Requer Nível X" (atualizado por Blueprint). Largura Fill, altura 40. |

**Nomes obrigatórios no WBP_SkillBook (Designer):**

| Nome | Tipo |
|------|------|
| CanvasPanel_Root | Canvas Panel |
| Header_Panel | Border |
| Header_HBox | Horizontal Box |
| Title_Text | Text Block |
| Class_Text | Text Block |
| SkillPoints_Text | Text Block |
| Close_Button | Button |
| Content_Panel | Horizontal Box |
| List_Column_VBox | Vertical Box |
| Filter_Panel | Horizontal Box |
| Filter_All_Button | Button |
| Filter_Active_Button | Button |
| Filter_Passive_Button | Button |
| Filter_Learned_Button | Button |
| Skills_ScrollBox | Scroll Box |
| Detail_Panel | Border |
| Detail_VBox | Vertical Box |
| Skill_Icon_Large | Image |
| Skill_Name_Text | Text Block |
| Skill_Type_Text | Text Block |
| Skill_Description_Text | Text Block |
| Skill_Stats_Panel | Horizontal Box (ou Wrap Box) |
| Cost_Text | Text Block |
| Cooldown_Text | Text Block |
| Range_Text | Text Block |
| Duration_Text | Text Block |
| Scaling_Text | Text Block |
| Rank_Panel | Horizontal Box |
| CurrentRank_Text | Text Block |
| RankStars_HBox | Horizontal Box |
| Action_Button | Button |

---

## 2. WBP_SkillBookEntry – Estrutura no Designer

Widget de **uma linha** da lista de skills. Cada entrada é um **Horizontal Box** com ícone, textos e indicadores.

### 2.1 Root

| Passo | Ação | Detalhes |
|-------|------|----------|
| 1 | Criar **User Widget** | Nome: **WBP_SkillBookEntry**. |
| 2 | Root | Troque o root para **Horizontal Box** (ou use **Canvas Panel** com um **Horizontal Box** filho). Recomendado: root = **Horizontal Box**, Nome: **Entry_HBox**. **Padding:** 4–8. **Size:** Height 72–80. |

### 2.2 Filhos do Entry_HBox (esquerda → direita)

| Índice | Tipo | Nome | Configuração |
|--------|------|------|--------------|
| 0 | **Image** | **Skill_Icon** | Size 64×64. Brush = ícone (Blueprint). |
| 1 | **Spacer** | **Entry_Spacer1** | Width 12. |
| 2 | **Vertical Box** | **Info_VBox** | Size = Fill. Contém: Skill_Name_Text, Skill_Type_Text, Level_Required_Text. |
| 3 | **Spacer** | **Entry_Spacer2** | Width 12. |
| 4 | **Horizontal Box** | **Rank_HBox** | 5× **Image** (estrela). Nomes: Star_1 a Star_5. Size 16×16 cada. |
| 5 | **Overlay** | **Status_Overlay** | Sobrepondo (opcional): **Locked_Icon** (Image, cadeado) e **Learned_Check** (Image, check). Visibility condicional no Blueprint. |
| 6 | **Button** (opcional) | **BTN_SkillSelect** | Se não usar **Button como root** (opção abaixo), adicione um Button como filho. Para receber cliques, ele deve estar por cima (último filho) e transparente, ou use a **opção Button como root**. |

**Estrutura alternativa (recomendada, igual a WBP_FriendSlot):** usar **Button como root** do Entry: root = **BTN_SkillSelect** (Button, alpha 0), e todo o conteúdo (ícone, textos, rank, overlay) como **filhos** do botão. Assim o botão cobre toda a entrada e recebe o clique sem depender da ordem dos irmãos. Se o clique ainda não funcionar, o problema está **fora** do Entry (ver **4.4.1**).

**Info_VBox – filhos:**

| Índice | Tipo | Nome | Configuração |
|--------|------|------|--------------|
| 0 | Text Block | **Skill_Name_Text** | Nome da skill. Fonte 16. |
| 1 | Text Block | **Skill_Type_Text** | "Ativa", "Passiva", etc. Fonte 12, cor cinza. |
| 2 | Text Block | **Level_Required_Text** | "Nível 10" ou "Requer Nível 10". Fonte 12. |

**Nomes obrigatórios no WBP_SkillBookEntry:**

| Nome | Tipo |
|------|------|
| Entry_HBox | Horizontal Box |
| Skill_Icon | Image |
| Info_VBox | Vertical Box |
| Skill_Name_Text | Text Block |
| Skill_Type_Text | Text Block |
| Level_Required_Text | Text Block |
| Rank_HBox | Horizontal Box |
| Star_1 … Star_5 | Image |
| Status_Overlay | Overlay (opcional) |
| Locked_Icon | Image |
| Learned_Check | Image |
| BTN_SkillSelect | Button |

---

## 3. Event Graph e funções – WBP_SkillBook

### 3.1 Variáveis (WBP_SkillBook)

| Nome | Tipo | Descrição |
|------|------|-----------|
| GameInstanceRef | Umbra Game Instance (Object Reference) | Cache do Game Instance. |
| SelectedSkillID | Integer | Skill atualmente selecionada (0 = nenhuma). |
| SelectedSkillData | FUmbraSkillData (struct) | Dados da skill selecionada (para detalhes). |

### 3.2 Event Construct (resumo)

1. Obter **Game Instance** e guardar em **GameInstanceRef**.
2. Fazer **Bind** de todos os delegates do Game Instance (skills) no Event Graph.
3. Chamar **Load Available Skills** e **UpdateHeader**.
4. Conectar **Close_Button** e **Action_Button** aos eventos de clique.

O detalhamento de cada conexão está na seção **3.2.1** abaixo.

---

### 3.2.1 Event Graph – Conexões dos delegates (passo a passo)

Todas as conexões abaixo são feitas no **Event Graph** do **WBP_SkillBook**. Ordem sugerida: primeiro **Event Construct** e cadeia de binds; depois, na **Designer**, associar **On Clicked** do **Close_Button** e do **Action_Button** aos eventos correspondentes no Event Graph.

---

#### Passo 0 – Abrir o Event Graph e iniciar no Event Construct

| Ação | Detalhes |
|------|----------|
| Abrir | WBP_SkillBook → aba **Event Graph**. |
| Nó inicial | Use o nó **Event Construct** (dispara quando o widget é criado/exibido). |

---

#### Passo 1 – Obter e guardar o Game Instance

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Obter Game Instance | **Get Game Instance** (contexto: **self**). Saída **Return Value** (Object Reference). |
| 2 | Cast para o tipo correto | **Cast to Umbra Game Instance**: **Object** = saída do Get Game Instance. **As Umbra Game Instance** = saída do cast. |
| 3 | Guardar referência | **Set GameInstanceRef**: **Game Instance Ref** = **As Umbra Game Instance**. Ligue a **exec** (branco) do **Event Construct** à **exec** do **Set**. |

**Resultado:** a partir daqui, use **Get GameInstanceRef** sempre que precisar do Game Instance. Todas as ligações de delegates usam **GameInstanceRef** como **Target**.

---

#### Passo 2 – Bind Event to On Available Skills Loaded

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Available Skills Loaded**. (Procure por "On Available Skills Loaded" no menu ao arrastar do pin **GameInstanceRef** ou pelo **Search** no canvas.) |
| 2 | Target | **Target** = **Get GameInstanceRef** (ou o mesmo objeto do Cast do Passo 1). |
| 3 | Event (callback) | O nó de Bind cria um **pin de evento** (Event). Ao ligar a **exec** desse Event, você define o que roda quando as skills disponíveis forem carregadas. |
| 4 | No callback | Ligue a **exec** do Event **On Available Skills Loaded** em sequência a: **PopulateSkillList** (sua função) → **UpdateHeader** (sua função). Opcional: **Set SelectedSkillID** = 0 e limpar **SelectedSkillData** se quiser “nenhuma skill selecionada” ao abrir. |

**Resumo do callback:** quando o servidor/API responder com a lista de skills, o Game Instance dispara esse delegate; o WBP_SkillBook então preenche a lista (**PopulateSkillList**) e atualiza pontos/classe no header (**UpdateHeader**).

---

#### Passo 3 – Bind Event to On Available Skills Load Failed

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Available Skills Load Failed**. **Target** = **Get GameInstanceRef**. |
| 2 | Parâmetro do Event | O Event recebe **ErrorMessage** (String). Use esse valor para exibir o erro. |
| 3 | No callback | **SkillPoints_Text** → **Set Text** = "Erro ao carregar" (ou **Append**("Erro: ", **ErrorMessage**)). Opcional: abrir um **Popup** ou **Toast** com **ErrorMessage**; ou **Print String** (apenas para debug). |

---

#### Passo 4 – Bind Event to On Skill Learned

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Skill Learned**. **Target** = **Get GameInstanceRef**. |
| 2 | Parâmetros do Event | O Event fornece **SkillID** (Integer) e **SkillName** (String). |
| 3 | No callback | Chamar **RefreshSkillList** (função que você cria: **Load Player Skills** no Game Instance e, em seguida, **PopulateSkillList** + **UpdateHeader**; ou apenas **PopulateSkillList** se os dados já forem atualizados no Game Instance). Opcional: **Print String** com **SkillName** + " aprendida!" ou mostrar notificação na UI. Se quiser manter a mesma skill selecionada, após refresh chame **SelectSkill(SelectedSkillID)** para atualizar o painel de detalhes. |

---

#### Passo 5 – Bind Event to On Skill Learn Failed

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Skill Learn Failed**. **Target** = **Get GameInstanceRef**. |
| 2 | Parâmetro do Event | **ErrorMessage** (String). |
| 3 | No callback | Mostrar **ErrorMessage** ao jogador: **Set Text** em um **Text Block** de erro, ou **Print String**, ou popup/toast. |

---

#### Passo 6 – Bind Event to On Skill Upgraded

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Skill Upgraded**. **Target** = **Get GameInstanceRef**. |
| 2 | Parâmetros do Event | **SkillID**, **NewRank** (Integer), **SkillName** (String). |
| 3 | No callback | Mesmo que **On Skill Learned**: **RefreshSkillList** (PopulateSkillList + UpdateHeader). Opcional: mensagem "**SkillName** melhorada para rank **NewRank**!". Se desejar, **SelectSkill(SelectedSkillID)** para atualizar o detalhe. |

---

#### Passo 7 – Bind Event to On Skill Upgrade Failed

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Nó de bind | **Bind Event to On Skill Upgrade Failed**. **Target** = **Get GameInstanceRef**. |
| 2 | Parâmetro do Event | **ErrorMessage** (String). |
| 3 | No callback | Exibir **ErrorMessage** (Text Block de erro, popup ou Print String). |

---

#### Passo 8 – Ordem da cadeia no Event Construct (ligar exec)

A **exec** (fio branco) deve fluir na ordem dos binds e terminar em **Load Available Skills** e **UpdateHeader**:

1. **Event Construct** → **Set GameInstanceRef** (Passo 1).
2. Em seguida (em série): **Bind Event to On Available Skills Loaded** → **Bind Event to On Available Skills Load Failed** → **Bind Event to On Skill Learned** → **Bind Event to On Skill Learn Failed** → **Bind Event to On Skill Upgraded** → **Bind Event to On Skill Upgrade Failed**.  
   Cada **Bind** não precisa de um “próximo” nó na exec: o Bind só registra o callback. Conecte a **exec** de **Set GameInstanceRef** ao primeiro **Bind**, e a **exec** de cada **Bind** ao próximo **Bind**, para garantir que todos sejam registrados na construção.
3. Por fim, após o último Bind, ligar a **exec** a: **Load Available Skills** (Target = **Get GameInstanceRef**) → **UpdateHeader**.  
   Assim, ao abrir o livro, as skills são solicitadas e o header é atualizado; quando a resposta chegar, **On Available Skills Loaded** dispara e executa **PopulateSkillList** e **UpdateHeader** no callback.

**Resumo da cadeia:**  
`Event Construct` → `Set GameInstanceRef` → `Bind On Available Skills Loaded` → `Bind On Available Skills Load Failed` → `Bind On Skill Learned` → `Bind On Skill Learn Failed` → `Bind On Skill Upgraded` → `Bind On Skill Upgrade Failed` → `Load Available Skills` → `UpdateHeader`.

---

#### Passo 9 – Close_Button (On Clicked)

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Onde conectar | No **Designer**, selecione **Close_Button** → **Details** → **Events** → **On Clicked** → **+** (ou arraste o pin **On Clicked** para o Event Graph). Isso cria no Event Graph um **Event On Clicked (Close_Button)**. |
| 2 | No callback | **Get Player Controller (0)** → **Cast to Umbra Eternum UE Player Controller** → **Close Skill Book**. Não use **Remove From Parent**: o C++ **CloseSkillBook()** remove o widget e restaura o input (cursor e modo de jogo). |

---

#### Passo 10 – Action_Button (On Clicked)

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Onde conectar | No **Designer**, selecione **Action_Button** → **Details** → **Events** → **On Clicked** → **+**. Isso cria **Event On Clicked (Action_Button)** no Event Graph. |
| 2 | No callback | Ligue a **exec** desse Event à sua função **OnActionButtonClicked** (sem parâmetros). Dentro de **OnActionButtonClicked** você usa **SelectedSkillID** e **SelectedSkillData** para chamar **Learn Skill** ou **Upgrade Skill** no Game Instance; a resposta será tratada pelos delegates **On Skill Learned** / **On Skill Learn Failed** e **On Skill Upgraded** / **On Skill Upgrade Failed** já ligados acima. |

---

#### Tabela resumo – Delegates e callbacks

| Delegate (Game Instance) | Parâmetros do Event | Ação no callback (WBP_SkillBook) |
|--------------------------|---------------------|-----------------------------------|
| **On Available Skills Loaded** | (nenhum) | **PopulateSkillList** → **UpdateHeader** |
| **On Available Skills Load Failed** | ErrorMessage (String) | Mostrar erro em **SkillPoints_Text** ou popup/toast |
| **On Skill Learned** | SkillID, SkillName | **RefreshSkillList**; opcional: mensagem de sucesso; opcional: **SelectSkill(SelectedSkillID)** |
| **On Skill Learn Failed** | ErrorMessage | Mostrar ErrorMessage ao jogador |
| **On Skill Upgraded** | SkillID, NewRank, SkillName | **RefreshSkillList**; opcional: mensagem; opcional: **SelectSkill(SelectedSkillID)** |
| **On Skill Upgrade Failed** | ErrorMessage | Mostrar ErrorMessage ao jogador |

---

#### Função RefreshSkillList (para usar nos callbacks)

Crie uma função **RefreshSkillList** (sem parâmetros) no WBP_SkillBook que:

1. Chama **Load Player Skills** no **GameInstanceRef** (para atualizar **Player Skills** no Game Instance; assim **Available Skills** já contém dados de rank/status atualizados, dependendo da sua API).
2. Em seguida chama **PopulateSkillList** e **UpdateHeader**.

Se a sua **Load Available Skills** já retornar dados atualizados (incluindo ranks e status), você pode fazer **RefreshSkillList** = apenas **PopulateSkillList** + **UpdateHeader**, sem chamar **Load Player Skills** de novo. Ajuste conforme o contrato do seu backend.

---

### 3.3 Atualizar Header (pontos e classe)

Crie uma função **UpdateHeader** (sem parâmetros):

1. **Get GameInstanceRef** → **Get Skill Points** → **Break FUmbraSkillPoints**.
2. **SkillPoints_Text** → **Set Text** = "Pontos: " + **Available** (ToString).
3. Classe: use **Get Current Character Info** → **Break** → **ClassName** (ou **Get Active Character** e pegar classe). **Class_Text** → **Set Text** = ClassName.

Chame **UpdateHeader** no **Event Construct** (após Load Available Skills) e no callback de **On Available Skills Loaded**.

### 3.4 PopulateSkillList

**Como obter o array de skills no Blueprint (não é função, é variável):**

- **Available Skills** é uma **propriedade** (variável Blueprint Read Only) do **Umbra Game Instance**, não um nó de função "Get Available Skills".
- Passos:
  1. Coloque no gráfico **Get GameInstanceRef** (ou **Get Game Instance** + **Cast to Umbra Game Instance**).
  2. Arraste o **pin de saída (azul)** do Cast (o objeto Umbra Game Instance) e solte no vazio.
  3. No menu de contexto, procure por **"Available Skills"** (com espaço) ou **"AvailableSkills"**. Deve aparecer um nó **"Get Available Skills"** (getter da variável), que retorna **Array of Umbra Skill Data**.
  4. Se não achar: com o **Cast to Umbra Game Instance** no gráfico, **clique com o botão direito** no canvas → **Search** → digite **Available**. Ou na **Details** do nó Cast, verifique se a classe expõe a variável; no Blueprint ela aparece como getter com o nome da propriedade.

Se ainda não aparecer, confira que o **Target** do Cast é realmente **Umbra Game Instance** (não Game Instance genérico) e que o projeto compilou após adicionar as variáveis de Skills no C++.

---

1. **Get GameInstanceRef** → obter a propriedade **Available Skills** (getter retorna **Array of Umbra Skill Data**).
2. **Clear Children** no **container das entradas**, não no Scroll Box:
   - Se dentro do **Skills_ScrollBox** você tem um **Vertical Box** (ex.: **Skills_VBox**) como conteúdo, use **Skills_VBox** → **Clear Children**.
   - **Não** use **Skills_ScrollBox** → Clear Children: isso remove o único filho do Scroll Box (o Vertical Box), e as entradas passam a ser adicionadas a um widget fora da árvore — a lista fica vazia na tela mesmo com a API retornando dados.
3. **For Each Loop** (Array = resultado do get **Available Skills**):
   - **Create Widget**: Class = **WBP_SkillBookEntry**, Owning Player = **Get Player Controller (0)**. Saída → variável local **Entry Widget**.
   - Chamar **SetSkillData** no **Entry Widget**: parâmetro **Skill Data** = **Array Element** (o elemento atual do For Each); parâmetro **Game Instance Ref** = **GameInstanceRef** (do WBP_SkillBook). Ver seção **4.2** e **4.3** para a implementação detalhada de SetSkillData.
   - **Add Child to Vertical Box**: **Target** = **Skills_VBox** (o mesmo Vertical Box que está dentro do Scroll Box), **Content** = **Entry Widget**. Se você não tiver um VBox e o conteúdo do Scroll Box for outro tipo de painel, use **Add Child** nesse painel (o que recebe as entradas).
   - Clique na entrada: ver **4.4 Clique na entrada** (On Mouse Button Down no Entry ou Button, chamar SelectSkill no parent com o SkillID do Entry).

**Troubleshooting – API retorna 15 skills mas a lista não aparece**

- Confira que **Clear Children** está ligado ao **Vertical Box** (Skills_VBox ou o container que fica *dentro* do Scroll Box), e **não** ao **Skills_ScrollBox**. Se limpar o Scroll Box, o filho (VBox) é removido e os novos filhos são adicionados a um widget que já não está no viewport.
- **Add Child** deve ser **Add Child to Vertical Box** (ou ao painel interno do Scroll Box), com **Target** = **Skills_VBox** e **Content** = **Entry Widget**.
- Se o nó **Add Child to Vertical Box** mostrar erro (ex.: ErrorType no nó): verifique se o **Target** é um **Vertical Box** e se o **Content** é um **User Widget** (o WBP_SkillBookEntry).

### 3.5 SelectSkill(SkillID)

Função no **WBP_SkillBook**: nome **SelectSkill**, parâmetro **SkillID** (Integer).

---

#### Passo 1 – Guardar o SkillID selecionado

- **Set SelectedSkillID** = **SkillID** (parâmetro da função).

---

#### Passo 2 – Encontrar a skill no array Available Skills

Não existe nó “Array Find by SkillID” para array de struct em Blueprint. Use **For Each Loop** e compare **Skill ID** do elemento com **SkillID** do parâmetro.

**Opção A – For Each Loop (recomendado)**

| Ordem | Ação | Nós no Blueprint |
|-------|------|-------------------|
| 1 | Obter o array | **Get GameInstanceRef** → **Get Available Skills** (retorna Array of Umbra Skill Data). |
| 2 | Iterar | **For Each Loop**: ligar o **Array** ao resultado de Get Available Skills. Saídas: **Array Element** (cada Umbra Skill Data), **Index**. |
| 3 | Quebrar o struct | **Break Umbra Skill Data**: entrada = **Array Element**. |
| 4 | Comparar ID | **Equal (Integer)**: A = **Skill ID** (pino do Break), B = **SkillID** (parâmetro da função SelectSkill). |
| 5 | Decidir | **Branch**: Condition = saída do Equal. **True** = encontrou a skill; **False** = seguir para a próxima iteração. |
| 6 | No **Branch True** | Usar o **Array Element** (e os pinos do **Break**) para os passos 3 e 4 abaixo. Guardar também: **Set SelectedSkillData** = **Array Element**, para o Action_Button usar depois (bCanLearn, bCanUpgrade, RequiredLevel, etc.). |

Assim, quando **Skill ID == SkillID**, você está no elemento certo: **Array Element** é o **SelectedSkillData** e o **Break** já fornece todos os campos para preencher o painel de detalhes.

**Opção B – Array Find (se existir no seu projeto)**

Em alguns projetos existe **Find** em array de struct que compara um campo. Se houver algo como **Array Find by Key** ou **Find by Predicate**:

- Array = **Get Available Skills**.
- Procurar por elemento cujo **Skill ID** seja igual a **SkillID**.
- Usar o elemento retornado (ou o índice e depois **Get** nesse índice) como **SelectedSkillData**.

Como isso varia por versão/plugins, a opção descrita acima é a **Opção A (For Each + Equal)**.

---

#### Passo 3 – Preencher o Detail Panel (dentro do Branch True do loop)

Com o **Array Element** encontrado (e o **Break Umbra Skill Data** ligado a ele), use os pinos do Break para:

| Widget / ação | Nó / valor |
|---------------|------------|
| **Set SelectedSkillData** | = **Array Element** (para uso no Action_Button). |
| **Skill_Icon_Large** | **GameInstanceRef** → **Get Skill Icon**(**Skill Key**, **true**) → **Set Brush from Texture** no Image. |
| **Skill_Name_Text** | **Set Text** = **Skill Name** (do Break). |
| **Skill_Type_Text** | **Set Text** = **Append**(**Skill Type**, `" • "`, **Element**). |
| **Skill_Description_Text** | **Set Text** = **Description**. |
| **Cost_Text** | **Set Text** = `"Custo: "` + **ToString(Resource Cost)** + `" "` + **Resource Type**. |
| **Cooldown_Text** | **Set Text** = `"CD: "` + **ToString(Cooldown Ms / 1000)** + `"s"`. |
| **Range_Text** | **Set Text** = `"Alcance: "` + **ToString(Range Max)**. |
| **Duration_Text** | **Set Text** = `"Duração: "` + **ToString(Duration Ms / 1000)** + `"s"`. |
| **Scaling_Text** | Montar string com **Str Scaling**, **Dex Scaling**, **Vit Scaling**, **Int Scaling**, **Lck Scaling** (ex.: `"STR "` + ToString(Str Scaling) + `"% "` + …). |
| **CurrentRank_Text** | **Set Text** = `"Rank "` + **ToString(Current Rank)** + `"/"` + **ToString(Max Rank)**. |
| **RankStars_HBox** | Para cada **Star_1** a **Star_5**: **Set Visibility** = **Visible** se índice da estrela ≤ **Current Rank**, senão **Collapsed**. |

---

#### Passo 4 – Atualizar o Action_Button (dentro do mesmo Branch True)

Use os pinos do **Break** (ou **SelectedSkillData** que você setou) para decidir o texto e o estado do botão:

| Condição | Ação no Action_Button |
|----------|------------------------|
| **NOT bIsUnlocked** (bloqueada por nível) | **Set Text** = `"Requer Nível "` + **ToString(Required Level)**. **Set Is Enabled** = false. |
| **bIsUnlocked** e **NOT bIsLearned** e **bCanLearn** | **Set Text** = `"Aprender ("` + **ToString(Skill Cost)** + `" pts)"`. **Set Is Enabled** = true. |
| **bIsLearned** e **bCanUpgrade** | **Set Text** = `"Melhorar ("` + **ToString(Skill Cost)** + `" pts)"`. **Set Is Enabled** = true. |
| **bIsLearned** e **Current Rank** ≥ **Max Rank** | **Set Text** = `"Rank Máximo"`. **Set Is Enabled** = false. |

Ordem sugerida: primeiro **Branch** em **NOT bIsUnlocked**; se True, texto “Requer Nível…” e desabilitar; se False, **Branch** em **bIsLearned**; se False, “Aprender (X pts)” e habilitar; se True, **Branch** em **Current Rank >= Max Rank**; se True, “Rank Máximo” e desabilitar; se False, “Melhorar (X pts)” e habilitar.

### 3.6 OnActionButtonClicked

1. **Get SelectedSkillID**.
2. **Get GameInstanceRef** → **Has Learned Skill(SkillID)** → **Branch**.
3. **False** → **Learn Skill(SkillID)** (Target = GameInstanceRef).
4. **True** → **Upgrade Skill(SkillID)** (Target = GameInstanceRef).

Ligar **Action_Button** → Details → **On Clicked** → + → no Event Graph chamar **OnActionButtonClicked**.

### 3.7 Filtros (opcional)

Nos **Filter_*_Button**, em **On Clicked**: setar um filtro (variável local, ex.: "All", "Active", "Passive", "Learned") e chamar **PopulateSkillList** filtrando o array **Available Skills** antes do **For Each** (ou refazer **PopulateSkillList** com parâmetro de filtro).

---

## 4. WBP_SkillBookEntry – Event Graph e funções

### 4.1 Variáveis (WBP_SkillBookEntry)

Crie uma variável para guardar o **SkillID** (Integer), usada quando o usuário clicar na entrada para chamar **SelectSkill(SkillID)** no parent. As demais informações podem vir só pelo parâmetro da função **SetSkillData**.

| Nome da variável | Tipo | Descrição |
|------------------|------|-----------|
| **SkillID** | Integer | ID da skill desta entrada. Setar dentro de SetSkillData; usar no evento de clique. |

### 4.2 SetSkillData – Implementação detalhada

A função **SetSkillData** fica no **WBP_SkillBookEntry**. Ela recebe **um único parâmetro**: **Skill Data** (tipo **Umbra Skill Data** / FUmbraSkillData). Para obter o ícone, use o **Game Instance** (get do mundo ou variável passada pelo parent).

**Assinatura sugerida no Blueprint:**

- **Função:** SetSkillData  
- **Parâmetros:**  
  - **Skill Data** (tipo **Umbra Skill Data**, por referência)  
  - **Game Instance Ref** (tipo **Umbra Game Instance**, Object Reference) — opcional se você usar **Get Game Instance** + **Cast to Umbra Game Instance** dentro da função.

**Passo a passo dentro de SetSkillData:**

| # | O que fazer | Nó no Blueprint / Detalhe |
|---|-------------|---------------------------|
| 1 | Guardar SkillID | **Set SkillID** (variável do widget) = **Skill Data** → **Break Umbra Skill Data** → pino **Skill ID**. |
| 2 | Nome da skill | **Skill_Name_Text** → **Set Text** = **Skill Data.Skill Name** (do Break). |
| 3 | Tipo da skill | **Skill_Type_Text** → **Set Text** = **Skill Data.Skill Type** (ex.: "ACTIVE", "PASSIVE"). |
| 4 | Nível necessário | **Level_Required_Text** → **Set Text** = **Append**: texto `"Nível "` + **ToString(Skill Data.Required Level)**. |
| 5 | Ícone da skill | **Game Instance Ref** (ou Get Game Instance → Cast to Umbra Game Instance) → **Get Skill Icon** (Skill Data.**Skill Key**, **false**) → retorna Texture2D. Com o retorno: **Skill_Icon** (Image) → **Set Brush from Texture** (Texture = retorno do Get Skill Icon; Size X/Y = 64). Se Get Skill Icon retornar **null**, use um ícone padrão ou deixe o Brush vazio. |
| 6 | Estrelas de rank | **Current Rank** vem do **Break** (Skill Data.**Current Rank**). Para cada **Star_1** a **Star_5** (Image): **Set Visibility** = **Visible** se o índice da estrela (1–5) ≤ Current Rank, senão **Collapsed** (ou **Hit Test Invisible**). Ex.: para Star_1: **Branch** com condição **Current Rank ≥ 1**; True → Star_1 Set Visibility Visible; False → Collapsed. Repita para 2, 3, 4, 5. |
| 7 | Ícone de cadeado | **Locked_Icon** → **Set Visibility**. Condição: **NOT Skill Data.bIsUnlocked**. Se True (bloqueada) → **Visible**; se False → **Collapsed**. |
| 8 | Ícone de aprendido | **Learned_Check** → **Set Visibility**. Condição: **Skill Data.bIsLearned**. Se True → **Visible**; se False → **Collapsed**. |
| 9 | Cor do elemento (opcional) | **Element Color** no struct é **String** (hex, ex. "#4A90D9"). Para usar como cor de borda/fundo: em Blueprint pode existir **Color from Hex** ou similar; ou use **Make Color** com valores R,G,B extraídos. Se não tiver conversão, pode ignorar ou usar uma cor fixa por **Skill Data.Element** (switch). |

**Ordem sugerida no Event Graph da SetSkillData:** 1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 (tudo em sequência a partir do nó **Entry** da função).

**Resumo dos pinos do Break Umbra Skill Data que você usa:**

- **Skill ID** → variável SkillID e (no parent) SelectSkill.
- **Skill Key** → Get Skill Icon.
- **Skill Name** → Skill_Name_Text.
- **Skill Type** → Skill_Type_Text.
- **Required Level** → Level_Required_Text.
- **Current Rank** → estrelas (Star_1..Star_5).
- **bIsUnlocked** → Locked_Icon visibility.
- **bIsLearned** → Learned_Check visibility.
- **Element**, **Element Color** → opcional (cor/borda).

**Se Current Rank ou os bools (bIsUnlocked, bIsLearned, etc.) não aparecerem no Break:** a struct em C++ já tem esses campos (Requirements: CurrentRank; Status: bIsUnlocked, bIsLearned, bCanLearn, bCanUpgrade). Recompile o projeto C++, depois no Blueprint **apague** o nó **Break Umbra Skill Data** e **adicione um novo** (arraste o pin Skill Data → Break Umbra Skill Data). No nó novo, use **Advanced** (seta) para exibir os pins que estiverem ocultos.

**Ícones não aparecem – DataTable e tipo (C++):** O erro `LogDataTable: Error: UDataTable::FindRow : '' specified incorrect type for DataTable` ocorre quando o **SkillIconsDataTable** usa um **Row Structure** diferente do esperado. O C++ aceita **dois** tipos:

1. **FUmbraSkillIconRow** — DataTable com colunas **Icon** e **IconLarge** (TSoftObjectPtr\<UTexture2D\>). **Row Name** = `skill_key` da API (ex.: **BARB_RUIN_STRIKE**). Em cada linha, preencha **Icon** com a textura (ex.: T_Ruin_Strike da pasta Barbarian).

2. **FUmbraSkillDataClient** — ex.: **DT_Skills_Client**. **Row Name** = `skill_key` da API (ex.: **BARB_RUIN_STRIKE**, **BARB_HARDENED_SKIN**). Em cada linha, preencha **IconPath**: só o nome do asset (ex.: **T_Ruin_Strike**) ou o caminho completo (ex.: `/Game/Widgets/UI/Skills/Barbarian/T_Ruin_Strike.T_Ruin_Strike`). O C++ carrega a textura a partir desse caminho; se for só o nome, usa a pasta `/Game/Widgets/UI/Skills/Barbarian/`.

**Resumo para DT_Skills_Client:** No DataTable, adicione linhas com **Row Name** = exatamente o `skill_key` que a API devolve (BARB_RUIN_STRIKE, BARB_HARDENED_SKIN, etc.) e em cada linha preencha **IconPath** com o nome da textura (ex.: T_Ruin_Strike) ou caminho completo. Evite **espaços no início/fim** do IconPath (o C++ faz TrimStartAndEnd como fallback). **Fallbacks no C++ (GetSkillIcon):** (1) trim no IconPath; (2) se houver espaço no meio (ex.: `.../T_Shadow_Strike T_Shadow_Strike`), converte para `.../T_Shadow_Strike.T_Shadow_Strike`; (3) se o caminho tiver `/` mas não tiver `.`, acrescenta `.<NomeDoAsset>`; (4) se IconPath for só o nome do asset (sem `/`), deriva a pasta do prefixo do skill_key (ASSN_→Assassin, DMAGE_→DarkMage, BARB_→Barbarian, TMPL_/TEMPLAR_→Templar).

### 4.3 Chamar SetSkillData a partir do WBP_SkillBook (PopulateSkillList)

**O que é “Entry Widget”?**  
**Entry Widget** é o nome que damos aqui à **variável** (ou ao pino de saída do nó) que guarda a **referência ao widget criado** em cada volta do loop. Ou seja: a cada skill do array, você usa **Create Widget** para criar **uma instância** do **WBP_SkillBookEntry**; essa instância é “uma entrada” da lista. A referência a essa instância é o **Entry Widget**. No Blueprint você pode chamar a variável de “Entry Widget”, “Entry”, “SkillEntry” ou outro nome — o importante é usar **a mesma referência** (o retorno do Create Widget) em **SetSkillData**, **SetSkillBookRef** e **Add Child to Vertical Box**, pois é sempre “esta entrada que acabei de criar”.

---

Dentro do **For Each Loop** (array = **Available Skills**):

1. **Create Widget**: Class = **WBP_SkillBookEntry**, Owning Player = **Get Player Controller (0)**. Guarde o **retorno** do nó em uma variável local — é essa referência que chamamos de **Entry Widget** nos passos abaixo.
2. **Get Game Instance** → **Cast to Umbra Game Instance** → guarde em variável (ex.: **GI**) se ainda não tiver.
3. Chame **SetSkillData** no **Entry Widget** (Target = Entry Widget):
   - **Skill Data** = o **Array Element** do For Each Loop (cada **Umbra Skill Data**).
   - **Game Instance Ref** = **GI** (ou deixe opcional e use Get Game Instance dentro do Entry).
4. **Set Skill Book Ref**: Target = **Entry Widget**, **In Skill Book** = **self** (o WBP_SkillBook). O C++ **UUmbraSkillBookEntryWidget** já expõe essa função; ao clicar na entrada, o C++ chama **SelectSkill(SkillID)** no SkillBookRef. **Não é necessário bind nem Custom Event** — tudo dentro da função. Ver **4.4**.
5. **Add Child to Vertical Box**: **Target** = **Skills_VBox**, **Content** = **Entry Widget**.

Assim, cada entrada recebe o struct completo; **Current Rank**, **bIsUnlocked**, **bIsLearned**, **bCanLearn** e **bCanUpgrade** já vêm preenchidos pela API (o C++ preenche no parse).

### 4.4 Clique na entrada – BTN_SkillSelect e SelectSkill (C++: SkillBookRef, sem bind)

O **WBP_SkillBookEntry** herda de **UUmbraSkillBookEntryWidget** (C++) e tem um **Button** **BTN_SkillSelect**. O **WBP_SkillBook** herda de **UUmbraSkillBookWidget** (C++). Em **PopulateSkillList** você só chama **Set Skill Book Ref(Entry, self)**; **não é possível** fazer bind a delegate nem chamar Custom Event a partir de uma função no Blueprint, então o fluxo usa referência direta: ao clicar, o C++ do Entry chama **SkillBookRef->SelectSkill(CachedSkillID)**.

#### 4.4.1 Clique não chega no botão – causa no código (C++)

**Causa real:** Com o input em modo **Game Only**, o motor envia o clique para o mundo (personagem, etc.) e **não** para os widgets da UI. Por isso o **On Clicked** do botão da entrada nunca dispara.

**Solução (já no código):** Em **UmbraEternumUEPlayerController::OpenSkillBook()** o C++ agora faz:
- **SetShowMouseCursor(true)**
- **FInputModeGameAndUI** com **SetWidgetToFocus(SkillBookWidgetRef->TakeWidget())**
- **SetInputMode(InputMode)**

Assim a UI passa a receber cliques. Se você ainda não recompilou após essa alteração, recompile o projeto e teste de novo.

**Botão Fechar:** O botão que fecha o Skill Book deve chamar **Get Player Controller (0)** → **Cast to UmbraEternumUEPlayerController** → **Close Skill Book** (e **não** Remove From Parent). O **CloseSkillBook()** em C++ remove o widget e restaura **SetShowMouseCursor(false)** e **FInputModeGameOnly**. Se o Blueprint só fizer Remove From Parent, o cursor e o input mode ficam em estado errado ao fechar.

**Outras checagens:** Root do Entry com **Visibility** = **Visible** (não Hit Test Invisible). Slot do Entry no **Skills_VBox** com altura definida (Fill ou Height mínima ~72).

#### No WBP_SkillBook e WBP_SkillBookEntry (recomendado – só C++ + PopulateSkillList)

- **WBP_SkillBook:** Reparent para **Parent Class = UmbraSkillBookWidget**. Implemente **Select Skill (SkillID)** no Blueprint (evento do parent ou função que preenche Detail_Panel e Action_Button).
- **WBP_SkillBookEntry:** Reparent para **Parent Class = UmbraSkillBookEntryWidget**. Botão da linha = **BTN_SkillSelect**. Em **SetSkillData**, no primeiro nó chame **Cache Skill Data (Data)**; depois preencha nome, tipo, nível, ícone.
- **PopulateSkillList:** Create Widget → SetSkillData(Entry, Array Element) → **Set Skill Book Ref(Entry, self)** → Add Child to Vertical Box. **Nenhum bind nem Custom Event** — tudo dentro da função. Guia passo a passo: **PROCEDIMENTO_WBP_SKILLBOOK_ENTRY_CLIQUE.md**.

---

#### Se ao clicar não acontecer nada (troubleshooting)

| Verificação | O que fazer |
|-------------|-------------|
| **SkillBookRef está null** | Em **PopulateSkillList**, após **SetSkillData**, chame **Set Skill Book Ref** (Target = Entry Widget, **In Skill Book** = **self**). O C++ do Entry usa essa referência para chamar **Select Skill** ao clicar. |
| **Clique não dispara (Print String não aparece)** | **Causa no código:** o input estava em modo Game Only; a UI não recebia cliques. O C++ foi alterado: **OpenSkillBook()** agora chama **SetShowMouseCursor(true)** e **SetInputMode(GameAndUI)** com foco no widget. Recompile e teste. O botão Fechar deve chamar **Close Skill Book** no Player Controller (não Remove From Parent). Ver **4.4.1**. |
| **Select Skill não existe ou nome errado** | O **WBP_SkillBook** deve ter Parent Class = **UmbraSkillBookWidget** e implementar **Select Skill (SkillID)** (evento Blueprint do parent). Assim o C++ do Entry consegue chamar no SkillBookRef. |
| **SkillID é 0** | Em **SetSkillData** do Entry, o **primeiro** nó deve ser **Cache Skill Data (Data)**; o C++ guarda **Data.SkillID** para o clique. Sem isso, **SelectSkill(0)** é chamado e o painel não mostra a skill correta. |

**Teste rápido:** No C++ do Entry há log ao clicar. Se ao clicar aparecer no log "Slot clicado: SkillID=123", o clique e o cache estão ok; se o painel não atualizar, o problema está na implementação de **Select Skill** no **WBP_SkillBook** (ex.: não encontra a skill no array ou não preenche o Detail_Panel).

---

## 5. Checklist final

- [ ] **WBP_SkillBook:** Root = Canvas Panel; Header_Panel (Border) com Header_HBox (Title_Text, Class_Text, SkillPoints_Text, Close_Button). Botão Fechar → **Get Player Controller (0)** → **Cast to UmbraEternumUEPlayerController** → **Close Skill Book** (não Remove From Parent).
- [ ] **WBP_SkillBook:** Content_Panel (Horizontal Box) com List_Column_VBox (Filter_Panel + Skills_ScrollBox) e Detail_Panel (Detail_VBox com ícone, textos, stats, rank, Action_Button).
- [ ] **WBP_SkillBook:** Nomes de todos os controles conforme tabela.
- [ ] **WBP_SkillBook:** Event Construct → Bind On Available Skills Loaded / On Skill Learned / On Skill Upgraded / On Available Skills Loaded Failed; Load Available Skills; Close_Button → Close Skill Book (no Player Controller).
- [ ] **WBP_SkillBook:** PopulateSkillList (Clear Children no Skills_VBox, For Each Available Skills → Create WBP_SkillBookEntry, SetSkillData, **Set Skill Book Ref(Entry, self)**, Add Child). Sem bind nem Custom Event.
- [ ] **WBP_SkillBook:** SelectSkill(SkillID) preenche Detail_Panel e Action_Button; Action_Button On Clicked → Learn/Upgrade via Game Instance.
- [ ] **WBP_SkillBookEntry:** Entry_HBox com Skill_Icon, Info_VBox, Rank_HBox, Status_Overlay; SetSkillData preenche textos e ícones; clique notifica parent SelectSkill(SkillID).
- [ ] **UpdateHeader** chamado ao carregar e após learn/upgrade (SkillPoints_Text e Class_Text).

Guia geral: **GUIA_WBP_SKILLBOOK.md**.
