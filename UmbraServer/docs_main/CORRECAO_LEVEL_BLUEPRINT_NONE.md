# 🔧 **CORREÇÃO: Erros "None" no Level Blueprint**

## ❌ **Problema**

Erros aparecendo no `Lvl_TestAuth`:

```
Acessado "None" ao tentar ler a propriedade CallFunc_Create_ReturnValue
Acessado "None" ao tentar ler a propriedade CallFunc_GetPlayerController_ReturnValue
```

## 🔍 **Causa**

O Level Blueprint está tentando usar objetos que ainda não foram inicializados ou que retornaram `None`:
- `Create Widget` pode retornar `None` se o widget não existir ou não carregar
- `Get Player Controller` pode retornar `None` se chamado muito cedo no `BeginPlay`

## ✅ **SOLUÇÃO COMPLETA**

### **1. Abrir Level Blueprint**

1. Abra o level `Lvl_TestAuth` no editor
2. **Blueprints** (menu superior) → **Open Level Blueprint**

### **2. Corrigir Event BeginPlay**

**SUBSTITUA** o Event BeginPlay atual por este:

```
[Event BeginPlay]
    ↓
[Delay] 0.2s  ← ADICIONAR DELAY para garantir que tudo carregou
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Is Valid?]  ← VALIDAR Player Controller
    ├─→ [TRUE] ──→ Continuar
    └─→ [FALSE] ──→ [Print String] "Player Controller não encontrado!" → STOP
    ↓
[Create Widget]
    • Class: WBP_Login  ← VERIFICAR se o widget existe!
    • Owning Player: (Player Controller validado)
    ↓
[Is Valid?]  ← VALIDAR Widget criado
    ├─→ [TRUE] ──→ Continuar
    └─→ [FALSE] ──→ [Print String] "Widget não foi criado!" → STOP
    ↓
[Add to Viewport]
    • Target: (Widget validado)
    • Z-Order: 0
    ↓
[Set Input Mode UI Only]
    • Player Controller: (Player Controller validado)
    • Widget to Focus: (Widget validado)
    ↓
[Set Show Mouse Cursor]
    • Target: (Player Controller validado)
    • Show Mouse Cursor: TRUE
```

### **3. Passo a Passo Detalhado**

#### **PASSO 1: Adicionar Delay**

1. No **Event BeginPlay**, adicione um nó **Delay**
2. Configure **Duration: 0.2**
3. Conecte **Event BeginPlay** → **Delay**

#### **PASSO 2: Obter e Validar Player Controller**

1. Adicione **Get Player Controller** (Index: 0)
2. Conecte **Delay** → **Get Player Controller**
3. Adicione **Is Valid** (nó de validação)
4. Conecte o **Return Value** do **Get Player Controller** ao **Is Valid**
5. No **Is Valid**, conecte:
   - **Is Valid** (saída booleana) → **Branch**
   - **Object** (saída do objeto) → Guarde em uma variável local (opcional)

#### **PASSO 3: Criar Widget com Validação**

1. Adicione **Create Widget**
2. Configure:
   - **Class**: `WBP_Login` (VERIFIQUE se existe no Content Browser!)
   - **Owning Player**: Conecte o **Return Value** do **Get Player Controller**
3. Adicione outro **Is Valid** para o widget criado
4. Conecte o **Return Value** do **Create Widget** ao **Is Valid**
5. No **Is Valid** do widget, conecte:
   - **Is Valid** → **Branch**
   - **Object** → Guarde em uma variável local (opcional)

#### **PASSO 4: Adicionar Widget ao Viewport**

1. Adicione **Add to Viewport**
2. Conecte o **Return Value** do **Create Widget** (validado) ao **Target**
3. Configure **Z-Order: 0**

#### **PASSO 5: Configurar Input Mode**

1. Adicione **Set Input Mode UI Only**
2. Conecte:
   - **Player Controller**: Return Value do **Get Player Controller** (validado)
   - **Widget to Focus**: Return Value do **Create Widget** (validado)

