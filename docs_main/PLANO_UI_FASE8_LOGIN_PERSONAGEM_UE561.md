# PLANO — Fase 8: login, cadastro, seleção e criação de personagem (UE 5.6.1)

Redesign das telas de entrada do jogo, no padrão visual da fase 7 (cartão escuro com borda dourada, Cinzel nos títulos, `UmbraUIStyle`), mostrando todas as informações de forma limpa e corrigindo o que está quebrado ou em inglês. O C++ continua dono da lógica; os WBPs só têm layout.

Telas: `/Game/Widgets/WBP_Login2`, `WBP_Register`, `WBP_CharacterSelection`, `WBP_CharacterSelectButton`, `WBP_CharacterItem`, `WBP_CreateCharacter` (+ `/Game/Widgets/UI/ClassSelectorItem/WBP_ClassSelectorItem`, usado dentro da criação).

## 1) Diagnóstico (lido via MCP em 18/11/2026)

| Tela | Hoje | Problemas |
|------|------|-----------|
| **Login** (`UmbraLoginWidget`) | Arte de fundo + logo bons; caixa pequena com Usuário/Senha, "Entrar" e "Criar conta" lado a lado, barra de status vazia sempre visível | Campos sem rótulo (só placeholder); botões do mesmo peso (ação principal não se destaca); barra de status vazia ocupa espaço; sem "lembrar usuário", sem mostrar senha, sem aviso de Caps Lock; sem versão/estado do servidor |
| **Cadastro** (`UmbraRegisterWidget`) | Mesma moldura, 4 campos, "Cadastrar"/"Voltar" | Texto "Cadastrar" corta na borda do botão; validação só no fim (um erro genérico); sem regras visíveis (tamanho de usuário/senha); barra de status vazia |
| **Seleção** (`UmbraCharacterSelectionWidget`) | Dois títulos ("Seleção de personagem" e "Escolha o personagem"), lista à esquerda em moldura grande, barra inferior larga com "Criar personagem"/"Voltar ao início" | Título duplicado; lista vazia sem mensagem; botões só com o nome; moldura pesada tapa o cenário 3D (os previews ficam atrás) |
| **Botão de personagem** (`UmbraCharacterSelectButtonWidget`) | Botão com o nome ("(CHARNAME) Select" no designer) | Só o nome: sem classe, nível, zona; sem estado selecionado |
| **Detalhes** (`UmbraCharacterItemWidget`) | Coluna alta: nome, nível, zona, classe, facção em 5 faixas iguais; HP/MP; atributos base e de combate; Jogar/Excluir/Voltar; confirmação de exclusão em botões | Hierarquia plana (nome tem o mesmo peso que "zona"); "Health: x/y"/"Mana: x/y" em inglês; "Atributos de combate" estoura a caixa; excluir sem proteção forte (um clique em "Excluir este personagem?"); faixa vazia de status; coluna cobre o personagem 3D |
| **Criação** (`UmbraCreateCharacterWidget`) | Título no topo, faixa de 6 classes **em inglês** (Barbarian, Assassin...), painel de classe (`WBP_ClassSelectorItem`) e painel de nome/aparência (SpinBox Cabelo/Cabeça) | Classes em inglês e só texto; SpinBox cru para aparência; nome sem regra visível; no `WBP_ClassSelectorItem` o 2º bloco "Recursos base" repete o título (é Precisão/Esquiva/Crítico...) e "Atributos de combate" estoura; números soltos sem comparação entre classes |

## 2) Direção visual (vale para as 4 telas)

- **Cartões** no padrão fase 7: fundo escuro translúcido (0,03/0,025/0,02 a ~85%), borda dourada fina, cantos de 6 px. Nada de moldura pesada cobrindo o cenário 3D.
- **Tipografia:** Cinzel nos títulos (`UmbraUIStyle::TitleFont`), Roboto no corpo; rótulos pequenos em caixa alta e dourado; valores em creme.
- **Botão principal** dourado/destacado (Entrar, Cadastrar, Jogar, Criar); secundários discretos (contorno); destrutivo em vermelho só na confirmação.
- **Status** em uma linha abaixo do botão, que só aparece quando há mensagem (cor: erro vermelho, sucesso verde, progresso creme com reticências animadas).
- **Textos em pt-BR** em todas as telas (inclui classes e rótulos que hoje vêm em inglês).
- **Teclado:** Tab entre campos, Enter confirma, Esc volta.

