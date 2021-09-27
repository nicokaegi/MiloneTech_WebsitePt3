

/*[
 Milone Wifi+LoRa Web Server
 Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500
 PRODUCT ID: 3010
 */
//#include <Time.h>
//#include <TimeLib.h>
#include <ArduinoJson.h>
#include <b64.h>
#include <HttpClient.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFi101.h>
#include <SD.h>
#include <RTCZero.h>
#include <time.h>
#include <RH_RF95.h>
#include <RHSPIDriver.h>

//#include <ArduinoLowPower.h>

#include "milone_logo.h"

#define FEATHER 1

#define MAX_RECORDS 16
#define MAX_CLIENTS 16
#define RECORD_TIMEOUT_SECS 43200
#define SERVER_RESTART_DURATION (60000*30)  //30 minutes in ms
#define delayMillis 5000UL // Added by Ethan A.

///* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

int led = LED_BUILTIN;
int status = WL_IDLE_STATUS;
//Change to your server domain
//char serverName[] = "usersmilonetech.com";
char serverName[] = "18.222.60.137"; //IP address of our server
// change to your server's port
int serverPort = 80; //Added by Ethan A.
// change to the page on that server
//Added by Ethan A.
char pageName[] = "/sensor";
unsigned long thisMillis = 0; //Added by Ethan A.
unsigned long lastMillis = 0; //Added by Ethan A.
//record_t rx_record;
char bodyBuff[400];
StaticJsonDocument<400> doc;

WiFiServer server(80);

WiFiServer *sensor;

RTCZero rtc;

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

char outbuff[4096];

typedef struct {
    char id[17];
    time_t timestamp;
    uint8_t valid;
    int8_t reading;
    int8_t batteryPercentage;
    int16_t rssi;
    char name[32];
} record_t;

record_t records[MAX_RECORDS];

typedef struct {
    char id[17];
    char name[32];
    uint16_t cal0;
    uint16_t cal25;
    uint16_t cal50;
    uint16_t cal75;
    uint16_t cal100;
    int updateMins;
} client_t;

client_t clients[MAX_CLIENTS];

int valid;
char wifiSSID[32];
char wifiPass[32];
byte serverIP[4];
uint32_t interval;
int serverport;

unsigned long epoch;

IPAddress server_dns(0x08080808);
IPAddress server_gateway;
IPAddress server_subnet(0x00FFFFFF);
IPAddress server_ip;
uint32_t serverRestart;
uint32_t currentMillis, previousMillis, loraMillis;

// Function Prototypes

byte wifiMac[6];

void errorHang(int t, int blinks) {
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
        if (blinks) {
            for (int i = 0; i < (blinks * 2); i++) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                delay(t);
            }
            delay(3000);
        } else {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(t);
        }

    }
}

bool EndsWith(char *search_in, char *to_find) {
    int search_in_len = strlen(search_in);
    int to_find_len = strlen(to_find);
    int spot;

    if (search_in_len < to_find_len || !to_find_len)
        return false;

    for (int i = 0; i < to_find_len; i++) {
        spot = search_in_len - to_find_len + i;
        if (search_in[spot] != to_find[i])
            return false;
    }

    return true;
}

bool StartsWith(char *search_in, char *to_find) {
    int search_in_len = strlen(search_in);
    int to_find_len = strlen(to_find);

    if (search_in_len < to_find_len || !to_find_len)
        return false;

    for (int i = 0; i < to_find_len; i++) {
        if (search_in[i] != to_find[i])
            return false;
    }

    return true;
}

int IndexOf(char *buff, char *to_find, int startingAt) {
    int buff_len = strlen(buff);
    int to_find_len = strlen(to_find);
    int matchcount;

    if (buff_len < to_find_len || !to_find_len)
        return 0;

    matchcount = 0;
    for (int i = startingAt; i < buff_len; i++) {
        if (buff[i] == to_find[matchcount])
            matchcount++;
        else
            matchcount = 0;

        if (matchcount >= to_find_len)
            return i - to_find_len + 1;
    }

    return 0;
}

//sanitizeString removes cr+lf, null terminates at first occurance
void sanitizeString(char *input, int len) {
    int i;
    for (i = 0; i < len && input[i] != 0; i++) {
        if (input[i] == '\r' || input[i] == '\n')
            break;
    }
    input[i] = 0;
    return;
}

File myFile;

char sIP[32], sGateway[32], sDNS[32], sSubnet[32];

void setup() {
    rtc.begin();

    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    memset(records, 0, sizeof(records));
    memset(clients, 0, sizeof(clients));

    //sprintf(clients[0].id, "0004A30B00E86F6A");
    //clients[0].updateMins = 1;

    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    //while (!Serial); // wait for serial port to connect. Needed for native USB port only

    for (int i = 0; i < 50; i++) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(50);
    }

    digitalWrite(4, HIGH);

    // set wifi pins - probably only necessary for Feather
#ifdef FEATHER
    WiFi.setPins(8, 7, 4, 2);
