#!/usr/bin/env python3
"""
Simple TCP server for testing the Qt login application
Run this server before testing the Qt application
"""

import socket
import threading
import json
import re

class LoginServer:
    def __init__(self, host='127.0.0.1', port=8080):
        self.host = host
        self.port = port
        self.users = {}  # Simple in-memory user storage
        self.socket = None
        
    def start(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.socket.bind((self.host, self.port))
            self.socket.listen(5)
            print(f"Server listening on {self.host}:{self.port}")
            
            while True:
                client_socket, address = self.socket.accept()
                print(f"Connection from {address}")
                
                client_thread = threading.Thread(
                    target=self.handle_client,
                    args=(client_socket, address)
                )
                client_thread.daemon = True
                client_thread.start()
                
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
    server = LoginServer()
    server.start()