/*
   ____  __  __  ____  _  _  _____       ___  _____  ____  _  _
  (  _ \(  )(  )(_  _)( \( )(  _  )___  / __)(  _  )(_  _)( \( )
   )(_) ))(__)(  _)(_  )  (  )(_)((___)( (__  )(_)(  _)(_  )  (
  (____/(______)(____)(_)\_)(_____)     \___)(_____)(____)(_)\_)
  Official code for ESP8266 boards                   version 3.3

  Duino-Coin Team & Community 2019-2022 © MIT Licensed
  https://duinocoin.com
  https://github.com/revoxhere/duino-coin

  If you don't know where to start, visit official website and navigate to
  the Getting Started page. Have fun mining!
*/

/* If optimizations cause problems, change them to -O0 (the default)
  NOTE: For even better optimizations also edit your Crypto.h file.
  On linux that file can be found in the following location:
  ~/.arduino15//packages/esp8266/hardware/esp8266/3.0.2/cores/esp8266/ */
#pragma GCC optimize ("-Ofast")

/* If during compilation the line below causes a
  "fatal error: arduinoJson.h: No such file or directory"
  message to occur; it means that you do NOT have the
  ArduinoJSON library installed. To install it, 
  go to the below link and follow the instructions: 
  https://github.com/revoxhere/duino-coin/issues/832 */
#include <ArduinoJson.h>

/// Modify by : Thanormsin.M
// Modify Date : 11-Jul-21
#include <EEPROM.h>
//#include <Wire.h>

#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266mDNS.h> // OTA libraries
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
#include <time.h>                                                    

//### Auto WiFi Connect ##################
const char* ssidNm    = "";

const char* ssid2     = "Tha1-2.4G";        //<-- ใส่ชือ Wifi รอง ตรงนี ถ้าใช้ WiFi ตัวเดียวก็ให้ใส่เหมือนกันกับ ssid1
const char* password2 = "0819065291";         //<-- ใส่รหัส Wifi รอง ตรงนี

const char* ssid1     = "Tha2-2.4G";        //<-- ใส่ชือ Wifi หลัก
const char* password1 = "1234567890";         //<-- ใส่รหัส Wifi หลัก ตรงนี

///### Other ##################
int lastWiFi = 0;
//int ReCnctWiFi = 1;

String USERNAME       = "";
//String RIG_IDENTIFIER = "";
String RIG_IDENTIFIER = "";
String AcctName       = "Thanormsin";  //<-- ใส่ชือ User Name ตรงนี
String BrdNm          = "8266-02"; //<-- ใส่ชือ RigName ตรงนี

String eeprom_acct;
String eeprom_brd;
String HostNm          = ""; 
int x;

// Change the part in brackets to your WiFi password
//const char *PASSWORD = "0819065291";
// Change the part in brackets to your Duino-Coin username
//const char *USERNAME = "Thanormsin";
// Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
//const char *RIG_IDENTIFIER = "";

// รายละเอียดของ OTI, ตำแหน่งเก็บไฟล์
const String FirmwareVer={"4.4"}; // Last of Version\
//Siwadol
//#define URL_fw_Version "/Thanormsin/FwESP01S/master/bin_version-Tha.txt"
//#define URL_fw_Bin "https://raw.githubusercontent.com/Thanormsin/FwESP01S/master/fw-Tha.bin"

//Thanormsin
#define URL_fw_Version "/Thanormsin/FwESP01/main/bin_version-Nop.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/Thanormsin/FwESP01/main/fw-Nop.bin"
                   
const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

void setClock() {
   // Set time via NTP, as required for x.509 validation
   configTime(6 * 3600, 0, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  //configTime(6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    
    //Blinking LED 
    digitalWrite(LED_BUILTIN,HIGH);
  }
  
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j
ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL
MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3
LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug
RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm
+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW
PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM
xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB
Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3
hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg
EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF
MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA
FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec
nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z
eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF
hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2
Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe
vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep
+OkuE6N36B9K
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);

extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

