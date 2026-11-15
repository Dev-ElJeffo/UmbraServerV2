# GUIA — Mini mapa da HUD (UE 5.6.1)

O mini mapa mostra o terreno visto de cima, ao vivo, e marca NPCs e jogadores. Tudo é feito em C++: a câmera de captura, os marcadores, o zoom e a posição salva. No Editor você só cria o `WBP_Minimap` (parent class e layout) e, se quiser, o material de máscara redonda e as texturas dos marcadores.

## Objetivo

- Mini mapa sempre visível na HUD, criado junto com o chat e a skillbar pelo `UUmbraPlayerHudWidget`.
- O mapa **gira com a câmera**: o topo é para onde a câmera olha, e a seta do centro mostra para onde o personagem está virado.
- Marcadores:

  | Tipo | Cor padrão |
  |---|---|
  | NPC hostil (atacável) | vermelho |
  | NPC neutro | branco |
  | NPC com vendor ou quest | amarelo |
  | Jogador | azul |
  | Membro da party | verde (preso na borda quando está longe) |

- **Zoom** pelos botões + / −. **Posição** arrastando pela moldura. Os dois ficam salvos no `UmbraClientSettings` (SaveGame local).
- **Sem mudança de protocolo, servidor, PHP ou Manager.** O mini mapa usa os atores que o cliente já recebe da zone, então só aparece o que está dentro da área de interesse.

---

## 0) Pré-requisitos

1. Rebuild do módulo `UmbraEternumUE` (classes novas `UmbraMinimapWidget` e `UmbraMinimapCaptureComponent`).
2. **Parar o PIE** antes de editar o Designer.

**Assets:**

| Asset | Path | Parent Class / Tipo |
|-------|------|---------------------|
| WBP_Minimap | `/Game/Widgets/HUD/WBP_Minimap` | `UmbraMinimapWidget` |
| M_MinimapMask (opcional) | `/Game/Widgets/HUD/Minimap/M_MinimapMask` | Material, domínio **User Interface** |
| Texturas dos marcadores (opcional) | `/Game/Widgets/HUD/Minimap/T_Marker_*` | Texture2D |
| Seta do jogador | `/Game/Widgets/HUD/Minimap/T_PlayerArrow` | Texture2D, apontando para **cima** |

> O HUD carrega `/Game/Widgets/HUD/WBP_Minimap.WBP_Minimap_C` sozinho. Se você salvar em outro lugar, preencha **Minimap Widget Class** nos Class Defaults do `WBP_PlayerHUD`.

---

## 1) Designer — WBP_Minimap

### 1.1 Criar o widget

1. Content Browser → `/Game/Widgets/HUD/` → botão direito → **User Interface → Widget Blueprint**.
2. Em **Pick Parent Class**, buscar **UmbraMinimapWidget**. Nome: `WBP_Minimap`.

### 1.2 Hierarquia

```
[SizeBox — Root]  (Width Override 220, Height Override 240)
  VerticalBox
    Border_DragArea        ← moldura/título para arrastar (BindWidgetOptional)
      HorizontalBox
        TextBlock "Mapa"   ← só visual
        BTN_ZoomOut        ← botão "−" (BindWidgetOptional)
        BTN_ZoomIn         ← botão "+" (BindWidgetOptional)
    SizeBox (220 x 220)
      Overlay
        Img_Map            ← terreno (BindWidgetOptional)
        Canvas_Markers     ← marcadores criados pelo C++ (BindWidgetOptional)
        Img_PlayerArrow    ← seta no centro (BindWidgetOptional)
```

### 1.3 Ajustes de cada widget

| Nome do bind | Tipo | Obrigatório | Ajustes no Details | Função |
|--------------|------|-------------|--------------------|--------|
| `Img_Map` | Image | Não (`BindWidgetOptional`) | Slot do Overlay: **Fill / Fill**. Visibility **Hit Test Invisible**. Brush vazio. | O C++ coloca a render target (ou o `M_MinimapMask`) no brush. |
| `Canvas_Markers` | Canvas Panel | Não | Slot: **Fill / Fill**, do mesmo tamanho de `Img_Map`. Visibility **Hit Test Invisible**. Sem filhos. | O C++ cria e posiciona os marcadores aqui. |
| `Img_PlayerArrow` | Image | Não | Slot: **Center / Center**, tamanho 16×16, Brush `T_PlayerArrow`. Visibility **Hit Test Invisible**. Render Transform **Pivot 0.5 / 0.5**. | O C++ só gira. |
| `BTN_ZoomIn` | Button | Não | Texto filho "+". | Aproxima (÷ Zoom Step). |
| `BTN_ZoomOut` | Button | Não | Texto filho "−". | Afasta (× Zoom Step). |
| `Border_DragArea` | Border | Não | Visibility **Visible**. | Arrastar por aqui move o mini mapa. Sem ele, o mini mapa fica fixo. |

