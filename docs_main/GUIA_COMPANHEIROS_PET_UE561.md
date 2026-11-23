# GUIA — Companheiros: base + pet de coleta (UE 5.6.1)

Esta etapa cria o sistema de companheiros com dois tipos, `pet` e `elemental`, e o pet de coleta. O jogador ganha o pet usando um item e o invoca ou recolhe pela janela de companheiros. O pet segue o dono e aparece para os outros jogadores. Com o pet ativo, **a zone coleta sozinha** o loot dos corpos que o dono matou. O **C++ já resolve** a rede, a coleta, o ator do pet e a janela. No Editor você só cria os WBPs (parent class + layout), a Input Action e cadastra o pet no Manager.

## Objetivo

- O jogador usa o **Selo da Fada Coletora** e ganha o pet de forma permanente.
- **P** abre a janela de companheiros, com os botões **Invocar** e **Recolher** e o filtro de coleta (raridade mínima e ouro).
- O pet segue o dono, flutua se tiver `hover_height` e some junto com o dono quando ele fica invisível (stealth) ou sai da visão.
- Quando o dono mata um mob perto do corpo, o pet **voa até o corpo** depois de `loot_delay_ms`. Os itens e o ouro que passam no filtro vão para a bolsa, e uma linha aparece no chat: *"Fada Coletora coletou: 25 de ouro, Poção de Vida x2"*.
- O que sobra (filtro ou bolsa cheia) abre na **janela de loot normal**. Se o dono estiver fora do raio, a janela de loot abre como antes.
- Ao relogar, o pet que estava invocado volta sozinho.
- Opcodes **novos**: **124** (pedido), **125** (estado visual) e **126** (coleta). Nenhum opcode existente muda.
- O tipo `elemental` já existe no banco e no protocolo, mas ainda não age; isso fica para a etapa 2.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `companion_templates.companion_key` / `name` / `description` | Manager → **Companheiros** | Identificação e texto da janela |
| `companion_templates.companion_type` | Manager → Companheiros → "Tipo" | `pet` = coleta; `elemental` = etapa 2 |
| `companion_templates.icon_path` | Manager → Companheiros → "Ícone" | Ícone da linha na janela (Texture2D) |
| `companion_templates.mesh_path` | Manager → Companheiros → "Mesh" | SkeletalMesh **ou** StaticMesh; vazio = esfera de teste |
| `companion_templates.idle_anim_path` / `move_anim_path` | Manager → Companheiros | AnimSequence parado/movendo (só SkeletalMesh) |
| `companion_templates.pickup_vfx_path` | Manager → Companheiros → "VFX ao coletar" | NiagaraSystem tocado no corpo (opcional) |
| `companion_templates.scale` / `follow_distance` / `hover_height` | Manager → Companheiros | Escala, distância atrás do dono, altura (0 = no chão) |
| `companion_templates.loot_radius_uu` | Manager → Companheiros → "Raio dono-corpo" | O pet só coleta se o corpo estiver nesse raio do dono (1500 = 15 m) |
| `companion_templates.loot_delay_ms` | Manager → Companheiros → "Espera após o kill" | Tempo até o pet "buscar" o loot |
| `companion_templates.required_level` / `is_active` | Manager → Companheiros | Nível para usar o selo e invocar; desativado = não invoca |
| `player_companions.*` | (jogo) | Posse, ativo e filtro de cada personagem; gravado pela zone |
| Item `item_subtype = companion` | Manager → **Itens** | Selo: `item_type = consumable`, `stats_json = {"companion_template_id": ID}` |

SQL (já aplicado no dev; idempotente):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\create_companion_system.sql
```

Ele cria as tabelas, o pet de teste `PET_SPRITE` ("Fada Coletora", sem mesh = esfera flutuando) e o item **Selo da Fada Coletora** (id 136 no dev).

---

## 0) Pré-requisitos

1. **Parar a zone**, recompilar o `zone_server` (`cmake --build . --config Release --target zone_server`) e subir de novo.
2. **Fechar o Manager** e rodar `dotnet build tools/UmbraManagerWpf/UmbraManager.sln` (a aba nova só aparece depois disso).
3. Rebuild do módulo `UmbraEternumUE` (classes novas: `UmbraCompanionActor`, `UmbraCompanionWindowWidget`, `UmbraCompanionEntryWidget`).
4. PHP já sincronizado com o WAMP.
5. **Parar o PIE** antes de editar o Designer.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_Companions` | `/Game/Widgets/UI/Companions/WBP_Companions` | `UmbraCompanionWindowWidget` |
| `WBP_CompanionEntry` | `/Game/Widgets/UI/Companions/WBP_CompanionEntry` | `UmbraCompanionEntryWidget` |
| `IA_OpenCompanions` | `/Game/Input/Actions/IA_OpenCompanions` | Input Action (Digital/bool) |

