# 📚 **GUIA COMPLETO: Input Actions no Unreal Engine 5**

## 🎯 **O QUE SÃO INPUT ACTIONS?**

**Input Actions** são parte do sistema **Enhanced Input** do Unreal Engine 5, que substituiu o sistema antigo de **Action Mapping** e **Axis Mapping**.

**Diferenças:**
- **Action Mapping (antigo):** Mapeia teclas diretamente para funções
- **Input Actions (novo):** Mapeia teclas para "ações" que podem ser usadas em vários lugares

---

## 📋 **PARTE 1: ENTENDENDO O SISTEMA**

### **1.1: Componentes do Sistema Enhanced Input**

**1. Input Action:**
- Define uma "ação" (ex: "Jump", "Move", "Look", "Zoom")
- Pode ser do tipo:
  - **Digital (bool)** - Botão pressionado/solto
  - **Axis1D (float)** - Valor contínuo (ex: scroll do mouse)
  - **Axis2D (Vector2D)** - Dois valores (ex: movimento WASD)

**2. Input Mapping Context:**
- Mapeia teclas para Input Actions
- Pode ter múltiplos mapeamentos (ex: W, Seta para cima, Gamepad stick)

**3. Enhanced Input Local Player Subsystem:**
- Gerencia os Input Mapping Contexts ativos
- Configurado no Player Controller ou Game Mode

---

## 📋 **PARTE 2: CRIAR INPUT ACTION DO ZERO**

### **PASSO 1: Localizar Pasta de Input**

1. **Content Browser** (aba inferior do Unreal Editor)
2. **Navegue** até a pasta **`Content/Input`**
   - Se não existir, crie: **Botão direito** → **New Folder** → Nome: `Input`
3. **Abra** a pasta `Input`

---

### **PASSO 2: Criar Input Action**

1. **Na pasta `Input`**, **botão direito** → **Input** → **Input Action**
2. **Nome:** `IA_MouseWheelZoom`
   - **Convenção:** Prefixo `IA_` indica Input Action
3. **Value Type:** Selecione **`Axis1D (float)`**
   - **Por quê?** Scroll do mouse retorna valores contínuos (positivo = scroll up, negativo = scroll down)
4. **Salve** (Ctrl+S ou botão Save no topo)

**O que você criou:**
- Um Input Action que representa "zoom com scroll do mouse"
- Tipo `Axis1D` significa que retorna um valor float (não apenas true/false)

---

### **PASSO 3: Verificar Input Action Criado**

**Após criar, você deve ver:**
- **Ícone:** Um ícone de teclado/input
- **Nome:** `IA_MouseWheelZoom`
- **Tipo:** `Axis1D (float)`

**Se não aparecer:**
- Verifique se salvou (Ctrl+S)
- Verifique se está na pasta correta
- Recarregue o Content Browser (F5)

---

## 📋 **PARTE 3: CRIAR OU USAR INPUT MAPPING CONTEXT**

### **PASSO 1: Verificar se Já Existe Input Mapping Context**

1. **Content Browser** → Pasta `Input`
2. **Procure** por arquivos com nome como:
   - `IMC_Default`
   - `IMC_Player`
   - `InputMappingContext`
   - Ou qualquer arquivo com ícone de "mapping"

**Se encontrar:**
- **Duplo clique** para abrir
- **Pule** para o PASSO 2

**Se NÃO encontrar:**
- **Crie um novo** (veja abaixo)

---

### **PASSO 2: Criar Input Mapping Context (Se Não Existir)**

1. **Na pasta `Input`**, **botão direito** → **Input** → **Input Mapping Context**
2. **Nome:** `IMC_Default` (ou `IMC_Player`)
3. **Salve** (Ctrl+S)

---

### **PASSO 3: Configurar Mapeamento no Input Mapping Context**

1. **Duplo clique** no `IMC_Default` para abrir
2. **No painel central**, você verá uma lista de mapeamentos
3. **Clique no botão `+`** (adicionar mapeamento)
4. **Configure:**
   - **Input Action:** Clique no campo → Selecione `IA_MouseWheelZoom`
   - **Key:** Clique no campo → Digite `Mouse` → Selecione **`Mouse Wheel Axis`**
   - **Scale:** `1.0` (ou `-1.0` se quiser inverter)
5. **Salve** (Ctrl+S)

**O que você fez:**
- Mapeou o scroll do mouse (`Mouse Wheel Axis`) para o Input Action `IA_MouseWheelZoom`
- Quando você usar o scroll, o Input Action será acionado com um valor float

---

## 📋 **PARTE 4: CONFIGURAR NO PLAYER CONTROLLER**

### **PASSO 1: Encontrar Player Controller**

1. **Content Browser** → Procure por:
   - `BP_PlayerController`
   - `PlayerController`
   - Ou qualquer Blueprint que seja Player Controller

**Se não encontrar:**
- Verifique no **World Settings** qual Player Controller está sendo usado
- Ou crie um novo Player Controller Blueprint

---

### **PASSO 2: Adicionar Input Mapping Context ao Player Controller**