Marque **Is Variable** em todos. Depois, **Compile** + **Save**.

> O tamanho total vem do `SizeBox` raiz: o C++ usa o tamanho desejado do widget para posicionar e manter dentro da tela.

### 1.4 Class Defaults do WBP_Minimap

| Categoria | Campo | Padrão | Observação |
|-----------|-------|--------|------------|
| Umbra\|Minimap\|Visual | **Minimap Material** | vazio | **Obrigatório na prática:** `M_MinimapMask` (seção 1.5). A captura grava alpha 0, então sem um material que ignore o alpha o terreno fica invisível e só aparecem os marcadores. |
| | **Map Texture Parameter** | `MapTexture` | Nome do parâmetro de textura no material. |
| | **Circular Clip** | ligado | Desligue se o mapa for quadrado. |
| | **Clamp Party Members To Edge** | ligado | Membro da party longe fica preso na borda. |
| | **Unlit Map** | ligado | Captura só a cor base (sem luz). Desligue para ver a iluminação real. |
| Umbra\|Minimap\|Position | **Default Margin Right** / **Default Margin Top** | 16 / 110 | Posição padrão (sem posição salva): distância da borda direita e do topo. O topo deixa livre a barra de botões da HUD. Arrastar pela moldura sobrescreve e salva. |
| Umbra\|Minimap\|Markers | **Hostile/Neutral/Service Npc Style**, **Player Style**, **Party Member Style** | cores da tabela acima | Cada um tem **Texture** (vazio = quadrado), **Color** e **Size**. |
| Umbra\|Minimap\|Zoom | **Min Zoom** / **Max Zoom** | 1000 / 12000 | Largura do mundo visível, em uu. |
| | **Zoom Step** | 1.25 | Fator por clique. |
| Umbra\|Minimap\|Performance | **Marker Update Hz** | 20 | Atualizações por segundo dos marcadores. |
| | **Capture Hz** | 10 | Capturas por segundo do terreno (custo de GPU). |
| | **Capture Resolution** | 256 | Resolução da render target. |
| | **Capture Height** | 3000 | Altura da câmera acima do jogador. O que está acima disso não aparece (ajuda com telhados altos). |

### 1.5 Material de máscara redonda (opcional)

1. Criar o material `M_MinimapMask`: **Material Domain = User Interface**, **Blend Mode = Masked** (ou Translucent).
2. Nós:
   - **TextureSampleParameter2D**, com o nome `MapTexture` (textura padrão qualquer). Ligar **RGB → Final Color**.
   - **TexCoord** − 0.5 → **Length** → 0.5 − x → × 100 → **Opacity Mask**. Fica visível só o círculo de raio 0.5, com o clip padrão de 0.333.
   - O alpha da textura **não** é usado. Isso é obrigatório, porque a captura grava alpha 0.
3. Salvar e colocar em **Minimap Material** no WBP_Minimap.

> O material só faz o visual. O recorte redondo dos marcadores é feito pelo C++ (**Circular Clip**).

---

## 2) O que o C++ já faz (não precisa de Graph)

| Peça | Arquivo | Comportamento |
|------|---------|---------------|
| `UUmbraMinimapWidget` | `Source/UmbraEternumUE/UI/UmbraMinimapWidget.cpp` | Cria a render target, a captura no pawn local (e recria no respawn), pool de marcadores, zoom, posição salva e rotação da seta. |
| `UUmbraMinimapCaptureComponent` | `Source/UmbraEternumUE/Components/UmbraMinimapCaptureComponent.cpp` | Captura ortográfica de cima, sem skeletal meshes, partículas, neblina e sombras, na frequência do widget. |
| `UUmbraPlayerHudWidget` | `Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` | Cria o `WBP_Minimap` junto com o chat e a skillbar e remove no destruct. |
| `UUmbraGameInstance` | `Core/UmbraGameInstanceClientSettings.cpp`, `Core/UmbraGameInstance.cpp` | `Get/SetMinimapZoom`, `Get/SetMinimapNormalizedPosition` (SaveGame) e `GetAllNpcActors()`. |
| `UUmbraClientSettings` | `Core/UmbraClientSettings.h` | Campos `MinimapZoom`, `MinimapPosX`, `MinimapPosY`. **Reset Defaults** nas Opções volta o mini mapa para o canto e o zoom padrão. |