O pet em si **não tem Blueprint**: o ator `AUmbraCompanionActor` é C++ puro e carrega mesh, animações e VFX pelos paths do Manager.

---

## 1) Designer — `WBP_Companions` (janela)

Crie em `/Game/Widgets/UI/Companions/` → **User Widget** → parent **UmbraCompanionWindowWidget**. Use o mesmo esqueleto das outras janelas (`WBP_Mail`, `WBP_QuestJournal`).

### 1.1 Hierarquia

```
[CanvasPanel — raiz, tela cheia]
  Border_Window                         ← painel que se move (padrão das janelas)
    VerticalBox
      Border_TitleBar
        Text_WindowTitle                ← "Companheiros"
        Btn_WindowClose
      ScrollBox
        VB_CompanionList                ← NOVO (VerticalBox): linhas criadas em C++
      Text_EmptyHint                    ← NOVO (TextBlock)
      Text_SelectedName                 ← NOVO (TextBlock)
      Panel_LootFilter                  ← NOVO (HorizontalBox/VerticalBox)
        Text "Coletar a partir de:"     ← rótulo fixo
        Combo_LootRarity                ← NOVO (ComboBoxString, sem opções: o C++ preenche)
        Check_LootGold                  ← NOVO (CheckBox) + Text "Coletar ouro"
      Text_CompanionStatus              ← NOVO (TextBlock)
```

### 1.2 Binds

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `VB_CompanionList` | `VerticalBox` (qualquer painel) | Não | Recebe uma linha `WBP_CompanionEntry` por companheiro |
| `Text_EmptyHint` | `TextBlock` | Não | "Você ainda não tem companheiros…" quando a lista está vazia |
| `Text_SelectedName` | `TextBlock` | Não | Nome do companheiro selecionado (o filtro vale para ele) |
| `Panel_LootFilter` | qualquer `Widget` | Não | Some quando o selecionado não é pet |
| `Combo_LootRarity` | `ComboBoxString` | Não | Raridade mínima. Opções preenchidas em C++: Comum ou melhor → Só lendário |
| `Check_LootGold` | `CheckBox` | Não | Coletar ouro sim/não |
| `Text_CompanionStatus` | `TextBlock` | Não | Erros e confirmações ("Filtro de coleta atualizado.") por 4 s |

Class Defaults (opcionais):
- **Companion Entry Class** = `WBP_CompanionEntry`. Se ficar vazio, o C++ carrega o path padrão.
- **Window Title** = "Companheiros".

## 1.3) Designer — `WBP_CompanionEntry` (linha)

Parent **UmbraCompanionEntryWidget**.

```
[Overlay — raiz]
  Btn_Select                 ← NOVO (Button, fundo da linha inteira; seleciona para o filtro)
  HorizontalBox (Visibility = Self Hit Test Invisible)
    Image_Icon               ← NOVO (Image 40×40)
    VerticalBox
      Text_Name              ← NOVO
      Text_Type              ← NOVO ("Pet de coleta" / "Elemental")
      Text_Status            ← NOVO ("Invocado" / "Recolhido" / "Requer nível X")
    Btn_Toggle               ← NOVO (Button)
      Text_ToggleLabel       ← NOVO ("Invocar" / "Recolher")
```

| Nome do bind | Tipo | Função |
|--------------|------|--------|
| `Image_Icon` | `Image` | Ícone do Manager (escondido se vazio) |
| `Text_Name` / `Text_Type` / `Text_Status` | `TextBlock` | Preenchidos em C++ |
| `Btn_Toggle` + `Text_ToggleLabel` | `Button` + `TextBlock` | Invoca ou recolhe (opcode 124) |
| `Btn_Select` | `Button` | Seleciona a linha (as outras ficam com opacidade 0.75) |

## 1.4) Input — tecla P