//void connect_wifi();
//void firmwareUpdate();


void FirmwareUpdate()
{  
  setClock();
  //Add New Line
  Serial.println(" ");
  
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
     Serial.println(URL_fw_Bin);
  }
  else
  {
    Serial.println("New firmware detected");
    Serial.println("Downloading and install new firmware");
    Serial.println(URL_fw_Bin);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW); 
  //t_httpUpdate_return ret = httpUpdate.update(client, URL_fw_Bin);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
  //t_httpUpdate_return ret = ESPhttpUpdate.update("http://mydomain.com/arduino.bin");
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.println(URL_fw_Bin);
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    } 
  }
 }  


unsigned long previousMillis_2 = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 60000;              // Change Time Firmware ReUpdate
const long mini_interval = 1000;

 void repeatedCall(){
    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= interval) 
    {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
//      setClock();
      FirmwareUpdate();
    }

    if ((currentMillis - previousMillis_2) >= mini_interval) {
      static int idle_counter=0;
      previousMillis_2 = currentMillis;    
      Serial.print("Active fw version:");
      Serial.println(FirmwareVer);
      Serial.print("Idle Loop....");
      Serial.println(idle_counter++);
      Serial.println(" ");
     if(idle_counter%2==0)
      digitalWrite(LED_BUILTIN, HIGH);
     else 
      digitalWrite(LED_BUILTIN, LOW);
     //if(WiFi.status() == !WL_CONNECTED) 
      //SetupWifi();
      //Connect2WiFi();
      
   }
 }


/* If during compilation the line below causes a
  "fatal error: Crypto.h: No such file or directory"
  message to occur; it means that you do NOT have the
  latest version of the ESP8266/Arduino Core library.
  To install/upgrade it, go to the below link and
  follow the instructions of the readme file:
  https://github.com/esp8266/Arduino */
#include <bearssl/bearssl.h>
#include <TypeConversion.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>

// Uncomment the line below if you wish to use a DHT sensor (Duino IoT beta)
// #define USE_DHT

// Uncomment the line below if you wish to register for IOT updates with an MQTT broker
// #define USE_MQTT

// If you don't know what MQTT means check this link:
// https://www.techtarget.com/iotagenda/definition/MQTT-MQ-Telemetry-Transport

#ifdef USE_DHT
  float temp = 0.0;
  float hum = 0.0;

  // Install "DHT sensor library" if you get an error
  #include <DHT.h>
  // Change D3 to the pin you've connected your sensor to
  #define DHTPIN D3
  // Set DHT11 or DHT22 accordingly
  #define DHTTYPE DHT11

  DHT dht(DHTPIN, DHTTYPE);
#endif

