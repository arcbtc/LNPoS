/*
ESP8266/ESP32 publish the RSSI as the WiFi signal strength to ThingSpeak channel.
This example is for explaining how to use the AutoConnect library.

In order to execute this example, the ThingSpeak account is needed. Sing up
for New User Account and create a New Channel via My Channels.
For details, please refer to the project page.
https://hieromon.github.io/AutoConnect/howtoembed.html#used-with-mqtt-as-a-client-application

This example is based on the environment as of March 20, 2018.
Copyright (c) 2020 Hieromon Ikasamo.
This software is released under the MIT License.
https://opensource.org/licenses/MIT
*/

// To properly include the suitable header files to the target platform.
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#define GET_CHIPID()    (ESP.getChipId())
#define GET_HOSTNAME()  (WiFi.hostname())
using WiFiWebServer = ESP8266WebServer;

#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#define GET_CHIPID()    ((uint16_t)(ESP.getEfuseMac()>>32))
#define GET_HOSTNAME()  (WiFi.getHostname())
using WiFiWebServer = WebServer;
#endif

#include <PubSubClient.h>
#include <AutoConnect.h>
#include "param.h"

// MQTT parameter settings page URLs
const char* AUX_SETTING_URI = "/mqtt_setting";
const char* AUX_SAVE_URI    = "/mqtt_save";
const char* AUX_CLEAR_URI   = "/mqtt_clear";

// This example shows a sketch that realizes the equivalent operation
// of mqttRSSI without using JSON.
// By comparing this example with the example using JSON, mqttRSSI or
// mqttRSSI_FS, you will better understand AutoConnect custom Web page
// facility.

// Declare AutoConnectElements for the page asf /mqtt_setting
ACStyle(style, "label+input,label+select{position:sticky;left:120px;width:230px!important;box-sizing:border-box;}");
ACText(header, "<h2>MQTT broker settings</h2>", "text-align:center;color:#2f4f4f;padding:10px;");
ACText(caption, "Publishing the WiFi signal strength to MQTT channel. RSSI value of ESP8266 to the channel created on ThingSpeak", "font-family:serif;color:#4682b4;");
ACInput(mqttserver, "", "Server", "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$", "MQTT broker server");
ACInput(channelid, "", "Channel ID", "^[0-9]{6}$");
ACInput(userkey, "", "User Key");
ACInput(apikey, "", "API Key");
ACElement(newline, "<hr>");
ACRadio(period, { "30 sec.", "60 sec.", "180 sec." }, "Update period", AC_Vertical, 1);
ACSubmit(save, "Start", AUX_SAVE_URI);
ACSubmit(discard, "Discard", "/");

// Declare the custom Web page as /mqtt_setting and contains the AutoConnectElements
AutoConnectAux mqtt_setting(AUX_SETTING_URI, "MQTT Setting", true, {
  style,
  header,
  caption,
  mqttserver,
  channelid,
  userkey,
  apikey,
  newline,
  period,
  save,
  discard
});

// Declare AutoConnectElements for the page as /mqtt_save
ACText(caption2, "<h4>Parameters available as:</h4>", "text-align:center;color:#2f4f4f;padding:10px;");
ACText(parameters);
ACSubmit(clear, "Clear channel", AUX_CLEAR_URI);

// Declare the custom Web page as /mqtt_save and contains the AutoConnectElements
AutoConnectAux mqtt_save(AUX_SAVE_URI, "MQTT Setting", false, {
  caption2,
  parameters,
  clear
});

WiFiWebServer server;
AutoConnect   portal(server);
AutoConnectConfig config;
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

unsigned long publishInterval = 0;
const char* userId = "anyone";

unsigned long lastPub = 0;
unsigned long lastAttempt = 0;
const unsigned long attemptInterval = 3000;
bool  reconnect = false;
int   retry;

mqtt_param_t  mqtt_param;

