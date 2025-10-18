#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include "time.h"

#define LED 2

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// WFI SET UP //
const char* STA_SSID = "VM4368569";
const char* STA_PASS = "f6gZxJs8xhnw";

// Fallback //
const char* AP_SSID = "DHT22_AP";
const char* AP_PASS = "esp32test";

// UK time zone rules
const char* ntpServer = "pool.ntp.org";
// For UK, 0 for GTM offset and DST offset 3600 sec in summer
const long gmtOffset_sec = 0;         // GMT+0
const int daylightOffset_sec = 3600; // +1h fro BTS

WebServer server(80);

String htmlPage(float t, float h){
  String status = "OK";
   if (isnan(t) || isnan(h)) status = "Sensor error";

  String page = R"HTML(
    <!doctype html>
    <html lang="en">
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Room Climate</title>
    <style>
      body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; margin: 0; padding: 1.2rem; background: #f7f7f7; }
      .card { max-width: 480px; margin: 0 auto; background: white; border-radius: 16px; padding: 1.2rem 1.4rem; box-shadow: 0 6px 20px rgba(0,0,0,0.08); }
      h1 { font-size: 1.2rem; margin: 0 0 .6rem; }
      .grid { display: grid; grid-template-columns: 1fr 1fr; gap: .8rem; }
      .tile { background: #fafafa; border-radius: 12px; padding: 1rem; text-align: center; }
      .big { font-size: 2.2rem; font-weight: 700; }
      .muted { color: #666; font-size: .9rem; }
      .status { margin-top: .6rem; font-size: .9rem; color: #333; }
      .footer { text-align: center; margin-top: 1rem; color: #888; font-size: .8rem; }
      button { margin-top: .8rem; width: 100%; padding: .8rem; border: 0; border-radius: 10px; background: #0a84ff; color: white; font-weight: 600; }
    </style>
    <div class="card">
      <h1>Room Climate (ESP32 + DHT22)</h1>
      <div class="grid">
        <div class="tile">
          <div class="muted">Temperature</div>
          <div id="temp" class="big">--.- °C</div>
        </div>
        <div class="tile">
          <div class="muted">Humidity</div>
          <div id="hum" class="big">--.- %</div>
        </div>
      </div>
      <div class="status">Status: <span id="status">)HTML";
    
      page += status;
      page += R"HTML(</span></div>
      <button onclick="refreshNow()">Refresh Now</button>
      <div class="footer">Auto-updates every 5s - API: <code>/api</code></div>
    </div>
    <script>
    async function fetchData() {
      try {
        const res = await fetch('/api');
        const js = await res.json();
        document.getElementById('temp').textContent = (js.temperature_c ?? '--') + ' °C';
        document.getElementById('hum').textContent  = (js.humidity ?? '--') + ' %';
        document.getElementById('status').textContent = js.ok ? 'OK' : 'Sensor error';
      } catch (e) {
        document.getElementById('status').textContent = 'Network error';
      }
    }
    function refreshNow(){ fetchData(); }
    fetchData();
    setInterval(fetchData, 5000);
    </script>
    </html>
    )HTML";
  return page;
}

void handleRoot(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  server.send(200, "text/html", htmlPage(t,h));
}

void handleApi(){
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius
  bool ok = !(isnan(t) || isnan(h));
  String json = "{";
  json += "\"ok\":" + String(ok ? "true":"false") + ",";
  json += "\"temperature_c\":" + (ok ? String(t,1) : "null") + ",";
  json += "\"humidity\":" + (ok ? String(h,1) : "null");
  json += "}";
  server.send(200, "application/json", json); 
}

void setupWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(STA_SSID, STA_PASS);
  unsigned long start = millis();
  while(WiFi.status() != WL_CONNECTED & millis() - start < 800){
    delay(200);
  }


  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to Wi-Fi. IP: ");
    Serial.println(WiFi.localIP());
    return;
  }

  // Fallback to Access Point
  Serial.println("STA failed, starting AP…");
  WiFi.mode(WIFI_AP);
  bool ap = WiFi.softAP(AP_SSID, AP_PASS);
  if (ap) {
    Serial.print("AP started. SSID: ");
    Serial.print(AP_SSID);
    Serial.print("  Pass: ");
    Serial.println(AP_PASS);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP start failed!");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello from ESP32");
  setupWiFi();

  pinMode(LED, OUTPUT);
  dht.begin();

  server.on("/", handleRoot);
  server.on("/api", handleApi);
  server.begin();
  Serial.println("HTTP server started");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}


void recordTemps(){
  Serial.println("");
  Serial.println("LOCAL READINGS");
  Serial.println("");

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  Serial.print("Temprature = ");
  Serial.print(t, 1);
  Serial.println("°C");

  Serial.print("Humidity = ");
  Serial.print(h, 0);
  Serial.print("%");

//  printData(t,h);
}

void tempCheck(){
  float t = dht.readTemperature();
  if(t > 23.5){
    Serial.print(t);
    Serial.println("Temp too high"); 
  }else if(t < 16){
    Serial.print(t);
    Serial.println("Temp Too Low");
  }else if( t > 19 && t <= 22 ){
    Serial.println("Optimal Day Temp");
  }else if(t > 16 && t <= 19){
    Serial.println("Optimal Night Temp");
  }
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;  
  }
  Serial.println(&timeinfo, "%A, %d %B %Y %H:%M:%S");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);

  recordTemps();
  server.handleClient();
  printLocalTime();
}
