# Guia completo — `WBP_PersonalShopBuyer` (UE 5.6.1)

Painel do **comprador** da loja pessoal. Parent C++: **`UmbraPersonalShopBuyerWidget`**. Há **dois modos** (o C++ escolhe sozinho):

- **Modo grade** (paridade com o vendedor): `Grid_StoreSlots` + **Class Defaults → Store Slot Widget Class** (ex.: o mesmo `WBP_StoreSlot` do vendedor) + `Text_Price_0` … `Text_Price_9`. **Clique no slot** seleciona; **`BTN_BuyItem`** confirma a compra (se o botão existir).
- **Modo lista**: só `VBox_Listings` com linhas dinâmicas (**Selecionar**). Sem grade configurada, o guia antigo de “só lista” continua válido.

Em ambos: **`TXT_StoreInfo`**, **`Btn_Close`**, **`Text_ShopTitle`**. **`BTN_OpenPersonalShop`** fica no **nameplate remoto** (`UUmbraRemoteNameplateWidget`): o clique é ligado em **C++** (`OpenPersonalShopBuyerUI`) — **não** precisa de **OnClicked** em Blueprint nesse fluxo.

**Código de referência:** `UmbraPersonalShopBuyerWidget.cpp/.h`, `UmbraBuyerListingSelectButton.cpp/.h`, `UmbraGameInstance::OpenPersonalShopBuyerUI` / `LoadPersonalShopBySeller` / `PurchasePersonalShopListing`.

---

## 1. Pré-requisitos

| # | Onde | Ação |
|---|------|------|
| 1.1 | **Content Browser** → criar Widget Blueprint | Nome sugerido: `WBP_PersonalShopBuyer`. |
| 1.2 | **File → Reparent Blueprint** (ou Class Settings) | **Parent Class** = `UmbraPersonalShopBuyerWidget`. |
| 1.3 | `BP_UmbraGameInstance` (ou `UUmbraGameInstance` defaults) | **Personal Shop Buyer Widget Class** = este WBP (não a classe C++ crua). |
| 1.4 | **Modo grade (obrigatório para slots como o vendedor)** | Abre o **WBP_PersonalShopBuyer** → **Class Defaults** (ícone de **predefinições** no toolbar do Blueprint, não o Designer) → **Personal Shop \| Buyer \| Classes** → **Store Slot Widget Class** = **exatamente** o mesmo asset que o `WBP_PlayerStore` usa (ex. `WBP_StoreSlot`, parent `UmbraStoreSlotWidget` / `UmbraInventorySlotWidget`). **Se este campo estiver em *None*, o C++ usa modo lista** (linhas “Selecionar”), mesmo que exista `Grid_StoreSlots` no Hierarchy. |
| 1.5 | API / login | Comprador autenticado com token válido (o mesmo fluxo HTTP do projeto). |

---

## 2. Nomes obrigatórios no Designer (`Variable Name`)

O C++ resolve por **`GetWidgetFromName`** — os nomes têm de coincidir **exatamente** (maiúsculas/minúsculas).

### 2.1 Comuns (lista e grade)

| Nome no Hierarchy | Tipo sugerido | Função |
|-------------------|---------------|--------|
| **`Text_ShopTitle`** | `Text` | Título (ex.: nome da loja); preenchido pelo estado HTTP. |
| **`Btn_Close`** | `Button` | Fecha o painel (`RemoveFromParent`). |
| **`TXT_StoreInfo`** | `Text` (multilinha recomendado) | Mensagens de estado, erros e instruções (como no vendedor). |
| **`BTN_BuyItem`** | `Button` | **Opcional mas recomendado** no modo grade e na lista: confirma compra do item **selecionado** (slot ou linha). |

**Sem `BTN_BuyItem`:** no modo lista, **Selecionar** compra na hora; no modo grade, o clique no slot pode comprar direto (comportamento definido no C++).

### 2.2 Modo lista

| Nome | Tipo | Função |
|------|------|--------|
| **`VBox_Listings`** | `Vertical Box` | Onde o C++ cria as linhas (nome + preço + **Selecionar**). |

### 2.3 Modo grade (igual vendedor)

| Nome | Tipo | Função |
|------|------|--------|
| **`Grid_StoreSlots`** | `Uniform Grid Panel` | 10 slots; filhos = instâncias de **Store Slot Widget Class**. |
| **`Text_Price_0`** … **`Text_Price_9`** | `Text` | Preço por slot (ou `-` se vazio). |

**Hierarchy:** podes manter `Grid_StoreSlots` **dentro** de `VBox_Listings` (como no teu layout). O modo lista já **não** apaga a grade: só remove `HorizontalBox` dinâmicos. Mesmo assim, para ver **slots**, define **Store Slot Widget Class** (modo grade).

---

## 3. Hierarquia sugerida

### 3.0 Modo grade (recomendado se quiseres paridade visual com o vendedor)

Podes reutilizar a mesma árvore de **slots + preços** do WBP do vendedor (`Grid_StoreSlots`, `Grid_SlotPrices` com `Text_Price_*`), **sem** os controlos de vendedor (`BTN_Set_Price`, `HBox_SellItemOptions`, etc.) — o C++ do comprador **não** liga a esses nós.

### 3.1 Modo lista (mínimo)

```text
[Root] (ex.: Overlay ou Canvas Panel)
├── Text_ShopTitle
├── Btn_Close
├── TXT_StoreInfo        ← Auto Wrap Text ligado; altura mínima confortável
├── BTN_BuyItem          ← Texto: "Comprar item"
└── ScrollBox (opcional)
      └── VBox_Listings  ← Margem interna; não colocar nada por cima deste VBox
```

### 3.1 Z-order e hit-test

