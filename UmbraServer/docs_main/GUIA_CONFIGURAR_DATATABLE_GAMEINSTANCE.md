# 🎮 Guia Completo: Configurar Data Table no GameInstance

**Data:** 15 de Novembro de 2025  
**Objetivo:** Configurar `DT_ItemIcons` no `UmbraGameInstance` para exibir ícones dos itens

---

## 📋 **PRÉ-REQUISITOS:**

✅ Data Table `DT_ItemIcons` criado  
✅ Pelo menos 1 ícone adicionado (ex: Item ID 1)  
✅ C++ compilado com sucesso

---

## 🎯 **PARTE 1: CONFIGURAR GAMEINSTANCE NO PROJECT SETTINGS**

### **Passo 1: Abrir Project Settings**

1. No Unreal Editor, clique em **`Edit`** (menu superior)
2. Selecione **`Project Settings`**

![Project Settings](imagem_project_settings.png)

---

### **Passo 2: Configurar GameInstance Class**

1. No painel esquerdo, procure por **`Maps & Modes`**
2. Role para baixo até encontrar a seção **`Game Instance`**
3. No campo **`Game Instance Class`**, clique no dropdown
4. Selecione **`UmbraGameInstance`** (sua classe C++)

![GameInstance Class](imagem_gameinstance_class.png)

**🔍 Se `UmbraGameInstance` não aparecer:**
- Certifique-se de que compilou o C++ com sucesso
- Reinicie o Unreal Editor
- Verifique se `UmbraGameInstance` está em `Source/UmbraEternumUE/Core/`

---

### **Passo 3: Criar Blueprint do GameInstance**

**Por que criar um Blueprint?**  
Para poder configurar variáveis públicas (`UPROPERTY`) do C++ visualmente no editor.

1. No **Content Browser**, navegue até a pasta `Content/Blueprints/`
2. Clique com o botão direito → **`Blueprint Class`**
3. Na janela que abrir, procure por **`UmbraGameInstance`**
4. Selecione **`UmbraGameInstance`** (sua classe C++)
5. Nomeie como **`BP_UmbraGameInstance`**
6. Pressione `Enter` para criar

![Create BP GameInstance](imagem_create_bp_gameinstance.png)

---

### **Passo 4: Configurar Data Table no Blueprint**

1. **Abra** `BP_UmbraGameInstance` (dê duplo clique)
2. No painel **`Details`** (lado direito), você verá uma seção **`Default`**
3. Procure pela variável **`Item Icons Data Table`** (ou `ItemIconsDataTable`)
4. Clique no dropdown ao lado dela
5. Selecione **`DT_ItemIcons`** (o Data Table que você criou)

![Configure Data Table](imagem_configure_datatable.png)

**🔍 Se `Item Icons Data Table` não aparecer:**
- Certifique-se de que o C++ tem:
  ```cpp
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
  UDataTable* ItemIconsDataTable = nullptr;
  ```
- Recompile o C++ e reabra o Blueprint

6. **Compile** o Blueprint (botão verde **`Compile`** no canto superior esquerdo)
7. **Save** (botão **`Save`** ao lado de Compile)
8. **Feche** o Blueprint

---

### **Passo 5: Configurar Project Settings para Usar o Blueprint**

1. Volte para **`Edit → Project Settings`**
2. Em **`Maps & Modes → Game Instance`**
3. Mude o **`Game Instance Class`** de `UmbraGameInstance` para **`BP_UmbraGameInstance`**

![Use BP GameInstance](imagem_use_bp_gameinstance.png)

4. **Feche** as Project Settings

---

## 🎯 **PARTE 2: RECOMPILAR C++ E TESTAR**

### **Passo 1: Recompilar C++ (correções aplicadas)**

Acabamos de corrigir 2 erros no C++:
- ❌ `item_id` → ✅ `item_template_id`
- ❌ `stats` como array → ✅ verificação de tipo

**Recompilar:**

1. **Feche** o Unreal Editor
2. No Visual Studio, clique em **`Build → Build Solution`**
3. Aguarde a compilação terminar
4. **Abra** o Unreal Editor novamente

**Ou via terminal:**
```powershell
cd D:\UmbraServerV2\UmbraEternumUE
& "D:\UE_5.6\Engine\Build\BatchFiles\Build.bat" UmbraEternumUEEditor Win64 DebugGame -Project="D:\UmbraServerV2\UmbraEternumUE\UmbraEternumUE.uproject" -WaitMutex
```

---

### **Passo 2: Testar no Jogo**

1. **Play** no editor (botão Play verde)
2. Entre no jogo com seu personagem
3. Abra o inventário (tecla configurada, ex: `I`)
4. **Verifique os logs:**

**✅ Logs esperados (SEM warnings):**
```
LogTemp: [UmbraGameInstance] 📦 [AUDIT] Tentando carregar inventário
LogVaRest: Response (200): {"success":true,"inventory":[...]}
LogTemp: [UmbraGameInstance] 🖼️ Ícone encontrado para ItemID 1
LogTemp: [UmbraGameInstance] ✅ [AUDIT] Inventário carregado com sucesso - Total Itens: 8
```

**❌ Se ainda aparecer:**
```
LogTemp: Warning: [UmbraGameInstance] ⚠️ ItemIconsDataTable não configurado!
```

**Causas possíveis:**
1. Você não configurou `BP_UmbraGameInstance` no Project Settings
2. Você não selecionou `DT_ItemIcons` no Blueprint
3. Você não salvou o Blueprint após configurar

