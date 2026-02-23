# Procedimento – Clique na entrada do Skill Book (selecionar skill)

Guia para fazer o clique numa linha da lista de skills **selecionar** a skill e atualizar o painel de detalhes. Usa **referência ao parent (SkillBookRef)** e chamada direta a **SelectSkill**: **não é necessário bind nem Custom Event** — tudo é feito dentro da função **PopulateSkillList**.

---

## 1. O que foi adicionado no C++

### 1.1 UUmbraSkillBookWidget (classe base do WBP_SkillBook)

- **Arquivos:** `Source/UmbraEternumUE/UI/UmbraSkillBookWidget.h` e `.cpp`.
- **SelectSkill(int32 SkillID):** **BlueprintNativeEvent**; no Blueprint você implementa o **Event Select Skill** (override) e chama **Update Detail Panel (SkillID)**.
- O C++ do Entry chama **SelectSkill** via **ProcessEvent** para garantir que o override do Blueprint seja executado. O **WBP_SkillBook** deve ter **Parent Class = UmbraSkillBookWidget**.

### 1.2 UUmbraSkillBookEntryWidget (classe base do WBP_SkillBookEntry)

- **Arquivos:** `Source/UmbraEternumUE/UI/UmbraSkillBookEntryWidget.h` e `.cpp`.
- **BTN_SkillSelect:** botão com esse nome no Designer; o C++ faz bind de **OnClicked** no **NativeConstruct**.
- **SkillBookRef:** referência ao Skill Book (tipo **Umbra Skill Book Widget**). Ao clicar, o C++ chama **SkillBookRef->SelectSkill(CachedSkillID)**. Nenhum bind no parent é necessário.
- **SetSkillBookRef(UUmbraSkillBookWidget* InSkillBook):** BlueprintCallable; chame em **PopulateSkillList** com **self** (o WBP_SkillBook) para cada Entry criado.
- **CacheSkillData(const FUmbraSkillData& Data):** guarda **Data.SkillID** para o clique. No Blueprint, no início da sua **SetSkillData** chame **Cache Skill Data (Data)**.

---

## 2. WBP_SkillBookEntry – herdar do widget C++

| Passo | Ação |
|-------|------|
| 1 | Abra o **WBP_SkillBookEntry**. **File → Reparent Blueprint** → Parent Class = **UmbraSkillBookEntryWidget**. Salve. |
| 2 | No Designer, o botão da linha deve se chamar **BTN_SkillSelect**. |
| 3 | Na **SetSkillData** do Blueprint, no primeiro nó chame **Cache Skill Data** (Data = seu parâmetro Skill Data). Depois preencha nome, tipo, nível, ícone, etc. |

---

## 3. WBP_SkillBook – reparent e PopulateSkillList (sem bind, sem Custom Event)

### 3.1 Reparent do WBP_SkillBook

| Passo | Ação |
|-------|------|
| 1 | Abra o **WBP_SkillBook**. **File → Reparent Blueprint** → Parent Class = **UmbraSkillBookWidget**. Salve. |
| 2 | No Event Graph, use o **Event Select Skill** (override do parent) e chame a sua função **Update Detail Panel (SkillID)**. A função **Update Detail Panel** deve receber **SkillID** e preencher **Detail_Panel** e **Action_Button** (ver seção 4). **Não** crie uma função chamada Select Skill — o nome já é usado pelo evento do C++. |

### 3.2 PopulateSkillList – só chamadas de função

Dentro da **função** **PopulateSkillList** (apenas chamadas; **não** use Bind Event nem Custom Event):

| Ordem | Ação |
|-------|------|
| 1 | **Clear Children** no **Skills_VBox**. |
| 2 | **For Each Loop** em **Available Skills**. |
| 3 | **Create Widget**: Class = **WBP_SkillBookEntry**, Owning Player = **Get Player Controller (0)**. Saída → **Entry Widget**. |
| 4 | **SetSkillData**: Target = **Entry Widget**, parâmetros = **Array Element** (Skill Data) e **Game Instance Ref**. |
| 5 | **Set Skill Book Ref**: Target = **Entry Widget**, **In Skill Book** = **self** (referência ao próprio WBP_SkillBook). Assim, ao clicar na entrada, o C++ chama **Select Skill** no parent. |
| 6 | **Add Child to Vertical Box**: Target = **Skills_VBox**, Content = **Entry Widget**. |

