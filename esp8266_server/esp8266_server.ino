#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

const char* ssid = "ssid";
const char* password =  "password";
float temp = 0, light = 0, ph = 0, tss = 0, tds = 0;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1500);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Waiting to connect...");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_index); //Handle Index page
  server.on("/update", update_data); //Handle AJAX calls
  server.begin();
  Serial.println("Server listening");
}

void loop() {
  server.handleClient(); //Handling of incoming client requests
}

void update_data() {
  StaticJsonDocument<512> doc;

  // Sending request
  doc["type"] = "request";
  serializeJson(doc,Serial);

  // Reading response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) {
    if(Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  DeserializationError error = deserializeJson(doc,message);
  if(error) {
    Serial.print(F("ESP: deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  
  // Prepare HTTP response
  temp = doc["temp"];
  light = doc["light"];
  ph = doc["ph"];
  tss = doc["tss"];
  tds = doc["tds"];
  String output = "<h2>Temperature: " + String(temp) + " C</h2>";
  output += "<h2>Light: " + String(light) + " lx</h2>";
  output += "<h2>pH: " + String(ph) + "</h2>";
  output += "<h2>TSS: " + String(tss) + " NTU</h2>";
  output += "<h2>TDS: " + String(tds) + " ppm</h2>";
  server.send(200,"text/html",output);
  Serial.readString();
}

void handle_index() {
  StaticJsonDocument<512> doc;

  // Sending request
  doc["type"] = "request";
  serializeJson(doc,Serial);

  // Reading response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) {
    if(Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  DeserializationError error = deserializeJson(doc,message);
  if(error) {
    Serial.print(F("ESP: deserializeJson() failed: "));
    Serial.println(error.c_str());
  }

  // Prepare HTTP response
  temp = doc["temp"];
  light = doc["light"];
  ph = doc["ph"];
  tss = doc["tss"];
  tds = doc["tds"];

  String output = "<!DOCTYPE html>";
  output += "<html lang=\"en\">";
  output += "<head>";
      output += "<meta charset=\"UTF-8\">";
      output += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">";
      output += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
      output += "<title>Water Quality</title>";
  output += "</head>";
  output += "<body>";
    output += "<h1>Water quality:</h1>";
    output += "<div id=\"data\">";
      output += "<h2>Temperature: " + String(temp) + " C</h2>";
      output += "<h2>Light: " + String(light) + " lx</h2>";
      output += "<h2>pH: " + String(ph) + "</h2>";
      output += "<h2>TSS: " + String(tss) + " NTU</h2>";
      output += "<h2>TDS: " + String(tds) + " ppm</h2>";
    output += "</div>";
    output += "<script>";
      output += "setInterval(function () { getData(); }, 2000);";
      output += "function getData() {";
        output += "var xhttp = new XMLHttpRequest();";
        output += "xhttp.onreadystatechange = function () {";
          output += "if (this.readyState == 4 && this.status == 200) {";
            output += "document.getElementById(\"data\").innerHTML = this.responseText;}};";
        output += "xhttp.open(\"GET\", \"update\", true);";
        output += "xhttp.send();}";
  output += "</script>";
  output += "</body>";
  output += "</html>";

  server.send(200,"text/html",output);
  Serial.readString();
}
