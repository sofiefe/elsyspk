// Arduino Code for ESP32. The code recieves input via bluetooth and use it to control led strips.
// Code made for ESDA 2 Project for group Hiv3mind. 



#include "BluetoothSerial.h" //library used for bluetooth connection between mobile app and ESP32
#include <FastLED.h>    // library used to controll led strips
#include <esp_now.h>    // library used for ESP32 to ESP32 comunication
#include <WiFi.h>       // library used for ESP32 to ESP32 comunication

uint8_t broadcastAddress[] = {0x58, 0xbf, 0x25, 0x83, 0x1d, 0xac};  // MAC address to CoolBox ESP32

typedef struct struct_message {                                     // data sendt from CoolFlex to CoolBox
    int id; // must be unique for each sender board
    bool arrived;
} struct_message;


// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;


FASTLED_USING_NAMESPACE

// init Class:
BluetoothSerial ESP_BT; //initialize bluetooth class

#define led_pin_1 0   // data pin used to send control signal to first led strip
#define led_pin_2 2  // data pin used to send control signal to the second led strip
#define led_pin_3 15  // data pin used to send control signal to third led strip
#define led_pin_4 4   // data pin used to send control signal to fourth led strip

#define clock_pin_1 16 // data pin used to send control clock signal to first led strip
#define clock_pin_2 17 // data pin used to send control clock signal to second led strip
#define clock_pin_3 5 // data pin used to send control clock signal to first led strip
#define clock_pin_4 18 // data pin used to send control clock signal to second led strip


#define LED_TYPE    APA102 // type of ledstrip (used by fastLED library)
#define COLOR_ORDER GRB    // Colour order for fastLED
#define NUM_LEDS    72     // Number of individual leds in the ledstrip
CRGB leds1[NUM_LEDS];      // Array to store values for each led in the first led strip. 
CRGB leds2[NUM_LEDS];      // Array to store values for each led in the second led strip.
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];


#define BRIGHTNESS          255   //set Brightness
#define FRAMES_PER_SECOND   120   //set update frequency

char arrayTest[20];               //Array where input from bluetooth is parsed into individual numbers
String incomingString = "";       //String used to recieve the bluetooth data
char *strings[6];                 //array where each individual number from incomingString gets its own index. 
char *ptr = NULL;                 //Used in parsing of IncomingString
bool dataRecieved = false;        //boolean value for knowing if the ESP32 have recieved data or not
bool LEDstrip1 = false;           //Boolean value for knowing which LED strip one should change
bool LEDstrip2 = false;           //Boolean value for knowing which LED strip one should change
bool LEDstrip3 = false;           //Boolean value for knowing which LED strip one should change
bool LEDstrip4 = false;           //Boolean value for knowing which LED strip one should change


//----------------------------------------------------------------//  Code for inactivity handling

const unsigned short int LIGHT_SENSOR_PIN_1 = 32;                   // Declares the pin to be used for the input from the photocell
const unsigned short int LIGHT_SENSOR_PIN_2 = 33;
unsigned short int TILT_SENSOR_PINS = 36;                         // Declares all pins to be used for tilt sensors

unsigned short int TILT_SENSOR_CURRENT_LIST = 0;                  // Stores the current readings from the sensors in binary form
unsigned short int TILT_SENSOR_PREVIOUS_LIST = 0;                 // Stores the readings from the previous iteration in binary form
unsigned long int TILT_SENSOR_INACTIVITY_COUNTER = 0;             // Stores the counters for inactivity from all tilt sensors 
bool ACTIVE;
unsigned int DEACTIVATION_TIME = 50000;                           // Time of inactivity before lights turn off.

//----------------------------------------------------------------//


