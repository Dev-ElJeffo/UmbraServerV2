# Guia – Botão na HUD e tecla K para abrir WBP_SkillBook

Procedimento completo para abrir o Livro de Skills pela **tecla K** e por um **botão na HUD**, usando Enhanced Input e a função **Open Skill Book** do Player Controller.

---

## Visão geral

| O quê | Onde |
|-------|------|
| **Tecla K** | Input Action **IA_OpenSkillBook** mapeada no **IMC_Default**; Character faz o bind e chama **Player Controller → Open Skill Book**. |
| **Botão na HUD** | Um botão (ex.: ícone de livro) no widget de HUD; **On Clicked** → **Get Player Controller (0)** → **Open Skill Book**. |
| **Abrir o painel** | **Open Skill Book** (C++) no Player Controller cria o widget **WBP_SkillBook** (se ainda não existir), adiciona ao viewport e torna visível. |

O código C++ já está no projeto: **UmbraEternumUECharacter** (bind da ação) e **UmbraEternumUEPlayerController** (função **OpenSkillBook** e variáveis **SkillBookWidgetClass** / **SkillBookWidgetRef**). No Editor você só precisa criar o Input Action, configurar o IMC e atribuir as variáveis nos Blueprints.

---

## Parte 1 – Input Action (tecla K)

### 1.1 Criar o Input Action

1. No **Content Browser**, vá até a pasta de Input (ex.: **Content/Input** ou **Content/Input/Actions**). Se não existir, crie **Input** e dentro **Actions**.
2. Botão direito na pasta **Actions** → **Input** → **Input Action**.
3. Nome do asset: **IA_OpenSkillBook**.
4. Abra o asset e, em **Details**:
   - **Value Type**: **Digital (bool)**.
   - Demais opções podem ficar padrão.
5. Salve (Ctrl+S).

### 1.2 Mapear a tecla K no IMC_Default

1. Localize o **Input Mapping Context** usado no jogo (ex.: **IMC_Default** em **Content/Input**). Se o projeto usar outro IMC para o personagem, use esse.
2. Abra **IMC_Default** (duplo clique).
3. Clique em **+ Add Mapping**.
4. Em **Mappings**:
   - **Action**: selecione **IA_OpenSkillBook** (arraste do Content Browser ou use o dropdown).
   - **Key**: **K** (teclado). Clique no campo **Key**, pressione a tecla **K** no teclado ou escolha **K** na lista.
   - Modificadores (Shift, Ctrl, Alt): deixe em branco se quiser apenas **K**.
5. Salve o IMC.

Se o jogo já adiciona **IMC_Default** no **Player Controller** (como no C++ do projeto), não é preciso fazer mais nada para o contexto. Caso use outro IMC, adicione o mapeamento nesse IMC.

---

## Parte 2 – Blueprint do Character (tecla K)

O **Character** (ex.: **ThirdPersonCharacter** ou o Blueprint que o jogo usa no nível) precisa ter a variável **Open Skill Book Action** apontando para **IA_OpenSkillBook** para o bind da tecla K funcionar.

1. Abra o Blueprint do **personagem** (ex.: **ThirdPersonCharacter**).
2. No **Details** (ou na aba **Class Defaults**), procure a categoria **Input**.
3. Localize **Open Skill Book Action**.
4. Atribua **IA_OpenSkillBook** (arraste do Content Browser ou use o dropdown).
5. **Compile** e **Salve**.

Assim, ao pressionar **K**, o Character chama **OpenSkillBookPressed** no C++, que por sua vez chama **OpenSkillBook()** no Player Controller.

---

## Parte 3 – Blueprint do Player Controller (classe do widget)

O **Player Controller** precisa saber qual widget criar ao abrir o Livro de Skills. Isso é feito pela variável **Skill Book Widget Class**.

1. Abra o Blueprint do **Player Controller** (ex.: o que herda de **UmbraEternumUEPlayerController**).
2. Em **Class Defaults** (ou **Details** quando nenhum nó está selecionado), procure **UI | Skills**.
3. Em **Skill Book Widget Class**, atribua **WBP_SkillBook** (arraste do Content Browser ou use o dropdown). Use a classe do Blueprint (ex.: **WBP_SkillBook_C**).
4. **Compile** e **Salve**.

Se não definir essa variável, o C++ tenta carregar **WBP_SkillBook** de caminhos padrão; definir no Blueprint evita dependência do caminho do asset.

---

## Parte 4 – Botão na HUD

A HUD é o widget principal exibido durante o jogo (ex.: **WBP_PlayerHUD**, **WBP_MainHUD** ou o que contém a barra de vida, inventário, etc.). Você vai adicionar um **botão** que chama **Open Skill Book** no Player Controller.

### 4.1 Onde está a HUD?

- Pode ser um widget criado no **Player Controller** (BeginPlay) ou no **Game Mode** e adicionado ao viewport.
- Pode ser o mesmo widget que contém a **WBP_SkillBar** (barra de skills). Se a Skill Bar já estiver em um widget “principal”, adicione o botão do Livro de Skills nesse mesmo widget ou em um painel ao lado.

Se não tiver um HUD único, crie um **Canvas** ou **Overlay** que seja o root do widget principal do jogo e coloque a Skill Bar e o novo botão nele.

### 4.2 Adicionar o botão

1. Abra o **widget da HUD** (ex.: **WBP_PlayerHUD**).
2. No **Designer**, escolha um lugar para o botão (ex.: ao lado da Skill Bar, canto da tela, ou dentro de um **Horizontal Box** com outros ícones).
3. Arraste para o layout:
   - **Button** (ou **Image** com **Is Variable** e um **On Clicked** se preferir um ícone clicável).
