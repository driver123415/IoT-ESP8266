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


// menggunakan OneWire dan DallasTemperature untuk menghubungkan sensor suhu tipe DS18B20 ke NodeMCU (ESP8266) melalui jalur satu kabel (OneWire)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// menyimpan informasi suhu yang diukur oleh sensor suhu DS18B20
String tempString, tempString1;
float temp;
float Fahrenheit = 0;

char ssid[] = "ChangeThisToYourWifiUsername";          //Isikan username wifi yang akan digunakan untuk terhubung ke internet
char password[] = "ChangeThisToYourWifiPassword";     //Isikan password wifi yang digunakan

#define BOTtoken "6919629877:AAHLSMQv4cJUXtxH35UVbqFiGACSkTNYf4s"  //id bot token yang didapat setelah membuat bot pada telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Array untuk menyimpan ID Chat yang diizinkan
String allowedChats[] = {"1762649368", "your_second_id", "your_third_id"};  //id telegram orang yang menggunakan telegram setelah mendapat dari user info bot

unsigned long lastTemperatureSendTime = 0;
const unsigned long temperatureSendInterval = 120000; // setiap 2 menit mengirim pesan suhu ke telegram

int Bot_mtbs = 1000;  //digunakan untuk mengatur frekuensi pemanggilan fungsi getUpdates dari objek bot
long Bot_lasttime;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");                    //menampilkan serial monitor
  Serial.println(String(numNewMessages));

  //digunakan untuk menangani pesan-pesan baru yang diterima dari Telegram. Mari kita bahas setiap barisnya
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
      else if (text == "/kipas1on") {                         //text pada telegram yang digunakan sebagai pengontrol
        digitalWrite(kipas1, LOW);                            //memanggil variable pin kipas 1 untuk LOW
        bot.sendMessage(chat_id, "Kipas 1 sudah menyala", "");
      } 
      else if (text == "/kipas1off") {                        //text pada telegram yang digunakan sebagai pengontrol
        digitalWrite(kipas1, HIGH);                           //memanggil variable pin kipas 1 untuk HIGH
        bot.sendMessage(chat_id, "Kipas 1 sudah mati", "");   //tulisan yang muncul pada telegram setelah kipas sudah mati
      }  
      else if (text == "/start") {                            //variable yang muncul setelah mengetik kata start pada telegram 
        String welcome = "Welcome " + from_name + " .\n";     // nama tulisan yang muncul 
        welcome += "/ceksuhu : Cek suhu ruangan\n";           // tulisan yang muncul pada telegram 
        welcome += "/kipas1on : Nyalakan kipas1\n";           // tulisan yang muncul pada telegram 
        welcome += "/kipas1off : Matikan kipas1\n";           // tulisan yang muncul pada telegram 
        bot.sendMessage(chat_id, welcome, "Markdown");        //mengirim ke telegram
      }
    } else {
      // ID Chat tidak valid, kirim pesan ke pengirim
      String errorMessage = "Anda tidak memiliki izin untuk menjalankan perintah ini.";
      bot.sendMessage(chat_id, errorMessage, "");
    }
  }
}