**Resumo:** Create Widget → SetSkillData(Entry, Array Element) → **Set Skill Book Ref(Entry, self)** → Add Child. Nenhum bind nem Custom Event é necessário.

---

## 4. Ligar o C++ Select Skill ao Blueprint (Event + UpdateDetailPanel)

### 4.1 Como o C++ chama o Blueprint

Quando o jogador clica numa entrada da lista, o C++ do Entry chama **SkillBookRef->SelectSkill(SkillID)**. O **WBP_SkillBook** herda de **UmbraSkillBookWidget**, então o Unreal dispara no Blueprint o **evento** **Select Skill** herdado do parent. Você **não** cria uma função com o mesmo nome; você implementa esse **evento** e chama sua função **Update Detail Panel** a partir dele.

### 4.2 Passo a passo no WBP_SkillBook

#### A) Colocar o evento Select Skill no Event Graph

1. Abra o **WBP_SkillBook** e vá ao **Event Graph**.
2. Clique com o botão direito no canvas → **Add Event** → procure por **Select Skill** (ou **Event Select Skill**).
   - Deve aparecer um evento do tipo **Override** (ou "Parent: Umbra Skill Book Widget") com um parâmetro **SkillID** (Integer).
   - Se não aparecer em Add Event, procure na paleta **Override** em **Umbra Skill Book Widget** → **Select Skill**.
3. Esse nó é o **Event Select Skill**: quando o C++ chama **SelectSkill(SkillID)** no widget, esse evento executa com o **SkillID** recebido.

#### B) Chamar Update Detail Panel a partir do evento

4. Do nó **Event Select Skill**, ligue o **pin de execução** (branco) ao **nó Update Detail Panel** (sua função).
5. Ligue o parâmetro **SkillID** do **evento** (não a variável SelectedSkillID) ao parâmetro **SkillID** da **Update Detail Panel**. Opcional: antes de Update Detail Panel, use **Set SelectedSkillID** = **SkillID** (do evento) para guardar.
6. Esse fluxo (**Event Select Skill** → **Update Detail Panel**) deve ser **uma cadeia separada** no Event Graph. **Não** ligue o Event Select Skill à cadeia de **Event Construct** / **UpdateHeader** / **LoadAvailableSkills**.

#### C) O que NÃO fazer (erro comum)

- **Não** coloque **Call Select Skill** (a função) na sequência de abertura do livro (depois de UpdateHeader ou LoadAvailableSkills). O **Select Skill** que aparece no Blueprint é o **evento** que dispara quando o jogador **clica** numa entrada — não é para ser chamado na abertura.
- **Não** use a variável **SelectedSkillID** como entrada de **Update Detail Panel** no fluxo de abertura (nesse momento ela vale 0). Use o **parâmetro SkillID** do **Event Select Skill**.
- **Não** use **Assign Delegate** nem **Bind Event to On Skill Entry Clicked** em lugar nenhum; remova esses nós se ainda existirem.

#### D) Fluxo de abertura (Event Construct) — sem Select Skill

A abertura do livro deve ser algo como: **Event Construct** → Set GameInstanceRef → (binds dos delegates do Game Instance) → **UpdateHeader** → **Load Available Skills**. Pare aí. Quando as skills carregarem, o delegate **On Available Skills Loaded** chama **PopulateSkillList** (e **UpdateHeader** se quiser). Não chame SelectSkill nem UpdateDetailPanel nessa cadeia.

#### E) O que a função Update Detail Panel deve fazer

A função **Update Detail Panel** deve receber **SkillID** (Integer) e:

