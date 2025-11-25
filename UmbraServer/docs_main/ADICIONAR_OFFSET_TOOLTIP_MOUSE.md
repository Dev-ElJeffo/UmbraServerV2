# 🎯 ADICIONAR OFFSET NO MOUSE PARA TOOLTIP

## 🎯 **OBJETIVO:**

Adicionar um offset na posição do mouse para que o tooltip não fique selecionando o slot várias vezes quando o mouse está muito próximo às bordas.

---

## 📋 **ONDE ADICIONAR:**

No evento `OnMouseEnter` do `WBP_InventorySlot`, após obter a posição do mouse e antes de criar o `Vector2D`.

---

## 🔧 **IMPLEMENTAÇÃO:**

### **PASSO 1: Adicionar nós de cálculo de offset**

**Onde:** Entre `Get Mouse Position` e `Make Vector2D`

**Como fazer:**

1. Após o nó **"Get Mouse Position"** (K2Node_CallFunction_7)
2. Adicione dois nós **"Add"** (Float + Float):
   - Um para **LocationX** (X do mouse)
   - Um para **LocationY** (Y do mouse)

3. **Para LocationX:**
   - **A:** Conecte ao **LocationX** de "Get Mouse Position"
   - **B:** Crie um **Make Literal Float** com valor **20.0** (ou o offset desejado)
   - **Return Value:** Conecte ao **X** de "Make Vector2D"

4. **Para LocationY:**
   - **A:** Conecte ao **LocationY** de "Get Mouse Position"
   - **B:** Crie um **Make Literal Float** com valor **20.0** (ou o offset desejado)
   - **Return Value:** Conecte ao **Y** de "Make Vector2D"

---

## 📊 **ESTRUTURA VISUAL:**

```
Get Mouse Position
  ├─ LocationX → Add (Float + Float)
  │   ├─ A: LocationX
  │   ├─ B: 20.0 (Make Literal Float)
  │   └─ Return Value → Make Vector2D (X)
  │
  └─ LocationY → Add (Float + Float)
      ├─ A: LocationY
      ├─ B: 20.0 (Make Literal Float)
      └─ Return Value → Make Vector2D (Y)
```

---

## 🎨 **PASSO A PASSO DETALHADO:**

### **1. Adicionar primeiro "Add" (para LocationX):**

1. **Desconecte** a conexão atual entre **LocationX** de "Get Mouse Position" e **X** de "Make Vector2D"
2. Adicione um nó **"Add"** (Float + Float)
3. **Conecte:**
   - **A:** ao **LocationX** de "Get Mouse Position"
   - **B:** Crie um **Make Literal Float** → defina o valor como **20.0**
   - **Return Value:** ao **X** de "Make Vector2D"

### **2. Adicionar segundo "Add" (para LocationY):**

1. **Desconecte** a conexão atual entre **LocationY** de "Get Mouse Position" e **Y** de "Make Vector2D"
2. Adicione outro nó **"Add"** (Float + Float)
3. **Conecte:**
   - **A:** ao **LocationY** de "Get Mouse Position"
   - **B:** Crie um **Make Literal Float** → defina o valor como **20.0**
   - **Return Value:** ao **Y** de "Make Vector2D"

---

## ⚙️ **AJUSTE DO OFFSET:**

O valor **20.0** é um offset padrão. Você pode ajustar:

- **Valores menores (10.0-15.0):** Offset menor, tooltip mais próximo do mouse
- **Valores maiores (25.0-30.0):** Offset maior, tooltip mais distante do mouse

**Recomendação:** Comece com **20.0** e ajuste conforme necessário.

---

## ✅ **RESULTADO:**

Agora o tooltip aparecerá **20 pixels** à direita e **20 pixels** abaixo da posição do mouse, evitando que o mouse fique muito próximo das bordas do slot e cause múltiplas chamadas de `OnMouseEnter`.

---

## 📝 **RESUMO:**

1. **Desconecte** LocationX de "Get Mouse Position" do X de "Make Vector2D"
2. **Adicione** "Add" (Float + Float) → A: LocationX, B: 20.0 → Return Value: X de "Make Vector2D"
3. **Desconecte** LocationY de "Get Mouse Position" do Y de "Make Vector2D"
4. **Adicione** "Add" (Float + Float) → A: LocationY, B: 20.0 → Return Value: Y de "Make Vector2D"

**Pronto! O tooltip agora tem offset e não ficará selecionando o slot várias vezes.**