- Nada com **Visibility = Visible** deve ficar **por cima** de `VBox_Listings` se não for para receber clique.
- Se usar **Overlay**, a ordem dos filhos importa: o último filho desenha por cima.
- **“Simulando”** / overlays de debug não são deste WBP, mas qualquer **retângulo full-screen** com hit ativo bloqueia os botões.

---

## 4. Fluxo de jogo (o que o C++ faz)

1. **`OpenPersonalShopBuyerUI(SellerPlayerId)`** (GameInstance) cria o WBP, `AddToViewport`, chama **`InitializeForSeller`**.
2. **`InitializeForSeller`** inscreve delegates (`OnPersonalShopStateLoaded`, `OnPersonalShopPurchaseCompleted`, `OnPersonalShopActionFailed`) e chama **`LoadPersonalShopBySeller`** (GET `get_personal_shop.php`).
3. Ao receber o JSON, **`PopulateFromState`**:
   - **Modo grade:** monta/atualiza os 10 slots, preços `Text_Price_*`, mapeamento `listing_id`; clique no slot → **`SelectBuyerStoreSlot`**.
   - **Modo lista:** limpa `VBox_Listings` e, para cada listing com **`listing_id` > 0**, adiciona linha com **Selecionar** (`UUmbraBuyerListingSelectButton`).
4. **Selecionar** (lista) ou seleção + **`BTN_BuyItem`** (grade/lista com botão): **`RequestPurchaseListing`** / **`PurchasePersonalShopListing`** conforme já implementado.
5. Erros HTTP / API → **`OnPersonalShopActionFailed`** → **`TXT_StoreInfo`**.

**Drop de inventário na grade do comprador:** é **rejeitado** (loja só leitura).

---

## 5. `listing_id` e “clique não faz nada”

O servidor devolve `listing_id` por linha. Se vier **0** ou a linha for ignorada, a compra não dispara.

**Checklist:**

- Resposta JSON de `get_personal_shop.php` com array `listings` e campo **`listing_id`** (o PHP já usa `psl.listing_id`).
- No Output Log, procure avisos **`Listing sem listing_id`** ou **`listing_id inválido`**.
- No WBP, **não** uses `AddToViewport` num segundo nameplate/widget da mesma loja por cima deste painel.

---

## 6. Paridade com o WBP do vendedor

| Vendedor (`UmbraPlayerStoreWidget`) | Comprador (`UmbraPersonalShopBuyerWidget`) |
|-------------------------------------|--------------------------------------------|
| `Grid_StoreSlots` + classe de slot | Igual no **modo grade** + **Store Slot Widget Class** |
| Preços / UI por slot | `Text_Price_0` … `9` |
| `TXT_StoreInfo` | `TXT_StoreInfo` (mesmo nome) |
| Seleção de slot + ações de venda | Seleção de slot (grade) ou **Selecionar** (lista) + **`BTN_BuyItem`** |
| DnD para listar | Comprador: **sem** drop (rejeitado) |

O nameplate **`BTN_OpenPersonalShop`**: com **`UUmbraRemoteNameplateWidget`**, o clique já abre o comprador — **não** é obrigatório **OnClicked** em Blueprint.

---

## 7. Estilo e textos sugeridos

- **Text_ShopTitle:** pode espelhar o nome da loja (preenchido pelo C++).
- **BTN_BuyItem:** “Comprar item” ou “Comprar selecionado”.
- **Btn_Close:** “Fechar” ou ícone X.
- **TXT_StoreInfo:** mensagens curtas; o C++ envia texto em PT nas ações comuns.

---

## 8. Blueprint opcional

- Podes estender visualmente o WBP (imagens, sons) sem alterar os **Variable Name** dos nós listados na §2.
- **`GetSelectedListingId`** (BlueprintCallable no parent C++) pode ser usado em BP para lógica extra.
- **Não** é necessário implementar `UpdateNameplate` aqui — isso é do nameplate remoto.

---

## 9. Testes rápidos

1. PIE com 2 clientes: vendedor abre loja; comprador usa **`BTN_OpenPersonalShop`** no nameplate → abre `WBP_PersonalShopBuyer`.
2. **Modo lista:** linhas com preço e **Selecionar**.
3. **Modo grade:** ícones nos 10 slots, preços em **`Text_Price_*`**, clique seleciona → **Comprar item** (se existir botão).
4. Forçar erro (ex.: fechar loja no vendedor) e confirmar mensagem em **`TXT_StoreInfo`** via `OnPersonalShopActionFailed`.

---

## 10. Ligações úteis

- Plano WBP vendedor: [PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md](PLANO_INTEGRACAO_LOJA_PESSOAL_WBP_NODOS.md)
- Nameplate + comprador (fluxo geral): [GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md](GUIA_NODOS_LOJA_NAMEPLATE_COMPRADOR_UE561.md)
- Mapa C++ / BP loja: [GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md](GUIA_IMPLEMENTACAO_LOJA_PESSOAL_UI_CPP.md)

---

## 11. Resumo de alterações recentes no C++

- **Modo grade:** `Grid_StoreSlots`, `Text_Price_0`…`9`, **`Store Slot Widget Class`**, clique em `UmbraInventorySlotWidget` → `SelectBuyerStoreSlot`.
- Linhas da lista: **Selecionar** (`UmbraBuyerListingSelectButton`) + tamanho mínimo para hit estável.
- **`TXT_StoreInfo`** + **`OnPersonalShopActionFailed`**.
- **`BTN_BuyItem`** opcional.
- Drop na UI do comprador: **bloqueado** (`UmbraStoreSlotWidget` / `UmbraInventorySlotWidget`).
- **`UUmbraShopBuyButton`** continua disponível com texto **“Comprar”** se fores reutilizar em outro layout (compra imediata por linha).
