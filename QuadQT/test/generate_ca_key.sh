#!/bin/bash

# CA 개인키 생성 스크립트
# 기존 CA 인증서에 대응하는 개인키를 생성합니다

echo "CA 개인키 생성 중..."

# CA 개인키 생성 (2048비트 RSA)
openssl genrsa -out ca.key.pem 2048

echo "CA 개인키 생성 완료: ca.key.pem"

# 기존 CA 인증서가 있는지 확인
if [ -f "ca.cert.pem" ]; then
    echo "기존 CA 인증서 발견: ca.cert.pem"
    echo "새로운 CA 인증서를 생성하려면 다음 명령을 실행하세요:"
    echo "openssl req -new -x509 -key ca.key.pem -out ca.cert.pem -days 365 -subj \"/C=KR/ST=Seoul/L=Seoul/O=TestCA/OU=TestUnit/CN=TestCA\""
else
    echo "CA 인증서 생성 중..."
    openssl req -new -x509 -key ca.key.pem -out ca.cert.pem -days 365 -subj "/C=KR/ST=Seoul/L=Seoul/O=TestCA/OU=TestUnit/CN=TestCA"
    echo "CA 인증서 생성 완료: ca.cert.pem"
fi

echo ""
echo "생성된 파일들:"
echo "  - ca.key.pem (CA 개인키)"
echo "  - ca.cert.pem (CA 인증서)"