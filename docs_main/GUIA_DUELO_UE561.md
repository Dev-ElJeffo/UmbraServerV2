# GUIA — Duelo e regras de PvP (UE 5.6.1)

Duelo 1x1 combinado entre dois jogadores, com a zone decidindo tudo (etapa 6): desafio, aceite, contagem, área com bandeira, limite de 1 de HP, fuga, tempo esgotado, desistência e desconexão. **O C++ já resolve** o envio, o estado, a bandeira e o HUD. Os WBPs só definem a parent class e o layout.

## Objetivo

- Botão direito num jogador → **Duelo**. O alvo recebe um popup com **Aceitar**/**Recusar** (expira em 30 s).
- Ao aceitar: bandeira no ponto médio, marcos no limite da área e contagem de 3 s. Depois os dois podem se atacar.
- **Ninguém morre em duelo:** o golpe que mataria deixa 1 de HP e encerra o duelo.
- **Fim do duelo:**
  - **derrota** do jogador que fica com 1 de HP;
  - **fuga**: mais de 5 s fora da área;
  - **empate** quando o tempo esgota (180 s);
  - **desistência**, pelo botão do HUD;
  - **desconexão** de um dos dois;
  - **interrompido** quando um dos dois morre por outra fonte ou entra numa instância.
- **PvP fora do duelo:** desligado por padrão (`pvp_settings.open_world_pvp = 0`).
  - Atacar outro jogador fora do duelo é recusado ("Alvo não hostil").
  - No futuro, a **facção** e as **safezones** decidem isso, no mesmo ponto do código: `CombatCoreEngine::canAffectPlayer`.
- **Terceiros não interferem:**
  - quem está fora do duelo não causa dano, não cura e não aplica buff nos duelistas;
  - os duelistas também não atingem terceiros.
- **Ao fim do duelo:**
  - os DOTs e debuffs de um no outro são removidos;
  - o resultado vai para `duel_results`.
- Opcodes **50–54** (estavam reservados, sem uso). Nenhum opcode novo. Os frames de movimento continuam iguais.

## Dados / Manager

| Campo | Onde | Uso |
|-------|------|-----|
| `pvp_settings.open_world_pvp` | Manager → **PvP** → "PvP livre no mundo" | Ligado = qualquer jogador ataca qualquer jogador fora do duelo |
| `pvp_settings.duel_request_distance` | **PvP** → "Distância para desafiar" | uu (100 uu = 1 m). Padrão 1500 |
| `pvp_settings.duel_request_timeout_sec` | **PvP** → "Tempo para aceitar" | 5–120 s. Padrão 30 |
| `pvp_settings.duel_radius` | **PvP** → "Raio da área" | uu a partir da bandeira (medido em XY). Padrão 3000 |
| `pvp_settings.duel_countdown_sec` | **PvP** → "Contagem" | 0–10 s. Padrão 3 |
| `pvp_settings.duel_max_duration_sec` | **PvP** → "Duração máxima" | Esgotou = empate. Padrão 180 |
| `pvp_settings.duel_out_of_bounds_sec` | **PvP** → "Tolerância fora da área" | 1–30 s. Padrão 5 |
| `pvp_settings.duel_restore_on_end` | **PvP** → "Restaurar HP/MP" | Ligado = os dois terminam com HP/MP cheios |
| `pvp_settings.duel_flag_mesh_path` | **PvP** → "Mesh da bandeira" | `/Game/.../SM_X.SM_X`. Vazio = poste padrão |
| `pvp_settings.duel_flag_vfx_path` | **PvP** → "VFX da área" | `/Game/.../NS_X.NS_X`. Vazio = sem VFX. Recebe o parâmetro de usuário `Radius` (uu) |
| `duel_results` | (só leitura por enquanto) | Um registro por duelo: jogadores, vencedor, motivo, duração (base do ranking da arena) |

SQL (já aplicado no banco de dev):

```bat
mysql -u root -p umbra_eternum < www\umbra_api\scripts\add_pvp_duel.sql
```

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE`.
2. **Parar o PIE** antes de editar o Designer.
3. SQL acima aplicado.
4. PHP sincronizado com o WAMP (`get_pvp_settings.php`, `save_pvp_settings.php`).
5. `zone_server` recompilado e reiniciado. No log deve aparecer `[DuelService] PvP livre no mundo=0 ...`.

**Assets:**

| Asset | Path | Parent Class |
|-------|------|--------------|
| `WBP_DuelRequested` | `/Game/Widgets/UI/Duel/WBP_DuelRequested` | `UmbraRequestPopupWidget` |
| `WBP_DuelStatus` | `/Game/Widgets/UI/Duel/WBP_DuelStatus` | `UmbraDuelStatusWidget` |

A bandeira é um ator C++ (`AUmbraDuelFlagActor`) e não precisa de BP.

---

## 1) Designer — `WBP_DuelRequested` (popup do desafio)

Mesmo padrão do `WBP_PartyRequested`: o jeito mais rápido é duplicar esse WBP e renomear.

```
[Root — Border / VerticalBox]
  Text_RequestFrom     "Fulano desafiou você para um duelo"
  [HorizontalBox]
    BTN_Accept         "Aceitar"
    BTN_Decline        "Recusar"
  Btn_Close            (opcional, X)
```

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `Text_RequestFrom` | `UTextBlock` | Não | Texto do desafio (o C++ formata) |
| `BTN_Accept` | `UButton` | Não | Envia o 52 com a ação 1 (aceitar) |
| `BTN_Decline` | `UButton` | Não | Envia o 52 com a ação 0 (recusar) |
| `Btn_Close` | `UButton` | Não | Fecha o popup |

**Class Defaults → Umbra|Social → Request Kind = `Duel`** (obrigatório). Sem isso o popup fica como `Trade`: mostra "Negociar com Fulano?" e o **Aceitar** chama o aceite de troca em vez do 52.

O popup some sozinho quando o desafio expira ou quando o desafiante cancela.

## 1.1) Designer — `WBP_DuelStatus` (HUD do duelo)

```
[Root — Canvas Panel]
  [VerticalBox] (topo-centro)
    Text_DuelOpponent    "Duelo contra Fulano"
    Text_DuelTimer       "2:57"
    Btn_DuelForfeit      "Desistir"
  Text_DuelCountdown     (centro, fonte grande) "3"
  Text_DuelWarning       (abaixo do centro, cor de alerta) "Volte para a área do duelo! 4"
  Text_DuelResult        (centro, fonte grande) "Vitória!\nVocê venceu Fulano."
```

| Nome do bind | Tipo | Obrigatório | Função |
|--------------|------|-------------|--------|
| `Text_DuelOpponent` | `UTextBlock` | Não | Nome do oponente |
| `Text_DuelCountdown` | `UTextBlock` | Não | 3, 2, 1 e "Lutem!" (o C++ mostra e esconde) |
| `Text_DuelTimer` | `UTextBlock` | Não | Tempo restante (mm:ss) |
| `Text_DuelWarning` | `UTextBlock` | Não | Aviso de fuga com contagem |
| `Text_DuelResult` | `UTextBlock` | Não | Resultado no fim (fica 4 s) |
| `Btn_DuelForfeit` | `UButton` | Não | Desistir (envia o 52 com a ação 2) |

Visibilidade inicial: deixe tudo **Visible**; o C++ esconde o que não se aplica.

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| Desafio/respostas | `Core/UmbraGameInstanceDuel.cpp` | `SendDuelRequest` (menu de contexto → **Duelo**), `AcceptDuelRequest`, `DeclineDuelRequest`, `ForfeitDuel`, `CancelDuelRequest` |
| Handlers 51/53/54 | `Core/UmbraGameInstanceDuel.cpp` | Popup, bandeira e HUD no início, resultado e limpeza no fim. Respostas ao desafio aparecem como aviso no centro da tela |
| Popup | `UI/UmbraRequestPopupWidget.cpp` (tipo `Duel`), `UI/UmbraPlayerHudWidget.cpp` | Abre no 51 e fecha sozinho quando o desafio expira ou é cancelado |
| HUD | `UI/UmbraDuelStatusWidget.cpp` | Contagem, tempo, aviso de fuga (calculado localmente; quem decide é o servidor) e resultado |
| Bandeira | `Actors/UmbraDuelFlagActor.cpp` | Mesh/VFX por soft path e 48 marcos no raio |
| Classe do HUD | `UmbraGameInstance` → **Duel Status Widget Class** | Vazio = `/Game/Widgets/UI/Duel/WBP_DuelStatus` |

> **Não** adicione lógica no Event Graph.

---

## 4) Protocolo

Strings: `u16` de tamanho + UTF-8. Tudo little-endian.

| Opcode | Direção | Formato |
|--------|---------|---------|
| **50** `DuelRequest` | C→S | `[targetPlayerId:u32]` |
| **51** `DuelRequestReceived` | S→C | `[fromPlayerId:u32][expiresSec:u8][name:str≤64]` |
| **52** `DuelRequestResponse` | C→S | `[action:u8][otherPlayerId:u32]`: 0 recusar, 1 aceitar, 2 desistir, 3 cancelar o desafio enviado |
| **53** `DuelStarted` | S→C (os dois) | `[duelId:u32][p1:u32][p2:u32][flagX:f32][flagY:f32][flagZ:f32][radius:f32][countdownMs:u16][maxDurationSec:u16][oobSec:u8][opponentName:str≤64][flagMesh:str≤255][flagVfx:str≤255]` (strings trailing; frame curto = vazias) |
| **54** `DuelEnded` | S→C | `[duelId:u32][winner:u32][loser:u32][reason:u8]` |

Motivos do **54**:

| reason | Significado |
|--------|-------------|
| 1 | Derrota (oponente ficou com 1 de HP) |
| 2 | Fuga (fora da área além da tolerância) |
| 3 | Tempo esgotado (empate, winner = 0) |
| 4 | Desistência |
| 5 | Desconexão |
| 6 | Interrompido (morte por outra fonte, instância), winner = 0 |
| 10–17 | Com `duelId = 0`, resposta ao desafio: 10 recusado, 11 expirado, 12 ocupado, 13 longe demais, 14 alvo inválido, 15 não pode duelar aqui, 16 desafio enviado, 17 cancelado. `winner` = o outro jogador |

Servidor: `src/zone/MovementProtocol.hpp`, `src/zone/DuelService.{hpp,cpp}`, `CombatCoreEngine::canAffectPlayer` (ponto único de PvP) e o limite de 1 de HP em `applyPlayerDamage`.

## 5) Manager — passo a passo

1. Abrir o **UmbraManager** → aba **PvP**. Ela aparece para o perfil super.
2. Ajustar os números. Os padrões já funcionam.
3. **Salvar PvP**. A barra de status mostra "Salvo hh:mm:ss; recarregado em N/N zone(s)".

Duelos em andamento continuam com os valores antigos; os próximos usam os novos.

---

## 6) QA (PIE)

PIE com **3 clientes** (A e B duelam; C tenta interferir).

| # | Cenário | Esperado |
|---|---------|----------|
| D1 | A ataca B sem duelo | Skill recusada ("Alvo não hostil"); ataque básico não sai |
| D2 | A → botão direito em B → **Duelo** | A vê "Desafio enviado para B"; B vê o popup |
| D3 | B **Recusar** | A vê "B recusou o duelo." |
| D4 | Desafio sem resposta por 30 s | Popup de B some; os dois veem "O desafio de duelo expirou." |
| D5 | B **Aceitar** | Bandeira entre os dois, marcos no raio, contagem 3-2-1-"Lutem!" nos dois |
| D6 | Atacar durante a contagem | Nada acontece |
| D7 | Lutar até um ficar sem vida | Perdedor fica com **1 de HP**, sem tela de morte; "Vitória!"/"Derrota" por 4 s; bandeira some |
| D8 | C ataca A ou cura A durante o duelo | Recusado / a cura vira self-cast em C |
| D9 | A sai da área | Aviso "Volte para a área do duelo! 5…"; ao zerar, A perde por fuga |
| D10 | **Desistir** | Quem desistiu perde ("Você desistiu.") |
| D11 | Fechar o cliente de B no meio | A vence por desconexão |
| D12 | DOT em B no momento da vitória | O DOT some no fim; B não morre depois |
| D13 | Manager: ligar "PvP livre no mundo" e salvar | A ataca B sem duelo (comportamento antigo) |
| D14 | Tentar desafiar dentro de uma instância | "Não é possível duelar aqui." |

Logs esperados na zone:
- `[DuelService] 27 desafiou 28`;
- `[DuelService] duelo 1 iniciado: 27 x 28`;
- `[DuelService] duelo 1 encerrado: vencedor=27 perdedor=28 motivo=1 (42s)`.

Cliente: `[UmbraGameInstance] Duelo (53) ...` e `Duelo (54) ...`.

## 7) Troubleshooting

| Sintoma | Causa | Ação |
|---------|-------|------|
| Popup não abre em B | `WBP_DuelRequested` ausente ou parent errada | Criar em `/Game/Widgets/UI/Duel/` com parent `UmbraRequestPopupWidget` |
| Popup diz "Negociar com…" e aceitar não faz nada | `Request Kind` do WBP ficou `Trade` | Class Defaults → **Request Kind = Duel**; o log mostra `Aceitando troca` quando está errado |
| Duelo começa sem HUD | `WBP_DuelStatus` ausente | Log `WBP_DuelStatus não encontrado`; criar o asset ou definir **Duel Status Widget Class** |
| "Muito longe para desafiar" | Distância > `duel_request_distance` | Aproximar ou aumentar no Manager |
| Ninguém consegue atacar jogador nenhum | Comportamento esperado com o PvP livre desligado | Usar duelo, ou ligar "PvP livre no mundo" |
| No fim do duelo um jogador segue o outro | Auto-ataque ainda engajado no ex-oponente (a aproximação é local) | Corrigido: o 54 encerra o engajamento e o "seguir" (`HandleDuelEndedMessage`) |
| Perdedor morre logo depois da derrota | Golpe que já estava a caminho (hit pendente, Ripostar, reflexo) | Corrigido: por 5 s depois do fim, o dano entre os dois ex-duelistas é descartado (`DuelService::kPostDuelGraceMs`) |
| Aviso de fuga aparece, mas ele não perde | O servidor mede a posição dele; o cliente pode estar adiantado | Normal com tolerância; conferir `duel_out_of_bounds_sec` |
| Zone avisa `tabela pvp_settings ausente` | SQL não aplicado | Rodar `add_pvp_duel.sql` e reiniciar a zone (ou **Salvar PvP**) |

## 8) Referência de código

- **Servidor:**
  - `src/zone/DuelService.{hpp,cpp}`;
  - `src/zone/MovementProtocol.hpp` (50–54);
  - `src/zone/MovementServer.hpp` (`setDuelHandlers`, `tryGetPlayerBrief`);
  - `src/zone/CombatCoreEngine.{hpp,cpp}` (`canAffectPlayer`, `enqueueDuelCleanup`, 1 de HP);
  - `src/zone/ZoneServer.{hpp,cpp}`;
  - `src/admin/ServiceAdminRegister.cpp` (`reload_pvp_settings`);
  - `tests/test_duel_rules.cpp`.
- **PHP/SQL:**
  - `scripts/add_pvp_duel.sql`;
  - `api/admin/get_pvp_settings.php`;
  - `api/admin/save_pvp_settings.php`.
- **Manager:**
  - `ViewModels/MainViewModel.Pvp.cs`;
  - `Views/Admin/PvpView.xaml`;
  - `PhpAdminClient.cs`;
  - `MainWindow.xaml`.
- **UE:**
  - `Core/UmbraGameInstanceDuel.cpp`;
  - `Data/UmbraDuelTypes.h`;
  - `UI/UmbraDuelStatusWidget.*`;
  - `Actors/UmbraDuelFlagActor.*`;
  - `UI/UmbraRequestPopupWidget.*`;
  - `UI/UmbraPlayerHudWidget.*`;
  - `Network/NetMovementClient.cpp`.
