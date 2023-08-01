#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
const String default_ssid = "Yazan";
const String default_wifipassword = "123456789";
const String ssid = "MR.Paper";
const String password = "123456789";
const int default_webserverporthttp = 80;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<!DOCTYPE html>
<html>
<head>
  <title>Paper Sequencer</title>
  <style>
    .heading-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      background-color: white;
      border: 3px solid black;
    }

    .heading-container h1, .heading-container h2 {
      color: black;
      margin: 0px;
      margin-top: 10px;
    }
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-image: url("pic1.jpeg");
      background-size: cover;
      background-repeat: no-repeat;
    }

    h3 {
      color: black;
      text-align: center;
      margin-top: 20px;
    }

    p {
      color: white;
      text-align: center;
      font-weight: bold;
    }

    .file-upload {
      text-align: center;
      margin-top: 20px;
    }

    .file-upload input[type="file"] {
      display: none;
    }

    .file-upload label {
      background-color: black;
      color: #ffffff;
      padding: 10px 20px;
      font-size: 16px;
      cursor: pointer;
    }

    .submit-button {
      display: block;
      margin: 0 auto;
      margin-top: 10px;
      padding: 15px 30px;
      font-size: 18px;
      background-color: black;
      color: #ffffff;
      border: none;
      cursor: pointer;
    }
    
  </style>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<body>
  <div class="heading-container">
  <h1>Paper Sequencer - Song Selection</h1>
  <h3>The Paper Sequencer is an innovative project comprising of five synchronized disks that produce captivating sounds upon encountering the color black.</h3>
  </div>
  <p>Please upload a WAV file for each of the five disks:</p>
  <form method="POST" action="/upload1" enctype="multipart/form-data"><input type="file" name="data" accept=".wav"/><input type="submit" name="upload" value="Upload First file" title="Upload File"></form>
  <form method="POST" action="/upload2" enctype="multipart/form-data"><input type="file" name="data" accept=".wav"/><input type="submit" name="upload" value="Upload Second file" title="Upload File"></form>
  <form method="POST" action="/upload3" enctype="multipart/form-data"><input type="file" name="data" accept=".wav"/><input type="submit" name="upload" value="Upload Third file" title="Upload File"></form>
  <form method="POST" action="/upload4" enctype="multipart/form-data"><input type="file" name="data" accept=".wav"/><input type="submit" name="upload" value="Upload Fourth file" title="Upload File"></form>
  <form method="POST" action="/upload5" enctype="multipart/form-data"><input type="file" name="data" accept=".wav"/><input type="submit" name="upload" value="Upload Fifth file" title="Upload File"></form>
  <p>%FILELIST%</p>
</body>
</html>
)rawliteral";

String listFiles(bool ishtml = false);

// configuration structure
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  int webserverporthttp;     // http port number for web admin
};

// variables
Config config;                        // configuration
AsyncWebServer *server;               // initialise webserver

void setup() {
  Serial.begin(115200);

  Serial.println("Booting ...");

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true)) {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    Serial.println("ERROR: Cannot mount SPIFFS, Rebooting");
    rebootESP("ERROR: Cannot mount SPIFFS, Rebooting");
  }

  Serial.print("SD Free: "); Serial.println(humanReadableSize((SD.totalBytes() - SD.usedBytes())));
  Serial.print("SD Used: "); Serial.println(humanReadableSize(SD.usedBytes()));
  Serial.print("SD Total: "); Serial.println(humanReadableSize(SD.totalBytes()));

  Serial.println(listFiles());

  Serial.println("\nLoading Configuration ...");

  config.ssid = default_ssid;
  config.wifipassword = default_wifipassword;
  config.webserverporthttp = default_webserverporthttp;

  Serial.print("\nConnecting to Wifi: ");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  //WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(500);
  //  Serial.print(".");
  //}


  Serial.println("\n\nNetwork Configuration:");
  Serial.println("----------------------");
  Serial.print("         SSID: "); Serial.println(WiFi.SSID());
  Serial.print("  Wifi Status: "); Serial.println(WiFi.status());
  Serial.print("Wifi Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  Serial.print("          MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("           IP: "); Serial.println(WiFi.localIP());
  Serial.print("       Subnet: "); Serial.println(WiFi.subnetMask());
  Serial.print("      Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("        DNS 1: "); Serial.println(WiFi.dnsIP(0));
  Serial.print("        DNS 2: "); Serial.println(WiFi.dnsIP(1));
  Serial.print("        DNS 3: "); Serial.println(WiFi.dnsIP(2));

  // configure web server
  Serial.println("\nConfiguring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();

  // startup web server
  Serial.println("Starting Webserver ...");
  server->begin();

  pinMode(5, OUTPUT); 
    digitalWrite(5, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)
    if(!SD.begin(5)) {
        Serial.println("Error talking to SD card!");
        while(true);                  // end program
    }
}

void loop() {
}

void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on SPIFFS");
  File root = SD.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
    } else {
      returnText += "File: " + String(foundfile.name()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void configureWebServer() {
  server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();
    Serial.println(logmessage);
    request->send_P(200, "text/html", index_html, processor);
  });

  // run handleUpload function when any file is uploaded
  server->on("/upload1", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload1);
  server->on("/upload2", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload2);
  server->on("/upload3", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload3);
  server->on("/upload4", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload4);
  server->on("/upload5", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
      }, handleUpload5);
}

// handles uploads
void handleUpload1(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    if(SD.exists("/test1.wav")){
        SD.remove("/test1.wav"); 
     }
    request->_tempFile = SD.open("/test1.wav" , "w");

    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}
void handleUpload2(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    if(SD.exists("/test2.wav")){
        SD.remove("/test2.wav"); 
     }
    request->_tempFile = SD.open("/test2.wav" , "w");

    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

void handleUpload3(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    if(SD.exists("/test3.wav")){
        SD.remove("/test3.wav"); 
     }
    request->_tempFile = SD.open("/test3.wav" , "w");

    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

void handleUpload4(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    if(SD.exists("/test4.wav")){
        SD.remove("/test4.wav"); 
     }
    request->_tempFile = SD.open("/test4.wav" , "w");

    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

void handleUpload5(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    if(SD.exists("/test5.wav")){
        SD.remove("/test5.wav"); 
     }
    request->_tempFile = SD.open("/test5.wav" , "w");

    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}





String processor(const String& var) {
  if (var == "FILELIST") {
    return listFiles(true);
  }
  if (var == "FREESD") {
    return humanReadableSize((SD.cardSize() - SD.usedBytes()));
  }

  if (var == "USEDSD") {
    return humanReadableSize(SD.usedBytes());
  }

  if (var == "TOTALSD") {
    return humanReadableSize(SD.cardSize());
  }

  return String();
}
