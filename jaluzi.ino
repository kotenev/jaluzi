#include <ESP8266WiFi.h>        //Содержится в пакете
#include <ESP8266WebServer.h>   //Содержится в пакете
#include <ESP8266SSDP.h>        //Содержится в пакете
#include <FS.h>                 //Содержится в пакете
#include <time.h>               //Содержится в пакете
#include <Servo.h>              //Содержится в пакете
#include <Ticker.h>             //Содержится в пакете
#include <WiFiUdp.h>            //Содержится в пакете
#include <ESP8266HTTPUpdateServer.h> //Содержится в пакете
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>          //Содержится в пакете
#include <ArduinoJson.h>

// Настройки DNS сервера и адреса точки в режиме AP
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
// Web интерфейс для устройства
ESP8266WebServer HTTP(80);
//ESP8266WebServer HTTPWAN(ddnsPort);
ESP8266WebServer *HTTPWAN;
ESP8266HTTPUpdateServer httpUpdater;
// Для файловой системы
File fsUploadFile;
// Для сервопривода
Servo myservo;
// Для тикера
Ticker tickerSetLow;
Ticker tickerAlert;

#define Tach0 0           // Кнопка управления
#define servo_pin 2       // Сервопривод на ноге
#define turnSensor_pin 14 // Сенсор вращения

// Определяем строку для json config
String jsonConfig = "";

// Определяем переменные
String module[]={"jalousie-motor"};
//,"sonoff","rbg"};

String ssidName     = "WiFi";     // Для хранения SSID
String ssidPass = "Pass";     // Для хранения пароля сети
String ssidApName = "Zaluzi03";   // SSID AP точки доступа
String ssidApPass = "";       // пароль точки доступа
String ssdpName = "jalousie"; // SSDP
String timeUp = "08:00:00";    // время открытия
String timeDown = "21:00:00";  // время закрытия
// Переменные для обнаружения модулей
String Devices = "";           // Поиск IP адресов устройств в сети
String DevicesList = "";       // IP адреса устройств в сети
String Language ="ru";         // язык web интерфейса
String Lang = "";              // файлы языка web интерфейса
int timeZone = 3;              // часовой пояс GTM
int Led1 = 12;                 // индикатор движения вверх
int Led2 = 13;                 // индикатор движения вниз
float timeServo1 = 10.0;       // Время вращения
float timeServo2 = 10.0;       // Время вращения
int speed = 90;                // Скорость вращения
int calibration = 90;          // Колибруем серву
int turn = 7;                  //Количество оборотов
int turnSensor = 0;
// Переменные для ddns
String ddns = "";              // url страницы тестирования WanIP
String ddnsName = "";          // адрес сайта ddns
int ddnsPort = 8080; // порт для обращение к устройству с wan

String kolibrTime = "03:00:00";// Время колибровки часов
volatile int chaingtime = LOW;
volatile int chaing = LOW;
//volatile int chaing1 = LOW;
int state0 = 0;
unsigned int localPort = 2390;
unsigned int ssdpPort = 1900;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void setup() {
 Serial.begin(115200);
 pinMode(turnSensor_pin, INPUT_PULLUP);
 pinMode(Tach0, INPUT);
 pinMode(Led1, OUTPUT);
 pinMode(Led2, OUTPUT);
 Serial.println("");
 // Включаем работу с файловой системой
 FS_init();
 // Загружаем настройки из файла
 loadConfig();
 HTTPWAN = new ESP8266WebServer(ddnsPort);
 // Подключаем сервомотор
 myservo.attach(servo_pin);
 //myservo.write(calibration);
 // Кнопка будет работать по прерыванию
 attachInterrupt(Tach0, Tach_0, FALLING);
 // Сенсор будет работать по прерыванию
 attachInterrupt(turnSensor_pin, turn_0, FALLING );
 //Запускаем WIFI
 WIFIAP_Client();
 // Закускаем UDP
 udp.begin(localPort);
 //udp.beginMulticast(WiFi.localIP(), ssdpAdress1, ssdpPort);
 Serial.print("Local port: ");
 Serial.println(udp.localPort());
 //настраиваем HTTP интерфейс
 HTTP_init();
 Serial.println("HTTP Ready!");
 //запускаем SSDP сервис
 SSDP_init();
 Serial.println("SSDP Ready!");
 // Включаем время из сети
 Time_init(timeZone);
 // Будет выполняться каждую секунду проверяя будильники
 tickerAlert.attach(1, alert);
 ip_wan();
}

void loop() {
 dnsServer.processNextRequest();
 delay(1);
 HTTP.handleClient();
 delay(1);
 HTTPWAN->handleClient();
 delay(1);
 handleUDP();

 // if (chaing && !chaing1) {
 if (chaing) {
  noInterrupts();
  switch (state0) {
   case 0:
    MotorUp();
    break;
   case 1:
    MotorDown();
    break;
  }
  interrupts();
 }
 if (chaingtime) {
  Time_init(timeZone);
  chaingtime=0;
 }
}

// Вызывается каждую секунду в обход основного циклу.
void alert() {
 String Time=XmlTime();
 if (timeUp.compareTo(Time) == 0) {
  MotorUp();
 }
 if (timeDown.compareTo(Time) == 0) {
  MotorDown();
 }
 if (kolibrTime.compareTo(Time) == 0) {
  chaingtime=1;
 }
 // В 15, 30, 45 минут каждого часа идет запрос на сервер ddns
 if ((Time == "00:00" || Time == "15:00" || Time == "30:00" || Time == "45:00") && ddns != "") {
  ip_wan();
 }
}