#ifdef USE_MQTT
  // Install "PubSubClient" if you get an error
  #include <PubSubClient.h>

  long lastMsg = 0;

  // Change the part in brackets to your MQTT broker address
  #define mqtt_server "broker.hivemq.com"
  // broker.hivemq.com is for testing purposes, change it to your broker address

  // Change this to your MQTT broker port
  #define mqtt_port 1883
  // If you want to use user and password for your MQTT broker, uncomment the line below
  // #define mqtt_use_credentials

  // Change the part in brackets to your MQTT broker username
  #define mqtt_user "My cool mqtt username"
  // Change the part in brackets to your MQTT broker password
  #define mqtt_password "My secret mqtt pass"

  // Change this if you want to send data to the topic every X milliseconds
  #define mqtt_update_time 5000

  // Change the part in brackets to your MQTT humidity topic
  #define humidity_topic "sensor/humidity"
  // Change the part in brackets to your MQTT temperature topic
  #define temperature_topic "sensor/temperature"

  WiFiClient espClient;
  PubSubClient mqttClient(espClient);

  void mqttReconnect()
  {
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
      Serial.print("Attempting MQTT connection...");

      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);

      // Attempt to connect
      #ifdef mqtt_use_credentials
        if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_password))
      #else
        if (mqttClient.connect(clientId.c_str()))
      #endif
      {
        Serial.println("connected");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
#endif

namespace
{
  // Change the part in brackets to your WiFi name
  const char *SSID = "Tha1-2.4G";
  // Change the part in brackets to your WiFi password
  const char *PASSWORD = "0819065291";
//  // Change the part in brackets to your Duino-Coin username
//  const char *USERNAME = "Thanormsin";
//  // Change the part in brackets if you want to set a custom miner name (use Auto to autogenerate, None for no name)
//  const char *RIG_IDENTIFIER = "8266_01";
  // Change the part in brackets to your mining key (if you have enabled it in the wallet)
  const char *MINER_KEY = "None";
  // Set to true to use the 160 MHz overclock mode (and not get the first share rejected)
  const bool USE_HIGHER_DIFF = true;
  // Set to true if you want to host the dashboard page (available on ESPs IP address)
  const bool WEB_DASHBOARD = false;
  // Set to true if you want to update hashrate in browser without reloading the page
  const bool WEB_HASH_UPDATER = false;
  // Set to false if you want to disable the onboard led blinking when finding shares
  const bool LED_BLINKING = true;

/* Do not change the lines below. These lines are static and dynamic variables
   that will be used by the program for counters and measurements. */
const char * DEVICE = "ESP8266";
const char * POOLPICKER_URL[] = {"https://server.duinocoin.com/getPool"};
const char * MINER_BANNER = "Official ESP8266 Miner";
const char * MINER_VER = "3.3";
unsigned int share_count = 0;
unsigned int port = 0;
unsigned int difficulty = 0;
float hashrate = 0;
String AutoRigName = "";
String host = "";
String node_id = "";

const char WEBSITE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<!--
    Duino-Coin self-hosted dashboard
    MIT licensed
    Duino-Coin official 2019-2022
    https://github.com/revoxhere/duino-coin
    https://duinocoin.com
-->
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Duino-Coin @@DEVICE@@ dashboard</title>
    <link rel="stylesheet" href="https://server.duinocoin.com/assets/css/mystyles.css">
    <link rel="shortcut icon" href="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
    <link rel="icon" type="image/png" href="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
</head>
<body>
    <section class="section">
        <div class="container">
            <h1 class="title">
                <img class="icon" src="https://github.com/revoxhere/duino-coin/blob/master/Resources/duco.png?raw=true">
                @@DEVICE@@ <small>(@@ID@@)</small>
            </h1>
            <p class="subtitle">
                Self-hosted, lightweight, official dashboard for your <strong>Duino-Coin</strong> miner
            </p>
        </div>
        <br>
        <div class="container">
            <div class="columns">
                <div class="column">
                    <div class="box">
                        <p class="subtitle">
                            Mining statistics
                        </p>
                        <div class="columns is-multiline">
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    <span id="hashratex">@@HASHRATE@@</span>kH/s
                                </div>
                                <div class="heading is-size-5">
                                    Hashrate
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@DIFF@@
                                </div>
                                <div class="heading is-size-5">
                                    Difficulty
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@SHARES@@
                                </div>
                                <div class="heading is-size-5">
                                    Shares
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@NODE@@
                                </div>
                                <div class="heading is-size-5">
                                    Node
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="column">
                    <div class="box">
                        <p class="subtitle">
                            Device information
                        </p>
                        <div class="columns is-multiline">
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@DEVICE@@
                                </div>
                                <div class="heading is-size-5">
                                    Device type
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@ID@@
                                </div>
                                <div class="heading is-size-5">
                                    Device ID
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@MEMORY@@
                                </div>
                                <div class="heading is-size-5">
                                    Free memory
                                </div>
                            </div>
                            <div class="column" style="min-width:15em">
                                <div class="title is-size-5 mb-0">
                                    @@VERSION@@
                                </div>
                                <div class="heading is-size-5">
                                    Miner version
                                </div>
                            </div>
)====="
#ifdef USE_DHT
"                            <div class=\"column\" style=\"min-width:15em\">"
"                                <div class=\"title is-size-5 mb-0\">"
"                                    @@TEMP@@ °C"
"                                </div>"
"                                <div class=\"heading is-size-5\">"
"                                    Temperature"
"                                </div>"
"                            </div>"
"                            <div class=\"column\" style=\"min-width:15em\">"
"                                <div class=\"title is-size-5 mb-0\">"
"                                    @@HUM@@ %"
"                                </div>"
"                                <div class=\"heading is-size-5\">"
"                                    Humidity"
"                                </div>"
"                            </div>"
#endif
  R"=====(
                        </div>
                    </div>
                </div>
            </div>
            <br>
            <div class="has-text-centered">
                <div class="title is-size-6 mb-0">
                    Hosted on
                    <a href="http://@@IP_ADDR@@">
                        http://<b>@@IP_ADDR@@</b>
                    </a>
                    &bull;
                    <a href="https://duinocoin.com">
                        duinocoin.com
                    </a>
                    &bull;
                    <a href="https://github.com/revoxhere/duino-coin">
                        github.com/revoxhere/duino-coin
                    </a>
                </div>
            </div>
        </div>
        <script>
            setInterval(function(){
                getData();
            }, 3000);
            
            function getData() {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        document.getElementById("hashratex").innerHTML = this.responseText;
                    }
                };
                xhttp.open("GET", "hashrateread", true);
                xhttp.send();
            }
        </script>
    </section>
