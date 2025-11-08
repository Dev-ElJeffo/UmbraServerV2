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
?>