| Ordem | Ação |
|-------|------|
| 1 | **Set SelectedSkillID** = **SkillID** (variável do WBP_SkillBook). |
| 2 | **For Each Loop** no array **Available Skills** (ex.: **Get Game Instance** → **Get Available Skills**). |
| 3 | Dentro do loop: **Branch** → comparar **Skill ID** do **Array Element** (do **Break Umbra Skill Data**) com **SkillID** (parâmetro). |
| 4 | No **Branch True**: preencher o **Detail_Panel** (nome, tipo, descrição, custo, cooldown, alcance, scaling, rank, etc.) e atualizar o **Action_Button** (texto e visibilidade conforme **bCanLearn** / **bCanUpgrade**). Opcional: guardar o **Array Element** em **SelectedSkillData** para uso no botão Learn/Upgrade. |
| 5 | **Break** o loop após encontrar (opcional) ou deixar o loop terminar após preencher. |

Ver **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md** seção 3.5 para detalhes dos campos do painel e do Action_Button.

### 4.3 Se ainda nada acontecer ao clicar (diagnóstico)

O C++ do Entry grava mensagens no **Output Log** (LogTemp):

| Mensagem | Causa | O que fazer |
|----------|--------|-------------|
| **Clique ignorado: CachedSkillID=0** | O Entry não guardou o SkillID. | Em **WBP_SkillBookEntry**, na **SetSkillData**, a primeira ação deve ser **Cache Skill Data (Data)** com o struct da skill (Array Element). |
| **SkillBookRef é null** | O parent não passou a referência. | Em **PopulateSkillList**, após **Set Skill Data**, chame **Set Skill Book Ref** (Target = **Entry Widget**, In Skill Book = **self**). |
| **FindFunction(SelectSkill) retornou null** | O widget não é UmbraSkillBookWidget. | **WBP_SkillBook** → Reparent para **Parent Class = UmbraSkillBookWidget**. |

**Se aparece "BTN_SkillSelect vinculado" ao abrir o livro mas ao clicar NÃO aparece "OnSkillSelectClicked chamado":** o clique não chega ao botão.

**Causa:** Com a lista **dentro de um ScrollBox**, os botões dos entries não recebem o clique (comportamento conhecido do engine). A solução é usar **apenas Vertical Box** na coluna da lista, sem ScrollBox — igual a uma lista que já funciona.

#### 4.3.1 Remover o ScrollBox da lista no WBP_SkillBook (obrigatório para o clique funcionar)

Se o clique na entrada **ainda** não disparar após o acima, pode tentar fazer do **BTN_SkillSelect** o **root** do widget (em vez de um Canvas Panel com o botão dentro).

| Passo | Ação |
|-------|------|
| 1 | Abra **WBP_SkillBookEntry**. Na **Hierarchy**, o root atual é um **Canvas Panel** e o único filho é **BTN_SkillSelect**. |
| 2 | Selecione **BTN_SkillSelect** → botão direito → **Move to Parent** (ou arraste o BTN_SkillSelect para fora, para ser irmão do Canvas). Em alguns editores: **Remove Parent** ou promova o botão para root. |
| 3 | Apague o **Canvas Panel** que ficou vazio (ou remova o Canvas e deixe o Button como único root). O **root** do Blueprint deve passar a ser **BTN_SkillSelect**. |
| 4 | Confira no **Details** do root que o **Name** do widget é **BTN_SkillSelect** (obrigatório para o C++ encontrar o botão). Se o nome mudar ao promover, renomeie para **BTN_SkillSelect**. |
| 5 | O conteúdo atual do botão (Entry_HBox, Skill_Icon, Info_VBox, Rank_HBox, Status_Overlay, etc.) deve continuar **dentro** do BTN_SkillSelect. Não mexa neles. |
| 6 | Salve e recompile. Ao adicionar o entry em **Skills_VBox**, o primeiro widget “clicável” no caminho será o próprio botão (root), e o ScrollBox tende a entregar o clique corretamente. |

Se não existir “Move to Parent”, use: duplicar o conteúdo do botão, criar um novo User Widget com **Button** como root, nomear o root **BTN_SkillSelect**, colar o conteúdo no botão e ajustar o restante do layout.

