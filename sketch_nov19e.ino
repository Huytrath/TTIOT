// dang sua

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

#define WIFI_SSID "M32"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyBsyW-s3857KeVhe5WAMcU2nmvFHLPFvZM"
#define DATABASE_URL "https://ttiot-2f1cd-default-rtdb.firebaseio.com"

// #define led1 13
// #define led2 12
// #define led3 14
// #define led4 27

// dong co
const int LPWM = 17;  // Chân PWM kết nối với LPWM của BTS7960
const int RPWM = 5;   // Chân PWM kết nối với RPWM của BTS7960
const int bientroPin = 34; // Chân analog kết nối với biến trở
//----------------------


const int ledPin = 13; // Chân kết nối với LED
// const int btnPin = 15; // Chân kết nối với nút nhấn
// int btnState = 0;   


// #define btn1 15
const int DHTPIN = 14;
const int DHTTYPE = DHT22; 
DHT dht(DHTPIN, DHTTYPE);

//sieu am
#define TRIG_PIN 4
#define ECHO_PIN 15
#define TIME_OUT 5000
//-----------


FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

unsigned long t1 = 0;
unsigned int x;
// String path_led = "/livingroom/device/led";
// String path_maylanh = "/livingroom/device/maylanh";
// String path_tv = "/livingroom/device/tv";
// String path_hutbui = "/livingroom/device/hutbui";
// String path_sensor = "/livingroom/sensor";
String path_sensor ="/site1";

int gtbientro;  // gia tri dôc tu bien tro(dong co)

// ham doc khoang cach(sieu âm)

float getDistance()
{
	long duration, distanceCm;
	 
	 digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);
	
	duration = pulseIn(ECHO_PIN, HIGH, TIME_OUT);
 
	// convert to distance
	distanceCm = duration / 29.1 / 2;
	
	return distanceCm;
}

//---------------------

