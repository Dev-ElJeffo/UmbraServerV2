# Guia: Configurar Botões do WBP_ContextMenu

O menu de contexto (WBP_PlayerContextMenu ou WBP_ContextMenu) exibe as opções sociais ao clicar com o botão direito em um jogador remoto. As opções **follow**, **add friend**, **trade**, **invite**, **whisper**, **inspect**, **duel**, **report** e **block** precisam estar conectadas para executar as ações.

## Fluxo Atual (C++)

1. **PlayerController** faz binding de `OnPlayerContextActionSelected` → `HandlePlayerContextAction` no GameInstance
2. Quando o Blueprint chama `ExecuteContextAction(Action)` no `PlayerSelectionComponent`, o delegate dispara e o GameInstance executa a ação correta

## O que fazer no Blueprint WBP_PlayerContextMenu

Cada botão deve chamar `Execute Context Action` no **OnClicked** com o enum correto.

### Passo a passo para cada botão

1. Abra o **WBP_PlayerContextMenu** no Editor
2. Para cada botão (follow, add friend, trade, invite, whisper, inspect, duel, report, block):

   **No evento OnClicked do botão:**

   ```
   [BTN_XX OnClicked]
       │
       ├──► Get Player Controller
       │         │
       │         └──► Get Player Selection Component
       │                   │
       │                   └──► Execute Context Action
       │                             Action = [enum correto - ver tabela abaixo]
       │
       └──► Remove From Parent  (ou Set Visibility Collapsed, para fechar o menu)
   ```

### Tabela de mapeamento Botão → Enum

| Botão no menu | Enum EUmbraPlayerContextAction |
|---------------|--------------------------------|
| follow        | Follow                         |
| add friend    | AddFriend                      |
| trade         | Trade                          |
| invite        | InviteToParty                  |
| whisper       | Whisper                        |
| inspect       | Inspect                        |
| duel          | Duel                           |
| report        | Report                         |
| block         | Block                          |

### Exemplo visual (Blueprint)

**BTN_Trade OnClicked:**
- Get Player Controller (0) → Get Player Selection Component
- Execute Context Action (Action = **Trade**)
- Remove From Parent (ou CloseContextMenu se o HUD tiver essa função)

**BTN_AddFriend OnClicked:**
- Get Player Controller (0) → Get Player Selection Component
- Execute Context Action (Action = **AddFriend**)
- Remove From Parent

E assim por diante para cada botão.

## Alternativa: Chamar HandlePlayerContextAction diretamente

Se preferir não usar o delegate, cada botão pode chamar o GameInstance diretamente:

```
[BTN_Trade OnClicked]
    │
    ├──► Get Player Controller → Get Player Selection Component → Get Selected Player Info
    │         (guarde em variável local "PlayerInfo")
    │
    ├──► Get Game Instance → Cast to UmbraGameInstance
    │         │
    │         └──► Handle Player Context Action
    │                   PlayerInfo = [PlayerInfo do passo anterior]
    │                   Action = Trade
    │
    └──► Remove From Parent
```

## Verificação

- O `PlayerSelectionComponent` mantém o jogador selecionado até que a seleção seja limpa
- Ao clicar direito, o jogador é selecionado e o menu abre
- Ao clicar em uma opção, `ExecuteContextAction` usa o jogador já selecionado
- O binding no PlayerController encaminha automaticamente para o GameInstance

## ⚠️ IMPORTANTE: Evitar chamada duplicada (Trade enviado 2x)

O **PlayerController** já faz binding de `OnPlayerContextActionSelected` → `HandlePlayerContextAction` no `BeginPlay`. Isso significa:

- Quando você chama `ExecuteContextAction(Action)`, o delegate dispara e o GameInstance recebe a ação **automaticamente**.
- Se o Blueprint **também** chama `HandlePlayerContextAction` diretamente, a ação é executada **duas vezes** (ex.: duas solicitações de trade).

**Solução:** Cada botão deve chamar **apenas** `ExecuteContextAction`. **Não** chame `HandlePlayerContextAction` diretamente.

```
[BTN_Trade OnClicked]  ✅ CORRETO
    │
    ├──► Get Player Controller (0) → Cast to UmbraPlayerController
    │         └──► Get Player Selection Component
    │                   └──► Execute Context Action (Action = Trade)
    │
    └──► Hide
```

**Evite** este fluxo duplicado:

```
[BTN_Trade OnClicked]  ❌ ERRADO - causa 2x requests
    │
    ├──► Execute Context Action (Trade)   ← delegate chama HandlePlayerContextAction (1ª vez)
    ├──► Handle Player Context Action     ← chamada direta (2ª vez) = DUPLICADO!
    └──► Hide
```

