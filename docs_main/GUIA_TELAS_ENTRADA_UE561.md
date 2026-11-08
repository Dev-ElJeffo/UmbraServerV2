# GUIA — Telas de entrada em C++: login e cadastro (UE 5.6.1)

R4.1 do `docs_main/PLANO_RODADAS_UI_UE561.md`, etapas **4.1a** (cadastro) e **4.1b** (login). A seleção (4.1c) e a criação de personagem (4.1d) entram neste guia nas próximas etapas.

A lógica das telas saiu do Event Graph e foi para C++. O fluxo protegido (AGENTS.md §3) continua todo no `UUmbraGameInstance` e **não mudou de ordem**:
1. `LoginUser` faz o HTTP (`/api/login.php`), guarda o token e a conta;
2. chama `LoadCharacterList`;
3. conecta o TCP com 1 s de delay;
4. ao conectar, valida o token (opcode do Gateway). Se o token for inválido, faz `Logout`.

## Objetivo

- **Cadastro:**
  - as mesmas validações do BP, na mesma ordem e com as mesmas mensagens: campos vazios; usuário com menos de 3 caracteres; senhas diferentes; senha com menos de 6 caracteres; e-mail sem `@`;
  - **corrige um bug do BP:** o bind de `OnRegistrationFailed` estava no ramo "Is Not Valid" e nunca ficava ativo. Agora o erro do servidor (usuário ou e-mail já usado) aparece em `TXT_Status`;
  - o botão fica desabilitado enquanto espera a resposta, o que evita cadastro duplo.
- **Login:**
  - mesmas mensagens, cores e sons, e o mesmo gatilho de troca de tela: `OnCharacterListLoaded` → `Lvl_Character_Selection`;
  - **Enter** no campo de senha também faz o login;
  - o botão fica desabilitado durante o login e volta em caso de falha, inclusive se a lista de personagens falhar;
  - sem `Print String` na tela.
- **O que saiu do BP e não volta:**
  - o caminho de cadastro antigo com `CallURL` direto (`APIRegisterResponse`), que já estava desligado;
  - o `ValidateCurrentTokenViaTCP` duplicado do widget, porque o GameInstance já valida ao conectar;
  - o set de `GameServerIP`/`GameServerPort`/`bUseTCPConnection` no Construct, porque os defaults do `BP_UmbraGameInstance` já são `127.0.0.1` / `9000` / ligado.
- **Protocolo:** nenhum opcode novo ou alterado.

---

## 0) Pré-requisitos

1. **Fechar o editor** e fazer **rebuild** de `UmbraEternumUEEditor` (Development Editor). Há duas classes novas.
2. Reabrir o editor. Pelo MCP, Claude faz:
   - o **reparent** dos dois WBPs;
   - a **limpeza do Event Graph** e das variáveis que sobrarem;
   - compile e save.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| Cadastro | `/Game/Widgets/WBP_Register` | `UmbraRegisterWidget` |
| Login | `/Game/Widgets/WBP_Login2` | `UmbraLoginWidget` |
| Level de login | `/Game/Maps/Lvl_TestAuth` | Level BP, que **fica como está** (ver §3) |

## 1) Designer — nomes de bind (já existem, não renomear)

### `WBP_Register`

| Nome do bind | Tipo | Função |
|---|---|---|
| `TXT_Username` / `TXT_Email` / `TXT_Password` / `TXT_ConfirmPassword` | `EditableText` | Campos |
| `TXT_Status` | `TextBlock` | Mensagem: vermelho = erro, amarelo = "Criando conta...", verde = sucesso |
| `BTNRegister` | `Button` | Valida e chama `RegisterUser` |
| `BTNBackLogin` | `Button` | Volta para o `WBP_Login2` |

**Class Defaults** (`Umbra|Register`):
- `LoginScreenClass` (`WBP_Login2`);
- `MinUsernameLength` (3) e `MinPasswordLength` (6);
- `ReturnDelayAfterSuccess` (2 s).

### `WBP_Login2`

| Nome do bind | Tipo | Função |
|---|---|---|
| `TXT_Username` / `TXT_Password` | `EditableText` | Campos (som ao digitar; Enter na senha faz login) |
| `TXT_Status` | `TextBlock` | "Logging in..." (amarelo), "Connecting TCP..." / "TCP Connected!" (verde), erro (vermelho) |
| `btn_login` | `Button` | `LoginUser` |
| `btn_registerPAGE` | `Button` | Abre o `WBP_Register` |

