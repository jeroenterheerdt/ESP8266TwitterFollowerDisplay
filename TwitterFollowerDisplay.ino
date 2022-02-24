// Includes
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <TM1637TinyDisplay6.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>

//------- Replace the following! ------
char ssid[] = "SSID";       // your network SSID (name)
char password[] = "PASSWORD";  // your network key
String twitterHandle = "TWITTERHANDLE"; // Twitter username
#define BEARER_TOKEN "TOKEN" //get it from developer.twitter.com
String bearer = strcat("Bearer ",BEARER_TOKEN);
// Module connection pins (Digital Pins)
#define CLK D1
#define DIO D2

// Update delay
unsigned long api_delay = 1 * 60000;
unsigned long api_due_time;

String TWITTER_URL  = "https://api.twitter.com/1.1/users/show.json?screen_name=";

//HTTP without SSL checking
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

HTTPClient https;

//Display
TM1637TinyDisplay6 display(CLK, DIO);

void setup() {
  Serial.begin(115200);

  display.setBrightness(BRIGHT_HIGH);
  display.clear();

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print(F("Connecting Wifi: "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  ESP.wdtFeed();

  client->setInsecure();
}

void loop() {
  if (millis() > api_due_time)  {
    String url = TWITTER_URL + twitterHandle;
    Serial.println(url);
    ESP.wdtFeed();
    https.begin(*client, url);
    https.addHeader("Authorization", bearer.c_str());
    // Send HTTPS request
    ESP.wdtFeed();
    int httpResponseCode = https.GET();
    ESP.wdtFeed();
    String payload = "{}";

    if (httpResponseCode > 0) {
      Serial.print(F("HTTP Response code: "));
      Serial.println(httpResponseCode);
      payload = https.getString();
    }
    else {
      Serial.print(F("Error code: "));
      Serial.println(httpResponseCode);
    }
    // Free resources
    https.end();
    Serial.println(payload);
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println(F("Parsing input failed!"));
      return;
    }
    Serial.print(F("JSON object = "));
    Serial.println(myObject);
    unsigned long followers_count = long(myObject["followers_count"]);
    Serial.println(followers_count);
    //display.clear();
    ESP.wdtFeed();
    display.showNumber(long(followers_count), false);
    ESP.wdtFeed();
    api_due_time = millis() + api_delay;
  }
  ESP.wdtFeed();
}