# Guia nó a nó — Loja pessoal: nameplate, comprador, bloqueio de movimento (UE **5.6.1**)

Este guia assume **Unreal Engine 5.6.1** e o código C++ já presente no repositório. O foco é **passos no Editor** (e o mínimo de PHP) para **ligar** vendedor, **balão** no nameplate, **comprador** e **bloqueio de movimento**, sem repetir a montagem visual completa da grade da loja (isso continua em [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md) e [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md)).

---

## 0. Referência rápida (C++ que você precisa saber que existe)

| Área | Classe / função | Arquivo(s) |
|------|-----------------|-------------|
| Painel do **vendedor** | `UUmbraPlayerStoreWidget` | `UmbraEternumUE/.../UI/UmbraPlayerStoreWidget.h/.cpp` |
| Modal de preço | `UUmbraSetItemPriceWidget` | `.../UI/UmbraSetItemPriceWidget.h/.cpp` |
| Slot da grade da loja | `UUmbraStoreSlotWidget` (opcional) / `UUmbraInventorySlotWidget` | `.../UI/` |
| **Nameplate** remoto + balão | `UUmbraRemoteNameplateWidget` | `.../UI/UmbraRemoteNameplateWidget.h/.cpp` |
| **Comprador** | `UUmbraPersonalShopBuyerWidget`, `UUmbraBuyerListingSelectButton`, `UUmbraShopBuyButton` | `.../UI/UmbraPersonalShopBuyerWidget.*`, `UmbraBuyerListingSelectButton.*`, `UmbraShopBuyButton.*` |
| HTTP / WS / abrir UI comprador | `UUmbraGameInstance::OpenPersonalShopBuyerUI`, `UpdateRemotePlayerPersonalShopSign`, `LoadPersonalShopBySeller`, `PurchasePersonalShopListing` | `.../Core/UmbraGameInstance.h/.cpp` |
| PHP loja | `shop_bootstrap.php`, `open_personal_shop.php`, `get_personal_shop.php`, `purchase_listing.php` | `www/umbra_api/api/shop/` |

**Guia completo só do WBP comprador** (hierarquia, `TXT_StoreInfo`, `BTN_BuyItem`, seleção vs compra): [GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md](GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md).

---

## 1. Pré-requisitos (fora de Blueprint)

| # | Onde | O que fazer |
|---|------|-------------|
| 1.1 | Máquina de desenvolvimento | Instalar / usar **Unreal Engine 5.6.1** (mesma versão do `.uproject` do time). |
| 1.2 | Visual Studio (Workload **Game dev with C++**) | Abrir `UmbraEternumUE.sln` ou usar **Compile** no Editor após alterar C++. |
| 1.3 | Editor UE | **Tools → Refresh Visual Studio Project** (se faltar arquivo `.cpp` novo na solução). |
| 1.4 | Menu **Build** do Editor | **Compile** até zerar erros (classes `UmbraRemoteNameplateWidget`, `UmbraPersonalShopBuyerWidget`, etc.). |
| 1.5 | API PHP | Em `www/umbra_api/helpers/` existir `personal_shop_helper.php`; endpoints da pasta `api/shop/` usarem `shop_bootstrap.php` (repositório atual). |
| 1.6 | WAMP / Apache | `umbra_api` acessível na URL que o VaRest usa (ex.: `http://localhost/umbra_api/`). |

---

## 2. GameInstance — classe do comprador (Class Defaults)

O comprador abre por **`OpenPersonalShopBuyerUI(SellerPlayerId)`**, que cria um widget cuja classe vem de **`PersonalShopBuyerWidgetClass`**.

| # | No Editor | Ação literal |
|---|-----------|--------------|
| 2.1 | **Content Browser** → localizar o **Blueprint do GameInstance** que o projeto usa (ex.: `BP_UmbraGameInstance` ou o definido em **Project Settings → Maps & Modes → Game Instance Class**). | Abrir esse asset. |
| 2.2 | Barra superior do Blueprint → **Class Defaults**. | Abre o painel de defaults. |
| 2.3 | Painel **Details** → categoria **Social → Personal Shop → UI** (ou nome próximo). | Localizar **Personal Shop Buyer Widget Class**. |
| 2.4 | Dropdown da propriedade | Escolher o **asset Blueprint** do comprador criado na **§5** (ex.: `WBP_PersonalShopBuyer`). **Não** deixe só a classe C++ literal `UmbraPersonalShopBuyerWidget` no seletor — esse widget **não** tem hierarquia UMG no Designer (falta `VBox_Listings`, etc.). |
| 2.5 | **Compile** → **Save**. | — |