| Verificação | O que fazer |
|-------------|-------------|
| **ScrollBox no WBP_SkillBook** | O C++ faz bind de **Skills_ScrollBox** e chama **SetConsumeMouseWheel(false)**. O ScrollBox da lista no Blueprint deve ter **Name = Skills_ScrollBox**. |
| **Botão não preenche o entry** | **BTN_SkillSelect** → **Slot (Canvas Panel Slot)**: **Anchors** = stretch (0,0 a 1,1), **Offsets** = 0. |
| **Root do WBP_SkillBookEntry** | Root = **Visible**. Se for Canvas Panel, Visibility não pode ser Hit Test Invisible. |
| **SetWidgetToFocus / Input** | Removido SetWidgetToFocus. Use **FInputModeGameAndUI** (não UI Only). Recompile. |

No Event Graph do **WBP_SkillBook**: use **apenas** o nó **Event Select Skill** (ex.: 350×150) no slot do Canvas Panel. Se o entry for mais alto que 150 px, a área **abaixo** do botão recebe o hit no **Canvas Panel** (root), não no botão — e o clique não dispara. **Solução:** no **WBP_SkillBookEntry**, selecione **BTN_SkillSelect** → em **Details** abra **Slot (Canvas Panel Slot)**. Defina **Anchors** para preencher tudo (ex.: Min 0,0 / Max 1,1 ou “Stretch”) e **Offsets** 0 (ou Left/Right/Top/Bottom = 0) para o botão **preencher todo o entry**. Assim qualquer clique na linha cai no botão. |
| **Root do WBP_SkillBookEntry** | No Designer, selecione o **root** (o **Canvas Panel** que contém o BTN_SkillSelect). Em **Details → Appearance → Visibility** deve estar **Visible**. Se estiver **Hit Test Invisible** ou **Self Hit Test Invisible**, mude para **Visible**. |
| **WBP_SkillBook – algo cobrindo a lista** | No **WBP_SkillBook**, algum painel (ex.: imagem de fundo, Border full-screen ou o **Detail_Panel**) pode estar **por cima** da coluna da lista. Confira a ordem na hierarquia e o tamanho dos painéis; a coluna da lista não pode estar coberta. Se houver fundo que cobre tudo, use **Visibility = Hit Test Invisible** nele. |
| **C++** | O C++ força **SetVisibility(Visible)** no Entry e no BTN_SkillSelect em **NativeConstruct**. Recompile e teste. |

No Event Graph do **WBP_SkillBook**: use **apenas** o nó **Event Select Skill** (override) → **Update Detail Panel (SkillID)**. Não coloque nenhum nó **Call Select Skill** no meio.

### 4.4 Resumo do fluxo

- **Clique na entrada** → C++ Entry chama **SelectSkill(SkillID)** no **SkillBookRef** via **ProcessEvent** (para disparar o Blueprint).
- **WBP_SkillBook** → o **Event Select Skill (SkillID)** dispara.
- No Event Graph: **Event Select Skill** → **Update Detail Panel (SkillID)**.
- **Update Detail Panel** → guarda SelectedSkillID, encontra a skill no array, preenche Detail_Panel e Action_Button.

---

## 5. Checklist

- [ ] **C++:** Projeto recompilado (UmbraSkillBookWidget e UmbraSkillBookEntryWidget).
- [ ] **WBP_SkillBook:** Parent Class = **UmbraSkillBookWidget**; **Event Select Skill** no Event Graph chama **Update Detail Panel (SkillID)**; a função **Update Detail Panel** preenche Detail_Panel e Action_Button.
- [ ] **WBP_SkillBookEntry:** Parent Class = **UmbraSkillBookEntryWidget**; botão = **BTN_SkillSelect**; em **SetSkillData**, primeiro nó = **Cache Skill Data (Data)**.
- [ ] **WBP_SkillBook – PopulateSkillList:** Create Widget → SetSkillData(Entry, Array Element) → **Set Skill Book Ref(Entry, self)** → Add Child. Sem bind, sem Custom Event.

---

## 6. Referências

- **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md** — Estrutura do WBP_SkillBook, SelectSkill, ícones.
- **GUIA_HUD_SKILLBOOK_BOTAO_INPUT.md** — Abrir/fechar o Skill Book (tecla K, CloseSkillBook).
