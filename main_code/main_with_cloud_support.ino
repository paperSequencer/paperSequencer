//------------------------------------------------------------------------------------------------------------------------
// 
// Title: SD Card Wav Player With Mixing
//
// Description:
//    Simple example to demonstrate the fundamentals of mixing WAV files (digitized sound) from SPIFFS via the I2S 
//    interface of the ESP32. To keep this simple the WAVs must be stereo and 16bit samples. 
//    The Samples Per second can be anything. On the SD Card the wav file must be in root and called wav1_16.wav and
//    wav2_16.wav. wav1_16.wav will play repeatedly and wav2_16.wav will play when a designated pin on the ESP32
//    is grounded.
//    Libraries are available to play WAV's on ESP32, this code does not use these so that we can see what is happening.
//
//
//------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------
//
// Includes

#include "SD.h"                         // SD Card library, usually part of the standard install
#include "driver/i2s.h"                 // Library of I2S routines, comes with ESP32 standard install
#include <ESP32Servo.h>
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
//------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------
// Defines

// Volume control
#define POT_VOL_ANALOG_IN 14      // Pin that will connect to the middle pin of the potentiometer.
 
//    SD Card
#define SD_CS          5          // SD Card chip select
   
//    I2S
#define I2S_DOUT      25          // i2S Data out oin
#define I2S_BCLK      27          // Bit clock
#define I2S_LRC       26          // Left/Right clock, also known as Frame clock or word select
#define I2S_NUM       0           // i2s port number

// Wav File reading
#define NUM_BYTES_TO_READ_FROM_FILE 1024    // How many bytes to read from wav file at a time
#define stack_size 2048

//------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------
// structures and also variables

bool sensors[5];
int IR_pin1 = 35;
int IR_pin2 = 34;
int IR_pin3 = 32;
int IR_pin4 = 33;
int IR_pin5 = 22;
Servo myservo; 
int servoPin = 21;
int pushButton = 2;
//  I2S configuration

static const i2s_config_t i2s_config = 
{
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,                                 // Note, all files must be this
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // high interrupt priority
    .dma_buf_count = 8,                                   // 8 buffers
    .dma_buf_len = 256,                                   // 256 bytes per buffer, so 2K of buffer space
    .use_apll=0,
    .tx_desc_auto_clear= true, 
    .fixed_mclk=-1    
};

// These are the physical wiring connections to our I2S decoder board/chip from the esp32, there are other connections
// required for the chips mentioned at the top (but not to the ESP32), please visit the page mentioned at the top for
// further information regarding these other connections.

static const i2s_pin_config_t pin_config = 
{
    .bck_io_num = I2S_BCLK,                           // The bit clock connectiom, goes to pin 27 of ESP32
    .ws_io_num = I2S_LRC,                             // Word select, also known as word select or left right clock
    .data_out_num = I2S_DOUT,                         // Data out from the ESP32, connect to DIN on 38357A
    .data_in_num = I2S_PIN_NO_CHANGE                  // we are not interested in I2S data into the ESP32
};

struct WavHeader_Struct
{
    //   RIFF Section    
    char RIFFSectionID[4];      // Letters "RIFF"
    uint32_t Size;              // Size of entire file less 8
    char RiffFormat[4];         // Letters "WAVE"
    
    //   Format Section    
    char FormatSectionID[4];    // letters "fmt"
    uint32_t FormatSize;        // Size of format section less 8
    uint16_t FormatID;          // 1=uncompressed PCM
    uint16_t NumChannels;       // 1=mono,2=stereo
    uint32_t SampleRate;        // 44100, 16000, 8000 etc.
    uint32_t ByteRate;          // =SampleRate * Channels * (BitsPerSample/8)
    uint16_t BlockAlign;        // =Channels * (BitsPerSample/8)
    uint16_t BitsPerSample;     // 8,16,24 or 32

    // Data Section
    char DataSectionID[4];      // The letters "data"
    uint32_t DataSize;          // Size of the data that follows
};