Se essa propriedade ficar **None**, ao clicar no nameplate o C++ emite falha via `OnPersonalShopActionFailed` (mensagem pedindo configurar a classe).

---

## 3. Nameplate remoto — `WBP` com parent `UmbraRemoteNameplateWidget`

O `GameInstance` chama **`ApplyPersonalShopVisual`** no widget do **Widget Component** do personagem (local ou remoto) quando a loja abre/fecha (WebSocket + HTTP). O C++ resolve widgets **pelo nome** (igual à loja do vendedor).

### 3.1 Duplicar ou abrir o WBP atual do nameplate

| # | Ação |
|---|------|
| 3.1.1 | No **Content Browser**, localize o User Widget usado no **Widget** do personagem (ex.: `WBP_PlayerNameplate`). |
| 3.1.2 | Recomendação: fazer **Duplicate** antes de mudar o parent, para ter backup. |

### 3.2 Class Settings — parent class

| # | Onde | Definir |
|---|------|---------|
| 3.2.1 | Blueprint aberto → **Class Settings** (ícone engrenagem). | — |
| 3.2.2 | **Details** → **Class Options** → **Parent Class**. | **`UmbraRemoteNameplateWidget`** (não `UserWidget` genérico). |
| 3.2.3 | **Compile**. | Corrigir erros de propriedades duplicadas (nomes iguais aos do C++ `Cached*`, se houver). |

### 3.3 Class Settings — Editor (evitar vazamento / `TransBuffer`)

| # | Onde | Ação |
|---|------|------|
| 3.3.1 | **Class Settings** → **Blueprint Options** (ou **Widget** conforme versão). | Desmarcar **`Run Construction Script on Drag`** (execução extra no Editor pode empilhar transações no **Undo** e contribuir para `REINST_*` + *world leaks* após recompilar o BP). |
| 3.3.2 | Antes de **Compile** pesado do nameplate ou do C++ do módulo. | **Parar o PIE** (nenhuma sessão Play ativa). |
| 3.3.3 | Se aparecer *Fatal world leaks* / `TransBuffer` → `REINST_WBP_PlayerNameplate`. | Fechar o Editor, reabrir o projeto; na próxima vez, seguir 3.3.1–3.3.2. O C++ agora remove o bind do botão da loja em **`NativeDestruct`** do `UmbraRemoteNameplateWidget` para não segurar o widget no delegate. |

### 3.4 Designer — hierarquia mínima sugerida

Crie (ou renomeie) estes **Variable Name** / nomes no **Hierarchy** (exatamente como abaixo para o `GetWidgetFromName` do C++):

| Nome no Designer | Tipo sugerido | Comportamento |
|------------------|---------------|---------------|
| **`HBX_ShopBalloon`** | `Horizontal Box` ou `Overlay` | Container do “balão”. **Visibility** inicial: **Collapsed** (o C++ mostra quando a loja estiver aberta). |
| **`BTN_OpenPersonalShop`** | `Button` | Dentro do balão (ou sobreposto). Abre a UI do comprador. Inicial **Collapsed**. |
| **`Text_ShopBalloon`** | `Text` (opcional) | Nome da loja; inicial **Collapsed**. |

**Não** é obrigatório Event Graph: o C++ faz **`Bind`** do clique em **`NativeConstruct`** / **`ApplyPersonalShopVisual`**.

| # | Event Graph |
|---|-------------|
| 3.4.1 | **Graph** → preferir **vazio** (sem lógica de loja). |

### 3.5 Widget Component no personagem

| # | Onde | Ação |
|---|------|------|
| 3.5.1 | Abrir o **Blueprint do personagem** (ou do **NetMovementClient** / actor que carrega o nameplate — o que o projeto já usar). | — |
| 3.5.2 | Selecionar o **Widget** (Widget Component). | — |
| 3.5.3 | **Details** → **User Interface** → **Widget Class**. | Definir para o WBP que você acabou de configurar (**§3**). |
| 3.5.4 | **Compile** → **Save**. | — |