//    Serial.println("feather...");
#endif

    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
        // don't continue
        while (true);
    }

    Serial.println("Reading SD Card");

    //start SD card
    if (!SD.begin(5)) {
        // couldnt mount SD card
        errorHang(500, 2);
    }

    //open wifi text file for reading
    myFile = SD.open("wifi.txt", FILE_READ);

    if (!myFile) {
        //couldnt open it!
        errorHang(500, 3);
    } else {
        //file opened, pull out wifi settings
        String filedata = "";

        while (myFile.available())
            filedata += (char) myFile.read();

        myFile.close();

        int idx, idx2, i;

        //now parse
        // get SSID out of read file
        idx = filedata.indexOf("wifi:", 0) + 5;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&wifiSSID[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(wifiSSID, strlen(wifiSSID));

        // get Password out of read file
        idx = filedata.indexOf("password:", 0) + 9;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&wifiPass[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(wifiPass, strlen(wifiPass));

        // get Static IP out of read file
        idx = filedata.indexOf("serverip:", 0) + 9;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&sIP[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(sIP, strlen(sIP));

        // get subnet mask out of read file
        idx = filedata.indexOf("subnetmask:", 0) + 11;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&sSubnet[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(sSubnet, strlen(sSubnet));

        // get Static IP out of read file
        idx = filedata.indexOf("gatewayip:", 0) + 10;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&sGateway[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(sGateway, strlen(sGateway));

        // get Static IP out of read file
        idx = filedata.indexOf("dnsip:", 0) + 6;
        idx2 = filedata.indexOf("\n", idx);
        memcpy(&sDNS[0], filedata.c_str() + idx, idx2 - idx);
        sanitizeString(sDNS, strlen(sDNS));

        Serial.println("=== Read from wifi.txt ===");
        Serial.print("ServerIP: ");
        Serial.println(sIP);
        Serial.print("SubnetMask: ");
        Serial.println(sSubnet);
        Serial.print("GatewayIP: ");
        Serial.println(sGateway);
        Serial.print("DNS IP: ");
        Serial.println(sDNS);
        Serial.print("Wifi Network Name: ");
        Serial.println(wifiSSID);
        Serial.print("Wifi Network Password: ");
        for (int i = 0; i < strlen(wifiPass); i++)
            Serial.print('*');
        Serial.println();

        //while(1);
    }

    //open known clients text file for reading
    myFile = SD.open("clients.txt", FILE_READ);

    if (!myFile) {
        //couldnt open it!
        //create and add new data to clients.txt on SD card
        File newClients = SD.open("clients.txt", FILE_WRITE);

        if (newClients) {
            //write temporary ID
            newClients.write("0004A30B00FFFFFF");
            newClients.write(',');
            //write temporary name
            newClients.write("LoRaClient-FFFF");
            newClients.write(',');
            //write temporary cal0
            newClients.write("0");
            newClients.write(',');
            //write temporary cal25
            newClients.write("25");
            newClients.write(',');
            //write temporary cal50
            newClients.write("50");
            newClients.write(',');
            //write temporary cal75
            newClients.write("75");
            newClients.write(',');
            //write temporary cal100
            newClients.write("100");
            newClients.write(',');
            //write temporary interval time in minutes
            newClients.write("60");
            newClients.close();
        }
        errorHang(500, 3);
    } else {
        //file opened, pull out known clients
        String filedata = "";

        while (myFile.available())
            filedata += (char) myFile.read();

        myFile.close();

        int idx, idx2, i;
        int subidx;
        String line;
        char tmp[256];
        size_t len = filedata.length();

        client_t readClient;
        memset(&readClient, 0, sizeof(readClient));

        memcpy(tmp, filedata.c_str(), len);
        char *filepart;

        // pull out ID
        filepart = strtok(tmp, ",\0\n");
        strcpy(readClient.id, filepart);

        // pull out name
        filepart = strtok(NULL, ",\0\n");
        strcpy(readClient.name, filepart);

        // pull out cal0
        filepart = strtok(NULL, ",\0\n");
        readClient.cal0 = atoi(filepart);

        // pull out cal25
        filepart = strtok(NULL, ",\0\n");
        readClient.cal25 = atoi(filepart);

        // pull out cal50
        filepart = strtok(NULL, ",\0\n");
        readClient.cal50 = atoi(filepart);

        // pull out cal75
        filepart = strtok(NULL, ",\0\n");
        readClient.cal75 = atoi(filepart);

        // pull out cal100
        filepart = strtok(NULL, ",\0\n");
        readClient.cal100 = atoi(filepart);

        // pull out updateMins
        filepart = strtok(NULL, ",\0\n");
        readClient.updateMins = atoi(filepart);

        memcpy(&clients[0], &readClient, sizeof(client_t));

        Serial.println("=== Read from clients.txt ===");
        Serial.print("id:");
        Serial.println(clients[0].id);
        Serial.print("name:");
        Serial.println(clients[0].name);
        Serial.print("cal0:");
        Serial.println(clients[0].cal0);
        Serial.print("cal25:");
        Serial.println(clients[0].cal25);
        Serial.print("cal50:");
        Serial.println(clients[0].cal50);
        Serial.print("cal75:");
        Serial.println(clients[0].cal75);
        Serial.print("cal100:");
        Serial.println(clients[0].cal100);

    }

    // ===========  Setup LoRa  ============

    Serial.println("Starting LoRa...");

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!rf95.init()) {
        Serial.println("LoRa radio init failed");
        //Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
        while (1);
    }

    Serial.println("...LoRa started");
    rf95.setTxPower(23, false);

    rf95.setSpreadingFactor(7);
    rf95.setSignalBandwidth(125);
    rf95.setPreambleLength(8);
    rf95.setPayloadCRC(1);
    rf95.setCodingRate4(5);

    rf95.setModemConfig(rf95.Bw125Cr45Sf128);
    rf95.spiWrite(RH_RF95_REG_39_SYNC_WORD, 0x12);

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }

    loraMillis = previousMillis = currentMillis = millis();
}



void loop() {

    uint8_t loraRxBuf[RH_RF95_MAX_MESSAGE_LEN];

    record_t rx_record;

    currentMillis = millis();

//  Serial.println("loop...");
    //delay(50);

    status = WiFi.status();

    while (status != WL_CONNECTED) {
        Serial.print("====Trying to connect to ");
        Serial.println(wifiSSID);
        

        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(wifiSSID, wifiPass);

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(1000);

        if (status == WL_CONNECTED) {
            server_ip.fromString(sIP);
            server_gateway.fromString(sGateway);
            server_subnet.fromString(sSubnet);
            server_dns.fromString(sDNS);
            WiFi.config(server_ip, server_dns, server_gateway, server_subnet);
            Serial.println("WiFi Status: CONNECTED :)");
            Serial.println("Waiting 5 seconds for connection to stabilize...");
            delay(5000);

            Serial.print("Setting RTC time to (epoch seconds): ");
            Serial.println(WiFi.getTime());
            rtc.setEpoch(WiFi.getTime());

            Serial.println("Starting webpage server!");
            

            server.begin();
        } else {
            Serial.println(" FAILED.  Trying again...");
        }

    }

    if ((currentMillis - loraMillis) > 100) {
        //Serial.println("LoRa check rx");
        //update lora receive timer
        loraMillis = currentMillis;
        if (rf95.available()) {
            // Should be a message for us now

            uint8_t len = sizeof(loraRxBuf);

            if (rf95.recv(loraRxBuf, &len)) {
                Serial.print("Got: ");
                Serial.println((char *) loraRxBuf);
                Serial.print("RSSI: ");
                Serial.println(rf95.lastRssi(), DEC);

                int idx, idx2, cnt;
                char tmp[64];
                int b1, b2;

                //sample packet  {"id": 0011223344556677,"bat": 1024,"b1": 123,"b2": 321}
                idx = 0;
                memset(&rx_record, 0, sizeof(rx_record));

                // get id frome loraRxBuf
                idx = IndexOf((char *) loraRxBuf, "{\"id\": \"", idx) + 8;
                idx2 = IndexOf((char *) loraRxBuf, "\",", idx); //the end of the name data within the response

                cnt = (idx2 - idx);
                cnt = (cnt < 0) ? 0 : cnt;
                cnt = (cnt >= 16) ? 16 : cnt;

                // copy id string into tmp to process
                //memcpy(tmp, &loraRxBuf[idx], cnt);
                //tmp[cnt]=0; // null terminate
                memcpy(rx_record.id, &loraRxBuf[idx], cnt);
                rx_record.id[16] = 0; // null terminate

                // get battery voltage out of loraRxBuf
                idx = IndexOf((char *) loraRxBuf, "\"bat\": ", idx2) + 7;
                idx2 = IndexOf((char *) loraRxBuf, ",", idx);

                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, &loraRxBuf[idx], idx2 - idx);
                Serial.print("raw battery: ");
                Serial.println(tmp);
                //Serial.println("Battery Voltage");
                if (strlen(tmp)) {

                    float batt;
                    float battScale = (100.0) / (4.15 - 3.4);
                    float battOffset = 100.0 - (battScale * 4.15);

                    batt = 3.3 * ((atof(tmp) * 2.0) / 1023.0);

                    int8_t percentage = int8_t(battScale * batt + battOffset);

                    if (percentage < 0)
                        percentage = 0;
                    if (percentage > 100)
                        percentage = 100;

                    rx_record.batteryPercentage = percentage;
                    Serial.print("battery: ");
                    Serial.println(rx_record.batteryPercentage);
                }

                // get b1 sensor reading out of rx_packet
                idx = IndexOf((char *) loraRxBuf, "\"b1\": ", idx2) + 6;
                idx2 = IndexOf((char *) loraRxBuf, ",", idx);

                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, &loraRxBuf[idx], idx2 - idx);

                if (strlen(tmp)) {
                    b1 = atoi(tmp);
                    Serial.println(b1);
                }

                // get b2 sensor reading out of rx_packet
                idx = IndexOf((char *) loraRxBuf, "\"b2\": ", idx2) + 6;
                idx2 = IndexOf((char *) loraRxBuf, "}", idx);

                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, &loraRxBuf[idx], idx2 - idx);

                if (strlen(tmp)) {
                    b2 = atoi(tmp);
                    Serial.println(b2);
                }

                Serial.println("Recording RTC time");
                rx_record.timestamp = WiFi.getTime();

                Serial.println("done parsing.");

                // packet received and de-serialized correctly, now add it to the records array
                bool inserted = false;
                /*
                 for(int i=0; i<MAX_RECORDS; i++)
                 {
                 if( !strcmp(records[i].name, rx_record.name) )
                 {
                 //found a record with the same name, overwrite it.
                 records[i].timestamp = rx_record.timestamp;
                 memcpy(records[i].name, rx_record.name, strlen(rx_record.name));
                 records[i].name[strlen(rx_record.name)] = 0;
                 records[i].clientIP = rx_record.clientIP;
                 records[i].batteryVoltage = rx_record.batteryVoltage;
                 records[i].sensorReading  = rx_record.sensorReading;
                 inserted = true;
                 }
                 }
                 */

                // if the new record hasn't been inserted, find an available slot
                if (!inserted) {
                    Serial.println("====inserting====");

                    uint16_t cal0;
                    uint16_t cal25;
                    uint16_t cal50;
                    uint16_t cal75;
                    uint16_t cal100;

                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        //if(rx_record.id != clients[j].id)
                        if (strcmp(rx_record.id, clients[j].id))
                            continue;

                        // If we get here, the ID has been found in the client list

                        cal0 = clients[j].cal0;
                        cal25 = clients[j].cal25;
                        cal50 = clients[j].cal50;
                        cal75 = clients[j].cal75;
                        cal100 = clients[j].cal100;

                        rx_record.rssi = rf95.lastRssi();

                        /*
                         *  For linear interpolation
                         *
                         *  m = (y2-y1)/(x2-x1)
                         *  y = mx + b
                         *  b = y - mx
                         *
                         * */
                        Serial.println("matched id!");

                        //calculate the interpolation coefficients

                        //Y=MX+B
                        //b=y-mx               --> offset
                        //m=(y2-y1)/(x2-x1)    --> scale
                        //x is input, y is output

                        float scale, offset;

                        if (b1 < cal0 && b1 >= cal25) {
                            scale = 25.0 / (float(cal25 - cal0));
                            offset = 25.0 - scale * cal25;
                        } else if (b1 < cal25 && b1 >= cal50) {
                            scale = 25.0 / (float(cal50 - cal25));
                            offset = 50.0 - scale * cal50;
                        } else if (b1 < cal50 && b1 >= cal75) {
                            scale = 25.0 / (float(cal75 - cal50));
                            offset = 75.0 - scale * cal75;
                        } else if (b1 < cal75 && b1 >= cal100) {
                            scale = 25.0 / (float(cal100 - cal75));
                            offset = 100.0 - scale * cal100;
                        }

                        float position = int(scale * b1 + offset); // convert to percentage
                        //float vb = vref - vsns;
                        //float rsns2 = (rref*r3 + (r3*(r1+rref)*vb)/3.3)/(r1-((r1+rref)*vb)/3.3);

                        //clamp sensor readings
                        if (position < 0.0)
                            position = 0.0;
                        if (position > 100.0)
                            position = 100.0;

                        Serial.print("position %=");
                        Serial.println(position);

                        rx_record.reading = int8_t(position);

                        Serial.print("reported position: ");
                        Serial.println(rx_record.reading);
                        // now insert the new reading into the records array

                        //move all records towards end of array
                        for (int i = MAX_RECORDS - 2; i >= 0; i--) {
                            memcpy(&records[i + 1], &records[i],
                                   sizeof(record_t));
                        }

                        //insert new record at the top
                        records[0].timestamp = rx_record.timestamp;
                        strcpy(records[0].id, rx_record.id);
                        strcpy(records[0].name, clients[j].name);
                        records[0].batteryPercentage =
                                rx_record.batteryPercentage;
                        records[0].reading = rx_record.reading;
                        records[0].rssi = rx_record.rssi;

                        //mark record as valid so its displayed
                        records[0].valid = true;

                        memset(tmp, 0, sizeof(tmp));
                        unsigned long long bob = strtoull(records[0].id, NULL,
                                                          16);
                        Serial.print("Response to client: ");
                        //Serial.print((long)(bob>>32),HEX);
                        //Serial.print((long)(bob),HEX);
                        snprintf(tmp, sizeof(tmp), "%08X%08X %d",
                                 (long) (bob >> 32), (long) (bob),
                                 clients[j].updateMins * 15);

                        Serial.println(tmp);

                        //send a response to the client
                        rf95.send((uint8_t *) tmp, strlen(tmp));
                        delay(10);
                        rf95.waitPacketSent();

                        break;  // no need to search more
                    }
                    postData();
                } // end if inserted (dirty joke here...)

                Serial.println("====done inserting====");

            } else {
                Serial.println("Receive failed");
            }
        }

    }

    //zero out any old records
