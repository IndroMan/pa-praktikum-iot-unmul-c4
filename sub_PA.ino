#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <PubSubClient.h>

int freq = 0;
bool on = false;
int diley = 0;

//===================Wifi==================
#define WIFI_SSID "POCO F3"
#define WIFI_PASSWORD ""
//===================Wifi==================

//===================Sensor Pin==================
int buzzer = D0;
//===================Sensor Pin==================


//=======================MQTT======================
const char* mqtt_server = "broker.hivemq.com"; // broker gratisan
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Fungsi untuk menerima data
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String data = ""; // variabel untuk menyimpan data yang berbentuk array char
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i]; // menyimpan kumpulan char kedalam string
  }
  Serial.println(" Cm\n");
  int jarak = data.toFloat(); // konvert string ke int
  
  tone(buzzer, 250);
  if (jarak <= 60){
    freq = 500;
  }else if(jarak <= 40){
    freq = 1500;
  }else if(jarak <= 20){
    freq = 2500;
  }else{
    freq = 0;
  }
}

// fungsi untuk mengubungkan ke broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("iot_unmul/iot_c_1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//=======================MQTT======================


// ==================Telegram BOT===================
#define BOT_TOKEN "5995753020:AAFyZIzuMzpRxp4mvrPK28r894f6LMkrGSc"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    // String from_name = bot.messages[i].from_name;
    // if (from_name == "")
    //   from_name = "Guest";
    
    if (text == "/nyalakanAlat") {
      client.publish("iot_unmul/iot_c_1/alat", "on");
      on = true;
      bot.sendMessage(chat_id, "Alat dinyalakan...", "");
    }

    if (text == "/matikanAlat") {
      client.publish("iot_unmul/iot_c_1/alat", "off");
      on = false;
      bot.sendMessage(chat_id, "Alat dimatikan...", "");
    }

    if (text == "/statusAlat") {
      String message = "Status Alat : ";
      if(on == true) {
        message += "Sedang Menyala";
      } else {
        message += "Mati";
      }
      
      bot.sendMessage(chat_id, message, "");
    }

    if (text == "/start") {
      String welcome = "Projek Akhir C1\n";
      welcome += "Daftar Perintah :\n\n";
      welcome += "/nyalakanAlat : Untuk Menyalakan Alat\n";
      welcome += "/matikanAlat : Untuk Mematikan Alat\n";
      welcome += "/statusAlat : Menampilkan Status Alat\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}
// ==================Telegram BOT===================

void setup(){
  Serial.begin(115200);
  Serial.println();

  // Sensor Pin Mode
  pinMode(buzzer, OUTPUT);

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  // Serial.print("Retrieving time: ");
  // time_t now = time(nullptr);
  // while (now < 24 * 3600)
  // {
  //   Serial.print(".");
  //   delay(100);
  //   now = time(nullptr);
  // }
  // Serial.println(now);

  client.setServer(mqtt_server, 1883); // setup awal ke server mqtt
  client.setCallback(callback);   
}

void loop() {
  if (on == false){
    freq == 0;
  }
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    
    while (numNewMessages) {
      Serial.println("Got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  tone(buzzer,freq);
}