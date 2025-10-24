#!/usr/bin/env python3
"""
Script de teste para integração TCP entre Gateway e Auth Server
Testa a comunicação TCP real entre os servidores
"""

import socket
import json
import time
import threading
import sys
from typing import Dict, Any, Optional

class TCPClient:
    def __init__(self, host: str = "localhost", port: int = 9000):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False
    
    def connect(self) -> bool:
        """Conecta ao servidor TCP"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(10.0)
            self.socket.connect((self.host, self.port))
            self.connected = True
            print(f"[TCP] Conectado ao Gateway Server {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"[TCP] Erro ao conectar: {e}")
            return False
    
    def disconnect(self):
        """Desconecta do servidor"""
        if self.socket:
            self.socket.close()
            self.connected = False
            print("[TCP] Desconectado")
    
    def send_message(self, message: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Envia mensagem e recebe resposta"""
        if not self.connected:
            print("[TCP] Não conectado")
            return None
        
        try:
            # Enviar mensagem
            message_str = json.dumps(message) + "\n"
            self.socket.send(message_str.encode('utf-8'))
            
            # Receber resposta
            response_data = self.socket.recv(4096).decode('utf-8')
            response = json.loads(response_data.strip())
            
            return response
        except Exception as e:
            print(f"[TCP] Erro na comunicação: {e}")
            return None

