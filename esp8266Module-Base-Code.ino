//INCLUDES

#include "U8glib.h"
#include <SoftwareSerial.h>

// OBJECTS
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);	// Display which does not send AC
SoftwareSerial esp8266Module(10, 11); // RX, TX

// GLOBALS
String network = "---";
String password = "---";
bool breakOut = false;
int wifiStatus = 0;

// Temp variables
String val1 = "----------";
String val2 = "----------";
String val3 = "----------";
String val4 = "----------";
int loopNum = 0;
void setup() {
  // Serial Start
  Serial.begin(9600);
  esp8266Module.begin(9600);
  delay(10000);
  // Serial end
}

void loop() {
  // picture loop
  u8g.firstPage();
  do {
    displayShow();
  } while ( u8g.nextPage() );
  runEsp8266("www.davidjwatts.com", "/arduino/esp8266.php");
}

// START DISPLAY
void displayShow() {
  u8g.setFont(u8g_font_helvB08);
  u8g.setPrintPos(0, 8);
  String loops = "Loops:";
  u8g.print(loops + loopNum);
  u8g.setFont(u8g_font_5x8);
  u8g.setPrintPos(0, 18);
  u8g.print(val1);
  u8g.setFont(u8g_font_5x8);
  u8g.setPrintPos(0, 50);
  u8g.print(val2);
  u8g.setPrintPos(0, 60);
  u8g.print(val3);

}
// END DISPLAY

// MAIN ESP8266 FUNCTION
void runEsp8266(String website, String page) {
  // 0 need to reset or beginning of loop
  // 1 reset complete check wifi mode
  // 2 wifi mode is 3, now check network connection
  // 3 If not connected connect to network
  // 4 request page from server
  // 5 unlink from server after request
  // 6 close network connection
  switch (wifiStatus) {
    case 0:    // 0 need to reset or beginning of loop
      Serial.println("TRYING esp8266Reset");
      esp8266Reset();
      break;
    case 1:    // 1 reset complete check wifi mode
      Serial.println("TRYING changeWifiMode");
      changeWifiMode();
      break;
    case 2:    // 2 wifi mode is 3, now check network connection
      Serial.println("TRYING checkWifiStatus");
      checkWifiStatus();
      break;
    case 3:    // 3 If not connected connect to network
      Serial.println("TRYING connectToWifi");
      connectToWifi();
      //connectToWifi("networkIdetifier", "networkPassword");
      break;
    case 4:    // 4 request page from server
      Serial.println("TRYING getPage");
      getPage(website, page);
      //getPage(website, page, "?num=", "3", "&num2=", "2000");
      break;
    case 5:    // 5 unlink from server after request
      Serial.println("TRYING unlinkPage");
      unlinkPage();
      break;
  }
}

// END MAIN ESP8266 FUNCTION

// 0 - RESET
bool esp8266Reset() {
  esp8266Module.println(F("AT+RST"));
  delay(5000);
  if (esp8266Module.find("OK"))
  {
    val1 = F("-RESET-");
    wifiStatus = 1;
    return true;
  }
  else
  {
    val1 = F("-FAILED-");
    wifiStatus = 0;
    return false;
  }
}
// END RESET

// 1 - CHANGE MODE
bool changeWifiMode()
{
  esp8266Module.println(F("AT+CWMODE?"));
  delay(5000);
  if (esp8266Module.find("3"))
  {
    val1 = F("Wifi Mode is 3");
    wifiStatus = 2;
    return true;
  }
  else
  {
    esp8266Module.println(F("AT+CWMODE=3"));
    delay(5000);
    if (esp8266Module.find("no change") || esp8266Module.find("OK"))
    {
      val1 = F("Wifi Mode is 3");
      wifiStatus = 2;
      return true;
    }
    else
    {
      val1 = F("Wifi Mode failed");
      wifiStatus = 0;
      return false;
    }
  }

}
// END CHANGE MODE

// 2 - CHECK WIFI NETWORK STATUS
bool checkWifiStatus() {
  esp8266Module.println("AT+CWJAP?");
  delay(5000);
  if (esp8266Module.find(":")) {
    Serial.println("WIFI NETWORK CONNECTED");
    val1 = F("WIFI:");
    val1 += esp8266Module.readStringUntil('\n');
    wifiStatus = 4;
    return true;
  }
  else
  {
    wifiStatus = 3;
    return false;
  }
}
// END CHECK WIFI NETWORK STATUS

// 3 - CONNECT TO WIFI
bool connectToWifi() {
  String cmd = F("AT+CWJAP=\"");
  cmd += network;
  cmd += F("\",\"");
  cmd += password;
  cmd += F("\"");
  esp8266Module.println(cmd);
  delay(5000);
  if (esp8266Module.find("OK"))
  {
    Serial.println("CONNECTED TO WIFI");

    val1 = F("CONNECTED TO WIFI");
    wifiStatus = 4;
    return true;
  }
  else
  {
    wifiStatus = 0;
    return false;
  }
}
// optional function that accepts the network ID and password as variables
bool connectToWifi(String networkId, String networkPassword) {
  String cmd = F("AT+CWJAP=\"");
  cmd += networkId;
  cmd += F("\",\"");
  cmd += networkPassword;
  cmd += F("\"");
  esp8266Module.println(cmd);
  delay(5000);
  if (esp8266Module.find("OK"))
  {
    Serial.println("CONNECTED TO WIFI");

    val1 = F("CONNECTED TO WIFI");
    wifiStatus = 4;
    return true;
  }
  else
  {
    wifiStatus = 0;
    return false;
  }
}
// END CONNECT TO WIFI NETWORK

