<?php
/**
 * JWT Helper - Validação de Tokens JWT compatível com AuthServer C++
 * 
 * Implementa validação JWT usando HS256 (HMAC-SHA256) para compatibilidade
 * com o JWTManager C++ do AuthServer.
 * 
 * Formato do token: header.payload.signature
 * Payload esperado: {account_id, player_id, username, iat, exp}
 */

/**
 * Valida um token JWT e retorna o payload
 * 
 * @param string $token Token JWT completo (header.payload.signature)
 * @param string $secret Chave secreta para assinatura HMAC-SHA256
 * @return array|false Payload decodificado ou false se inválido
 */
function validateJWT($token, $secret = null) {
    if (empty($token)) {
        return false;
    }
    
    // Usar chave secreta padrão (DEVE ser a mesma de helpers/jwt_helper.php)
    if ($secret === null) {
        $secret = getenv('JWT_SECRET') ?: 'umbra_eternum_secret_key_2024_very_secure';
    }
    
    // Separar token em partes
    $parts = explode('.', $token);
    if (count($parts) !== 3) {
        return false; // Formato inválido
    }
    
    list($headerEncoded, $payloadEncoded, $signatureEncoded) = $parts;
    
    // Decodificar payload para verificar expiração
    $payloadJson = base64UrlDecode($payloadEncoded);
    $payload = json_decode($payloadJson, true);
    
    if ($payload === null) {
        return false; // JSON inválido
    }
    
    // Verificar expiração
    if (isset($payload['exp']) && $payload['exp'] < time()) {
        return false; // Token expirado
    }
    
    // Verificar assinatura
    $expectedSignature = base64UrlEncode(hash_hmac('sha256', $headerEncoded . '.' . $payloadEncoded, $secret, true));
    
    if (!hash_equals($signatureEncoded, $expectedSignature)) {
        return false; // Assinatura inválida
    }
    
    return $payload;
}

/**
 * Valida token JWT e retorna resultado formatado
 * 
 * @param array $data Dados da requisição (JSON decodificado)
 * @param array $headers Headers HTTP (opcional, para Authorization header)
 * @return array ['valid' => bool, 'payload' => array|null, 'error' => string|null]
 */
function validateJWTRequest($data, $headers = null) {
    // Tentar obter token de múltiplas fontes
    $token = null;
    
    // 1. Do JSON body
    if (isset($data['token']) && !empty($data['token'])) {
        $token = $data['token'];
    }
    
    // 2. Do header Authorization (Bearer token)
    if (empty($token) && $headers !== null) {
        $authHeader = $headers['Authorization'] ?? $headers['HTTP_AUTHORIZATION'] ?? null;
        if ($authHeader && preg_match('/Bearer\s+(.*)$/i', $authHeader, $matches)) {
            $token = $matches[1];
        }
    }
    
    // 3. Do header HTTP_AUTHORIZATION (fallback)
    if (empty($token) && isset($_SERVER['HTTP_AUTHORIZATION'])) {
        $authHeader = $_SERVER['HTTP_AUTHORIZATION'];
        if (preg_match('/Bearer\s+(.*)$/i', $authHeader, $matches)) {
            $token = $matches[1];
        }
    }
    
    if (empty($token)) {
        return [
            'valid' => false,
            'payload' => null,
            'error' => 'Token não fornecido'
        ];
    }
    
    $payload = validateJWT($token);
    
    if ($payload === false) {
        return [
            'valid' => false,
            'payload' => null,
            'error' => 'Token inválido ou expirado'
        ];
    }
    
    return [
        'valid' => true,
        'payload' => $payload,
        'error' => null
    ];
}

/**
 * Decodifica Base64URL (compatível com C++ base64UrlDecode)
 * 
 * @param string $input String codificada em Base64URL
 * @return string String decodificada
 */
function base64UrlDecode($input) {
    // Converter de URL-safe para Base64 padrão
    $input = str_replace(['-', '_'], ['+', '/'], $input);
    
    // Adicionar padding
    $remainder = strlen($input) % 4;
    if ($remainder) {
        $padlen = 4 - $remainder;
        $input .= str_repeat('=', $padlen);
    }
    
    return base64_decode($input, true);
}

/**
 * Codifica para Base64URL (compatível com C++ base64UrlEncode)
 * 
 * @param string $input String para codificar
 * @return string String codificada em Base64URL
 */
function base64UrlEncode($input) {
    $encoded = base64_encode($input);
    
    // Converter para URL-safe
    $encoded = str_replace(['+', '/', '='], ['-', '_', ''], $encoded);
    
    return $encoded;
}

/**
 * Gera um token JWT compatível com JWTManager C++
 * 
 * @param int $accountId ID da conta
 * @param int $playerId ID do personagem (0 se não houver)
 * @param string $username Nome de usuário
 * @param int $expirationMinutes Minutos até expiração (padrão: 60)
 * @param string $secret Chave secreta para assinatura (padrão: lê de JWT_SECRET ou usa padrão)
 * @return string Token JWT completo (header.payload.signature)
 */
function generateJWT($accountId, $playerId, $username, $expirationMinutes = 60, $secret = null) {
    // Usar chave secreta padrão (DEVE ser a mesma de helpers/jwt_helper.php)
    if ($secret === null) {
        $secret = getenv('JWT_SECRET') ?: 'umbra_eternum_secret_key_2024_very_secure';
    }
    
    // Criar header (mesmo formato do C++)
    $header = [
        'alg' => 'HS256',
        'typ' => 'JWT'
    ];
    $headerJson = json_encode($header);
    $headerEncoded = base64UrlEncode($headerJson);
    
    // Criar payload (mesmo formato do C++)
    $now = time();
    $payload = [
        'account_id' => (int)$accountId,
        'player_id' => (int)$playerId,
        'username' => $username,
        'iat' => $now,
        'exp' => $now + ($expirationMinutes * 60)
    ];
    $payloadJson = json_encode($payload);
    $payloadEncoded = base64UrlEncode($payloadJson);
    
    // Gerar assinatura HMAC-SHA256 (mesmo formato do C++)
    $signatureInput = $headerEncoded . '.' . $payloadEncoded;
    $signature = hash_hmac('sha256', $signatureInput, $secret, true); // raw binary output
    $signatureEncoded = base64UrlEncode($signature);
    
    // Montar token completo
    $token = $headerEncoded . '.' . $payloadEncoded . '.' . $signatureEncoded;
    
    return $token;
}