> **Nota:** O `GameInstance` procura o componente com **`FindComponentByClass<UWidgetComponent>`** no actor do jogador remoto (ou no pawn local se for o vendedor). Tem que existir **um** `WidgetComponent` com o widget certo.

---

## 4. Loja do vendedor (resumo cruzado)

Não repetimos todos os nós aqui; siga na ordem:

1. [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md) — parent `UmbraPlayerStoreWidget`, Class Defaults, nomes `Grid_StoreSlots`, `HBox_SellItemOptions`, `TXT_StoreInfo`, etc.
2. [PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md) — checklist estendido.
3. [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md) — mapa BP ↔ C++.

Pontos que impactam o **nameplate**:

| # | Comportamento |
|---|----------------|
| 4.1 | Ao **Start Store** com sucesso, o servidor + WS atualizam outros clientes → **`UpdateRemotePlayerPersonalShopSign`** mostra o balão. |
| 4.2 | O painel do vendedor (**`UmbraPlayerStoreWidget`**) bloqueia **`SetIgnoreMoveInput(true)`** enquanto estiver na tela; ao fechar/remove, desbloqueia. |

---

## 5. Widget do comprador — criar `WBP_PersonalShopBuyer` (mínimo)

Detalhamento completo (paridade com vendedor, `TXT_StoreInfo`, `BTN_BuyItem`, hit-test): **[GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md](GUIA_IMPLEMENTACAO_WBP_PERSONAL_SHOP_BUYER_UE561.md)**.

| # | Ação |
|---|------|
| 5.1 | **Content Browser** → **User Interface** → **Widget Blueprint** → nome sugerido: `WBP_PersonalShopBuyer`. |
| 5.2 | **Class Settings** → **Parent Class** → **`UmbraPersonalShopBuyerWidget`**. |
| 5.3 | **Designer** — raiz típica: `Canvas Panel` ou `VerticalBox`. |

### 5.4 Nomes obrigatórios (Hierarchy)

| Nome exato | Tipo | Função |
|-------------|------|--------|
| **`VBox_Listings`** | `Vertical Box` | O C++ **limpa** e **preenche** com linhas (nome do item, preço, botão **Selecionar**). Nada por cima deste painel com hit ativo. |
| **`Btn_Close`** | `Button` | Fecha o widget (`RemoveFromParent`). |
| **`Text_ShopTitle`** | `Text` (opcional) | Título com nome da loja após o GET. |
| **`TXT_StoreInfo`** | `Text` (recomendado) | Mensagens de estado / erro (delegate `OnPersonalShopActionFailed`). |
| **`BTN_BuyItem`** | `Button` (recomendado) | Confirma compra do item **selecionado**; sem este botão, **Selecionar** compra na hora. |

### 5.5 Event Graph

| # | Ação |
|---|------|
| 5.5.1 | Deixar **vazio**; o C++ faz `LoadPersonalShopBySeller`, `OnPersonalShopStateLoaded`, compra e refresh. |

### 5.6 Compile / Save

| # | **Compile** → **Save**. |
|---|-------------------------|

### 5.7 Ligar no GameInstance

Voltar à **§2** e apontar **Personal Shop Buyer Widget Class** para este asset.

---

## 6. Fluxo de teste (PIE)

| # | Passo |
|---|--------|
| 6.1 | **Editor Preferences** → **Play** → número de jogadores **2** (se quiser testar remoto + comprador). |
| 6.2 | **Cliente 1:** login, abrir loja, colocar itens, preço, **Confirmar oferta**, **Start Store**. |
| 6.3 | **Cliente 2:** aproximar-se do personagem do cliente 1; no nameplate deve aparecer o **balão** / botão quando a loja estiver aberta. |
| 6.4 | Clicar em **`BTN_OpenPersonalShop`** → deve abrir **`WBP_PersonalShopBuyer`** com lista; **Selecionar** + **Comprar item** (ou só **Selecionar** se não houver `BTN_BuyItem`) → `PurchasePersonalShopListing`. |
| 6.5 | **Cliente 1:** com o painel da loja aberto, tentar WASD → movimento deve estar **bloqueado**; ao fechar o painel, deve voltar. |

