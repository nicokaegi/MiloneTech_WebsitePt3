/*
   Web client sketch for IDE v1.0.1 and w5100/w5200
   Uses POST method.
   Posted November 2012 by SurferTim
*/
#include <b64.h>
#include <HttpClient.h>
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

//Change to your server domain
char serverName[] = "www.ptsv2.com";
// change to your server's port
int serverPort = 80;
// change to the page on that server
char pageName[] = "/t/sensor/post";

WiFiClient client;
char* data =  "There!";
// insure params is big enough to hold your variables
//char params[32];
char bodyBuff[420];

// set this to the number of milliseconds delay
// this is 30 seconds
#define delayMillis 5000UL

unsigned long thisMillis = 0;
unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  WiFi.setPins(8,7,4,2);
 if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);

  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");

  printWiFiStatus();

  Serial.println("\nStarting connection to server...");

}

void loop()
{
  thisMillis = millis();

  if(thisMillis - lastMillis > delayMillis)
  {
    lastMillis = thisMillis;
   // params must be url encoded.
   strcat(bodyBuff, "\r\n");
   strcat(bodyBuff, "{ \n");
   strcat(bodyBuff, "\"artists\" : [ \n");
   strcat(bodyBuff, "{ \n");
   strcat(bodyBuff, "\"artistname\" : \"Leonard Cohen\", \n");
   strcat(bodyBuff, "\"born\" : \"1934\"  \n");
   strcat(bodyBuff, "},  \n");
   strcat(bodyBuff, "{  \n");
   strcat(bodyBuff, "\"artistname\" : \"Joe Satriani\",  \n");
   strcat(bodyBuff, "\"born\" : \"1956\"  \n");
   strcat(bodyBuff, "},  \n");
   strcat(bodyBuff, "{  \n");
   strcat(bodyBuff, "\"artistname\" : \"Snoop Dogg\",  \n");
   strcat(bodyBuff, "\"born\" : \"1971\"  \n");
   strcat(bodyBuff, "}  \n");
   strcat(bodyBuff, "]  \n");
   strcat(bodyBuff, "}  \n");
    

//    strcpy(bodyBuff, "Ayo ");
//    strcat(bodyBuff, data);
//    strcat(bodyBuff, "\n");
//    strcat(bodyBuff, "Big booty bitches");
    
//    sprintf(params,"Hello " + data);
//    sprintf(params2,"Big booty bitches");
    
    if(!postPage(serverName,serverPort,pageName,bodyBuff)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));
  
  }    
}


byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[64];
  int stringLength;

  Serial.print(F("connecting..."));

  if(client.connect(domainBuffer,thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: text/plain"));
    client.println("User-Agent: Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500");
    stringLength = strlen(thisData);
    sprintf(outBuf,"Content-Length: %u\r\n",stringLength);
    client.println(outBuf);
    // send the body (variables)
    client.print(thisData);
  }
  else
  {
    Serial.println(F("failed"));
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      Serial.write(inChar);
      connectLoop = 0;
    }
    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }
  Serial.println();
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}
void printWiFiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
