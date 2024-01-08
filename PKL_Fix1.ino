#include <ESP8266WiFi.h>             //Memanggil Library WIFI ESP8266
#include <WiFiClientSecure.h>        //Memanggil Library WIFI ESP8266 KE SERVER
#include <UniversalTelegramBot.h>    //Memanggil Library Telegram
#include <SPI.h>                     //Memanggil Library LCD OLED I2C
#include <Wire.h>                    //Memanggil Library LCD OLED I2C
#include <Adafruit_GFX.h>            //Memanggil Library LCD OLEDSP8266
#include <Adafruit_SSD1306.h>        //Memanggil Library LCD OLED
#include <DallasTemperature.h>       //Memanggil Library Sensor Suhu
#include <OneWire.h>                 //Memanggil Library protokol OneWire

#define SCREEN_WIDTH 128             //mendefinisikan jumlah layar oled kesamping
#define SCREEN_HEIGHT 64             //mendefinisikan jumlah layar oled keatas
#define ONE_WIRE_BUS 12              //mendefinisikan pin yang akan digunakan oleh sensor
#define kipas1 D0                    //mendefinisikan pin yang akan digunakan oleh kipas 1
#define kipas2 D5                    //mendefinisikan pin yang akan digunakan oleh kipas 2
#define OLED_RESET -1                // memberikan nilai variable pada layar oled -1 digunakan karna pin reset tidak digunakan
#define SCREEN_ADDRESS 0x3C          //memberikan nilai hexadecimal pada layar oled pada bus I2C


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   //menggunakan varible display untuk memanggil 4 variable yang sudah dijumlahkan dalam tanda kurung

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

String tempString, tempString1;
float temp;
float Fahrenheit = 0;

char ssid[] = "CREW_SQUAD";          //username wifi yang akan digunakan untuk terhubung ke internet
char password[] = "opopassword";     //password wifi yang digunakan

#define BOTtoken "6919629877:AAHLSMQv4cJUXtxH35UVbqFiGACSkTNYf4s" //id bot token yang didapat setelah membuat bot pada telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Array untuk menyimpan ID Chat yang diizinkan
String allowedChats[] = {"1762649368", "your_second_id", "your_third_id"}; //id telegram orang yang menggunakan telegram setelah mendapat dari user info bot

unsigned long lastTemperatureSendTime = 0;
const unsigned long temperatureSendInterval = 120000; // setiap 2 menit mengirim pesan suhu ke telegram
int Bot_mtbs = 1000;
long Bot_lasttime;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    // Pemeriksaan ID Chat sebelum menjalankan perintah
    if (isAllowedChat(chat_id)) {
      if (text == "/ceksuhu") {
        DS18B20.requestTemperatures();
        temp = DS18B20.getTempCByIndex(0);
        Fahrenheit = DS18B20.toFahrenheit(temp);
        String suhu = "Suhu sekarang : ";
        suhu += String(temp, 2);
        suhu += " °C\n";
        suhu += String(Fahrenheit, 2);
        suhu += " °F";
        bot.sendMessage(chat_id, suhu, "");
      } 
      else if (text == "/kipas1on") {
        digitalWrite(kipas1, LOW);
        bot.sendMessage(chat_id, "Kipas 1 sudah menyala", "");
      } 
      else if (text == "/kipas1off") {
        digitalWrite(kipas1, HIGH);                              //memanggil variable pin kipas 1 untuk high
        bot.sendMessage(chat_id, "Kipas 1 sudah mati", "");      //tulisan yang muncul pada telegram setelah kipas sudah mati
      } 
      else if (text == "/start") {                               //variable yang muncul setelah mengetik kata start pada telegram  
        String welcome = "Welcome " + from_name + " .\n";        // nama tulisan yang muncul 
        welcome += "/ceksuhu : Cek suhu ruangan\n";              // tulisan yang muncul pada telegram  
        welcome += "/kipas1on : Nyalakan kipas1\n";              // tulisan yang muncul pada telegram       
        welcome += "/kipas1off : Matikan kipas1\n";              // tulisan yang muncul pada telegram   
        bot.sendMessage(chat_id, welcome, "Markdown");           //mengirim ke telegram
      }
    } else {
      // ID Chat tidak valid, kirim pesan ke pengirim
      String errorMessage = "Anda tidak memiliki izin untuk menjalankan perintah ini.";
      bot.sendMessage(chat_id, errorMessage, "");
    }
  }
}