// The data for one particular wav file
struct Wav_Struct
{
    File WavFile;                               // Object for accessing the opened wavfile
    uint32_t DataSize;                          // Size of wav file data
    bool Playing = false;                         // Is file playing
    bool Repeat;                                // If true, when wav ends, it will auto start again
    byte Samples[NUM_BYTES_TO_READ_FROM_FILE];  // Buffer to store data red from file
    uint32_t TotalBytesRead=0;                  // Number of bytes read from file so far
    uint16_t LastNumBytesRead;                  // Num bytes actually read from the wav file which will either be
                                                // NUM_BYTES_TO_READ_FROM_FILE or less than this if we are very
                                                // near the end of the file. i.e. we can't read beyond the file.
};
//------------------------------------------------------------------------------------------------------------------------

//  Global Variables/objects    
    
static const i2s_port_t i2s_num = I2S_NUM_0;  // i2s port number   
Wav_Struct Wav1;                              // Main Wave to play
Wav_Struct Wav2;
Wav_Struct Wav3;
Wav_Struct Wav4; 
Wav_Struct Wav5;                             // Secondary "short" wav 
float Volume = 1;                              // Volume
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

//------------------------------------------------------------------------------------------------------------------------

// HTML webpage

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

//------------------------------------------------------------------------------------------------------------------------

void setup() {
    sensors[0] = false;
    sensors[1] = false;
    sensors[2] = false;
    sensors[3] = false;
    sensors[4] = false;  
    Serial.begin(115200);                               // Used for info/debug
    pinMode(IR_pin1,INPUT);
    pinMode(IR_pin2,INPUT);
    pinMode(IR_pin3,INPUT);
    pinMode(IR_pin4,INPUT);
    pinMode(IR_pin5,INPUT);
    SDCardInit();
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(i2s_num, &pin_config);
    if(InitWavFiles() == false)
        while(true);                                   // If a problem terminate program   
    Wav1.Repeat = true;                                  // Wav1 will auto repeat
    Wav1.Playing = sensors[0];                           // We set wav1 to play comtinuously 
    Wav2.Repeat = true;                                  // Wav2 will auto repeat
    Wav2.Playing = sensors[1];                           // We set wav2 to play comtinuously
    Wav3.Repeat = true;
    Wav3.Playing = sensors[2];
    Wav4.Repeat = true;
    Wav4.Playing = sensors[3];
    Wav5.Repeat = true;
    Wav5.Playing = sensors[4];
    myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object
	// using default min/max of 1000us and 2000us
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
}

void loop()
{   
    PlayWavs();  // Have to keep calling this to keep the wav file playing
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
        request->send(200);}, handleUpload1);
  server->on("/upload2", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);}, handleUpload2);
  server->on("/upload3", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);}, handleUpload3);
  server->on("/upload4", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200); }, handleUpload4);
  server->on("/upload5", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200); }, handleUpload5);
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

void spin(){
    myservo.write(85);
}

int findMax(int a, int b, int c, int d, int e) {
    int maxNum = a;
    if (b > maxNum) {
        maxNum = b;
    }
    if (c > maxNum) {
        maxNum = c;
    }
    if (d > maxNum) {
        maxNum = d;
    }
    if (e > maxNum) {
        maxNum = e;
    }
    return maxNum;
}


void getSensors(){
    int IRvalue1 = digitalRead(IR_pin1);
    int IRvalue2 = digitalRead(IR_pin2);
    int IRvalue3 = digitalRead(IR_pin3);
    int IRvalue4 = digitalRead(IR_pin4);
    int IRvalue5 = digitalRead(IR_pin5);
    int counter = 0;
    if (IRvalue1 == HIGH){//second to the left
        counter++;
        sensors[0] = true;
    }
    if(IRvalue2 == HIGH){//first on the left
        counter++;
        sensors[1] = true;
    }
    if(IRvalue3 == HIGH){//middle
        counter++;
        sensors[2] = true;
    }
    if(IRvalue4 == HIGH){//middle
        counter++;
        sensors[3] = true;
    }
    if(IRvalue5 == HIGH){//middle
        counter++;
        sensors[4] = true;
    }
   // Serial.println(counter);
}


