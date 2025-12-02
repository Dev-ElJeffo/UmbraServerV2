# 🎮 GUIA: Sistema de Criação de Itens (Admin)

## ✅ **O QUE FOI CRIADO:**

### **1. Função Helper para Verificar Admin via JWT**
- ✅ Adicionada função `verifyAdminFromJWT()` em `www/umbra_api/helpers/jwt_helper.php`
- ✅ Verifica se o token JWT é válido E se a conta tem `isadmin = 1`
- ✅ Retorna informações detalhadas sobre o status de admin

### **2. APIs Criadas:**

#### **`create_item.php`** (POST)
- ✅ Cria novos itens na tabela `item_templates`
- ✅ **REQUER:** Conta com `isadmin = 1`
- ✅ Valida todos os campos (ENUMs, tipos, valores)
- ✅ Verifica se já existe item com o mesmo nome
- ✅ Processa stats em JSON corretamente

#### **`delete_item.php`** (POST)
- ✅ Deleta itens da tabela `item_templates`
- ✅ **REQUER:** Conta com `isadmin = 1`
- ✅ Verifica se há instâncias do item no inventário antes de deletar
- ✅ Retorna erro se houver itens em uso

#### **`list_items.php`** (POST/GET)
- ✅ Lista todos os itens da tabela `item_templates`
- ✅ **REQUER:** Conta com `isadmin = 1`
- ✅ Suporta filtros: `type`, `rarity`, `search`, `equipment_slot`
- ✅ Ordena por raridade e nome

### **3. Interface HTML Visual**
- ✅ `www/umbra_api/admin/create_item.html`
- ✅ Interface completa e visual para criar itens
- ✅ Formulário com todos os campos necessários
- ✅ Seção de stats organizada (Atributos Base, Combate, Bônus)
- ✅ Lista de itens criados com opção de deletar
- ✅ Autenticação via JWT token
- ✅ Mensagens de sucesso/erro
- ✅ Design moderno e responsivo

---

## 📋 **ESTRUTURA DAS APIs:**

### **create_item.php**

**URL:** `http://localhost/umbra_api/api/admin/create_item.php`

**Método:** POST

**Body (JSON):**
```json
{
  "token": "jwt_token",
  "item_name": "Espada de Ferro",
  "item_description": "Uma espada forjada em ferro puro",
  "item_type": "weapon",
  "item_subtype": "sword",
  "icon_path": "/Game/UI/Icons/Items/ICO_Sword",
  "max_stack_size": 1,
  "equipment_slot": "main_hand",
  "required_level": 1,
  "stats": {
    "strength": 10,
    "dexterity": 5,
    "attack": 50,
    "magic_attack": 0,
    "defense": 0,
    "magic_defense": 0,
    "accuracy": 5,
    "dodge": 0,
    "critical": 10,
    "resistance": 0,
    "double_attack_rate": 0,
    "health_bonus": 0,
    "mana_bonus": 0,
    "movement": 0
  },
  "rarity": "common",
  "value": 100,
  "weight": 2.5
}
```

**Resposta (Sucesso):**
```json
{
  "success": true,
  "message": "Item 'Espada de Ferro' criado com sucesso",
  "item_id": 123,
  "item": { ... }
}
```

---

### **delete_item.php**

**URL:** `http://localhost/umbra_api/api/admin/delete_item.php`

**Método:** POST

**Body (JSON):**
```json
{
  "token": "jwt_token",
  "item_id": 123
}
```

**Resposta (Sucesso):**
```json
{
  "success": true,
  "message": "Item 'Espada de Ferro' (ID: 123) deletado com sucesso",
  "item_id": 123,
  "item_name": "Espada de Ferro"
}
```

---

### **list_items.php**

**URL:** `http://localhost/umbra_api/api/admin/list_items.php`

**Método:** POST ou GET

**Body (JSON) - Opcional:**
```json
{
  "token": "jwt_token",
  "type": "weapon",
  "rarity": "legendary",
  "search": "espada",
  "equipment_slot": "main_hand"
}
```

**Resposta:**
```json
{
  "success": true,
  "message": "Itens listados com sucesso",
  "items": [ ... ],
  "total": 10,
  "filters_applied": { ... }
}
```

---

## 🎨 **COMO USAR A INTERFACE HTML:**

### **1. Acessar a Interface:**
```
http://localhost/umbra_api/admin/create_item.html
```

### **2. Autenticação:**
1. Faça login no jogo ou via API de login
2. Copie o JWT token retornado
3. Cole o token no campo "JWT Token" na interface
4. Clique em "Salvar Token"

