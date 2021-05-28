#include <esp_task_wdt.h>
//20 seconds WDT
#define WDT_TIMEOUT 20
#include <WiFi.h>
#include <PubSubClient.h>
#include "M5Atom.h"
#include "AtomSocket.h"
#include <WiFiClient.h>

//Serial para conectar con arduino
#define RXD2 19
#define TXD2 18
#define limite_fallos 100

//Definición red wifi
const char* ssid = "WifiAX";
const char* password = "hkmhkm1234566";

//Definicion MQTT
const char* mqtt_server = "iot.igromi.com";
const char* mqtt_id = "energia1";
const char* mqtt_user = "energia1";
const char* mqtt_pass = "hkmhkm1234566";

//Variables
WiFiClient espClient;
PubSubClient client(espClient);

//Definicion variables electricas
int Voltage, ActivePower = 0;
float Current = 0;

#define RXD 22
#define RELAY 23

ATOMSOCKET ATOM;
HardwareSerial AtomSerial(2);

String str;
char payload[100];
int fallos=0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch 
  setup_wifi();
 
  client.setServer(mqtt_server, 1883);
  client.subscribe("v1/devices/me/telemetry",1);

   M5.begin(true, false, true);
   M5.dis.drawpix(0, 0xe0ffff);
   ATOM.Init(AtomSerial, RELAY, RXD);
  
}

//Conexión wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP : ");
  Serial.println(WiFi.localIP());
};

//Reconección MQTT
void reconnect() 
{
  // Loop de reconexion MQTT
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    // Intentado conectar
    if (client.connect(mqtt_id,mqtt_user,mqtt_pass)) {
      Serial.println("conectado al server");
      //Topico
      client.subscribe("v1/devices/me/telemetry",1);     
    } else {
      Serial.print("RC-");
      Serial.println(client.state());
      Serial.println("Reintentando....");
      delay(500);      
  }
 }
};

//Rutina para el envio de datos por MQTT
void EnvioMQTT(float Data,String ID) {
      //Se genera estructura de thingsboard
      str= "{\""+ID+"\":\""+String(Data)+"\"}";
      str.toCharArray(payload,100);
      Serial.println(payload);
      client.publish("v1/devices/me/telemetry",payload);
}
void loop() {
  if (!client.connected()) {
    reconnect();
  };
  
   ATOM.SerialReadLoop();
  if(ATOM.SerialRead == 1)   
  {
        Voltage = ATOM.GetVol();
        Current = ATOM.GetCurrent();
        ActivePower = ATOM.GetActivePower();
        
        Serial.println(Voltage);
        Serial.println(Current);
        Serial.println(ActivePower);  
        M5.update();
  
        EnvioMQTT(Voltage,"Vmedidor1");
        EnvioMQTT(Current,"Amedidor1");
        EnvioMQTT(ActivePower,"Pmedidor1");
        
  }
  esp_task_wdt_reset();
  
  if (fallos < limite_fallos ) {
      esp_task_wdt_reset();  
    }
 }
  