void PlayWavs()
{
    getSensors();
    Wav1.Playing = sensors[0];
    Wav2.Playing = sensors[1];
    Wav3.Playing = sensors[2];
    Wav4.Playing = sensors[3];
    Wav5.Playing = sensors[4];

    static bool ReadingFile = true;                       // True if reading files from SD. false if filling I2S buffer
    static byte Samples[NUM_BYTES_TO_READ_FROM_FILE];   // Memory allocated to store the data read in from the wav files
    static uint16_t BytesReadFromFile;                  // Max Num bytes actually read from the wav files which will either be
                                                        // NUM_BYTES_TO_READ_FROM_FILE or less than this if we are very
                                                        // near the end of all files.

    Volume=1;//float(analogRead(POT_VOL_ANALOG_IN))/2047;   // You possibly don't need to sample volume this often, perhaps every 1/10 sec would be fine
    if(ReadingFile)                                     // Read next chunk of data in from files 
    {
        ReadFiles();                                      // Read data into the wavs own buffers
        BytesReadFromFile = MixWavs(Samples);                       // Mix the samples together and store in the samples buffer
        ReadingFile = false;                                // Switch to sending the buffer to the I2S
    }
    else
    ReadingFile = FillI2SBuffer(Samples,BytesReadFromFile);   // We keep calling this routine until it returns true, at which point
                                                                // this will swap us back to Reading the next block of data from the file.
                                                                // Reading true means it has managed to push all the data to the I2S 
                                                                // Handler, false means there still more to do and you should call this
                                                                // routine again and again until it returns true.
    sensors[0] = false;
    sensors[1] = false;
    sensors[2] = false;
    sensors[3] = false;
    sensors[4] = false;
}

uint16_t MixWavs(byte* Samples)
{
    // Mix all playing wavs together, returns the max bytes that are in the buffer, usually this would be the full buffer but
    // in rare cases wavs may be close to the end of the file and thus not fill the entire buffer 

    uint16_t Wav1Idx,Wav2Idx,Wav3Idx,Wav4Idx,Wav5Idx;                           // Index into the wavs sample data
    int16_t Sample;                                     // The mixed sample
    uint16_t i;                                         // index into main samples buffer
    uint16_t MaxBytesInBuffer;                          // Max bytes of data in buffer, most of time buffer will be full
    
    Wav1Idx = 0;
    Wav2Idx = 0;
    Wav3Idx = 0;
    Wav4Idx = 0;
    Wav5Idx = 0;

    while((Wav1Idx < Wav1.LastNumBytesRead) | (Wav2Idx < Wav2.LastNumBytesRead) | (Wav3Idx < Wav3.LastNumBytesRead) | (Wav4Idx < Wav4.LastNumBytesRead) | (Wav5Idx < Wav5.LastNumBytesRead))
    {
        Sample = 0;
        if(Wav1.Playing)
            Sample = *((int16_t *)(Wav1.Samples+Wav1Idx));
        if(Wav2.Playing)
            Sample += *((int16_t *)(Wav2.Samples+Wav2Idx));    // This does the actual mix, just add togther
        if(Wav3.Playing)
            Sample += *((int16_t *)(Wav3.Samples+Wav3Idx));
        if(Wav4.Playing)
            Sample += *((int16_t *)(Wav4.Samples+Wav4Idx));
        if(Wav5.Playing)
            Sample += *((int16_t *)(Wav5.Samples+Wav5Idx));
        
        *((int16_t *)(Samples+i)) = Sample;
        Wav1Idx += 2;
        Wav2Idx += 2;
        Wav3Idx += 2;
        Wav4Idx += 2;
        Wav5Idx += 2;
        i += 2;
    }
    MaxBytesInBuffer = findMax(Wav1Idx,Wav2Idx,Wav3Idx,Wav4Idx,Wav5Idx);
        
    // We now alter the data according to the volume control
    for(i = 0; i < MaxBytesInBuffer; i += 2)  // We step 2 bytes at a time as we're using 16bits per channel
        *((int16_t *)(Samples + i)) = (*((int16_t *)(Samples + i)))*1;//*Volume; 

    return MaxBytesInBuffer;
}


