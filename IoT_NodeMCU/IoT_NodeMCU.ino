// NodeMCU I2C
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Servo.h> 

// 변수 정의
String data;  // MQTT 송신용
String Air = "Bad ";   // 서보 모터 제어용
int water = 0;  // 물 감지 정보 측정
int servo1_angle = 0; // 1번 서보모터 각도
int servo2_angle = 0; // 2번 서보모터 각도
// 핀 정의
#define SERVO1_PIN D3 // 서보모터 1
#define SERCO2_PIN D4 // 서보모터 2
#define WATER_PIN D5  // 물 감지 센서

// 와이파이 세팅
const char* ssid = "Your ID";
const char* password = "Your PW";
const char* mqtt_server = "Your IP"; // 매번 확인하고 바꿀 것

// 객체 정의
WiFiClient espClient;       // WiFiClient 객체. 와이파이 연결을 위함.
PubSubClient client(espClient); // PubSubClinet 객체 MQTT 통신을 위함
Servo servo1;   // 1번 서보모터 객체. 창문 ON/OFF를 위함
Servo servo2;   // 2번 서보모터 객체. 창문 ON/OFF를 위함
void setup_wifi(){
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("House/Temperature", "hello world");
      client.publish("House/Humidity", "hello world");
      client.publish("House/Car", "hello world");
      client.publish("House/Air", "hello world");
      client.publish("Window/Servo1", "hello world");
      client.publish("Window/Servo2", "hello world");
      client.publish("Window/Water", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600); // 시리얼 모니터용 하드웨어 시리얼 시작
  setup_wifi();   // 와이파이 연결
  client.setServer(mqtt_server, 1883); // MQTT 서버 세팅
  client.setCallback(callback); // MQTT 연결 여부 확인
  servo1.attach(SERVO1_PIN);  // 3번 핀 : Servo 1번
  servo1.attach(SERVO2_PIN);  // 4번 핀 : Servo 2번
  pinMode(WATER_PIN, INPUT); // 5번 핀 : 물감지 센서
}

void loop() {
  water = digitalRead(WATER_PIN);
  data = Serial.readStringUntil('\n');
  // 아두이노 결과 보내기
  if (data[0] == 'A')
    client.publish("House/Air", data.substring(1));
  if (data[0] == 'B')
    client.publish("House/Temperature", data.substring(1));
  if (data[0] == 'C'){
    client.publish("House/Humidity", data.substring(1));
    strncpy(Air, data.substring(1), 4);
  }
  if (data[0] == 'D')
    client.publish("House/Car", data.substring(1));
  
  // 물 감지 센서 결과 보내기
  client.publish("Window/Water", water);

  // 창문 여는 서보모터 조건
  if(water == 0 && Air.equals("Good")){
    servo1.write(30);
    servo2.write(150);
    client.publish("Window/Servo1", 30);
    client.publish("Window/Servo2", 150);
  }
  else{
    servo1.write(0);
    servo2.write(180);
    client.publish("Window/Servo1", 0);
    client.publish("Window/Servo2", 180);
  }
  delay(50);
}