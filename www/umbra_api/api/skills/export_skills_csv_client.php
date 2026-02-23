<?php
/**
 * Umbra Eternum - Export Skills to CSV (Client-Only / Minimal)
 *
 * Gera CSV apenas com dados seguros para o cliente: identificação, nome, classe,
 * tipo, elemento, ícone e descrição flavor. Nenhum valor de jogo (cooldown, cast time,
 * custo, scaling, alcance, etc.) é exportado — esses dados ficam apenas no servidor.
 *
 * Use este CSV para importar como DataTable no Unreal com a struct FUmbraSkillDataClient.
 *
 * URL: http://localhost/umbra_api/api/skills/export_skills_csv_client.php
 */

require_once __DIR__ . '/../../config/database.php';

try {
    $pdo = getConnection();

    $stmt = $pdo->query("
        SELECT
            s.skill_id AS SkillID,
            s.skill_key AS SkillKey,
            s.skill_name AS SkillName,
            c.class_id AS ClassID,
            c.class_name AS ClassName,
            st.type_key AS SkillType,
            el.element_key AS Element,
            el.color_hex AS ElementColor,
            s.icon_path AS IconPath,
            REPLACE(REPLACE(s.description, '\"', '\"\"'), ',', ' ') AS Description
        FROM skills s
        JOIN classes c ON s.class_id = c.class_id
        JOIN skill_types st ON s.type_id = st.type_id
        JOIN skill_elements el ON s.element_id = el.element_id
        WHERE s.is_enabled = 1
        ORDER BY c.class_id, s.skill_order
    ");

    $skills = $stmt->fetchAll(PDO::FETCH_ASSOC);

    if (count($skills) === 0) {
        header('Content-Type: text/plain; charset=utf-8');
        echo "Nenhuma skill encontrada. Execute os scripts SQL do sistema de skills.";
        exit;
    }

    header('Content-Type: text/csv; charset=utf-8');
    header('Content-Disposition: attachment; filename="DT_Skills_Client.csv"');

    $output = fopen('php://output', 'w');

    // BOM UTF-8
    fprintf($output, chr(0xEF) . chr(0xBB) . chr(0xBF));

    // Primeira coluna = Row Name (---); demais = nomes das colunas da struct
    $headers = array_merge(['---'], array_keys($skills[0]));
    fputcsv($output, $headers);

    foreach ($skills as $skill) {
        $row = array_merge([$skill['SkillKey']], array_values($skill));
        fputcsv($output, $row);
    }

    fclose($output);
} catch (Exception $e) {
    error_log("Erro em export_skills_csv_client: " . $e->getMessage());
    http_response_code(500);
    echo "Erro ao exportar skills para CSV (client).";
}
