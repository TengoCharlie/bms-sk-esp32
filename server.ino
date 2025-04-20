void handleRoot() {
  String html = "<html><body>";
  html += "<h1>INA219 Sensor Readings</h1>";
  html += "<p>Cell 1 -> Voltage: " + String(cellVoltage_1) + " V, Current: " + String(current_mA_1) + " mA</p>";
  html += "<p>Cell 2 -> Voltage: " + String(cellVoltage_2) + " V, Current: " + String(current_mA_2) + " mA</p>";
  html += "<p>Cell 3 -> Voltage: " + String(cellVoltage_3) + " V, Current: " + String(current_mA_3) + " mA</p>";
  html += "<p>Cell 4 -> Voltage: " + String(cellVoltage_4) + " V, Current: " + String(current_mA_4) + " mA, Power: " + String(ina219_4.getPower_mW()) + " mW</p>";
  
  // Add temperatures to the web page
  html += "<h2>Temperatures</h2>";
  html += "<p>Temperature 1: " + String(temperature1) + " C</p>";
  html += "<p>Temperature 2: " + String(temperature2) + " C</p>";
  html += "<p>Temperature 3: " + String(temperature3) + " C</p>";
  html += "<p>Temperature 4: " + String(temperature4) + " C</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}