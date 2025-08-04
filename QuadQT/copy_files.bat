@echo off
echo QuadQT 설정 파일 및 인증서 복사 스크립트

set BUILD_DIR=build\Desktop_Qt_6_9_1_MinGW_64_bit-Debug\debug

echo 빌드 디렉토리: %BUILD_DIR%

if not exist "%BUILD_DIR%" (
    echo 오류: 빌드 디렉토리가 존재하지 않습니다.
    pause
    exit /b 1
)

echo config.ini 복사 중...
copy /Y config.ini "%BUILD_DIR%\"

echo resources 디렉토리 생성 중...
if not exist "%BUILD_DIR%\resources" mkdir "%BUILD_DIR%\resources"
if not exist "%BUILD_DIR%\resources\certs" mkdir "%BUILD_DIR%\resources\certs"

echo 인증서 파일들 복사 중...
if exist "resources\certs\ca.cert.pem" copy /Y "resources\certs\ca.cert.pem" "%BUILD_DIR%\resources\certs\"
if exist "resources\certs\client.cert.pem" copy /Y "resources\certs\client.cert.pem" "%BUILD_DIR%\resources\certs\"
if exist "resources\certs\client.key.pem" copy /Y "resources\certs\client.key.pem" "%BUILD_DIR%\resources\certs\"

echo 루트 디렉토리에도 인증서 복사 중...
if exist "ca.cert.pem" copy /Y "ca.cert.pem" "%BUILD_DIR%\"
if exist "client.cert.pem" copy /Y "client.cert.pem" "%BUILD_DIR%\"
if exist "client.key.pem" copy /Y "client.key.pem" "%BUILD_DIR%\"

echo OpenCV DLL 파일들 복사 중...
if exist "dlls" copy /Y "dlls\*.dll" "%BUILD_DIR%\"

echo 복사 완료!
pause