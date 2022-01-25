#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

const char* ssid = "ssid";
const char* password =  "password";
float temp = 0, light = 0, ph = 0, tss = 0, tds = 0;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
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
  serializeJson(doc, Serial);

  // Reading response
  boolean messageReady = false;
  String message = "";
  while (messageReady == false) {
    if (Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
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
  String output = "<h2>• Temperature: " + String(temp) + " °C</h2>";
  output += "<h2>• Acidity: " + String(ph) + " pH</h2>";
  output += "<h2>• TSS: " + String(tss) + " NTU</h2>";
  output += "<h2>• TDS: " + String(tds) + " ppm</h2>";
  output += "<h2>• Light: " + String(light) + " lx</h2>";
  server.send(200, "text/html", output);
  Serial.readString();
}

void handle_index() {
  StaticJsonDocument<512> doc;

  // Sending request
  doc["type"] = "request";
  serializeJson(doc, Serial);

  // Reading response
  boolean messageReady = false;
  String message = "";
  while (messageReady == false) {
    if (Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
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
  output += "<style>body {background-color: #769FCD;color: #F7FBFC;font-family: sans-serif;  }";
  output += "button {font-size: 20px;border-radius: 20px;color: #769FCD;font-family: Arial;background-color: #F7FBFC;text-decoration: none;";
  output += "display: inline-block;cursor: pointer;text-align: center;font-weight: bold;border: none;}";
  output += "p {max-width: 400px;}</style>";
  output += "</head>";
  output += "<body>";
  output += "<h1>Parameters:</h1>";
  output += "<div id=\"data\">";
  output += "<h2>• Temperature: " + String(temp) + " °C</h2>";
  output += "<h2>• Acidity: " + String(ph) + " pH</h2>";
  output += "<h2>• TSS: " + String(tss) + " NTU</h2>";
  output += "<h2>• TDS: " + String(tds) + " ppm</h2>";
  output += "<h2>• Light: " + String(light) + " lx</h2>";
  output += "</div>";
  output += "<button id=\"description_button\" onclick=\"showHide()\">Show description</button>";
  output += "<div id=\"description\" style=\"visibility: hidden;\">";
  output += "<h3>Parameters description:</h3>";
  output += "<h4>Temperature:</h4>";
  output += "<p> Using water that falls within the 17-22°C range will ensure your plants have enough dissolved oxygen.</p>";
  output += "<h4>Acidity:</h4>";
  output += "<p>Generally, the irrigation water should have a pH between 5.0 and 7.0, but a lot depends on the species of the plant.</p>";
  output += "<h4>Total suspended solids (TSS):</h4>";
  output += "<p>Ideally below 1 NTU.</p>";
  output += "<h4>Total dissolved solids (TDS):</h4>";
  output += "<p>The desirable range for irrigation water is 0-1000 ppm. Levels between 300-600 ppm are considered optimum for most plants.</p>";
  output += "<h4>Light:</h4>";
  output += "<p>500-2500 lx - Low light plants<br>2500-10000 lx - Medium light plants<br>10000-20000 lx - Bright light plants<br>20000-50000 lx - Very bright light plants</p>";
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
  output += "function showHide() {";
  output += "var description = document.getElementById(\"description\");";
  output += "var button = document.getElementById(\"description_button\");";
  output += "if (description.style.visibility === \"hidden\") {";
  output += "description.style.visibility = \"visible\";";
  output += "button.innerHTML = \"Hide description\";";
  output += " } else { description.style.visibility = \"hidden\";button.innerHTML = \"Show description\";}}";
  output += "</script>";
  output += "</body>";
  output += "</html>";

  server.send(200, "text/html", output);
  Serial.readString();
}