</body>
</html>
)=====";

ESP8266WebServer server(80);

void hashupdater(){ //update hashrate every 3 sec in browser without reloading page
  server.send(200, "text/plain", String(hashrate / 1000));
  Serial.println("Update hashrate on page");
};

void UpdateHostPort(String input) {
  // Thanks @ricaun for the code
  DynamicJsonDocument doc(256);
  deserializeJson(doc, input);
  const char* name = doc["name"];
  
  host = String((const char*)doc["ip"]);
  port = int(doc["port"]);
  node_id = String(name);

  Serial.println("Poolpicker selected the best mining node: " + node_id);
}

String httpGetString(String URL) {
  String payload = "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  if (http.begin(client, URL)) {
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) payload = http.getString();
    else Serial.printf("Error fetching node from poolpicker: %s\n", http.errorToString(httpCode).c_str());

    http.end();
  }
  return payload;
}

void UpdatePool() {
  String input = "";
  int waitTime = 1;
  int poolIndex = 0;
  int poolSize = sizeof(POOLPICKER_URL) / sizeof(char*);

  while (input == "") {
    Serial.println("Fetching mining node from the poolpicker in " + String(waitTime) + "s");
    input = httpGetString(POOLPICKER_URL[poolIndex]);
    poolIndex += 1;

    // Check if pool index needs to roll over
    if( poolIndex >= poolSize ){
      poolIndex %= poolSize;
      delay(waitTime * 1000);

      // Increase wait time till a maximum of 32 seconds (addresses: Limit connection requests on failure in ESP boards #1041)
      waitTime *= 2;
      if( waitTime > 32 )
        waitTime = 32;
    }
  }

  // Setup pool with new input
  UpdateHostPort(input);
}

WiFiClient client;
String client_buffer = "";
String chipID = "";
String START_DIFF = "";

// Loop WDT... please don't feed me...
// See lwdtcb() and lwdtFeed() below
Ticker lwdTimer;
#define LWD_TIMEOUT   20000

unsigned long lwdCurrentMillis = 0;
unsigned long lwdTimeOutMillis = LWD_TIMEOUT;

#define END_TOKEN  '\n'
#define SEP_TOKEN  ','

#define LED_BUILTIN 2

#define BLINK_SETUP_COMPLETE 2
#define BLINK_CLIENT_CONNECT 3
#define BLINK_RESET_DEVICE   5

