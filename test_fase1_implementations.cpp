// Teste das implementações da Fase 1
// JWT HMAC-SHA256, Bcrypt (PBKDF2) e MySQL Connector

#include "src/core/Utils.hpp"
#include "src/auth/JWTManager.hpp"
#include "src/database/MySQLConnector.hpp"
#include "src/core/Logger.hpp"
#include <iostream>
#include <cassert>

using namespace Umbra::Core;
using namespace Umbra::Auth;
using namespace Umbra::Database;

void testPasswordHashing() {
    std::cout << "\n=== TESTE 1: Password Hashing (PBKDF2) ===\n";
    
    std::string password = "senha123";
    std::string hash = Utils::hashPassword(password);
    
    std::cout << "Password: " << password << "\n";
    std::cout << "Hash gerado: " << hash << "\n";
    
    // Verificar que o hash tem o formato correto
    assert(hash.substr(0, 8) == "$pbkdf2$");
    std::cout << "✓ Formato do hash está correto\n";
    
    // Verificar senha correta
    bool verified = Utils::verifyPassword(password, hash);
    assert(verified);
    (void)verified; // Suprimir warning de variável não usada
    std::cout << "✓ Verificação de senha correta: PASS\n";
    
    // Verificar senha incorreta
    bool wrongVerified = Utils::verifyPassword("senha_errada", hash);
    assert(!wrongVerified);
    (void)wrongVerified; // Suprimir warning
    std::cout << "✓ Verificação de senha incorreta: PASS (rejeitada corretamente)\n";
    
    // Testar com salt específico
    std::string hashWithSalt = Utils::hashPassword(password, "salt_teste");
    bool verifiedWithSalt = Utils::verifyPassword(password, hashWithSalt);
    assert(verifiedWithSalt);
    (void)verifiedWithSalt; // Suprimir warning
    std::cout << "✓ Hash com salt específico: PASS\n";
    
    std::cout << "✅ TESTE 1 PASSOU!\n";
}

void testJWTGeneration() {
    std::cout << "\n=== TESTE 2: JWT HMAC-SHA256 ===\n";
    
    std::string secretKey = "UmbraEternum2025SecretKey123456789";
    JWTManager jwtManager(secretKey);
    
    // Gerar token
    std::string token = jwtManager.generateToken(
        1,      // accountId
        0,      // playerId
        "testuser",  // username
        60      // 60 minutos
    );
    
    std::cout << "Token gerado: " << token.substr(0, 50) << "...\n";
    
    // Verificar formato (header.payload.signature)
    size_t dot1 = token.find('.');
    size_t dot2 = token.find('.', dot1 + 1);
    assert(dot1 != std::string::npos);
    assert(dot2 != std::string::npos);
    (void)dot2; // Suprimir warning
    std::cout << "✓ Formato do token está correto (tem 3 partes separadas por '.')\n";
    
    // Validar token
    auto payload = jwtManager.validateToken(token);
    assert(payload.has_value());
    assert(payload->accountId == 1);
    assert(payload->username == "testuser");
    std::cout << "✓ Token válido e payload correto\n";
    std::cout << "  - Account ID: " << payload->accountId << "\n";
    std::cout << "  - Username: " << payload->username << "\n";
    
    // Testar token inválido (assinatura errada)
    std::string invalidToken = token;
    invalidToken[invalidToken.length() - 1] = 'X'; // Modificar último caractere
    auto invalidPayload = jwtManager.validateToken(invalidToken);
    assert(!invalidPayload.has_value());
    std::cout << "✓ Token com assinatura inválida foi rejeitado corretamente\n";
    
    // Testar revogação
    bool revoked = jwtManager.revokeToken(token);
    assert(revoked);
    (void)revoked; // Suprimir warning
    auto revokedPayload = jwtManager.validateToken(token);
    assert(!revokedPayload.has_value());
    std::cout << "✓ Token revogado não é mais válido\n";
    
    std::cout << "✅ TESTE 2 PASSOU!\n";
}

void testMySQLConnection() {
    std::cout << "\n=== TESTE 3: MySQL Connector ===\n";
    
    MySQLConnector::Config config;
    config.host = "localhost";
    config.port = 3306;
    config.database = "umbra_eternum";
    config.username = "root";
    config.password = "!Mister4126";
    config.connectionTimeout = 10;
    config.autoReconnect = true;
    
    MySQLConnector connector(config);
    
    // Tentar conectar
    bool connected = connector.connect();
    if (!connected) {
        std::cout << "⚠️  Não foi possível conectar ao MySQL.\n";
        std::cout << "   Verifique se o MySQL está rodando e as credenciais estão corretas.\n";
        std::cout << "   Este teste será pulado.\n";
        return;
    }
    
    std::cout << "✓ Conexão com MySQL estabelecida\n";
    
    // Testar query simples
    bool result = connector.execute("SELECT 1");
    assert(result);
    (void)result; // Suprimir warning
    std::cout << "✓ Execução de query simples: PASS\n";
    
    // Testar query escalar
    auto scalar = connector.executeScalar("SELECT CONCAT('Hello, ', 'World')");
    assert(scalar.has_value());
    assert(scalar.value() == "Hello, World");
    std::cout << "✓ Query escalar retornou: " << scalar.value() << "\n";
    
    // Testar query de seleção (se tabela existir)
    auto count = connector.executeScalar("SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = 'umbra_eternum'");
    if (count.has_value()) {
        std::cout << "✓ Query de contagem retornou: " << count.value() << " tabelas\n";
    }
    
    // Desconectar
    connector.disconnect();
    std::cout << "✓ Desconexão do MySQL: PASS\n";
    
    std::cout << "✅ TESTE 3 PASSOU!\n";
}

int main() {
    // Forçar flush imediato da saída
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    std::cout << "========================================\n";
    std::cout << "TESTE DAS IMPLEMENTACOES FASE 1\n";
    std::cout << "========================================\n";
    std::cout.flush();
    
    try {
        // Inicializar logger se necessário
        Umbra::Core::Logger::getInstance().initialize("logs/test_fase1.log");
        
        std::cout << "Iniciando teste de password hashing...\n";
        std::cout.flush();
        testPasswordHashing();
        
        std::cout << "Iniciando teste de JWT...\n";
        std::cout.flush();
        testJWTGeneration();
        
        std::cout << "Iniciando teste de MySQL...\n";
        std::cout.flush();
        testMySQLConnection();
        
        std::cout << "\n========================================\n";
        std::cout << "✅ TODOS OS TESTES PASSARAM!\n";
        std::cout << "========================================\n";
        std::cout.flush();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ ERRO: " << e.what() << "\n";
        std::cerr.flush();
        return 1;
    } catch (...) {
        std::cerr << "\n❌ ERRO DESCONHECIDO\n";
        std::cerr.flush();
        return 1;
    }
}

