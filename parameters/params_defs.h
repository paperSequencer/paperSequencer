const String ssid = "Paper Sequencer";
const String password = "123456789";
const int default_webserverporthttp = 80;
#define POT_VOL_ANALOG_IN 35      // Pin that will connect to the middle pin of the potentiometer.
//    SD Card
#define SD_CS          5          // SD Card chip select
#define I2S_DOUT      25          // i2S Data out oin
#define I2S_BCLK      27          // Bit clock
#define I2S_LRC       26          // Left/Right clock, also known as Frame clock or word select
#define I2S_NUM       0           // i2s port number
#define NUM_BYTES_TO_READ_FROM_FILE 1024    // How many bytes to read from wav file at a time
#define stack_size 2048
int IR_pin1 = 14;
int IR_pin2 = 34;
int IR_pin3 = 32;
int IR_pin4 = 33;
int IR_pin5 = 22;

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

// configuration structure
struct Config {
  String ssid;               // wifi ssid
  String wifipassword;       // wifi password
  int webserverporthttp;     // http port number for web admin
};



static const i2s_port_t i2s_num = I2S_NUM_0;  // i2s port number   
Wav_Struct Wav1;                              // Main Wave to play
Wav_Struct Wav2;
Wav_Struct Wav3;
Wav_Struct Wav4; 
Wav_Struct Wav5;                             // Secondary "short" wav 
float Volume;                              // Volume

int previousSwitchState = LOW;
bool isSwitchOn = false;

int PinState1 = LOW;    // Initial state of the switch
int previousPinState1 = LOW;
int PinState2 = LOW;    // Initial state of the switch
int previousPinState2 = LOW;
int PinState3 = LOW;    // Initial state of the switch
int previousPinState3 = LOW;
int PinState4 = LOW;    // Initial state of the switch
int previousPinState4 = LOW;
int PinState5 = LOW;    // Initial state of the switch
int previousPinState5 = LOW;



// variables
Config config;                        // configuration
AsyncWebServer *server;               // initialise webserver