bool mqttConnect() {
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";  // For random generation of client ID.
  char  clientId[9];
  bool  rc;

  rc = mqttserver.value.length() > 0;
  if (rc) {
    // Attempts to connect to the MQTT broker based on a valid server name.
    mqttClient.setServer(mqttserver.value.c_str(), 1883);
    Serial.println(String("Attempting MQTT broker:") + mqttserver.value);

    // Changing the client ID each time you open a session with a broker is
    // important for publishing continuity. Sessions interrupted by
    // communication anomalies are thrown away and do not interfere with
    // subsequent publications.
    uint8_t i = 0;
    while (i < sizeof(clientId))
      clientId[i++] = alphanum[random(sizeof(alphanum))];
    clientId[i - 1] = '\0';

    rc = mqttClient.connect(clientId, userId, userkey.value.c_str());
    if (rc)
      Serial.println("Established:" + String(clientId));
    else
      Serial.println("Connection failed:" + String(mqttClient.state()));
  }
  return rc;
}

bool mqttPublish(const String& endPoint, const String& payload) {
  // By checking the connection with the broker at the time of the publish
  // request, it can delegate the reconnection attempt to the loop function.
  // This strategy eliminates handling the delay that occurs during the
  // broker reconnection attempt loop and smoothes AutoConnect communication
  // with the client.
  reconnect = !mqttClient.connected();
  if (!reconnect)
    return mqttClient.publish(endPoint.c_str(), payload.c_str());
  else
    return false;
}

int getStrength(uint8_t points) {
  uint8_t sc = points;
  long    rssi = 0;

  while (sc--) {
    rssi += WiFi.RSSI();
    delay(20);
  }
  return points ? static_cast<int>(rssi / points) : 0;
}

// Load parameters from persistent storage
String loadParams(AutoConnectAux& aux, PageArgument& args) {
  getParams(mqtt_param);
  mqttserver.value = mqtt_param.server;
  channelid.value = mqtt_param.channelid;
  userkey.value = mqtt_param.userkey;
  apikey.value = mqtt_param.apikey;
  publishInterval = mqtt_param.publishInterval;
  if (publishInterval == 30000UL)
    period.checked = 1;
  else if (publishInterval == 60000UL)
    period.checked = 2;
  else if (publishInterval == 180000UL)
    period.checked = 3;
  else
    period.checked = 1;
  return String("");
}

// Retreive the value of each element entered by '/mqtt_setting'.
String saveParams(AutoConnectAux& aux, PageArgument& args) {
  mqttserver.value.trim();
  channelid.value.trim();
  userkey.value.trim();
  apikey.value.trim();
  if (period.checked == 1)
    publishInterval = 30000UL;
  else if (period.checked == 2)
    publishInterval = 60000UL;
  else if (period.checked == 3)
    publishInterval = 180000UL;

  strncpy(mqtt_param.server, mqttserver.value.c_str(), sizeof(mqtt_param_t::server));
  strncpy(mqtt_param.channelid, channelid.value.c_str(), sizeof(mqtt_param_t::channelid));
  strncpy(mqtt_param.userkey, userkey.value.c_str(), sizeof(mqtt_param_t::userkey));
  strncpy(mqtt_param.apikey, apikey.value.c_str(), sizeof(mqtt_param_t::apikey));
  mqtt_param.publishInterval = publishInterval;
  putParams(mqtt_param);

  // Echo back saved parameters to AutoConnectAux page.
  String echo = "Server: " + mqttserver.value + "<br>";
  echo += "Channel ID: " + channelid.value + "<br>";
  echo += "User Key: " + userkey.value + "<br>";
  echo += "API Key: " + apikey.value + "<br>";
  echo += "Update period: " + String(publishInterval / 1000) + " sec.<br>";
  parameters.value = echo;

  return String("");
}

// The root of the URL to include the channel views that provide the Thingspeak.
void handleRoot() {
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "</head>"
    "<body>"
    "<iframe width=\"450\" height=\"260\" style=\"transform:scale(0.79);-o-transform:scale(0.79);-webkit-transform:scale(0.79);-moz-transform:scale(0.79);-ms-transform:scale(0.79);transform-origin:0 0;-o-transform-origin:0 0;-webkit-transform-origin:0 0;-moz-transform-origin:0 0;-ms-transform-origin:0 0;border: 1px solid #cccccc;\" src=\"https://thingspeak.com/channels/{{CHANNEL}}/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&type=line\"></iframe>"
    "<p style=\"padding-top:5px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
    "</body>"
    "</html>";

  content.replace("{{CHANNEL}}", channelid.value);
  server.send(200, "text/html", content);
}

