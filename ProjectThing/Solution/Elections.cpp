

// Ex09.cpp/.ino
// adapting access point and web server to allow connection of a device to an
// arbitrary wifi network
#ifdef ARDUINO_ARCH_SAMD
#include <WiFi101.h>
#elif defined ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#error Wrong platform
#endif 
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "Thing.h"
#include <WifiLocation.h>
#include "private.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "config.h"

//API KEY FOR GOOGLE AUTH
const char* googleApiKey = "AIzaSyD-7-j12Dhv3n2IezWMfmi1Tj-YRsEweEQ";
//LOCATION
WifiLocation location(googleApiKey);


String ssid = "";
String key = "";

// tweet via IFTTT
void setClock(); // we need the time to check certif expiry
void doPOST(String, String); // HTTPS POST
void iftttTweet(String); // IFTTT hookup
String doGET(String); // IFTTT hookup

unsigned long lastTime = 0;
unsigned long timerDelay = 600000;
String sensorReadings;
int httpResponseCode;

void setup09() { // initialisation
  // in previous exercises I've chained all the setups, but now things are
  // getting more complex I've amalgamated all the setup code so far here...

  Serial.begin(115200); // initialise the serial line
  getMAC(MAC_ADDRESS);  // store the MAC address as a chip identifier
  pinMode(BUILTIN_LED, OUTPUT);       // set up GPIO pin for built-in LED

  dln(startupDBG, "\nsetup09...");
  blink(5);                           // blink the on-board LED to say "hi"

  firstSliceMillis = millis();        // remember when we began
  lastSliceMillis = firstSliceMillis; // an approximation to use in 1st loop

  startAP();            // fire up the AP...
  initWebServer();      // ...and the web server
  WiFi.begin();         // for when credentials are already stored by board
  blink(5);             // blink the on-board LED to say "bye"
}

const char *templatePage[] = {    // we'll use Ex07 templating to build pages
  "<html><head><title>",                                                //  0
  "default title",                                                      //  1
  "</title>\n",                                                         //  2
  "<meta charset='utf-8'>",                                             //  3
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
  "<style>body{background:#FFF; color: #000; font-family: sans-serif;", //  4
  "font-size: 150%;}</style>\n",                                        //  5
  "</head><body>\n",                                                    //  6
  "<h2>Welcome to Thing!</h2>\n",                                       //  7
  "<!-- page payload goes here... -->\n",                               //  8
  "<!-- ...and/or here... -->\n",                                       //  9
  "\n<p><a href='/'>Home</a>&nbsp;&nbsp;&nbsp;</p>\n",                  // 10
  "</body></html>\n\n",                                                 // 11
};

void loop09() { // the workhorse
  webServer.handleClient(); // serve pending web requests every loop
}

void initWebServer() { // changed naming conventions to avoid clash with Ex06
  // register callbacks to handle different paths
  webServer.on("/", hndlRoot);              // slash
  webServer.onNotFound(hndlNotFound);       // 404s...
  webServer.on("/generate_204", hndlRoot);  // Android captive portal support
  webServer.on("/L0", hndlRoot);            // TODO is this...
  webServer.on("/L2", hndlRoot);            // ...IoS captive portal...
  webServer.on("/ALL", hndlRoot);           // ...stuff?
  webServer.on("/wifi", hndlWifi);          // page for choosing an AP
  webServer.on("/wifichz", hndlWifichz);    // landing page for AP form submit
  webServer.on("/status", hndlStatus);      // status check, e.g. IP address
  webServer.on("/election", getElections);
  webServer.on("/result", getResults);

  webServer.begin();
  dln(startupDBG, "HTTP server started");
}

