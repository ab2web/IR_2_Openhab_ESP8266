/*
 * IR_2_Openhab_ESP8266: receiving IR codes with IRrecv and send commands to openhab devices (toggle lights).
 * An IR detector/demodulator must be connected to the input RECV_PIN. 
 * Change Wlan-SSID and password and also set the right host (or ip-adress) and port of your openhab-server.
 * rename or add items in myItems[] array.
 * if you add items you've to adjust the integer arraySize to the number of values in your array.
 * Add ir-codes to the myIrCommands[] array. One for each item in myItems[], use the same order (myIrCommands[2] is according to myItems[2]).
 * To find out the codes of your remote control, turn on debug mode (#define DEBUG) and connect to serial monitor.
 * Take care of the right format. It has to be an hex value, you have to add "0x" in front of the code.
 */
#include <IRremoteESP8266.h>
#include <ESP8266WiFi.h>

//#define DEBUG //auskommentieren um Debuggen zu deaktivieren.

#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTHEX(x)     Serial.print (x, HEX)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTHEX(x)
 #define DEBUG_PRINTLN(x) 
#endif


int status = WL_IDLE_STATUS;     // the Wifi radio's status
int RECV_PIN = 2; //an IR detector/demodulatord is connected to GPIO pin 2
char ssid[] = "FRITZ!Box.......";     //  your network SSID (name) 
char pass[] = "*************";  // your network password
const char* host = "odroid";
const int httpPort = 8080;
String getRestRequest = "GET http://"+(String)host+":"+(String)httpPort+"/rest/items/";
String cmdRequest =  "GET http://"+(String)host+":"+(String)httpPort+"/CMD?"; 

unsigned long previousMillis = 0;
const long irReadInterval = 100;

const int arraySize = 4; //number of items in myItems-Array and myIrCommands
int wantedpos = -1;
//todo: hashmap or workaround with struct would be better
String myItems[arraySize] = {"Lights_GF_Kitchen","Lights_GF_Lunchroom", "Lights_GF_Entry", "Lights_GF_LivingRoom"};
long unsigned int myIrCommands[arraySize] ={0xE0E020DF,0xE0E0A05F,0xE0E0609F,0xE0E010EF};

// Use WiFiClient class to create TCP connections
WiFiClient client;

IRrecv irrecv(RECV_PIN);

decode_results results;

//#############################################################################
//#_________________setup_____________________________________________________#
void setup()
{
  Serial.begin(115200);
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    DEBUG_PRINTLN("WiFi shield not present"); 
    // don't continue:
    while(true);
  }
  DEBUG_PRINTLN("Hello World");
  irrecv.enableIRIn(); // Start the receiver

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    DEBUG_PRINT("Attempting to connect to WPA SSID: ");
    DEBUG_PRINTLN(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection:
    delay(5000);
  }
     
  // you're connected now, so print out the data: 
  DEBUG_PRINT("You're connected to the network");
   printWifiData();
   printCurrentNet();
}//############################################################################




//#############################################################################
//#_________________main loop_________________________________________________#
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= irReadInterval) {
  previousMillis = currentMillis;
    if (irrecv.decode(&results)) {
      DEBUG_PRINTHEX(results.value);
      DEBUG_PRINTLN();
      irrecv.resume(); // Receive the next value
    }
  }
  //delay(100);

  if(results.value != 0X000000){
  for (int i=0; i<arraySize; i++) {
    if (results.value == myIrCommands[i]) {
      wantedpos = i;
      DEBUG_PRINT("wantedpos: ");
      DEBUG_PRINTLN(wantedpos);
      break;
    }
  }
  }
   
  if(wantedpos != -1){
    DEBUG_PRINT("connecting to ");
    DEBUG_PRINTLN(host);
  
    if (!client.connect(host, httpPort)) {
      DEBUG_PRINTLN("connection failed");
      return;
    }

    String itemResult;
    itemResult = getItemState(myItems[wantedpos]);
    setItem(myItems[wantedpos],itemResult);
  
    //reset IR-Result and wantedpos
    results.value = 0X000000;
    wantedpos = -1;
  
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("closing connection");
  }
}//############################################################################



