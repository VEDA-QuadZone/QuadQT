# OpenSSL TCP 통신 구현

이 프로젝트는 Qt 애플리케이션에서 OpenSSL을 사용한 안전한 TCP 통신을 구현합니다.

## 구현된 기능

### 1. SSL/TLS 지원 NetworkManager
- QSslSocket을 사용한 암호화된 통신
- 클라이언트 인증서 기반 상호 인증
- CA 인증서를 통한 서버 검증

### 2. 인증서 설정
프로젝트에 포함된 인증서 파일들:
- `ca.cert.pem`: CA 인증서 (서버 검증용)
- `client.cert.pem`: 클라이언트 인증서 (클라이언트 인증용)
- `client.key.pem`: 클라이언트 개인키

### 3. 설정 파일 (config.ini)
```ini
[Server]
ip=127.0.0.1
port=8080
timeout=5000

[SSL]
enabled=true
ca_cert=ca.cert.pem
client_cert=client.cert.pem
client_key=client.key.pem
```

## 사용 방법

### 1. SSL 서버 실행
```bash
python test_ssl_server.py
```

### 2. Qt 애플리케이션 실행
애플리케이션이 자동으로 SSL 모드로 서버에 연결됩니다.

### 3. SSL 비활성화 (선택사항)
config.ini에서 `enabled=false`로 설정하면 일반 TCP 통신을 사용합니다.

## 주요 클래스 및 메서드

### NetworkManager 클래스
- `connectToServer()`: 서버 연결 (SSL/비SSL 자동 선택)
- `connectToServerSSL()`: SSL 전용 연결
- `setupSSLConfiguration()`: SSL 설정 초기화
- `setSSLEnabled(bool)`: SSL 활성화/비활성화

### SSL 관련 시그널
- `onEncrypted()`: SSL 핸드셰이크 완료
- `onSslErrors()`: SSL 오류 처리

## 보안 고려사항

### 개발 환경
- 자체 서명된 인증서 오류를 무시하도록 설정됨
- 테스트 목적으로 인증서 검증이 완화됨

### 프로덕션 환경
프로덕션 배포 시 다음 사항을 수정해야 합니다:
1. 신뢰할 수 있는 CA에서 발급받은 인증서 사용
2. 자체 서명된 인증서 오류 무시 코드 제거
3. 더 엄격한 인증서 검증 로직 구현

## 인증서 생성 스크립트

프로젝트에 포함된 스크립트들:
- `generate_ca_key.sh`: CA 개인키 및 인증서 생성
- `generate_client_cert.sh`: 클라이언트 인증서 생성
- `generate_server_cert.sh`: 서버 인증서 생성

## 트러블슈팅

### 일반적인 문제들

1. **SSL 핸드셰이크 실패**
   - 인증서 파일 경로 확인
   - 인증서 유효성 검사
   - 시간 동기화 확인

2. **인증서 오류**
   - CA 인증서와 클라이언트/서버 인증서 일치 확인
   - 인증서 만료일 확인

3. **연결 실패**
   - 서버가 실행 중인지 확인
   - 방화벽 설정 확인
   - 포트 사용 가능 여부 확인

## 로그 메시지

SSL 연결 시 다음과 같은 로그가 출력됩니다:
```
SSL 설정 완료
CA 인증서 주체: [CA 이름]
클라이언트 인증서 주체: [클라이언트 이름]
서버에 SSL 연결 중: 127.0.0.1:8080
SSL 연결이 암호화되었습니다
사용된 암호화 방식: [암호화 방식]
서버 인증서 주체: [서버 이름]
```

## 지원되는 프로토콜

- TLS 1.2 이상
- RSA 2048비트 키
- PEM 형식 인증서