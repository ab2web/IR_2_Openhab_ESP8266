# IR_2_Openhab_ESP8266

 * IR_2_Openhab_ESP8266: receiving IR codes with IRrecv and send commands to openhab devices (toggle lights).
 * An IR detector/demodulator must be connected to the input RECV_PIN. 
 * Change Wlan-SSID and password and also set the right host (or ip-adress) and port of your openhab-server.
 * rename or add items in myItems[] array.
 * if you add items you've to adjust the integer arraySize to the number of values in your array.
 * Add ir-codes to the myIrCommands[] array. One for each item in myItems[], use the same order (myIrCommands[2] is according to myItems[2]).
 * To find out the codes of your remote control, turn on debug mode (#define DEBUG) and connect to serial monitor.
 * Take care of the right format. It has to be an hex value, you have to add "0x" in front of the code.