## 3) Lotes

| Lote | Conteúdo | Camadas | Status |
|------|----------|---------|--------|
| **F1** | Login + cadastro (detalhes na §4) | UE (C++ + WBP), config local, guia | **QA ok** (18/11/2026) |
| **F2** | Seleção de personagem: lista em cartões + painel de detalhes compacto (§5) | UE, guia | **QA ok** (18/11/2026) |
| **F3** | Criação de personagem: classes com ícone e nome pt-BR, painel de classe com barras comparativas, nome e aparência (§6) | UE, guia | **QA ok** (18/11/2026) |
| **F4** | Palco dos personagens (seleção e criação): fila lado a lado com todos os personagens da conta, ponto de destaque único onde o escolhido aparece (o mesmo na seleção e na criação), câmera do próprio palco (§10) | UE, mapas, guia | QA ok |
| **F5** | Apresentação no palco: animação por classe + fala diferente na seleção e na criação, configuradas no Manager (`classes.showcase_anim_path`, `selection_voice_path`, `creation_voice_path`) | SQL, PHP, Manager, UE, guia (§5) | Implementado, falta QA |

Ordem: F1 → F2 → F3 (uma por vez, com QA e commit entre elas).

## 4) F1 — Login e cadastro

**Login (`WBP_Login2`)**
- Mantém arte de fundo e logo. Cartão de ~360 px no centro-baixo: rótulos "USUÁRIO"/"SENHA" acima de cada campo; campo com fundo escuro e borda que acende no foco.
- **Entrar** largo (botão principal) e, abaixo, link "Criar conta" discreto.
- **Lembrar usuário** (CheckBox): salva só o nome de usuário em `UUmbraClientSettings` (nunca a senha) e preenche ao abrir; foco vai direto para a senha.
- **Mostrar senha** (ícone de olho ao lado do campo): alterna `IsPassword`.
- **Caps Lock ligado:** aviso pequeno abaixo da senha.
- Status de progresso em etapas ("Conectando...", "Validando sessão...", "Carregando personagens...") usando os eventos que o C++ já escuta (login, TCP, token, lista de personagens); botão desabilitado enquanto processa (já existe `SetLoginEnabled`).
- Rodapé: versão do cliente (do `ProjectVersion` em `DefaultGame.ini`).

**Cadastro (`WBP_Register`)**
- Mesmo cartão; título "Criar conta" em Cinzel.
- Rótulos + **validação por campo, ao digitar**: usuário (tamanho/caracteres), e-mail (formato), senha (mínimo e indicador de força: fraca/média/forte), confirmação ("as senhas não conferem"). Mensagem em vermelho embaixo do campo com problema; **Cadastrar** só habilita com tudo válido.
- Regras lidas das mesmas constantes que o C++ já usa no envio (sem duplicar números).
- Sucesso: mensagem verde e volta ao login com o usuário já preenchido.

**C++:** `UmbraLoginWidget` e `UmbraRegisterWidget` (binds novos opcionais: `CB_RememberUser`, `BTN_ShowPassword`, `Text_CapsLock`, `Text_Version`, `Text_*Error` por campo, `PB_PasswordStrength`); `UmbraClientSettings` (campo `LastUsername`).

## 5) F2 — Seleção de personagem

**Layout:** cenário 3D livre no centro; à **esquerda**, coluna de cartões; à **direita**, painel de detalhes do selecionado; embaixo, "Entrar no jogo" grande.

