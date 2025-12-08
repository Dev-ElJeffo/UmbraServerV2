# 🔥 **PROBLEMA ENCONTRADO: Visibility Está Como Overlap**

## ❌ **CAUSA REAL:**

Na imagem do **Details do `Collision_Box`**, o canal **Visibility** está configurado como **"Sobrepor" (Overlap)**.

**O `OnClicked` NÃO FUNCIONA se Visibility estiver como Overlap!**

O componente precisa **BLOQUEAR** o canal Visibility para o `OnClicked` funcionar.

---

## ✅ **SOLUÇÃO:**

### **No `BP_Class_Placeholder`:**

1. Selecione o componente **`Collision_Box`** (Box_Collision)
2. No painel **Details**, vá em **"Colisão" (Collision)**
3. Na tabela **"Respostas de colisão" (Collision Responses)**
4. Em **"Rastrear Respostas" (Trace Responses)**, encontre **"Visibility"**
5. ✅ **DESMARQUE** a checkbox em **"Sobrepor" (Overlap)**
6. ✅ **MARQUE** a checkbox em **"Bloquear" (Block)**

**OU:**

1. Mude **"Predefinições de colisão" (Collision Presets)** de `OverlapAllDynamic` para:
   - `BlockAll` (bloqueia tudo)
   - OU `BlockAllDynamic` (bloqueia apenas dinâmicos)
   - OU crie um preset customizado que bloqueia Visibility

---

## 🎯 **TESTE:**

1. Compile o Blueprint
2. Execute o jogo
3. Clique no personagem
4. **DEVE FUNCIONAR AGORA!**

---

## 📝 **RESUMO:**

**O problema:** Visibility estava como "Overlap" em vez de "Block"

**A solução:** Mudar Visibility para "Block" na tabela de Collision Responses

**Isso é obrigatório para OnClicked funcionar!**

---

**Fim do Guia**