## Problema: Variáveis SelectionComponent e MyGameInstance null

Se o botão usa variáveis `SelectionComponent` e `MyGameInstance` que **não são atribuídas** quando o menu é exibido, elas ficam **null** em runtime. Isso causa:

- `GetSelectedPlayerInfo` retorna struct vazia (PlayerID=0)
- `HandlePlayerContextAction` recebe PlayerID=0 e a API **não é chamada** (validação falha)
- O Print String pode aparecer, mas a requisição nunca é enviada

### Solução recomendada: Obter referências no momento do clique

**Não use variáveis cacheadas** para SelectionComponent e GameInstance. Em cada botão, obtenha as referências diretamente no OnClicked:

```
[BTN_Trade OnClicked]
    │
    ├──► Get Player Controller (0)
    │         └──► Get Player Selection Component  [guarde o retorno]
    │
    ├──► Get Selected Player Info  (Target = SelectionComponent do passo anterior)
    │         └──► [PlayerInfo]
    │
    ├──► Get Game Instance
    │         └──► Cast to UmbraGameInstance  [guarde o retorno]
    │
    ├──► Handle Player Context Action
    │         Target = UmbraGameInstance
    │         PlayerInfo = [PlayerInfo]
    │         Action = Trade
    │
    └──► Hide  (ou Remove From Parent)
```

Ou, de forma mais simples, use apenas **Execute Context Action** (o delegate já encaminha para o GameInstance):

```
[BTN_Trade OnClicked]
    │
    ├──► Get Player Controller (0) → Get Player Selection Component
    │         └──► Execute Context Action (Action = Trade)
    │
    └──► Hide
```

### Se insistir em usar variáveis

O widget deve receber e armazenar `SelectionComponent` e `MyGameInstance` **ao ser exibido**. No HUD (ou onde o menu é criado), ao conectar `OnPlayerContextMenuRequested`:

1. Crie o widget do menu
2. **Set SelectionComponent** = Get Player Controller → Get Player Selection Component
3. **Set MyGameInstance** = Get Game Instance → Cast to UmbraGameInstance
4. Adicione o widget ao viewport

## Get Player Controller retorna null

Se **Get Player Controller (0)** retorna null no widget, use **Get Owning Player** em vez disso. O widget UMG tem referência ao player que o criou/possui.

**No OnClicked de cada botão:**

```
[BTN_Trade OnClicked]
    │
    ├──► Get Owning Player  (em vez de Get Player Controller)
    │         └──► Cast to UmbraPlayerController
    │                   └──► Get Player Selection Component
    │                             └──► Execute Context Action (Action = Trade)
    │
    └──► Hide
```

**Importante:** `Get Owning Player` retorna `APlayerController*`. Faça **Cast to UmbraPlayerController** para acessar `Get Player Selection Component` (o `UmbraEternumUEPlayerController` é quem tem esse componente).

Se o Cast falhar, o GameMode pode estar usando outro PlayerController. Verifique em **World Settings → GameMode → Player Controller Class** se está configurado como `UmbraPlayerController` (ou BP que herda dele).

**Alternativa com Get Game Instance:** Se Get Owning Player também falhar, use `Get Game Instance` que geralmente está disponível:

```
Get Game Instance → Cast to UmbraGameInstance
                      └──► Handle Player Context Action (PlayerInfo, Action)
```

Nesse caso, obtenha `PlayerInfo` via `Get Owning Player` → Cast to UmbraPlayerController → Get Player Selection Component → Get Selected Player Info.

## Troubleshooting

**Ação não executa ao clicar (Print String funciona mas API não é chamada):**
1. **SelectionComponent ou MyGameInstance estão null** → Use Get Player Controller / Get Game Instance diretamente no OnClicked (ver seção acima)
2. Verifique se o botão tem evento **OnClicked** conectado
3. Verifique se está chamando **Execute Context Action** ou **Handle Player Context Action**
4. Verifique se o enum está correto (ex: Trade, não InviteToParty para o botão de troca)
5. Verifique no Output Log se aparece `[UmbraPlayerController] ✅ OnPlayerContextActionSelected vinculado ao Handler` ao iniciar
6. Verifique se `[UmbraGameInstance] 🔄 Solicitando troca com...` aparece no log ao clicar (se não aparecer, PlayerInfo está vazio ou HandlePlayerContextAction não foi chamado)

**"Nenhum jogador selecionado":**
- O menu só deve abrir quando há jogador selecionado (clique direito faz a seleção)
- Se o menu for aberto por outro meio, certifique-se de que o PlayerSelectionComponent tem um jogador selecionado
