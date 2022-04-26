// Arduino Code for ESP32. The code recieves data via the esp32_now library. This data include an ID which is then used to send 
// post-requests to a server.
// Code made for ESDA 2 Project for group CoolGroup.

#include <esp_now.h>
#include <HTTPClient.h>
#include <WiFi.h>

const char* ssid = "simons-nett";
const char* password = "zxqggocxsqcnk";
const char* serverName = "http://192.168.43.95:8000/data?="; 
//https://gruppe3.innovasjon.ed.ntnu.no/data?=
//http://192.168.43.95:8000/data?=

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

// Flag to indicate new data and send to server
bool SendtoServer = false; 
uint8_t mAddress[] = {0x78, 0xe3, 0x6d, 0x18, 0xe9, 0x4c};
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  bool arrived;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;
struct_message board5;

// Create an array with all the structures
struct_message boardsStruct[5] = {board1, board2, board3, board4, board5};
boolean ServerArray[5] = {false, false, true, true, true};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  memcpy(&myData, incomingData, sizeof(myData));
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].arrived = myData.arrived;
  for (int i = 0; i < 5; i++){
    if(boardsStruct[i].arrived == true){
      if(ServerArray[i] == false){
        ServerArray[i] = true;  
        Serial.print("User with user ID = ");
        Serial.print(i+1);
        Serial.println(" has arrived");             
        }
        else {
        Serial.print("User with ID = ");  
        Serial.print(i+1); 
        Serial.println( " is already registered as arrived");
        }
      //Serial.println(ServerArray[i]);   
    }
  }
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 

  //esp_now_set_pmk((uint8_t*)PMK_KEY_STR);

  esp_now_peer_info_t masterInfo;
  memcpy(masterInfo.peer_addr, mAddress, 6);
  masterInfo.channel = 0;
  masterInfo.encrypt = false;

  if(esp_now_add_peer(&masterInfo) != ESP_OK){
    Serial.println("There was an error registering our overlord");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info 
  esp_now_register_recv_cb(OnDataRecv); 
}

void loop() {
  
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  Serial.println("Connecting");
  float timer = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - timer < 5000) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: "); Serial.println(WiFi.localIP());
  if ((millis() - lastTime) > timerDelay) {

    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
  http.addHeader("Content-Type", "text/plain");
  for(int i = 0; i < sizeof(ServerArray); i++){
    if(ServerArray[i] == true){
      int box_id = 420;     
      String cooluser_id = String(i+1);
      int httpResponseCode = http.POST(cooluser_id);  
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);   
    }
  }
      http.end();
    }
    else {
      Serial.println("no WiFi connection");
    }
    lastTime = millis();
  }
  
  delay(1000); 
}