// webserver handler callbacks
void hndlNotFound() {
  dbg(netDBG, "URI Not Found: ");
  dln(netDBG, webServer.uri());
  webServer.send(200, "text/plain", "URI Not Found");
}
void hndlRoot() {
  dln(netDBG, "serving page notionally at /");
  replacement_t repls[] = { // the elements to replace in the boilerplate
    {  1, apSSID.c_str() },
    {  8, "<p>Choose a <a href=\"wifi\">wifi access point</a>.</p>"  },
    {  9, "<p>Check <a href='/status'>wifi status</a>.</p>"},
    { 10, "<p>Check <a href='/election'>Start Elections</a>.</p>" },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
  webServer.send(200, "text/html", htmlPage);
}
void hndlWifi() {
  dln(netDBG, "serving page at /wifi");

  String form = ""; // a form for choosing an access point and entering key
  apListForm(form);
  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Network configuration</h2>\n" },
    { 8, "" },
    { 9, form.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07

  webServer.send(200, "text/html", htmlPage);
}
void hndlWifichz() {
  dln(netDBG, "serving page at /wifichz");

  String title = "<h2>Joining wifi network...</h2>";
  String message = "<p>Check <a href='/status'>wifi status</a>.</p>";
  for(uint8_t i = 0; i < webServer.args(); i++ ) {
    if(webServer.argName(i) == "ssid")
      ssid = webServer.arg(i);
    else if(webServer.argName(i) == "key")
      key = webServer.arg(i);
  }

  if(ssid == "") {
    message = "<h2>Ooops, no SSID...?</h2>\n<p>Looks like a bug :-(</p>";
  } else {
    char ssidchars[ssid.length()+1];
    char keychars[key.length()+1];
    ssid.toCharArray(ssidchars, ssid.length()+1);
    key.toCharArray(keychars, key.length()+1);
    WiFi.begin(ssidchars, keychars);
  }

  replacement_t repls[] = { // the elements to replace in the template
    { 1, apSSID.c_str() },
    { 7, title.c_str() },
    { 8, "" },
    { 9, message.c_str() },
  };
  String htmlPage = "";     // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls);

  webServer.send(200, "text/html", htmlPage);
}
void hndlStatus() {         // UI for checking connectivity etc.
  dln(netDBG, "serving page at /status");
  String s = "";
  s += "<ul>\n";
  s += "\n<li>SSID: ";
  s += WiFi.SSID();
  s += "</li>";
  s += "\n<li>Status: ";
  switch(WiFi.status()) {
    case WL_IDLE_STATUS:
      s += "WL_IDLE_STATUS</li>"; break;
    case WL_NO_SSID_AVAIL:
      s += "WL_NO_SSID_AVAIL</li>"; break;
    case WL_SCAN_COMPLETED:
      s += "WL_SCAN_COMPLETED</li>"; break;
    case WL_CONNECTED:
      s += "WL_CONNECTED</li>"; break;
    case WL_CONNECT_FAILED:
      s += "WL_CONNECT_FAILED</li>"; break;
    case WL_CONNECTION_LOST:
      s += "WL_CONNECTION_LOST</li>"; break;
    case WL_DISCONNECTED:
      s += "WL_DISCONNECTED</li>"; break;
    default:
      s += "unknown</li>";
  }

  s += "\n<li>Local IP: ";     s += ip2str(WiFi.localIP());
  s += "</li>\n";
  s += "\n<li>Soft AP IP: ";   s += ip2str(WiFi.softAPIP());
  s += "</li>\n";
  s += "\n<li>AP SSID name: "; s += apSSID;
  s += "</li>\n";

  s += "</ul></p>";

  replacement_t repls[] = { // the elements to replace in the boilerplate
    { 1, apSSID.c_str() },
    { 7, "<h2>Status</h2>\n" },
    { 8, "" },
    { 9, s.c_str() },
  };
  String htmlPage = ""; // a String to hold the resultant page
  GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
  webServer.send(200, "text/html", htmlPage);
}

void apListForm(String& f) { // utility to create a form for choosing AP
  const char *checked = " checked";
  int n = WiFi.scanNetworks();
  dbg(netDBG, "scan done: ");

  if(n == 0) {
    dln(netDBG, "no networks found");
    f += "No wifi access points found :-( ";
    f += "<a href='/'>Back</a><br/><a href='/wifi'>Try again?</a></p>\n";
  } else {
    dbg(netDBG, n); dln(netDBG, " networks found");
    f += "<p>Wifi access points available:</p>\n"
         "<p><form method='POST' action='wifichz'> ";
    for(int i = 0; i < n; ++i) {
      f.concat("<input type='radio' name='ssid' value='");
      f.concat(WiFi.SSID(i));
      f.concat("'");
      f.concat(checked);
      f.concat(">");
      f.concat(WiFi.SSID(i));
      f.concat(" (");
      f.concat(WiFi.RSSI(i));
      f.concat(" dBm)");
      f.concat("<br/>\n");
      checked = "";
    }
    f += "<br/>Pass key: <input type='textarea' name='key'><br/><br/> ";
    f += "<input type='submit' value='Submit'></form></p>";
  }
}

String ip2str(IPAddress address) { // utility for printing IP addresses
  return
    String(address[0]) + "." + String(address[1]) + "." +
    String(address[2]) + "." + String(address[3]);
}

void getElections(){
  
  dln(netDBG, "serving page at /elections");
  switch(WiFi.status()) {
     case WL_CONNECTED:
      location_t loc = location.getGeoFromWiFi();
      Serial.println("Location request data");
      Serial.println(location.getSurroundingWiFiJson());
      //Getting IoT Device Location
      Serial.println("Latitude: " + String(loc.lat, 7));
      Serial.println("Longitude: " + String(loc.lon, 7));
      Serial.println("Accuracy: " + String(loc.accuracy));
      //Inserting Details as points on the election page
      String s = "";
      s += "<ul>\n";
      s += "\n<li>Latitude: ";     s += String(loc.lat, 7);
      s += "</li>\n";
      s += "\n<li>Longitude: ";     s += String(loc.lon, 7);
      s += "</li>\n";
      s += "\n<li>Accuracy: ";     s += String(loc.accuracy);
      s += "</li>\n";
      s += "</ul></p>";
      replacement_t repls[] = { // the elements to replace in the boilerplate
        { 1, apSSID.c_str() },
        { 7, "<h2>Election</h2>\n" },
        { 8, "<p>Get <a href='/result'>Results</a>.</p>" },
        { 9, s.c_str() },
      };
      String tweet = "Lattitude:";
      iftttTweet(tweet);
      String htmlPage = ""; // a String to hold the resultant page
      GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
      webServer.send(200, "text/html", htmlPage);
      
  }
  
}

void getResults(){
  dln(netDBG, "serving page at /results");
  String url(
    // 2021 version: /api/v2/{username}/feeds/{feed_key}
    "https://io.adafruit.com/api/v2/likiths/feeds/esp-32/data" 
  );
  do {
  switch(WiFi.status()) {
     case WL_CONNECTED:
      Serial.println("URL: "+url);
      //GET Method from AdaFruit IO
      sensorReadings = doGET(url);
      //GET the JSON Objects
      JSONVar myObjects = JSON.parse(sensorReadings);
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObjects) == "undefined") {
        Serial.println("Parsing input failed!");
      }
     Serial.print("JSON object = ");
     Serial.println(myObjects);
     String s = "";
     s += "<ul>\n";
     for (int i = 0; i < myObjects.length(); i++){
         //Getting  data from each of the array of JSON objects
         JSONVar myObject  = myObjects[i];
         //Search by Value
         Serial.println("Keys OBJECT: "+ JSON.stringify(myObject["value"]));
         //Insert Value as a Bullet Point in result page
         s += "\n<li>Vote: ";
         s += JSON.stringify(myObject["value"]);
         s += "</li>\n";  
          //sensorReadingsArr[i] = double(value);
      }
     replacement_t repls[] = { // the elements to replace in the boilerplate
        { 1, apSSID.c_str() },
        { 7, "<h2>Results</h2>\n" },
        { 8, "" },
        { 9, s.c_str() },
      };
      String htmlPage = ""; // a String to hold the resultant page
      GET_HTML(htmlPage, templatePage, repls); // GET_HTML sneakily added to Ex07
      webServer.send(200, "text/html", htmlPage);
    }
    lastTime = millis();
  } 
while (millis() - lastTime > timerDelay);
}

