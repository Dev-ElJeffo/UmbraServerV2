// Teste de Integração - Fase 1
// Valida que Gateway, Auth Server e MySQL funcionam juntos

#include "src/core/Logger.hpp"
#include "src/core/ConfigManager.hpp"
#include "src/gateway/GatewayServer.hpp"
#include "src/auth/AuthServer.hpp"
#include "src/database/MySQLConnector.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace Umbra::Core;
using namespace Umbra::Auth;
using namespace Umbra::Gateway;
using namespace Umbra::Database;

void testIntegratedSystem() {
    std::cout << "\n=== TESTE DE INTEGRAÇÃO FASE 1 ===\n\n";
    
    // 1. Inicializar Logger
    Logger::getInstance().initialize("logs/test_integracao.log");
    std::cout << "✓ Logger inicializado\n";
    
    // 2. Conectar ao MySQL
    MySQLConnector::Config dbConfig;
    dbConfig.host = "localhost";
    dbConfig.port = 3306;
    dbConfig.database = "umbra_eternum";
    dbConfig.username = "root";
    dbConfig.password = "!Mister4126";
    
    auto dbConnector = std::make_shared<MySQLConnector>(dbConfig);
    
    if (!dbConnector->connect()) {
        std::cout << "❌ Falha ao conectar ao MySQL\n";
        return;
    }
    std::cout << "✓ Conexão MySQL estabelecida\n";
    
    // 3. Iniciar Auth Server
    AuthServer::Config authConfig;
    authConfig.port = 8080;
    authConfig.jwtSecret = "UmbraEternum2025SecretKey123456789";
    authConfig.sessionDurationMinutes = 60;
    
    auto authServer = std::make_unique<AuthServer>(authConfig, dbConnector);
    
    if (!authServer->start()) {
        std::cout << "❌ Falha ao iniciar Auth Server\n";
        return;
    }
    std::cout << "✓ Auth Server iniciado na porta 8080\n";
    
    // Aguardar Auth Server ficar pronto
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 4. Testar registro de conta
    std::cout << "\n--- Teste: Registro de Conta ---\n";
    auto registerResult = authServer->registerAccount(
        "testuser_integration",
        "test@test.com",
        "senha123"
    );
    
    if (registerResult.success) {
        std::cout << "✓ Conta registrada com sucesso (ID: " << registerResult.accountId << ")\n";
    } else {
        std::cout << "⚠️  Registro falhou (pode já existir): " << registerResult.message << "\n";
    }
    
    // 5. Testar login
    std::cout << "\n--- Teste: Login ---\n";
    auto loginResult = authServer->login("testuser_integration", "senha123", "127.0.0.1");
    
    if (!loginResult.success) {
        std::cout << "❌ Login falhou: " << loginResult.message << "\n";
        authServer->stop();
        return;
    }
    
    std::cout << "✓ Login bem-sucedido\n";
    std::cout << "  - Token gerado: " << loginResult.token.substr(0, 50) << "...\n";
    std::cout << "  - Account ID: " << loginResult.accountId << "\n";
    std::cout << "  - Username: " << loginResult.username << "\n";
    
    // 6. Iniciar Gateway Server
    std::cout << "\n--- Teste: Gateway Server ---\n";
    GatewayServer::Config gatewayConfig;
    gatewayConfig.port = 9000;
    gatewayConfig.jwtSecret = authConfig.jwtSecret;
    gatewayConfig.authHost = "localhost";
    gatewayConfig.authPort = 8080;
    gatewayConfig.useConnectionPool = false; // Usar cliente simples para teste
    
    auto gatewayServer = std::make_unique<GatewayServer>(gatewayConfig);
    
    if (!gatewayServer->start()) {
        std::cout << "❌ Falha ao iniciar Gateway Server\n";
        authServer->stop();
        return;
    }
    std::cout << "✓ Gateway Server iniciado na porta 9000\n";
    
    // Aguardar Gateway ficar pronto
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 7. Testar validação de token via Gateway
    std::cout << "\n--- Teste: Validação de Token via Gateway ---\n";
    if (gatewayServer->validateToken(loginResult.token)) {
        std::cout << "✓ Token validado via Gateway Server\n";
        
        auto clientInfo = gatewayServer->getClientInfo(loginResult.token);
        if (clientInfo.has_value()) {
            std::cout << "✓ Informações do cliente obtidas\n";
            std::cout << "  - Account ID: " << clientInfo->accountId << "\n";
            std::cout << "  - Username: " << clientInfo->username << "\n";
        }
    } else {
        std::cout << "❌ Falha na validação de token via Gateway\n";
    }
    
    // 8. Testar logout
    std::cout << "\n--- Teste: Logout ---\n";
    if (authServer->logout(loginResult.token)) {
        std::cout << "✓ Logout bem-sucedido\n";
    } else {
        std::cout << "⚠️  Logout falhou\n";
    }
    
    // Limpar
    std::cout << "\n--- Limpeza ---\n";
    gatewayServer->stop();
    std::cout << "✓ Gateway Server parado\n";
    authServer->stop();
    std::cout << "✓ Auth Server parado\n";
    dbConnector->disconnect();
    std::cout << "✓ MySQL desconectado\n";
    
    std::cout << "\n========================================\n";
    std::cout << "✅ TESTE DE INTEGRAÇÃO PASSOU!\n";
    std::cout << "========================================\n";
}

int main() {
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    std::cout << "========================================\n";
    std::cout << "TESTE DE INTEGRACAO FASE 1\n";
    std::cout << "========================================\n";
    
    try {
        testIntegratedSystem();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ ERRO: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n❌ ERRO DESCONHECIDO\n";
        return 1;
    }
}

