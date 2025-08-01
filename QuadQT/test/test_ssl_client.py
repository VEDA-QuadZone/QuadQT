#!/usr/bin/env python3
"""
SSL 클라이언트 테스트 스크립트
Qt 애플리케이션 없이 SSL 연결을 테스트합니다
"""

import socket
import ssl
import json

def test_ssl_connection():
    # SSL 컨텍스트 생성
    context = ssl.create_default_context()
    
    # 자체 서명된 인증서 허용 (테스트용)
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE
    
    try:
        # 클라이언트 인증서 로드 (상호 인증용)
        context.load_cert_chain("../resources/certs/client.cert.pem", "../resources/certs/client.key.pem")
        print("클라이언트 인증서 로드 완료")
    except Exception as e:
        print(f"클라이언트 인증서 로드 실패: {e}")
        return
    
    try:
        # CA 인증서 로드
        context.load_verify_locations("../resources/certs/ca.cert.pem")
        print("CA 인증서 로드 완료")
    except Exception as e:
        print(f"CA 인증서 로드 실패: {e}")
    
    # 소켓 생성 및 SSL 래핑
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ssl_sock = context.wrap_socket(sock, server_hostname='localhost')
    
    try:
        # 서버에 연결
        print("서버에 SSL 연결 중...")
        ssl_sock.connect(('192.168.0.10', 8080))
        print("SSL 연결 성공!")
        
        # 연결 정보 출력
        print(f"사용된 암호화 방식: {ssl_sock.cipher()}")
        print(f"SSL 버전: {ssl_sock.version()}")
        
        # 서버 인증서 정보
        server_cert = ssl_sock.getpeercert()
        if server_cert:
            print(f"서버 인증서 주체: {server_cert.get('subject', 'Unknown')}")
        
        # 테스트 명령 전송
        test_commands = [
            "REGISTER test@example.com password123",
            "LOGIN test@example.com password123",
            "GET_HISTORY test@example.com 10 0"
        ]
        
        for command in test_commands:
            print(f"\n명령 전송: {command}")
            ssl_sock.send(command.encode('utf-8') + b'\n')
            
            # 응답 수신
            response = ssl_sock.recv(1024).decode('utf-8')
            print(f"서버 응답: {response}")
            
            # JSON 파싱 시도
            try:
                json_response = json.loads(response)
                print(f"파싱된 응답: {json.dumps(json_response, indent=2, ensure_ascii=False)}")
            except json.JSONDecodeError:
                print("JSON 파싱 실패")
        
    except Exception as e:
        print(f"연결 오류: {e}")
    finally:
        ssl_sock.close()
        print("연결 종료")

if __name__ == "__main__":
    test_ssl_connection()