- **Cartão do personagem (`WBP_CharacterSelectButton`)**: ícone da classe, nome (Cinzel), "Nv 50 · Bárbaro", zona em texto menor. Estado selecionado com borda dourada (`UmbraUI::ApplyListRowStyle`). **Duplo clique = Jogar.** No fim da lista, cartão "+ Criar personagem" (substitui o botão da barra inferior).
- Cabeçalho único "Personagens (N)"; lista vazia mostra "Nenhum personagem ainda" + botão de criar.
- **Painel de detalhes (`WBP_CharacterItem`)**, compacto (~320 px):
  - topo: nome grande, "Nível 50 · Bárbaro", zona e facção numa linha só;
  - barras de HP/MP com texto compacto (mesmo `UmbraUI::ResourceBarText` da HUD) e EXP do nível;
  - atributos em **grade 2 colunas** com ícone, rótulo e valor (base à esquerda, combate à direita), sem caixas por linha;
  - ações: **Jogar** (principal), "Excluir" discreto.
- **Excluir com proteção:** a confirmação pede para **digitar o nome do personagem**; o botão vermelho "Excluir para sempre" só habilita quando o nome confere.
- "Voltar ao início" vira um link pequeno no canto (logout).
- Textos pt-BR ("Vida", "Mana" no lugar de "Health:"/"Mana:").

**C++:** `UmbraCharacterSelectionWidget`, `UmbraCharacterSelectButtonWidget` (binds `Image_ClassIcon`, `Text_Name`, `Text_Subtitle`, `Text_Zone`, `Border_Card`; seleção e duplo clique), `UmbraCharacterItemWidget` (barras `PB_HP/PB_MP/PB_EXP`, `ED_ConfirmName`), helper de nome pt-BR da classe e ícone por `ClassID` (mapa no Class Defaults, como o `Class Icons` do quadro de alvo).

## 6) F3 — Criação de personagem

**Layout:** cenário 3D no centro (preview da classe, câmera já controlada pelo `AUmbraCharacterCreationManager`); **topo**: título "Criar personagem" + passos "1 Classe → 2 Aparência → 3 Nome"; **esquerda**: 6 cartões de classe; **direita**: painel da classe; **baixo**: nome + Criar.

- **Cartões de classe**: ícone + nome **pt-BR** (Bárbaro, Assassino, Alquimista, Mago Sombrio, Marcial, Templário) + papel em uma palavra ("Corpo a corpo", "Furtivo", "Alquimia", "Magia", "Marcial", "Defensor"). Selecionado com borda dourada.
- **Painel da classe (`WBP_ClassSelectorItem`)**: nome em Cinzel, descrição em texto corrido (quebra de linha), e atributos em **barras comparativas** (o máximo entre as 6 classes = barra cheia), agrupados em "Atributos", "Recursos" e "Combate"; corrigir o título duplicado ("Recursos base" → "Atributos secundários": Precisão, Esquiva, Crítico...).
- **Aparência:** setas "‹ Cabelo 2/5 ›" e "‹ Rosto 1/4 ›" no lugar dos SpinBox (o C++ continua usando `SpinBox_Hair/Head` ou os botões novos, chamando o mesmo `HandleAppearanceChanged`).
- **Nome:** regra visível ("3 a 20 letras"), contador, validação ao digitar (já existem `MinNameLength`/`MaxNameLength`); erro do servidor (nome em uso) aparece embaixo do campo.
- **Criar** só habilita com classe + nome válido; "Cancelar" volta à seleção.

**C++:** `UmbraCreateCharacterWidget` (nomes/papéis pt-BR por `ClassID`, setas de aparência, validação ao digitar, estado do botão), `UmbraClassSelectorItemWidget` (barras comparativas, binds `PB_*`), dados da classe vindos do servidor (`FUmbraClassData`, sem tabela local).

## 7) Como o Editor é tocado

- Via MCP, no padrão da fase 7: **cria os containers novos e move (reparent) os widgets que o C++ já usa**, mantendo os nomes; o layout antigo fica **colapsado** (sem `remove_widget`, que já derrubou o editor). Estilo por Python.
- Nada de lógica no Event Graph. Os WBPs continuam com os mesmos caminhos, então nada que os cria precisa mudar.
- Ícones de classe e de atributo: reaproveitar os que já estão nos WBPs; os que faltarem ficam com o usuário (texturas são do usuário).

## 8) Arquivos previstos