void setup() {
 Serial.begin(115200);
  dht.begin();
   pinMode(TRIG_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);
   pinMode(LPWM, OUTPUT);
  pinMode(RPWM, OUTPUT);

  pinMode(ledPin, OUTPUT);    // Chế độ OUTPUT cho chân LED
// pinMode(btnPin, INPUT);     // Chế độ INPUT cho chân nút nhấn

   // Khởi tạo PWM cho LPWM và RPWM
  ledcSetup(0, 5000, 8);  // Channel 0, tần số 5000 Hz, độ phân giải 8 bit
  ledcSetup(1, 5000, 8);  // Channel 1, tần số 5000 Hz, độ phân giải 8 bit

  ledcAttachPin(LPWM, 0);  // Kết nối PWM channel 0 với LPWM
  ledcAttachPin(RPWM, 1);  // Kết nối PWM channel 1 với RPWM


  // pinMode(led1, OUTPUT);
  // pinMode(led2, OUTPUT);
  // pinMode(led3, OUTPUT);
  // pinMode(led4, OUTPUT);
  // pinMode(btn1, INPUT_PULLUP);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
 
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up successful");
  } else {
    Serial.printf("Sign up failed. Reason: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  
}


void loop() {
  delay(2000);
  // int status_led,status_maylanh, status_hutbui;
  // String status_tv;
  // int trangthai1 = digitalRead(btn1);
  float doam = dht.readHumidity(); //Đọc độ ẩm

  float doC = dht.readTemperature(); //Đọc nhiệt độ C
  // Serial.println(trangthai1);
  if (isnan(doam) || isnan(doC)) 
  {
    Serial.println("Không có giá trị trả về từ cảm biến DHT");
    return;
  }
  // Firebase.setFloat(fbdo, path_sensor + "/temp",doC);
  // Firebase.setFloat(fbdo, path_sensor + "/doam",doam);
  if (Firebase.setFloat(fbdo, path_sensor + "/temp", doC)) {
  Serial.println("Temperature sent successfully");
} else {
  Serial.println("Failed to send temperature data");
}

if (Firebase.setFloat(fbdo, path_sensor + "/doam", doam)) {
  Serial.println("Humidity sent successfully");
} else {
  Serial.println("Failed to send humidity data");
}
   Serial.print(F("Humidity: "));
  Serial.print(doam);

  Serial.print(F("%  Temperature: "));
  Serial.print(doC);
    Serial.println();

  // in ra khoang cach
long distance = getDistance();

	if (distance <= 0)
	{
		Serial.println("Echo time out !!");
	}
	else
	{		
		Serial.print("Distance to nearest obstacle (cm): ");
		Serial.println(distance);
    Firebase.setFloat(fbdo, path_sensor + "/khoangcach", distance);
	}

//------------------

  // bat dau dong co 

  gtbientro = analogRead(bientroPin); // Đọc giá trị từ biến trở

  int giatripwm = map(gtbientro, 0, 4095, 0, 255); // Ánh xạ giá trị từ biến trở sang PWM

  // Serial.print("GT biến trở: ");
  // Serial.print(gtbientro);
  // Serial.print("    GT PWM: ");
  // Serial.println(giatripwm);

  // Ghi giá trị PWM cho LPWM và RPWM
  ledcWrite(0, giatripwm);  // Ghi giá trị PWM cho LPWM
  ledcWrite(1, 0);  // Tắt RPWM
  
  //------------------het dong co


  // Gửi giá trị PWM lên Firebase
if (Firebase.setInt(fbdo, path_sensor + "/motor_pwm", giatripwm)) {
    Serial.println("Motor PWM sent successfully");
} else {
    Serial.println("Failed to send motor PWM data");
}
//----------------------------

// //Đọc giá trị từ Firebase
//   int ledState = 0;
//   if (Firebase.getInt(fbdo, path_sensor + "/led_state")) {
//     ledState = fbdo.intData();
//     Serial.print("LED state from Firebase: ");
//     Serial.println(ledState);
//   } else {
//     Serial.println("Failed to get LED state from Firebase");
//   }

  //   // Điều khiển LED dựa trên giá trị từ Firebase
  //   digitalWrite(ledPin, ledState);

  //   // Gửi trạng thái LED lên Firebase
  //   if (Firebase.setInt(fbdo, path_sensor + "/led_state", ledState)) {
  //     Serial.println("LED state sent successfully");
  //   } else {
  //     Serial.println("Failed to send LED state to Firebase");
  //   }
  // } else {
  //   Serial.println("Failed to get LED state from Firebase");
  // }
 // Đọc giá trị từ Firebase
  int ledState;
  if (Firebase.getInt(fbdo, path_sensor + "/ledState")) {
    ledState = fbdo.intData();
    Serial.print("LED state from Firebase: ");
    Serial.println(ledState);
  } else {
    Serial.println("Failed to get LED state from Firebase");
  }




//   if(Firebase.getInt(fbdo, path_maylanh + "/status"))    status_maylanh = fbdo.intData();
//   if(Firebase.getInt(fbdo, path_led + "/status"))        status_led = fbdo.intData();
//   if(Firebase.getString(fbdo, path_tv + "/status"))         status_tv = fbdo.stringData();
//   if(Firebase.getInt(fbdo, path_hutbui + "/status"))    status_hutbui = fbdo.intData();
//   if(status_maylanh == 1)   
//   {
//     digitalWrite(led1, HIGH);
//     if(trangthai1 == 1){
//       delay(200);
//        digitalWrite(led1, LOW);
//       Firebase.setInt(fbdo, path_maylanh + "/status",0);
//     }
//   }
//   if(status_maylanh == 0)  {
//     digitalWrite(led1, LOW);
//    if(trangthai1 == 1){
//       delay(200);
//        digitalWrite(led1, HIGH);
//       Firebase.setInt(fbdo, path_maylanh + "/status",1);
//     }
//   }
//   if(status_led == 1)   digitalWrite(led2, HIGH);
//   if(status_led == 0)   digitalWrite(led2, LOW);

//   if(status_tv == "1")   digitalWrite(led3, HIGH);
// if(status_tv == "0")   digitalWrite(led3, LOW);

//   if(status_hutbui == 1)   digitalWrite(led4, HIGH);
//   if(status_hutbui == 0)   digitalWrite(led4, LOW);

}
