#include <SPI.h>
#include <WiFiNINA.h> // ESP8266에서 Wi-Fi 기능을 사용하기 위한 라이브러리 입니다.
#include <PubSubClient.h>

//For pedometer
#include "SparkFunLSM6DS3.h"
#include "Wire.h"

#define CLEAR_STEP      true
#define NOT_CLEAR_STEP  false 

//Create a instance of class LSM6DS3
LSM6DS3 pedometer( I2C_MODE, 0x6A );  //I2C device address 0x6A

const char* ssid = "Roboin";
const char* password = "roboin1234";
const char* mqtt_server = "192.168.0.46";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
String stepCount_str;
char stepCount_char[50];
char bpm_char[50];
int value = 0;


#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// DI pin of LED ring
#define PIN            10
// number of pixels in the ring
#define NUMPIXELS     8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRBW + NEO_KHZ800);

/*
   The format of our output.

   Set this to PROCESSING_VISUALIZER if you're going to run
    the Processing Visualizer Sketch.
    See https://github.com/WorldFamousElectronics/PulseSensor_Amped_Processing_Visualizer

   Set this to SERIAL_PLOTTER if you're going to run
    the Arduino IDE's Serial Plotter.
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;

/*
   Pinout:
     PULSE_INPUT = Analog Input. Connected to the pulse sensor
      purple (signal) wire.
     PULSE_BLINK = digital Output. Connected to an LED (and 220 ohm resistor)
      that will flash on each detected pulse.
     PULSE_FADE = digital Output. PWM pin onnected to an LED (and resistor)
      that will smoothly fade with each pulse.
      NOTE: PULSE_FADE must be a pin that supports PWM.
       If USE_INTERRUPTS is true, Do not use pin 9 or 10 for PULSE_FADE,
       because those pins' PWM interferes with the sample timer.
*/
const int PULSE_INPUT = 0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int THRESHOLD = 530;   // Adjust this number to avoid noise when idle

