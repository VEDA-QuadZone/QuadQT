# MQTT 테스트 도구 사용법

QuadQT 애플리케이션의 MQTT 알림 시스템을 테스트하기 위한 Python 스크립트입니다.

## 설치

```bash
# Python 패키지 설치
pip install -r requirements.txt
```

## 기본 사용법

### 1. 단일 알림 전송
```bash
# 주정차감지 알림
python mqtt_test.py --event 0

# 과속감지 알림  
python mqtt_test.py --event 1

# 어린이감지 알림
python mqtt_test.py --event 2
```

### 2. 랜덤 알림 여러 개 전송
```bash
# 랜덤 알림 5개 (2초 간격)
python mqtt_test.py --random 5

# 랜덤 알림 10개 (1초 간격)
python mqtt_test.py --random 10 --interval 1
```

### 3. 전체 테스트 시퀀스
```bash
# 모든 이벤트 타입을 순서대로 테스트
python mqtt_test.py --test
```

### 4. 대화형 모드
```bash
# 대화형 모드로 실시간 테스트
python mqtt_test.py --interactive
```

대화형 모드에서 사용 가능한 명령어:
- `0`, `1`, `2`: 해당 이벤트 타입 알림 전송
- `r [개수]`: 랜덤 알림 전송 (예: `r 5`)
- `t`: 테스트 시퀀스 실행
- `q`: 종료

### 5. 다른 MQTT 브로커 사용
```bash
# 다른 호스트/포트 지정
python mqtt_test.py --host localhost --port 1883 --event 0
```

## 이벤트 타입

| 번호 | 이벤트 타입 | 설명 |
|------|-------------|------|
| 0    | 주정차감지  | 불법 주정차 감지 |
| 1    | 과속감지    | 속도 위반 감지 |
| 2    | 어린이감지  | 어린이 보호구역 감지 |

## 메시지 형식

전송되는 MQTT 메시지는 다음과 같은 JSON 형식입니다:

```json
{
  "event": 0,
  "timestamp": "2025-07-30T10:30:00.123456"
}
```

## 문제 해결

### 연결 실패 시
1. MQTT 브로커가 실행 중인지 확인
2. IP 주소와 포트 번호 확인 (기본: 192.168.0.10:1883)
3. 방화벽 설정 확인

### QuadQT에서 알림이 표시되지 않을 때
1. QuadQT 애플리케이션이 실행 중인지 확인
2. MQTT 연결 상태 확인 (로그에서 "[MQTT] Connected!" 메시지 확인)
3. 토픽 이름이 일치하는지 확인 (기본: "alert")

## 예시 시나리오

### 시나리오 1: 기본 테스트
```bash
# 1. 전체 테스트 실행
python mqtt_test.py --test

# 2. QuadQT에서 3개의 알림이 순서대로 나타나는지 확인
```

### 시나리오 2: 연속 알림 테스트
```bash
# 1. 빠른 간격으로 여러 알림 전송
python mqtt_test.py --random 10 --interval 0.5

# 2. QuadQT에서 스크롤 기능이 정상 작동하는지 확인
```

### 시나리오 3: 실시간 테스트
```bash
# 1. 대화형 모드 시작
python mqtt_test.py --interactive

# 2. 다양한 명령어로 실시간 테스트
# 예: 0, 1, 2, r 3, t 등
```