//  for(int i=0; i<MAX_RECORDS; i++)
//  {
//    if(records[i].timestamp + RECORD_TIMEOUT_SECS < rtc.getEpoch())
//      memset(&records[i], 0, sizeof(record_t)); 
//  }

    // listen for incoming clients
    WiFiClient client = server.available();
    char request[2048];
    int rx_cnt;
    if (client) {
//    Serial.println("new web client");

        bool readyToParse = false;
        bool terminate = false;
        rx_cnt = 0;
        memset(request, 0, sizeof(request));
        uint32_t startTime = millis();
        while (client.connected()) {
            char c;

//      Serial.println("web client connected");

            if ((millis() - startTime) > 1000) {
                //timed out, but may still have good data...
                int idx, idx2, len;
                String tmp;
                tmp = "";

                idx = IndexOf(request, "Length: ", 0) + 8;
                idx2 = IndexOf(request, "\r\n", idx);
                while (idx < idx2) {
                    tmp += request[idx++];
                }
                len = tmp.toInt();

                idx = IndexOf(request, "\r\n\r\n", idx) + 4;
                idx2 = strlen(request);

                if ((idx2 - idx) == len)
                    readyToParse = true;
                else
                    terminate = true;
            }

            if (terminate) {
//          Serial.println("Web client TIMED OUT and FLUSHED.");
//        for(int i=0; i<strlen(request); i++)
//        {
//            Serial.print(request[i], HEX);
//            Serial.print(' ');
//        }

                client.flush();
                client.stop();
                break;
            }

            //startTime = millis();
            // get the full cliert request
            while (client.available()) { // if there's bytes to read from the client,
                c = client.read();             // read a byte, then

                request[rx_cnt++] = c;
                //Serial.print(c);

            }

            if (EndsWith(request, "\r\n\r\n"))
                readyToParse = true;

            if (readyToParse) {
                readyToParse = false;
                //while(client.available());  //burn any available data in the client rx buffer
                char tmp[32];

                memset(outbuff, 0, sizeof(outbuff));

                Serial.println("============ request! ===========");
                Serial.print(request);
                Serial.println("============ /request! ===========");

                if (StartsWith(request, "GET / ")) {
                    strcat(outbuff, "HTTP/1.1 200 OK\r\n");
                    strcat(outbuff, "Content-Type: text/html\r\n");
                    strcat(outbuff,
                           "Connection: close\r\n"); // the connection will be closed after completion of the response
                    strcat(outbuff, "Refresh: 10\r\n");
                    strcat(outbuff, "\r\n");
                    strcat(outbuff, "<!DOCTYPE HTML>\r\n");

                    strcat(outbuff, "<html>\r\n<center>");

                    client.println(outbuff);
                    for (int i = 0; i < milone_logo_htm_len;) {
                        if ((milone_logo_htm_len - i) > 1400) {
                            client.write(&milone_logo_htm[i], 1400);
                            i += 1400;
                        } else {
                            client.write(&milone_logo_htm[i],
                                         milone_logo_htm_len - i);
                            break;
                        }
                    }

                    memset(outbuff, 0, sizeof(outbuff));

                    strcat(outbuff,
                           "</center><h1 align=center>Milone Wifi Server</h1>\r\n");

                    strcat(outbuff,
                           "<table align=\"center\" border=\"1\">\r\n");

                    strcat(outbuff, "<tr>");
                    strcat(outbuff, "<th>Record #</th>");
                    strcat(outbuff, "<th>Sensor ID</th>");
                    strcat(outbuff, "<th>Name</th>");
                    strcat(outbuff, "<th>Liquid %</th>");
                    strcat(outbuff, "<th>Battery %</th>");
                    strcat(outbuff, "<th>RSSI</th>");
                    strcat(outbuff, "<th>Time Stamp</th>");
                    strcat(outbuff, "</tr>");

                    for (int i = 0; i < MAX_RECORDS; i++) {
                        if (records[i].valid) {
                            strcat(outbuff, "<tr>");

                            // Entry #
                            strcat(outbuff, "<td>");
                            itoa(i, tmp, 10);
                            strcat(outbuff, tmp);  //entry number
                            strcat(outbuff, "</td>");

                            // Sensor ID #
                            strcat(outbuff, "<td>");
                            strcat(outbuff, records[i].id);  // client id
                            strcat(outbuff, "</td>");

                            // Sensor Name
                            strcat(outbuff, "<td>");
                            strcat(outbuff, records[i].name);  // client id
                            strcat(outbuff, "</td>");

                            strcat(outbuff, "<td>");
                            memset(tmp, 0, sizeof(tmp));
                            sprintf(tmp, "%d", records[i].reading);
                            strcat(outbuff, tmp);  // sensor reading
                            strcat(outbuff, "</td>");

                            strcat(outbuff, "<td>");
                            memset(tmp, 0, sizeof(tmp));
                            sprintf(tmp, "%d", records[i].batteryPercentage);
                            strcat(outbuff, tmp);  // battery voltage
                            strcat(outbuff, "</td>");

                            // Sensor RSSI
                            strcat(outbuff, "<td>");
                            memset(tmp, 0, sizeof(tmp));
                            sprintf(tmp, "%d", records[i].rssi);
                            strcat(outbuff, tmp);  // battery voltage
                            strcat(outbuff, "</td>");

                            strcat(outbuff, "<td>");
                            //sprintf(tmp, "%lu", (records[i].timestamp));  // timestamp
                            //sprintf(tmp, "%d:%d:%d  %d/%d/%d", hour(records[i].timestamp), minute(records[i].timestamp), second(records[i].timestamp), year(records[i].timestamp), month(records[i].timestamp), day(records[i].timestamp));
                            strcat(outbuff, ctime(&records[i].timestamp));
                            //strcat(outbuff, tmp);  // timestamp
                            strcat(outbuff, "</td>");

                            strcat(outbuff, "</tr>");
                        }
                    }
                    strcat(outbuff, "</table>");
                    strcat(outbuff,
                           "<br><br><center><a href='setup.htm'>Setup New Sensor</a></center>");
                    strcat(outbuff,
                           "<br><br><hr><center><font size=\"3\">&copy; 2019 <a href=\"http://milonetech.com\">MiloneTech.com</a>; v0.2.0</font><br>Server Time (UTC): ");
                    rx_record.timestamp = WiFi.getTime();
                    strcat(outbuff, ctime(&rx_record.timestamp));
                    //sprintf(tmp, "%d:%d:%d  %d/%d/%d", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getYear(), rtc.getMonth(), rtc.getDay());
                    //strcat(outbuff, tmp);
                    strcat(outbuff, "</center><hr></html>");

                    // The HTTP response ends with another blank line:

                    for (int i = 0; i < strlen(outbuff);) {
                        if ((strlen(outbuff) - i) > 1400) {
                            client.write(&outbuff[i], 1400);
                            i += 1400;
                        } else {
                            client.write(&outbuff[i], strlen(outbuff) - i);
                            break;
                        }
                    }

                    //client.println(outbuff);
                    client.println();

                    client.stop();
                }else if (StartsWith(request, "GET /sensor.json ")) {
                    #define tempInString "AT+CPMS=\"ME\""
                    strcat(outbuff, "HTTP/1.1 200 OK\r\n");
                    strcat(outbuff, "Content-Type: application/json\r\n");
                    strcat(outbuff, "\r\n");

                    for (int i = 0; i < MAX_RECORDS; i++) {
                        if (records[i].valid) { 
                          doc["Sensor ID"] = records[i].id;
                          JsonArray sensorInfo = doc.createNestedArray("Sensor Data");
                          JsonObject sensData = sensorInfo.createNestedObject();
                          
                          itoa(i, tmp, 10);
                          sensData["Entry #"] = i;
                          
                          sensData["Name"] = records[i].name;
                          
                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%d", records[i].reading);
                          sensData["Liquid %"] = records[i].reading;

                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%d", records[i].batteryPercentage);
                          sensData["Battery %"] = records[i].batteryPercentage;

                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%d", records[i].rssi);
                          sensData["RSSI"] = records[i].rssi;
                          
                          sensData["Time Stamp"] = ctime(&rx_record.timestamp);
                          serializeJsonPretty(doc, bodyBuff);
                          Serial.println("This uses:");
                          Serial.println(doc.memoryUsage());  
                          doc.garbageCollect();
                        }
                    }
                    for (int i = 0; i < strlen(bodyBuff);) {
                        if ((strlen(bodyBuff) - i) > 1400) {
                            client.write(&bodyBuff[i], 1400);
                            i += 1400;
                        } else {
                            client.write(&bodyBuff[i], strlen(bodyBuff) - i);
                            break;
                        }
                    }

                    //client.println(outbuff);
                    client.println();

                    client.stop();


                } else if (StartsWith(request, "GET /setup.htm ")) {
                    // serve the setup page
//            Serial.println("Webpage requested!");

                    //Serial.write(request, strlen(request));

                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    strcat(outbuff, "HTTP/1.1 200 OK\r\n");
                    strcat(outbuff, "Content-Type: text/html\r\n");
                    strcat(outbuff, "\r\n");

                    strcat(outbuff, "<!DOCTYPE HTML>\r\n");
                    strcat(outbuff, "<html>\r\n");
                    strcat(outbuff, "<head><style></style></head>");

                    strcat(outbuff, "<body><center>");

                    client.println(outbuff);

                    for (int i = 0; i < milone_logo_htm_len;) {
                        if ((milone_logo_htm_len - i) > 1400) {
                            client.write(&milone_logo_htm[i], 1400);
                            i += 1400;
                        } else {
                            client.write(&milone_logo_htm[i],
                                         milone_logo_htm_len - i);
                            break;
                        }
                    }

                    memset(outbuff, 0, sizeof(outbuff));

                    strcat(outbuff,
                           "</center><h1 align=center>Milone LoRa Liquid Level Sensor<br><i>Client Setup</i></h1>\r\n");

                    strcat(outbuff, "<form action=\"/\" method=\"post\"><hr>");
                    strcat(outbuff,
                           "<center><table border='1'><tr><th align='center'>Live Raw Value</th><th align='center'>Interpolated %</th><th align='center'>From LoRa ID</th></tr>");
                    strcat(outbuff,
                           "<tr><td align='center'><div id='result'></div></td><td align='center'><div id='interpolated'></div></td><td align='center'><div id='rxid'></div></td></tr>");
                    strcat(outbuff, "</table></center>");

                    strcat(outbuff,
                           "<p>0% Raw Value: <input type='text' name='cal0' value='0' id='cal0'><button type='button' onclick='update_cal0()'><--Update with Live Raw Value</button></p><p>25% Raw Value: <input type='text' name='cal25' value='256' id='cal25'><button type='button' onclick='update_cal25()'><--Update with Live Raw Value</button></p><p>50% Raw Value: <input type='text' name='cal50' value='512' id='cal50'><button type='button' onclick='update_cal50()'><--Update with Live Raw Value</button></p><p>75% Raw Value: <input type='text' name='cal75' value='768' id='cal75'><button type='button' onclick='update_cal75()'><--Update with Live Raw Value</button></p><p>100% Raw Value: <input type='text' name='cal100' value='1023' id='cal100'><button type='button' onclick='update_cal100()'><--Update with Live Raw Value</button></p><hr />");
                    strcat(outbuff,
                           "<p>Enter Sensor Name:<input id=\"name\" type=\"text\" name=\"name\" value=\"LoRaClient-FFFF\" maxlength=\"31\"></p><p>Measurement Interval (minutes):<input id=\"interval\" type=\"text\" name=\"interval\" value=\"60\"></p><p>Set Client ID: <input id=\"clientid\" type=\"text\" name=\"clientid\" value=\"0004A30B00FFFFFF\"><button type='button' onclick='update_id()'><--Update with Last Received ID</button></p>");

                    strcat(outbuff,
                           "<hr><center><input type=\"submit\" value=\"Save and Return\"><br><br><a href='/'>Cancel</a></center></form>");

                    strcat(outbuff, "<script>\r\n");
                    strcat(outbuff,
                           "var reading; var scale; var offset; var id; var msg;");
                    strcat(outbuff,
                           "if(typeof(EventSource) !== \"undefined\") {\r\n");
                    strcat(outbuff,
                           "var source = new EventSource('read.php');\r\n");
                    //strcat(outbuff, "var outputElement = document.getElementById('result');"
                    strcat(outbuff, "source.onmessage = function(event) {\r\n");
                    strcat(outbuff,
                           "msg = JSON.parse(event.data); reading = msg.b1; id = msg.id;");
                    strcat(outbuff,
                           "document.getElementById('result').innerHTML = reading; document.getElementById('rxid').innerHTML = id;\r\n");
                    strcat(outbuff,
                           "var cal0 = parseInt(document.getElementById('cal0').value, 10);");
                    strcat(outbuff,
                           "var cal25 = parseInt(document.getElementById('cal25').value, 10);");
                    strcat(outbuff,
                           "var cal50 = parseInt(document.getElementById('cal50').value, 10);");
                    strcat(outbuff,
                           "var cal75 = parseInt(document.getElementById('cal75').value, 10);");
                    strcat(outbuff,
                           "var cal100 = parseInt(document.getElementById('cal100').value, 10);");
                    strcat(outbuff, "if(reading < cal0 && reading >= cal25) {");
                    strcat(outbuff,
                           "scale = (25)/(cal25-cal0); offset=(25-(scale*cal25));}");
                    strcat(outbuff,
                           "else if(reading < cal25 && reading >= cal50) {");
                    strcat(outbuff,
                           "scale = (25)/(cal50-cal25); offset=(50-(scale*cal50));}");
                    strcat(outbuff,
                           "else if(reading < cal50 && reading >= cal75) {");
                    strcat(outbuff,
                           "scale = (25)/(cal75-cal50); offset=(75-(scale*cal75));}");
                    strcat(outbuff,
                           "else if(reading < cal75 && reading >= cal100) {");
                    strcat(outbuff,
                           "scale = (25)/(cal100-cal75); offset=(100-(scale*cal100));}");
                    strcat(outbuff, "else {");
                    strcat(outbuff, "}");
                    strcat(outbuff,
                           "document.getElementById('interpolated').innerHTML = Math.round( scale*reading+offset );");
                    strcat(outbuff, "};\r\n");
                    strcat(outbuff, "} else {\r\n");
                    strcat(outbuff,
                           "document.getElementById('result').innerHTML = \"Sorry, your browser does not support server-sent events...\";\r\n");
                    strcat(outbuff, "}\r\n");
                    strcat(outbuff, "</script>\r\n");

                    strcat(outbuff, "<script>");
                    strcat(outbuff,
                           "function update_cal0(){document.getElementById('cal0').value = document.getElementById('result').innerHTML;}");
                    strcat(outbuff,
                           "function update_cal25() {document.getElementById('cal25').value = document.getElementById('result').innerHTML;}");
                    strcat(outbuff,
                           "function update_cal50() {document.getElementById('cal50').value = document.getElementById('result').innerHTML;}");
                    strcat(outbuff,
                           "function update_cal75() {document.getElementById('cal75').value = document.getElementById('result').innerHTML;}");
                    strcat(outbuff,
                           "function update_cal100() {document.getElementById('cal100').value = document.getElementById('result').innerHTML;}");
                    strcat(outbuff,
                           "function update_id() {document.getElementById('clientid').value = document.getElementById('rxid').innerHTML;}");

                    strcat(outbuff, "</script>");

                    //strcat(outbuff, "<script>");
                    //strcat(outbuff, "var reading = parseInt(document.getElementById('result').innerHTML, 10);");
                    //strcat(outbuff, "var data = {labels: ['Raw Input'], series: [[reading]]};");
                    //strcat(outbuff, "var options = {high: 1023, low: 0, width: 300, height: 500};");
                    //strcat(outbuff, "var responsiveOptions = [['screen and (max-width: 640px)', {seriesBarDistance: 5,axisX: {labelInterpolationFnc: function (value) {return value[0];}}}]];");
                    //strcat(outbuff, "new Chartist.Bar('#chart1', data, options/*, responsiveOptions*/);");
                    //strcat(outbuff, "</script>");

                    strcat(outbuff,
                           "<br><br><hr><center><font size=\"3\">&copy; 2019 <a href=\"http://milonetech.com\">MiloneTech.com</a>; v0.2.0</font><br>\r\n");

                    //strcat(outbuff, "<script>if(typeof(EventSource) !== \"undefined\"){var source = new EventSource(\"read.php\");source.onmessage = function(event){document.getElementById(\"result\").innerHTML += event.data + \"<br>\";};}else{document.getElementById(\"result\").innerHTML = \"Sorry, your browser does not support server-sent events...\";}</script>");
                    strcat(outbuff, "</center><hr></body></html>");

                    // The HTTP response ends with another blank line:

                    for (int i = 0; i < strlen(outbuff);) {
                        if ((strlen(outbuff) - i) > 1400) {
                            client.write(&outbuff[i], 1400);
                            i += 1400;
                        } else {
                            client.write(&outbuff[i], strlen(outbuff) - i);
                            break;
                        }
                    }

                    //client.println(outbuff);
                    client.println();
                    client.stop();
                } else if (StartsWith(request, "POST / ")) {
                    
                    uint16_t cal0, cal25, cal50, cal75, cal100;
                    char sensorName[33];
                    char sensorId[17];
                    String tmp;
                    int idx = 0, idx2;
                    int i = 0;
                    // received a POST from the web page, process it.
//            Serial.println(request);

                    Serial.println("Processing POST");

                    // get cal0
                    idx = IndexOf(request, "cal0=", idx) + 5;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    cal0 = tmp.toInt();

                    Serial.print("cal0:");
                    Serial.println(cal0);

                    // get cal25
                    idx = IndexOf(request, "cal25=", idx) + 6;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    cal25 = tmp.toInt();

                    Serial.print("cal25:");
                    Serial.println(cal25);

                    // get cal50
                    idx = IndexOf(request, "cal50=", idx) + 6;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    cal50 = tmp.toInt();

                    Serial.print("cal50:");
                    Serial.println(cal50);

                    // get cal75
                    idx = IndexOf(request, "cal75=", idx) + 6;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    cal75 = tmp.toInt();

                    Serial.print("cal75:");
                    Serial.println(cal75);

                    // get cal100
                    idx = IndexOf(request, "cal100=", idx) + 7;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    cal100 = tmp.toInt();

                    Serial.print("cal100:");
                    Serial.println(cal100);

                    // get sensor name out of POST
                    idx = IndexOf(request, "name=", idx) + 5; //start of SSID string
                    idx2 = IndexOf(request, "&", idx); //the end of the SSID within the response
                    memset(sensorName, 0, sizeof(sensorName));
                    memcpy(sensorName, &request[idx], idx2 - idx);

                    Serial.print("name: ");
                    Serial.println(sensorName);

                    // get Interval out of POST
                    idx = IndexOf(request, "interval=", 0) + 9;
                    idx2 = IndexOf(request, "&", idx);

                    tmp = "";
                    while (idx < idx2)
                        tmp += request[idx++];
                    interval = tmp.toInt();

                    Serial.print("interval: ");
                    Serial.println(interval);

                    // get sensor ID out of POST
                    request[strlen(request)] = '&'; //Don't remove, this is needed at the end of the request.

                    idx = IndexOf(request, "id=", idx) + 3; //start of ID string
                    idx2 = IndexOf(request, "&", idx); //the end of the ID within the response
                    memset(sensorId, 0, sizeof(sensorId));
                    memcpy(sensorId, &request[idx], idx2 - idx);

                    Serial.print("id: ");
                    Serial.println(sensorId);

                    Serial.println("========= proc.htm ============");
                    Serial.print("name: ");
                    Serial.println(sensorName);
                    Serial.print("id: ");
                    Serial.println(sensorId);
                    Serial.print("cal0: ");
                    Serial.println(cal0);
                    Serial.print("cal25: ");
                    Serial.println(cal25);
                    Serial.print("cal50: ");
                    Serial.println(cal50);
                    Serial.print("cal75: ");
                    Serial.println(cal75);
                    Serial.print("cal100: ");
                    Serial.println(cal100);
                    Serial.print("interval: ");
                    Serial.println(interval);
                    Serial.println("===============================");

                    //0004A30B00E86F6A,HF_Tank001,0,25,50,75,100
                    //remove the old clients.txt from SD card
                    SD.remove("clients.txt");

                    //create and add new data to clients.txt on SD card
                    File newClients = SD.open("clients.txt", FILE_WRITE);

                    if (newClients) {
                        newClients.print(sensorId);
                        newClients.write(',');
                        newClients.print(sensorName);
                        newClients.write(',');
                        newClients.print(cal0);
                        newClients.write(',');
                        newClients.print(cal25);
                        newClients.write(',');
                        newClients.print(cal50);
                        newClients.write(',');
                        newClients.print(cal75);
                        newClients.write(',');
                        newClients.print(cal100);
                        newClients.write(',');
                        newClients.print(interval);
                        newClients.close();
                    }

                    //save new readings onto client ID
                    memcpy(clients[0].id, sensorId, sizeof(clients[0].id));
                    memcpy(clients[0].name, sensorName, sizeof(clients[0].name));
                    clients[0].cal0 = cal0;
                    clients[0].cal25 = cal25;
                    clients[0].cal50 = cal50;
                    clients[0].cal75 = cal75;
                    clients[0].cal100 = cal100;
                    clients[0].updateMins = interval;

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print(
                            "<html><head><meta http-equiv=\"Refresh\" content=\"0; url=/\" /></head></html>");
                    //client.println("Settings Saved!  ");

                    delay(10);
                    client.stop();

                } else if (StartsWith(request, "GET /style.css ")) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Connection: keep-alive");
                    client.println("Content-Type: text/css");
                    client.println("Cache-Control: no-cache");
                    client.print("\n\n");

                    client.print("\n\n");
                    client.flush();

                    client.stop();
                    //Serial.println("...done");

                } else if (StartsWith(request, "GET /read.php ")) {
                    //Serial.println(request);
                    //Serial.print("Sending reading...");
                    float reading = 0.0;
                    for (int i = 0; i < 100; i++) {
                        reading += float(analogRead(0));
                        delay(1);
                    }
                    reading /= 100.0;

                    client.println("HTTP/1.1 200 OK");
                    client.println("Connection: keep-alive");
                    client.println(
                            "Content-Type: text/event-stream; charset=UTF-8");
                    client.println("Cache-Control: no-cache");
                    client.println();
                    client.print("data: ");
                    client.print((char *) loraRxBuf);
                    //Serial.print(data);
                    client.print("\n\n");
                    client.print("retry: 100\n\n");

                    client.println();
                    client.flush();

                    client.stop();
                    //Serial.println("...done");

                } else {
                    Serial.println("Unknown request!");
                    Serial.write(request, strlen(request));

                    client.println("HTTP/1.1 404 Not Found");
                    client.println();

                    client.stop();
                }

            }

        }
         //postData();
        Serial.println("web client disconnected");
    }

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//Added by Ethan Armbruster
byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  //WiFiSSLClient clientPOST;
  WiFiClient clientPOST;
  int inChar;
  char outBuf[64];

  Serial.print(F("====Connecting to server===="));
  Serial.println("Trying to send A JSON");

  if(clientPOST.connect(domainBuffer,thisPort) == 1)
  {
    Serial.println(F("connected"));

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    clientPOST.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    clientPOST.println(outBuf);
    clientPOST.println(F("Connection: close\r\nContent-Type: application/json"));
    clientPOST.println("User-Agent: Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500");
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    clientPOST.println(outBuf);
    // send the body (variables)
    clientPOST.print(thisData);
  }
  else
  {
    Serial.println(F("failed"));
    Serial.println("====CANT SEND IT OUT====");
    return 0;
  }

  int connectLoop = 0;

  while(clientPOST.connected())
  {
    while(clientPOST.available())
    {
      
      inChar = clientPOST.read();
      Serial.write(inChar);
      connectLoop = 0;
    }
    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      clientPOST.stop();
    }
  }
  Serial.println();
  Serial.println(F("disconnecting."));
  clientPOST.stop();
  return 1;
}

