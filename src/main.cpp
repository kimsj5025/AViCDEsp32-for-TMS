#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "HX711.h"

// --- 설정 변수 (사용자 환경에 맞게 수정) ---
const char* ap_ssid = "AViCDEsp32"; // 생성할 Wi-Fi 네트워크 이름
const char* ap_password = "12345678";   // 생성할 Wi-Fi 네트워크 비밀번호 (8자 이상)

// HX711 핀 연결
#define LOADCELL_DOUT_PIN 17    // DT
#define LOADCELL_SCK_PIN  18    // SCK

// 점화(IGNITE) MOSFET 게이트 핀
#define IGNITE_PIN         3    // MOSFET G(Gate)에 연결

// --- 전역 변수 ---
HX711 scale;
WebServer server(80); // 웹 서버 객체

float calibration_factor = 6600; // 보정 계수
bool ignition_status = false;    // 점화 상태

// 데이터를 메모리에 저장하기 위한 변수
String dataLog = "Time (ms),Weight (kg)\n";
float current_weight = 0;
unsigned long current_time = 0;

// 10Hz 로깅을 위한 시간 변수
unsigned long previousMillis = 0;
const long interval = 100; // 100ms = 10Hz

// --- 웹 서버 핸들러 함수 프로토타입 선언 ---
void handleRoot();
void handleIgnite();
void handleExtinguish();
void handleData();
void handleDownload();
void handleNotFound();


void setup() {
  Serial.begin(115200);
  pinMode(IGNITE_PIN, OUTPUT);
  digitalWrite(IGNITE_PIN, LOW);

  // --- Wi-Fi 액세스 포인트(AP) 모드 시작 ---
  Serial.println();
  Serial.print("Setting up Access Point...");
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP Ready.");
  Serial.print("AP SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // --- HX711 초기화 ---
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("Calibrating scale... Do not apply any weight.");
  scale.set_scale();
  scale.tare();
  delay(1000);
  scale.set_scale(calibration_factor);
  Serial.println("Calibration complete.");

  // --- 웹 서버 핸들러 설정 ---
  server.on("/", HTTP_GET, handleRoot);
  server.on("/ignite", HTTP_GET, handleIgnite);
  server.on("/extinguish", HTTP_GET, handleExtinguish);
  server.on("/data", HTTP_GET, handleData); // 실시간 데이터 전송용
  server.on("/download", HTTP_GET, handleDownload); // 데이터 다운로드용
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // 웹 서버 요청을 지속적으로 처리
  server.handleClient();

  // 점화 핀 제어
  if (ignition_status) {
    digitalWrite(IGNITE_PIN, HIGH);
  } else {
    digitalWrite(IGNITE_PIN, LOW);
  }

  // 10Hz 주기에 맞춰 데이터 측정 및 로깅 (Non-blocking)
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // 무게 측정
    current_weight = scale.get_units(); 
    current_time = currentMillis;
    
    // 측정값을 CSV 형식으로 메모리에 누적
    dataLog += String(current_time) + "," + String(current_weight, 4) + "\n";
  }
}

// --- 웹 서버 핸들러 함수들 ---

