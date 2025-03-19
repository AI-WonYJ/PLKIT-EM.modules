// ESP32를 사용하여 BLE를 통해 Wi-Fi SSID, 비밀번호, MQTT 서버 정보를 광고하는 코드
#include <BLEDevice.h>  // BLE 기능을 위한 라이브러리 포함
#include <BLEServer.h>  // BLE 서버 기능을 위한 라이브러리 포함
#include <BLEUtils.h>   // BLE 관련 유틸리티 함수 포함
#include <BLE2902.h>    // BLE 특성에 대한 CCCD(클라이언트 구성 디스크립터) 포함

// BLE 서비스 및 특성의 UUID 정의
#define SERVICE_UUID           "736D6172-7462-6F61-7264-5F706C6B6974"  // BLE 서비스 UUID
#define CHARACTERISTIC_UUID_SSID  "53534343-0000-0000-0000-000000000000"  // SSID 정보 특성 UUID: SSCC
#define CHARACTERISTIC_UUID_PASS  "32303235-7373-6363-0000-000000000000"  // Wi-Fi 비밀번호 특성 UUID: 2025sscc
#define CHARACTERISTIC_UUID_MQTT  "5F5F706D-7174-745F-7365-727665725F49"  // MQTT 서버 IP 특성 UUID

BLEServer *pServer = nullptr; // BLE 서버 포인터 선언 및 초기화 (서버 객체를 저장할 변수)
bool deviceConnected = false; // BLE 장치 연결 상태를 저장하는 변수 (연결 여부 확인용)
unsigned long lastConnectionTime = 0; // 마지막으로 BLE 장치가 연결된 시간을 저장하는 변수수
const unsigned long connectionDuration = 10000; // 최대 연결 유지 시간(10초), 초과시 자동 해제

// BLE 서버 콜백 클래스 정의 (BLE 연결 및 해제 시 동작 설정)
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true; // 장치가 연결되었음을 표시
    lastConnectionTime = millis();  // 현재 시간을 마지막 연결 시간으로 저장
    Serial.println("장치가 연결되었습니다."); // 시리얼 모니터에 연결됨 메시지 출력
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false; // 장치가 연결되지 않았음을 표시
    Serial.println("장치가 연결이 끊겼습니다."); // 시리얼 모니터에 연결 해제 메시지 출력
    pServer->startAdvertising();  // 연결이 끊기면 다시 BLE 광고 시작 (다른 장치가 연결할 수 있도록)
  }
};

void setup() {
Serial.begin(115200); // 시리얼 통신 시작 (전송 속도: 115200bps)

// BLE 장치 초기화 (BLE 장치의 이름 설정)
BLEDevice::init("ESP32 WiFi and MQTT Advertiser");

// BLE 서버 생성
pServer = BLEDevice::createServer();
pServer->setCallbacks(new MyServerCallbacks());  // BLE 서버 콜백 설정 (연결 및 해제 감지)

// BLE 서비스 생성
BLEService *pService = pServer->createService(SERVICE_UUID);

// SSID 특성 생성 및 설정 (Wi-Fi SSID 정보를 읽을 수 있도록 설정)
BLECharacteristic *pCharacteristicSSID = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_SSID,
                                        BLECharacteristic::PROPERTY_READ | // 읽기 속성 부여
                                        BLECharacteristic::PROPERTY_NOTIFY // 알림 속성 부여
                                      );
pCharacteristicSSID->setValue("PLkit"); // 기본 SSID 값 설정

// Wi-Fi 비밀번호 특성 생성 및 설정
BLECharacteristic *pCharacteristicPass = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_PASS,
                                        BLECharacteristic::PROPERTY_READ | // 읽기 속성 부여
                                        BLECharacteristic::PROPERTY_NOTIFY // 알림 속성 부여
                                      );
pCharacteristicPass->setValue("987654321"); // 기본 비밀번호 값 설정

// MQTT 서버 IP 주소 특성 생성 및 설정
BLECharacteristic *pCharacteristicMQTT = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_MQTT,
                                        BLECharacteristic::PROPERTY_READ | // 읽기 속성 부여
                                        BLECharacteristic::PROPERTY_NOTIFY // 알림 속성 부여
                                      );
pCharacteristicMQTT->setValue("192.168.1.5"); // 기본 MQTT 서버 IP 값 설정

// BLE 서비스 시작 (클라이언트가 연결할 수 있도록 준비)
pService->start();

// BLE 광고 시작 설정 (클라이언트가 BLE 장치를 검색할 수 있도록 광고 설정)
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
pAdvertising->addServiceUUID(SERVICE_UUID);  // 서비스 UUID를 광고에 추가 (클라이언트가 식별할 수 있도록)
pAdvertising->setScanResponse(true);  // 스캔 응답 활성화 (추가 데이터 제공 가능)
pAdvertising->setMinPreferred(0x06);  // 최소 광고 간격 설정 (빠른 연결을 위해)
pAdvertising->setMinPreferred(0x12);  // 추가적인 광고 설정
pAdvertising->start();  // BLE 광고 시작 (클라이언트가 장치를 발견할 수 있도록)

Serial.println("BLE WiFi 및 MQTT 정보 광고 시작..."); // 시리얼 모니터에 광고 시작 메시지 출력
}

void loop() {
// BLE 장치가 연결되었는지 확인 후 자동 해제 (10초 후 강제 해제)
if (deviceConnected && (millis() - lastConnectionTime > connectionDuration)) {
  pServer->disconnect(0);  // 10초 후 BLE 연결 강제 해제
}
delay(1000); // 1초마다 루프 실행 (CPU 과부하 방지)
}
