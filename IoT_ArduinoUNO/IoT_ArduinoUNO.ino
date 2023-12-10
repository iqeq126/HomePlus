// Arduino I2C
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
// LCD header
#include <LiquidCrystal_I2C.h>
//DHT header
#include <DHT.h>
// #include <SoftwareSerial.h> 
// #define RX 3 // nodeMcu softSerial TX(NodeMCU D6) -> arduino 2
// #define TX 2 // nodeMcu softSerial RX(NodeMCU D7) -> arduino 3

#define TRIG_PIN 4    // TRIG 핀 설정. 미세먼지 대체
#define ECHO_PIN 5    // ECHO 핀 설정. 미세먼지 대체
#define FAN_PIN 6     // 선풍기 핀 설정
#define DHT_PIN 7    // 온습도 핀 설정
#define BUZ_PIN 8    // 부저 핀 설정
#define RST_PIN 9    // 리셋 핀 설정
#define SS_PIN 10    // SDA_PIN. spi 통신을 위한 SS(chip select)핀 설정
const int BUZ[3] = {523, 622, 659}; // 부저 음 정의

MFRC522 rfid(SS_PIN, RST_PIN); // 'rfid' 이름으로 클래스 객체 선언
MFRC522::MIFARE_Key key;
//Servo myservo; // 서보 모터 객체 선언 => RaspberryPi3로 옮김
LiquidCrystal_I2C lcd(0x27, 16, 2);  //LCD 객체 선언. 이상하면 0x3F,16,2로 바꾸기
// SoftwareSerial nodeMCUSerial(RX, TX); // (RX, TX) SoftWareSerial 객체 생성
DHT dht(DHT_PIN, DHT22);

float Humidity = 1.0; // 습도 변수
float Temperature = 1.0; // 온도 변수
long Dust = 1;     // 미세먼지 변수. 초음파로 대체
boolean WaterFlag = false; // 물 플래그 변수
boolean RFIDFlag = false; // RFID 플래그 변수
byte *temp; // 1바이트 포인터 선언(메모리 주소)

void setup ( ) {
  Serial.begin(9600);         // 시리얼 모니터용 하드웨어 시리얼 시작
  //nodeMCUSerial.begin(9600);     // 센서 제어용 소프트웨어 시리얼 시작
  SPI.begin();           // SPI 통신 시작
  rfid.PCD_Init();       // RFID(MFRC522) 초기화
  lcd.init(); lcd.backlight(); lcd.clear(); //LCD 초기화

  // ID값 초기화 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
// RFID : MIFARE 타입의 카드키 종류들만 인식됨을 표시
  //Serial.println(F("This code scan the MIFARE Classsic NUID."));
  //Serial.print(F("Using the following key:"));
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

// 핀모드 정의
  pinMode(TRIG_PIN, OUTPUT); // 4 : TRIG핀
  pinMode(ECHO_PIN, INPUT); // 5 : ECHO 핀
  pinMode(FAN_PIN, OUTPUT); // 6 : 선풍기 핀
  //pinMode(DHT_PIN, OUTPUT); // 7 : 온습도 센서
  dht.begin();
  pinMode(BUZ_PIN, OUTPUT); // 8 : 부저
}

void loop ( )  {
 // RFID Part
 // 카드 읽힘이 제대로 되면 토글해줌
  if (rfid.PICC_IsNewCardPresent() || rfid.PICC_ReadCardSerial()){
    RFIDFlag = !RFIDFlag; // RFIDFlag값을 바꿔준다.
    // 차량 출입을 알리는 부저를 울려준다.
    for ( int i = 0; i < 3; i++){
      tone(BUZ_PIN, BUZ[i], 500);
    }
  }
  // DHT Part
  if(!isnan(dht.readTemperature()) && !isnan(dht.readHumidity())){
    Temperature = dht.readTemperature(); // 온도값 읽어줌
    Humidity = dht.readHumidity(); // 습도값 읽어줌
  }
  // FAN Part
  if (Temperature > 28.0){
    digitalWrite(FAN_PIN, HIGH);
  }
  else{
    digitalWrite(FAN_PIN, LOW);
  }
  delay(10);
  // Dust Part. 초음파로 대체
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delay(10);
  digitalWrite(TRIG_PIN, HIGH);
  delay(980);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn (ECHO_PIN, HIGH); //물체에 반사되어돌아온 초음파의 시간을 변수에 저장
  //34000*초음파가 물체로 부터 반사되어 돌아오는시간 /1000000 / 2(왕복값이아니라 편도값이기때문에 2를 나눠줌)
 //초음파센서의 거리값이 mm로 환산. 여기서는 미세먼지로 치환
  Dust = duration * 17 / 100;
  char Temp[8], Humi[8];
  
  // 현황 파악
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (Dust < 2000.0){
    Serial.println("AGood");
  }
  else{
    Serial.println("ABad ");
  }
  Serial.print("B"); Serial.println(Temperature);
  Serial.print("C"); Serial.println(Humidity);
  Serial.println("D" + String(RFIDFlag));
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  dtostrf(Temperature,6, 2, Temp);
  dtostrf(Humidity,6, 2, Humi);
  // 소프트웨어 시리얼 부분. "\n" 으로 구분 
  // 1. 차량, 2. 온도, 3. 습도, 4. 미세먼지
  //if(nodeMCUSerial.available() > 0){
    //char buffer = Serial.read();
    //nodeMCUSerial.write(buffer);
    //nodeMCUSerial.write(RFIDFlag + "\n"); // 1. 차량
    //nodeMCUSerial.write(Temp,"\n"); // 2. 온도
    //nodeMCUSerial.write(Humi,"\n"); // 3. 습도
    //nodeMCUSerial.write(Dust + "\n"); // 4. 미세먼지*/
  //}
  // LCD Part. 출력형식
  // T: 온도C, H: 습도%
  // Air : Good/Bad Car : O/X
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(2, 0);
  lcd.print(Temperature);
  lcd.setCursor(6, 0);
  lcd.print("C");
  lcd.setCursor(9, 0);
  lcd.print("H:");
  lcd.setCursor(11, 0);
  lcd.print(Humidity);
  lcd.setCursor(15, 0);
  lcd.print("% ");
  lcd.setCursor(0, 1);
  lcd.print("Air:");
  lcd.setCursor(4, 1);
  // Dust Part. 2000 이하일 때 좋음
  if(Dust < 2000.0)
    lcd.print("Good");
  else
    lcd.print("Bad ");
  lcd.setCursor(9,1);
  lcd.print("Car:");
  lcd.setCursor(13,1);
  if(RFIDFlag){
    lcd.print(" O  ");
  }
  else{
    lcd.print(" X  ");
  }

  // delay(1000);
}


// 16진 값으로 변환 해주는 함수 정의
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// 10진 값으로 변환 해주는 함수 정의
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}