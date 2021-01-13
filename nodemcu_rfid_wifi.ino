// FOR WIFI CONNECTION
//*******************
#include <EEPROM.h> // For saving wifi password and ssid
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

//Variables
int i = 0;
int statusCode;
String webPageHtml;
String wifiListHtmlOlTag;
const char *ssid = "Default_SSID";
const char *passphrase = "Default_Password";

//Function Decalration
void setupAP(void);
bool testWifi(void);
void launchWeb(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
//*******************

// RFID READER MFRC522
//*******************
#include <SPI.h>
#include <MFRC522.h>

const uint8_t rfid_pin = D8;
constexpr uint8_t SS_PIN = D4;
constexpr uint8_t RST_PIN = D3;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

//Function Decalration
void activateRfidReader(void);

String tag;
//*******************

const uint8_t led_G_indicator_pin = D0;
const uint8_t led_R_indicator_pin = D8;

void setup()
{
  // Initialising if (DEBUG) Serial Monitor
  Serial.begin(115200);

  // Setup RFID Reader
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  pinMode(rfid_pin, OUTPUT);
  //*******************

  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); // Initialasing EEPROM
  delay(10);
  pinMode(led_G_indicator_pin, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //-------------------- Read EEPROM for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  //-------------------- End Read EEPROM

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP(); // Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting…");

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    digitalWrite(led_G_indicator_pin, HIGH);
    delay(100);
    digitalWrite(led_G_indicator_pin, LOW);
    delay(100);
    server.handleClient();
  }
}

//-------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < 20)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  wifiListHtmlOlTag = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    wifiListHtmlOlTag += "<li>";
    wifiListHtmlOlTag += WiFi.SSID(i);
    wifiListHtmlOlTag += " (";
    wifiListHtmlOlTag += WiFi.RSSI(i);

    wifiListHtmlOlTag += ")";
    wifiListHtmlOlTag += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    wifiListHtmlOlTag += "</li>";
  }
  wifiListHtmlOlTag += "</ol>";
  delay(100);
  WiFi.softAP("Gym Synergy", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}

void createWebServer()
{
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      webPageHtml = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
      webPageHtml += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      webPageHtml += ipStr;
      webPageHtml += "<p>";
      webPageHtml += wifiListHtmlOlTag;
      webPageHtml += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      webPageHtml += "</html>";
      server.send(200, "text/html", webPageHtml);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      webPageHtml = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", webPageHtml);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0)
      {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i)
        {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        webPageHtml = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      }
      else
      {
        webPageHtml = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", webPageHtml);
    });
  }
}

void activateRfidReader()
{
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (rfid.PICC_ReadCardSerial())
  {
    for (byte i = 0; i < 4; i++)
    {
      tag += rfid.uid.uidByte[i];
    }
    Serial.println(tag);
    if (tag == "57100129163")
    {
      Serial.println("Access Granted!");
      digitalWrite(led_G_indicator_pin, HIGH);
      delay(100);
      digitalWrite(led_G_indicator_pin, LOW);
      delay(100);
      digitalWrite(led_G_indicator_pin, HIGH);
      delay(100);
      digitalWrite(led_G_indicator_pin, LOW);
      delay(100);
      digitalWrite(led_G_indicator_pin, HIGH);
      delay(100);
      digitalWrite(led_G_indicator_pin, LOW);
      delay(100);
    }
    else
    {
      Serial.println("Access Denied!");
      digitalWrite(led_R_indicator_pin, HIGH);
      delay(2000);
      digitalWrite(led_R_indicator_pin, LOW);
    }
    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    for (int i = 0; i < 10; i++)
    {
      digitalWrite(led_G_indicator_pin, HIGH);
      // delay(1000);
      // digitalWrite(led_G_indicator_pin, LOW);
      // delay(1000);
    }
    activateRfidReader();
  }
  else
  {
    delay(100);
    digitalWrite(led_R_indicator_pin, HIGH);
    delay(100);
    digitalWrite(led_R_indicator_pin, LOW);
  }
}