| Lote | C++ (submódulo) | WBPs (MCP) |
|------|-----------------|------------|
| F1 | `UI/UmbraLoginWidget.h/.cpp`, `UI/UmbraRegisterWidget.h/.cpp`, `Core/UmbraClientSettings.h/.cpp` | `WBP_Login2`, `WBP_Register` |
| F2 | `UI/UmbraCharacterSelectionWidget.h/.cpp`, `UI/UmbraCharacterSelectButtonWidget.h/.cpp`, `UI/UmbraCharacterItemWidget.h/.cpp` | `WBP_CharacterSelection`, `WBP_CharacterSelectButton`, `WBP_CharacterItem` |
| F3 | `UI/UmbraCreateCharacterWidget.h/.cpp`, `UI/UmbraClassSelectorItemWidget.h/.cpp` | `WBP_CreateCharacter`, `WBP_ClassSelectorItem` |

Docs: `docs_main/GUIA_UI_FASE8_LOGIN_PERSONAGEM_UE561.md` (uma seção por lote, com binds, QA e troubleshooting) e este plano (status). Sem mudança de servidor, protocolo, PHP ou banco.

## 9) QA (resumo; detalhado no guia de cada lote)

- **F1:** login certo/errado; Enter e Tab; lembrar usuário entre sessões (senha nunca salva); mostrar senha; Caps Lock; etapas de status; cadastro com cada regra violada e com sucesso (volta ao login preenchido).
- **F2:** lista com 0, 1 e vários personagens; seleção destacada; duplo clique entra; detalhes com barras e grade; excluir só com o nome digitado certo; logout.
- **F3:** cada classe mostra nome pt-BR, papel e barras coerentes; aparência pelas setas muda o preview; nome curto/longo/inválido/em uso; criar volta à seleção com o novo personagem selecionado.
- Todas: resoluções 1280×720 e 1920×1080, escala da UI 50–125%.

## 10) F4 — Palco dos personagens (seleção e criação)

### Diagnóstico (código lido em 18/11/2026)

| Onde | Hoje | Efeito |
|------|------|--------|
| Seleção: `AUmbraCharacterSelectionManager::SpawnAllCharacterPreviews` | Previews em `FVector(i * 400, 0, 0)`: coordenada **absoluta do mundo**, ignora onde o manager está | Personagens nascem "fora de posição" (na origem do mapa, longe do cenário) |
| Seleção: câmera ao escolher | Vai para `SelectedCameraLocation` fixo `(700, -400, 280)`, igual para qualquer personagem | Com 2+ personagens a câmera enquadra o vazio ou o personagem errado |
| Seleção: ao escolher | **Destrói** os outros previews; trocar exige `ReturnToSelection` (respawna tudo) | Pisca/recria tudo a cada troca |
| Criação: classes | Placeholders `BP_*_Placeholder` **posicionados à mão no mapa**, cada um num lugar; o preview nasce em cima do placeholder | Cada classe aparece numa posição diferente |
| Criação: câmera | `ClassButtonShots` com uma posição/rotação de câmera **absoluta por classe** (valores do BP antigo) | Enquadramento diferente por classe; qualquer mudança no mapa quebra |
| As duas | Câmera achada por nome (`FindCameraActor`: qualquer ator com "CameraActor"/"Camera" no nome); interpolação por timer de 16 ms; código de câmera duplicado nos dois managers | Frágil (depende do nome no mapa) e repetido |

### Proposta

Um **palco** único, reaproveitado pelas duas telas: `AUmbraCharacterStage` (C++, novo), base dos dois managers. Tudo é **relativo ao ator** — no mapa basta posicionar o manager (um gizmo) e, se quiser, ajustar os pontos do palco pelos componentes no viewport.

| Componente (relativo ao manager) | Função | Padrão |
|----------------------------------|--------|--------|
| `ShowcasePoint` (seta) | Onde o personagem **escolhido** fica — o mesmo na seleção e na criação | (0, 0, 0), virado para +X |
| `LineupCenter` (seta) | Centro da **fila lado a lado** | (-120, 0, 0) |
| `OverviewCameraPoint` | Câmera da visão geral (fila inteira) | (520, 0, 160), olhando para a fila |
| `ShowcaseCameraPoint` | Câmera de destaque (corpo inteiro, centralizado entre os painéis da UI) | (300, 0, 110), olhando para o destaque |
| `StageCamera` (`UCameraComponent`) | A câmera do palco (o PC passa a usar o próprio manager como view target) | — |

