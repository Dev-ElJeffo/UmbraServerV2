# 🎯 **GUIA: Alterar Classe Pai do Blueprint (SEM PERDER NADA)**

## 📋 **OBJETIVO**

Alterar o `BP_CharacterCreationManager` para herdar de `Umbra Character Creation Manager` (C++) **SEM perder nenhum trabalho já feito**.

---

## ✅ **PASSO A PASSO**

### **PASSO 1: Abrir o Blueprint**

1. **No Content Browser**, encontre `BP_CharacterCreationManager`
2. **Clique com botão direito** no Blueprint
3. **Selecione:** "Open"

### **PASSO 2: Alterar Classe Pai**

1. **No Editor do Blueprint**, clique no botão **"File"** (canto superior esquerdo)
2. **Selecione:** "Reparent Blueprint..."
3. **Na janela que abrir:**
   - Procure por: `Umbra Character Creation Manager`
   - **Selecione** `Umbra Character Creation Manager`
   - Clique em **"Reparent"**

### **PASSO 3: Confirmar**

1. O Unreal vai mostrar uma mensagem de confirmação
2. **Clique em "Yes"** ou "OK"
3. **Pronto!** Todas as suas variáveis, funções e eventos são preservados!

---

## ⚠️ **IMPORTANTE**

- ✅ **Todas as variáveis são preservadas**
- ✅ **Todas as funções são preservadas**
- ✅ **Todos os eventos são preservados**
- ✅ **Todo o Event Graph é preservado**
- ✅ **Todos os componentes são preservados**

**Você NÃO perde NADA!**

---

## 🔍 **VERIFICAR SE FUNCIONOU**

Após reparentar, verifique:

1. **No painel "My Blueprint" → "Functions"**
   - Você deve ver a função **"Move Camera To Selection"** disponível
   - Ela vem da classe C++ pai

2. **No Event Graph**
   - Você pode arrastar e soltar a função **"Move Camera To Selection"**
   - Ela terá os parâmetros: `Target Placeholder`, `Camera Actor`, `Duration`, `Offset X`, `Offset Z`

---

## ✅ **RESUMO**

1. Abrir `BP_CharacterCreationManager`
2. **File** → **Reparent Blueprint...**
3. Selecionar `Umbra Character Creation Manager`
4. Confirmar
5. **Pronto!** Tudo preservado + função C++ disponível

**NÃO precisa recriar nada!**

---

**Fim do Guia**