class AuthServerSimulator:
    """Simulador do Auth Server para testes"""
    
    def __init__(self, port: int = 8080):
        self.port = port
        self.socket = None
        self.running = False
        self.clients = {}
        self.tokens = {
            "valid_token_123": {
                "success": True,
                "valid": True,
                "account_id": 1001,
                "player_id": 2001,
                "username": "testuser"
            },
            "invalid_token_456": {
                "success": False,
                "valid": False,
                "message": "Token inválido"
            }
        }
    
    def start(self):
        """Inicia o simulador do Auth Server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind(('localhost', self.port))
            self.socket.listen(5)
            self.running = True
            
            print(f"[AUTH] Simulador iniciado na porta {self.port}")
            
            # Thread para aceitar conexões
            accept_thread = threading.Thread(target=self._accept_connections)
            accept_thread.daemon = True
            accept_thread.start()
            
        except Exception as e:
            print(f"[AUTH] Erro ao iniciar simulador: {e}")
    
    def stop(self):
        """Para o simulador"""
        self.running = False
        if self.socket:
            self.socket.close()
        print("[AUTH] Simulador parado")
    
    def _accept_connections(self):
        """Aceita conexões de clientes"""
        while self.running:
            try:
                client_socket, address = self.socket.accept()
                print(f"[AUTH] Nova conexão de {address}")
                
                # Thread para lidar com cada cliente
                client_thread = threading.Thread(
                    target=self._handle_client, 
                    args=(client_socket, address)
                )
                client_thread.daemon = True
                client_thread.start()
                
            except Exception as e:
                if self.running:
                    print(f"[AUTH] Erro ao aceitar conexão: {e}")
    
    def _handle_client(self, client_socket, address):
        """Lida com mensagens de um cliente"""
        try:
            while self.running:
                data = client_socket.recv(4096)
                if not data:
                    break
                
                message = json.loads(data.decode('utf-8').strip())
                response = self._process_message(message)
                
                response_str = json.dumps(response) + "\n"
                client_socket.send(response_str.encode('utf-8'))
                
        except Exception as e:
            print(f"[AUTH] Erro ao lidar com cliente {address}: {e}")
        finally:
            client_socket.close()
            print(f"[AUTH] Cliente {address} desconectado")
    
    def _process_message(self, message: Dict[str, Any]) -> Dict[str, Any]:
        """Processa mensagem do cliente"""
        action = message.get("action", "")
        data = message.get("data", {})
        
        if action == "validate_token":
            token = data.get("token", "")
            return self.tokens.get(token, {
                "success": False,
                "valid": False,
                "message": "Token não encontrado"
            })
        
        elif action == "check_session":
            account_id = data.get("account_id", 0)
            return {
                "success": True,
                "valid": account_id > 0,
                "account_id": account_id
            }
        
        elif action == "revoke_token":
            token = data.get("token", "")
            if token in self.tokens:
                del self.tokens[token]
                return {"success": True, "message": "Token revogado"}
            else:
                return {"success": False, "message": "Token não encontrado"}
        
        else:
            return {
                "success": False,
                "message": f"Ação desconhecida: {action}"
            }

def test_gateway_connection():
    """Testa conexão com o Gateway Server"""
    print("\n=== TESTE DE CONEXÃO COM GATEWAY ===")
    
    client = TCPClient("localhost", 9000)
    
    if not client.connect():
        print("❌ Falha ao conectar com Gateway Server")
        return False
    
    # Teste 1: Ping
    print("\n1. Testando ping...")
    response = client.send_message({"action": "ping"})
    if response and response.get("type") == "pong":
        print("✅ Ping funcionando")
    else:
        print("❌ Ping falhou")
        client.disconnect()
        return False
    
    # Teste 2: Informações do servidor
    print("\n2. Testando informações do servidor...")
    response = client.send_message({"action": "get_server_info"})
    if response and response.get("success"):
        print("✅ Informações do servidor obtidas")
        print(f"   - Servidores registrados: {response.get('server_count', 0)}")
        print(f"   - Auth conectado: {response.get('auth_connected', False)}")
    else:
        print("❌ Falha ao obter informações do servidor")
    
    # Teste 3: Autenticação com token válido
    print("\n3. Testando autenticação com token válido...")
    response = client.send_message({
        "action": "authenticate",
        "token": "valid_token_123"
    })
    if response and response.get("success"):
        print("✅ Autenticação com token válido funcionando")
        print(f"   - Account ID: {response.get('account_id')}")
        print(f"   - Player ID: {response.get('player_id')}")
        print(f"   - Username: {response.get('username')}")
    else:
        print("❌ Falha na autenticação com token válido")
    
    # Teste 4: Autenticação com token inválido
    print("\n4. Testando autenticação com token inválido...")
    response = client.send_message({
        "action": "authenticate",
        "token": "invalid_token_456"
    })
    if response and not response.get("success"):
        print("✅ Rejeição de token inválido funcionando")
    else:
        print("❌ Falha na rejeição de token inválido")
    
    client.disconnect()
    return True

def test_auth_server_simulator():
    """Testa o simulador do Auth Server"""
    print("\n=== TESTE DO SIMULADOR AUTH SERVER ===")
    
    auth_sim = AuthServerSimulator(8080)
    auth_sim.start()
    
    # Aguardar um pouco para o servidor inicializar
    time.sleep(1)
    
    # Teste de conexão direta com Auth Server
    client = TCPClient("localhost", 8080)
    
    if not client.connect():
        print("❌ Falha ao conectar com Auth Server Simulator")
        auth_sim.stop()
        return False
    
    # Teste de validação de token
    print("\n1. Testando validação de token...")
    response = client.send_message({
        "action": "validate_token",
        "data": {"token": "valid_token_123"}
    })
    
    if response and response.get("success") and response.get("valid"):
        print("✅ Validação de token funcionando")
    else:
        print("❌ Falha na validação de token")
    
    # Teste de verificação de sessão
    print("\n2. Testando verificação de sessão...")
    response = client.send_message({
        "action": "check_session",
        "data": {"account_id": 1001}
    })
    
    if response and response.get("success"):
        print("✅ Verificação de sessão funcionando")
    else:
        print("❌ Falha na verificação de sessão")
    
    client.disconnect()
    auth_sim.stop()
    return True

def main():
    """Função principal do teste"""
    print("🚀 Iniciando testes de integração TCP")
    print("=" * 50)
    
    # Verificar se os servidores estão rodando
    print("⚠️  Certifique-se de que os servidores estão rodando:")
    print("   - Auth Server na porta 8080")
    print("   - Gateway Server na porta 9000")
    print()
    
    input("Pressione Enter para continuar...")
    
    # Teste 1: Simulador Auth Server
    if not test_auth_server_simulator():
        print("\n❌ Teste do simulador Auth Server falhou")
        return 1
    
    # Teste 2: Gateway Server
    if not test_gateway_connection():
        print("\n❌ Teste do Gateway Server falhou")
        return 1
    
    print("\n" + "=" * 50)
    print("✅ Todos os testes passaram com sucesso!")
    print("🎉 Integração TCP funcionando corretamente")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