//#############################################################################
//#_____________String getItemState___________________________________________#
String getItemState(String _ItemName){
  String getStateRequest = getRestRequest + _ItemName + "/state\r\n"; 
  String _resultLine;
  client.print(getStateRequest);
  delay (100);

  while(client.available()){
    _resultLine = client.readStringUntil('\r');
  }
  if(!client.connected()){
    if (!client.connect(host, httpPort)) {
      DEBUG_PRINTLN("connection failed");
      return "Error: Connection failed.";
    }
  }
  DEBUG_PRINT("Item state is: ");
  DEBUG_PRINTLN(_resultLine);
  return _resultLine;
}//############################################################################



//#############################################################################
//#_____________void setItem__________________________________________________#
void setItem(String _itemName, String _itemState){
  String setItemRequest;
  if (_itemState=="ON"){
    setItemRequest = cmdRequest+_itemName+"=OFF\r\n"; 
  }
  else if(_itemState=="100"){
    setItemRequest = cmdRequest+_itemName+"=0\r\n";
  }
  else if(_itemState=="OFF"){
    setItemRequest = cmdRequest+_itemName+"=ON\r\n";
  }
  else if(_itemState=="0"){
    setItemRequest = cmdRequest+_itemName+"=100\r\n";
  }
  else{
    //unknow _itemState
    DEBUG_PRINTLN("Error: Unknow item state.");
    return;
  }
  
  client.print(setItemRequest);
  DEBUG_PRINTLN(setItemRequest);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
  }
}//############################################################################



//#############################################################################
//#_____________void printWifiData____________________________________________#
void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
    DEBUG_PRINT("IP Address: ");
  DEBUG_PRINTLN(ip);
  DEBUG_PRINTLN(ip);
  
  // print your MAC address:
  byte mac[6];  
  WiFi.macAddress(mac);
  DEBUG_PRINT("MAC address: ");
  DEBUG_PRINTHEX(mac[5]);
  DEBUG_PRINT(":");
  DEBUG_PRINTHEX(mac[4]);
  DEBUG_PRINT(":");
  DEBUG_PRINTHEX(mac[3]);
  DEBUG_PRINT(":");
  DEBUG_PRINTHEX(mac[2]);
  DEBUG_PRINT(":");
  DEBUG_PRINTHEX(mac[1]);
  DEBUG_PRINT(":");
  DEBUG_PRINTHEX(mac[0]);
  DEBUG_PRINTLN();
}//############################################################################




//#############################################################################
//#_____________void printCurrentNet__________________________________________#
void printCurrentNet() {
  // print the SSID of the network you're attached to:
  DEBUG_PRINT("SSID: ");
  DEBUG_PRINTLN(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  uint8_t bssid;
  WiFi.BSSID(bssid);    
  DEBUG_PRINT("BSSID: ");
  DEBUG_PRINTHEX(bssid);
//  DEBUG_PRINT(":");
//  DEBUG_PRINTHEX(bssid[4]);
//  DEBUG_PRINT(":");
//  DEBUG_PRINTHEX(bssid[3]);
//  DEBUG_PRINT(":");
//  DEBUG_PRINTHEX(bssid[2]);
//  DEBUG_PRINT(":");
//  DEBUG_PRINTHEX(bssid[1]);
//  DEBUG_PRINT(":");
//  DEBUG_PRINTHEX(bssid[0]);
//  DEBUG_PRINTLN();

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  DEBUG_PRINT("signal strength (RSSI):");
  DEBUG_PRINTLN(rssi);

//  // print the encryption type:
//  byte encryption = WiFi.encryptionType();
//  DEBUG_PRINT("Encryption Type:");
//  DEBUG_PRINTHEX(encryption);
//  DEBUG_PRINTLN();
}//############################################################################