void setup() {
  Serial.begin(115200);
  ESP_BT.begin("ESP32_Control");  //Naming of ESP32, the name that will show up on mobile device (when connecting to bluetooth)


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

     
 // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }

   
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer (the CoolBox for the school you go to)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  myData.id = 1;                  // Should be equivalent for the given studen's ID in the Database
  myData.arrived = true;

  pinMode (led_pin_1, OUTPUT);    // data pin used to send control signal to led strip
  pinMode (led_pin_2, OUTPUT);    // data pin used to send control signal to second led strip
  pinMode (led_pin_3, OUTPUT);    // data pin used to send control signal to third led strip
  pinMode (led_pin_4, OUTPUT);    // data pin used to send control signal to fourth led strip
  pinMode (LIGHT_SENSOR_PIN_1, INPUT); // Input pin used to recieve data about light around the sytem
  pinMode (LIGHT_SENSOR_PIN_2, INPUT); // Input pin used to recieve data about light around the sytem
  pinMode (TILT_SENSOR_PINS, INPUT); // Input pin used to recieve data about movement of the sytem

  
  FastLED.addLeds<LED_TYPE,led_pin_1,clock_pin_1, COLOR_ORDER>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);   //Initialize ledstrip
  FastLED.addLeds<LED_TYPE,led_pin_2,clock_pin_2, COLOR_ORDER>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);   //Initialize second ledstrip
  FastLED.addLeds<LED_TYPE,led_pin_3,clock_pin_3, COLOR_ORDER>(leds3, NUM_LEDS).setCorrection(TypicalLEDStrip);   //Initialize third ledstrip
  FastLED.addLeds<LED_TYPE,led_pin_4,clock_pin_4, COLOR_ORDER>(leds4, NUM_LEDS).setCorrection(TypicalLEDStrip);   //Initialize fourth ledstrip

  FastLED.setBrightness(BRIGHTNESS);                                                                              // set brightness level of every led in the ledstrip
}

// List of available patterns.  Each is defined as a separate void function below loop().
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns1 = {test, sinelon, test1, sinelon_colour, rainbowWithGlitter, confetti, juggle, bpm, shutOff, setColour, test2};
SimplePatternList gPatterns2 = {test, sinelon, test1, sinelon_colour, rainbowWithGlitter, confetti, juggle, bpm, shutOff, setColour, test2};
SimplePatternList gPatterns3 = {test, sinelon, test1, sinelon_colour, rainbowWithGlitter, confetti, juggle, bpm, shutOff, setColour, test2};
SimplePatternList gPatterns4 = {test, sinelon, test1, sinelon_colour, rainbowWithGlitter, confetti, juggle, bpm, shutOff, setColour, test2};
uint8_t gCurrentPatternNumber1 = 0;  // Index number of which pattern is active for LED strip 1
uint8_t gCurrentPatternNumber2 = 0;  // Index number of which pattern is active for LED strip 2
uint8_t gCurrentPatternNumber3 = 0;  // Index number of which pattern is active for LED strip 3
uint8_t gCurrentPatternNumber4 = 0;  // Index number of which pattern is active for LED strip 4

uint8_t gHue = 0;                    // base colour
uint8_t varying_hue = 0;             // varying colour hue (with time)
uint8_t varying_hue_2 = 128;         // varying colour hue (with time) - different colour than varying_hue
bool pattern_sweep = false;          // boolean that determine if one should cycle through patterns or not

unsigned long light;

void loop() {
    
  int LIGHT_INTENSITY_1 = 10*analogRead(LIGHT_SENSOR_PIN_1);             // Current reading from the photocell
  int LIGHT_INTENSITY_2 = analogRead(LIGHT_SENSOR_PIN_2);             // Current reading from the photocell

 // Serial.println(ACTIVE);


  FastLED.show();                                 //Make fastled actually show the leds
  FastLED.delay(1000/FRAMES_PER_SECOND);          //delay in order to make the code not cycle through super fast

  // Periodic updates
  EVERY_N_MILLISECONDS( 20 ) { varying_hue++; varying_hue_2++;} // slowly change the hue colour, when it reach the maximum value of 255 it is reset.
  if(pattern_sweep == true){
    EVERY_N_SECONDS( 5 ) { nextPattern();}                      // change patterns periodically if the boolean pattern_sweep is set to true
  }

  EVERY_N_SECONDS(5){

    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
    } else {
    }
    
    // Send message via ESP-NOW
    if(true){
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
       
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      }
      else {
        Serial.println("Error sending the data");
      } 
    }  
  /*
  Serial.println("Light sensors: ");
  Serial.println(LIGHT_INTENSITY_1);
  Serial.println(LIGHT_INTENSITY_2);
  Serial.println("-----------------");  
  */
  }

  EVERY_N_MILLISECONDS(50){
    light += LIGHT_INTENSITY_1;
    light += LIGHT_INTENSITY_2;
    Serial.println(light);
  }

  if(LIGHT_INTENSITY_1 < 3000 || LIGHT_INTENSITY_2 < 3000){
    light = 0;
  }

