# Guia da API de Inventário - Umbra Eternum

## Visão Geral

A API de inventário permite gerenciar completamente o sistema de itens dos jogadores, incluindo:
- Visualizar inventário
- Adicionar/remover itens
- Mover itens entre slots
- Equipar/desequipar itens
- Consultar templates de itens

**Base URL**: `http://localhost/umbra_api/api/inventory/`

---

## Autenticação

Todos os endpoints (exceto `get_item_templates.php`) requerem autenticação via JWT token.

**Header obrigatório**:
```
Authorization: Bearer {seu_jwt_token}
```

---

## 📋 Endpoints Disponíveis

### 1. GET `/get_inventory.php`
Obtém o inventário completo de um jogador autenticado.

**Request**:
```http
GET /api/inventory/get_inventory.php
Authorization: Bearer {jwt_token}
```

**Response (200 OK)**:
```json
{
  "success": true,
  "message": "Inventário carregado com sucesso",
  "player": {
    "player_id": 1,
    "character_name": "Jefor",
    "level": 15
  },
  "inventory": [
    {
      "inventory_id": 1,
      "player_id": 1,
      "item_template_id": 1,
      "quantity": 1,
      "slot_index": 0,
      "is_equipped": false,
      "durability": 100.0,
      "custom_properties": {},
      "acquired_at": "2025-11-14 10:30:00",
      "item_name": "Espada de Ferro",
      "item_description": "Uma espada comum feita de ferro.",
      "item_type": "weapon",
      "item_subtype": "sword",
      "icon_path": "/Game/UI/Icons/Items/ICO_IronSword",
      "max_stack_size": 1,
      "equipment_slot": "main_hand",
      "required_level": 1,
      "stats": {
        "damage": 15,
        "attack_speed": 1.2
      },
      "rarity": "common",
      "value": 50,
      "weight": 2.5
    }
  ],
  "total_items": 8
}
```

---

### 2. POST `/add_item.php`
Adiciona um item ao inventário do jogador.

**Request**:
```http
POST /api/inventory/add_item.php
Authorization: Bearer {jwt_token}
Content-Type: application/json

{
  "item_template_id": 7,
  "quantity": 5,
  "slot_index": null
}
```

**Parâmetros**:
- `item_template_id` (int, obrigatório): ID do template do item
- `quantity` (int, opcional): Quantidade a adicionar (padrão: 1)
- `slot_index` (int, opcional): Slot específico (padrão: primeiro slot vazio)

**Comportamento**:
- Se o item for empilhável e já existir no inventário, adiciona ao stack existente
- Se `slot_index` for `null`, encontra automaticamente o primeiro slot vazio
- Valida limite de empilhamento (`max_stack_size`)

**Response (201 Created)**:
```json
{
  "success": true,
  "message": "Item adicionado ao inventário",
  "inventory_id": 25,
  "item_template_id": 7,
  "slot_index": 15,
  "quantity": 5,
  "item_name": "Poção de Vida Menor",
  "stacked": false
}
```

**Possíveis Erros**:
- `400`: Inventário cheio (50 slots)
- `400`: Quantidade excede limite de empilhamento
- `400`: Slot já está ocupado
- `404`: Template de item não encontrado

---

### 3. POST `/remove_item.php`
Remove um item (ou quantidade) do inventário.

**Request**:
```http
POST /api/inventory/remove_item.php
Authorization: Bearer {jwt_token}
Content-Type: application/json

{
  "inventory_id": 25,
  "quantity": 2
}
```

**Parâmetros**:
- `inventory_id` (int, obrigatório): ID da instância do item
- `quantity` (int, opcional): Quantidade a remover (padrão: toda a quantidade)

**Comportamento**:
- Se `quantity >= quantidade atual`, remove completamente
- Se `quantity < quantidade atual`, apenas reduz a quantidade
- **Não permite** remover itens equipados

**Response (200 OK)**:
```json
{
  "success": true,
  "message": "Quantidade do item reduzida",
  "inventory_id": 25,
  "removed_quantity": 2,
  "remaining_quantity": 3
}
```

**Possíveis Erros**:
- `400`: Tentativa de remover item equipado
- `400`: Quantidade a remover maior que disponível
- `404`: Item não encontrado

---

### 4. POST `/move_item.php`
Move um item para outro slot (ou troca dois itens de posição).

**Request**:
```http
POST /api/inventory/move_item.php
Authorization: Bearer {jwt_token}
Content-Type: application/json

{
  "inventory_id": 10,
  "target_slot_index": 25
}
```

**Parâmetros**:
- `inventory_id` (int, obrigatório): ID do item a mover
- `target_slot_index` (int, obrigatório): Índice do slot de destino (0-49)

**Comportamento**:
- Se o slot de destino estiver **vazio**: simplesmente move o item
- Se o slot de destino estiver **ocupado**: faz um **swap** (troca as posições)

