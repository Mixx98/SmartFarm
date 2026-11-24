# 🌱 SmartFarm

ESP32로 만든 **MQTT 기반 스마트팜 IoT 디바이스**입니다.
온도·습도·조도·토양 수분을 1초마다 서버로 보내고, 서버에서 오는 명령으로 LED 조명과 물 펌프를 제어합니다.

## 주요 기능

- **센서 데이터 수집 & 전송** – DHT11(온습도), 조도 센서, 토양 수분 센서 값을 JSON으로 만들어 MQTT 발행
- **원격 제어** – `cmd/unit001/led`, `cmd/unit001/pump` 토픽으로 0~255 값을 받으면 PWM으로 LED 밝기·펌프 세기 조절
- **듀얼 코어 활용** – FreeRTOS 태스크로 코어 0은 송신, 코어 1은 수신을 맡아 동시에 처리
- **NTP 시간 동기화** – 한국 시간(KST) 기준 타임스탬프 기록

## MQTT 메시지

**발행** `data/unit001`

```json
{
  "device_id": "unit001",
  "temp": 24.5,
  "humidity": 60,
  "light": 812,
  "moisture": 1530,
  "created_at": 1682900000
}
```

**구독** `cmd/unit001/#`

| 토픽 | 값 | 동작 |
| --- | --- | --- |
| `cmd/unit001/led` | 0 ~ 255 | LED 밝기 |
| `cmd/unit001/pump` | 0 ~ 255 | 펌프 세기 |

## 핀 배치

| 핀 | 연결 |
| --- | --- |
| GPIO 4 | DHT11 |
| GPIO 34 | 토양 수분 센서 |
| GPIO 35 | 조도 센서 |
| GPIO 32 / 33 | LED (모터 드라이버) |
| GPIO 25 / 26 | 펌프 (모터 드라이버) |

## 사용 기술

`ESP32 (Feather)` `PlatformIO` `Arduino` `MQTT (PubSubClient)` `ArduinoJson` `FreeRTOS` `DHT11`

## 실행 방법

1. VS Code에 PlatformIO 확장을 설치하고 이 폴더를 엽니다.
2. `src/main.cpp` 상단의 WiFi 정보와 MQTT 브로커 주소·계정을 본인 환경에 맞게 수정합니다.
3. PlatformIO **Upload** 로 보드에 업로드합니다.

---
📅 2023년 · 엘리스 IoT 트랙 프로젝트 (MVP 달성)