1. `/Game/Input/Actions/` → botão direito → **Input** → **Input Action** → `IA_OpenCompanions` (Value Type **Digital (bool)**).
2. Abrir `/Game/Input/IMC_Default` → **+** Mapping → `IA_OpenCompanions` → tecla **P**.
3. No Blueprint do personagem jogável (o mesmo que tem `Open Quest Journal Action`), em **Class Defaults → Input**: **Open Companions Action** = `IA_OpenCompanions`.
4. (Opcional) No Blueprint do PlayerController: **Companions Widget Class** = `WBP_Companions`.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `CompanionService` | `src/zone/CompanionService.cpp` | Carrega `companion_templates`; trata o 124 (confere posse, validade e nível no DB, grava `is_active` e o filtro, um ativo por tipo); emite o 125 e limpa ao desconectar |
| `LootService` (pet) | `src/zone/LootService.cpp` | No kill de quem tem pet: não abre a janela; após `loot_delay_ms`, numa thread própria, confere o raio, coleta o que passa no filtro, envia o 126 e abre a janela com as sobras |
| `MovementServer` | `src/zone/MovementServer.hpp` | Branch do 124 (dono vem do client, nunca do payload), retrato do 125 para quem entra na zone, 126 para o dono e quem está perto |
| `reload_companions` | `src/admin/ServiceAdminRegister.cpp` | Recarrega os modelos (o Manager chama depois de salvar) |
| `AUmbraCompanionActor` | `Actors/UmbraCompanionActor.cpp` | Segue o dono (atrás e ao lado, com flutuação se `hover_height` > 0), alterna parado/movendo, voa até o corpo no 126 e toca o VFX; some com o dono invisível ou fora da visão |
| GameInstance | `Core/UmbraGameInstanceCompanions.cpp` | Lista via `get_player_companions.php`, envia o 124, spawna/destrói atores no 125, atualiza bolsa/ouro e escreve no chat no 126; reinvoca o pet 1,5 s depois do opcode 109 |
| Selo | `Core/UmbraGameInstance.cpp` (`OnUseItemRequestComplete`) | Resposta com `companion_granted`: recarrega inventário e companheiros e mostra o aviso |
| Janela / linha | `UI/UmbraCompanionWindowWidget.cpp`, `UI/UmbraCompanionEntryWidget.cpp` | Monta as linhas, o combo de raridade e o checkbox; salva o filtro na hora |
| Tecla | `UmbraEternumUECharacter.cpp` / `UmbraEternumUEPlayerController.cpp` | `IA_OpenCompanions` → `ToggleCompanions` pela camada de janelas (ESC fecha) |

> **Não** adicione lógica no Event Graph de nenhum desses WBPs.

---

## 4) Protocolo

### Opcode 124 (`CompanionRequest`) — cliente → zone

| Campo | Tipo | Observação |
|-------|------|------------|
| `action` | `u8` | 0 = recolher, 1 = invocar, 2 = só atualizar o filtro |
| `companionId` | `u32` | `companion_templates.companion_id` |
| `minRarity` | `u8` | 0..4 (só usado na action 2; na invocação vale o salvo no DB) |
| `flags` | `u8` | bit0 = coletar ouro |

8 bytes fixos. O dono é o player mapeado no client WS.

### Opcode 125 (`CompanionState`) — zone → todos

`[ownerId:u32][slot:u8][companionId:u32][active:u8][scale:f32][followDist:f32][hover:f32][name][mesh][idleAnim][moveAnim][pickupVfx]`. As strings vão como `u16` + UTF-8 (nome até 64 bytes, paths até 255). `slot` 0 = pet, 1 = elemental; `active` 0 = recolhido. Também vai no retrato de quem entra na zone.

### Opcode 126 (`CompanionLootCollected`) — zone → dono + AOI

`[ownerId:u32][corpseId:u64][x,y,z:f32][playerGold:i64 (-1 = sem mudança)][goldCollected:u32][invChanged:u8][count:u8]` + `count × [itemTemplateId:u32][qty:u32]` (máx. 10). Quem não é o dono só usa a posição para o voo do pet.

Servidor: `src/zone/MovementProtocol.hpp`, `CompanionService`, `LootService`.
Cliente: `NetMovementClient.cpp` (forward 125/126; opcode 3 remove os companheiros), `UmbraGameInstanceCompanions.cpp`.

## 5) Manager — passo a passo

1. Abrir **UmbraManager** → **Companheiros** → **Atualizar**.
2. **Editar** `PET_SPRITE` (ou **Novo companheiro**) e preencher os paths `/Game/...Asset.Asset` de mesh, animações e VFX.
3. **Salvar**. O Manager envia `reload_companions` para as zones autenticadas no AdminHub. Quem já está com o pet invocado vê o visual novo ao **Recolher → Invocar**.
4. Para dar o pet a um jogador: aba **Itens** → item do tipo selo (`consumable`, `item_subtype = companion`, `stats_json = {"companion_template_id": ID}`) → entregar por GM, loja ou loot.

---

## 6) QA (PIE)

