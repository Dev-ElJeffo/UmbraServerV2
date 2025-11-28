<?php
/**
 * JWT Helper Functions
 * Funções auxiliares para validação e decodificação de tokens JWT
 */

require_once __DIR__ . '/../vendor/autoload.php';
use \Firebase\JWT\JWT;
use \Firebase\JWT\Key;

/**
 * Obtém o token JWT do header Authorization
 * 
 * @return string|null O token JWT ou null se não encontrado
 */
function getJWTFromHeader($server = []) {
    // Se $server não foi passado, usar $_SERVER global
    if (empty($server)) {
        $server = $_SERVER;
    }
    
    $headers = getallheaders();
    
    // Verificar Authorization header
    if (isset($headers['Authorization'])) {
        $authHeader = $headers['Authorization'];
        
        // Formato esperado: "Bearer {token}"
        if (preg_match('/Bearer\s+(.*)$/i', $authHeader, $matches)) {
            return $matches[1];
        }
    }
    
    // Verificar variações de caso (authorization, AUTHORIZATION, etc.)
    foreach ($headers as $key => $value) {
        if (strtolower($key) === 'authorization') {
            if (preg_match('/Bearer\s+(.*)$/i', $value, $matches)) {
                return $matches[1];
            }
        }
    }
    
    return null;
}

/**
 * Valida e decodifica um token JWT
 * 
 * @param string $token O token JWT
 * @return array|null Os dados decodificados do token ou null se inválido
 */
function validateJWT($token) {
    if (!$token) {
        return null;
    }
    
    // Chave secreta (deve ser a mesma usada na geração do token)
    // Esta chave está definida em www/umbra_api/auth/login.php
    $secret_key = "umbra_eternum_secret_key_2024_very_secure";
    
    try {
        // Decodificar token
        $decoded = JWT::decode($token, new Key($secret_key, 'HS256'));
        
        // Converter objeto stdClass para array
        $decoded_array = (array) $decoded;
        
        // Verificar se o token não expirou
        if (isset($decoded_array['exp']) && $decoded_array['exp'] < time()) {
            error_log("[JWT] Token expirado: exp={$decoded_array['exp']}, now=" . time());
            return null;
        }
        
        // Log de sucesso
        error_log("[JWT] Token válido decodificado: player_id={$decoded_array['player_id']}, account_id={$decoded_array['account_id']}");
        
        return $decoded_array;
        
    } catch (Exception $e) {
        error_log("[JWT] Erro ao validar token: " . $e->getMessage());
        return null;
    }
}

/**
 * Valida o token JWT da requisição atual e retorna os dados
 * Combina getJWTFromHeader() e validateJWT()
 * 
 * @return array|null Os dados do token ou null se inválido
 */
function validateJWTRequest($data = [], $server = []) {
    // Tentar pegar token do corpo JSON primeiro (compatível com Unreal/VaRest)
    $token = null;
    if (isset($data['token']) && !empty($data['token'])) {
        $token = $data['token'];
        error_log("[JWT] Token encontrado no corpo JSON");
    }
    // Fallback: tentar pegar do header Authorization
    else {
        $token = getJWTFromHeader($server);
        if ($token) {
            error_log("[JWT] Token encontrado no header Authorization");
        }
    }
    
    if (!$token) {
        error_log("[JWT] Nenhum token fornecido (nem no corpo, nem no header)");
        return ['valid' => false, 'error' => 'Token não fornecido'];
    }
    
    $payload = validateJWT($token);
    if (!$payload) {
        return ['valid' => false, 'error' => 'Token inválido ou expirado'];
    }
    
    return ['valid' => true, 'payload' => $payload];
}

/**
 * Gera um novo token JWT
 * 
 * @param array $payload Os dados a serem incluídos no token
 * @param int $expiration_hours Tempo de expiração em horas (padrão: 1 hora)
 * @return string O token JWT gerado
 */
function generateJWT($payload, $expiration_hours = 1) {
    $secret_key = "umbra_eternum_secret_key_2024_very_secure";
    
    $issued_at = time();
    $expiration_time = $issued_at + (3600 * $expiration_hours);
    
    $token_data = array_merge($payload, [
        'iat' => $issued_at,
        'exp' => $expiration_time
    ]);
    
    try {
        $jwt = JWT::encode($token_data, $secret_key, 'HS256');
        error_log("[JWT] Token gerado com sucesso: exp=$expiration_time");
        return $jwt;
    } catch (Exception $e) {
        error_log("[JWT] Erro ao gerar token: " . $e->getMessage());
        return null;
    }
}

/**
 * Extrai o player_id do token JWT da requisição
 * Função auxiliar comum
 * 
 * @return int|null O player_id ou null se não encontrado
 */
function getPlayerIdFromJWT() {
    $jwt_data = validateJWTRequest();
    
    if (!$jwt_data || !isset($jwt_data['player_id'])) {
        return null;
    }
    
    return (int) $jwt_data['player_id'];
}

/**
 * Extrai o account_id do token JWT da requisição
 * Função auxiliar comum
 * 
 * @return int|null O account_id ou null se não encontrado
 */
function getAccountIdFromJWT() {
    $jwt_data = validateJWTRequest();
    
    if (!$jwt_data || !isset($jwt_data['account_id'])) {
        return null;
    }
    
    return (int) $jwt_data['account_id'];
}

/**
 * Verifica se a requisição tem um token JWT válido
 * Retorna true/false sem retornar os dados
 * 
 * @return bool True se o token é válido, false caso contrário
 */
function isJWTValid() {
    return validateJWTRequest() !== null;
}
?>
