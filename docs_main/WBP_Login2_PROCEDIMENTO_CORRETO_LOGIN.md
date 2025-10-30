## WBP_Login2 — Procedimento Completo e Detalhado (Login Correto, sem falsos sucessos)

Este guia descreve exatamente quais nós adicionar, remover e como conectar no Blueprint `WBP_Login2` para que:

- O botão de login só navegue para a seleção de personagens após o resultado real do HTTP (success/failure).
- Mensagens de status sejam corretas ("Logging in…", erro de credenciais, etc.).
- O botão seja desabilitado durante a requisição e reabilitado quando terminar.
- Não haja triggers indevidos de sucesso ao digitar nos campos.

Terminologia usada:
- `MyGameInstance`: variável do tipo `UmbraGameInstance` dentro do `WBP_Login2`.
- `BTN_Login`: botão de login.
- `TXT_Username`, `TXT_Password`: campos de texto.
- `TXT_Status`: `TextBlock` para mensagens ao usuário.
- Delegate no `UmbraGameInstance`: `OnLoginCompleted(bool bSuccess, FString Message)`.

### 1) Limpeza do que causa o falso sucesso

Remova/Desconecte completamente:
- Qualquer execução saindo do pino `then` do nó `MyGameInstance.LoginUser(...)`. NÃO use o pino `then` para acionar navegação/troca de tela. Deixe-o sem nada conectado.
- Qualquer lógica de navegação (criar/abrir `WBP_CharacterSelection`) conectada a:
  - OnChanged/OnTextCommitted de `TXT_Username`/`TXT_Password`.
  - Qualquer outro evento que não seja o delegate `OnLoginCompleted`.
- Qualquer chamada direta a funções de pós-login (abrir seleção, iniciar TCP, carregar lista) fora do handler do `OnLoginCompleted`.

Checklist de nós a remover (se existirem):
- [ ] Execuções que saem do `then` de `LoginUser` indo para criar widget de seleção.
- [ ] Execuções ligadas a eventos de digitação/commit dos campos de texto que navegam para outra tela.
- [ ] Bind múltiplo duplicado do `OnLoginCompleted` (ver seção Unbind abaixo).

### 2) Event Construct — Preparação e Bind do Delegate

Adicione os nós, nesta ordem:
1. `Event Construct`
2. `Get Game Instance`
3. `Cast To UmbraGameInstance`
4. `Set MyGameInstance` (guarde a referência)
5. Escolha UMA das opções de delegate abaixo (A ou B):
   - A) Se existir `OnLoginCompleted(bool bSuccess, FString Message)` no `UmbraGameInstance`:
     - `Create Event` → `OnLoginCompleted_Event(Success: bool, Message: string)`
     - `Bind Event to OnLoginCompleted` em `MyGameInstance` apontando para `OnLoginCompleted_Event`.
   - B) Se NÃO existir `OnLoginCompleted`, mas existirem `OnLoginSuccess` e `OnLoginFailed` (padrão atual):
     - `Create Event` → `OnLoginSuccess_Event()`
     - `Bind Event to OnLoginSuccess` em `MyGameInstance` apontando para `OnLoginSuccess_Event`.
     - `Create Event` → `OnLoginFailed_Event(Message: string)` (se o delegate falho tiver mensagem; se não tiver, crie sem parâmetro)
     - `Bind Event to OnLoginFailed` em `MyGameInstance` apontando para `OnLoginFailed_Event`.

Conexões:
- Opção A: `Event Construct` → `Get Game Instance` → `Cast To UmbraGameInstance` → `Set MyGameInstance` → `Bind Event to OnLoginCompleted`.
- Opção B: `Event Construct` → `Get Game Instance` → `Cast To UmbraGameInstance` → `Set MyGameInstance` → `Bind Event to OnLoginSuccess` e `Bind Event to OnLoginFailed`.

Observações:
- Certifique-se de que o bind acontece UMA única vez por criação do widget. Não faça binds no `OnClicked`.
- Opcional: No `Event Destruct`, faça `Unbind` (ver Seção 6) para evitar binds acumulados ao reabrir a tela.

