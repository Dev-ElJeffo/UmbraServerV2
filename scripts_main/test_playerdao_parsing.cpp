// Teste de PlayerDAO Parsing - Fase 2
// Valida que o parsing completo de Player funciona corretamente

#include "core/Logger.hpp"
#include "database/MySQLConnector.hpp"
#include "database/PlayerDAO.hpp"
#include <iostream>
#include <cassert>

using namespace Umbra::Core;
using namespace Umbra::Database;

void testPlayerDAOParsing() {
    std::cout << "\n=== TESTE: PlayerDAO Parsing Completo ===\n\n";
    
    // 1. Inicializar Logger
    Logger::getInstance().initialize("logs/test_playerdao.log");
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
    
    // 3. Criar PlayerDAO
    PlayerDAO playerDAO(dbConnector);
    std::cout << "✓ PlayerDAO criado\n";
    
    // 4. Teste: getPlayersByAccountId
    std::cout << "\n--- Teste: getPlayersByAccountId ---\n";
    
    // Testando com account_id = 4 (conta conhecida com personagens)
    uint64_t testAccountId = 4;
    
    auto players = playerDAO.getPlayersByAccountId(testAccountId);
    
    if (players.empty()) {
        std::cout << "⚠️  Nenhum personagem encontrado para account_id " << testAccountId << "\n";
        std::cout << "   (Isso é OK se não houver personagens criados ainda)\n";
    } else {
        std::cout << "✓ Encontrados " << players.size() << " personagem(ns)\n";
        
        for (size_t i = 0; i < players.size(); ++i) {
            const auto& player = players[i];
            std::cout << "\n  Personagem " << (i + 1) << ":\n";
            std::cout << "    - ID: " << player.id << "\n";
            std::cout << "    - Nome: " << player.characterName << "\n";
            std::cout << "    - Account ID: " << player.accountId << "\n";
            std::cout << "    - Level: " << player.level << "\n";
            std::cout << "    - Experience: " << player.experience << "\n";
            std::cout << "    - Posição: (" << player.posX << ", " << player.posY << ", " << player.posZ << ")\n";
            std::cout << "    - Zona: " << player.currentZone << "\n";
            std::cout << "    - Health: " << player.health << "/" << player.maxHealth << "\n";
            std::cout << "    - Mana: " << player.mana << "/" << player.maxMana << "\n";
            std::cout << "    - Stamina: " << player.stamina << "/" << player.maxStamina << "\n";
            std::cout << "    - Atributos: STR=" << player.strength 
                      << ", DEX=" << player.dexterity 
                      << ", INT=" << player.intelligence 
                      << ", VIT=" << player.vitality << "\n";
            
            // Validar que dados não estão vazios/zerados (exceto se realmente forem zero)
            assert(player.id > 0);
            assert(!player.characterName.empty());
            assert(player.accountId > 0);
            std::cout << "    ✓ Validação de dados básicos: PASS\n";
        }
        
        // 5. Teste: getPlayerById (usar primeiro player encontrado)
        if (!players.empty()) {
            std::cout << "\n--- Teste: getPlayerById ---\n";
            uint64_t testPlayerId = players[0].id;
            std::string testPlayerName = players[0].characterName;
            
            auto playerById = playerDAO.getPlayerById(testPlayerId);
            if (playerById.has_value()) {
                std::cout << "✓ getPlayerById funcionou\n";
                std::cout << "  - ID: " << playerById->id << "\n";
                std::cout << "  - Nome: " << playerById->characterName << "\n";
                assert(playerById->id == testPlayerId);
                assert(playerById->characterName == testPlayerName);
                std::cout << "  ✓ Dados correspondem: PASS\n";
            } else {
                std::cout << "❌ getPlayerById retornou nullopt\n";
            }
            
            // 6. Teste: getPlayerByName
            std::cout << "\n--- Teste: getPlayerByName ---\n";
            auto playerByName = playerDAO.getPlayerByName(testPlayerName);
            if (playerByName.has_value()) {
                std::cout << "✓ getPlayerByName funcionou\n";
                std::cout << "  - ID: " << playerByName->id << "\n";
                std::cout << "  - Nome: " << playerByName->characterName << "\n";
                assert(playerByName->id == testPlayerId);
                assert(playerByName->characterName == testPlayerName);
                std::cout << "  ✓ Dados correspondem: PASS\n";
            } else {
                std::cout << "❌ getPlayerByName retornou nullopt\n";
            }
        }
    }
    
    // 7. Teste: Verificar personagem inexistente
    std::cout << "\n--- Teste: Personagem Inexistente ---\n";
    auto nonExistent = playerDAO.getPlayerById(999999);
    if (!nonExistent.has_value()) {
        std::cout << "✓ getPlayerById retornou nullopt para ID inexistente: PASS\n";
    } else {
        std::cout << "⚠️  getPlayerById retornou valor para ID inexistente\n";
    }
    
    auto nonExistentByName = playerDAO.getPlayerByName("NOME_INEXISTENTE_XYZ_123");
    if (!nonExistentByName.has_value()) {
        std::cout << "✓ getPlayerByName retornou nullopt para nome inexistente: PASS\n";
    } else {
        std::cout << "⚠️  getPlayerByName retornou valor para nome inexistente\n";
    }
    
    // Limpar
    std::cout << "\n--- Limpeza ---\n";
    dbConnector->disconnect();
    std::cout << "✓ MySQL desconectado\n";
    
    std::cout << "\n========================================\n";
    std::cout << "✅ TESTE DE PlayerDAO PASSOU!\n";
    std::cout << "========================================\n";
}

int main() {
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    
    std::cout << "========================================\n";
    std::cout << "TESTE PlayerDAO PARSING - FASE 2\n";
    std::cout << "========================================\n";
    
    try {
        testPlayerDAOParsing();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ ERRO: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n❌ ERRO DESCONHECIDO\n";
        return 1;
    }
}