void postData() //Added by Ethan A.
{
  char tmp[256];


   for (int i = 0; i < MAX_RECORDS; i++) {
      if (records[i].valid) {
   
           String dt= ctime(&records[i].timestamp);
         doc["Sensor ID"] = records[i].id;
         JsonArray sensorInfo = doc.createNestedArray("Sensor Data"); 
         JsonObject sensData = sensorInfo.createNestedObject();
         WiFi.getTime();
         
         itoa(i, tmp, 10);
         sensData["Entry #"] = tmp;
         
         sensData["Name"] = records[i].name;
         
         memset(tmp, 0, sizeof(tmp));
         sprintf(tmp, "%d", records[i].reading);
         sensData["Liquid %"] = tmp;

         memset(tmp, 0, sizeof(tmp));
         sprintf(tmp, "%d", records[i].batteryPercentage);
         sensData["Battery %"] = tmp;

         memset(tmp, 0, sizeof(tmp));
         sprintf(tmp, "%d", records[i].rssi);
         sensData["RSSI"] = tmp;         
         sensData["Time Stamp"] = dt;
         serializeJsonPretty(doc, bodyBuff);
         doc.garbageCollect();
       }
   }

    if(!postPage(serverName,serverPort,pageName,bodyBuff)) Serial.print(F("Fail "));
    else Serial.print(F("Pass "));


}