/*
  if(LIGHT_INTENSITY_1 > 4000 || LIGHT_INTENSITY_2 > 4000){
      FastLED.setBrightness(0);                                                                              // set brightness level of every led in the ledstrip
  } else if((LIGHT_INTENSITY_1 < 4000 && LIGHT_INTENSITY_1 > 3000) || (LIGHT_INTENSITY_2 < 4000 && LIGHT_INTENSITY_2 > 3000)){
      FastLED.setBrightness(64);
  } else if((LIGHT_INTENSITY_1 < 3000 && LIGHT_INTENSITY_1 > 2000) || (LIGHT_INTENSITY_2 < 3000 && LIGHT_INTENSITY_2 > 2000)){
      FastLED.setBrightness(128);
  } else if((LIGHT_INTENSITY_1 < 2000 && LIGHT_INTENSITY_1 > 1000) || (LIGHT_INTENSITY_2 < 2000 && LIGHT_INTENSITY_2 > 1000)){
      FastLED.setBrightness(192);
  } else if((LIGHT_INTENSITY_1 < 1000 || (LIGHT_INTENSITY_2 < 1000))){
      FastLED.setBrightness(255);
  }
  */
  if (ACTIVE == true && light < 1600000) {   // Check if the sensor registers lights or not and if there is movement
    
  gPatterns1[gCurrentPatternNumber1]();           //ensures that current pattern are on. 
  gPatterns2[gCurrentPatternNumber2]();           //ensures that current pattern are on. 
  gPatterns3[gCurrentPatternNumber3]();           //ensures that current pattern are on. 
  gPatterns4[gCurrentPatternNumber4]();           //ensures that current pattern are on. 
  
    if (ESP_BT.available())                           //checks if there is a bluetooth connection
      {

        incomingString = ESP_BT.readString();         // set incomingString to whatever data is recieved from the ESP32 via bluetooth    
        if(incomingString.equals("")){  
          Serial.println("no recieved data from ESP32");
        }else{
          incomingString.toCharArray(arrayTest, 20);  // takes incomingString and converts it to a charArray.
  
        byte index = 0;                               // index used to determine the number of different numbers recieved from the ESP32 in one data transfer
        ptr = strtok(arrayTest, ",");                 // split arrayTest based on a given symbol (for this project, comma is used for the symbol)

        while(ptr != NULL ){                          // ptr is set to NULL if strtok() has reached the end
          strings[index] = ptr;                       // add each individual number to a new array 
          index++;                                    // incrementing index
          ptr = strtok(NULL, ",");                    // continuing splitting arrayTest based on the symbol(comma), this continues until the end of arrayTest, when ptr is set to NULL and the while loop ends.
        }


        for (int n = 0; n < index; n++){              // serial print test to check ESP32 recieves and parse the data correctly
          Serial.print(n);
          Serial.print("  ");
          Serial.println(strings[n]);
        } 

 

        int button = 0;                               // value used to determine which button was pressed in the app
        int value = 0;                                // value used to determine if the user wish to turn a given pattern on or off
        if(incomingString.equals("")){                // checks if testString has been set to recieved data from ESP32
        } else {
          
          if(atoi(strings[0])%10==1){
            Serial.println("LED strip 4 success: " + atoi(strings[0])%10);
            LEDstrip4 = true;
          } else {
            Serial.println("LED strip 4 fail: " + atoi(strings[0])%10);
            LEDstrip4 = false;
          }
          if((atoi(strings[0])/10U)%10 == 1){
            Serial.println("LED strip 3 success: " + (atoi(strings[0])/10U)%10);
            LEDstrip3 = true;
          } else {
            Serial.println("LED strip 3 fail: " + (atoi(strings[0])/10U)%10);
            LEDstrip3 = false;
          }
          
          if((atoi(strings[0])/100U)%10 == 1){
            Serial.println("LED strip 2 success: " + (atoi(strings[0])/100U)%10);
            LEDstrip2 = true;
          } else {
            Serial.println("LED strip 2 fail: " + (atoi(strings[0])/100U)%10);
            LEDstrip2 = false;
          }
          if((atoi(strings[0])/1000U)%10 == 1){
            Serial.println("LED strip 1 success: " + (atoi(strings[0])/1000U)%10);
            LEDstrip1 = true;
          }
          else {
            Serial.println("LED strip 1 fail: " + (atoi(strings[0])/1000U)%10);
            LEDstrip1 = false;
          }
            if(atoi(strings[1]) != 9){                // checks if the user want to set individual lights (button = 9) or use a predetirmined pattern (button < 9)
              button = atoi(strings[1])/10;           // 
              value = atoi(strings[1])%10;            //
            } else {
              button = 9;                             // user wants to set an individual led
              value = atoi(strings[2]);
            }
        }
          switch (button) {                             // Switch used to handle all the different user interactions
           
            case 1:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 10;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 10;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 10;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 10;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
             
              pattern_sweep = false;                    // turn off pattern_sweeping
              break;
              
            case 2:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 1;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 1;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 1;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 1;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
              
            case 3:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 2;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 2;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 2;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 2;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
      
            case 4:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 3;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 3;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 3;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 3;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;   
      
            case 5:  
              if(LEDstrip1 == true){  
               if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 4;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 4;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 4;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 4;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
      
      
            case 6:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 5;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 5;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 5;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 5;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
      
            case 7:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 6;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 6;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 6;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 6;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
      
            case 8:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 7;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 7;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 7;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 7;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;
              break;
      
            case 9:  
              if(LEDstrip1 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber1 = 9;              // set the current pattern
                } else {
                  gCurrentPatternNumber1 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip2 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber2 = 9;              // set the current pattern
                } else {
                  gCurrentPatternNumber2 = 8;              // turn the current pattern off  
                }
              }
              
              if(LEDstrip3 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber3 = 9;              // set the current pattern
                } else {
                  gCurrentPatternNumber3 = 8;              // turn the current pattern off  
                }
              }
              if(LEDstrip4 == true){  
                if(value == 1){                           // checks if the user want to turn the given pattern on or off
                  gCurrentPatternNumber4 = 9;              // set the current pattern
                } else {
                  gCurrentPatternNumber4 = 8;              // turn the current pattern off  
                }
              }
              
              pattern_sweep = false;   
              break;  
    
            case 10:  
              if(pattern_sweep == false){
                pattern_sweep = true;   
              } else{
                pattern_sweep = false;
              }
              break;   
          } 
        }
      }// It's dark, turn on lights
    }   else {
              for(int i = 0; i < NUM_LEDS; i++){
                leds1[i] =  CHSV(0, 0, 0); 
                leds2[i] =  CHSV(0, 0, 0); 
                leds3[i] =  CHSV(0, 0, 0); 
                leds4[i] =  CHSV(0, 0, 0); 
              } 
  }


    TILT_SENSOR_CURRENT_LIST = (round(analogRead(TILT_SENSOR_PINS)/4095));    // Reads all the tilt pins and reduces to binary results in a new list
    if (TILT_SENSOR_CURRENT_LIST != TILT_SENSOR_PREVIOUS_LIST) {              // Checks if the current result is the same as the last for each pin
          Serial.println("inactivity counter set to millis");
          TILT_SENSOR_INACTIVITY_COUNTER = millis();                                 // If the results are different, the counter is reset to the current time
          Serial.println(TILT_SENSOR_INACTIVITY_COUNTER);
          Serial.println(millis());
    }
    ACTIVE = false;
   // Serial.println(round((analogRead(TILT_SENSOR_PINS)/4095)));
    if (millis() - TILT_SENSOR_INACTIVITY_COUNTER < DEACTIVATION_TIME) {         // Checks if each timer has registered activity within the set timeframe
    //  Serial.println("bool set to active");
      ACTIVE = true;                                                                // If any sensors have had recent activity, the system stays active
    }
    TILT_SENSOR_PREVIOUS_LIST = TILT_SENSOR_CURRENT_LIST;                     // Writes the current list over the last, before the next iteration