### 3) BTN_Login.OnClicked — Validação, UI de carregamento, chamada LoginUser

Objetivo: validar campos, atualizar status, desabilitar botão e iniciar login sem acionar sucesso imediato.

Nós a adicionar e conectar:
1. `Event OnClicked (BTN_Login)`
2. `TXT_Username.Text` → `Conv_TextToString` → `LocalUsername (Set)`
3. `TXT_Password.Text` → `Conv_TextToString` → `LocalPassword (Set)`
4. `IsEmpty(LocalUsername)` → `Not` (A)
5. `IsEmpty(LocalPassword)` → `Not` (B)
6. `AND` entre (A) e (B)
7. `Branch` com `Condition = AND(A,B)`
   - False:
     - `TXT_Status.SetText("Preencha usuário e senha.")`
     - `TXT_Status.SetColorAndOpacity(VERMELHO)`
     - (Fim)
   - True:
     - `TXT_Status.SetText("Logging in...")`
     - `TXT_Status.SetColorAndOpacity(AMARELO)`
     - `BTN_Login.SetIsEnabled(false)`
     - `MyGameInstance.LoginUser(LocalUsername, LocalPassword)`
     - NÃO conectar nada ao `then` de `LoginUser`.

Resumo das conexões do fluxo True:
`Branch(True)` → `TXT_Status.SetText("Logging in...")` → `TXT_Status.SetColorAndOpacity(AMARELO)` → `BTN_Login.SetIsEnabled(false)` → `MyGameInstance.LoginUser(...)` (sem saída conectada).

### 4) Tratamento do resultado — ÚNICO ponto que trata sucesso/falha

Use conforme a opção escolhida no passo 2:

Opção A — `OnLoginCompleted` (recomendado quando disponível):
- Crie `OnLoginCompleted_Event(Success: bool, Message: string)`
- Conecte os nós:
  1. Entrada do `OnLoginCompleted_Event`
  2. `BTN_Login.SetIsEnabled(true)`
  3. `Branch` com `Condition = Success`
     - True (SUCESSO):
       - `TXT_Status.SetText("Login OK")`
       - `TXT_Status.SetColorAndOpacity(VERDE)`
       - Criar/abrir `WBP_CharacterSelection` e adicionar ao viewport
       - Remover `WBP_Login2` do viewport
     - False (FALHA):
       - `TXT_Status.SetText(Message)`
       - `TXT_Status.SetColorAndOpacity(VERMELHO)`
       - Permanecer na tela; opcional limpar senha

Opção B — `OnLoginSuccess` e `OnLoginFailed` (layout atual comum):
- `OnLoginSuccess_Event()`
  - `BTN_Login.SetIsEnabled(true)`
  - `TXT_Status.SetText("Login OK")`
  - `TXT_Status.SetColorAndOpacity(VERDE)`
  - Criar/abrir `WBP_CharacterSelection` → Remover `WBP_Login2`
- `OnLoginFailed_Event(Message: string)` (ou sem parâmetro, se o delegate não tiver):
  - `BTN_Login.SetIsEnabled(true)`
  - `TXT_Status.SetText(Message ou "Usuário ou senha inválidos.")`
  - `TXT_Status.SetColorAndOpacity(VERMELHO)`
  - Permanecer na tela; opcional limpar senha

Importante:
- Toda a navegação pós-login deve acontecer SOMENTE aqui: na Opção A, ramo True do `OnLoginCompleted`; na Opção B, dentro do handler `OnLoginSuccess`.

### Apêndice — Como bindar DOIS delegates com uma única saída (Then)

Você tem duas formas simples; ambas funcionam sem Branch/condição:

1) Encadeando nós (sem Sequence):
- `Set MyGameInstance` (Then) → `Bind Event to OnLoginSuccess`
- Saída (Then) do `Bind Event to OnLoginSuccess` → `Bind Event to OnLoginFailed`

