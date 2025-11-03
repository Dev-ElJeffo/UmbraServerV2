# 🔍 **VERIFICAÇÃO: Divisões em ProcessNextFrame**

## 📋 **INSTRUÇÕES:**

Como você confirmou que não há divisões usando `SendRateHz` em `SendMoveUpdate`, o erro de "divide by zero" deve estar em **`ProcessNextFrame`** ou em alguma função relacionada.

---

## 🔍 **PASSO A PASSO PARA VERIFICAR:**

### **1. Abrir ProcessNextFrame**

1. No **Event Graph** do `BP_NetMovementClient`
2. Localize o **Custom Event "ProcessNextFrame"**
3. Clique nele para ver seu conteúdo

### **2. Buscar Divisões**

1. **Use Ctrl+F** (busca no Blueprint)
2. Digite: **"Divide"**
3. **Todos os nós `Divide` ou `Divide_DoubleDouble` serão destacados**

### **3. Para Cada Divisão Encontrada:**

**Verifique:**
- **Qual variável está conectada ao pin `B` (divisor)?**
- **Essa variável pode ser zero?**

**Se encontrar uma divisão usando uma variável numérica:**
- Adicione proteção `Max` antes da conexão:
  ```
  Get [Variável]
    ↓
  Max (Max Double Double)
    - A: [Variável]
    - B: 1.0 (ou valor mínimo apropriado)
    ↓ (ReturnValue)
  Divide (÷)
    - B: Max_Result ← PROTEGIDO!
  ```

---

## 🔍 **VARIÁVEIS COMUNS QUE PODEM CAUSAR DIVISÃO POR ZERO:**

Procure por divisões usando:
- `DeltaTime` (pode ser 0 em alguns casos)
- Qualquer variável de taxa/frequência
- Variáveis de velocidade ou aceleração
- Variáveis de interpolação (alpha, lerp speed, etc.)
- Qualquer variável calculada que pode resultar em 0

---

## ✅ **SE NÃO ENCONTRAR NENHUMA DIVISÃO:**

Se não houver divisões em `ProcessNextFrame`, o erro pode estar:

1. **Em uma função chamada dentro de `ProcessNextFrame`:**
   - Verifique todas as funções customizadas chamadas
   - Verifique funções de biblioteca (ex: `GetOrCreatePlayerState`, etc.)

2. **Em código C++:**
   - Se você tem acesso ao código C++, verifique as funções de Blueprint Function Library
   - Especialmente `WSBinaryBPFL.cpp` e funções relacionadas

3. **Em um sistema não relacionado:**
   - Pode ser um erro de outro sistema sendo reportado incorretamente

---

## 🔧 **SOLUÇÃO TEMPORÁRIA:**

Enquanto investiga, adicione a proteção global no `BeginPlay` (documentada em `GUIA_COMPLETO_CORRECOES_BLUEPRINT.md` - PASSO 1.4) para garantir que `SendRateHz` nunca seja zero, mesmo se for resetado em runtime.

---

## 📝 **CHECKLIST:**

1. [ ] Abri o Custom Event `ProcessNextFrame`
2. [ ] Busquei por todas as divisões usando Ctrl+F "Divide"
3. [ ] Identifiquei todas as variáveis usadas como divisor
4. [ ] Verifiquei se alguma dessas variáveis pode ser zero
5. [ ] Adicionei proteção `Max` em todas as divisões encontradas
6. [ ] Verifiquei funções chamadas dentro de `ProcessNextFrame`
7. [ ] Adicionei proteção global no `BeginPlay`
