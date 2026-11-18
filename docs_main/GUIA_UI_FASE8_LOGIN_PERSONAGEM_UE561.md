# GUIA — Fase 8: login, cadastro, seleção e criação de personagem (UE 5.6.1)

Guia dos lotes da fase 8 ([plano](PLANO_UI_FASE8_LOGIN_PERSONAGEM_UE561.md)). Cada lote acrescenta uma seção. O C++ resolve a lógica; o Blueprint só define parent class e layout.

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (os lotes mudam headers; Live Coding não basta).
2. Servidor e WAMP no ar (login HTTP + TCP 9000 + zone).
3. Nada muda no servidor, no protocolo, no PHP ou no banco nesta fase.

---

## 1) F1 — Login e cadastro

### Objetivo

- **Login:** cartão escuro com borda dourada abaixo do logo; rótulos "USUÁRIO"/"SENHA" acima dos campos; **Entrar** em destaque (dourado) e "Não tem conta? **Criar conta**" como link.
  - **Lembrar usuário:** salva só o nome em `UUmbraClientSettings` (`bRememberUsername`, `RememberedUsername`); a **senha nunca é salva**. Ao abrir, o usuário vem preenchido e o foco vai para a senha.
  - **Mostrar/Ocultar** senha.
  - **Caps Lock ligado:** aviso abaixo da senha (só com a senha em foco).
  - **Enter** no usuário vai para a senha; Enter na senha entra.
  - **Status em etapas:** "Validando conta..." → "Carregando personagens..." → "Conectado ao servidor. Entrando...". A linha de status só aparece quando há mensagem.
  - **Versão** no canto inferior direito (`ProjectVersion` do `DefaultGame.ini`; some se estiver vazio).
- **Cadastro:** mesmo cartão, com **validação por campo enquanto digita** e **Cadastrar** só habilitado com tudo certo.

| Campo | Regra | Mensagem |
|-------|-------|----------|
| Usuário | 3 a 20 caracteres (`MinUsernameLength`/`MaxUsernameLength`), só letras, números e `_` (sem acento/espaço) | "De 3 a 20 caracteres." / "Use só letras, números e _ (sem espaços ou acentos)." |
| E-mail | `nome@dominio.ext`, sem espaços | "E-mail inválido (ex.: nome@dominio.com)." |
| Senha | mínimo de 6 (`MinPasswordLength`) + barra de força (fraca/média/forte) | "Mínimo de 6 caracteres." |
| Confirmar | igual à senha | "As senhas não conferem." |

- Campo vazio só mostra erro depois de tentar cadastrar. Enter avança de campo em campo; no último, cadastra.
- Depois do cadastro, o login abre com o **usuário já preenchido** e o foco na senha.

### Editor (feito via MCP, sem Graph)

Os widgets que o C++ já usa foram **movidos** para os cartões novos (mesmos nomes); os painéis antigos (`Border_545` no login, `Border_Bar` no cadastro) ficaram **colapsados** como reserva.