1. **Duplo clique** no Player Controller Blueprint para abrir
2. **Event Graph** → **Event BeginPlay**
3. **Adicione:**

```
[Event BeginPlay]
  ↓
[Get Enhanced Input Local Player Subsystem]
  ↓
[Add Mapping Context]
  ├─ Mapping Context: IMC_Default (ou seu Input Mapping Context)
  ├─ Priority: 0 (ou qualquer número)
```

**Detalhamento dos nodes:**

**1. Get Enhanced Input Local Player Subsystem:**
- **Botão direito** → Procure por **`Get Enhanced Input Local Player Subsystem`**
- **Output:** `Return Value` (Enhanced Input Local Player Subsystem)

**2. Add Mapping Context:**
- **Botão direito** → Procure por **`Add Mapping Context`**
- **Target:** Conecte ao `Return Value` do `Get Enhanced Input Local Player Subsystem`
- **Input Mapping Context:** Clique no campo → Selecione `IMC_Default`
- **Priority:** `0` (número inteiro - quanto maior, maior prioridade)

---

## 📋 **PARTE 5: ATRIBUIR INPUT ACTION NO CHARACTER BLUEPRINT**

### **PASSO 1: Abrir Character Blueprint**

1. **Content Browser** → Encontre `BP_ThirdPersonCharacter` (ou seu Character Blueprint)
2. **Duplo clique** para abrir

---

### **PASSO 2: Atribuir Input Action**

1. **Details Panel** (painel direito) → Procure por **"Input"** (seção)
2. **Encontre:** `Mouse Wheel Zoom Action`
   - Deve aparecer como uma variável editável
3. **Clique** no campo (deve estar vazio ou `None`)
4. **Selecione:** `IA_MouseWheelZoom` (o Input Action que você criou)
5. **Compile** o Blueprint (botão verde no topo)
6. **Salve** (Ctrl+S)

**O que você fez:**
- Atribuiu o Input Action `IA_MouseWheelZoom` à variável `Mouse Wheel Zoom Action` do Character
- Agora, quando o scroll do mouse for usado, a função `Zoom()` será chamada

---

## 📋 **PARTE 6: VERIFICAÇÃO E TESTE**

### **CHECKLIST:**

- [ ] `IA_MouseWheelZoom` criado (tipo: `Axis1D (float)`)
- [ ] `IMC_Default` criado e configurado
- [ ] Mapeamento `Mouse Wheel Axis` → `IA_MouseWheelZoom` adicionado
- [ ] Player Controller adiciona `IMC_Default` no `BeginPlay`
- [ ] Character Blueprint tem `Mouse Wheel Zoom Action` atribuído

---

### **TESTE:**

1. **Compile** o projeto (C++ foi modificado)
2. **Compile** o Character Blueprint
3. **Compile** o Player Controller Blueprint
4. **Execute** o jogo (Play)
5. **Use o scroll do mouse:**
   - **Scroll Up** → Câmera deve se aproximar
   - **Scroll Down** → Câmera deve se afastar

---

## 🔧 **TROUBLESHOOTING:**

### **PROBLEMA 1: Scroll Não Funciona**

**Verifique:**
1. Input Action está atribuído no Character Blueprint?
2. Input Mapping Context está sendo adicionado no Player Controller?
3. Input Action é do tipo `Axis1D (float)`?
4. Mapeamento está correto (`Mouse Wheel Axis`)?

**Solução:**
- Adicione logs na função `Zoom()` para verificar se está sendo chamada
- Verifique se o Input Mapping Context está ativo

---

### **PROBLEMA 2: Zoom Invertido**

**Solução:**
- No Input Mapping Context, mude o **Scale** de `1.0` para `-1.0`

---

### **PROBLEMA 3: Zoom Muito Rápido/Lento**

**Solução:**
- No Character Blueprint, ajuste `Zoom Speed`:
  - Muito rápido → Diminua (ex: `25.0`)
  - Muito lento → Aumente (ex: `100.0`)

---

## 📝 **RESUMO VISUAL:**

```
1. Criar Input Action (IA_MouseWheelZoom)
   ↓
2. Criar/Configurar Input Mapping Context (IMC_Default)
   ├─ Adicionar mapeamento: Mouse Wheel Axis → IA_MouseWheelZoom
   ↓
3. Player Controller (BeginPlay)
   ├─ Get Enhanced Input Local Player Subsystem
   ├─ Add Mapping Context (IMC_Default)
   ↓
4. Character Blueprint
   ├─ Atribuir IA_MouseWheelZoom à variável Mouse Wheel Zoom Action
   ↓
5. Testar
   ├─ Scroll do mouse → Zoom funciona!
```

---

## ✅ **RESULTADO ESPERADO:**

**Após seguir todos os passos:**
- ✅ Scroll do mouse aumenta/diminui zoom da câmera
- ✅ Câmera gira automaticamente junto com o personagem
- ✅ Sistema Enhanced Input configurado corretamente

**Com isso, tudo deve funcionar perfeitamente!**

