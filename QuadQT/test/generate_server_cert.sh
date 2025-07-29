#!/bin/bash

# 서버 인증서 생성 스크립트
# 이미 존재하는 CA를 사용하여 서버 인증서를 생성합니다

echo "서버 인증서 생성 중..."

# 서버 개인키 생성
openssl genrsa -out server.key.pem 2048

# 서버 인증서 서명 요청(CSR) 생성
openssl req -new -key server.key.pem -out server.csr -subj "/C=KR/ST=Seoul/L=Seoul/O=TestOrg/OU=TestUnit/CN=localhost"

# CA를 사용하여 서버 인증서 서명
openssl x509 -req -in server.csr -CA ca.cert.pem -CAkey ca.key.pem -CAcreateserial -out server.cert.pem -days 365 -extensions v3_req

# 임시 파일 정리
rm server.csr

echo "서버 인증서 생성 완료:"
echo "  - server.cert.pem (서버 인증서)"
echo "  - server.key.pem (서버 개인키)"

# 인증서 정보 확인
echo ""
echo "서버 인증서 정보:"
openssl x509 -in server.cert.pem -text -noout | grep -A 1 "Subject:"