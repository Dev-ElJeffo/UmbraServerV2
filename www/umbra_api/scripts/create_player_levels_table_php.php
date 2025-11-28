<?php
/**
 * Script PHP para gerar e inserir níveis 1-50
 * Execute este script via navegador ou linha de comando
 * 
 * URL: http://localhost/umbra_api/scripts/create_player_levels_table_php.php
 */

require_once __DIR__ . '/../config/database.php';

try {
    $pdo = getDatabaseConnection();
    
    // Criar tabela
    $create_table = "
    CREATE TABLE IF NOT EXISTS player_levels (
        level_id INT UNSIGNED NOT NULL AUTO_INCREMENT,
        level_number INT UNSIGNED NOT NULL UNIQUE COMMENT 'Número do nível (1-50)',
        exp_required INT UNSIGNED NOT NULL COMMENT 'EXP necessário para alcançar este nível (EXP total acumulado)',
        exp_for_next_level INT UNSIGNED NOT NULL COMMENT 'EXP necessário para passar deste nível para o próximo',
        stat_points_gained INT UNSIGNED DEFAULT 10 COMMENT 'Pontos de atributos ganhos neste nível',
        hp_gain INT UNSIGNED DEFAULT 20 COMMENT 'HP máximo ganho neste nível',
        mp_gain INT UNSIGNED DEFAULT 20 COMMENT 'MP máximo ganho neste nível',
        phys_atk_gain INT UNSIGNED DEFAULT 5 COMMENT 'Ataque físico ganho neste nível',
        mag_atk_gain INT UNSIGNED DEFAULT 5 COMMENT 'Ataque mágico ganho neste nível',
        phys_def_gain INT UNSIGNED DEFAULT 3 COMMENT 'Defesa física ganha neste nível',
        mag_def_gain INT UNSIGNED DEFAULT 3 COMMENT 'Defesa mágica ganha neste nível',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        PRIMARY KEY (level_id),
        UNIQUE KEY unique_level_number (level_number),
        INDEX idx_level_number (level_number)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
    ";
    
    $pdo->exec($create_table);
    echo "✅ Tabela criada com sucesso!<br><br>";
    
    // Limpar dados existentes
    $pdo->exec("DELETE FROM player_levels");
    echo "✅ Dados antigos removidos!<br><br>";
    
    // Preparar statement
    $stmt = $pdo->prepare("
        INSERT INTO player_levels (
            level_number, 
            exp_required, 
            exp_for_next_level,
            stat_points_gained,
            hp_gain,
            mp_gain,
            phys_atk_gain,
            mag_atk_gain,
            phys_def_gain,
            mag_def_gain
        ) VALUES (?, ?, ?, 10, 20, 20, 5, 5, 3, 3)
    ");
    
    // Calcular e inserir níveis 1-50
    $current_exp = 0;
    $exp_for_current_level = 1000;
    
    // Level 1
    $stmt->execute([1, 0, 1000]);
    echo "✅ Level 1 inserido (EXP: 0, Próximo: 1000)<br>";
    
    $current_exp = 1000;
    
    // Níveis 2-50
    for ($level = 2; $level <= 50; $level++) {
        // Calcular EXP para próximo nível (EXP anterior * 1.15)
        $exp_for_next = (int)round($exp_for_current_level * 1.15);
        
        $stmt->execute([$level, $current_exp, $exp_for_next]);
        
        if ($level <= 10 || $level % 10 == 0) {
            echo "✅ Level $level inserido (EXP Total: $current_exp, Próximo: $exp_for_next)<br>";
        }
        
        // Atualizar para próximo nível
        $current_exp += $exp_for_next;
        $exp_for_current_level = $exp_for_next;
    }
    
    echo "<br>✅ Todos os níveis inseridos com sucesso!<br><br>";
    
    // Verificar alguns níveis
    $check = $pdo->query("
        SELECT 
            level_number,
            exp_required,
            exp_for_next_level
        FROM player_levels
        WHERE level_number IN (1, 2, 5, 10, 20, 30, 40, 50)
        ORDER BY level_number
    ");
    
    echo "<h3>Verificação - Alguns Níveis:</h3>";
    echo "<table border='1' cellpadding='5'>";
    echo "<tr><th>Nível</th><th>EXP Total</th><th>EXP Próximo</th></tr>";
    
    while ($row = $check->fetch(PDO::FETCH_ASSOC)) {
        echo "<tr>";
        echo "<td>" . $row['level_number'] . "</td>";
        echo "<td>" . number_format($row['exp_required']) . "</td>";
        echo "<td>" . number_format($row['exp_for_next_level']) . "</td>";
        echo "</tr>";
    }
    
    echo "</table>";
    
    echo "<br>✅ Script concluído com sucesso!";
    
} catch (PDOException $e) {
    echo "❌ Erro: " . $e->getMessage();
}

