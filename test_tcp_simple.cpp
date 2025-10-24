#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

int main() {
    std::cout << "===========================================\n";
    std::cout << "    Teste de Integração TCP - UmbraEternum\n";
    std::cout << "===========================================\n\n";

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar Winsock" << std::endl;
        return 1;
    }
#endif

    // Teste 1: Conexão com Gateway Server
    std::cout << "[TESTE 1] Conectando ao Gateway Server (porta 9000)...\n";
    
    int gatewaySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (gatewaySocket == -1) {
        std::cerr << "❌ Falha ao criar socket para Gateway" << std::endl;
        return 1;
    }

    sockaddr_in gatewayAddr{};
    gatewayAddr.sin_family = AF_INET;
    gatewayAddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &gatewayAddr.sin_addr);

    if (connect(gatewaySocket, (sockaddr*)&gatewayAddr, sizeof(gatewayAddr)) == -1) {
        std::cout << "⚠️  Gateway Server não está rodando na porta 9000\n";
        std::cout << "   Para testar, execute: build\\bin\\Release\\umbra_server.exe\n\n";
    } else {
        std::cout << "✅ Conectado ao Gateway Server com sucesso!\n";
        
        // Enviar mensagem de ping
        std::string pingMessage = R"({"action": "ping"})";
        send(gatewaySocket, pingMessage.c_str(), pingMessage.length(), 0);
        
        // Receber resposta
        char buffer[1024];
        int bytesReceived = recv(gatewaySocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "✅ Resposta do Gateway: " << buffer << "\n";
        }
        
#ifdef _WIN32
        closesocket(gatewaySocket);
#else
        close(gatewaySocket);
#endif
    }

    // Teste 2: Conexão com Auth Server
    std::cout << "\n[TESTE 2] Conectando ao Auth Server (porta 8080)...\n";
    
    int authSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (authSocket == -1) {
        std::cerr << "❌ Falha ao criar socket para Auth Server" << std::endl;
        return 1;
    }

    sockaddr_in authAddr{};
    authAddr.sin_family = AF_INET;
    authAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &authAddr.sin_addr);

    if (connect(authSocket, (sockaddr*)&authAddr, sizeof(authAddr)) == -1) {
        std::cout << "⚠️  Auth Server não está rodando na porta 8080\n";
        std::cout << "   Para testar, execute: build\\bin\\Release\\auth_server.exe\n\n";
    } else {
        std::cout << "✅ Conectado ao Auth Server com sucesso!\n";
        
        // Enviar mensagem de validação de token
        std::string tokenMessage = R"({"action": "validate_token", "data": {"token": "test_token"}})";
        send(authSocket, tokenMessage.c_str(), tokenMessage.length(), 0);
        
        // Receber resposta
        char buffer[1024];
        int bytesReceived = recv(authSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "✅ Resposta do Auth Server: " << buffer << "\n";
        }
        
#ifdef _WIN32
        closesocket(authSocket);
#else
        close(authSocket);
#endif
    }

    std::cout << "\n===========================================\n";
    std::cout << "✅ Teste de integração TCP concluído!\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "Para executar os servidores:\n";
    std::cout << "1. build\\bin\\Release\\umbra_server.exe (servidor completo)\n";
    std::cout << "2. build\\bin\\Release\\auth_server.exe (apenas auth)\n";
    std::cout << "3. build\\bin\\Release\\gateway_server.exe (apenas gateway)\n\n";

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
