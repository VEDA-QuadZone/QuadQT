#!/usr/bin/env python3
"""
SSL/TLS TCP server for testing the Qt login application with OpenSSL
Run this server before testing the Qt application with SSL
"""

import socket
import ssl
import threading
import json
import re
import os

class SSLLoginServer:
    def __init__(self, host='127.0.0.1', port=8080):
        self.host = host
        self.port = port
        self.users = {}  # Simple in-memory user storage
        self.socket = None
        
        # SSL 인증서 파일 경로 (서버용)
        self.ca_cert_file = "../resources/certs/ca.cert.pem"
        self.server_cert_file = "../resources/certs/client.cert.pem"  # 임시로 클라이언트 인증서 사용
        self.server_key_file = "../resources/certs/client.key.pem"   # 임시로 클라이언트 개인키 사용
        
    def start(self):
        # SSL 컨텍스트 생성
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        
        # 로컬 환경에서는 클라이언트 인증서를 요구하지 않음
        if self.host in ['127.0.0.1', 'localhost', '::1']:
            context.verify_mode = ssl.CERT_NONE
            print("로컬 환경 감지: 클라이언트 인증서 검증 비활성화")
        else:
            context.verify_mode = ssl.CERT_REQUIRED
            print("프로덕션 환경: 클라이언트 인증서 검증 활성화")
        
        try:
            # CA 인증서 로드
            if os.path.exists(self.ca_cert_file):
                context.load_verify_locations(self.ca_cert_file)
                print(f"CA 인증서 로드됨: {self.ca_cert_file}")
            else:
                print(f"경고: CA 인증서 파일을 찾을 수 없습니다: {self.ca_cert_file}")
                print("클라이언트 인증서 검증이 비활성화됩니다.")
                context.verify_mode = ssl.CERT_NONE
            
            # 서버 인증서와 개인키 로드
            if os.path.exists(self.server_cert_file) and os.path.exists(self.server_key_file):
                context.load_cert_chain(self.server_cert_file, self.server_key_file)
                print(f"서버 인증서 로드됨: {self.server_cert_file}")
            else:
                print(f"경고: 서버 인증서 파일을 찾을 수 없습니다.")
                print("클라이언트 인증서를 서버 인증서로 사용합니다.")
                # 클라이언트 인증서를 서버용으로 임시 사용
                if os.path.exists("client.cert.pem") and os.path.exists("client.key.pem"):
                    context.load_cert_chain("client.cert.pem", "client.key.pem")
                    print("클라이언트 인증서를 서버용으로 사용")
                else:
                    print("오류: 사용할 수 있는 인증서가 없습니다.")
                    return
            
        except Exception as e:
            print(f"SSL 설정 오류: {e}")
            return
        
        # 소켓 생성 및 바인딩
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.socket.bind((self.host, self.port))
            self.socket.listen(5)
            print(f"SSL Server listening on {self.host}:{self.port}")
            
            while True:
                client_socket, address = self.socket.accept()
                print(f"Connection from {address}")
                
                # SSL 래핑
                try:
                    ssl_client_socket = context.wrap_socket(client_socket, server_side=True)
                    print(f"SSL handshake completed with {address}")
                    
                    # 클라이언트 인증서 정보 출력
                    peer_cert = ssl_client_socket.getpeercert()
                    if peer_cert:
                        print(f"클라이언트 인증서 주체: {peer_cert.get('subject', 'Unknown')}")
                    
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(ssl_client_socket, address)
                    )
                    client_thread.daemon = True
                    client_thread.start()
                    
                except ssl.SSLError as e:
                    print(f"SSL handshake failed with {address}: {e}")
                    client_socket.close()
                except Exception as e:
                    print(f"Error wrapping socket with SSL: {e}")
                    client_socket.close()
                
        except KeyboardInterrupt:
            print("\nServer shutting down...")
        finally:
            if self.socket:
                self.socket.close()
    
    def handle_client(self, client_socket, address):
        try:
            while True:
                data = client_socket.recv(1024).decode('utf-8').strip()
                if not data:
                    break
                
                print(f"Received from {address}: {data}")
                response = self.process_command(data)
                
                if response:
                    client_socket.send(response.encode('utf-8'))
                    print(f"Sent to {address}: {response}")
                    
        except Exception as e:
            print(f"Error handling client {address}: {e}")
        finally:
            client_socket.close()
            print(f"Connection closed: {address}")
    
    def process_command(self, command):
        parts = command.split()
        if len(parts) < 1:
            return self.error_response(400, "Invalid command format")
        
        cmd = parts[0].upper()
        
        if cmd == "REGISTER":
            return self.handle_register(parts[1:])
        elif cmd == "LOGIN":
            return self.handle_login(parts[1:])
        elif cmd == "RESET_PASSWORD":
            return self.handle_reset_password(parts[1:])
        elif cmd == "GET_HISTORY":
            return self.handle_get_history(parts[1:])
        elif cmd == "ADD_HISTORY":
            return self.handle_add_history(parts[1:])
        elif cmd == "GET_FRAME":
            return self.handle_get_frame()
        elif cmd == "CHANGE_FRAME":
            return self.handle_change_frame(parts[1:])
        elif cmd == "GET_LOG":
            return self.handle_get_log()
        else:
            return self.error_response(400, "Unknown command")
    
    def handle_register(self, args):
        if len(args) != 2:
            return self.error_response(400, "Email or password is missing")
        
        email, password = args
        
        if not self.is_valid_email(email):
            return self.error_response(400, "Invalid email format")
        
        if email in self.users:
            return self.error_response(409, "Email already exists")
        
        self.users[email] = password
        print(f"User registered: {email}")
        return self.success_response(200, "User registered successfully")
    
    def handle_login(self, args):
        if len(args) != 2:
            return self.error_response(400, "Email or password is missing")
        
        email, password = args
        
        if email not in self.users:
            return self.error_response(404, "User not found")
        
        if self.users[email] != password:
            return self.error_response(401, "Invalid email or password")
        
        print(f"User logged in: {email}")
        return self.success_response(200, "Login successful")
    
    def handle_reset_password(self, args):
        if len(args) != 2:
            return self.error_response(400, "Email or new password is missing")
        
        email, new_password = args
        
        if email not in self.users:
            return self.error_response(404, "User not found")
        
        self.users[email] = new_password
        print(f"Password reset for user: {email}")
        return self.success_response(200, "Password reset successful")
    
    def handle_get_history(self, args):
        # 더미 히스토리 데이터 반환
        history_data = {
            "histories": [
                {"id": 1, "date": "2024-01-01", "event": "Login", "user": args[0] if args else "test@example.com"},
                {"id": 2, "date": "2024-01-02", "event": "Logout", "user": args[0] if args else "test@example.com"}
            ]
        }
        response = self.success_response(200, "History retrieved successfully")
        response_dict = json.loads(response)
        response_dict.update(history_data)
        return json.dumps(response_dict)
    
    def handle_add_history(self, args):
        print(f"History added: {' '.join(args)}")
        return self.success_response(200, "History added successfully")
    
    def handle_get_frame(self):
        frame_data = {
            "frame": {
                "width": 1920,
                "height": 1080,
                "format": "RGB24"
            }
        }
        response = self.success_response(200, "Frame retrieved successfully")
        response_dict = json.loads(response)
        response_dict.update(frame_data)
        return json.dumps(response_dict)
    
    def handle_change_frame(self, args):
        print(f"Frame changed: {' '.join(args)}")
        return self.success_response(200, "Frame changed successfully")
    
    def handle_get_log(self):
        log_data = {
            "logs": [
                {"timestamp": "2024-01-01 10:00:00", "level": "INFO", "message": "Server started"},
                {"timestamp": "2024-01-01 10:01:00", "level": "INFO", "message": "Client connected"}
            ]
        }
        response = self.success_response(200, "Log retrieved successfully")
        response_dict = json.loads(response)
        response_dict.update(log_data)
        return json.dumps(response_dict)
    
    def is_valid_email(self, email):
        pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
        return re.match(pattern, email) is not None
    
    def success_response(self, code, message):
        response = {
            "status": "success",
            "code": code,
            "message": message
        }
        return json.dumps(response)
    
    def error_response(self, code, message):
        response = {
            "status": "error",
            "code": code,
            "message": message
        }
        return json.dumps(response)

if __name__ == "__main__":
    server = SSLLoginServer()
    server.start()