void SetupWifi() {
  Serial.println("Connecting to: " + String(SSID));
  WiFi.mode(WIFI_STA); // Setup ESP in client mode
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(SSID, PASSWORD);

  int wait_passes = 0;
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++wait_passes >= 10) {
      WiFi.begin(SSID, PASSWORD);
      wait_passes = 0;
    }
  }

  Serial.println("\n\nnSuccessfully connected to WiFi");
  Serial.println("Local IP address: " + WiFi.localIP().toString());
  Serial.println("Rig name: " + String(RIG_IDENTIFIER));
  Serial.println();

  UpdatePool();
}

void SetupOTA() {
  // Prepare OTA handler
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.setHostname(RIG_IDENTIFIER.c_str()); // Give port a name not just address
  ArduinoOTA.begin();
}

void blink(uint8_t count, uint8_t pin = LED_BUILTIN) {
  if (LED_BLINKING){
    uint8_t state = HIGH;

    for (int x = 0; x < (count << 1); ++x) {
      digitalWrite(pin, state ^= HIGH);
      delay(50);
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void RestartESP(String msg) {
  Serial.println(msg);
  Serial.println("Restarting ESP...");
  blink(BLINK_RESET_DEVICE);
  ESP.reset();
}

// Our new WDT to help prevent freezes
// code concept taken from https://sigmdel.ca/michel/program/esp8266/arduino/watchdogs2_en.html
void ICACHE_RAM_ATTR lwdtcb(void) {
  if ((millis() - lwdCurrentMillis > LWD_TIMEOUT) || (lwdTimeOutMillis - lwdCurrentMillis != LWD_TIMEOUT))
    RestartESP("Loop WDT Failed!");
}

void lwdtFeed(void) {
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
}

void VerifyWifi() {
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
    WiFi.reconnect();
}

void handleSystemEvents(void) {
  VerifyWifi();
  ArduinoOTA.handle();
  yield();
}

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int max_index = data.length() - 1;

  for (int i = 0; i <= max_index && found <= index; i++) {
    if (data.charAt(i) == separator || i == max_index) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == max_index) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void waitForClientData(void) {
  client_buffer = "";

  while (client.connected()) {
    if (client.available()) {
      client_buffer = client.readStringUntil(END_TOKEN);
      if (client_buffer.length() == 1 && client_buffer[0] == END_TOKEN)
        client_buffer = "???\n"; // NOTE: Should never happen

      break;
    }
    handleSystemEvents();
  }
}

void ConnectToServer() {
  if (client.connected())
    return;

  Serial.println("\n\nConnecting to the Duino-Coin server...");
  while (!client.connect(host.c_str(), port));

  waitForClientData();
  Serial.println("Connected to the server. Server version: " + client_buffer );
  blink(BLINK_CLIENT_CONNECT); // Sucessfull connection with the server
}

bool max_micros_elapsed(unsigned long current, unsigned long max_elapsed) {
  static unsigned long _start = 0;

  if ((current - _start) > max_elapsed) {
    _start = current;
    return true;
  }
  return false;
}

void dashboard() {
  Serial.println("Handling HTTP client");

  String s = WEBSITE;
  s.replace("@@IP_ADDR@@", WiFi.localIP().toString());
  
  s.replace("@@HASHRATE@@", String(hashrate / 1000));
  s.replace("@@DIFF@@", String(difficulty / 100));
  s.replace("@@SHARES@@", String(share_count));
  s.replace("@@NODE@@", String(node_id));

  s.replace("@@DEVICE@@", String(DEVICE));
  s.replace("@@ID@@", String(RIG_IDENTIFIER));
  s.replace("@@MEMORY@@", String(ESP.getFreeHeap()));
  s.replace("@@VERSION@@", String(MINER_VER));
#ifdef USE_DHT
  s.replace("@@TEMP@@", String(temp));
  s.replace("@@HUM@@", String(hum));
#endif
  server.send(200, "text/html", s);
}

} // namespace

////// EEPROM Setup ////////
void GetAcctName(){
//  Serial.println("");
//  Serial.println("------------------Get Account Name ------------------"); 
  // อ่าน AcctName
  eeprom_acct = "";
  for(int i=0;i< 20;i++){ 
//    Serial.print(i);
//    Serial.print(" GAcctNm ");
//    Serial.println(char(EEPROM.read(i))); 
  
    if(EEPROM.read(i) > 20)eeprom_acct += char(EEPROM.read(i));
  }  

  USERNAME = eeprom_acct;
  Serial.println(USERNAME); 
 
}

void GetBoardName(){
  // อ่าน BrdNm
  eeprom_brd = "";
  for(int i=20;i< 40;i++){
   if(EEPROM.read(i) > 0){
//      Serial.print(i);
//      Serial.print(" GBrdNm ");
//      Serial.println(char(EEPROM.read(i)));
    
      eeprom_brd += char(EEPROM.read(i));
   }
  }

  RIG_IDENTIFIER = String(eeprom_brd);
  Serial.println(RIG_IDENTIFIER); 
}

void SaveName(){  
  Serial.println("");
  Serial.println("------------------Start Save ------------------");
  // CLEAR EEPROM /////////
  for(int i=0;i < 20;i++)
  {
     EEPROM.write(i, 0);
  }
 
  Serial.print("AcctName : ");
  Serial.println(AcctName);  

  // เขียน AcctName
  for(int i=0;i < AcctName.length();i++)
  {
//    Serial.print(i);
//    Serial.print(" SvAcctNm ");
//    Serial.println(AcctName[i]);
    
    EEPROM.write(i, AcctName[i]); 
  }
  USERNAME = AcctName;

  // เคลียร์พื้นที่ก่อน เขียน BrdNm
  for(int i=20;i < 40;i++)
  {
   EEPROM.write(i, 0);
  }

  // เขียน BrdNm
//  Serial.println("------------------Show : BrdNm ------------------"); 
  Serial.print("BoardName : ");
  Serial.println(BrdNm); 
  
  x = 0;
  for(int i=20;i < 40;i++)
  {
//    Serial.print(i);
//    Serial.print(" SvBrdNm ");
//    Serial.println(BrdNm[x]); 

    if(x < BrdNm.length())EEPROM.write(i, BrdNm[x]); //Write one by one with starting address of 0x0F
    x++;
  }
  RIG_IDENTIFIER = String(BrdNm);
  
  EEPROM.commit();
   
  Serial.println("------------------End Save ------------------");
}

////// ตั้งค่า 2 WiFi //////
void Connect2WiFi()
{
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.mode(WIFI_OFF);
      
      //Blinking
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);
    
      digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
      delay(250);

      digitalWrite(LED_BUILTIN,HIGH);
    
//      Serial.print("O");
      
      //delay(250);
      
      WiFi.mode(WIFI_STA);
      if (lastWiFi == 1) {
        Serial.print("Connecting WiFi#2 :");
        Serial.println(ssid2);
        WiFi.begin(ssid2, password2);
        
        lastWiFi = 2;
        ssidNm = ssid2;   
    
      } else {        
        Serial.print("Connecting WiFi#1 :");
        //WiFi.begin(ssid1, password1);
        
        Serial.println("Start Connect to WiFi"); 
        Serial.println(ssid1);
        WiFi.begin(ssid1, password1);
        
        lastWiFi = 1;
        ssidNm = ssid1;  
      } 
    
      unsigned long timeout = millis();
      while (WiFi.status() != WL_CONNECTED) {
         if (millis() - timeout > 20000) {
            Serial.println("");
            Serial.println(">>> Connecting Wait Timeout !");
            break;
         }
//         delay(500);
         Serial.print(".");
         digitalWrite(LED_BUILTIN,!(digitalRead(LED_BUILTIN)));
         delay(500);
         
         digitalWrite(LED_BUILTIN,HIGH);
      }
    }
    Serial.println("");
    Serial.print("Connected to WIFi: ");
//    Serial.print("SSID: ");
    Serial.println(ssidNm);
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());     
    Serial.println(""); 

    delay(5000);
}

