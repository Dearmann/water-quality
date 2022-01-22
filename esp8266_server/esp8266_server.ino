#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

const char* ssid = "ssid"; //Enter Wi-Fi SSID
const char* password =  "passwd"; //Enter Wi-Fi Password
 
void setup() {
  Serial.begin(9600); //Begin Serial at 115200 Baud
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  server.on("/", handle_index); //Handle Index page
  
  server.begin(); //Start the server
  Serial.println("Server listening");
}

void loop() {
  server.handleClient(); //Handling of incoming client requests
}

void handle_index() {
    // Send a JSON-formatted request with key "type" and value "request"
  // then parse the JSON-formatted response with keys "gas" and "distance"
  DynamicJsonDocument doc(512);
  float temp = 0, light = 0, ph = 0, tss = 0, tds = 0;
//  // Sending the request
  doc["type"] = "request";
  serializeJson(doc,Serial);
//  // Reading the response
  boolean messageReady = false;
  String message = "";
  while(messageReady == false) { // blocking but that's ok
    if(Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
//  // Attempt to deserialize the JSON-formatted message
  DeserializationError error = deserializeJson(doc,message);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  temp = doc["temp"];
  light = doc["light"];
  ph = doc["ph"];
  tss = doc["tss"];
  tds = doc["tds"];
  // Prepare the data for serving it over HTTP
  String output = "Temerature: " + String(temp) + "\n";
  output += "Light: " + String(light) + "\n";
  output += "pH: " + String(ph) + "\n";
  output += "TSS: " + String(tss) + "\n";
  output += "TDS: " + String(tds);
  // Serve the data as plain text, for example
  server.send(200,"text/plain",output);
  Serial.readString();
}
