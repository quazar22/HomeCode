#include "WifiConnect.h"

WebServer* SetupOTAServer(const char* ssid, const char* password) {
  WebServer* server = new WebServer(80);
  const char* host = "esp32";
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server->on("/", HTTP_GET, [server]() {
    server->sendHeader("Connection", "close");
    server->send(200, "text/html", loginIndex);
  });
  server->on("/serverIndex", HTTP_GET, [server]() {
    server->sendHeader("Connection", "close");
    server->send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server->on("/update", HTTP_POST, [server]() {
    server->sendHeader("Connection", "close");
    server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, [server]() {
    HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server->begin();

  return server;
}