#### **PASSO 6: Mostrar Mouse Cursor**

1. Adicione **Set Show Mouse Cursor**
2. Conecte:
   - **Target**: Return Value do **Get Player Controller** (validado)
   - **Show Mouse Cursor**: TRUE

### **4. Verificações Importantes**

#### **✅ Verificar se WBP_Login Existe**

1. Abra **Content Browser**
2. Procure por `WBP_Login`
3. Se não existir:
   - Crie o widget (Right Click → User Interface → Widget Blueprint)
   - OU corrija o nome no **Create Widget** para o widget correto

#### **✅ Verificar Game Mode**

1. **Edit → Project Settings → Maps & Modes**
2. Verifique se há um **Game Mode** configurado
3. Se não houver, crie um Game Mode básico ou use o padrão

#### **✅ Verificar Player Start**

1. No level `Lvl_TestAuth`, verifique se há um **Player Start** actor
2. Se não houver, adicione um:
   - **Place Actors** → **Basic** → **Player Start**
   - Coloque na posição (0, 0, 0) ou onde desejar

### **5. Estrutura Final Recomendada**

```
[Event BeginPlay]
    ↓
[Delay] 0.2s
    ↓
[Get Player Controller] (Index: 0)
    ↓
[Is Valid?]
    ├─→ [FALSE] ──→ [Print String] "Erro: Player Controller None" → STOP
    └─→ [TRUE] ──→
        ↓
        [Create Widget]
        • Class: WBP_Login
        • Owning Player: (Player Controller)
        ↓
        [Is Valid?]
        ├─→ [FALSE] ──→ [Print String] "Erro: Widget None" → STOP
        └─→ [TRUE] ──→
            ↓
            [Add to Viewport]
            • Target: (Widget)
            ↓
            [Set Input Mode UI Only]
            • Player Controller: (Player Controller)
            • Widget to Focus: (Widget)
            ↓
            [Set Show Mouse Cursor]
            • Target: (Player Controller)
            • Show Mouse Cursor: TRUE
```

### **6. Alternativa: Usar Variáveis Locais**

Para facilitar, você pode criar variáveis locais no Event Graph:

1. **My Variables** (painel esquerdo) → **+** → Criar:
   - `PlayerController` (tipo: Player Controller)
   - `LoginWidget` (tipo: User Widget)

2. Use **Set** para atribuir valores:
   - **Set PlayerController** ← Return Value do Get Player Controller
   - **Set LoginWidget** ← Return Value do Create Widget

3. Use **Get** para acessar:
   - **Get PlayerController** → Set Input Mode UI Only
   - **Get LoginWidget** → Add to Viewport

### **7. Teste**

1. **Compile** o Level Blueprint (botão **Compile** no topo)
2. **Save** o level
3. **Play** (Alt+P)
4. Verifique o **Output Log** para mensagens de erro

### **8. Se Ainda Não Funcionar**

#### **Opção A: Mover para Player Controller**

Se o Level Blueprint continuar dando problemas, mova a lógica para o **Player Controller**:

1. Abra o **Player Controller Blueprint** (ex: `BP_ThirdPersonPlayerController`)
2. No **Event BeginPlay** do Player Controller, adicione a mesma lógica
3. Remova do Level Blueprint

#### **Opção B: Usar Timer**

Se o problema persistir, use um Timer:

```
[Event BeginPlay]
    ↓
[Set Timer by Function Name]
    • Function Name: "InitializeUI"
    • Time: 0.5s
    • Looping: FALSE
    ↓
[Custom Event: InitializeUI]
    ↓
[Get Player Controller] → [Is Valid?] → [Create Widget] → ...
```

## 📝 **Resumo**

**SEMPRE** valide objetos antes de usar:
- ✅ Use **Is Valid** antes de acessar propriedades
- ✅ Adicione **Delay** no BeginPlay para garantir inicialização
- ✅ Verifique se os widgets/classes existem no Content Browser
- ✅ Use variáveis locais para facilitar debug

**O problema principal**: Tentar usar objetos `None` sem validação!