// 4 - GET PAGE
bool getPage(String website, String page) {
  String cmd = F("AT+CIPSTART=\"TCP\",\"");
  cmd += website;
  cmd += F("\",80");
  esp8266Module.println(cmd);
  delay(5000);
  if (esp8266Module.find("Linked"))
  {
    Serial.print("Connected to server");

  }
  cmd =  "GET ";
  cmd += page;
  cmd += "?num=1";  //construct the http GET request
  cmd += " HTTP/1.0\r\n";
  cmd += "Host:";
  cmd += website;
  cmd += "\r\n\r\n";        
  Serial.println(cmd);  
  esp8266Module.print("AT+CIPSEND=");
  esp8266Module.println(cmd.length());
  Serial.println(cmd.length());

  if (esp8266Module.find(">"))  
  {
    Serial.println("found > prompt - issuing GET request");  
    esp8266Module.println(cmd); 
  }
  else
  {
    wifiStatus = 5;
    Serial.println("No '>' prompt received after AT+CPISEND");
    val1 = F("Failed request, retrying...");
    return false;
  }

  while (esp8266Module.available() > 0)
  {
    esp8266Module.read();
  }

  if (esp8266Module.find("*")) {
    String tempMsg = esp8266Module.readStringUntil('\n');
    val2 = splitToVal(tempMsg, "@", "|");
    val3 = splitToVal(tempMsg, "+", "@");
    String piecetemp = splitToVal(tempMsg, "|", "$");
    int peice = piecetemp.toInt();
    Serial.println(val2);
    Serial.println(val3);
    Serial.println(piecetemp);
    wifiStatus = 5;
    return true;
  }
  else {
    wifiStatus = 5;
    return false;
  }

}

// optional getPage function that accepts user variables
bool getPage(String website, String page, String urlVariableName1, String variable1, String urlVariableName2, String variable2) {
  String cmd = F("AT+CIPSTART=\"TCP\",\"");
  cmd += website;
  cmd += F("\",80");
  esp8266Module.println(cmd);
  delay(5000);
  if (esp8266Module.find("Linked"))
  {
    Serial.print("Connected to server");

  }
  cmd =  "GET ";
  cmd += page;
  cmd += urlVariableName1;  // something like ?num=
  cmd += variable1;
  cmd += urlVariableName2;  // something like &num2=
  cmd += variable2;
  cmd += " HTTP/1.0\r\n";
  cmd += "Host:";
  cmd += website;
  cmd += "\r\n\r\n";        
  Serial.println(cmd); 
  esp8266Module.print("AT+CIPSEND=");
  esp8266Module.println(cmd.length());
  Serial.println(cmd.length());

  if (esp8266Module.find(">"))
  {
    Serial.println("found > prompt - issuing GET request");
    esp8266Module.println(cmd);  
  }
  else
  {
    wifiStatus = 5;
    Serial.println("No '>' prompt received after AT+CPISEND");
    val1 = F("Failed request, retrying...");
    return false;
  }

  while (esp8266Module.available() > 0)
  {
    esp8266Module.read();
  }

  if (esp8266Module.find("*")) {
    String tempMsg = esp8266Module.readStringUntil('\n');
    val2 = splitToVal(tempMsg, "@", "|");
    val3 = splitToVal(tempMsg, "+", "@");
    String piecetemp = splitToVal(tempMsg, "|", "$");
    int peice = piecetemp.toInt();
    Serial.println(val2);
    Serial.println(val3);
    Serial.println(piecetemp);
    wifiStatus = 5;
    return true;
  }
  else {
    wifiStatus = 5;
    return false;
  }

}

// END GET PAGE

// 5 - UNLINK
bool unlinkPage() {
  esp8266Module.println(F("AT+CIPCLOSE"));
  delay(5000);
  if (esp8266Module.find("Unlink"))
  {
    val1 = F("UNLINKED");
    wifiStatus = 0;
    loopNum++;
    return true;
  }
  else
  {
    wifiStatus = 4;
    return false;
  }
}
// END UNLINK

// 6 - CLOSE NETWORK  --- This is not ready
bool closeNetwork() {
  esp8266Module.println(F("AT+CWQAP"));
  delay(5000);
  if (esp8266Module.find("OK"))
  {
    val1 = F("NETWORK DISCONNECTED");
    wifiStatus = 0;
    return true;
  }
  else
  {
    wifiStatus = 4;
    return false;
  }
}
// END CLOSE NETWORK

// 7 - FIND WIFI NETWORKS  --- This is not ready
bool findWifiNetworks() {
  esp8266Module.println("AT+CWJAP");
  delay(15000);
  if (esp8266Module.find(":")) {
    Serial.println("WIFI NETWORK CONNECTED");
    val1 = F("WIFI:");
    val1 += esp8266Module.readStringUntil('\n');
    wifiStatus = 4;
    return true;
  }
  else
  {
    wifiStatus = 3;
    return false;
  }
}
// END CHECK WIFI NETWORK STATUS

// SPLIT UP STRINGS
String splitToVal(String inputString, String delimiter, String endChar) {
  String tempString = "";
  int from;
  int to;
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.substring(i, i + 1) == delimiter) {
      from = i + 1;
    }
    if (inputString.substring(i, i + 1) == endChar) {
      to = i;
    }
  }
  tempString = inputString.substring(from, to);
  return tempString;
}
// END SPLIT UP STRINGS

// SPLIT UP STRINGS --- This is not ready
String splitWifi(String inputWifi) {
  int firstListItem = inputWifi.indexOf("+CWLAP:(");
  int secondListItem = inputWifi.indexOf("item", firstListItem + 1 );
  String indexWifi = "+CWLAP:("; // unhappy face
  String findName = "\"";
 
}
// END SPLIT UP STRINGS
