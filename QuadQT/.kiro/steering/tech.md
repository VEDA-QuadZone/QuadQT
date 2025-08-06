# 기술 스택

## 빌드 시스템
- **Qt qmake** (.pro 파일) - 주요 빌드 시스템
- **C++17** 표준
- **MinGW** 컴파일러 (Windows)

## 핵심 기술
- **Qt 5/6** - 메인 애플리케이션 프레임워크
  - QtCore, QtGui, QtWidgets - UI 프레임워크
  - QtNetwork - TCP/SSL 네트워킹
  - QtMqtt - MQTT 메시징
  - QtMultimedia, QtMultimediaWidgets - 비디오 재생
- **OpenCV 4.5.5** - 컴퓨터 비전 및 이미지 처리
- **OpenSSL** - SSL/TLS 암호화 (Qt 내장 지원 사용)

## 주요 라이브러리 및 의존성
- OpenCV 모듈: core, videoio, imgcodecs, imgproc, highgui
- 보안 통신을 위한 Qt SSL 지원
- 기업 브랜딩을 위한 한화 커스텀 폰트

## 설정
- **config.ini** - 애플리케이션 설정 (RTSP, MQTT, TCP, SSL 설정)
- **리소스 시스템** (.qrc) - 임베디드 폰트, 이미지, UI 파일
- **UI 파일** (.ui) - 로그인 및 메인 윈도우용 Qt Designer 폼

## 주요 명령어

### 빌드
```bash
# .pro 파일에서 Makefile 생성
qmake QuadQT.pro

# 애플리케이션 빌드
make
# 또는 Windows MinGW에서
mingw32-make
```

### 테스트
```bash
# MQTT 테스트 스크립트 실행
python test/mqtt_test.py --test

# 테스트용 SSL 서버 실행
python test/test_ssl_server.py

# SSL 클라이언트 테스트 실행
python test/test_ssl_client.py
```

### 인증서 생성 (Windows)
```powershell
# SSL 인증서 생성
cd test
.\generate_certs.ps1
```

## 개발 환경
- **Windows** 주요 타겟 플랫폼
- **MinGW** 컴파일 툴체인
- **Qt Creator** 권장 IDE
- **Python 3.x** 테스트 스크립트용

## 네트워크 프로토콜
- **RTSP/RTSPS** - 비디오 스트리밍
- **MQTT** - 알림 메시징
- **TCP with SSL/TLS** - 보안 데이터 통신
- **HTTP/HTTPS** - 사용자 인증 API 호출