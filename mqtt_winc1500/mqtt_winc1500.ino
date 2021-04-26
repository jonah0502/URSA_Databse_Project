/***************************************************
  Adafruit MQTT Library WINC1500 Example

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>
#include <ArduinoJson.h>
#define LEDPIN 13


#include <Loom.h>

// Include configuration
const char* json_config = 
#include "config.h"
;


// Set enabled modules
LoomFactory<
  Enable::Internet::WiFi,
  Enable::Sensors::Enabled,
  Enable::Radios::Disabled,
  Enable::Actuators::Enabled,
  Enable::Max::Disabled
> ModuleFactory{};

LoomManager Loom{ &ModuleFactory };


/************************* WiFI Setup *****************************/
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

char ssid[] = "The Promised LAN";     //  your network SSID (name)
char pass[] = "jzc2kms77vcmxxa8ptkyj";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "jonah0502"
#define AIO_KEY         "aio_OTdV17Edt09Pg2mBZTOQGpoIu833"

/************ Global State (you don't need to change this!) ******************/


//Set up the wifi client
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish soilSen = Adafruit_MQTT_Publish(&mqtt, tempURL.c_str());

// Setup a feed called 'onoff' for subscribing to changes.
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/

void setup() {
//loom stuff
  Loom.begin_LED();
  Loom.begin_serial(true);
  Loom.parse_config(json_config);
  Loom.print_config();
  Loom.measure();
  Loom.package();
  Loom.display_data();




//mqtt start
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  while (!Serial);
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo for WINC1500"));

  // Initialise the Client
  Serial.print(F("\nInit the WiFi module..."));
  // check for the presence of the breakout
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WINC1500 not present");
    // don't continue:
    while (true);
  }
  Serial.println("ATWINC OK!");
  
  pinMode(LEDPIN, OUTPUT);
}


void loop() {

char name[20];
Loom.get_device_name(name);

String groupName = String(name + String(Loom.get_instance_num()));
groupName.toLowerCase();


char tempURL[50];
snprintf(tempURL, 50, "%s%s%s", AIO_USERNAME, "/groups/", groupName.c_str());
//String tempURL = String(String(AIO_USERNAME) + "/groups/" + groupName + "/json");


Adafruit_MQTT_Publish soilSen = Adafruit_MQTT_Publish(&mqtt, tempURL);

  
  StaticJsonDocument<300> JSONencoder ;
  JsonObject root = JSONencoder.to<JsonObject>();
  JsonObject feeds = root.createNestedObject("feeds");
  feeds["key-1"] = 20;
  feeds["key-2"] = 30;
  feeds["key-3"] = 40;
  Serial.println(groupName);
  //Serial.println(Loom.device_name);
  Serial.println(tempURL);
  Loom.print_config(true);

  char JSONmessageBuffer[100];
  serializeJson(root, JSONmessageBuffer);
  Serial.println(JSONmessageBuffer);
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop
  //int32_t moistureVal = Loom.get_data_as<int32_t>("STEMMA_7", "capactive");
  // Now we can publish stuff!
  Serial.print(F("\nSending soilSen val "));
  //Serial.print(moistureVal);
  Serial.print("...");
  if (! soilSen.publish(JSONmessageBuffer)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  Loom.measure();
  Loom.package();
  Loom.pause();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