void sendTemperatureToTelegram() {
  // Pemeriksaan ID Chat sebelum mengirim suhu
  for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
    String chat_id = allowedChats[i];
    if (chat_id != "") {
      DS18B20.requestTemperatures();
      temp = DS18B20.getTempCByIndex(0);
      Fahrenheit = DS18B20.toFahrenheit(temp);

      String suhuMessage = "Suhu saat ini : ";
      suhuMessage += String(temp, 2);
      suhuMessage += " °C\n";
      suhuMessage += String(Fahrenheit, 2);
      suhuMessage += " °F";

      bot.sendMessage(chat_id, suhuMessage, "");
    }
  }
}

void setup() {
  pinMode(kipas1, OUTPUT);    // variale kipas1 dipanggil dan digunakan sebagai output
  digitalWrite(kipas1, HIGH); // kondisi pin diawal
  pinMode(kipas2, OUTPUT);    // variale kipas2 dipanggil dan digunakan sebagai output
  digitalWrite(kipas2, LOW);  // kondisi pin diawal 
  Serial.begin(115200);

  DS18B20.begin();
  Wire.begin();
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      delay(1000);
    }
  }

  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  displayDisplayCenter("Teknik Elektro", 4);
  displayDisplayCenter("POLINEMA 2024", 56);

  display.setTextSize(2);
  display.setCursor(0, 32);

  tempString.reserve(10);
}

void loop() {
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  Fahrenheit = DS18B20.toFahrenheit(temp);
  Serial.println(temp);
  Serial.println(Fahrenheit);

  tempString = String(temp, 2);
  tempString += (char)247;
  tempString += "C";

  if (tempString != tempString1) {
    display.setTextColor(BLACK);
    displayDisplayCenter(tempString1, 28);
  }

  tempString1 = tempString;
  Serial.println(tempString);

  display.setTextColor(WHITE);
  displayDisplayCenter(tempString, 28);

  if (millis() - lastTemperatureSendTime >= temperatureSendInterval) {
    lastTemperatureSendTime = millis();   // Perbarui waktu terakhir pengiriman
    sendTemperatureToTelegram();
  }

  if (temp > 35.00) {
    digitalWrite(kipas2, HIGH);
    for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
      String chat_id = allowedChats[i];
      if (chat_id != "") {
        bot.sendMessage(chat_id, "Kipas 2 aktif.", "");
      }
    }
  } else if (temp < 30.00) {
    digitalWrite(kipas2, LOW);
  }

  if (temp > 35.00) {
    bot.sendChatAction(allowedChats[0], "Sedang mengetik...");
    Serial.print("Suhu saat ini : ");
    Serial.println(temp);
    delay(2000);

    String suhu = "Intensitas suhu : ";
    suhu += String(temp, 2);
    suhu += " *C\n";
    suhu += "Suhu maksimal\n";
    for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
      String chat_id = allowedChats[i];
      if (chat_id != "") {
        bot.sendMessage(chat_id, suhu, "");
      }
    }
    Serial.print("Mengirim data sensor ke telegram");
  }

  if (millis() > Bot_lasttime + Bot_mtbs) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Memeriksa Respon");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    Bot_lasttime = millis();
  }
}

void displayDisplayCenter(String text, int posisi) {
  int16_t x1, y1;
  uint16_t width, height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  display.setCursor((SCREEN_WIDTH - width) / 2, posisi);
  display.println(text);
  display.display();
}

bool isAllowedChat(String chatId) {
  // Mengecek apakah chatId ada dalam array allowedChats
  for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
    if (allowedChats[i] == chatId) {
      return true;
    }
  }
  return false;
}
