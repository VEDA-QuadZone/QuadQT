#!/bin/bash

# 클라이언트 인증서 생성 스크립트
# CA를 사용하여 클라이언트 인증서를 생성합니다

echo "클라이언트 인증서 생성 중..."

# 클라이언트 개인키 생성
openssl genrsa -out client.key.pem 2048

# 클라이언트 인증서 서명 요청(CSR) 생성
openssl req -new -key client.key.pem -out client.csr -subj "/C=KR/ST=Seoul/L=Seoul/O=TestOrg/OU=TestUnit/CN=client"

# CA를 사용하여 클라이언트 인증서 서명
openssl x509 -req -in client.csr -CA ca.cert.pem -CAkey ca.key.pem -CAcreateserial -out client.cert.pem -days 365

# 임시 파일 정리
rm client.csr

echo "클라이언트 인증서 생성 완료:"
echo "  - client.cert.pem (클라이언트 인증서)"
echo "  - client.key.pem (클라이언트 개인키)"

# 인증서 정보 확인
echo ""
echo "클라이언트 인증서 정보:"
openssl x509 -in client.cert.pem -text -noout | grep -A 1 "Subject:"