---

## 🎯 **PARTE 3: ADICIONAR MAIS ÍCONES**

### **Passo 1: Importar Imagens de Ícones**

1. No **Content Browser**, navegue até `Content/UI/Icons/Items/`
2. Arraste suas imagens de ícones para dentro da pasta
3. Para cada imagem, configure:
   - **Texture Group:** `UI`
   - **Compression Settings:** `UserInterface2D (RGBA)`
   - **MipGenSettings:** `NoMipmaps`

---

### **Passo 2: Adicionar Linhas ao Data Table**

1. Abra `DT_ItemIcons` (duplo clique)
2. Para cada item, clique em **`+ Add`** (canto superior direito)
3. Configure:
   - **Row Name:** `Item_X` (onde X é o `item_template_id` da API)
   - **Item ID:** `X` (número inteiro, ex: `1`, `2`, `3`)
   - **Item Icon:** Selecione a textura correspondente

**Exemplo para os 8 itens do seu inventário:**

| Row Name | Item ID | Item Icon | Item (da API) |
|----------|---------|-----------|---------------|
| `Item_1` | `1` | `ICO_IronSword` | Espada de Ferro |
| `Item_7` | `7` | `ICO_HealthPotionSmall` | Poção de Vida Menor |
| `Item_9` | `9` | `ICO_ManaPotion` | Poção de Mana |
| `Item_4` | `4` | `ICO_LeatherHelmet` | Capacete de Couro |
| `Item_5` | `5` | `ICO_SteelChestplate` | Peitoral de Aço |
| `Item_11` | `11` | `ICO_IronOre` | Minério de Ferro |
| `Item_13` | `13` | `ICO_WolfLeather` | Couro de Lobo |
| `Item_16` | `16` | `ICO_GoldCoin` | Moeda de Ouro |

4. **Save** o Data Table

---

## 🎯 **PARTE 4: CRIAR WIDGETS DE INVENTÁRIO (PRÓXIMO PASSO)**

Agora que o GameInstance está configurado, você pode prosseguir com a criação dos widgets:

1. **`WBP_InventorySlot`** - Widget para cada slot (item)
2. **`WBP_Inventory`** - Widget principal do inventário (grid 5x10)

Siga o guia **`GUIA_PASSO_PASSO_WIDGETS_INVENTARIO.md`** a partir da **Etapa 2.2**.

---

## 📊 **RESUMO VISUAL DA ARQUITETURA:**

```
┌────────────────────────────────────────┐
│     Project Settings                   │
│  Game Instance Class:                  │
│  └─ BP_UmbraGameInstance (Blueprint)  │
└────────────────────────────────────────┘
              │
              ▼
┌────────────────────────────────────────┐
│   BP_UmbraGameInstance                 │
│  (Blueprint baseado em C++)            │
│                                        │
│  [Default]                             │
│  └─ Item Icons Data Table:             │
│     └─ DT_ItemIcons ◄──────────────┐  │
└────────────────────────────────────┼───┘
                                     │
                                     │
┌────────────────────────────────────┼───┐
│  DT_ItemIcons (Data Table)         │   │
│  Row Structure: FUmbraItemIconMapping  │
│                                        │
│  Rows:                                 │
│  ├─ Item_1 → ICO_IronSword            │
│  ├─ Item_7 → ICO_HealthPotionSmall    │
│  ├─ Item_9 → ICO_ManaPotion           │
│  └─ ...                                │
└────────────────────────────────────────┘
```

---

## 🐛 **TROUBLESHOOTING:**

### **Problema:** "ItemIconsDataTable não configurado"

**Soluções:**
1. Verifique se `BP_UmbraGameInstance` está configurado em **Project Settings**
2. Abra `BP_UmbraGameInstance` e verifique se `DT_ItemIcons` está selecionado
3. Compile e salve o Blueprint
4. Reinicie o Unreal Editor
5. Teste novamente

---

### **Problema:** "Ícone não encontrado para ItemID X"

**Soluções:**
1. Abra `DT_ItemIcons`
2. Verifique se existe uma linha com **Row Name** = `Item_X`
3. Verifique se o campo **Item ID** = `X` (número inteiro)
4. Verifique se **Item Icon** está selecionado (não está `None`)
5. Salve o Data Table

---

### **Problema:** UmbraGameInstance não aparece no dropdown

**Soluções:**
1. Verifique se o C++ compilou sem erros
2. Verifique se `UmbraGameInstance.h` tem `UCLASS()` antes da declaração
3. Verifique se `UmbraGameInstance` herda de `UGameInstance`
4. Feche e reabra o Unreal Editor
5. Regenere o projeto (clique direito no `.uproject` → `Generate Visual Studio project files`)

---

## ✅ **CHECKLIST COMPLETO:**

- [ ] **Project Settings** configurado com `BP_UmbraGameInstance`
- [ ] **Blueprint** `BP_UmbraGameInstance` criado
- [ ] **Data Table** `DT_ItemIcons` selecionado no Blueprint
- [ ] **Pelo menos 1 ícone** configurado (ex: Item ID 1)
- [ ] **C++ recompilado** com correções (`item_template_id` e `stats`)
- [ ] **Testado no jogo** sem warnings
- [ ] **Logs confirmam** ícone carregado: `🖼️ Ícone encontrado para ItemID X`

---

**🎉 Configuração Completa! Agora você pode criar os widgets de inventário!**

