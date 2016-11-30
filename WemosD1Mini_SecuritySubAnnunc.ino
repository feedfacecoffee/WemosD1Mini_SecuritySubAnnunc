//ESP8266 and MQTT
#include<ESP8266WiFi.h>
#include<PubSubClient.h>

#define _WLAN_SSID "MySSID"
#define _WLAN_PASS "MyPassword"

#define _MQTT_BROKER "10.0.1.250"
#define _MQTT_PORT 1883
#define _MQTT_USER "MyMQTTUser"
#define _MQTT_PASS "MyMQTTPassword"
#define NOTE_E4  330
#define NOTE_G4  392
#define NOTE_C4  262
#define NOTE_G3  196

const char* ssid = _WLAN_SSID;
const char* password = _WLAN_PASS;

WiFiClient wlanClient;
PubSubClient mqttClient(wlanClient);
long lastMsg = 0;
char msg[50];
int buttonState[6] = {-1, -1, -1, -1, -1, -1};
int previousButtonState[6] = {-1, -1, -1, -1, -1, -1};
int securityPublisherState = -1;//0 == disconnected, 1 == connected
int previousSecurityPublisherState = -1;
int firstRun = 1;

//SMB Song
 int arrayLength;
 int tempo = 1000;

// intro notes and durations
int smbIntroNotes[] = {
NOTE_E4, NOTE_E4,0,NOTE_E4,0, NOTE_C4, NOTE_E4, 0, 
NOTE_G4, 0, 0, NOTE_G3, 0, 0 
};
int smbIntroDurations[] = {
8, 8, 8, 8, 8, 8, 8, 8,
8, 8, 4, 8, 8, 4
};

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status()!= WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("SecurityBeep")) {
      Serial.println("Connected");
      mqttClient.subscribe("/security/#", 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);//TODO: Modify to remove delay.
    }
    
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') {
    arrayLength = sizeof(smbIntroDurations) / sizeof(int);
    playNotes(smbIntroNotes, smbIntroDurations, arrayLength);
  }

}

void playNotes(int myNotes[], int myDurations[], int myLength) {
  
  Serial.println(arrayLength);
  for (int thisNote = 0; thisNote < arrayLength; thisNote++) {
    // to calculate the note duration, take one second 
    // divided by the note type.
    int noteDuration = tempo/myDurations[thisNote];

    if(myNotes[thisNote] != 0) {
      tone(4, myNotes[thisNote],noteDuration);
    } else {
      noTone(4);
    }
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(4);
  }
}

void setup() {
  Serial.begin(9600);
  
  //WiFi & MQTT setup
  setup_wifi();
  mqttClient.setServer(_MQTT_BROKER, _MQTT_PORT);
  mqttClient.setCallback(callback);
}

void loop() {

  int rc = -1;

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  delay(2);
}