**Comportamento**

- **Fila lado a lado** (`LayoutLineup`): N atores centralizados no `LineupCenter`, espaçados pelo vetor "direita" (`LineupSpacing` = 130 uu; encolhe se passar de `MaxLineupWidth`). Todos virados para a câmera. A câmera de visão geral **recua sozinha** conforme a largura da fila (`bAutoFrameOverview`).
- **Destaque** (`ShowcaseActor`): o escolhido vai para o `ShowcasePoint` (os outros ficam **escondidos, não destruídos**); voltar devolve cada um ao seu lugar na fila. Trocar de personagem = trocar quem está no destaque (sem respawn).
- **Chão**: trace para baixo a partir do ponto e ajuste pela meia-altura da cápsula (`bSnapToGround`), para os pés ficarem no chão em qualquer mapa.
- **Câmera**: interpolação no `Tick` (smoothstep, `CameraMoveDuration`), sem timer e sem procurar ator pelo nome.
- **Girar o modelo** (opcional, F4b): arrastar o mouse em área vazia gira o personagem do destaque.

**Seleção (`AUmbraCharacterSelectionManager`)**

- Spawna **todos os personagens da conta lado a lado** na fila (relativo ao manager).
- Escolher no cartão → o personagem vai para o destaque e a câmera para `ShowcaseCameraPoint`; "Ver todos" → volta à fila e à câmera de visão geral.
- `UUmbraCharacterSelectionWidget` deixa de chamar `ReturnToSelection` para trocar (só pede o destaque do novo).

**Criação (`AUmbraCharacterCreationManager`)**

- Os 6 placeholders passam a ser **spawnados pelo C++ na mesma fila** (classes `BP_*_Placeholder` já mapeadas no código), em vez de dependerem da posição à mão no mapa.
- Escolher classe → o preview da classe nasce no **mesmo `ShowcasePoint`** da seleção, com a mesma câmera de destaque; trocar de classe só troca o preview (sem `ReturnToClassList`).
- `ClassButtonShots` (câmera por classe) deixa de ser usado; fica só por compatibilidade.

**Mapas** (`Lvl_Character_Selection`, `Lvl_Character_Creation`, via MCP, com o seu OK antes):

- Conferir onde estão o manager, a câmera antiga e os placeholders; posicionar o manager num ponto de chão livre com fundo bonito.
- Placeholders e câmera antiga do mapa: **escondidos** (não apagados) como reserva.
- Os dois mapas usam os mesmos pontos relativos, então o personagem aparece igual nas duas telas.

### Arquivos

| Camada | Arquivos |
|--------|----------|
| C++ (submódulo) | **novo** `Actors/UmbraCharacterStage.h/.cpp`; `Actors/UmbraCharacterSelectionManager.h/.cpp`; `Actors/UmbraCharacterCreationManager.h/.cpp`; `UI/UmbraCharacterSelectionWidget.cpp`; `UI/UmbraCreateCharacterWidget.cpp` |
| Mapas (MCP) | `Lvl_Character_Selection`, `Lvl_Character_Creation` (posição do manager; esconder câmera/placeholders antigos) |
| Docs | guia fase 8 §4 (pontos do palco, como ajustar no viewport, QA) e este plano |

Sem mudança de servidor, PHP ou banco.

### QA (resumo)

- Seleção com 1, 3 e 6 personagens: todos lado a lado, pés no chão, centralizados; câmera enquadra a fila inteira.
- Escolher → personagem no destaque, câmera fecha nele; trocar → troca sem piscar; "Ver todos" → volta à fila.
- Criação: as 6 classes lado a lado; escolher qualquer uma → preview no **mesmo lugar e enquadramento** da seleção; trocar de classe mantém o lugar.
- Mesma posição na tela entre seleção e criação; resoluções 1280×720 e 1920×1080 (personagem entre os painéis).