---

## 7. PHP / servidor (se o comprador não carregar)

| # | Verificação |
|---|-------------|
| 7.1 | Arquivo `www/umbra_api/helpers/personal_shop_helper.php` presente no servidor (WAMP). |
| 7.2 | `GET .../api/shop/get_personal_shop.php?seller_player_id=X` retorna JSON `success: true` no browser ou Postman. |
| 7.3 | Token JWT válido no cliente (mesmo fluxo do resto da API). |

---

## 8. Fallback sem `UmbraRemoteNameplateWidget`

Se o nameplate **não** puder mudar de parent ainda:

| # | No WBP atual |
|---|----------------|
| 8.1 | Manter parent `UserWidget`. |
| 8.2 | Adicionar **função Blueprint** (ou C++ filho) chamada exatamente **`UpdatePersonalShopSign`** com parâmetros compatíveis: **`ShopName`** (String), **`ShopId`** ou **`ShopID`** (Integer), **`bVisible`** ou **`Visible`** (Boolean), e opcional **`SellerPlayerId`** / **`SellerPlayerID`** / **`PlayerId`** (Integer) — o `GameInstance` preenche por reflexão. |

O caminho preferido continua sendo **`UmbraRemoteNameplateWidget`** (tudo em C++ para o balão).

---

## 9. Problemas comuns (UE 5.6.1)

| Sintoma | Causa provável | O que fazer |
|---------|----------------|-------------|
| Erro de compile `ConstructWidget`: não deduz template | UE 5.6 exige **`WidgetTree->ConstructWidget<UHorizontalBox>()`** (tipo explícito), não `ConstructWidget(UHorizontalBox::StaticClass())` | Já corrigido em `UmbraPersonalShopBuyerWidget.cpp` |
| Clique no nameplate não abre loja | `PersonalShopBuyerWidgetClass` None ou WBP não herda `UmbraPersonalShopBuyerWidget` | **§2** e **§5** |
| Balão / texto da loja aparecem mas o clique **não faz nada** | `UpdateRemotePlayerPersonalShopSign` não encontrava o ator em `RemotePlayerActorsMap`, logo `CachedSellerPlayerId` nunca era preenchido | Corrigido em C++: o nameplate escuta `OnRemotePersonalShopVisualUpdated`, casa o ID com o dono do `WidgetComponent` e repõe estado com `TryGetLastRemotePersonalShopVisual` no `NativeConstruct`. Nome do botão: `BTN_OpenPersonalShop` ou `Btn_OpenPersonalShop` |
| Lista vazia no comprador | `VBox_Listings` com nome errado ou fora da hierarquia visível | Conferir nome exato **§5.4** |
| Balão nunca aparece | Nameplate não é `UmbraRemoteNameplateWidget` e não existe `UpdatePersonalShopSign` | **§3** ou **§8** |
| `TXT_StoreInfo` com erro PHP | Helper em falta no WAMP | **§7** |
| Movimento não bloqueia no vendedor | Widget não é `UmbraPlayerStoreWidget` ou foi removido sem `RemoveFromParent` | Parent class da loja; fechar com botão que chama `RemoveFromParent` |
| **Fatal world leaks** / `TransBuffer` / `REINST_WBP_PlayerNameplate` | Reinstanciação do BP com PIE aberto, **Run Construction Script on Drag**, ou delegate do botão segurando o widget | **§3.3**; atualizar C++ (`NativeDestruct` no nameplate); reiniciar o Editor se já crashou |

---

## 10. Índice de documentos relacionados

| Documento | Conteúdo |
|-----------|-----------|
| [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md) | Integração nó a nó da **loja do vendedor** |
| [PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md](PLANO_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP_NODOS.md) | Plano estendido UI C++ |
| [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md) | Visão geral C++ loja |
| [GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md](GUIA_IMPLEMENTACAO_WBP_LOJA_PESSOAL.md) | Guia longo legado / Designer |

---

## 11. Regras do repositório (agentes)

- Versão UE e ponteiro para este guia: **`AGENTS.md` §2.1** e **`.cursor/rules/modify-files.mdc`**.