### **3. Criar um Item:**
1. Preencha os campos obrigatórios:
   - **Nome do Item** (obrigatório)
   - **Tipo** (obrigatório)
   - **Raridade** (obrigatório)
2. Preencha os campos opcionais conforme necessário
3. Configure os **Stats** do item (todos os campos são opcionais)
4. Clique em **"✨ Criar Item"**

### **4. Visualizar Itens:**
- Clique em **"📋 Carregar Itens"** para ver todos os itens criados
- Os itens são exibidos em cards com informações resumidas
- Cada card tem um botão **"🗑️ Deletar"** para remover o item

### **5. Deletar um Item:**
- Clique no botão **"🗑️ Deletar"** no card do item
- Confirme a ação
- ⚠️ **ATENÇÃO:** Não é possível deletar itens que estão no inventário de jogadores

---

## 📊 **CAMPOS DO FORMULÁRIO:**

### **Informações Básicas:**
- `item_name` (obrigatório) - Nome do item
- `item_description` - Descrição do item
- `item_type` (obrigatório) - Tipo: weapon, armor, consumable, material, quest, misc
- `item_subtype` - Subtipo (ex: sword, helmet, health_potion)
- `icon_path` - Caminho do ícone no Unreal Engine
- `rarity` (obrigatório) - Raridade: common, uncommon, rare, epic, legendary

### **Propriedades:**
- `max_stack_size` - Tamanho máximo de stack (padrão: 1)
- `equipment_slot` - Slot de equipamento (padrão: none)
- `required_level` - Nível mínimo para usar (padrão: 1)
- `value` - Valor em moeda (padrão: 0)
- `weight` - Peso do item (padrão: 0.0)

### **Stats (Todos Opcionais):**

**Atributos Base:**
- `strength` - Força
- `dexterity` - Destreza
- `intelligence` - Inteligência
- `vitality` - Vitalidade
- `luck` - Sorte

**Stats de Combate:**
- `attack` / `physical_attack` - Ataque Físico
- `magic_attack` - Ataque Mágico
- `defense` / `physical_defense` - Defesa Física
- `magic_defense` - Defesa Mágica
- `accuracy` - Precisão
- `dodge` - Esquiva
- `critical` - Crítico
- `resistance` - Resistência
- `double_attack_rate` - Taxa de Ataque Duplo

**Bônus:**
- `health_bonus` - Bônus de Vida
- `mana_bonus` - Bônus de Mana
- `movement` - Movimento

---

## 🔒 **SEGURANÇA:**

1. ✅ **Autenticação JWT:** Todas as APIs requerem token JWT válido
2. ✅ **Verificação de Admin:** Apenas contas com `isadmin = 1` podem acessar
3. ✅ **Validação de Dados:** Todos os campos são validados antes de inserir no banco
4. ✅ **Proteção contra Duplicatas:** Verifica se já existe item com o mesmo nome
5. ✅ **Proteção ao Deletar:** Não permite deletar itens em uso

---

## 🧪 **TESTE RÁPIDO:**

### **1. Verificar se sua conta é admin:**
```sql
SELECT id, username, isadmin FROM accounts WHERE username = 'seu_usuario';
```

### **2. Tornar uma conta admin (se necessário):**
```sql
UPDATE accounts SET isadmin = 1 WHERE username = 'seu_usuario';
```

### **3. Obter JWT Token:**
- Faça login via API de login
- Copie o token retornado
- Use na interface HTML

### **4. Testar Criação de Item:**
- Acesse `http://localhost/umbra_api/admin/create_item.html`
- Cole o token
- Preencha o formulário
- Clique em "Criar Item"

---

## 📝 **NOTAS IMPORTANTES:**

1. **Stats JSON:** A interface HTML envia os stats como objeto JSON, que é convertido para `stats_json` no banco
2. **Compatibilidade:** Os stats usam `attack` e `defense` (não `physical_attack` e `physical_defense`) para compatibilidade com o banco
3. **Valores Padrão:** Campos numéricos têm valores padrão seguros (0 ou 1)
4. **ENUMs:** Os valores de `item_type`, `rarity` e `equipment_slot` são validados contra os ENUMs do banco

---

## ✅ **PRONTO PARA USO!**

Todas as APIs e a interface estão prontas. Basta:
1. Ter uma conta com `isadmin = 1`
2. Obter um JWT token válido
3. Acessar a interface HTML
4. Começar a criar itens!