/*
   samplesUntilReport = the number of samples remaining to read
   until we want to report a sample over the serial connection.

   We want to report a sample value over the serial port
   only once every 20 milliseconds (10 samples) to avoid
   doing Serial output faster than the Arduino can send.
*/
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password); // 앞서 설정한 ssid와 페스워드로 Wi-Fi에 연결합니다.
  while (WiFi.status() != WL_CONNECTED) { // 연결될 때 까지 0.5초 마다 Wi-Fi 연결상태를 확인합니다.
    delay(500);
  }
  randomSeed(micros()); // 렌덤 문자를 위한 렌덤 시드를 설정합니다.
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Topic에 메시지가 도착하면 실행됨
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "33IoTClient-"; // 클라이언트 ID를 설정합니다.
    clientId += String(random(0xffff), HEX); // 같은 이름을 가진 클라이언트가 발생하는것을 방지하기 위해, 렌덤 문자를 클라이언트 ID에 붙입니다.
    if (client.connect(clientId.c_str())) { // 앞서 설정한 클라이언트 ID로 연결합니다.
      // Publisher이기 때문에 Subscribe를 하지 않습니다.
    } else {
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883); // MQTT 서버에 연결합니다.
  client.setCallback(callback);
  Serial.begin(9600);   
  if( pedometer.begin() != 0 ){
    Serial.println("Device error");
  }
  else{
      Serial.println("Device OK!");
  }
  
  //Configure LSM6DS3 as pedometer 
  if( 0 != config_pedometer(NOT_CLEAR_STEP) )
  {
    Serial.println("Configure pedometer fail!");
  }
  Serial.println("Success to Configure pedometer!");
  pixels.begin(); // This initializes the NeoPixel library.


  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    /*
       PulseSensor initialization failed,
       likely because our Arduino platform interrupts
       aren't supported yet.

       If your Sketch hangs here, try changing USE_PS_INTERRUPT to false.
    */
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
    Serial.println("reconnect");
  }
  uint8_t dataByte = 0;
  uint16_t stepCount = 0;
  /*
     See if a sample is ready from the PulseSensor.

     If USE_INTERRUPTS is true, the PulseSensor Playground
     will automatically read and process samples from
     the PulseSensor.

     If USE_INTERRUPTS is false, this call to sawNewSample()
     will, if enough time has passed, read and process a
     sample (analog voltage) from the PulseSensor.
  */
  if (pulseSensor.sawNewSample()) {
    /*
       Every so often, send the latest Sample.
       We don't print every sample, because our baud rate
       won't support that much I/O.
    */
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      pulseSensor.outputSample();

      /*
         At about the beginning of every heartbeat,
         report the heart rate and inter-beat-interval.
      */
      if (pulseSensor.sawStartOfBeat()) {
        pulseSensor.outputBeat();
      }
   }

   int myBPM = pulseSensor.getBeatsPerMinute();
   int bpm = myBPM+170;
    
    int r,g,b,w;
    int max_bright = 100; //value of maximum brightness, max 255. But you don't always want it at max :)
    float dd = 20; //change in BPM between color tones (blue->green->yellow->pink->red)
    float t1 = 60, t2, t3, t4; //t1 - "base" BPM, lower than t1 would be blue
    t2 = t1 + dd;
    t3 = t2 + dd;
    t4 = t3 + dd;
    
    if(bpm < t1){ r = 0; g = 0; b = max_bright, w=0; }
    else if(bpm < t2) { r = 0; g = max_bright * (bpm-t1)/dd; b = max_bright - g, w=0; }
    else if(bpm < t3) { r = max_bright * (bpm-t2)/dd; g = max_bright - r; b = r/4, w=0; }
    else if(bpm < t4) { r = max_bright; g = 0; b = max_bright/2 - max_bright * (bpm-t3)/(2*dd), w=0; }
    else {r = max_bright; g = 0; b = 0, w=0; }
    
    if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
      r *= 0.01;
      g *= 0.01;
      b *= 0.01;
      w *= 0.01;
    }

    int on_pixels = (bpm+5)/20; //number of used LEDs: for 60 BPM, 6 LEDs will be on, for 120 - 12 etc
    for(int i=0;i<NUMPIXELS;i++)
    {
    if(i < on_pixels) pixels.setPixelColor(i, pixels.Color(r,g,b,w));
    else pixels.setPixelColor(i, pixels.Color(0,0,0,0)); //turn off all other LEDs
    }
    pixels.show();
    pedometer.readRegister(&dataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);
    stepCount = (dataByte << 8) & 0xFFFF;
    
    pedometer.readRegister(&dataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);
    stepCount |=  dataByte;
    
    Serial.print("Step: ");
    Serial.println(stepCount);
    Serial.print("BPM: ");                        // Print phrase "BPM: " 
    Serial.println(bpm);
    client.loop();
    int bpm_len = sprintf(bpm_char, "%d", bpm);
    int stepCount_len = sprintf(stepCount_char, "%d", stepCount); //converting 만보기 수 (the float variable above) to 문자열
    //stepCount_str.toCharArray(stepCount, stepCount_str.length() + 1); //packaging up the data to publish to mqtt whoa...
    client.publish("StepCount", stepCount_char); // inTopic 토픽으로 메시지를 전송합니다.
    client.publish("BPM", bpm_char);
  }
  /******
     Don't add code here, because it could slow the sampling
     from the PulseSensor.
  ******/
}



//Setup pedometer mode
int config_pedometer(bool clearStep)
{
  uint8_t errorAccumulator = 0;
  uint8_t dataToWrite = 0;  //Temporary variable

  //Setup the accelerometer******************************
  dataToWrite = 0; 
  
  //  dataToWrite |= LSM6DS3_ACC_GYRO_BW_XL_200Hz;
  dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
  dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_26Hz;

  
  // Step 1: Configure ODR-26Hz and FS-2g
  errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, dataToWrite);

  // Step 2: Set bit Zen_G, Yen_G, Xen_G, FUNC_EN, PEDO_RST_STEP(1 or 0)
  if(clearStep)
    errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3E);
  else
    errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3C);
  
  // Step 3:  Enable pedometer algorithm
  errorAccumulator += pedometer.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x40);
  
  //Step 4: Step Detector interrupt driven to INT1 pin, set bit INT1_FIFO_OVR
  errorAccumulator += pedometer.writeRegister( LSM6DS3_ACC_GYRO_INT1_CTRL, 0x10 );
  
  return errorAccumulator;
}