**Class Defaults** (`Umbra|Login`):
- `RegisterScreenClass` (`WBP_Register`);
- `CharacterSelectionLevel` (`Lvl_Character_Selection`);
- sons (`Umbra|Login|Sound`): `ClickSound`, `HoverSound`, `TypingSound`, `LoginSuccessSound`, `LoginFailedSound`. Os defaults são os mesmos sons `/Engine/VREditor/...` de antes; a rodada de sons troca só o asset.

## 2) O que o C++ já faz

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraRegisterWidget` | `Source/UmbraEternumUE/UI/UmbraRegisterWidget.cpp` | Input mode UI Only + cursor; validação local; `RegisterUser`; `OnRegistrationSuccess` → mensagem verde → 2 s → login; `OnRegistrationFailed` → mensagem vermelha e libera o botão |
| `UUmbraLoginWidget` | `Source/UmbraEternumUE/UI/UmbraLoginWidget.cpp` | Input mode UI Only + cursor; `LoginUser`; status de `OnLoginSuccess`/`OnLoginFailed` e dos eventos do `TCPClient`; `OnCharacterListLoaded` → `OpenLevel(Lvl_Character_Selection)` (uma vez só); sons de clique, hover e digitação |
| Fluxo de autenticação | `Core/UmbraGameInstance.cpp` (`LoginUser`, `OnLoginRequestComplete`, `OnTCPConnectionSuccess`, `OnTCPTokenValidated`) | Sem mudança |

> **Não** adicione lógica no Event Graph desses WBPs.

## 3) Level BP do `Lvl_TestAuth` (exceção documentada)

Fica em BP porque só cria a primeira tela:

```
[Event BeginPlay] → [Delay 1 s] → [Branch: Is Valid(Player Controller)]
  → [Create Widget WBP_Login2] → [Branch: Is Valid] → [Add to Viewport]
  → [Set Input Mode Game And UI] → [Set bShowMouseCursor] → [Play Sound 2D: música /Game/Music/Untitled__1_, volume 0,2]
```

## 6) QA (PIE)

| Cenário | Esperado |
|---------|----------|
| Cadastro com campo vazio / usuário "ab" / senhas diferentes / senha "12345" / e-mail sem `@` | A mensagem correspondente, em vermelho, em `TXT_Status` |
| Cadastro com usuário já existente | **Agora** aparece o erro do servidor em vermelho (antes não aparecia nada) |
| Cadastro válido | "Criando conta..." (amarelo), depois a mensagem de sucesso (verde) e, em 2 s, volta ao login |
| Back no cadastro | Volta ao login |
| Login com campo vazio | "Preencha todos os campos!" |
| Login com senha errada | Mensagem do servidor em vermelho, som de erro, e o botão volta a funcionar |
| Login certo (botão ou Enter) | "Logging in..." → `Lvl_Character_Selection`; no log, TCP conectado e token validado |
| Clicar Login várias vezes rápido | Uma requisição só |
| Logout na seleção → login de novo | Funciona normalmente (sem bind duplicado) |
| Tela | Nenhum texto azul de `Print String` |

Logs esperados:
- `[UmbraGameInstance] ✅ Login bem-sucedido!`;
- `Token: (N caracteres)`;
- `🔐 Validando token após conexão TCP`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Clicar nos botões não faz nada | Reparent não aplicado ou bind renomeado | Parent = `UmbraLoginWidget` / `UmbraRegisterWidget`; conferir os nomes da §1 |
| Login fica em "Logging in..." | PHP fora do ar ou lista de personagens sem resposta | Ver o Output Log (`[UmbraGameInstance]`) e o WAMP |
| Abre o level errado depois do login | `CharacterSelectionLevel` alterado | Class Defaults do `WBP_Login2` |
| Sem som | Asset de som movido | Preencher os sons em `Umbra|Login|Sound` |

## 8) Referência de código

- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraLoginWidget.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraRegisterWidget.cpp`
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstance.cpp`: `LoginUser`, `RegisterUser`, `OnTCPConnectionSuccess`

**Fim do guia.**
