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

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].arrived = myData.arrived;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].arrived);
  Serial.println();
}



boolean ServerArray[5] = {false, false, false, false, false};


 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);

   WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: "); Serial.println(WiFi.localIP());

  Serial.println("id1 arrived? ");
  Serial.println(ArrivedArray[0]);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  
  esp_now_register_recv_cb(OnDataRecv);

  

  
}



 
void loop() {

While(
   if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
  for (int i = 0; i < 5; i++){
    if(boardsStruct[i].arrived == true){
      if(ServerArray[i] = false){
        
        
        
        }
      ServerArray[i] = true;
      Serial.println("Inside for loop");
      Serial.println(ServerArray[i]);
      
    }
  }

      //int box_id = 420;
      //int cooluser_id = 69;
      //http.addHeader("Content-Type", "application/json");
      //int httpResponseCode = http.POST("[{\"timestamp\":\"2021-07-03 16:21:12.357246\",\"coolbox_id\":"+String(box_id)+",\"cooluser_id\":"+String(cooluser_id)+"}]");
      
      http.addHeader("Content-Type", "text/plain");
      int httpResponseCode = http.POST("coolflex_spotted");
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      http.end();
    }
    else {
      Serial.println("no WiFi connection");
    }
    lastTime = millis();
  }
  delay(1000);  
}