> **Não** adicione lógica no Event Graph do WBP_Minimap.

---

## 3) QA (PIE)

PIE com **Number of Players = 2** (Net Mode Standalone, como nos outros testes de zone).

| Cenário | Esperado |
|---------|----------|
| Entrar no mapa | Mini mapa no canto superior direito com o terreno aparecendo. |
| Girar a câmera | O mapa gira; o topo acompanha a câmera. |
| Girar só o personagem (câmera parada) | A seta do centro gira. |
| Player 2 perto | Marcador azul na posição certa, nos dois clients. |
| Convidar para party | O marcador do outro vira verde; longe, fica preso na borda. |
| NPC hostil / vendor / quest | Vermelho / amarelo / amarelo. |
| Matar um NPC | O marcador some. |
| **+** / **−** | Zoom muda dentro de 1000..12000. Relogar mantém o zoom. |
| Arrastar pela moldura | O mini mapa se move. Relogar mantém a posição. |
| Opções → **Reset Defaults** e relogar | Volta para o canto superior direito com zoom 3000. |
| `stat unit` com e sem o mini mapa | Diferença pequena no GPU (captura a 10 Hz, 256²). |

Log esperado: `[UmbraMinimapWidget] Mini mapa construído (zoom 3000).`

## 4) Troubleshooting

| Sintoma | Causa provável | Ação |
|---------|----------------|------|
| Mini mapa não aparece | WBP em outro path ou com parent errado | Conferir `/Game/Widgets/HUD/WBP_Minimap` e **Parent Class = UmbraMinimapWidget**, ou preencher **Minimap Widget Class** no `WBP_PlayerHUD`. Ver o log `Classe não encontrada`. |
| Só aparecem os marcadores, sem terreno | **Minimap Material** vazio (alpha 0 da captura); o log avisa `Minimap Material vazio` | Colocar `M_MinimapMask` em **Minimap Material**. |
| Moldura/botões escondidos atrás da barra da HUD | Posição padrão em cima da barra | Ajustar **Default Margin Top**; se já arrastou, a posição salva vale (Opções → **Reset Defaults** volta ao padrão). |
| Mapa todo preto | Câmera dentro de geometria (teto, caverna) | Diminuir **Capture Height**. |
| Mapa escuro ou com cores lavadas | Captura sem luz em RGBA8 | Desligar **Unlit Map** para usar a cor final. |
| Mini mapa ocupa a tela toda por 1 frame | A posição é aplicada no primeiro tick | Normal; confirmar que o root é um `SizeBox` com override de tamanho. |
| Marcadores deslocados do terreno | `Canvas_Markers` com tamanho diferente de `Img_Map` | Os dois em Fill/Fill dentro do mesmo Overlay. |
| Não arrasta | Falta `Border_DragArea`, ou ele está Hit Test Invisible | Criar o Border com Visibility **Visible**. |
| Seta gira ao contrário | Textura não aponta para cima | Girar a textura (ou a imagem) para apontar para o topo. |
| Jogador longe não aparece | Fora da área de interesse da zone | Esperado: o cliente só recebe quem está perto. |
| Telhados cobrindo a cidade | A captura vê de cima | Diminuir **Capture Height** para cortar o que está acima do jogador. |

## 5) Referência de código

- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraMinimapWidget.{h,cpp}` (`UpdateMarkers`, `EnsureCapture`, `ApplySavedPosition`, `SaveCurrentPosition`)
- `UmbraEternumUE/Source/UmbraEternumUE/Components/UmbraMinimapCaptureComponent.{h,cpp}` (`CaptureAt`)
- `UmbraEternumUE/Source/UmbraEternumUE/UI/UmbraPlayerHudWidget.cpp` (`EnsurePersistentHudWidgets`)
- `UmbraEternumUE/Source/UmbraEternumUE/Core/UmbraGameInstanceClientSettings.cpp` (`Get/SetMinimapZoom`, `Get/SetMinimapNormalizedPosition`)

**Fim do guia.**