bool InitWavFiles()
{
    // initialise wav files 
    // only bother trying to load this if first loads ok
    if(LoadWavFileHeader("/harp_16_16.wav", &Wav1)){
        if(LoadWavFileHeader("/bongo_16_16.wav", &Wav2)){
            if( LoadWavFileHeader("/guitar_16_16.wav", &Wav3)){
                if( LoadWavFileHeader("/piano_16_16.wav", &Wav4)){
                    return LoadWavFileHeader("/flute_16_16.wav", &Wav5);
                }
            }
        }
    }         
    return false;  
}


void ReadFiles()
{
  // Read in all files samples into their buffers 
    if(Wav1.Playing)
        ReadFile(&Wav1);
    if(Wav2.Playing)
        ReadFile(&Wav2);
    if(Wav3.Playing)
        ReadFile(&Wav3);
    if(Wav4.Playing)
        ReadFile(&Wav4);
    if(Wav5.Playing)
        ReadFile(&Wav5);
}

void ReadFile(Wav_Struct *Wav)
{
    uint16_t i;                                         // loop counter
    int16_t SignedSample;                               // Single Signed Sample
    float Volume = 1;               
    
    if(Wav->TotalBytesRead + NUM_BYTES_TO_READ_FROM_FILE > Wav->DataSize)   // If next read will go past the end then adjust the 
        Wav->LastNumBytesRead = Wav->DataSize-Wav->TotalBytesRead;                    // amount to read to whatever is remaining to read
    else
        Wav->LastNumBytesRead = NUM_BYTES_TO_READ_FROM_FILE;                          // Default to max to read
      
    Wav->WavFile.read(Wav->Samples, Wav->LastNumBytesRead);                  // Read in the bytes from the file
    Wav->TotalBytesRead += Wav->LastNumBytesRead;                        // Update the total bytes red in so far
    
    if(Wav->TotalBytesRead >= Wav->DataSize)              // Have we read in all the data?
    {
        if(Wav->Repeat){
            Wav->WavFile.seek(44);                                 // Reset to start of wav data  
            Wav->TotalBytesRead = 0;                         // Clear to no bytes read in so far 
        } 
        else
            Wav->Playing = false;                                    // Flag that wav has completed
    }
}

bool LoadWavFileHeader(String FileName, Wav_Struct* Wav)
{
    // Load wav file, if all goes ok returns true else false
    WavHeader_Struct WavHeader;                   

    Wav->WavFile = SD.open(FileName);                  // Open the wav file
    if(Wav->WavFile == false){
        Serial.println("Could not open :");
        Serial.println(FileName);
        return false;
    }
    else {
        Wav->WavFile.read((byte *) &WavHeader,44);        // Read in the WAV header, which is first 44 bytes of the file. 
                                                        // We have to typecast to bytes for the "read" function
        if(ValidWavData(&WavHeader))
        {
            DumpWAVHeader(&WavHeader);                      // Dump the header data to serial, optional!
            Serial.println();
            Wav->DataSize=WavHeader.DataSize;                    // Copy the data size into our wav structure
            return true;
        }
        else
            return false;
    }
}



bool FillI2SBuffer(byte* Samples,uint16_t BytesInBuffer)
{
    // Writes bytes to buffer, returns true if all bytes sent else false, keeps track itself of how many left
    // to write, so just keep calling this routine until returns true to know they've all been written, then
    // you can re-fill the buffer
    
    size_t BytesWritten;                        // Returned by the I2S write routine, 
    static uint16_t BufferIdx = 0;                // Current pos of buffer to output next
    uint8_t* DataPtr;                           // Point to next data to send to I2S
    uint16_t BytesToSend;                       // Number of bytes to send to I2S
    
    // To make the code eaier to understand I'm using to variables to some calculations, normally I'd write this calcs
    // directly into the line of code where they belong, but this make it easier to understand what's happening
    
    DataPtr = Samples + BufferIdx;                               // Set address to next byte in buffer to send out
    BytesToSend = BytesInBuffer - BufferIdx;                     // This is amount to send (total less what we've already sent)
    i2s_write(i2s_num,DataPtr, BytesToSend, &BytesWritten, 1);  // Send the bytes, wait 1 RTOS tick to complete
    BufferIdx += BytesWritten;                                 // increasue by number of bytes actually written
    
    if(BufferIdx >= BytesInBuffer) {
        // sent out all bytes in buffer, reset and return true to indicate this
        BufferIdx = 0; 
        return true;                             
    }
    else
        return false;       // Still more data to send to I2S so return false to indicate this
}

