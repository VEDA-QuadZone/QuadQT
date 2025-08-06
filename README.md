[README.md](https://github.com/user-attachments/files/21572540/README.md)
# <img width="24" height="24" alt="image" src="https://github.com/user-attachments/assets/1062fe1a-9ade-41e9-b087-f2192e0e7652" /> QuadQT

> 실시간 영상 감시 및 모니터링을 위한 Qt 기반 데스크톱 애플리케이션

[![Qt](https://img.shields.io/badge/Qt-6.9.1-green.svg)](https://www.qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.5.5-blue.svg)](https://opencv.org/)
[![C++](https://img.shields.io/badge/C++-17-red.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-Proprietary-yellow.svg)]()

QuadQT는 교통 및 주차 단속을 위한 실시간 영상 감시 시스템입니다. RTSP 스트리밍, MQTT 알림, SSL 보안 통신을 통합하여 안정적이고 보안성 높은 모니터링 솔루션을 제공합니다.



## ✨ 주요 기능

### 🎬 실시간 영상 모니터링
- **RTSP/RTSPS 스트리밍**: OpenCV 기반 실시간 영상 처리
- **다중 카메라 지원**: 여러 CCTV 피드 동시 모니터링
- **영상 설정 제어**: 화면 표시 및 영상 처리 옵션

### 🚨 지능형 알림 시스템
- **실시간 이벤트 감지**: 주정차 위반, 과속, 보행자 감지
- **MQTT 기반 알림**: 즉시 알림 전송 및 수신
- **알림 이력 관리**: 이벤트 로그 및 검색 기능

### 🔐 보안 통신
- **SSL/TLS 암호화**: 인증서 기반 보안 통신
- **사용자 인증**: 로그인/회원가입 시스템
- **데이터 보호**: 모든 네트워크 통신 암호화

### 📊 데이터 관리
- **히스토리 뷰**: 이벤트 기록 조회 및 필터링
- **이미지 비교**: 정차 시작/경과 이미지 비교 기능
- **CSV 내보내기**: 데이터 분석을 위한 내보내기



## 🛠 기술 스택

### 개발 환경
- **언어**: C++17
- **프레임워크**: Qt 6.9.1
- **빌드 시스템**: qmake
- **컴파일러**: MinGW 13.1.0 64-bit

### 핵심 라이브러리
- **Qt Modules**:
  - QtCore, QtGui, QtWidgets - UI 프레임워크
  - QtNetwork - TCP/SSL 네트워킹
  - QtMqtt - MQTT 메시징
  - QtMultimedia - 비디오 재생
- **OpenCV 4.5.5** - 컴퓨터 비전 및 이미지 처리
- **OpenSSL** - SSL/TLS 암호화 (Qt 내장)



## 🚀 빠른 시작

### 📋 시스템 요구사항

#### 필수 요구사항
- **OS**: Windows 10/11 (64-bit) - MinGW 64-bit 컴파일러 사용
- **RAM**: 최소 2GB, 권장 4GB (OpenCV 영상 처리용)
- **저장공간**: 
  - 기본 설치: 약 500MB
  - Qt 6.9.1 + OpenCV: 약 3-4GB
  - 개발 환경 전체: 약 5-6GB
- **네트워크**: 인터넷 연결 (RTSP, MQTT, TCP 통신용)

#### 개발 환경 요구사항
- **Qt**: 6.9.1 MinGW 13.1.0 64-bit
- **OpenCV**: 4.5.5 (MinGW 빌드)
- **컴파일러**: MinGW 13.1.0 64-bit
- **Python**: 3.x (테스트 스크립트용, 선택사항)

### 🔧 개발 환경 설정

#### 1. Qt 설치
```bash
# Qt 6.9.1 MinGW 13.1.0 64-bit 설치
# Qt Creator 설치 시 Additional Libraries에서 다음 모듈 추가:
```
- ✅ Qt Multimedia
- ✅ Qt Network Authorization

#### 2. Qt MQTT 모듈 설치
```bash
# Qt MQTT 모듈을 Qt 설치 경로에 추가
# 다운로드: https://github.com/qt/qtmqtt/tree/6.9.1
# 설치 경로: ~Qt\6.9.1\mingw_64
```

#### 3. OpenCV 설치
```bash
# OpenCV-MinGW-Build 다운로드 및 설치
# 필수 설치 경로: C:\opencv
# 다운로드: https://github.com/huihut/OpenCV-MinGW-Build
# 압축 해제 후 C:\opencv\OpenCV-MinGW-Build 구조가 되도록 설치
```

**중요**: OpenCV는 `C:\opencv` 경로에 설치해야 합니다. 

### 🏗 프로젝트 빌드

#### Qt Creator 사용 (권장)
1. `QuadQT.pro` 파일을 Qt Creator에서 열기
2. 프로젝트 구성 (MinGW 64-bit 선택)
3. 빌드 및 실행 (Ctrl+R)

#### 명령줄 빌드
```bash
# 프로젝트 디렉토리로 이동
cd QuadQT

# Makefile 생성
qmake QuadQT.pro

# 빌드 실행
mingw32-make
```

### 🔧 DLL 설정 (필수)
빌드 후 실행 시 OpenCV DLL 오류가 발생합니다. 

  copy_files.bat 실행을 통해 자동 작업
  
  < 실행이 안될경우>
  
```bash
# 프로젝트의 dlls 폴더에 있는 16개의 OpenCV DLL 파일을
# 빌드 출력 디렉토리로 복사
copy dlls\*.dll Desktop_Qt_6_9_1_MinGW_64_bit-Debug\debug\
```

**포함된 DLL 파일들** (총 16개, 약 200MB):

#### 🔧 핵심 모듈
- `libopencv_core455.dll` - OpenCV 핵심 기능 (행렬 연산, 메모리 관리)
- `libopencv_imgproc455.dll` - 이미지 처리 (필터링, 변환, 기하학적 변환)
- `libopencv_imgcodecs455.dll` - 이미지 코덱 (JPEG, PNG, BMP 등 읽기/쓰기)
- `libopencv_videoio455.dll` - 비디오 입출력 (카메라, 비디오 파일 처리)
- `libopencv_highgui455.dll` - GUI 기능 (윈도우 표시, 이벤트 처리)

#### 🎬 비디오 및 스트리밍
- `opencv_videoio_ffmpeg455_64.dll` - FFmpeg 비디오 코덱 (다양한 비디오 포맷 지원)
- `libopencv_video455.dll` - 비디오 분석 (객체 추적, 배경 제거)

#### 🤖 고급 기능
- `libopencv_calib3d455.dll` - 카메라 캘리브레이션 및 3D 재구성
- `libopencv_features2d455.dll` - 특징점 검출 및 매칭 (SIFT, ORB 등)
- `libopencv_flann455.dll` - 고속 근사 최근접 이웃 라이브러리
- `libopencv_objdetect455.dll` - 객체 검출 (얼굴, 보행자 검출)
- `libopencv_photo455.dll` - 사진 처리 (노이즈 제거, HDR)
- `libopencv_ml455.dll` - 머신러닝 알고리즘
- `libopencv_dnn455.dll` - 딥러닝 추론 엔진
- `libopencv_stitching455.dll` - 이미지 스티칭 (파노라마 생성)
- `libopencv_gapi455.dll` - Graph API (성능 최적화된 이미지 처리)


## ⚙️ 설정

### 📝 config.ini
애플리케이션의 주요 설정을 관리합니다:

```ini
[rtsp]
# RTSP 스트리밍 URL (RTSPS 보안 연결 지원)
url=rtsps://127.0.0.1:8555/test

[mqtt]
# MQTT 브로커 설정
broker_url=mqtt://127.0.0.1:1883
subscribe_topic=alert
publish_topic=alert

[tcp]
# TCP 서버 연결 설정
ip=127.0.0.1
port=8080
timeout=5000

[SSL]
# SSL/TLS 보안 설정
enabled=true
ca_cert=resources/certs/ca.cert.pem
client_cert=resources/certs/client.cert.pem
client_key=resources/certs/client.key.pem
```

### 🔐 SSL 인증서 구성
```
resources/certs/
├── ca.cert.pem      # CA 인증서 (Certificate Authority)
├── client.cert.pem  # 클라이언트 인증서
└── client.key.pem   # 클라이언트 개인키
```

### 🎛 네트워크 설정
- **RTSP**: 실시간 영상 스트리밍 프로토콜
- **MQTT**: 경량 메시징 프로토콜 (알림용)
- **TCP**: 안정적인 데이터 전송 (SSL 암호화)
- **SSL/TLS**: 모든 통신 암호화



## 🧪 테스트

### 🔒 SSL 서버 테스트
로컬 SSL 테스트 서버를 실행하여 애플리케이션의 SSL 통신을 테스트합니다:

```bash
cd test
python test_ssl_server.py
```

**테스트 기능**:
- 사용자 등록/로그인
- SSL 핸드셰이크
- 인증서 검증
- 데이터 암호화 통신




## 📁 프로젝트 구조

```
QuadQT/
├── 📂 src/                     #💻 소스 파일 (.cpp)
│   ├── 📂 login/              # 🔐 로그인 시스템
│   │   ├── loginpage.cpp      # 로그인 UI 처리
│   │   ├── networkmanager.cpp # TCP/SSL 네트워크 통신
│   │   └── custommessagebox.cpp # 커스텀 메시지 박스
│   └── 📂 mainwindow/         # 🏠 메인 애플리케이션
│       ├── mainwindow.cpp     # 메인 윈도우 관리
│       ├── rtspthread.cpp     # RTSP 스트리밍 처리
│       ├── mqttmanager.cpp    # MQTT 메시징
│       ├── notificationpanel.cpp # 알림 패널
│       ├── historyview.cpp    # 히스토리 뷰
│       ├── getimageview.cpp # 이미지 캡처 뷰
│       ├── compareimageview.cpp # 이미지 비교 뷰
│       ├── topbarwidget.cpp # 상단 바 위젯
│       ├── procsettingbox.cpp # 처리 설정 박스
│       ├── displaysettingbox.cpp # 디스플레이 설정 박스
│       ├── tcphistoryhandler.cpp # TCP 히스토리 핸들러
│       └── filenameutils.cpp # 파일명 유틸리티
├── 📂 include/                # 📋 헤더 파일 (.h)
│   ├── 📂 login/              # 로그인 시스템 헤더
│   └── 📂 mainwindow/         # 메인 애플리케이션 헤더
├── 📂 resources/              # 🎨 정적 리소스
│   ├── 📂 certs/             # SSL 인증서
│   ├── 📂 fonts/             # 한화 기업 폰트
│   └── 📂 images/            # 아이콘 및 이미지
├── 📂 ui/                     # Qt Designer UI 파일
├── 📂 test/                   # 🧪 테스트 스크립트
│   └── test_ssl_server.py    # SSL 서버 테스트
├── 📂 dlls/                   # 📚 OpenCV DLL 파일들
├── 📄 QuadQT.pro             # qmake 프로젝트 파일
├── 📄 config.ini             # 애플리케이션 설정
├── 📄 resource.qrc           # Qt 리소스 파일
└── 📄 README.md              # 프로젝트 문서
```



## 🔧 주요 명령어

### 빌드 명령어
```bash
# Makefile 생성
qmake QuadQT.pro

# 프로젝트 빌드
mingw32-make

# 클린 빌드
mingw32-make clean
```

### 개발 도구
```bash
# Qt Designer로 UI 편집
designer ui/loginpage.ui

# Qt Assistant 도움말
assistant

# qmake 도움말
qmake --help
```



## 🌐 네트워크 프로토콜

| 프로토콜 | 용도 | 포트 | 보안 |
|---------|------|------|------|
| **RTSPS** | 실시간 비디오 스트리밍 | 8555 | TLS 암호화 |
| **MQTT** | 경량 메시징 (알림) | 1883 | SSL/TLS 지원 |
| **TCP** | 안정적인 데이터 전송 | 8080 | SSL/TLS 암호화 |



## 🎯 사용 사례

### 교통 모니터링
- 🚗 **과속 감지**: 실시간 속도 측정 및 알림
- 🚫 **주정차 위반**: 불법 주차 차량 감지
- 🚶 **보행자 안전**: 횡단보도 보행자 감지

### 보안 감시
- 📹 **24시간 모니터링**: 연속 영상 감시
- 🚨 **실시간 알림**: 즉시 이벤트 통지
- 📊 **데이터 조회**: 이벤트 기록 및 이미지 보기