2) Usando o nó Sequence (organização visual):
- Como criar: clique direito no grafo → procure por "Sequence" e adicione o nó `Sequence` (ele possui entradas/saídas de execução "Then 0", "Then 1", ...).
- Ligue assim:
  - `Set MyGameInstance` (Then) → `Sequence` (Input)
  - `Sequence.Then 0` → `Bind Event to OnLoginSuccess`
  - `Sequence.Then 1` → `Bind Event to OnLoginFailed`

Notas cruciais:
- Não use `Branch`/`IsAuthenticated` para decidir os binds; os binds devem ocorrer sempre no `Construct` para esta instância do widget.
- Nunca conecte a saída (Then) de um `Bind Event ...` de volta para o `Construct` ou para o próprio delegate (evita loop).
- Se o widget for recriado com frequência, faça `Unbind/Remove` no `Event Destruct` para evitar binds duplicados.

## Fluxo recomendado (TCP ativo) — Consumir OnCharacterListLoaded no WBP_Login2 e injetar dados no WBP_CharacterSelection

Objetivo: evitar race/duplicidade de binds e garantir que a seleção receba a lista pronta.

### A) WBP_Login2

1) Event Construct (binds, sem Branch):
- Get Game Instance → Cast to `UmbraGameInstance` → `Set MyGameInstance`
- Then → `Bind Event to OnLoginFailed` → `OnLoginFailed_Event(Message)`
- Then → `Bind Event to OnLoginSuccess` → `OnLoginSuccess_Event()`
- Then → `Bind Event to OnCharacterListLoaded` → `OnCharacterListLoaded_Event()`

2) BTN_Login.OnClicked:
- Validar campos → `TXT_Status = "Logging in..."` (amarelo)
- `BTN_Login.SetIsEnabled(false)`
- `MyGameInstance.LoginUser(Username, Password)` (não usar o Then)

3) OnLoginFailed_Event(Message):
- `BTN_Login.SetIsEnabled(true)`
- `TXT_Status = Message` (vermelho)

4) OnLoginSuccess_Event():
- `TXT_Status = "Login OK"` (verde)
- Não navegue aqui com TCP ativo; a lista vem primeiro.

5) OnCharacterListLoaded_Event():
- `RemoveFromParent` (fechar `WBP_Login2`)
- `CreateWidget(WBP_CharacterSelection)` → `AddToViewport`
- Imediatamente após criar, chame no widget criado a função pública `InitializeWithPlayers()` (descrita abaixo)

Nós exatos para a criação e chamada:
- `GameplayStatics.GetPlayerController` (PlayerIndex=0) → OwningPlayer
- `CreateWidget` (Class=`WBP_CharacterSelection`) com OwningPlayer conectado
- `AddToViewport` no ReturnValue do `CreateWidget`
- Do ReturnValue → `Call Function` `InitializeWithPlayers` (do próprio widget)

### B) WBP_CharacterSelection — Função pública para popular a lista a partir do GameInstance

Crie uma função pública no Blueprint (não é Evento, é Function): `InitializeWithPlayers()`

Passo-a-passo dos nós dentro desta função:
1. `Get Game Instance`
2. `Cast To UmbraGameInstance`
3. `Get MyGameInstance` (opcional; você pode usar o cast diretamente)
4. Do cast: `Get CurrentPlayers` (array)
5. `ClearChildren` no `ScrollBox`/`VerticalBox` que lista personagens (ex.: `VB_List`)
6. `ForEachLoop` no array `CurrentPlayers`
   - `CreateWidget` (Class=`WBP_CharacterItem`)
   - Preencher o item:
     - `SetCharacterId`/`SetName`/`SetLevel` (ou variáveis expostas do item)
     - Se o item tiver evento/Binding para seleção, também setar o `PlayerID`
   - `AddChild` no container (`VB_List.AddChild`)

Observações específicas:
- O tipo `CurrentPlayers` vem da `UmbraGameInstance`. Você não precisa passar parâmetros; a função lê direto do GI, garantindo que pega o estado mais recente.
- Se seu item for `WBP_CharacterItem`, exponha as variáveis necessárias como `Editable/Expose on Spawn` para poder setar via `CreateWidget`.

### C) O que remover do WBP_CharacterSelection

