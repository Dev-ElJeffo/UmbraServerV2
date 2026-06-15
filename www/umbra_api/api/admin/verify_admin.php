<?php
// Helper para verificar se usuário é admin
// Retorna array com status e dados do usuário

function verifyAdmin($db, $username) {
    try {
        $query = "SELECT id, username, email, isadmin, banned 
                 FROM accounts WHERE username = :username";
        $stmt = $db->prepare($query);
        $stmt->bindParam(':username', $username);
        $stmt->execute();
        
        if ($stmt->rowCount() > 0) {
            $account = $stmt->fetch(PDO::FETCH_ASSOC);
            
            if ($account['banned']) {
                return ['success' => false, 'message' => 'Conta banida'];
            }
            
            if ($account['isadmin'] != 1) {
                return ['success' => false, 'message' => 'Acesso negado. Apenas administradores'];
            }
            
            return [
                'success' => true,
                'admin' => [
                    'id' => $account['id'],
                    'username' => $account['username'],
                    'email' => $account['email']
                ]
            ];
        } else {
            return ['success' => false, 'message' => 'Usuário não encontrado'];
        }
    } catch (Exception $e) {
        return ['success' => false, 'message' => 'Erro: ' . $e->getMessage()];
    }
}

function getAdminSharedSecret() {
    static $cached = null;
    if ($cached !== null) {
        return $cached;
    }

    $env = getenv('UMBRA_ADMIN_SHARED_SECRET');
    if (is_string($env) && $env !== '') {
        $cached = $env;
        return $cached;
    }

    $candidates = [
        __DIR__ . '/../../../config/server.json',
        __DIR__ . '/../../../../config/server.json',
    ];

    foreach ($candidates as $path) {
        if (!is_readable($path)) {
            continue;
        }
        $json = json_decode(file_get_contents($path), true);
        if (is_array($json) && !empty($json['admin']['shared_secret'])) {
            $cached = (string)$json['admin']['shared_secret'];
            return $cached;
        }
    }

    $cached = '';
    return $cached;
}

function verifyAdminManagerSecret($providedSecret) {
    $expected = getAdminSharedSecret();
    if ($expected === '') {
        return ['success' => false, 'message' => 'Admin shared_secret não configurado no servidor'];
    }
    if (!is_string($providedSecret) || $providedSecret === '') {
        return ['success' => false, 'message' => 'admin_secret é obrigatório'];
    }
    if (!hash_equals($expected, $providedSecret)) {
        return ['success' => false, 'message' => 'admin_secret inválido'];
    }
    return ['success' => true];
}

function authenticateAdminRequest(array $data, array $server = []) {
    if (!empty($data['admin_username'])) {
        $secretCheck = verifyAdminManagerSecret($data['admin_secret'] ?? '');
        if (empty($secretCheck['success'])) {
            return array_merge($secretCheck, ['http_code' => 403]);
        }

        try {
            $database = new Database();
            $db = $database->connect();
            $adminCheck = verifyAdmin($db, $data['admin_username']);
            if (empty($adminCheck['success'])) {
                return array_merge($adminCheck, ['http_code' => 403]);
            }
            return $adminCheck;
        } catch (Exception $e) {
            return [
                'success' => false,
                'message' => 'Erro de autenticação: ' . $e->getMessage(),
                'http_code' => 500,
            ];
        }
    }

    if (!function_exists('verifyAdminFromJWT')) {
        require_once __DIR__ . '/../../helpers/jwt_helper.php';
    }

    $jwtAdmin = verifyAdminFromJWT($data, $server);
    if (empty($jwtAdmin['valid'])) {
        return [
            'success' => false,
            'message' => $jwtAdmin['error'] ?? 'Token inválido ou expirado (forneça admin_username + admin_secret ou token admin)',
            'http_code' => 401,
        ];
    }

    return [
        'success' => true,
        'admin' => $jwtAdmin['account'] ?? null,
    ];
}

function authenticateAdminObjectRequest($data, array $server = []) {
    $payload = [
        'admin_username' => $data->admin_username ?? '',
        'admin_secret' => $data->admin_secret ?? '',
        'token' => $data->token ?? null,
    ];
    return authenticateAdminRequest($payload, $server);
}
?>