// Clear channel using ThingSpeak's API.
void handleClearChannel() {
  HTTPClient  httpClient;

  String  endpoint = mqttserver.value;
  endpoint.replace("mqtt", "api");
  String  delUrl = "http://" + endpoint + "/channels/" + channelid.value + "/feeds.json?api_key=" + userkey.value;

  Serial.print("DELETE " + delUrl);
  if (httpClient.begin(wifiClient, delUrl)) {
    Serial.print(":");
    int resCode = httpClient.sendRequest("DELETE");
    const String& res = httpClient.getString();
    Serial.println(String(resCode) + String(",") + res);
    httpClient.end();
  }
  else
    Serial.println(" failed");

  // Returns the redirect response. The page is reloaded and its contents
  // are updated to the state after deletion.
  server.sendHeader("Location", String("http://") + server.client().localIP().toString() + String("/"));
  server.send(302, "text/plain", "");
  server.client().stop();
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  // Join the custom Web pages and register /mqtt_save handler
  portal.join({ mqtt_setting, mqtt_save });
  portal.on(AUX_SETTING_URI, loadParams);
  portal.on(AUX_SAVE_URI, saveParams);

  // Assign the captive portal popup screen to the URL as the root path.
  server.on("/", handleRoot);
  server.on(AUX_CLEAR_URI, handleClearChannel);
  config.homeUri = "/";
  config.bootUri = AC_ONBOOTURI_HOME;

#ifdef ARDUINO_ARCH_ESP8266
  // If the EEPROM contains an area that is used for the purpose of the
  // user sketch, its area must not collide with the stored area of the 
  // AutoConnect credentials. Specify AutoConnect::boundaryOffset to
  // avoid this collision.
  // This measure only needs to be taken care of on the ESP8266 platform.
  config.boundaryOffset = sizeof(mqtt_param_t);
#endif
  // Reconnect and continue publishing even if WiFi is disconnected.
  config.autoReconnect = true;
  config.reconnectInterval = 1;
  portal.config(config);

  // Restore configured MQTT broker settings
  PageArgument  args;
  AutoConnectAux& mqtt_setting = *portal.aux(AUX_SETTING_URI);
  loadParams(mqtt_setting, args);

  Serial.print("WiFi ");
  if (portal.begin()) {
    Serial.println("connected:" + WiFi.SSID());
    Serial.println("IP:" + WiFi.localIP().toString());
    Serial.print("mDNS responder ");
    if (MDNS.begin(GET_HOSTNAME())) {
      MDNS.addService("http", "tcp", 80);
      Serial.println("started");
    }
    else
      Serial.println("setting up failed");
  }
  else {
    Serial.println("connection failed:" + String(WiFi.status()));
    Serial.println("Needs WiFi connection to start publishing messages");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && publishInterval > 0) {
    if (reconnect) {
      // Attempts to reconnect with the broker do not involve a delay.
      // The reconnect interval is realized as the measurement of elapsed
      // time using the millis function.
      if (millis() - lastAttempt > attemptInterval) {
        reconnect = !mqttConnect();
        lastAttempt = millis();
        if (++retry >= 3) {
          retry = 0;
          reconnect = false;
        }
      }
    }
    else {
      // It is not the delay function that produces the publish interval.
      // Using delay inside a loop function is deprecated for web server
      // sketches. It blocks HTTP request replies.
      // The publish interval is guaranteed by measuring the elapsed time.
      if (millis() - lastPub > publishInterval) {
        String  topic = String("channels/") + channelid.value + String("/publish/") + apikey.value;
        String  message = String("field1=") + String(getStrength(7));
        mqttPublish(topic, message);
        lastPub = millis();
      }
      mqttClient.loop();
    }
  }

#ifdef ARDUINO_ARCH_ESP8266
  MDNS.update();
#endif
  portal.handleClient();
}
