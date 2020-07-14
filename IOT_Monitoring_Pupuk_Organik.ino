//Wifi & Firebase
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
//Software Serial Arduino
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
//Sensor Suhu dan Kelembaban
#include "DHT.h"
#include <DallasTemperature.h>
#include <OneWire.h>
//Verifikasi Konek Ke Wifi dan Firebase
#define FIREBASE_HOST "set firebase host here"                  
#define FIREBASE_AUTH "set firebase auth here"            
#define WIFI_SSID "Mi Phone"                                        
#define WIFI_PASSWORD ""  
//Identifikasi Pin Sensor
#define Relay1 5  //D1
#define Relay2 4  //D2
#define ONE_WIRE_BUS 0 //D3 
#define DHTPIN 14  //D5
#define DHTTYPE DHT22   // DHT 22
//Identifikasi Sensor Suhu
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature SuhuPupuk(&oneWire); 
float Celcius;
//Identifikasi Variable           
int val1;
int val2;
int val;
int data;
SoftwareSerial s(D7,D6);

void setup() {
  Serial.begin(115200);
  s.begin(115200);
  SuhuPupuk.begin();
  while (!Serial) continue;
  dht.begin();
  pinMode(Relay1,OUTPUT);
  pinMode(Relay2,OUTPUT);
  digitalWrite(Relay1,HIGH);
  digitalWrite(Relay2,HIGH);
  
  
  // Koneksi ke wifi dan Firebase.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting Wifi To ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
  }
  Serial.println();
  Serial.print("Connected to IP: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setString("Status Koneksi", "Disconnected");
  Firebase.setInt("Relay1",0);                  
  Firebase.setInt("Relay2",0);

}

int n = 0;

void KoneksiFirebase()
{
  // set value firebase
  Firebase.setFloat("number", 42.0);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      Firebase.setString("Status Koneksi", "Disconnected");  
      return;
  }
  delay(1000);

  // update value firebase
  Firebase.setFloat("number", 43.0);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      Firebase.setString("Status Koneksi", "Disconnected");  
      return;
  }
  delay(1000);
  
  // get value firebase
  Serial.print("number: ");
  Serial.println(Firebase.getFloat("number"));
  delay(1000);

  // remove value firebase
  Firebase.remove("number");
  delay(1000);

  // set string value firebase
  Firebase.setString("TITLE", "MONITORING DEKOMPOSER");
  Firebase.setString("Status Koneksi", "Connected");
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error()); 
      Firebase.setString("Status Koneksi", "Disconnected"); 
      return;
  }
  delay(1000);

  // set bool value
  Firebase.setBool("truth", false);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /truth failed:");
      Serial.println(Firebase.error());
      Firebase.setString("Status Koneksi", "Disconnected");  
      return;
  }
  delay(1000);

  // append a new value to /logs
  String name = Firebase.pushInt("logs", n++);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logs failed:");
      Serial.println(Firebase.error());
      Firebase.setString("Status Koneksi", "Disconnected");  
      return;
  }
  
  
  Serial.print("pushed: /logs/");
  Serial.println(name);
  delay(1000);
}

void loop() {
  
//Pengambilan data dari arduino
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s);
 
  if (root == JsonObject::invalid())
  {
    return;
  }
  Serial.println("JSON received and parsed");
  root.prettyPrintTo(Serial);
  Serial.println("");
  delay(1000);
  Serial.print("Kelembaban Pupuk");
  int data1=root["Kelembaban Pupuk"];
  Serial.println(data1);
  Firebase.setFloat("Kelembaban Pupuk",data1);
  int data2=root["Status Pupuk"];
  if (data2 == 0)
  {
    Firebase.setString("Status Pupuk","Lembab");
  }
  else
  {
    Firebase.setString("Status Pupuk","Kering");
  }
  Serial.println(data2);
  Serial.println("");
  Serial.println("---------------------JSO--------------------");
  Serial.println("");
  
  //Mencari nilai suhu dan kelembaban sensor DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
    // Check Jika pembacaan sensor gagal
    if (isnan(h) || isnan(t))
    {
        Serial.println("Gagal membaca data dari DHT sensor!");
        return;
    }
  Firebase.setInt ("Suhu Ruangan",t);
  Firebase.setFloat ("Kelembaban Ruangan",h);
  
  //Pengambilan data sensor Suhu Pupuk 
  SuhuPupuk.requestTemperatures();
  Celcius = SuhuPupuk.getTempCByIndex(0);   
  Serial.print("Suhu Pupuk:");         
  Serial.println(Celcius);
  Firebase.setInt ("Suhu Pupuk",Celcius);
  delay(500);

  //Kontrol Relay
  delay(200);
  val1=Firebase.getString("Relay1").toInt(); 
  val2=Firebase.getString("Relay2").toInt();   

  if(val1==1)                                                          
     {
      digitalWrite(Relay1,LOW);
      Serial.println("Mesin ON");
      Firebase.setString("Status Mesin", "Menyala");
    }
    else if(val1==0)                                                    
    {                                      
      digitalWrite(Relay1,HIGH);
      Serial.println("Mesin OFF");
      Firebase.setString("Status Mesin", "Mati");
    }
                              
  if(val2==1)                                                             
     {
      digitalWrite(Relay2,LOW);
      Serial.println("Pompa ON");
    }
    else if(val2==0)                                                      
    {                                      
      digitalWrite(Relay2,HIGH);
      Serial.println("Pompa OFF");
    }

    //Pemanggilan Void Koneksi Firebase
    KoneksiFirebase();
}
  
