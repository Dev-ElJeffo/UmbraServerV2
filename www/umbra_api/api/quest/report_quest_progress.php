<?php
/**
 * POST /api/quest/report_quest_progress.php — DESATIVADO (Quests v2, etapa 3a).
 *
 * reach_area e use_item_at confiavam na posição enviada pelo cliente. Agora a zone decide:
 * - reach_area: QuestProgressService confere a posição real a cada 1 s;
 * - use_item_at: opcode 132 (QuestInteract), com posição real e item na bolsa.
 * Cliente antigo recebe 409 com use_via_zone = true.
 */
header('Content-Type: application/json; charset=utf-8');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST');
header('Access-Control-Allow-Headers: Content-Type, Authorization');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

http_response_code(409);
echo json_encode([
    'success' => false,
    'message' => 'Progresso de área e de uso de item agora é feito pelo servidor do jogo. Atualize o cliente.',
    'use_via_zone' => true,
], JSON_UNESCAPED_UNICODE);
