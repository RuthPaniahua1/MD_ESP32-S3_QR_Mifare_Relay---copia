//MASTER 
#include <Arduino.h>
#include "QReaderWork.h"
#include "SPG75QR.h"
#include "ESPWEB.h"
#include <Preferences.h>
#include "StatisticsValues.h"
#include "buzzerledcontrol.h"
#include "SPIFFS.h"
#include <SPI.h>
#include "MFRC522.h"
#include <RTClib.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <WebServiceComunication.h>
#include "TimeOut.h"
#include <Separator.h>
#include "ArduinoUniqueID.h"
//#include <Wire.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_5 = "ipWS"; 
const char* PARAM_INPUT_6 = "Select";
const char* PARAM_INPUT_7 = "ConectionPort";
const char* PARAM_INPUT_8 = "ConsultPort";
//******Definicion de variables WiFi*******
String ssid;
String password;
String ipWS;
String ip;
String gateway;
String mascSub;
String arrayWS, validatorSN;
bool miFareWifi;
String Value;
int indexQR;
int finQR;
String ConectionPort;
String ConsultPort;
String Card;
#define LastTimeAlive 0
#define lastTimeWiFiConnection 1
#define lastTimeServerConnection 2
#define LastTimeQRStart 3
#define lastTimetoReconnectWifi 4
#define LastTimeRelay 5
#define LastTimeMifare 6

int myPin = 16;
RTC_DS3231 rtc;
//****************Loop2**********************
TaskHandle_t WebService;

//**********Definicion de Instancias***********
SPG75 myQrreaderwork;
ESPWEB myWEBService;
Preferences preferences;
StatisticsValues EEPROMVariables;
buzzerled mySignal(4,6,5,7);
AsyncWebServer server(80);
WebServiceComunication myServerComunic;
Separator separator;
String DATA;
//****Dedinicion para Variables de contadores*****
int countQRValido, counQRIni, countQRInv, countErrWifi, counQRSoli, counWifiC, counErrServ, counErrServT, countQRValidoT,countHere = 0,countQR =0,https, counIni;
String DateTime;
int countTryQR;
String SN; // Variable para Numero serial de esp32
const void* SNS;
//******Definicion de RFCHIP******
#define RST_PIN         9        // Configurable, see typical pin layout above
#define SS_PIN          10
#define PICC_RX_SIZE      18
#define PICC_TX_SIZE      16

#define BLOCK_0 0x00 //Bloque 0 y accedes al bloque sumandole su mismo numero

#define KEYACCESS_SECTOR_0  0x03 //Sector0
#define KEYACCESS_SECTOR_1  0x07 //Sector1
#define KEYACCESS_SECTOR_2  0x0B //Sector2


