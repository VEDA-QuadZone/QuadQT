# 프로젝트 구조

## 디렉토리 구성

```
QuadQT/
├── src/                    # 소스 파일 (.cpp)
│   ├── login/             # 로그인 시스템 구현
│   └── mainwindow/        # 메인 애플리케이션 로직
├── include/               # 헤더 파일 (.h)
│   ├── login/             # 로그인 시스템 헤더
│   └── mainwindow/        # 메인 애플리케이션 헤더
├── resources/             # 정적 리소스
│   ├── certs/            # SSL 인증서
│   ├── fonts/            # 한화 기업 폰트
│   └── images/           # 애플리케이션 아이콘 및 이미지
├── test/                  # 테스트 유틸리티 및 스크립트
├── build/                 # 빌드 출력 디렉토리
└── .kiro/                # Kiro IDE 설정
```

## 코드 구성 패턴

### 헤더/소스 분리
- **헤더**는 `include/`에 네임스페이스 형태의 하위 디렉토리로 구성
- **구현**은 `src/`에 include 구조를 미러링
- 내부 헤더에는 `#include "module/classname.h"` 형식 사용

### 모듈 구조
- **login/** - 인증, 사용자 관리, 네트워크 통신
- **mainwindow/** - 메인 UI, 비디오 표시, MQTT 처리, 알림

### 파일 명명 규칙
- **설명적인 소문자 이름**: `mqttmanager.cpp`, `rtspthread.h`
- **클래스 이름과 일치**: `MainWindow` 클래스 → `mainwindow.h/cpp`
- **UI 파일**: 루트 디렉토리의 `loginpage.ui`, `mainwindow.ui`

## 주요 파일

### 빌드 설정
- `QuadQT.pro` - 모든 소스, 헤더, 의존성을 포함한 qmake 프로젝트 파일
- `resource.qrc` - 임베디드 자산을 위한 Qt 리소스 파일

### 설정
- `config.ini` - 런타임 설정 (RTSP, MQTT, TCP, SSL 설정)
- SSL 인증서: `ca.cert.pem`, `client.cert.pem`, `client.key.pem`

### 진입점
- `main.cpp` - 애플리케이션 진입점, 폰트 로딩, 로그인/메인 윈도우 루프

## 아키텍처 패턴

### Qt 패턴
- **QMainWindow** - 메인 애플리케이션 윈도우용
- **QDialog** - 로그인/인증용
- **QStackedWidget** - 멀티페이지 네비게이션용
- **Signals/Slots** - 이벤트 처리 및 컴포넌트 통신용

### 스레딩
- **QThread** 서브클래스 - 백그라운드 작업용 (RTSP 스트리밍)
- **Qt 이벤트 루프** - UI 반응성 유지

### 네트워크 컴포넌트
- **NetworkManager** - SSL 지원 HTTP API 통신
- **MqttManager** - 실시간 알림을 위한 MQTT pub/sub
- **RtspThread** - 별도 스레드에서 비디오 스트리밍

### UI 컴포넌트
- **커스텀 위젯** - 특수 기능용
- **Qt Designer 폼** (.ui 파일) - 복잡한 레이아웃용
- **리소스 임베딩** - 폰트 및 이미지용

## 의존성 관리
- **OpenCV** - .pro 파일의 LIBS를 통해 절대 경로로 링크
- **Qt 모듈** - .pro 파일의 QT 변수에 선언
- **시스템 라이브러리** - Qt 내장 OpenSSL 통합을 통한 SSL 지원