// 루트 페이지 핸들러 (HTML, CSS, JavaScript 포함)
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>GPX.SPACE Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background-color: #121212; color: #e0e0e0; margin: 0; padding: 20px; }
    .container { max-width: 800px; margin: auto; }
    h1 { color: #bb86fc; text-align: center; }
    .card { background-color: #1e1e1e; border-radius: 12px; padding: 20px; margin-bottom: 20px; box-shadow: 0 4px 20px rgba(0,0,0,0.3); }
    .controls { display: flex; justify-content: center; gap: 15px; margin-bottom: 20px; }
    .button { border: none; color: white; padding: 12px 24px; text-align: center; font-size: 16px; cursor: pointer; border-radius: 8px; transition: all 0.2s ease; font-weight: bold; }
    .btn-ignite { background-color: #cf6679; } .btn-ignite:hover { background-color: #c34257; }
    .btn-extinguish { background-color: #03dac6; } .btn-extinguish:hover { background-color: #01bfa9; }
    .btn-download { display: block; width: fit-content; margin: 20px auto; text-decoration: none; background-color: #373737; }
    .btn-download:hover { background-color: #4f4f4f; }
    #status { text-align: center; font-size: 1.2em; margin-bottom: 20px; }
    #table-container { max-height: 50vh; overflow-y: auto; border: 1px solid #373737; border-radius: 8px;}
    table { width:100%; border-collapse: collapse; }
    th, td { padding: 8px 12px; text-align: left; border-bottom: 1px solid #373737; }
    th { position: sticky; top: 0; background-color: #2a2a2e; }
    tr:nth-child(even) { background-color: #2a2a2e; }
  </style>
</head>
<body>
  <div class="container">
    <h1>AViCDEsp32 Control Center (ACC)</h1>
    
    <div class="card">
      <h2>Ignition Controls</h2>
      <div id="status">Status: <span style="color:#cf6679;">IDLE</span></div>
      <div class="controls">
        <button class="button btn-ignite" onclick="sendCommand('/ignite')">IGNITE</button>
        <button class="button btn-extinguish" onclick="sendCommand('/extinguish')">Disable</button>
      </div>
    </div>

    <div class="card">
      <h2>Real-time Data</h2>
      <div id="table-container">
        <table id="data-table">
          <thead>
            <tr>
              <th>Time (s)</th>
              <th>Weight (kg)</th>
            </tr>
          </thead>
          <tbody id="data-table-body">
          </tbody>
        </table>
      </div>
    </div>
    
    <a href="/download" class="button btn-download">Download Data (CSV)</a>
  </div>

<script>
  function sendCommand(command) {
    const statusElem = document.querySelector('#status span');
    fetch(command)
      .then(response => response.text())
      .then(data => {
        console.log(data);
        if (command === '/ignite') {
          statusElem.textContent = "IGNITION ACTIVE";
          statusElem.style.color = '#03dac6';
        } else {
          statusElem.textContent = "DEACTIVE";
          statusElem.style.color = '#cf6679';
        }
      });
  }

  const tableBody = document.getElementById('data-table-body');
  setInterval(function() {
    fetch('/data')
      .then(response => response.json())
      .then(data => {
        const timeInSeconds = (data.time / 1000).toFixed(2);
        const weight = data.weight.toFixed(4);

        const newRow = tableBody.insertRow(0); // Add new row to the top
        
        const timeCell = newRow.insertCell(0);
        const weightCell = newRow.insertCell(1);
        
        timeCell.innerHTML = timeInSeconds;
        weightCell.innerHTML = weight;

        // Limit the number of rows in the table to 100
        if(tableBody.rows.length > 100) {
          tableBody.deleteRow(tableBody.rows.length - 1);
        }
      })
      .catch(error => console.error('Error fetching data:', error));
  }, 100); // Fetch data every 100ms (10Hz) to match sensor rate
</script>
</body>
</html>
  )rawliteral";
  server.send(200, "text/html", html);
}

// 실시간 데이터 전송 핸들러
void handleData() {
  // 최신 측정값을 JSON 형식으로 전송
  String json = "{\"time\":" + String(current_time) + ", \"weight\":" + String(current_weight, 4) + "}";
  server.send(200, "application/json", json);
}

// 데이터 다운로드 핸들러
void handleDownload() {
  server.sendHeader("Content-Disposition", "attachment; filename=thrust_data.csv");
  server.send(200, "text/csv", dataLog);
}

// 점화 핸들러
void handleIgnite() {
  ignition_status = true;
  Serial.println("IGNITION command received from web.");
  server.send(200, "text/plain", "Ignition ON");
}

// 소화 핸들러
void handleExtinguish() {
  ignition_status = false;
  Serial.println("EXTINGUISH command received from web.");
  server.send(200, "text/plain", "Ignition OFF");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}