void sendTemperatureToTelegram() { 
  // Pemeriksaan ID Chat sebelum mengirim dan suhu data suhu dari sensor DS18B20 dan mengirimkannya ke pengguna melalui layanan Telegram
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
  pinMode(kipas1, OUTPUT);     // variale kipas1 dipanggil dan digunakan sebagai output
  digitalWrite(kipas1, HIGH);  // kondisi pin diawal 
  pinMode(kipas2, OUTPUT);     // variale kipas2 dipanggil dan digunakan sebagai output
  digitalWrite(kipas2, LOW);   // kondisi pin diawal 
  Serial.begin(115200);        // menginisialisasi komunikasi serial pada perangkat ESP8266 dengan kecepatan baud 11520


  // menginisialisasi koneksi WiFi dan komponen lainnya
  DS18B20.begin();
  Wire.begin();
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.print("Connecting Wifi: ");    //menampilkan pada serial monitor saat terkoneksi dengan wifi
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //menunggu hingga perangkat ESP8266 berhasil terhubung ke jaringan Wi-Fi sebelum melanjutkan eksekusi program
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("");
    Serial.println("ESP8266 sudah terkonesi dg Wifi!");  
    Serial.println("IP address esp8266 : ");
    Serial.println(WiFi.localIP());  
    }
  
  //menginisialisasi layar OLED menggunakan perpustakaan Adafruit_SSD1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      delay(1000);
    }
  }
  
  // mengatur tampilan pada layar OLED setelah perangkat berhasil terhubung ke jaringan Wi-Fi
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
  //membaca suhu dari sensor DS18B20, mengonversinya ke Celsius dan Fahrenheit, dan kemudian menampilkannya di Serial Monitor.
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  Fahrenheit = DS18B20.toFahrenheit(temp);
  Serial.println(temp);
  Serial.println(Fahrenheit);

  // mengonversi nilai suhu (temp) menjadi string yang akan ditampilkan di layar OLED
  tempString = String(temp, 2);
  tempString += (char)247;
  tempString += "C";


  //mengubah tampilan di layar OLED jika nilai tempString berbeda dengan nilai sebelumnya (tempString1).
  if (tempString != tempString1) {
    display.setTextColor(BLACK);  //Menampilkan text color hitam
    displayDisplayCenter(tempString1, 28);
  }

 
  tempString1 = tempString;    // memperbarui nilai tempString1 dengan nilai tempString saat ini 
  Serial.println(tempString);  // mencetak nilai tempString ke Serial Monitor
  display.setTextColor(WHITE); // menampilkan nilai tempString di layar OLED dengan warna teks putih
  displayDisplayCenter(tempString, 28);


  //Potongan kode ini memeriksa apakah sudah waktunya mengirim data suhu ke Telegram
  if (millis() - lastTemperatureSendTime >= temperatureSendInterval) {
    lastTemperatureSendTime = millis();
    sendTemperatureToTelegram();
  }

  //perintah dimana jika suhu 35 derajat maka kipas 2 akan HIGH dan akan mengirimkan pesan
  if (temp > 35.00) {
    digitalWrite(kipas2, HIGH);               //jika suhu 35 derajat maka kipas2 akan HIGH
    for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
      String chat_id = allowedChats[i];
      if (chat_id != "") {
        bot.sendMessage(chat_id, "Kipas 2 aktif.", ""); //mengirim pesan ke telegram
      }
    }
  } 
  else if (temp < 30.00) {
    digitalWrite(kipas2, LOW); //Jika suhu sudah menyentuh dibawah 30 derajat maka kipas akan LOW
  }


  if (temp > 35.00) {                                           // ketika suhu (temp) melebihi 35.00 derajat Celsius
    bot.sendChatAction(allowedChats[0], "Sedang mengetik...");  //indikasi bahwa bot sedang memproses pesan atau tindakan
    Serial.print("Suhu saat ini : ");                           //menampilkan pada serial monitor
    Serial.println(temp);
    delay(2000);

    String suhu = "Intensitas suhu : ";   //inisialisasi string suhu dengan teks awal "Intensitas suhu
    suhu += String(temp, 2);              // Menambahkan nilai suhu (temp) ke dalam string suhu dengan dua digit di belakang koma
    suhu += " *C\n";                      //Menambahkan teks " *C" (mengindikasikan satuan Celsius) dan karakter newline ("\n") ke string suhu
    suhu += "Suhu maksimal\n";            //Menambahkan teks "Suhu maksimal" dan karakter newline ("\n") ke string suhu
    
    for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {  // Loop untuk mengirim pesan ke setiap ID chat yang diizinkan
      String chat_id = allowedChats[i];                                         //Mengambil ID chat dari array allowedChats
      if (chat_id != "") {                                                      //Memeriksa apakah ID chat tidak kosong
        bot.sendMessage(chat_id, suhu, "");                                     //Mengirim pesan ke ID chat dengan isi pesan suhu
      }
    }
    Serial.print("Mengirim data sensor ke telegram");                           //menampilkan pada serial monitor
  }


  //memeriksa pesan-pesan baru dari bot dan menjalankan fungsi handleNewMessages untuk memproses pesan-pesan
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

void displayDisplayCenter(String text, int posisi) {            //menampilkan teks secara pusat pada layar OLED
  int16_t x1, y1;             //Mendeklarasikan dua variabel bertipe data int16_t (x1 dan y1) yang akan digunakan untuk menyimpan koordinat titik paling kiri atas dari teks
  uint16_t width, height;     // Mendeklarasikan dua variabel bertipe data uint16_t (width dan height) yang akan digunakan untuk menyimpan lebar dan tinggi kotak pembatas teks

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);  // untuk mendapatkan batas teks yang akan ditampilkan

  display.setCursor((SCREEN_WIDTH - width) / 2, posisi);  //Menetapkan posisi kursor untuk menampilkan tek
  display.println(text);                                  // Menampilkan teks dengan menggunakan metode println dari objek display
  display.display();                                      //Memperbarui tampilan layar OLED agar perubahan yang dilakukan terlihat
}

bool isAllowedChat(String chatId) {
  //memeriksa apakah suatu ID chat tertentu termasuk dalam daftar ID chat yang diizinkan (allowedChats)
  // Mengecek apakah chatId ada dalam array allowedChats
  for (int i = 0; i < sizeof(allowedChats) / sizeof(allowedChats[0]); i++) {
    if (allowedChats[i] == chatId) {
      return true;
    }
  }
  return false;
}