MFRC522 RfChip(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

//******Definicion de variables para control de tiempo en ejecucion**********
unsigned long lastTimetoConfigureWifi;

int counCW, countPCW;
int timetoConfigureWifi =10000;
int lastWiFiStatus = 0;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int timeToReconnectWifi;

const long interval = 10000; 

int timeToAlive = 20000;
bool relayON = false;
int AP ;
//Definicion de variables para lector QR
String myQR = "";
String CadenaV, ServerResponse2,ServerResponse;
char data;
int countcad = 0;
int LongQR;
String QRAnsw = "";
String datos;
bool QRStatus;
bool QRsendComand = false;
int timeToQRStart = 5300;
int timeToRelay = 500;
//unsigned long LastTimeQRStart = 0;
int countAlive;

bool ModeConfigure = true;
bool alive = false;
bool reconnectNetwork= false;
bool counting;
IPAddress localIP;
bool MifareReaderAvailable = false;
bool accessPoint = false;
bool alivetrue = false;
bool QRActive = false;
bool WifiConnected = false;
bool reconnectWifi = false;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

//Varbiales de mifare
byte nuidPICC[4];
byte UniqueIDArduino[16];
TimeOut OkStatusLedTimeout;
TimeOut ErrorStatusLedTimeout;
TimeOut MifareActivateTimeout;
uint8_t tv[16] ={2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t ET[16] ={2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int TTE = 30;
byte TTE_MSB = 0;
byte TTE_LSB = 0;

byte EntryDate[16];
byte ReadDatas[16];

//*****************MQTT variables**************************
const char* MQTT_BROKER_ADRESS = "192.168.2.61";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_NAME = "ESP8266Client_1";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
String content = "";
bool MQTTRecive = false;
String toUpperCaseString(String input);
void WiFiConnection();
bool initWiFi();
void initSPIFFS();
void toggleCounting(boolean enable,int timerIndex);
void SelectQR(String QR, int LongQR);
void ServerConnection(String DATA);
void IDArduino();
String returnValidator(byte *buffer, byte bufferSize);
void printHex(byte *buffer, byte bufferSize);
void relayOn();
void relayOff();
String toUpperCaseString(String input);
void SuscribeMqtt();
void PublisMqtt(String data);

void relay();
int countReconect;
int Buzzer = 2;

struct Timer {
  unsigned long startTime;
  bool enabled;
};

Timer timers[10];

boolean elapsedTime (int timerIndex,unsigned long timeToCount)
{ 
  if (!timers[timerIndex].enabled) {
    // El temporizador no está habilitado, no hagas nada
    return false;
  }

  unsigned long countTime = 0;
  unsigned long actualMillis = millis();
  if (actualMillis < timers[timerIndex].startTime)
  {
    countTime = (actualMillis+(4294967295-timers[timerIndex].startTime));
  }
  else
  {
    countTime = actualMillis-timers[timerIndex].startTime;
  }
  if (countTime > timeToCount)
  {
    Serial.println(countTime);
    timers[timerIndex].startTime = actualMillis;
    return true;
  }
  else
    return false;
}

void dateTime(String QR)
{
  String qr = QR;
  String Date, Time;
  String Data;
  Date = (qr.substring(6,8)+"/"+(qr.substring(4,6))+"/"+ "20" +(qr.substring(2,4)));
  Serial.print("Date QR: ");
  Serial.println(Date);
  Serial.print("Date WS: ");
  Data = myServerComunic.Hour();
  Data = Data.substring(0,10);
  Serial.println(Data);
  if(Date == Data)
  {
    Serial.println("OPEN");
    //relay();
  }
}

void SelectQR(String QR, int LongQR)
{
  //Serial.println(QR);
  String* _QR;
  int i,j;
    String StartQR = QR.substring(0,4);
    Serial.println(LongQR);
    Serial.println(QR);
    //if (LongQR==93)
    if(StartQR=="http" || LongQR==50)
    {
        String qr;
        
        //qr= QR.substring(43,92); 
        if(StartQR=="http")
        {
          int l = LongQR;
          indexQR =  QR.indexOf("=");
          qr = QR.substring(indexQR+1,l-1);
          Serial.println(qr);
          Serial.println(qr.length());
          if (WifiConnected == true)
          {
            //ServerConnection(qr);
            PublisMqtt(qr);
            
          }
          else
          {
            dateTime(qr);
          }
          
        }
        if (LongQR==50)
        {
          String qr = QR.substring(0,49);
          if(StartQR=="WIFI")
          {
            mySignal.flashLed(3,YELLOW,100,100,true);
            _QR= myQrreaderwork.SeparatorIndex(QR,";");
            for ( int i = 0; i < 9; i++ ) 
            {
              myQrreaderwork.VariableConfigure(_QR,j);
              j=i;
            }
            WiFiConnection();
          }
          else
          {
            if (WifiConnected == true)
            {
              //ServerConnection(qr);
              PublisMqtt(qr);  
            }
            else
            {
              dateTime(qr);
            }
          }
        }
    }
    else
    {
        if(StartQR=="WIFI")
        {
          mySignal.flashLed(3,YELLOW,100,100,true);
          _QR= myQrreaderwork.SeparatorIndex(QR,";");
          for ( int i = 0; i < 9; i++ ) 
          {
            myQrreaderwork.VariableConfigure(_QR,j);
            j=i;
          }
          WiFiConnection();
        }
        else
        {
            mySignal.flashLed(3,RED,100,100,true);
            Serial.println("false SelectQR");
            Serial.println(myQR);
        }
    }
}

void PublisMqtt(String data)
{
  mySignal.ledOFF();
  String payload = myServerComunic.SerializeObject(false,validatorSN,data,"192");
  mqttClient.publish("readSensor", (char*)payload.c_str());
  
}

void ConnectMqtt()
{
  while (!mqttClient.connected())
  {
    Serial.print("Starting MQTT connection...");
    if (mqttClient.connect(MQTT_CLIENT_NAME))
    {
      SuscribeMqtt();
    }
    else
    {
      Serial.print("Failed MQTT connection, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void HandleMqtt()
{
  if (!mqttClient.connected())
  {
    ConnectMqtt();
  }
  mqttClient.loop();
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Received on ");
  Serial.print(topic);
  Serial.print(": ");

  content = "";  
  for (size_t i = 0; i < length; i++) {
    content.concat((char)payload[i]);
  }
  Serial.print(content);
  Serial.println();

  if (content=="Activate")
  {

    MQTTRecive = true;

  }
  if (content=="Desactivate")
  {
    relayOff();
    //MQTTRecive = false;
    //MifareReaderAvailable = false;
    mySignal.ledOFF();
  }
  
}

void SuscribeMqtt()
{
  mqttClient.subscribe("activeSensor");
  Serial.println("MQTT suscribe activeSensor");
  mySignal.flashLed(3,BLUE,200,100,false);
}

void InitMqtt() 
{
  mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
  SuscribeMqtt();
  mqttClient.setCallback(callback);
}

void ServerConnection(String DATA)
{
  String qr = DATA;
  if(myServerComunic.HttpClientRequest(false,DATA,true,ipWS,ip,validatorSN,https,ConectionPort,ConsultPort))
  {
    toggleCounting(true,LastTimeAlive);
    Serial.println(myServerComunic.getResponseQR());
    if (myServerComunic.getResponseQR()==1 || myServerComunic.getResponseQR()==2)
    {
      countQRValido ++;
      countQRValidoT ++;
      EEPROMVariables.SaveVariables(QRValid,countQRValido);
      EEPROMVariables.SaveVariables(TotalValidQR, countQRValidoT);
      mySignal.flashLed(2,GREEN,150,25,true);
      if (QRActive==false)
      {
        // miFareReaderWriter.MifareReaderAvailable=true;
      }
      //relay();
    }
    else
    {
      countQRInv ++;
      Serial.println(countQRInv);
      EEPROMVariables.SaveVariables(QRInvalid,countQRInv);
      mySignal.flashLed(2,RED,500,150,true);
       if (QRActive==false)
      {
        // miFareReaderWriter.MifareReaderAvailable=true;
      }
    }  
  }
  else
  {
    mySignal.flashLed(2,RED,500,150,true);
    counErrServ++;
    counErrServT++; 
    EEPROMVariables.SaveVariables(ServerFailures,counErrServ);
    EEPROMVariables.SaveVariables(TotalFailuresServer,counErrServT);
    Serial.println("No conecto alWeb Service");
    
  }
  if(QRStatus==1)
  {
    QRsendComand = true;
  }
}

void initSPIFFS() 
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void AccessPoint ()
{
  toggleCounting(true,lastTimeWiFiConnection);
  //lastTimeWiFiConnection = millis();
  // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", "!sp123456");

    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 
    
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost())
        {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            if (ssid!="")
            {
            preferences.begin("Wifi", false);
            preferences.putString("ssid",ssid);
            preferences.end();
            }
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            
            password = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(password);
            // Write file to save value
            if (password!="")
            {
            preferences.begin("Wifi", false);
            preferences.putString("password",password);
            preferences.end();
            }
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            if (ip!="")
            {
            preferences.begin("Wifi",false);
            preferences.putString("ip", ip);
            preferences.end();
            }
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            if (gateway!="")
            {
            preferences.begin("Wifi",false);
            preferences.putString("gateway", gateway);
            preferences.end();
            }
          }
          if (p->name() == PARAM_INPUT_5)
          {
            ipWS = p->value().c_str();
            Serial.print("ipWSH set to: ");
            Serial.println(ipWS);
            // Write file to save value
            if (ipWS!="")
            {
            preferences.begin("Wifi",false);
            preferences.putString("ipWS", ipWS);
            preferences.end();
            }
          }
          if (p->name() == PARAM_INPUT_6)
          {
            Value = p->value().c_str();
            Serial.print("miFare Process set to: ");
            Serial.println(Value);
            if (Value == "value1")
              {
                miFareWifi = true;
              }
            else if (Value == "value2")
              {
                miFareWifi = false;
              }
             
            if (Value!="" || Value != "value0")
            {
              preferences.begin("Wifi",false);
              preferences.putBool("miFareWifi", miFareWifi);
              preferences.end();
            }
          }
          if (p->name() == PARAM_INPUT_7)
          {
            ConectionPort = p->value().c_str();
            Serial.print("Puerto de conexion set to: ");
            Serial.println(ConectionPort);
            // Write file to save value
            if (ConectionPort!="")
            {
            preferences.begin("Wifi",false);
            preferences.putString("ConPort", ConectionPort);
            preferences.end();
            }
          }
           if (p->name() == PARAM_INPUT_8)
          {
            ConsultPort = p->value().c_str();
            Serial.print("Puerto de consulta set to: ");
            Serial.println(ConsultPort);
            // Write file to save value
            if (ConsultPort!="")
            {
            preferences.begin("Wifi",false);
            preferences.putString("ConsPort", ConsultPort);
            preferences.end();
            }
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
      // if(ipWS!="")
      // {
      //   alive = true;
      // }
      // else{
      //   ESP.restart();
      // }
    });
    server.begin(); 
}

void getCountVariables()
{
  preferences.begin("Wifi", true);
  countQRValido = preferences.getInt("countQRValido",0);
  countQRInv = preferences.getInt("countQRInv",0);
  countErrWifi = preferences.getInt("countErrWifi",0);
  counQRIni = preferences.getInt("counQRIni",0);
  counQRSoli = preferences.getInt("counQRSoli",0);
  counWifiC = preferences.getInt("counWifiC",0);
  counErrServ = preferences.getInt("counErrServ", 0);
  counErrServT = preferences.getInt("counErrServT", 0);
  countQRValidoT = preferences.getInt("countQRValidoT",0);
  miFareWifi=preferences.getBool("miFareWifi",false);
  ConectionPort = preferences.getString("ConPort","");
  ConsultPort = preferences.getString("ConsPort","");
  DateTime = preferences.getString("datetime", "");
  https = preferences.getInt("Https", 0);
  preferences.end();
}

void setup() 
{
  preferences.begin("Wifi", true);
  counCW = preferences.getInt("counCW", 0);
  counQRIni = preferences.getInt("counQRIni",0);
  preferences.end();
  if (counQRIni==0){
    counCW = 0;
  }
  if(counCW > timetoConfigureWifi)
  {
    counCW = 0;
    counQRIni = 0;
    EEPROMVariables.SaveVariables(ESPBeginnings,counQRIni);
  }
  countReconect=0;
  Serial.begin (9600);
  mySignal.buzzerOFF();
  myQrreaderwork.StartBaudRate(18,17,9600); //Inicializacion QR
  myWEBService.port(); //Inicializacion de puerto Serial de Web service esp32
  SPI.begin(); // Init SPI bus
 // myServerComunic.hourBegin();
  //RfChip.PCD_Init();  
  Serial.println("**********ESP32 INIT***********");
  getCountVariables();
  pinMode(myPin, OUTPUT);
  digitalWrite(myPin, LOW); 
  //rtc.setTime();
  counQRIni ++;
  Serial.println(counQRIni);
  EEPROMVariables.SaveVariables(ESPBeginnings,counQRIni);
  mySignal.flashLed(2,WHITE,100,100,false);
  lastTimetoConfigureWifi = millis();
  countPCW = lastTimetoConfigureWifi + counCW;
  preferences.begin("Wifi",false);
  preferences.putInt("counCW",countPCW);
  preferences.end();
  timers[0].enabled=false; //lastTimeServerConnection;
  timers[1].enabled=false; //lastTimeWiFiConnection;
  timers[2].enabled=false; //LastTimeAlive = 0;
  timers[3].enabled = false;

 // toggleCounting(true,LastTimeQRStart);**************************************************************************QR START
  memset(UniqueIDArduino, 0 , 16); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<//UNIQUE ID ARDUINO
  IDArduino();
  toggleCounting(true,LastTimeQRStart);
  memset(UniqueIDArduino, 0 , 16); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   //UNIQUE ID ARDUINO
  // miFareReaderWriter.IDArduino();
  Serial.print(F("ID ESP32::"));
  // miFareReaderWriter.printHex(UniqueIDArduino, 16);
  Serial.println("");
  Serial.println(miFareWifi);
  // validatorSN = miFareReaderWriter.returnValidator(UniqueIDArduino, 16);
   if ((counQRIni == 4) && (countPCW <= timetoConfigureWifi))
  {
    Serial.println("modo de configuracion");
    counQRIni = 0;
    preferences.begin("Wifi",false);
    preferences.putInt("counQRIni", counQRIni);
    preferences.end();
    initSPIFFS();
    accessPoint = true;
    toggleCounting(false,LastTimeAlive);
    toggleCounting(false,LastTimeQRStart);
    AP=0;
    if(initWiFi())
    {
      Serial.println("conectado a la red ");
      AccessPoint();
      mySignal.ledON(YELLOW);
      //alive = true;
    }
    else
    {
      Serial.println("no conectado a la red");
      AccessPoint();
      mySignal.ledON(YELLOW);
    }
  }
  else
  {
    if (miFareWifi==true)
    {
      WiFiConnection();
    }
  }
}

void ipFijaConfig(String _ipFija, String _mascSub ,String _gateway)
{
  String* iparray;
  String* garray;
  String* mascSub;

  Serial.print("gateway:");
  Serial.println(_gateway);
  Serial.print("mascSub:");
  Serial.println(_mascSub);

  if (_ipFija == "" || _gateway == "" || _mascSub == "")
  {
    Serial.println(WiFi.localIP()); // en caso de no tener vaiables guardadas, asigna una IPlocal
  }
  else
  {
    iparray = myQrreaderwork.SeparatorIndex(_ipFija,".");
    for ( int i = 0; i < 4; i++ ) 
      {
        iparray[i];
      }
    IPAddress ip(iparray[0].toInt(),iparray[1].toInt(),iparray[2].toInt(),iparray[3].toInt());

    mascSub = myQrreaderwork.SeparatorIndex(_mascSub,".");
    for ( int k = 0; k < 4; k++ ) 
      {
        mascSub[k];
      }
    IPAddress subnet(mascSub[0].toInt(),mascSub[1].toInt(),mascSub[2].toInt(),mascSub[3].toInt());

    garray = myQrreaderwork.SeparatorIndex(_gateway,".");
    for ( int a = 0; a < 4; a++ ) 
      {
        garray[a];
      }
    IPAddress gateway(garray[0].toInt(),garray[1].toInt(),garray[2].toInt(),garray[3].toInt());

    WiFi.config(ip, gateway, subnet); 
  }
}

bool initWiFi() 
{
  if(ssid=="" || password==""|| ipWS== "" || ip ==""){
    Serial.println("Undefined SSID or IP address.");
    reconnectNetwork = false;
    return false;
  }

  Serial.println("AP");
  Serial.println(AP);

  switch (AP)
  {
    case 0:
      Serial.println("case 0");
      toggleCounting(true,lastTimeWiFiConnection);
      //lastTimeWiFiConnection = millis();
      reconnectNetwork = true;
      WiFi.mode(WIFI_STA);
      localIP.fromString(ip.c_str());
      localGateway.fromString(gateway.c_str());


      if (!WiFi.config(localIP, localGateway, subnet)){
        Serial.println("STA Failed to configure");
        return false;
      }
      WiFi.begin(ssid.c_str(), password.c_str());
      Serial.println("Connecting to WiFi...");
      
      currentMillis = millis();
      previousMillis = currentMillis;

      while(WiFi.status() != WL_CONNECTED) {
        currentMillis = millis();
        mySignal.delayLed (1000, PINK, 250,250,false);Serial.print('.');
        if (currentMillis - previousMillis >= interval) {
          Serial.println("Failed to connect.");
          return false;
        }
      }

      Serial.println(WiFi.localIP());
      return true;
    break;

    case 1:
      Serial.println("case 1");
      return true;
      //reconnectNetwork = false;
      //AccessPoint();
    break;
  
   default:
   return false;
    break;
  }
}

void WiFiConnection() 
{
  Serial.println("**STEP 1**");
  Serial.print("Serial Number: ");
  Serial.println(validatorSN);
  toggleCounting(true,lastTimeWiFiConnection);
  //lastTimeWiFiConnection = millis();
  preferences.begin("Wifi", true);
  password = preferences.getString("password", "");
  ssid = preferences.getString("ssid", "");
  ipWS = preferences.getString("ipWS", "");
  //ipWS = "192.168";
  gateway = preferences.getString("gateway", "");
  //mascSub = preferences.getString("mascSub", "");
  Serial.print("IP Web Service: ");
  Serial.println(ipWS);
  mascSub = "255.255.255.0";
  https = preferences.getInt("Https", 0);
  ip = preferences.getString("ip","");
  preferences.end();
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());
  Serial.print("Conecting to SSID:");
  Serial.println(ssid);
  Serial.print("Password:");
  Serial.println(password);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi ..");
  
  //analogWrite(pin_buzzer, 0); // apagamos buzzer
  while ((WiFi.status() != WL_CONNECTED) && !elapsedTime (lastTimeWiFiConnection,10000))
  {
    Serial.println("**STEP 2**");
    //analogWrite(pin_buzzer, 0); // apagamos buzzer
    Serial.print('.');
    mySignal.delayLed (1000, PINK, 250,250,false);
    if(myQrreaderwork.ReadQR()>0)
    {
      myQR = myQrreaderwork.getQR();
      LongQR = myQR.length();
      //SelectQR(myQR,LongQR);**************************************************************************************
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("**STEP 3**");
    //mySignal.ledON(BLUE);
    Serial.println("\nNetwork WiFi connected");
    ipFijaConfig(ip,mascSub,gateway);
    Serial.print("IP Modulo ESP32: ");
    Serial.println(WiFi.localIP());
    myWEBService.Sbegin();
    Serial.print("Channel: ");
    Serial.println(WiFi.channel());
    Serial.print("MAC Address:  ");
    Serial.println(WiFi.macAddress());
    //ServerConnection();*******************************************************************************************
    lastWiFiStatus = WiFi.status();
    alive = true;
    WifiConnected = true;
    InitMqtt();
  }
  else
  {
    Serial.println("\nNetwork WiFi unable to connect");
    lastWiFiStatus = WiFi.status();
    alivetrue = true;
    toggleCounting(true,lastTimetoReconnectWifi);
  }
}

String returnArrayWS(byte *buffer, byte bufferSize, String validatorSN, String card){
  String dateTime, Day, Month, Year, Hour, Min, Sec, ME;
  String Provedor = "01";
  String tarjeta = card;
  String paridad = "0000";
  String Tienda ="1234";
  String returnarrayWS = "";
  String index = " ";
  String* DateTime;
  for(byte i = 0; i < bufferSize; i++){
    dateTime += (buffer[i] < 10 ? " 0" : " ");
    dateTime += String(buffer[i], DEC);
  }
  DateTime = separator.SeparatorIndex(dateTime,index);
  for ( int h = 0; h < 8; h++ ) 
      {
        DateTime[h];
      }
  Day = DateTime[1];
  Month = DateTime[2];
  Year = DateTime[3];
  Hour = DateTime[4];
  Min = DateTime[5];
  Sec = DateTime[6];
  ME = DateTime[7];
  if (ME.length()<3)
  {
    for(int i = ME.length(); i<=2;i++)
    ME= "0"+ME;

  }
  Serial.print("ME: ");
  Serial.println(ME);
  Serial.print("tarjeta: ");
  Serial.println(tarjeta);
  returnarrayWS = Provedor + Year + Month + Day + Hour + Min + Sec + Tienda + ME + tarjeta + validatorSN + paridad;
  return returnarrayWS;
}

String returnValidator(byte *buffer, byte bufferSize){
  String validator;
  String Validator;
  for (byte i = 0; i < bufferSize; i++) {
    validator += (buffer[i] < 0x10 ? "0" : "");
    validator += String(buffer[i], HEX);
  }
  validator = toUpperCaseString(validator);
  Validator = (validator.substring(0,8))+ (validator.substring(16,24));
  return Validator;
}

void MifareActivateTimeoutHandler(void)
{
  Serial.println("MifareActivateTimeout");
  //MifareActivateTimeout.detach();
  MifareReaderAvailable = true;
  mySignal.ledON(BLUE); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
 // ReadingStatusLed.write(LED_ON);
}

void OkStatusLedTimeoutHandler(void)
{
  Serial.println("OkStatusLedTimeoutHandler");
  analogWrite(Buzzer, 0);
  //OkStatusLedTimeout.detach();
  //OkStatusLed.write(LED_OFF);
  mySignal.buzzerOFF(); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
  //MifareActivateTimeout.attach(&MifareActivateTimeoutHandler, TIME_SLEEP_MIFARE);
  MifareActivateTimeout.timeOut(400, MifareActivateTimeoutHandler); //delay 10000=10seg, callback function  
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

String returnCard(byte *buffer, byte bufferSize)
{
  String card;
  String* Card;
  String ArrayCard;
  String index = " ";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
        card += "0";
    }
    // Convertimos el byte a su representación hexadecimal en mayúsculas y lo añadimos a 'card'
    card += String(buffer[i], HEX);
    // Añadimos un espacio entre los bytes, excepto después del último byte
    if (i < bufferSize - 1) {
        card += " ";
    }
  }
  card = toUpperCaseString(card);
  Card = separator.SeparatorIndex(card,index);
  for ( int h = 0; h < 8; h++ ) 
      {
        Card[h];
      }
  ArrayCard = Card[3] + Card[2] + Card[1] + Card[0];
  if(ArrayCard.length()>8)
  {
    ArrayCard = Card[2] + Card[1] + Card[0];
  }
  return ArrayCard;
}

String toUpperCaseString(String input) {
  String result = "";
  for (int i = 0; i < input.length(); i++) {
    result += char(toupper(input.charAt(i)));
  }
  return result;
}

void IDArduino()
{
  memcpy(UniqueIDArduino, UniqueID8, 4);//copia los primeros 4 en UniqueIDArduino
  memcpy(UniqueIDArduino+8, UniqueID8+4, 4);//UniqueID8+4 recorre 4 y copia los 2dos 4 a UniqueIDArduino+8 a partir de la posicion 8
 }

void ErrorStatusLedTimeoutHandler(void)
{
 Serial.println("ErrorStatusLedTimeout"); 
  mySignal.buzzerOFF(); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
  MifareActivateTimeout.timeOut(400, MifareActivateTimeoutHandler); //delay 10000=10seg, callback function  
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

bool MifareReadProcess(byte SectorAccess,byte BlockAccess)
{
  if(!RfChip.PICC_IsNewCardPresent())
  {    
    return false;
  } 
  if (!RfChip.PICC_ReadCardSerial())
  {
    return false;
  }
  //Debug.printf("PICC Card UID: "); 
  for (byte i = 0; i < 4; i++) 
  {
    Serial.println(" Card present");
    nuidPICC[i] = RfChip.uid.uidByte[i];
  }

  Serial.print(F("UID::"));
   printHex(RfChip.uid.uidByte, RfChip.uid.size);
  Card = returnCard(RfChip.uid.uidByte, RfChip.uid.size);
  //printDec(RfChip.uid.uidByte, RfChip.uid.size);
  Serial.println();
  Serial.println("Card: ");
  Serial.print(Card);
  Serial.println();
  Serial.println(" Card present");
  //Card type
  uint8_t piccType = RfChip.PICC_GetType(RfChip.uid.sak);
 // Serial.println(RfChip.PICC_GetTypeName(piccType));

  for(int mainCounter = 0; mainCounter < MFRC522::MF_KEY_SIZE; mainCounter++)
      key.keyByte[mainCounter] = 0xFF;   
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  //Serial.println(); 

   MFRC522::StatusCode status;
  //Authenticate using key A
  //Serial.println("Authenticating using key A...");
  MFRC522::PICC_Command piccAuthKey = MFRC522::PICC_CMD_MF_AUTH_KEY_A;
  uint8_t piccStatus = RfChip.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, SectorAccess, &key, &RfChip.uid);
  //status = (MFRC522::StatusCode) RfChip.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, PICC_TRAILER_BLOCK_1, &key, &(RfChip.uid));
  if (piccStatus != MFRC522::STATUS_OK) 
  {
      Serial.println("Authenticate key A::failed: ");
      Serial.println("");
      RfChip.PCD_StopCrypto1();
      RfChip.PICC_HaltA();
      if(!RfChip.PICC_IsNewCardPresent())
        {
          //Debug.printf("NO Card Detected\r\n");
          return false;
        }
      if (!RfChip.PICC_ReadCardSerial())
        {
           //Debug.printf("Cannot read serial number\r\n");
          return false;
        }
        
      //Debug.printf("PICC Card UID: ");
      for (byte i = 0; i < 4; i++) {
          nuidPICC[i] = RfChip.uid.uidByte[i];
        }

      piccType = RfChip.PICC_GetType(RfChip.uid.sak);

      for(int mainCounter = 0; mainCounter < MFRC522::MF_KEY_SIZE; mainCounter++)
          key.keyByte[mainCounter] = 0xFF;
      
      piccStatus = RfChip.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, SectorAccess, &key, &RfChip.uid);

      if(piccStatus != MFRC522::STATUS_OK)
      {
        Serial.println("Authenticate key B::failed: ");
        Serial.println("");
        RfChip.PCD_StopCrypto1();
        RfChip.PICC_HaltA();
        mySignal.ledON(RED);
        //setColor(0, 255, 255); //RGB= RedErrorStatusLed/GreenReadingStatusLed/BlueOkStatusLed
        MifareReaderAvailable = false;
        ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
        return false;
      }
      //Serial.println("Authenticating using key B...");
      piccAuthKey = MFRC522::PICC_CMD_MF_AUTH_KEY_B;
  }
  else
  //Serial.println("Authentification type -> A");

  memset(UniqueIDArduino, 0 , 16); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   //UNIQUE ID ARDUINO
  IDArduino();
   
  uint8_t piccReadBuffer[PICC_RX_SIZE];
  uint8_t piccReadBufferSize = sizeof(piccReadBuffer);
  piccStatus = RfChip.PCD_Authenticate(piccAuthKey, SectorAccess, &key, &RfChip.uid);
  if(piccStatus != MFRC522::STATUS_OK)
  {
    //Debug.printf("ERROR: Authentification block[%d] -> %s\r\n\r\n", PICC_TRAILER_BLOCK_1, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA();
    mySignal.ledON(RED); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    return false;
  }
  piccStatus = RfChip.MIFARE_Read(BlockAccess, piccReadBuffer, &piccReadBufferSize);
  if(piccStatus != MFRC522::STATUS_OK)
  {
    //Debug.printf("ERROR: Reading Block [%d] -> %s\r\n\r\n", PICC_LPC_UID_BLOCK, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA();
    mySignal.ledON(RED);
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    //ErrorStatusLedTimeout.attach(&ErrorStatusLedTimeoutHandler, TIME_SLEEP_MIFARE);
    return false;
  }  

  //Serial.print(F("ReadDataS1B0::"));
  //printHex(piccReadBuffer, 16);
  //Serial.println("");       
  
  //Serial.print(TTE_MSB, HEX);
  //Serial.print(" ");  
  //Serial.println(TTE_LSB, HEX);      
  //Serial.println(""); 
  //int TolEntryCard = (( TTE_MSB << 7) |  TTE_LSB);
  //Serial.println(TolEntryCard, HEX);   
  
  TTE_MSB = ((TTE >> 8) & 0xFF);
  TTE_LSB = (TTE & 0xFF);
  memset(EntryDate, 0 , 16);
  memcpy(EntryDate, piccReadBuffer, 16);
         
  EntryDate[7]=TTE_MSB;
  EntryDate[8]=TTE_LSB;

  piccStatus = RfChip.MIFARE_Write(BlockAccess, (uint8_t *)&EntryDate[0], 16);
  if(piccStatus != MFRC522::STATUS_OK)
  {
    //Debug.printf("ERROR: Writing Block [%d] -> %s\r\n\r\n", PICC_LPC_UID_BLOCK, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA(); 
    mySignal.ledON(RED);
    //setColor(0, 255, 255); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    return false;
  }

  Serial.print(F("EntryDate&TTE:"));
  printHex(EntryDate, 16);
  Serial.println("");
 
  /*ACCESO SECTOR 2*/

  //Serial.print(F("S2B0"));
  //KEYACCESS_SECTOR_1,BLOCK_0 + 4

  SectorAccess = KEYACCESS_SECTOR_2;  
  BlockAccess = BLOCK_0 + 8;

  //MFRC522::StatusCode status;
 
  //Authenticate using key A
  //Serial.println("Authenticating using key A...");
  //MFRC522::PICC_Command piccAuthKey = MFRC522::PICC_CMD_MF_AUTH_KEY_A;
  
   piccStatus = RfChip.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, SectorAccess, &key, &RfChip.uid);
  if (piccStatus != MFRC522::STATUS_OK) {
      Serial.println("Authenticate key A::failed: ");
      Serial.println("");
      RfChip.PCD_StopCrypto1();
      RfChip.PICC_HaltA();
      if(!RfChip.PICC_IsNewCardPresent())
        {
          //Debug.printf("NO Card Detected\r\n");
          return false;
        }
      if (!RfChip.PICC_ReadCardSerial())
        {
           //Debug.printf("Cannot read serial number\r\n");
          return false;
        }
        
      //Debug.printf("PICC Card UID: ");
      for (byte i = 0; i < 4; i++) {
          nuidPICC[i] = RfChip.uid.uidByte[i];
        }

      piccType = RfChip.PICC_GetType(RfChip.uid.sak);

      for(int mainCounter = 0; mainCounter < MFRC522::MF_KEY_SIZE; mainCounter++)
          key.keyByte[mainCounter] = 0xFF;
      
      piccStatus = RfChip.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, SectorAccess, &key, &RfChip.uid);
      if(piccStatus != MFRC522::STATUS_OK)
      {
        Serial.println("Authenticate key B::failed: ");
        Serial.println("");
        RfChip.PCD_StopCrypto1();
        RfChip.PICC_HaltA();
        mySignal.ledON(RED);
        //setColor(0, 255, 255); //RGB= RedErrorStatusLed/GreenReadingStatusLed/BlueOkStatusLed
        MifareReaderAvailable = false;
        ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
        return false;
      }
      //Serial.println("Authenticating using key B...");
      piccAuthKey = MFRC522::PICC_CMD_MF_AUTH_KEY_B;
  }
  else
  
  //Serial.println("Authentification type -> A");

  memset(UniqueIDArduino, 0 , 16); // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   //UNIQUE ID ARDUINO
  IDArduino();
  
  // Serial.print(F("ID Arduino::"));
  // printHex(UniqueIDArduino, 16);
  // Serial.println("");
  //  printDec(UniqueIDArduino,16);
  // Serial.println("");
   
  //uint8_t piccReadBuffer[PICC_RX_SIZE];
  //uint8_t piccReadBufferSize = sizeof(piccReadBuffer);
  piccStatus = RfChip.PCD_Authenticate(piccAuthKey, SectorAccess, &key, &RfChip.uid);
  if(piccStatus != MFRC522::STATUS_OK)
  {
    //Debug.printf("ERROR: Authentification block[%d] -> %s\r\n\r\n", PICC_TRAILER_BLOCK_1, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA();
    mySignal.ledON(RED);
    //setColor(0, 255, 255); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    return false;
  }
  
    piccStatus = RfChip.MIFARE_Write(BlockAccess,(uint8_t *)&UniqueIDArduino[0], 16);
    //Serial.println(SN);
    //piccStatus = RfChip.MIFARE_Write(BlockAccess,(uint8_t *)&SN[0], 16);
  if(piccStatus != MFRC522::STATUS_OK)
  {
     Serial.println("stop2");
    //Debug.printf("ERROR: Writing Block [%d] -> %s\r\n\r\n", PICC_LPC_UID_BLOCK, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA(); 
    mySignal.ledON(RED);
    //setColor(0, 255, 255); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    return false;
  }
  Serial.println("c");
  piccStatus = RfChip.MIFARE_Write(BlockAccess + 1, tv, 16);
  if(piccStatus != MFRC522::STATUS_OK)
  {
 //   Debug.printf("ERROR: Writing Block [%d] -> %s\r\n\r\n", PICC_LPC_UID_BLOCK + 1, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA();
    mySignal.ledON(RED); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    //ErrorStatusLedTimeout.attach(&ErrorStatusLedTimeoutHandler, TIME_SLEEP_MIFARE);
    return false;
  }

  piccStatus = RfChip.MIFARE_Read(BlockAccess, piccReadBuffer, &piccReadBufferSize);
  if(piccStatus != MFRC522::STATUS_OK)
  {
    //Debug.printf("ERROR: Reading Block [%d] -> %s\r\n\r\n", PICC_LPC_UID_BLOCK, RfChip.GetStatusCodeName(piccStatus));
    RfChip.PCD_StopCrypto1();
    RfChip.PICC_HaltA();
    mySignal.ledON(RED);
    //setColor(0, 255, 255); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
    MifareReaderAvailable = false;
    ErrorStatusLedTimeout.timeOut(500, ErrorStatusLedTimeoutHandler); //delay 10000=10seg, callback function
    //ErrorStatusLedTimeout.attach(&ErrorStatusLedTimeoutHandler, TIME_SLEEP_MIFARE);
    return false;
  }  

  Serial.print(F("Validator::"));
  //printHex(piccReadBuffer, 16);
  //Serial.println("");
  validatorSN = returnValidator(piccReadBuffer, 16);
  Serial.println(validatorSN);
  //Web Service Comunication
// if (myServerCom.HttpClientRequest(false,arrayWS,false,ipWS,ipFija,validatorSN,0)){
  arrayWS = returnArrayWS(EntryDate, 16, validatorSN,Card);
  Serial.print("ArrayWS: ");
  Serial.println(arrayWS);
//   }
/*End Acces*/
  RfChip.PCD_StopCrypto1();
  RfChip.PICC_HaltA();
  mySignal.ledON(GREEN);
  //setColor(255, 255, 0); //RGB= RedErrorStatusLed/BlueOkStatusLed/GreenReadingStatusLed
  analogWrite(Buzzer, 255);
  delay(300);
  //Beep_Buzzer(2058,300, 2);
  //MifareReaderAvailable = false;
  mySignal.flashLed(1,GREEN,500,0,true);
  //toggleCounting(true,LastTimeMifare);
  //OkStatusLedTimeout.timeOut(400, OkStatusLedTimeoutHandler); //delay 10000=10seg, callback function
  //MifareActivateTimeoutHandler();
  Serial.println("");  
  return true;  
}

void loop() 
{
  HandleMqtt();
  if(MQTTRecive == true)
  {
    if(relayON==true)
    counQRIni = 0;
    EEPROMVariables.SaveVariables(ESPBeginnings,counQRIni);
  } 
  if (elapsedTime(LastTimeQRStart, timeToQRStart)  || QRsendComand == true)
      {
        QRsendComand = false;
        toggleCounting(true,LastTimeQRStart);
        QRStatus = myQrreaderwork.StartQR();
        if (QRStatus == 0)
        {
          QRsendComand = true;
          countTryQR++;
          delay(500);
          QRsendComand = true;
           if (countTryQR ==5 && QRActive == false){
            Serial.println("No hay QR conectado");
            toggleCounting(false,LastTimeQRStart);
            QRsendComand = false;
            // miFareReaderWriter.MifareReaderAvailable = true;
            toggleCounting(true,LastTimeAlive);
          }
        }
        else
        {
          //miFareWifi=true;
          //toggleCounting(true,LastTimeAlive);
          //WifiConnected = true;
          // miFareReaderWriter.MifareReaderAvailable = true;
          QRActive = true;
          if (alivetrue ==true)
          {
            mySignal.ledON(CIAN);
          }
          else 
          {
            mySignal.ledON(RED);
          }   
        }
      }
  if (MifareReaderAvailable)
    {
      relayOff();
      delay(100);
    }
    mqttClient.publish("readSensor", (char*)validatorSN.c_str());
    MQTTRecive = false;
    relayOn();
    mySignal.ledON(PINK);
    //QRsendComand = true;
    Serial.println("Activo Relay");
  
  if (myQrreaderwork.ReadQR()>0)
  {
        //mySignal.ledON(BLUE);
        myQR = myQrreaderwork.getQR();
        LongQR = myQR.length();
        counQRSoli++;
        EEPROMVariables.SaveVariables(QRReadings,counQRSoli);
        SelectQR(myQR,LongQR);
  }
  if ((elapsedTime(LastTimeAlive,timeToAlive) || alive == true)&& WiFi.status() == WL_CONNECTED && miFareWifi==true)
    {
      alive = false;
      Serial.println("Alive");
      if(myServerComunic.HttpClientRequest(true,arrayWS,false,ipWS,ip,validatorSN,0,ConectionPort,ConsultPort))
             {
                //mySignal.ledOFF();
                toggleCounting(true,LastTimeAlive);
                Serial.print("DateTime: ");
                Serial.println(myServerComunic.Hour());
                alivetrue = true;
                miFareWifi = true;
                if (QRActive==false)
                {
                  MifareReaderAvailable = true;
                }
                else
                {
                  MifareReaderAvailable = true;
                }
                //if (! rtc.begin()) {
                //Serial.println("No hay un módulo RTC");
                //while (1);
              //}
                //mySignal.ledON(BLUE);
              }
              else
              {
                mySignal.ledOFF();
                mySignal.ledON(RED);
                Serial.println("error server");
                countAlive ++;
                Serial.println(countAlive);
                AP=1;
                alive = true;
                alivetrue = false;
                if(elapsedTime(lastTimeServerConnection,30000))
                {
                  mySignal.ledOFF();
                  mySignal.ledON(RED);
                  toggleCounting(true,LastTimeAlive);
                  alive = false;
                  AccessPoint();
                }
              }
    }
  if (elapsedTime(LastTimeRelay,timeToRelay))
    {
      relayOff();
      toggleCounting(false,LastTimeRelay);
    }
 TimeOut::handler();  
}

void relayOn()
{
  toggleCounting(true, LastTimeRelay);
  digitalWrite(myPin, HIGH); 
  Serial.println("high");
  relayON= true;       
}

void relayOff()
{
   digitalWrite(myPin, LOW); 
   //toggleCounting(false,LastTimeRelay);
   Serial.println("low");
   relayON=false;
}

void toggleCounting(boolean enable,int timerIndex) 
{
  timers[timerIndex].enabled = enable;
  if (enable) {
    timers[timerIndex].startTime = millis(); // Iniciar el temporizador
  }
}