PIE com **Number of Players = 2** (A e B), zone recompilada e reiniciada.

| # | Cenário | Esperado |
|---|---------|----------|
| C1 | A recebe o **Selo da Fada Coletora** (GM/add_item) e usa no inventário | Aviso "Você obteve Fada Coletora!"; o selo some; a janela (P) lista o pet como **Recolhido** |
| C2 | A usa outro selo igual | Erro "Você já possui Fada Coletora"; o selo **não** é consumido |
| C3 | A clica **Invocar** | A esfera (ou o mesh) aparece atrás de A, flutuando; status **Invocado**; botão vira **Recolher** |
| C4 | B olha para A | B vê o pet de A seguindo A |
| C5 | B entra na zone depois de A já ter invocado | B vê o pet de A (retrato do 125) |
| C6 | A mata um mob com drop, perto do corpo | Após ~0,8 s o pet voa até o corpo e volta; itens e ouro vão para a bolsa; linha no chat; **a janela de loot não abre** |
| C7 | Filtro **Raro ou melhor** e mob que dropa só comum | O pet não pega; a janela de loot abre com tudo |
| C8 | Desmarcar **Coletar ouro** | O ouro fica na janela; os itens que passam no filtro são coletados |
| C9 | Bolsa cheia | O ouro é coletado; os itens ficam e a janela abre com eles |
| C10 | A mata de longe (corpo > 15 m de A) | A janela de loot abre normal; o pet não coleta |
| C11 | A recolhe o pet | O pet some para A e B; kills seguintes abrem a janela normal |
| C12 | A com o pet invocado reloga | O pet volta sozinho ~1,5 s depois de entrar |
| C13 | A entra em stealth | O pet some junto para B; volta quando A aparece |
| C14 | A desloga | O pet de A some para B |
| C15 | Manager: trocar a escala e **Salvar**; A recolhe e invoca | O pet aparece com o tamanho novo |

Logs esperados na zone:
- `"[CompanionService] player X invocou 1 (PET_SPRITE)"`;
- `"[LootService] pet 1 do player X coletou corpse Y: itens=N ouro=G sobrou=0"`.

No UE: `"[UmbraGameInstance] opcode 125 owner=X slot=0 companion=1 active=1"`.

## 7) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| A aba Companheiros não aparece | Manager antigo aberto | Fechar o Manager e rodar o `dotnet build` |
| Invocar não faz nada | Zone antiga (sem o 124) ou o player ainda não foi mapeado no WS | Reiniciar a zone recompilada; andar um pouco e tentar de novo; procurar `CompanionRequest ignorado` no log |
| Log `não possui o companheiro` | Selo não usado ou companheiro expirado | Usar o selo; conferir `player_companions` |
| Pet aparece como esfera | `mesh_path` vazio ou inválido | Conferir o path `/Game/...Asset.Asset`; o log `[UmbraCompanionActor] mesh_path inválido` mostra qual |
| Pet de lado/de costas | Mesh com frente diferente de +Y | O C++ gira o SkeletalMesh -90° (padrão UE); reexportar o mesh com a frente em +Y |
| Pet sem animação | Paths de animação vazios ou de outro esqueleto | Usar AnimSequence do mesmo esqueleto do mesh |
| O loot nunca abre com pet ativo | Dono offline no momento da coleta | Normal se desconectou; ao voltar, o corpo já expirou |
| P não abre nada | `Open Companions Action` vazio no Blueprint do personagem ou WBP fora do path padrão | Preencher a Input Action; definir **Companions Widget Class** no PlayerController |

## 8) Referência de código

- Servidor: `src/zone/CompanionService.{hpp,cpp}`, `src/zone/LootService.cpp` (`processPetPickup`), `src/zone/MovementServer.hpp` (`setPlayerCompanionState`), `src/zone/MovementProtocol.hpp` (124–126), `tests/test_companion_loot.cpp`
- Cliente: `Actors/UmbraCompanionActor.*`, `Core/UmbraGameInstanceCompanions.cpp`, `Data/UmbraCompanionTypes.h`, `UI/UmbraCompanionWindowWidget.*`, `UI/UmbraCompanionEntryWidget.*`
- PHP/SQL: `www/umbra_api/scripts/create_companion_system.sql`, `api/companion/get_player_companions.php`, `api/inventory/use_item.php` (branch `companion`), `api/admin/*companion_template*.php`
- Manager: `ViewModels/MainViewModel.Companions.cs`, `Views/CompanionsEditorView.xaml`, `Models/CompanionTemplateRow.cs`

**Fim do guia.**