**Response - Move Simples (200 OK)**:
```json
{
  "success": true,
  "message": "Item movido com sucesso",
  "inventory_id": 10,
  "from_slot": 5,
  "to_slot": 25
}
```

**Response - Swap (200 OK)**:
```json
{
  "success": true,
  "message": "Itens trocados de slot",
  "moved_item": {
    "inventory_id": 10,
    "from_slot": 5,
    "to_slot": 25
  },
  "swapped_item": {
    "inventory_id": 18,
    "from_slot": 25,
    "to_slot": 5
  }
}
```

**Possíveis Erros**:
- `400`: `slot_index` inválido (deve ser 0-49)
- `404`: Item não encontrado

---

### 5. POST `/equip_item.php`
Equipa ou desequipa um item.

**Request - Equipar**:
```http
POST /api/inventory/equip_item.php
Authorization: Bearer {jwt_token}
Content-Type: application/json

{
  "inventory_id": 10,
  "equip": true
}
```

**Request - Desequipar**:
```http
POST /api/inventory/equip_item.php
Authorization: Bearer {jwt_token}
Content-Type: application/json

{
  "inventory_id": 10,
  "equip": false
}
```

**Parâmetros**:
- `inventory_id` (int, obrigatório): ID do item
- `equip` (bool, opcional): `true` para equipar, `false` para desequipar (padrão: true)

**Comportamento ao Equipar**:
1. Verifica se o jogador tem o nível requerido
2. **Desequipa automaticamente** qualquer item no mesmo slot de equipamento
3. Equipa o novo item

**Response - Equipar (200 OK)**:
```json
{
  "success": true,
  "message": "Item equipado com sucesso",
  "inventory_id": 10,
  "item_name": "Peitoral de Aço",
  "equipment_slot": "chest",
  "is_equipped": true,
  "unequipped_other": true
}
```

**Response - Desequipar (200 OK)**:
```json
{
  "success": true,
  "message": "Item desequipado com sucesso",
  "inventory_id": 10,
  "item_name": "Peitoral de Aço",
  "equipment_slot": "chest",
  "is_equipped": false
}
```

**Possíveis Erros**:
- `400`: Item não pode ser equipado (`equipment_slot = 'none'`)
- `400`: Nível insuficiente
- `400`: Item já está equipado/desequipado
- `404`: Item não encontrado

**Slots de Equipamento**:
- `head` (Capacete)
- `chest` (Peitoral)
- `legs` (Calças)
- `feet` (Botas)
- `hands` (Luvas)
- `main_hand` (Mão Principal - Arma)
- `off_hand` (Mão Secundária - Escudo/Arma)
- `ring` (Anel)
- `amulet` (Amuleto)
- `none` (Não equipável)

---

### 6. GET `/get_item_templates.php`
Obtém todos os templates de itens disponíveis no jogo.

**⚠️ Este endpoint NÃO requer autenticação** (útil para wikis, databases públicas, etc.)

**Request**:
```http
GET /api/inventory/get_item_templates.php
```

**Query Parameters (todos opcionais)**:
- `type`: Filtrar por tipo (`weapon`, `armor`, `consumable`, `material`, `quest`, `misc`)
- `rarity`: Filtrar por raridade (`common`, `uncommon`, `rare`, `epic`, `legendary`)
- `search`: Buscar por nome ou descrição (case-insensitive)

**Exemplos de Query**:
```http
GET /api/inventory/get_item_templates.php?type=weapon
GET /api/inventory/get_item_templates.php?rarity=legendary
GET /api/inventory/get_item_templates.php?search=espada
GET /api/inventory/get_item_templates.php?type=consumable&rarity=rare
```

**Response (200 OK)**:
```json
{
  "success": true,
  "message": "Templates carregados com sucesso",
  "templates": [
    {
      "item_id": 1,
      "item_name": "Espada de Ferro",
      "item_description": "Uma espada comum feita de ferro.",
      "item_type": "weapon",
      "item_subtype": "sword",
      "icon_path": "/Game/UI/Icons/Items/ICO_IronSword",
      "max_stack_size": 1,
      "equipment_slot": "main_hand",
      "required_level": 1,
      "stats": {
        "damage": 15,
        "attack_speed": 1.2
      },
      "rarity": "common",
      "value": 50,
      "weight": 2.5,
      "created_at": "2025-11-14 08:00:00",
      "updated_at": "2025-11-14 08:00:00"
    }
  ],
  "total": 17,
  "filters_applied": {
    "type": null,
    "rarity": null,
    "search": null
  }
}
```

---

## 🎮 Integração com Unreal Engine

### Exemplo de Chamada em Blueprint (VaRest)

**1. Get Inventory**:
```
Construct JSON Request (VaRest)
  - URL: http://localhost/umbra_api/api/inventory/get_inventory.php
  - Verb: GET
  - Request Headers: 
      Authorization: Bearer {jwt_token}

On Request Complete:
  - Parse Response Content as JSON
  - For Each inventory item:
      Spawn UI Widget (WBP_InventorySlot)
      Set Item Data (icon, name, quantity, etc.)
```