**`/Game/Widgets/WBP_Login2`** (parent `UmbraLoginWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `Border_LoginCard` | `UBorder` | Cartão (âncora 0,5 × 0,36) |
| `TXT_Username` / `TXT_Password` | `UEditableText` | Dentro de `Border_UserField` / `Border_PassField` |
| `BTN_ShowPassword` + `Text_ShowPassword` | botão + texto | Mostrar/Ocultar senha (novo) |
| `Text_CapsLock` | `UTextBlock` | Aviso de Caps Lock (novo) |
| `CB_RememberUser` | `UCheckBox` | Lembrar usuário (novo) |
| `btn_login` | `UButton` | Entrar (principal) |
| `Border_Status` + `TXT_Status` | borda + texto | Status (some sem mensagem) |
| `btn_registerPAGE` | `UButton` | Link "Criar conta" |
| `Text_Version` | `UTextBlock` | Versão (canto inferior direito, novo) |

**`/Game/Widgets/WBP_Register`** (parent `UmbraRegisterWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `Border_RegisterCard` | `UBorder` | Cartão (âncora 0,5 × 0,34) |
| `TXT_Username`, `TXT_Email`, `TXT_Password`, `TXT_ConfirmPassword` | `UEditableText` | Dentro de `Border_Reg*Field` |
| `Text_UsernameError`, `Text_EmailError`, `Text_PasswordError`, `Text_ConfirmError` | `UTextBlock` | Erro de cada campo (novo) |
| `PB_PasswordStrength` + `Text_PasswordStrength` | barra + texto | Força da senha (novo) |
| `BTNRegister` | `UButton` | Cadastrar (principal) |
| `Border_Status` + `TXT_Status` | borda + texto | Status (some sem mensagem) |
| `BTNBackLogin` | `UButton` | Link "Já tem conta? Entrar" |

### O que o C++ já faz

- `UUmbraLoginWidget`: foco inicial, lembrar usuário (`SaveRememberedUsername` ao clicar em Entrar), mostrar senha (`SetIsPassword`), Caps Lock (`FSlateApplication::GetModifierKeys`, checado a cada 0,15 s), versão, status em etapas, `PrefillUsername` (usado pelo cadastro). O fluxo protegido (HTTP → token → personagens → TCP → validação) continua todo no GameInstance.
- `UUmbraRegisterWidget`: `RefreshValidation` a cada tecla, força da senha, Enter por campo, volta ao login com o usuário preenchido.
- Com o WBP antigo (sem os binds novos), as duas telas continuam funcionando como antes.

### QA (PIE)

| # | Teste | Esperado |
|---|-------|----------|
| 1 | Abrir o jogo | Cartão novo abaixo do logo; foco no usuário; sem barra de status vazia; versão no canto |
| 2 | Entrar com usuário/senha certos | Status "Validando conta..." → "Carregando personagens..." → seleção de personagem |
| 3 | Senha errada | Mensagem do servidor em vermelho; Entrar volta a habilitar |
| 4 | Campos vazios + Entrar | "Preencha o usuário e a senha." e foco no campo vazio |
| 5 | Enter no usuário / na senha | Vai para a senha / entra |
| 6 | Mostrar senha | Texto da senha aparece; "Ocultar" volta a esconder |
| 7 | Caps Lock ligado com a senha em foco | "Caps Lock ligado" aparece; some ao desligar ou sair do campo |
| 8 | Lembrar usuário marcado → entrar → sair do jogo → abrir de novo | Usuário preenchido, checkbox marcado, foco na senha; **senha vazia** |
| 9 | Desmarcar e entrar | Na próxima abertura o usuário vem vazio |
| 10 | Criar conta → digitar "ab" / "nome com espaço" / "joão" | Erro do usuário abaixo do campo; Cadastrar desabilitado |
| 11 | E-mail "teste@" / "teste.com" | "E-mail inválido..." |
| 12 | Senha "abc" → "abcdef" → "Abcdef12!" | Erro de mínimo; barra fraca → média/forte |
| 13 | Confirmação diferente | "As senhas não conferem." |
| 14 | Tudo certo + Enter na confirmação | Conta criada (verde) e volta ao login com o usuário preenchido |
| 15 | Usuário já existente | Mensagem do servidor no status; Cadastrar volta a habilitar |
| 16 | Resolução 1280×720 e 1920×1080 | Cartão centralizado abaixo do logo, sem cortar |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Tela antiga aparece por cima | `Border_545` / `Border_Bar` visível | Deixar **Collapsed** |
| Lembrar usuário não funciona | Rebuild não feito (campos novos no `UmbraClientSettings`) | Rebuild do módulo |
| Versão não aparece | `ProjectVersion` vazio | **Project Settings → Description → Project Version** |
| Cadastrar nunca habilita | Algum campo com erro (mesmo invisível) | Conferir as mensagens abaixo dos campos |

---

## 2) F2 — Seleção de personagem

### Objetivo

- **Lista à esquerda, detalhes à direita, cenário 3D livre no meio.** A lista continua visível depois de escolher um personagem (antes os detalhes substituíam a lista).
- **Cartão de personagem:** ícone da classe, nome (Cinzel), "Nv 50 · Bárbaro" e a zona. O escolhido fica destacado (estilo de lista da UI). **Dois cliques seguidos** no cartão entram no jogo.
- Título "Personagens (N)"; sem personagens, "Nenhum personagem ainda. Crie o primeiro!". **+ Criar personagem** e **Sair da conta** ficam no próprio cartão da lista (a barra inferior antiga sumiu).
- **Detalhes compactos:** nome, "Nível 50 · Bárbaro", "Zona · Facção"; barras de **Vida/Mana/EXP** com texto compacto; atributos em duas colunas (ícone, nome, valor); **Jogar** em destaque; "Excluir personagem" e "Ver todos" como links.
- **Excluir com proteção:** abre um campo para **digitar o nome do personagem**; **Excluir para sempre** só habilita com o nome certo (maiúsculas/minúsculas não importam).
- Nomes de classe em pt-BR: 1 Bárbaro, 2 Templário, 3 Mago Sombrio, 4 Alquimista, 5 Assassino, 6 Marcial (`UmbraUI::ClassDisplayName`).
- Trocar de personagem com outro já escolhido volta à visão geral (os outros previews voltam) e escolhe o novo.

### Editor (feito via MCP, sem Graph)

Widgets antigos colapsados como reserva: `Border_149` (cartão), `Border_338`/`Border_126`/`Border_142` (seleção), `Border_62` (detalhes).

**`/Game/Widgets/WBP_CharacterSelectButton`** (parent `UmbraCharacterSelectButtonWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `SB_Card` > `BTN_Select` > `HB_Card` | caixa + botão | Cartão clicável (268 px) |
| `Image_ClassIcon` | `UImage` | Ícone da classe (some sem textura) |
| `Text_Name` / `Text_Subtitle` / `Text_Zone` | `UTextBlock` | Nome / "Nv · Classe" / zona |
| `TextBTN_Select` | `UTextBlock` | Legado (fica no painel antigo colapsado) |

**`/Game/Widgets/WBP_CharacterSelection`** (parent `UmbraCharacterSelectionWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `Border_ListCard` | `UBorder` | Cartão da lista (esquerda, centro vertical) |
| `Text_ListTitle` | `UTextBlock` | "Personagens (N)" |
| `ScrollBox_Characters` > `VBox_CharacterList` | rolagem + caixa | Cartões (altura máxima 430) |
| `TXT_Status`, `BTN_CreateNew`, `BTN_Logout` | — | Movidos para o cartão |
| `Panel_Details` > `VB_Details` | borda + caixa | Painel de detalhes (direita); some sem personagem escolhido |

**`/Game/Widgets/WBP_CharacterItem`** (parent `UmbraCharacterItemWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `Image_ClassIcon`, `TXT_Name`, `Text_Subtitle`, `Text_Location` | — | Cabeçalho |
| `PB_HP`+`TXT_Health`, `PB_MP`+`TXT_Mana`, `PB_EXP`+`Text_Exp` | barras + texto | Vida, mana, EXP |
| `HB_Row_*` (STR, DEX, INT, VIT, LUCK, PATK, MATK, PDEF, MDEF) | linhas | Ícone + rótulo + `TXT_*` (valor) |
| `BTN_Play` | `UButton` | Jogar (principal) |
| `BTN_Delete`, `BTN_Return` | `UButton` | Links "Excluir personagem" / "Ver todos" |
| `Panel_DeleteConfirm` | `UVerticalBox` | `ED_ConfirmName` (`UEditableTextBox`) + `DeletePanel` (`BTN_Delete_1`, `BTN_Cancel`) |
| `txt_status` | `UTextBlock` | Aviso da exclusão / erro |

### Class Defaults

| Asset | Campo | Valor |
|-------|-------|-------|
| `WBP_CharacterSelection` | **Umbra\|CharacterSelection → Class Icons** | ClassID → textura (copiado do `WBP_TargetFramePlayer` pelo script `Saved/MCP_f2_defaults.py`, depois do rebuild) |
| `WBP_CharacterItem` | **Class Icons** | Idem |

### O que o C++ já faz

- `UUmbraCharacterSelectionWidget`: lista + detalhes separados (`VB_Details`/`Panel_Details`), destaque do escolhido, duplo clique (`HandleCharacterDoubleClicked` → `UUmbraCharacterItemWidget::RequestPlay`), troca de personagem com `ReturnToSelection` antes, título com a contagem.
- `UUmbraCharacterSelectButtonWidget`: cartão (nome, nível, classe pt-BR, zona, ícone), estilo de lista, detecção de dois cliques (`DoubleClickSeconds` = 0,35 s).
- `UUmbraCharacterItemWidget`: textos pt-BR, barras, números compactos, confirmação por nome. Com o WBP antigo (sem `Text_Subtitle`), mantém "Rótulo: valor".

### QA (PIE)

| # | Teste | Esperado |
|---|-------|----------|
| 17 | Entrar com conta que tem 2+ personagens | Cartão da lista à esquerda com "Personagens (N)" e um cartão por personagem (nome, "Nv · Classe", zona) |
| 18 | Clicar num personagem | Cartão destacado; câmera vai ao preview; detalhes à direita com barras e atributos |
| 19 | Clicar em outro personagem | Volta à visão geral e mostra o novo (sem erro de preview) |
| 20 | Dois cliques rápidos num cartão | Entra no jogo com aquele personagem |
| 21 | **Jogar** | Entra no jogo |
| 22 | **Ver todos** | Volta à visão geral; detalhes somem |
| 23 | **Excluir personagem** → digitar nome errado / certo | Botão vermelho desabilitado / habilitado; **Cancelar** esconde o bloco |
| 24 | Excluir para sempre | "Excluindo..." e a lista recarrega sem o personagem |
| 25 | Conta sem personagens | "Nenhum personagem ainda..." e **+ Criar personagem** |
| 26 | **Sair da conta** | Volta ao login |
| 27 | Classes | Nomes em pt-BR (Bárbaro, Templário, Mago Sombrio, Alquimista, Assassino, Marcial); ícone quando houver textura |
| 28 | 1280×720 e 1920×1080 | Lista e detalhes nas laterais, sem cobrir o personagem no centro |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Detalhes substituem a lista | `VB_Details` ausente / rebuild não feito | Rebuild; conferir o WBP |
| "Preview não encontrado" ao trocar de personagem | C++ antigo | Rebuild do módulo |
| Quadrado branco no lugar do ícone | Textura não configurada | Rodar o script de ícones ou preencher **Class Icons** |
| Excluir para sempre nunca habilita | Nome digitado diferente | Conferir o nome (espaços nas pontas são ignorados) |

---

## 3) F3 — Criação de personagem

### Objetivo

- **Título** "Criar personagem" com os passos "1 Classe → 2 Aparência → 3 Nome".
- **Classes à esquerda**, sempre visíveis: cartões com o nome em pt-BR (Cinzel) e o papel. A escolhida fica destacada; **dá para trocar de classe direto** (o C++ volta à visão das classes antes, porque o manager destrói os placeholders ao escolher).

| Botão | ClassID | Nome | Papel |
|-------|---------|------|-------|
| `BTN_Class1` | 1 | Bárbaro | Corpo a corpo |
| `BTN_Class2` | 5 | Assassino | Furtivo |
| `BTN_Class3` | 4 | Alquimista | Alquimia |
| `BTN_Class4` | 3 | Mago Sombrio | Magia |
| `BTN_Class5` | 2 | Templário | Defensor |
| `BTN_Class6` | 6 | Marcial | Artes marciais |

- **Antes de escolher:** dica à direita "Escolha uma classe à esquerda...".
- **Painel da classe (direita):** nome, papel, descrição e atributos com **barras comparativas** (a maior entre as 6 classes = barra cheia) em três grupos (Atributos, Recursos, Combate) + "Secundários" (Precisão, Esquiva, Crítico, Movimento, Ataque duplo, Res. crítico, Res. ataque duplo). Corrige o título repetido "Recursos base" e os rótulos em inglês.
- **Cartão de baixo:** nome com contador "5 / 20" e validação ao digitar (as mesmas regras do `create_character.php`: 3 a 20, letras/números/_); **Aparência** com setas ‹ › ("Estilo 2 (2 / 5)" ou "Padrão"), usando as peças do servidor (`AvailableAppearanceParts`) válidas para a classe; **Voltar** (visão das classes) e **Criar personagem** (só habilita com classe + nome válido). Erro do servidor (ex.: nome em uso) aparece abaixo do nome.

### Editor (feito via MCP, sem Graph)

Antigos colapsados: `Border_557` (título), `Border_BTN_Classes` (barra das classes), `VB_Holder` (guarda `Border_801` e os textos `Cancel_1..6`), `Border_107` (painel antigo da classe).

**`/Game/Widgets/WBP_CreateCharacter`** (parent `UmbraCreateCharacterWidget`)

| Nome | Tipo | Função |
|------|------|--------|
| `VB_CreateTitle` (`Text_CreateTitle`, `Text_CreateSteps`) | caixa + textos | Título e passos (topo) |
| `Border_ClassListCard` > `VB_ClassCards` | cartão | `BTN_Class1..6` + `BTN_Cancel` (link "Cancelar") |
| `Text_ClassName_N` / `Text_ClassRole_N` (N = 1..6) | `UTextBlock` | Dentro de cada `BTN_ClassN` (`VB_ClassCard_N`); o C++ preenche pelo nome |
| `Panel_ChooseHint` | `UBorder` | Dica antes de escolher |
| `Border_WBP_Creator` > … > `VBox_ClassList` | cartão | Painel da classe (direita); o C++ mostra ao escolher |
| `Border_CreationPanel` > `VB_CreateCard` | cartão | Nome, aparência e botões (baixo); o C++ mostra ao escolher |
| `TXT_Name` + `Text_NameCounter` | campo + contador | Nome |
| `TXT_Validation` | `UTextBlock` | Erro/ok do nome e do servidor |
| `BTN_HairPrev`/`BTN_HairNext` + `Text_HairValue` | setas + texto | Cabelo |
| `BTN_HeadPrev`/`BTN_HeadNext` + `Text_HeadValue` | setas + texto | Rosto |
| `BTN_Return`, `BTN_Create` | `UButton` | Voltar / Criar personagem |
| `SpinBox_Hair`, `SpinBox_Head` | `USpinBox` | Legado (dentro do `VB_Holder`); sem as setas, o C++ usa eles |

**`/Game/Widgets/UI/ClassSelectorItem/WBP_ClassSelectorItem`** (parent `UmbraClassSelectorItemWidget`)

| Nome | Função |
|------|--------|
| `TXT_ClassName`, `Text_ClassRole`, `TXT_ClassDescription` | Cabeçalho |
| `HB_Bar_*` (STR, DEX, INT, VIT, LUCK, HP, MP, PATK, MATK, PDEF, MDEF) | Rótulo + `PB_*` + valor (`TXT_*` dentro de `SB_Val_*`) |
| `HB_Sec_*` | Secundários: rótulo + `TXT_Accuracy`, `TXT_Dodge`, `TXT_CritAtk`, `TXT_BaseMovement`, `TXT_DoubleAtk`, `TXT_CritRes`, `TXT_DoubleRes` |

### O que o C++ já faz

- `UUmbraCreateCharacterWidget`: rótulos pt-BR dos cartões (`ApplyClassCardLabels`), destaque (`RefreshClassCardHighlight`), troca de classe com `ReturnToClassList`, `ValidateName`, contador, `RefreshCreateEnabled`, aparência por setas (`GetAppearanceOptions` = 0 + `PartId` do tipo "hair"/"head" da classe ou global). Com o WBP antigo (sem as setas), o fluxo antigo continua (botões somem ao escolher, SpinBox).
- `UUmbraClassSelectorItemWidget::ApplyClassDataCompared`: nome/papel pt-BR, números compactos, barras (`valor / maior entre as classes`, mínimo 8%).

### QA (PIE)

| # | Teste | Esperado |
|---|-------|----------|
| 29 | Abrir a criação | Título, 6 cartões em pt-BR à esquerda, dica à direita; nada de "Barbarian/Monk/Cleric" |
| 30 | Escolher Bárbaro | Cartão destacado; câmera vai ao preview; painel da classe com barras; cartão de nome/aparência embaixo |
| 31 | Trocar para Mago Sombrio sem voltar | Preview e painel trocam; barras mudam (comparação entre classes) |
| 32 | Setas de Cabelo/Rosto | Preview muda; texto "Estilo N (i / total)" ou "Padrão" |
| 33 | Nome "ab" / "nome com espaço" / "João" / 21+ letras | Erro abaixo do nome; **Criar** desabilitado; contador em vermelho acima de 20 |
| 34 | Nome válido | "Nome válido." verde; **Criar** habilita |
| 35 | Nome já existente | Mensagem do servidor abaixo do nome; botão volta a habilitar |
| 36 | Criar com sucesso | "Personagem criado com sucesso!" e volta à seleção com o novo personagem |
| 37 | **Voltar** | Visão das classes; painéis somem; dica volta |
| 38 | **Cancelar** | Volta à seleção |
| 39 | 1280×720 e 1920×1080 | Classes à esquerda, painel à direita, cartão embaixo, sem cobrir o preview |

### Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Classes somem ao escolher | Rebuild não feito (C++ antigo) | Rebuild do módulo |
| Cartão diz "Barbarian" | Rebuild não feito | Rebuild |
| Setas não mudam o preview | Sem peças de aparência para a classe | Conferir `appearance_parts` no Manager; "Padrão" é a única opção |
| Barras todas cheias | Classes ainda carregando | Reabrir a classe depois do `OnClassesLoaded` |
