
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi parameters to be configured
const char* ssid = "ssid"; // Write here your router's username
const char* password = "password"; // Write here your router's password

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
const char* host = "api.telegram.org";
const int httpsPort = 443;


// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
unsigned long timerDelay = 300000;
// Set timer to 5 seconds (5000)
//unsigned long timerDelay = 5000;

const char fingerprint[] PROGMEM = "F2 AD 29 9C 34 48 DD 8D F4 CF 52 32 F6 57 33 68 2E 81 C1 90"; //api.telegram.org SHA1 fingerprint

void setup(void)
{ 
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  sensors.begin();
 
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
 
   sensors.requestTemperatures(); 
   float temperatureC = sensors.getTempCByIndex(0);
   Serial.print(temperatureC);
   Serial.println("ºC");
  
    if (temperatureC > 55) { // We want the information only when degrees are above 55 celcius.
     WiFiClient wifiClient;
     WiFiClientSecure httpsClient;
       if ((millis() - lastTime) > timerDelay) {
        Serial.println(host);

        Serial.printf("Using fingerprint '%s'\n", fingerprint);
        httpsClient.setFingerprint(fingerprint);
        httpsClient.setTimeout(15000); // 15 Seconds
        delay(1000);
        
        Serial.print("HTTPS Connecting");
        int r=0; //retry counter
        while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
            delay(100);
            Serial.print(".");
            r++;
        }
        if(r==30) {
          Serial.println("Connection failed");
        }
        else {
          Serial.println("Connected to web");
        }
      
        //GET request
       
        String Temp = String(temperatureC);
        String Link = "/token/sendMessage?chat_id=chatid&text=Sauna%27s+temperature+is+" + Temp; //use HTML URL encoding for text
      
        Serial.print("requesting URL: ");
        Serial.println(host+Link);
      
        httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +               
                     "Connection: close\r\n\r\n");
      
        Serial.println("request sent");
                        
        while (httpsClient.connected()) {
          String line = httpsClient.readStringUntil('\n');
          if (line == "\r") {
            Serial.println("headers received");
            break;
          }
        }
      
        Serial.println("reply was:");
        Serial.println("==========");
        String line;
        while(httpsClient.available()){        
          line = httpsClient.readStringUntil('\n');  //Read Line by Line
          Serial.println(line); //Print response
        }
        Serial.println("==========");
        Serial.println("closing connection");
          
        delay(2000);  //GET delay 2 seconds
        lastTime = lastTime + 300000; // Add 5 minutes to the timer. We only want this information every 5 minutes.
    }
    }
  }
}