4. Nome sugerido para o botão: **SkillBook_Button** ou **BTN_OpenSkillBook**.
5. Ajuste o texto do botão (ex.: **"K"** ou **"Skills"**) ou associe um **Image** com ícone de livro.
6. **Compile** e **Salve** o widget.

### 4.3 Ligar o clique ao Open Skill Book

1. No mesmo widget da HUD, vá à aba **Event Graph**.
2. Selecione o **botão** na Hierarchy (ou no Designer) e no **Details** → **Events** → **On Clicked** → clique no **+** para criar o evento no Event Graph.
3. No **Event Graph**, você verá um nó **On Clicked (SkillBook_Button)** (ou nome que deu ao botão).
4. Ligue a **exec** (branco) desse nó à seguinte sequência:
   - **Get Player Controller**: **Player Index** = **0**.
   - **Cast to UmbraEternumUEPlayerController** (ou ao Blueprint do seu Player Controller): **Object** = **Return Value** do Get Player Controller.
   - **Open Skill Book**: **Target** = saída **As Umbra Eternum UE Player Controller** do Cast (ou equivalente no seu Blueprint).

**Resumo:** `On Clicked (botão)` → `Get Player Controller (0)` → `Cast to [seu Player Controller]` → `Open Skill Book`.

Assim, ao clicar no botão, o mesmo **OpenSkillBook()** do Player Controller é chamado (cria o widget se necessário, adiciona ao viewport e torna visível).

---

## Parte 5 – Fechar o WBP_SkillBook

O **WBP_SkillBook** já deve ter o **Close_Button** com **On Clicked** ligado a **Remove From Parent** (conforme **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md**). Ao fechar:

- O widget é removido do viewport.
- O **Player Controller** mantém a referência (**SkillBookWidgetRef**); na próxima vez que você chamar **Open Skill Book** (tecla K ou botão), o mesmo widget é **Add To Viewport** de novo e fica visível (não cria outro).

Não é necessário limpar **SkillBookWidgetRef** ao fechar.

---

## Parte 6 – Testar

1. **Compilar** o projeto (C++ e Blueprints alterados).
2. Abrir um nível onde o personagem usa o **Character** e o **Player Controller** configurados acima.
3. **Play** (Editor ou PIE).
4. **Tecla K**: deve abrir o **WBP_SkillBook**. Fechar com o botão **Fechar** do painel e pressionar **K** de novo: o painel deve reabrir.
5. **Botão na HUD**: clicar no botão que você configurou deve abrir o mesmo **WBP_SkillBook**.
6. Verificar no **Output Log** (se habilitado): mensagens como `[UmbraPlayerController] ✅ WBP_SkillBook criado.` e `WBP_SkillBook aberto.` indicam que o fluxo está correto.

---

## Resumo rápido

| Passo | Onde | Ação |
|-------|------|------|
| 1 | Content/Input/Actions | Criar **IA_OpenSkillBook** (Digital). |
| 2 | IMC_Default | Adicionar mapping **IA_OpenSkillBook** → tecla **K**. |
| 3 | Blueprint do Character | **Open Skill Book Action** = **IA_OpenSkillBook**. |
| 4 | Blueprint do Player Controller | **Skill Book Widget Class** = **WBP_SkillBook**. |
| 5 | Widget da HUD | Adicionar botão; **On Clicked** → **Get Player Controller (0)** → **Cast to Player Controller** → **Open Skill Book**. |
| 6 | WBP_SkillBook | **Close_Button** → **Remove From Parent** (já descrito no procedimento do Skill Book). |

---

## Troubleshooting

- **Tecla K não abre:** Confirme que **IMC_Default** (ou o IMC usado) está sendo adicionado no **Player Controller** (C++ já faz isso) e que no Blueprint do **Character** a variável **Open Skill Book Action** está definida como **IA_OpenSkillBook**. Confira também se o nível está usando esse Character e esse Player Controller.
- **Botão não abre:** Confirme que o **Target** do **Open Skill Book** é o **Player Controller** (saída do Cast). Verifique se o widget da HUD está no viewport (ele é criado no BeginPlay do Player Controller ou em outro ponto do seu fluxo).
- **"SkillBookWidgetClass não definido":** Atribua **WBP_SkillBook** em **Skill Book Widget Class** no Blueprint do **Player Controller**. Se o **WBP_SkillBook** estiver em outra pasta (ex.: **Content/UI/Skills/**), o C++ tenta alguns caminhos; o mais seguro é definir a classe no Blueprint.
- **Painel abre mas não carrega skills:** Verifique o **Event Construct** e os delegates do **WBP_SkillBook** (bind em **On Available Skills Loaded**, chamada a **Load Available Skills**), além de **GUIA_WBP_SKILLBOOK.md** e **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md**. O backend (MySQL + API PHP) deve estar rodando para **get_available_skills**.

---

## Referências

- **PROCEDIMENTO_WBP_SKILLBOOK_BLUEPRINT.md** – Estrutura do WBP_SkillBook, Event Graph, SelectSkill, delegates.
- **GUIA_WBP_SKILLBOOK.md** – Integração com GameInstance e fluxo de dados.
- **GUIA_WBP_SKILLBAR.md** – Barra de skills na HUD.
- **PROCEDIMENTO_WBP_WHISPER_BLUEPRINT.md** – Padrão similar (abrir painel a partir da HUD e reabrir).
