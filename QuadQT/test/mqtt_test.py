#!/usr/bin/env python3
"""
MQTT 테스트 스크립트
QuadQT 애플리케이션의 알림 시스템을 테스트하기 위한 MQTT 메시지 발송 도구
"""

import paho.mqtt.client as mqtt
import json
import time
import random
from datetime import datetime
import argparse
import sys

# MQTT 설정 (config.ini와 동일하게 설정)
MQTT_BROKER = "192.168.0.10"
MQTT_PORT = 1883
MQTT_TOPIC = "alert"

# 이벤트 타입 정의
EVENT_TYPES = {
    0: "주정차감지",
    1: "과속감지", 
    2: "보행자감지"
}

class MQTTTester:
    def __init__(self, broker_host=MQTT_BROKER, broker_port=MQTT_PORT):
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.client = mqtt.Client()
        self.connected = False
        
        # 콜백 함수 설정
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.on_publish = self.on_publish
        
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print(f"✅ MQTT 브로커에 연결됨: {self.broker_host}:{self.broker_port}")
            self.connected = True
        else:
            print(f"❌ MQTT 연결 실패. 코드: {rc}")
            self.connected = False
            
    def on_disconnect(self, client, userdata, rc):
        print(f"🔌 MQTT 브로커 연결 해제됨")
        self.connected = False
        
    def on_publish(self, client, userdata, mid):
        print(f"📤 메시지 발송 완료 (ID: {mid})")
        
    def connect(self):
        """MQTT 브로커에 연결"""
        try:
            print(f"🔄 MQTT 브로커 연결 시도: {self.broker_host}:{self.broker_port}")
            self.client.connect(self.broker_host, self.broker_port, 60)
            self.client.loop_start()
            
            # 연결 대기
            timeout = 5
            while not self.connected and timeout > 0:
                time.sleep(0.1)
                timeout -= 0.1
                
            return self.connected
        except Exception as e:
            print(f"❌ 연결 오류: {e}")
            return False
            
    def disconnect(self):
        """MQTT 브로커 연결 해제"""
        self.client.loop_stop()
        self.client.disconnect()
        
    def send_alert(self, event_type, custom_timestamp=None):
        """알림 메시지 전송"""
        if not self.connected:
            print("❌ MQTT 브로커에 연결되지 않음")
            return False
            
        # 타임스탬프 생성
        if custom_timestamp:
            timestamp = custom_timestamp
        else:
            timestamp = datetime.now().isoformat()
            
        # 메시지 생성
        message = {
            "event": event_type,
            "timestamp": timestamp
        }
        
        # JSON 문자열로 변환
        json_message = json.dumps(message, ensure_ascii=False)
        
        try:
            # 메시지 발송
            result = self.client.publish(MQTT_TOPIC, json_message)
            
            event_name = EVENT_TYPES.get(event_type, f"알 수 없는 이벤트({event_type})")
            print(f"📨 알림 전송: {event_name} - {timestamp}")
            print(f"📄 메시지: {json_message}")
            
            return result.rc == mqtt.MQTT_ERR_SUCCESS
        except Exception as e:
            print(f"❌ 메시지 전송 오류: {e}")
            return False
            
    def send_random_alerts(self, count=5, interval=2):
        """랜덤 알림 여러 개 전송"""
        print(f"🎲 랜덤 알림 {count}개 전송 시작 (간격: {interval}초)")
        
        for i in range(count):
            event_type = random.choice(list(EVENT_TYPES.keys()))
            self.send_alert(event_type)
            
            if i < count - 1:  # 마지막이 아니면 대기
                print(f"⏳ {interval}초 대기...")
                time.sleep(interval)
                
        print("✅ 랜덤 알림 전송 완료")
        
    def send_test_sequence(self):
        """테스트 시퀀스 실행"""
        print("🧪 테스트 시퀀스 시작")
        
        # 각 이벤트 타입별로 하나씩 전송
        for event_type, event_name in EVENT_TYPES.items():
            print(f"\n--- {event_name} 테스트 ---")
            self.send_alert(event_type)
            time.sleep(1)
            
        print("\n✅ 테스트 시퀀스 완료")

def main():
    parser = argparse.ArgumentParser(description='MQTT 알림 테스트 도구')
    parser.add_argument('--host', default=MQTT_BROKER, help='MQTT 브로커 호스트')
    parser.add_argument('--port', type=int, default=MQTT_PORT, help='MQTT 브로커 포트')
    parser.add_argument('--event', type=int, choices=[0, 1, 2], help='이벤트 타입 (0:주정차, 1:과속, 2:보행자)')
    parser.add_argument('--random', type=int, metavar='COUNT', help='랜덤 알림 개수')
    parser.add_argument('--interval', type=float, default=2.0, help='랜덤 알림 간격(초)')
    parser.add_argument('--test', action='store_true', help='테스트 시퀀스 실행')
    parser.add_argument('--interactive', action='store_true', help='대화형 모드')
    
    args = parser.parse_args()
    
    # MQTT 테스터 생성
    tester = MQTTTester(args.host, args.port)
    
    # 연결 시도
    if not tester.connect():
        print("❌ MQTT 브로커 연결 실패")
        sys.exit(1)
        
    try:
        if args.event is not None:
            # 단일 이벤트 전송
            tester.send_alert(args.event)
            
        elif args.random:
            # 랜덤 알림 전송
            tester.send_random_alerts(args.random, args.interval)
            
        elif args.test:
            # 테스트 시퀀스 실행
            tester.send_test_sequence()
            
        elif args.interactive:
            # 대화형 모드
            interactive_mode(tester)
            
        else:
            # 기본: 도움말 표시
            parser.print_help()
            print("\n📋 사용 예시:")
            print("  python mqtt_test.py --event 0          # 주정차감지 알림 1개")
            print("  python mqtt_test.py --random 5         # 랜덤 알림 5개")
            print("  python mqtt_test.py --test              # 전체 테스트")
            print("  python mqtt_test.py --interactive       # 대화형 모드")
            
    except KeyboardInterrupt:
        print("\n⏹️  사용자에 의해 중단됨")
    finally:
        tester.disconnect()
        print("👋 프로그램 종료")

def interactive_mode(tester):
    """대화형 모드"""
    print("\n🎮 대화형 모드 시작")
    print("명령어:")
    print("  0, 1, 2  - 해당 이벤트 타입 알림 전송")
    print("  r [개수] - 랜덤 알림 전송 (기본: 3개)")
    print("  t        - 테스트 시퀀스 실행")
    print("  q        - 종료")
    print()
    
    while True:
        try:
            cmd = input("명령어 입력> ").strip().lower()
            
            if cmd == 'q':
                break
            elif cmd in ['0', '1', '2']:
                event_type = int(cmd)
                tester.send_alert(event_type)
            elif cmd.startswith('r'):
                parts = cmd.split()
                count = int(parts[1]) if len(parts) > 1 else 3
                tester.send_random_alerts(count, 1)
            elif cmd == 't':
                tester.send_test_sequence()
            else:
                print("❓ 알 수 없는 명령어")
                
        except (ValueError, IndexError):
            print("❌ 잘못된 입력")
        except KeyboardInterrupt:
            break
            
    print("대화형 모드 종료")

if __name__ == "__main__":
    main()