# 📋 PLANO: Sistema de Pontos de Atributos

## 🎯 **REQUISITOS:**

### **1. Ganho por Nível:**
- ✅ 20 HP max
- ✅ 20 MP max
- ✅ 5 Phys Atk
- ✅ 5 Mag Atk
- ✅ 3 Phys Def
- ✅ 3 Mag Def
- ✅ 10 pontos livres para distribuir

### **2. Conversão de Atributos:**

#### **Strength:**
- Cada 5 pontos = 2 Phys Atk
- Cada 10 pontos = 1 Crit Atk e 1 Double Atk

#### **Dexterity:**
- Cada 5 pontos = 1 Accuracy
- Cada 10 pontos = 1 Phys Atk e 1 Dodge

#### **Intelligence:**
- Cada 5 pontos = 2 Mag Atk
- Cada 10 pontos = 1 Crit Atk e 30 Mana Bonus

#### **Vitality:**
- Cada 5 pontos = 1 Crit Res
- Cada 10 pontos = 1 Double Res e 30 HP Bonus

#### **Luck:**
- Ainda a definir

### **3. Sistema PvP:**
- Cada jogador morto = +1 PvP
- Cada jogador de outra facção morto = +1 PvP + 50 Honor

### **4. Cálculo de Dano:**
- Dano = Atk - Def (diferença)
- Pode ser Phys ou Mag

### **5. Sistema de Crítico:**
- Cada 1 de Crit Chance = 1% chance de crítico
- Crítico aumenta dano em 50%
- Chance de crítico é calculada contra Crit Res do oponente

### **6. Sistema de Double Atk:**
- Cada 1 de Double Chance = 1% chance de ataque duplo
- Double atk causa dano 2 vezes
- Chance de double é calculada contra Double Res do oponente

---

## 📋 **IMPLEMENTAÇÃO:**

### **FASE 1: Banco de Dados**
- [x] Criar tabela `player_stat_points`
- [ ] Adicionar coluna `level` para cálculo de ganhos por nível

### **FASE 2: APIs PHP**
- [ ] `distribute_stat_points.php` - Distribuir pontos
- [ ] `calculate_total_stats.php` - Calcular stats totais
- [ ] `update_pvp_honor.php` - Atualizar PvP e Honor
- [ ] Atualizar `get_character_info.php` para incluir pontos

### **FASE 3: Estruturas C++**
- [ ] `FUmbraStatPoints` - Estrutura para pontos
- [ ] Atualizar `FUmbraCharacterInfo` para incluir pontos

### **FASE 4: Funções C++**
- [ ] `CalculateTotalStats()` - Calcular stats totais
- [ ] `DistributeStatPoints()` - Distribuir pontos
- [ ] `CalculateDamage()` - Calcular dano
- [ ] `CalculateCriticalChance()` - Calcular chance de crítico
- [ ] `CalculateDoubleAttackChance()` - Calcular chance de double atk

### **FASE 5: Sistema de Combate**
- [ ] Função de cálculo de dano com crítico e double atk
- [ ] Sistema de PvP e Honor

---

## 🔧 **FÓRMULAS:**

### **Cálculo de Stats Totais:**

```
Base Stats = Stats da Classe + Pontos Distribuídos

Phys Atk = Base Phys Atk (classe) + (Nível * 5) + (Strength / 5 * 2) + (Dexterity / 10 * 1)
Mag Atk = Base Mag Atk (classe) + (Nível * 5) + (Intelligence / 5 * 2)
Phys Def = Base Phys Def (classe) + (Nível * 3)
Mag Def = Base Mag Def (classe) + (Nível * 3)

Crit Atk = (Strength / 10) + (Intelligence / 10)
Double Atk = (Strength / 10)
Accuracy = (Dexterity / 5)
Dodge = (Dexterity / 10)

Crit Res = (Vitality / 5)
Double Res = (Vitality / 10)

HP Bonus = (Vitality / 10 * 30)
Mana Bonus = (Intelligence / 10 * 30)

Max HP = Base HP (classe) + (Nível * 20) + HP Bonus
Max MP = Base MP (classe) + (Nível * 20) + Mana Bonus
```

### **Cálculo de Dano:**

```
Dano Base = Atk - Def
Se Dano Base <= 0, então Dano = 1 (dano mínimo)

Chance Crítico = Crit Atk - Crit Res (oponente)
Se Chance Crítico > 0:
  Chance = Chance Crítico %
  Se rolar crítico:
    Dano = Dano Base * 1.5

Chance Double = Double Atk - Double Res (oponente)
Se Chance Double > 0:
  Chance = Chance Double %
  Se rolar double:
    Dano = Dano * 2
```

---

## 📝 **PRÓXIMOS PASSOS:**

1. ✅ Criar tabela `player_stat_points`
2. Criar API `distribute_stat_points.php`
3. Criar API `calculate_total_stats.php`
4. Atualizar `get_character_info.php`
5. Criar estruturas C++
6. Criar funções C++