//delay(500);


}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
  
// add one to the current pattern number, and wrap around at the end
void nextPattern(){
  if(gCurrentPatternNumber1 > 7){
  gCurrentPatternNumber1 = 1;
  } else {
   gCurrentPatternNumber1 = (gCurrentPatternNumber1 + 1);
  }
  if(gCurrentPatternNumber2 > 7){
   gCurrentPatternNumber2 = 1;
  } else {
   gCurrentPatternNumber2 = (gCurrentPatternNumber2 + 1);
  } 
}
  
// FastLED's built-in rainbow generator
void rainbow() {
  if(LEDstrip1 == true){
    fill_rainbow( leds1, NUM_LEDS, gHue, 7);
  }
  if(LEDstrip2 == true){
    fill_rainbow( leds2, NUM_LEDS, gHue, 7);
  } 
 
  if(LEDstrip3 == true){
    fill_rainbow( leds3, NUM_LEDS, gHue, 7);
  } 
  if(LEDstrip4 == true){
    fill_rainbow( leds4, NUM_LEDS, gHue, 7);
  }  
}

// built-in FastLED rainbow, plus some random sparkly glitter
void rainbowWithGlitter() {
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    if(LEDstrip1 == true){
        leds1[ random16(NUM_LEDS) ] += CRGB::White;
    }
    if(LEDstrip2 == true){
        leds2[ random16(NUM_LEDS) ] += CRGB::White;
    }
   
    if(LEDstrip3 == true){
        leds3[ random16(NUM_LEDS) ] += CRGB::White;
    } 
    if(LEDstrip4 == true){
        leds4[ random16(NUM_LEDS) ] += CRGB::White;
    }            
  }
}

