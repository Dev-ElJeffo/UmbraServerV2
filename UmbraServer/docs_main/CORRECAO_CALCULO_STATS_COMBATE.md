# 🔧 CORREÇÃO: Cálculo de Stats de Combate

## 🎯 **PROBLEMA IDENTIFICADO:**

Após a correção do cálculo de HP/MP, os stats de combate ficaram incorretos porque:

1. **Substituição ao invés de soma** - Os bônus calculados dos atributos estavam SUBSTITUINDO os stats diretos dos equipamentos ao invés de SOMAR
2. **Falta de `double_attack_resistance`** - O campo não estava sendo calculado nem retornado
3. **Campo `resistance` legado** - Precisava ser mapeado para `critical_resistance` mas também suportar `double_attack_resistance`

## ✅ **CORREÇÃO APLICADA:**

### **1. Inicialização Correta dos Stats**
Inicializar stats APENAS com base + nível, SEM bônus de atributos:
```php
$total_stats = [
    'attack' => $base_phys_atk + $level_phys_atk, // Sem bônus ainda
    'magic_attack' => $base_mag_atk + $level_mag_atk, // Sem bônus ainda
    'accuracy' => $base_accuracy, // Sem bônus ainda
    // ...
];
```

### **2. Somar Stats Diretos dos Equipamentos**
Primeiro somar TODOS os stats diretos dos equipamentos:
```php
foreach ($equipped_items as $item) {
    // Somar stats diretos
    if (isset($stats['attack'])) $total_stats['attack'] += (int)$stats['attack'];
    if (isset($stats['critical'])) $total_stats['critical'] += (int)$stats['critical'];
    // ...
}
```

### **3. SOMAR (não substituir) Bônus Calculados**
Depois, SOMAR os bônus calculados dos atributos aos stats existentes:
```php
// ANTES (ERRADO - substituía):
$total_stats['attack'] = $base_phys_atk + $level_phys_atk + $strength_phys_atk_total; // ❌ Perde stats dos equipamentos

// DEPOIS (CORRETO - soma):
$total_stats['attack'] += $strength_phys_atk_bonus + $dexterity_phys_atk_bonus; // ✅ Mantém stats dos equipamentos
```

### **4. Suporte para `double_attack_resistance`**
- Adicionado campo `double_attack_resistance` no array de stats
- Soma dos equipamentos: `if (isset($stats['double_attack_resistance']))`
- Cálculo do bônus: `$vitality_double_res_bonus = floor($total_stats['vitality'] / 10)`
- Retorno na resposta: `'double_attack_resistance' => $total_stats['double_attack_resistance']`

### **5. Mapeamento de `resistance`**
- `resistance` (legado) → `critical_resistance`
- Suporte para `critical_resistance` (novo)
- Suporte para `double_attack_resistance` (novo)

## 📋 **ARQUIVOS CORRIGIDOS:**

1. ✅ `www/umbra_api/api/character/get_character_info.php`
   - Inicialização correta dos stats (sem bônus de atributos)
   - Soma dos stats diretos dos equipamentos
   - Soma (não substituição) dos bônus calculados
   - Adicionado suporte para `double_attack_resistance`
   - Retorno de `critical_resistance` e `double_attack_resistance` na resposta

## 🧪 **TESTE:**

Com 4 acessórios do mestre (cada um: 100 attack, 100 defense, 50 critical, 50 double_atk, 50 crit_res, 50 double_res):

**Esperado:**
- Physical Attack: base + level + (4 × 100) + bônus de atributos
- Magic Attack: base + level + (4 × 100) + bônus de atributos
- Critical: base + (4 × 50) + bônus de atributos
- Double Attack: base + (4 × 50) + bônus de atributos
- Critical Resistance: base + (4 × 50) + bônus de Vitality
- Double Attack Resistance: (4 × 50) + bônus de Vitality

## 📝 **NOTA:**

O problema era que ao recalcular os bônus dos atributos, estávamos SUBSTITUINDO os valores ao invés de SOMAR. Agora:
1. Inicializamos com base + nível
2. Somamos stats diretos dos equipamentos
3. Somamos bônus calculados dos atributos

Isso garante que todos os stats sejam calculados corretamente.