- Remova binds ao delegate `OnCharacterListLoaded` nesta tela.
- Remova chamadas a `PopulateCharacterList` no `Event Construct`.
- Toda a população deve acontecer apenas via `InitializeWithPlayers()` quando a tela for criada pelo `WBP_Login2`.

### D) Alternativa (se preferir bindar no WBP_CharacterSelection)

- Se optar por manter o bind à `OnCharacterListLoaded` no `WBP_CharacterSelection`, então crie o `WBP_CharacterSelection` ANTES de chamar `LoadCharacterList`, para ele já estar ouvindo.
- Neste caso, o `WBP_Login2` NÃO deve bindar `OnCharacterListLoaded` para evitar duplicidade. A criação da tela deve acontecer no `OnLoginSuccess` (antes de `LoadCharacterList`). Este fluxo não é recomendado com seu C++ atual, mas é viável.

### 5) Requisitos no UmbraGameInstance (para compatibilidade)

Garanta que o `UmbraGameInstance` faça o broadcast do delegate apenas quando a requisição HTTP terminar:
- Em caso de resposta HTTP 200 com `success=true` no JSON: `Broadcast OnLoginCompleted(true, "Login bem-sucedido!")`.
- Em caso de `success=false` (credenciais inválidas) ou HTTP 4xx/5xx: `Broadcast OnLoginCompleted(false, "Usuário ou senha inválidos." ou mensagem vinda do backend)`.

Sinais de que está correto:
- Ao clicar em login inválido, o texto muda para vermelho com a mensagem e o botão volta a habilitar.
- Ao clicar em login válido, a navegação só ocorre após o retorno da API (mensagem verde e troca de tela).

### 6) Evitar binds duplicados — Unbind no Destruct (recomendado)

Se o widget de login puder ser recriado diversas vezes, evite múltiplos binds acumulando:
1. `Event Destruct`
2. `Unbind All Events` (ou `Remove All` do `OnLoginCompleted` em `MyGameInstance`) OU `Unbind from OnLoginCompleted` se estiver disponível como nó específico.

Fluxo sugerido:
- `Event Destruct` → `IsValid(MyGameInstance)` → `Unbind/Remove from OnLoginCompleted`.

### 7) Paleta de cores sugerida (opcional)

- VERMELHO (erro): RGBA(1.0, 0.2, 0.2, 1.0)
- AMARELO (processando): RGBA(1.0, 0.85, 0.2, 1.0)
- VERDE (sucesso): RGBA(0.2, 1.0, 0.4, 1.0)

### 8) Checklist final

- [ ] `LoginUser` não possui nada conectado ao pino `then`.
- [ ] `OnLoginCompleted` está bindado no `Event Construct` (uma única vez).
- [ ] `BTN_Login` é desabilitado antes da chamada e reabilitado no `OnLoginCompleted`.
- [ ] Mensagens e cores atualizam corretamente em sucesso/falha.
- [ ] Navegação para `WBP_CharacterSelection` ocorre apenas no ramo True do `OnLoginCompleted`.
- [ ] Não há triggers vindos de eventos de digitação dos campos.
- [ ] (Opcional) `Unbind` no `Event Destruct` para evitar binds duplicados.

### 9) Problemas comuns e correções rápidas

- Problema: "Logging in…" fica para sempre.
  - Verifique se o `OnLoginCompleted` está realmente sendo broadcastado no `UmbraGameInstance` em todas as saídas possíveis (erro e sucesso) e se o bind no `Event Construct` ocorreu.

- Problema: Vai para seleção mesmo com senha errada.
  - Existe lógica de navegação conectada ao `then` de `LoginUser` ou a outro evento. Remova. A navegação só pode estar no `OnLoginCompleted(True)`.

- Problema: Evento dispara múltiplas vezes.
  - Há binds repetidos. Garanta bind único por instância e faça `Unbind` no `Destruct`.

---

Se desejar, posso anexar um diagrama em imagem com os nós e conexões na ordem descrita acima. Mas seguindo esta lista de nós/ordem/ramificações, o comportamento ficará correto e previsível.