// random colored speckles that blink in and fade smoothly
void confetti() {
    if(LEDstrip1 == true){ 
      fadeToBlackBy( leds1, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds1[pos] += CHSV( gHue + random8(64), 200, 255); 
    }
    if(LEDstrip2 == true){ 
      fadeToBlackBy( leds2, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds2[pos] += CHSV( gHue + random8(64), 200, 255); 
    }
    
    if(LEDstrip3 == true){ 
      fadeToBlackBy( leds3, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds3[pos] += CHSV( gHue + random8(64), 200, 255); 
    }
    if(LEDstrip4 == true){ 
      fadeToBlackBy( leds4, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds4[pos] += CHSV( gHue + random8(64), 200, 255); 
    }         
}

void sinelon(){
  // a colored dot sweeping back and forth, with fading trails
    if(LEDstrip1 == true){  
      fadeToBlackBy( leds1, NUM_LEDS, 20);
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds1[pos] += CHSV( gHue, 255, 192); 
    }
    if(LEDstrip2 == true){  
      fadeToBlackBy( leds2, NUM_LEDS, 20);
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds2[pos] += CHSV( gHue, 255, 192); 
    }
      
    if(LEDstrip3 == true){  
      fadeToBlackBy( leds3, NUM_LEDS, 20);
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds3[pos] += CHSV( gHue, 255, 192); 
    }
    if(LEDstrip4 == true){  
      fadeToBlackBy( leds4, NUM_LEDS, 20);
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds4[pos] += CHSV( gHue, 255, 192); 
    }   
}

// a colored dot sweeping back and forth, with fading trails
void sinelon_colour(){
    if(LEDstrip1 == true){  
      fadeToBlackBy( leds1, NUM_LEDS, 20); 
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds1[pos] += CHSV( varying_hue, 255, 192);
    }
    if(LEDstrip2 == true){  
      fadeToBlackBy( leds2, NUM_LEDS, 20); 
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds2[pos] += CHSV( varying_hue, 255, 192);
    }
    
    if(LEDstrip3 == true){  
      fadeToBlackBy( leds3, NUM_LEDS, 20); 
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds3[pos] += CHSV( varying_hue, 255, 192);
    }
    if(LEDstrip4 == true){  
      fadeToBlackBy( leds4, NUM_LEDS, 20); 
      int pos = beatsin16( 13, 0, NUM_LEDS-1 );
      leds4[pos] += CHSV( varying_hue, 255, 192);
    }    
}

// colored stripes pulsing at a defined Beats-Per-Minute (BPM)
void bpm(){
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    if(LEDstrip1 == true){  
      leds1[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    }
    if(LEDstrip2 == true){  
      leds2[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    }
    
    if(LEDstrip3 == true){  
      leds3[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    }
    if(LEDstrip4 == true){  
      leds4[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    }              
  }
}

// eight colored dots, weaving in and out of sync with each other
void juggle() {
    if(LEDstrip1 == true){ 
      fadeToBlackBy( leds1, NUM_LEDS, 20);
      uint8_t dothue = 0;
      for( int i = 0; i < 8; i++) {
        leds1[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32; 
    }
  }
    if(LEDstrip2 == true){ 
      fadeToBlackBy( leds2, NUM_LEDS, 20);
      uint8_t dothue = 0;
      for( int i = 0; i < 8; i++) {
        leds2[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32; 
    }
  }
  
    if(LEDstrip3 == true){ 
      fadeToBlackBy( leds3, NUM_LEDS, 20);
      uint8_t dothue = 0;
      for( int i = 0; i < 8; i++) {
        leds3[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32; 
    }
  }
    if(LEDstrip4 == true){ 
      fadeToBlackBy( leds4, NUM_LEDS, 20);
      uint8_t dothue = 0;
      for( int i = 0; i < 8; i++) {
        leds4[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32; 
    }
  } 
}

// set colour for an individual led
void setColour(){
  if(LEDstrip1 == true){ 
    leds1[atoi(strings[1])-1].r = atoi(strings[3]);
    leds1[atoi(strings[1])-1].g = atoi(strings[4]);
    leds1[atoi(strings[1])-1].b = atoi(strings[5]);    
  }
  if(LEDstrip2 == true){ 
    leds2[atoi(strings[1])-1].r = atoi(strings[3]);
    leds2[atoi(strings[1])-1].g = atoi(strings[4]);
    leds2[atoi(strings[1])-1].b = atoi(strings[5]);    
  }
  
  if(LEDstrip3 == true){ 
    leds3[atoi(strings[1])-1].r = atoi(strings[3]);
    leds3[atoi(strings[1])-1].g = atoi(strings[4]);
    leds3[atoi(strings[1])-1].b = atoi(strings[5]);    
  }
  if(LEDstrip4 == true){ 
    leds4[atoi(strings[1])-1].r = atoi(strings[3]);
    leds4[atoi(strings[1])-1].g = atoi(strings[4]);
    leds4[atoi(strings[1])-1].b = atoi(strings[5]);    
  } 
}

// test pattern, every other lights are on with a constant colour.
void test(){
}

// test pattern all leds on, varying colour
void test1(){
  if(LEDstrip1 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds1[i] =  CHSV(varying_hue, 255, 255); 
    } 
  }
  if(LEDstrip2 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds2[i] =  CHSV(varying_hue, 255, 255); 
    } 
  }
  
  if(LEDstrip3 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds3[i] =  CHSV(varying_hue, 255, 255); 
    } 
  }
  if(LEDstrip4 == true){ 
    for(int i = 0; i < NUM_LEDS; i++){
      leds4[i] =  CHSV(varying_hue, 255, 255); 
    } 
  }  
}

void test2(){
    if(LEDstrip1 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i+=2){
      leds1[i] = CHSV(varying_hue, 255, 255);
    }
    for(int j = 1; j < NUM_LEDS-1; j+=2){
      leds1[j] = CHSV(varying_hue_2, 255, 255);      
    }
  }
  if(LEDstrip2 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i+=2){
      leds2[i] = CHSV(varying_hue, 255, 255);
    }
    for(int j = 1; j < NUM_LEDS-1; j+=2){
      leds2[j] = CHSV(varying_hue_2, 255, 255);      
    }
  }
  
  if(LEDstrip3 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i+=2){
      leds3[i] = CHSV(varying_hue, 255, 255);
    }
    for(int j = 1; j < NUM_LEDS-1; j+=2){
      leds3[j] = CHSV(varying_hue_2, 255, 255);      
    }
  }
  if(LEDstrip4 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i+=2){
      leds4[i] = CHSV(varying_hue, 255, 255);
    }
    for(int j = 1; j < NUM_LEDS-1; j+=2){
      leds4[j] = CHSV(varying_hue_2, 255, 255);      
    }
  }  
}

// Shutt of pattern, set all leds to off
void shutOff(){
  if(LEDstrip1 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds1[i] =  CHSV(0, 0, 0); 
    }  
  }
  if(LEDstrip2 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds2[i] =  CHSV(0, 0, 0); 
    }      
  }
  
  if(LEDstrip3 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds3[i] =  CHSV(0, 0, 0); 
    }    
  }
  if(LEDstrip4 == true){ 
    for(int i = 0; i < NUM_LEDS-1; i++){
      leds4[i] =  CHSV(0, 0, 0); 
    }   
  }  
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