/////////////////////////////////////////////////////////////////////////////
// talk to IFTTT ////////////////////////////////////////////////////////////
void iftttTweet(
  String tweet
) {
  // create a string describing the gesture
   char body[1124];
  
  /* twitter duplicate results seem stricter now... */
   //sprintf(
     //body, "{ \"value1\": \"%f\", \"value2\": \"%f\", \"value3\": \"%f\" }\n", latitude , longitude , accuracy
   //);
   sprintf(body, "{ \"value1\": \"%s\" }\n", tweet);
   Serial.printf("posting %s\n", body);

   String url(
    // 2019 version: "https://maker.ifttt.com/trigger/waving-at-twitter/with/key/"
    "https://maker.ifttt.com/trigger/com3505-ifttt-2021/with/key/" _IFTTT_KEY
  );
  //doPOST(url, String(body));
}

void doPOST(String url, String body) {
  char *iftttRootCertif = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n";

  HTTPClient https;
  if(https.begin(url, iftttRootCertif)) { // HTTPS
    https.addHeader("User-Agent", "ESP32");
    https.addHeader("Content-Type", "application/json");

    Serial.printf("[HTTPS] POST %.62s...\n", url.c_str());
    Serial.printf("    len = %.6d\n", strlen(body.c_str()));
    //Serial.println(iftttRootCertif);

    // start connection and send HTTP header
    int httpCode = https.POST((uint8_t *) body.c_str(), strlen(body.c_str()));

    // httpCode will be negative on error
    if(httpCode > 0) {
      // HTTP header has been send and response header has been handled
      Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

      // file found at server
      if(
        httpCode == HTTP_CODE_OK ||
        httpCode == HTTP_CODE_MOVED_PERMANENTLY
      ) {
        String payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf(
        "[HTTPS] POST... failed, error: %s\n",
        https.errorToString(httpCode).c_str()
      );
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
} // doPOST()

//GET From Ada Fruit IO
String doGET(String url) {
  String payload = "{}"; 
  HTTPClient https;
  https.begin(url); // HTTPS
  https.addHeader("User-Agent", "ESP32");
  https.addHeader("Content-Type", "application/json");
  httpResponseCode= https.GET();
  //If there is Response Code it gets the value else returns empty string
  if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = https.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  https.end();
  return payload;
} // doGET()