void SDCardInit()
{        
    pinMode(SD_CS, OUTPUT); 
    digitalWrite(SD_CS, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)
    if(!SD.begin(SD_CS)) {
        Serial.println("Error talking to SD card!");
        while(true);                  // end program
    }
}

bool ValidWavData(WavHeader_Struct* Wav)
{
    if(memcmp(Wav->RIFFSectionID,"RIFF",4) != 0) {    
        Serial.println("Invalid data - Not RIFF format");
        return false;        
    }
    if(memcmp(Wav->RiffFormat,"WAVE",4) != 0) {
        Serial.println("Invalid data - Not Wave file");
        return false;           
    }
    if(memcmp(Wav->FormatSectionID,"fmt",3) != 0) {
        Serial.println("Invalid data - No format section found");
        return false;       
    }
    if(memcmp(Wav->DataSectionID,"data",4) != 0) {
        Serial.println("Invalid data - data section not found");
        return false;      
    }
    if(Wav->FormatID != 1) {
        Serial.println("Invalid data - format Id must be 1");
        return false;                          
    }
    if(Wav->FormatSize != 16) {
        Serial.println("Invalid data - format section size must be 16.");
        return false;                          
    }
    if((Wav->NumChannels != 1)&(Wav->NumChannels != 2)){
        Serial.println("Invalid data - only mono or stereo permitted.");
        return false;   
    }
    if(Wav->SampleRate>48000) {
        Serial.println("Invalid data - Sample rate cannot be greater than 48000");
        return false;                       
    }
    if((Wav->BitsPerSample != 8)& (Wav->BitsPerSample != 16)) {
        Serial.println("Invalid data - Only 8 or 16 bits per sample permitted.");
        return false;                        
    }
    return true;
}


void DumpWAVHeader(WavHeader_Struct* Wav)
{
    if(memcmp(Wav->RIFFSectionID,"RIFF",4) != 0) {
        Serial.println("Not a RIFF format file - ");    
        PrintData(Wav->RIFFSectionID,4);
        return;
    } 
    if(memcmp(Wav->RiffFormat,"WAVE",4) != 0) {
        Serial.println("Not a WAVE file - ");  
        PrintData(Wav->RiffFormat,4);  
        return;
    }  
    if(memcmp(Wav->FormatSectionID,"fmt",3) != 0) {
        Serial.println("fmt ID not present - ");
        PrintData(Wav->FormatSectionID,3);      
        return;
    } 
    if(memcmp(Wav->DataSectionID,"data",4) != 0) {
        Serial.println("data ID not present - "); 
        PrintData(Wav->DataSectionID,4);
        return;
    }  
    // All looks good, dump the data
    Serial.println("Total size :");Serial.println(Wav->Size);
    Serial.println("Format section size :");Serial.println(Wav->FormatSize);
    Serial.println("Wave format :");Serial.println(Wav->FormatID);
    Serial.println("Channels :");Serial.println(Wav->NumChannels);
    Serial.println("Sample Rate :");Serial.println(Wav->SampleRate);
    Serial.println("Byte Rate :");Serial.println(Wav->ByteRate);
    Serial.println("Block Align :");Serial.println(Wav->BlockAlign);
    Serial.println("Bits Per Sample :");Serial.println(Wav->BitsPerSample);
    Serial.println("Data Size :");Serial.println(Wav->DataSize);
}

void PrintData(const char* Data,uint8_t NumBytes)
{
    for(uint8_t i = 0; i < NumBytes; i++)
      Serial.println(Data[i]); 
      Serial.println();  
}
