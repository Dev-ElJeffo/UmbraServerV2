# Guia UE 5.6.1 — Parar flicker do nameplate remoto (`BP_RemotePlayer`)

## Causa

O `BP_RemotePlayer` tem **duas** formas de desenhar o mesmo `WBP_PlayerNameplate`:

1. **WidgetComponent** no pawn (já desenha o WBP em tela / mundo)
2. **Event BeginPlay** → `Create Widget` → `Set Widget` → **`AddToViewport`**

Isso gera **duas instâncias** sobrepostas na cabeça. A cada hit o layout invalida e as placas “piscam” / trocam de posição (uma com buffs, outra sem — se só o WC recebe sync C++).

O C++ remove órfãos de viewport (segunda instância). O conserto definitivo no Editor é limpar o Blueprint (sem `AddToViewport`).

**Não** force `WidgetSpace::World` nem `RemoveFromParent` no widget do WC pelo C++: isso tornava o nameplate Screen/viewport **invisível** só nos remotos.

---

## 1) Abrir `BP_RemotePlayer`

Path: `/Game/Blueprints/Player_BP/BP_RemotePlayer`

**Parar o PIE** antes de editar.

---

## 2) Event Graph — BeginPlay

Localize o fluxo após `Event BeginPlay` que contém:

- `Create Widget` (Class = `WBP_PlayerNameplate`)
- `Set Widget` (no WidgetComponent)
- **`Add To Viewport`**

### Remover

1. Apague o nó **`Add To Viewport`** (obrigatório).
2. Apague **`Create Widget`** se o WidgetComponent já tem **Widget Class** = `WBP_PlayerNameplate` nos defaults (não precisa criar de novo).
3. Apague **`Set Widget`** se o class já está no componente.

O BeginPlay do remoto **não** deve criar nameplate em viewport.

Compile + Save.

---

## 3) WidgetComponent (Details)

No componente de nameplate (geralmente `Widget`):

| Propriedade | Valor |
|-------------|--------|
| **Widget Class** | `WBP_PlayerNameplate` (parent `UmbraRemoteNameplateWidget`) |
| **Space** | **World** recomendado no Editor (não Screen) — ajustar no BP, não via C++ |
| **Draw Size** | ~256×128 (ajustar) |
| **Pivot** | centro / base conforme arte |
| Attach | socket/cabeça |

Só deve existir **um** WidgetComponent de nameplate na hierarquia.

---

## 4) Não fazer no hit

No Event Graph (remoto ou nameplate), **não** ligue `OnCombatEvent` / vitals a:

- `Create Widget`
- `Add To Viewport`
- `Init Widget` no WC

HP do alvo selecionado é o HUD `WBP_SelectedPlayerInfo` (topo da tela), separado do nameplate 3D.

---

## 5) QA

1. PIE 2 clients → World Outliner: **1** `BP_RemotePlayer` por jogador remoto.
2. Sem hit: **uma** placa sobre a cabeça do remote em **ambos** os clients.
3. Hit: ainda **uma** placa; sem flicker esquerda/direita; `-942` flutuante ok.
4. Output Log: se o BP ainda AddToViewport de uma **segunda** instância, aparece  
   `Removendo nameplate órfão do viewport (CreateWidget/AddToViewport duplicado)`.

---

## Defesa C++ (já no código)

- `SanitizeRemotePlayerNameplatePresentation` no register remoto
- Esconde WidgetComponents não preferidos (evita LifeBar fantasma)
- Remove `UUmbraRemoteNameplateWidget` **órfão** em viewport (não o widget do WC preferido)
- `FindPreferred` também reconhece WC cujo `GetUserWidgetObject()` já é `UmbraRemoteNameplateWidget`
- **Não** força `EWidgetSpace::World` nem `RemoveFromParent` no WC preferido (isso quebrava Screen/viewport)
- Strip de órfãos filtrado por `UWorld` (PIE multi-client)