**2. Add Item** (ex: ao pegar um loot):
```
Construct JSON Request (VaRest)
  - URL: http://localhost/umbra_api/api/inventory/add_item.php
  - Verb: POST
  - Request Headers: 
      Authorization: Bearer {jwt_token}
  - Request Content:
      {
        "item_template_id": 7,
        "quantity": 1
      }

On Request Complete:
  - If success:
      Show Notification "Item adicionado!"
      Refresh Inventory UI
```

---

## 📊 Estrutura de Dados

### Tipos de Itens (`item_type`)
- `weapon`: Armas
- `armor`: Armaduras
- `consumable`: Consumíveis (poções, food, etc.)
- `material`: Materiais de crafting
- `quest`: Itens de quest
- `misc`: Diversos

### Raridades (`rarity`)
- `common`: Comum (branco)
- `uncommon`: Incomum (verde)
- `rare`: Raro (azul)
- `epic`: Épico (roxo)
- `legendary`: Lendário (laranja/dourado)

### Stats JSON
Os stats são armazenados como JSON para máxima flexibilidade:

```json
{
  "damage": 50,
  "attack_speed": 1.5,
  "fire_damage": 25,
  "defense": 30,
  "health": 50,
  "speed": 15,
  "health_restore": 150,
  "mana_restore": 100,
  "strength_buff": 20,
  "duration": 300
}
```

---

## ✅ Testes com cURL

### 1. Login e obter token:
```bash
curl -X POST http://localhost/umbra_api/api/login.php \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"senha123"}'
```

### 2. Selecionar personagem e obter JWT:
```bash
curl -X POST http://localhost/umbra_api/api/character/select_character.php \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer {seu_token_de_login}" \
  -d '{"character_id":1}'
```

### 3. Ver inventário:
```bash
curl -X GET http://localhost/umbra_api/api/inventory/get_inventory.php \
  -H "Authorization: Bearer {seu_jwt_token}"
```

### 4. Adicionar item:
```bash
curl -X POST http://localhost/umbra_api/api/inventory/add_item.php \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer {seu_jwt_token}" \
  -d '{"item_template_id":7,"quantity":5}'
```

### 5. Equipar item:
```bash
curl -X POST http://localhost/umbra_api/api/inventory/equip_item.php \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer {seu_jwt_token}" \
  -d '{"inventory_id":10,"equip":true}'
```

### 6. Ver templates de itens (sem autenticação):
```bash
curl -X GET "http://localhost/umbra_api/api/inventory/get_item_templates.php?type=weapon&rarity=legendary"
```

---

## 🔒 Segurança

### Validações Implementadas
- ✅ JWT obrigatório para operações de inventário
- ✅ Verificação de propriedade do item (`player_id` no token vs `player_id` no item)
- ✅ Limite de inventário (50 slots)
- ✅ Validação de `max_stack_size`
- ✅ Proteção contra remoção de itens equipados
- ✅ Verificação de nível requerido ao equipar
- ✅ Transações SQL para operações críticas (evita estados inconsistentes)

### Próximas Melhorias de Segurança
- [ ] Rate limiting (limite de requisições por minuto)
- [ ] Logs de auditoria (rastrear quem adicionou/removeu itens)
- [ ] Validação de servidor para evitar exploits (ex: duplicação de itens)
- [ ] Webhook/evento para detecção de ações suspeitas

---

## 📝 Notas de Desenvolvimento

### Capacidade do Inventário
- **50 slots** (0-49)
- Slots 0-9: Barra rápida (recomendado, mas não forçado pela API)
- Slots 10-49: Inventário principal

### Itens Empilháveis
- Definido por `max_stack_size` no template
- `max_stack_size = 1`: Não empilhável (armas, armaduras)
- `max_stack_size > 1`: Empilhável (poções, materiais, etc.)

### Durabilidade
- Armazenada como `FLOAT` (0.0 - 100.0)
- `100.0` = novo
- `0.0` = quebrado
- Implementação da mecânica de durabilidade deve ser feita no cliente/servidor de jogo

### Custom Properties
- Campo JSON para propriedades únicas de cada instância
- Útil para: encantamentos, bônus aleatórios, nome personalizado, etc.

---

## 📞 Suporte

Para dúvidas ou problemas com a API de Inventário:
1. Verificar logs do servidor PHP: `C:\wamp64\logs\php_error.log`
2. Verificar logs do MySQL: WAMP → MySQL → Logs
3. Usar os exemplos de cURL fornecidos para testar isoladamente
4. Consultar a documentação completa: `README_NOVO.md`

---

**Data de criação**: 14/11/2025  
**Versão da API**: 1.0  
**Autor**: Dev-ElJeffo

