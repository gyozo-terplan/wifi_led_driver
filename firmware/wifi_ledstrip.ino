
/// Loading libraries
#include <IotWebConf.h>
#include <MQTT.h>





/// Defining inputs and outputs of the ESP
#define J1_PIN 16   
#define J2_PIN 12 
#define J3_PIN 14  
#define BUTTON_READ 4 
#define BUTTON_SEND 5 


/// Defining GUI variables
#define STRING_LEN 128
#define NUMBER_LEN 32
const char thingName[] = "led_driver";
const char wifiInitialApPassword[] = "proba1234";
char mqttServerAddress[STRING_LEN];
char mqtt_topic[STRING_LEN];
char mqtt_name[STRING_LEN];
char default_intensity_input[STRING_LEN];


/// Defining variables of the button switch 
int intensity = 0;
int prev_intensity = 0;
long my_time = 0;
long debounce = 50; 
int multiplier;
int default_intensity;


/// Defining variables of the MQTT controll 
int state = HIGH;      
int reading;           
int previous = LOW; 



/// Callback method declarations
void wifiConnected();
void configSaved();
boolean formValidator();
void mqttMessageReceived(String &topic, String &payload);



/// Defining used modules
DNSServer dnsServer;
WebServer server(80);
WiFiClient net;
MQTTClient mqttClient;
HTTPUpdateServer httpUpdater;



/// GUI variable configuration
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword);
IotWebConfParameter mqttServer_Param = IotWebConfParameter("MQTT server address", "mqttServer", mqttServerAddress, STRING_LEN);
IotWebConfParameter mqttTopic_Param = IotWebConfParameter("MQTT topic name", "mqtt_topic", mqtt_topic, STRING_LEN);
IotWebConfParameter mqttName_Param = IotWebConfParameter("MQTT name", "mqtt_name", mqtt_name, STRING_LEN);
IotWebConfParameter default_intensity_Param = IotWebConfParameter("Default light intensity", "default_intensity", default_intensity_input, NUMBER_LEN, "number", "1..100", NULL, "min='1' max='100' step='1'");


/// Defining variables for automatic reconnection
boolean needMqttConnect = false;
boolean needReset = false;
unsigned long lastMqttConnectionAttempt = 0;


/// Defining MQTT topics
char mqttActionTopic[STRING_LEN];
char mqttStatusTopic[STRING_LEN];





void setup() {
  
  Serial.begin(115200);
  Serial.println("Booting");


  /// Set the extra parameters of the GUI
  iotWebConf.addParameter(&mqttServer_Param);
  iotWebConf.addParameter(&mqttTopic_Param);
  iotWebConf.addParameter(&mqttName_Param);
  iotWebConf.addParameter(&default_intensity_Param);


  /// Set callback functions
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setupUpdateServer(&httpUpdater);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);


  /// Initialize the configuration
  boolean validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServerAddress[0] = '\0';
  }
  

  /// Set up required URL handlers on the web server
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  
  /// Set up MQTT client
 
  mqttClient.begin(mqttServerAddress, net);
  mqttClient.onMessage(mqttMessageReceived);


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  /// Set ESP inputs and outputs 
  pinMode(J1_PIN, OUTPUT);
  pinMode(J2_PIN, OUTPUT);
  pinMode(J3_PIN, OUTPUT);
  pinMode(BUTTON_SEND, OUTPUT);
  pinMode(BUTTON_READ, INPUT_PULLUP);
  digitalWrite(BUTTON_SEND, LOW);

  /// Converting default intensity parameter from String to Int
  default_intensity = String(default_intensity_input).toInt();

}

 

void loop() {
 
  /// doLoop should be called as frequently as possible
  
  iotWebConf.doLoop();
  mqttClient.loop();
  
  
  /// Checking connections and subscribing to MQTT action topic
  if ((iotWebConf.getState() == IOTWEBCONF_STATE_ONLINE) && (!mqttClient.connected()))
  {
    Serial.println("MQTT reconnect");
    unsigned long now = millis();
  if (1000 > now - lastMqttConnectionAttempt)
  {
    // Do not repeat within 1 sec.
  }
  
  Serial.println("Connecting to MQTT server...");
  
  if (!mqttClient.connect(iotWebConf.getThingName())) {
    lastMqttConnectionAttempt = now;
    //  Serial.println(!mqttClient.connected());
  }
  Serial.println("Connected!");

  mqttClient.subscribe("/" + String(mqtt_topic) + "/" + mqtt_name + "/action");
  }


  if (needReset)
  {
    Serial.println("Rebooting after 1 second.");
    iotWebConf.delay(1000);
    ESP.restart();
  }


/// Set intensity in line with the multiplier to ensure switch will change the status of the driver   
 if (intensity>0) {
    multiplier=1;
   }
 
 else if (intensity==0) {
    multiplier=0;
    }


/// Read physical button value
  reading = digitalRead(BUTTON_READ);


/// Set the status of the LED strips if the button pressed
  if (reading != previous) {
    // reset the debouncing timer
        my_time = millis(); 
  }
 
  if ((millis() - my_time) > debounce) {

    if (reading != state) {
      
      state = reading;

      if (state == LOW) {
        multiplier = !multiplier;
      }

          intensity = multiplier*default_intensity;
    }
  }
     
/// Save the reading of the button for comparison
  previous = reading;

  
/// Write the outputs with the actual intensity value

  analogWrite(J1_PIN,   1024*float(intensity)/100 ); 
  analogWrite(J2_PIN,   1024*float(intensity)/100 );
  analogWrite(J3_PIN,   1024*float(intensity)/100 );
  
  
/// If Intensity value changed send an MQTT status message 
  
  if(prev_intensity!=intensity) {
  
  char outbound_msg[50];
  snprintf (outbound_msg, sizeof(outbound_msg), "%1d", intensity);

  mqttClient.publish("/" + String(mqtt_topic) + "/" + mqtt_name + "/status",  outbound_msg);
  }

/// Save the previous intensity value for comparison 
  prev_intensity=intensity;
 
}



void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>Led unit</title></head><body>Hello world!";
  s += "<ul>";
  s += "<li>MQTT server: ";
  s += mqttServerAddress;
  s += "</ul>";
  s += "<ul>";
  s += "<li>MQTT topic: ";
  s += mqtt_topic;
  s += "</ul>";
  s += "<ul>";
  s += "<li>MQTT device name: ";
  s += mqtt_name;
  s += "</ul>";
  s += "Go to <a href='config'>configure page</a> to change settings.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}


void mqttMessageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);

  
  if (topic.endsWith(String(mqtt_topic) + "/" + mqtt_name + "/action") )  
  {
   
	 intensity = payload.toInt();
  }
}



void configSaved()
{
  Serial.println("Configuration was updated.");
  needReset = true;
}


boolean formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = server.arg(mqttServer_Param.getId()).length();
  if (l < 3)
  {
    mqttServer_Param.errorMessage = "Please provide at least 3 characters!";
    valid = false;
  }


  return valid;
}



void wifiConnected()
{
  needMqttConnect = true;
}