void setup() {
  EEPROM.begin(512);
  
  // Start serial connection
  Serial.begin(115200);
 
  pinMode(LED_BUILTIN, OUTPUT);
  
  //Blinking LED 
  digitalWrite(LED_BUILTIN,HIGH);
  
  delay(3000);

  SetupWifi();
  //Connect2WiFi();

  // EEPROM Setup ////////////////////////
  Serial.println("");
  Serial.println("------------------Get EEPROM Setting------------------");
  GetAcctName();
  GetBoardName();
  Serial.println("------------------Get EEPROM Setting------------------");

  Serial.println("");
  Serial.println("------------------Compair Setting------------------");

  Serial.print("AcctName :");
  Serial.print(eeprom_acct); 
  Serial.print(" <=> ");
  Serial.println(AcctName);
  Serial.println("------------------Compair Setting------------------");
   
  //ทำการตรวจสอบ Account
  if (String(eeprom_acct).equals(String(AcctName))){
    // อ่าน AcctNm
    GetAcctName();   
    // อ่าน BrdNm
    GetBoardName();

  }else{
    
    Serial.println("Writting config to New board.");
    Serial.print("With : ");
    Serial.print(AcctName);
    Serial.print("/");
    Serial.println(BrdNm);
    
    SaveName();
    Serial.println("------------ Done ------------");
  }
 
  Serial.println(" ");
  Serial.print("2-Hello my name is " );
  Serial.print(USERNAME);
  Serial.print("/");
  Serial.print(RIG_IDENTIFIER);
  Serial.println("");
 
//  Serial.println("\nDuino-Coin ESP8266 Miner v2.55");
//  
//  Serial.begin(500000);
  Serial.println("\nDuino-Coin " + String(MINER_VER));
  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef USE_MQTT
    mqttClient.setServer(mqtt_server, mqtt_port);
  #endif
  
  #ifdef USE_DHT
    Serial.println("Initializing DHT sensor");
    dht.begin();
    Serial.println("Test reading: " + String(dht.readHumidity()) + "% humidity");
    Serial.println("Test reading: temperature " + String(dht.readTemperature()) + "*C");
  #endif

  // Autogenerate ID if required
  chipID = String(ESP.getChipId(), HEX);

  //if(strcmp(RIG_IDENTIFIER, "Auto") == 0 ){
  if(strcmp(RIG_IDENTIFIER.c_str(), "Auto") == 0 ){
    AutoRigName = "ESP8266-" + chipID;
    AutoRigName.toUpperCase();
    RIG_IDENTIFIER = AutoRigName.c_str();
  }

  SetupWifi();
  SetupOTA();

  lwdtFeed();
  lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);
  if (USE_HIGHER_DIFF) START_DIFF = "ESP8266NH";
  else START_DIFF = "ESP8266N";

  if(WEB_DASHBOARD) {
    if (!MDNS.begin(RIG_IDENTIFIER)) {
      Serial.println("mDNS unavailable");
    }
    MDNS.addService("http", "tcp", 80);
    Serial.print("Configured mDNS for dashboard on http://" 
                  + String(RIG_IDENTIFIER)
                  + ".local (or http://"
                  + WiFi.localIP().toString()
                  + ")");
    server.on("/", dashboard);
    if (WEB_HASH_UPDATER) server.on("/hashrateread", hashupdater);
    server.begin();
  }

  blink(BLINK_SETUP_COMPLETE);
}

