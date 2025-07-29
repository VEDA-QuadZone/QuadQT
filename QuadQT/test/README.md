# Test 폴더

이 폴더에는 QuadQT 프로젝트의 테스트 관련 파일들이 포함되어 있습니다.

## 파일 목록

### 테스트 서버
- `test_server.py`: 기본 TCP 서버 (SSL 없음)
- `test_ssl_server.py`: SSL/TLS 지원 TCP 서버
- `test_ssl_client.py`: SSL 연결 테스트 클라이언트

### 테스트 메인 파일
- `main_test.cpp`: Qt 애플리케이션 테스트용 메인 파일

### 인증서 생성 스크립트
- `generate_ca_key.sh`: CA 개인키 및 인증서 생성
- `generate_client_cert.sh`: 클라이언트 인증서 생성
- `generate_server_cert.sh`: 서버 인증서 생성
- `generate_certs.ps1`: Windows PowerShell용 인증서 생성 스크립트
- `openssl.conf`: OpenSSL 설정 파일

### 문서
- `SSL_README.md`: SSL 구현에 대한 상세 설명

## 사용 방법

### 1. SSL 서버 테스트
```bash
cd test
python test_ssl_server.py
```

### 2. SSL 클라이언트 테스트
```bash
cd test
python test_ssl_client.py
```

### 3. 기본 TCP 서버 테스트
```bash
cd test
python test_server.py
```

### 4. 인증서 생성 (Windows)
```powershell
cd test
.\generate_certs.ps1
```

## 주의사항

- 인증서 파일들은 상위 디렉토리(`../`)에 위치해야 합니다
- SSL 서버를 실행하기 전에 필요한 인증서 파일들이 있는지 확인하세요
- 테스트 서버들은 포트 8080을 사용합니다