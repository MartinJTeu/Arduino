/*  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will output the GPIO state(s) and set a GPIO pin depending on the request.
 *   http://server_ip/gpio2=0 will set the GPIO2 low,
 *   http://server_ip/gpio2=1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be
 *  printed to Serial when the module is connected
 *  hostname will work fine too, of course 
 */

#include <ESP8266WiFi.h>
// wifi_set_macaddr() needs "user_interface.h"
/*
extern "C" {
#include "user_interface.h"
}
*/

const char* ssid = "your-ssid";
const char* password = "your-password";

// create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

  // change MAC address (I needed to...)
//  uint8_t mac[6] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//  wifi_set_macaddr(STATION_IF, mac);
  
  // connect to WiFi network
  Serial.print("\nConnecting to "); Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  char i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 30) {
    delay(500);
    Serial.print(".");
  }
  if(i == 31){Serial.print("\nCould not connect to "); Serial.println(ssid); delay(500);}
//  Serial.printf("\nConnection status: %d\n", WiFi.status());

  // print the IP address
  Serial.print("\nConnected, IP address: ");
  Serial.println(WiFi.localIP());
  
  // start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // wait until the client sends some data
  Serial.println("\nNew client connected");
  while(!client.available()){
    delay(1);
  }
  
  // read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.print("Client request: "); Serial.println(req);
//  client.flush();

  // read current state
  char state = digitalRead(2);
  // print to serial
  Serial.print("GPIO2 state is "); Serial.println((state)?"HIGH":"LOW");
    
  // send the current GPIO state back to client
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n<div>GPIO2 state is ";
  response += (state)?"HIGH":"LOW";
  response += "</div>\n";
  
  // match the request
  if (req.indexOf("/gpio2=") != -1){
    char val;
    if (req.indexOf("/gpio2=0") != -1) val = 0;
    else if (req.indexOf("/gpio2=1") != -1) val = 1;
    else {
      Serial.println("invalid request");
      client.stop();
      return;
    }
    // set GPIO2 according to the request
    digitalWrite(2, val);
    // print to serial
    Serial.print("GPIO2 set to "); Serial.println((val)?"HIGH":"LOW");
    // create response to client
    response += "<div>GPIO2 set to ";
    response += (val)?"HIGH":"LOW";
    response += "</div>\n";
  }
  response += "</html>";
  // send response to the client
  client.print(response); delay(1);
  
  // the client will actually be disconnected when the function returns and 'client' object is destroyed
  client.flush();
  Serial.println("Client disconnected");
}
