# PowerShell 스크립트로 인증서 생성
# Git Bash의 OpenSSL을 사용합니다

Write-Host "SSL 인증서 생성 시작..."

# Git Bash의 OpenSSL 경로 찾기
$gitPath = (Get-Command git).Source
$gitDir = Split-Path $gitPath
$opensslPath = Join-Path $gitDir "usr\bin\openssl.exe"

if (-not (Test-Path $opensslPath)) {
    Write-Host "OpenSSL을 찾을 수 없습니다. Git for Windows가 설치되어 있는지 확인하세요."
    exit 1
}

Write-Host "OpenSSL 경로: $opensslPath"

# 1. CA 개인키 생성
Write-Host "1. CA 개인키 생성 중..."
& $opensslPath genrsa -out ca.key.pem 2048

# 2. CA 인증서 생성 (기존 것을 덮어씀)
Write-Host "2. CA 인증서 생성 중..."
& $opensslPath req -new -x509 -key ca.key.pem -out ca.cert.pem -days 365 -config openssl.conf -extensions v3_ca

# 3. 서버 개인키 생성
Write-Host "3. 서버 개인키 생성 중..."
& $opensslPath genrsa -out server.key.pem 2048

# 4. 서버 CSR 생성
Write-Host "4. 서버 CSR 생성 중..."
& $opensslPath req -new -key server.key.pem -out server.csr -config openssl.conf

# 5. 서버 인증서 생성
Write-Host "5. 서버 인증서 생성 중..."
& $opensslPath x509 -req -in server.csr -CA ca.cert.pem -CAkey ca.key.pem -CAcreateserial -out server.cert.pem -days 365 -extensions v3_server -extfile openssl.conf

# 6. 클라이언트 개인키 생성 (기존 것을 덮어씀)
Write-Host "6. 클라이언트 개인키 생성 중..."
& $opensslPath genrsa -out client.key.pem 2048

# 7. 클라이언트 CSR 생성
Write-Host "7. 클라이언트 CSR 생성 중..."
& $opensslPath req -new -key client.key.pem -out client.csr -config openssl.conf -subj "/C=KR/ST=Seoul/L=Seoul/O=TestOrg/OU=TestUnit/CN=client"

# 8. 클라이언트 인증서 생성
Write-Host "8. 클라이언트 인증서 생성 중..."
& $opensslPath x509 -req -in client.csr -CA ca.cert.pem -CAkey ca.key.pem -CAcreateserial -out client.cert.pem -days 365 -extensions v3_client -extfile openssl.conf

# 임시 파일 정리
Remove-Item server.csr -ErrorAction SilentlyContinue
Remove-Item client.csr -ErrorAction SilentlyContinue

Write-Host "인증서 생성 완료!"
Write-Host "생성된 파일들:"
Write-Host "  - ca.cert.pem (CA 인증서)"
Write-Host "  - ca.key.pem (CA 개인키)"
Write-Host "  - server.cert.pem (서버 인증서)"
Write-Host "  - server.key.pem (서버 개인키)"
Write-Host "  - client.cert.pem (클라이언트 인증서)"
Write-Host "  - client.key.pem (클라이언트 개인키)"

# 인증서 정보 확인
Write-Host "`n인증서 정보 확인:"
Write-Host "CA 인증서:"
& $opensslPath x509 -in ca.cert.pem -text -noout | Select-String "Subject:", "Key Usage"

Write-Host "`n서버 인증서:"
& $opensslPath x509 -in server.cert.pem -text -noout | Select-String "Subject:", "Key Usage"

Write-Host "`n클라이언트 인증서:"
& $opensslPath x509 -in client.cert.pem -text -noout | Select-String "Subject:", "Key Usage"