void loop() {
  br_sha1_context sha1_ctx, sha1_ctx_base;
  uint8_t hashArray[20];
  String duco_numeric_result_str;
  
  // 1 minute watchdog
  lwdtFeed();

  // OTA handlers
  VerifyWifi();
  ArduinoOTA.handle();
  if(WEB_DASHBOARD) server.handleClient();

  ConnectToServer();
  Serial.println("Asking for a new job for user: " + String(USERNAME));

  #ifndef USE_DHT
    client.print("JOB," + 
                 String(USERNAME) + SEP_TOKEN +
                 String(START_DIFF) + SEP_TOKEN +
                 String(MINER_KEY) + END_TOKEN);
  #endif

  #ifdef USE_DHT
    temp = dht.readTemperature();
    hum = dht.readHumidity();

    Serial.println("DHT readings: " + String(temp) + "*C, " + String(hum) + "%");
    client.print("JOB," + 
                 String(USERNAME) + SEP_TOKEN +
                 String(START_DIFF) + SEP_TOKEN +
                 String(MINER_KEY) + SEP_TOKEN +
                 String(temp) + "@" + String(hum) + END_TOKEN);
  #endif
  
  #ifdef USE_MQTT
  
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
    #ifdef USE_DHT
    long now = millis();
    if (now - lastMsg > mqtt_update_time) {
      lastMsg = now;
      mqttClient.publish(temperature_topic, String(temp).c_str(), true);
      mqttClient.publish(humidity_topic, String(hum).c_str(), true); 
    }
    #endif

  #endif

  waitForClientData();
  String last_block_hash = getValue(client_buffer, SEP_TOKEN, 0);
  String expected_hash_str = getValue(client_buffer, SEP_TOKEN, 1);
  difficulty = getValue(client_buffer, SEP_TOKEN, 2).toInt() * 100 + 1;

  if (USE_HIGHER_DIFF) system_update_cpu_freq(160);

  int job_len = last_block_hash.length() + expected_hash_str.length() + String(difficulty).length();

  Serial.println("Received job with size of " + String(job_len) + " bytes: " + last_block_hash + " " + expected_hash_str + " " + difficulty);
 
  uint8_t expected_hash[20];
  experimental::TypeConversion::hexStringToUint8Array(expected_hash_str, expected_hash, 20);

  br_sha1_init(&sha1_ctx_base);
  br_sha1_update(&sha1_ctx_base, last_block_hash.c_str(), last_block_hash.length());

  float start_time = micros();
  max_micros_elapsed(start_time, 0);

  String result = "";
  if (LED_BLINKING) digitalWrite(LED_BUILTIN, LOW);
  for (unsigned int duco_numeric_result = 0; duco_numeric_result < difficulty; duco_numeric_result++) {
    // Difficulty loop
    sha1_ctx = sha1_ctx_base;
    duco_numeric_result_str = String(duco_numeric_result);

    br_sha1_update(&sha1_ctx, duco_numeric_result_str.c_str(), duco_numeric_result_str.length());
    br_sha1_out(&sha1_ctx, hashArray);

    if (memcmp(expected_hash, hashArray, 20) == 0) {
      // If result is found
      if (LED_BLINKING) digitalWrite(LED_BUILTIN, HIGH);
      unsigned long elapsed_time = micros() - start_time;
      float elapsed_time_s = elapsed_time * .000001f;
      hashrate = duco_numeric_result / elapsed_time_s;
      share_count++;
      client.print(String(duco_numeric_result)
                   + ","
                   + String(hashrate)
                   + ","
                   + String(MINER_BANNER)
                   + " "
                   + String(MINER_VER)
                   + ","
                   + String(RIG_IDENTIFIER)
                   + ",DUCOID"
                   + String(chipID)
                   + "\n");

      waitForClientData();
      Serial.println(client_buffer
                     + " share #"
                     + String(share_count)
                     + " (" + String(duco_numeric_result) + ")"
                     + " hashrate: "
                     + String(hashrate / 1000, 2)
                     + " kH/s ("
                     + String(elapsed_time_s)
                     + "s)");
      break;
    }
    if (max_micros_elapsed(micros(), 500000)) {
      handleSystemEvents();
    }
  }
  
  
  //////////////////////////////////////////////////////////////////////////////////////////////////
//    if (button_boot.pressed) { //to connect wifi via Android esp touch app 
//      Serial.println("Manual Firmware update Starting..");
//      FirmwareUpdate();
//      button_boot.pressed = false;
//    }
//  
    
    repeatedCall(); 
  
    //Blinking LED 
    digitalWrite(LED_BUILTIN,HIGH);  
  
}
