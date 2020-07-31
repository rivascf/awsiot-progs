#include <AWS_IOT.h>
#include <WiFi.h>
#include "DHT.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <String.h>


#define DHTPIN 4     // what digital pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AWS_IOT hornbill;   // AWS_IOT instance

// Variables to save date and time


char WIFI_SSID[]="XXXXXX";
char WIFI_PASSWORD[]="XXXXXXXXXX";
char HOST_ADDRESS[]="xxxx.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "xxxx";
char TOPIC_NAME[]= "xxxx/xxxx";

String formattedDate;
String dayStamp;
String timeStamp;

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void setup() {

    Serial.begin(115200);
    delay(2000);

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // wait 5 seconds for connection:
        delay(5000);
    }

    Serial.println("Connected to wifi");

    if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0) // Connect to AWS using Host Address and Cliend ID
    {
        Serial.println("Connected to AWS");
        delay(1000);
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }

    delay(2000);

    dht.begin(); //Initialize the DHT11 sensor
  // Initialize a NTPClient to get time
    timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
    timeClient.setTimeOffset(3600);
}


void loop() {
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    formattedDate = String(timeClient.getFormattedDate());
    //format in which date and time is returned 2018-04-30T16:00:13Z
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

    if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
        sprintf(payload,"{\"timestamp\":\"%s %s\",\"temperature\":%f,\"humidity\":%f}",dayStamp.c_str(),timeStamp.c_str(),t,h);

        if(hornbill.publish(TOPIC_NAME,payload) == 0)   // Publish the message(Temp and humidity)
        {
            Serial.print("Publish Message:");
            Serial.println(payload);
        }
        else
        {
            Serial.println("Publish failed");
        }
        // publish the temp and humidity every 5 seconds.
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}