#include <Wire.h>
#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>


#define _SSID "Tony House"          // Your WiFi SSID
#define _PASSWORD "songvanminh"      // Your WiFi Password
#define REFERENCE_URL "https://electratech-87423-default-rtdb.asia-southeast1.firebasedatabase.app/"  // Your Firebase project reference url


Firebase firebase(REFERENCE_URL);
AsyncWebServer server(80);


char currentStatus;
int socket1;
int socket2;
int socket3;


String URL = "";


int socket1State = D6;
int socket2State = D5;
int socket3State = D0;


int timezone = 7 * 3600;
int dst = 0;


float power1 = 0;
float power2 = 0;
float power3 = 0;


int lastDay = 0;
int hour1[24];
int hour2[24];
int hour3[24];


int countIndex1 = 0;
int countIndex2 = 0;
int countIndex3 = 0;


bool stateAuto1 = false;
bool stateAuto2 = false;
bool stateAuto3 = false;


int tempTime = 0;
int lastTime = 0;


String lostPower = "";
bool checkLost = false;


String userName = "";
String passwordDevice = "";
String socketName;
bool checkSystem = false;
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";


// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
    <title>ESP Input Form</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    </head><body>
    <form action="/get">
      Username: <input type="text" name="input1">
      <input type="submit" value="Submit">
    </form><br>
  </body></html>)rawliteral";


const char passwordWeb[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
    <title>ESP Input Form</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    </head><body>
    <form action="/get">
      Password: <input type="text" name="input2">
      <input type="submit" value="Submit">
    </form><br>
  </body></html>)rawliteral";


const char setWeb[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
    <title>ESP Input Form</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    </head><body>
    <form action="/get">
      <input type="submit" value="Set">
    </form><br><br>
  </body></html>)rawliteral";


  void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
  }


void setup() {
  Serial.begin(9600); /* begin serial for debug */

  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);


  // Connect to WiFi
  WiFi.begin(_SSID, _PASSWORD);
  Serial.println(millis());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }


  // Connect to time zone
  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  while(!time(nullptr)){
     Serial.print("*");
     delay(300);
  }
  delay(2000);


  //Intialize EEPROM
  EEPROM.begin(50);
  delay(50);


  Serial.println(EEPROM.read(0));
  Serial.println(EEPROM.read(1));
  Serial.println(EEPROM.read(2));
  Serial.println(EEPROM.read(3));
  Serial.println(EEPROM.read(4));
  Serial.println(EEPROM.read(5));
  Serial.println(EEPROM.read(6));
  Serial.println(EEPROM.read(7));
  Serial.println(EEPROM.read(8));
  Serial.println(EEPROM.read(9));
  Serial.println(EEPROM.read(12));
  Serial.println(EEPROM.read(13));
  Serial.println(EEPROM.read(14));

  // USE TO DISPLAY WEBSITE SIGN UP
  if(EEPROM.read(0) == 0) {
    // Send web page with input fields to client
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });


    // pw
    server.on("/pw", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", passwordWeb);
    });


    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/pw/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      String inputParam;


      // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
      if (request->hasParam(PARAM_INPUT_2)) {
        passwordDevice = request->getParam(PARAM_INPUT_2)->value();
        inputParam = PARAM_INPUT_2;
      }


      Serial.println(passwordDevice);


      request->send(200, "text/html", "Finish sign up");
    });


    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {


      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam(PARAM_INPUT_1)) {
        userName = request->getParam(PARAM_INPUT_1)->value();\
        saveToEEPROM(userName, 0);
        Serial.println("u: " + userName);
        Serial.println("passwordDevice: " + passwordDevice);
        request->send(200, "text/html", "Continue input password"
                                      "<br><a href=\"/pw\">Click to input password<a>");
      } else if(request->hasParam(PARAM_INPUT_2)) {
        passwordDevice = request->getParam(PARAM_INPUT_2)->value();
        saveToEEPROM(passwordDevice, 12);
        Serial.println("u: " + userName);
        Serial.println("passwordDevice: " + passwordDevice);
      }
    }
    );


    server.onNotFound(notFound);
    server.begin();
  } else {

    // Web to reset the username and password.
    checkSystem = true;
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", setWeb);
    });


    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        for(int i = 0; i < 100; i++) {
          EEPROM.write(i, 0);
          EEPROM.commit();
        }
        request->send(200, "text/html", "Set done");
        Serial.println(EEPROM.read(0));
        Serial.println(EEPROM.read(1));
        Serial.println(EEPROM.read(2));
        Serial.println(EEPROM.read(3));
    });


    server.onNotFound(notFound);
    server.begin();


   // Check the username and the name of the socket
    int indexUserName = 0;
    while(EEPROM.read(indexUserName) != 0) {
      userName += char(EEPROM.read(indexUserName));
      indexUserName++;
      Serial.println(userName);
      Serial.println(EEPROM.read(indexUserName + 1));
    }


      // Get first data
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);


    String day = String(p_tm->tm_mday) + "-" + String(p_tm->tm_mon + 1) + "-" + String(p_tm->tm_year + 1900);
    String hour = String(p_tm->tm_hour) + "h-" + String(p_tm->tm_hour + 1) + "h";
    String URLToFireBase = "PowerProviders/" + userName + "1" + "/ElectricAmount/" + String(day) + "/" + String(hour);
    int sec = p_tm->tm_sec;
    int time = calculateTime(tempTime, sec);
    tempTime = sec;


    String data = firebase.getString(URLToFireBase);


    Serial.println(data);
    Serial.println(URLToFireBase);


    // store previous data to the eeprom.
    int temp = 0;
    int status = false;
    String mydata = "";
    for(int i = 0; i < data.length(); i++) {
      char c = data.charAt(i);
      if(status == true) {
        mydata += c;
      }


      if(c == ',') {
        status = true;
        temp++;
        mydata = "";
      }
     
      if(data.charAt(i + 1) == 'W') {
        switch (temp) {
          case 1:
              power1 = (mydata.toFloat());
            break;
          case 2:
              power2 = (mydata.toFloat());
            break;
          case 3:
              power3 = (mydata.toFloat());
            break;
        }
       
        status = false;
      }
    }


    pinMode(socket1State, OUTPUT);
    pinMode(socket2State, OUTPUT);
    pinMode(socket3State, OUTPUT);
  }
}
 
void loop() {
  if(checkSystem == true) {
    // Get data about change of status
    String data = firebase.getString("PowerProviders/" + userName + "1" + "/StatusFromWeb");
    Serial.println(data);


    // process data and turn on, off
    int countStatus = 1;
   
    for(int i = 0; i < data.length(); i++) {
      char c = data.charAt(i);
     
      if(data.charAt(i - 1) == '.') {
        switch (countStatus) {
          case 1:
            if(c == 't') {
              digitalWrite(socket1State, LOW);
            } else {
              digitalWrite(socket1State, HIGH);
            }
            break;
          case 2:
            if(c == 't') {
              digitalWrite(socket2State, HIGH);
            } else {
              digitalWrite(socket2State, LOW);
            }
            break;
          case 3:
            if(c == 't') {
              digitalWrite(socket3State, HIGH);
            } else {
              digitalWrite(socket3State, LOW);
            }
            break;
        }
        countStatus++ ;
      }
    }


    //get the data from arduino
    Serial.println("Begin request");
    Wire.requestFrom(8, 24);
    String str = "";
    while(Wire.available()) {
      char c = Wire.read();
      str += String(c);
    }
    Serial.println(str);




    // get the time and url
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);


    String day = String(p_tm->tm_mday) + "-" + String(p_tm->tm_mon + 1) + "-" + String(p_tm->tm_year + 1900);
    String hour = String(p_tm->tm_hour) + "h-" + String(p_tm->tm_hour + 1) + "h";
    int theTime = p_tm->tm_hour;
    int sec = p_tm->tm_sec;
    String URLToFireBase = "PowerProviders/" + userName + "1" + "/ElectricAmountShow/" + String(p_tm->tm_hour) + "/" + String(p_tm->tm_min) + "/" +  String(sec);
    String dayOfWeek = String(p_tm->tm_wday);
    int time = calculateTime(tempTime, sec);
    tempTime = sec;


    // auto turn off base on firebase 
    String timeAutoTurnOff = firebase.getString("Recommend/" + userName + "1" + "/Friday");
    if(dayOfWeek.toInt() != lastDay) {
      countIndex1 = 0;
      countIndex2 = 0;
      countIndex3 = 0;
      processTime(timeAutoTurnOff);
    }
    lastDay = dayOfWeek.toInt();
   
    if(theTime != lastTime) {
      power1 = 0;
      power2 = 0;
      power3 = 0;
      if(arrayContainValue(hour1, theTime)) {
        stateAuto1 = true;
      }


      if(arrayContainValue(hour2, theTime)) {
        stateAuto2 = true;
      }


      if(arrayContainValue(hour3, theTime)) {
        stateAuto3 = true;
      }
    }


    lastTime = theTime;


    if(stateAuto1 == true) {
      digitalWrite(socket1State, HIGH);
      stateAuto1 = false;
    }
    if(stateAuto2 == true) {
      digitalWrite(socket2State, LOW);
      stateAuto2 = false;
    }
    if(stateAuto3 == true) {
      digitalWrite(socket3State, LOW);
      stateAuto3 = false;
    }


    //Get data on off
    int valueSocket1 = digitalRead(socket1State);
    int valueSocket2 = digitalRead(socket2State);
    int valueSocket3 = digitalRead(socket3State);


    String TheSocketStatus = "";


    if(valueSocket1 == 0) {
      TheSocketStatus += "1.t ";
    } else {
      TheSocketStatus += "1.f ";
    }


    if(valueSocket2 == 1) {
      TheSocketStatus += "2.t ";
    } else {
      TheSocketStatus += "2.f ";
    }
   
    if(valueSocket3 == 1) {
      TheSocketStatus += "3.t";
    } else {
      TheSocketStatus += "3.f";
    }
   




    // process string of data have from arduino
    int temp = 0;
    int count = 0;
    String myStr = "";
   
    for(int i = 0; i < str.length(); i++) {
      char c = str.charAt(i);
     
      if(count == 1) {
        myStr += c;
      }


      if(c == ',') {
        count = 1;
        temp++;
        myStr = "";
      }
     
      if(str.charAt(i + 1) == 'k') {
        switch (temp) {
          case 1:
            if(valueSocket1 == 0 && myStr.toFloat() > 0.1) {
              power1 += (myStr.toFloat() * 220 * time);
            } else {
              power1 += (0.01 * 220 * time);
            }
            break;
          case 2:
            if(valueSocket2 == 1 && myStr.toFloat() > 0.1) {
              power2 += (myStr.toFloat() * 220 * time);
            } else {
              power2 += (0.01 * 220 * time);
            }
            break;
          case 3:
            if(valueSocket3 == 1 && myStr.toFloat() > 0.1) {
              power3 += (myStr.toFloat() * 220 * time);
            } else {
              power3 += (0.01 * 220 * time);
            }
            break;
        }
        count = 0;
      }
    }


    // Send to firebase Ws
    String passToFirebase = String("1,") + String(power1) + String("Ws ") +
    String("2,") + String(power2) + String("Ws ") +
    String("3,") + String(power3) + String("Ws ");


    firebase.setString(URLToFireBase, passToFirebase);
    firebase.setString("PowerProviders/" + userName + "1" + "/Status", TheSocketStatus);
  }
  if(checkSystem == false && userName != "" && passwordDevice != "") {
    firebase.setString("UserAccount/" + userName + "/password", passwordDevice);
    firebase.setString("UserAccount/" + userName + "/Providers", userName + "1");
    firebase.setString("PowerProviders/" + userName + "1" + "/StatusFromWeb", "1.t 2.t 3.t");
    firebase.setString("PowerProviders/" + userName + "1" + "/socketName", "socket1 - socket2 - socket3");
  }
}


int calculateTime(int firstSec, int secondSec) {
  if(firstSec > secondSec) {
    return(60 - (firstSec - secondSec));
  } else {
    return(secondSec - firstSec);
  }
}


void processTime (String s) {
  int count = 0;
  bool checkMode = false;
  char temp;
  String hoursInterval = "";




  for(int i = 0; i < s.length(); i++) {


    if(s.charAt(i) == ']') {
      checkMode = false;
     
      switch(count) {
        case 1:
          Serial.println("hI:  " + hoursInterval);
          subStringToArray(hoursInterval, 1);
          break;
        case 2:
          subStringToArray(hoursInterval, 2);
          break;
        case 3:
          subStringToArray(hoursInterval, 3);
          break;
      }
      hoursInterval = "";
    }


    if(checkMode == true) {
      hoursInterval += String(s.charAt(i));
    }




    if(s.charAt(i) == '[') {
      checkMode = true;
      count++;
    }
  }
}


void subStringToArray(String s, int choice) {
  int count = 0;
  bool check = false;
  String str = "";
  for(int i = 0; i < s.length(); i++) {
    if(check == true) {
      str += String(s.charAt(i));
    }
   
    if(s.charAt(i) == '"') {
      if(count % 2 == 0) {
        check = true;
      } else {
        check = false;
        Serial.println(str);
        stringAToHourA(str, choice);
        str = "";
      }
      count ++ ;
    }
  }
}


void stringAToHourA(String s, int choice) {
  int count = 0;
  int first = 0;
  int second = 0;
  bool check = true;
  String str = "";
  int countAssign = 0;


  for(int i = 0; i < s.length(); i++) {
    if(check == true) {
      str += String(s.charAt(i));
    }
   
    if(s.charAt(i + 1) == 'h') {
      Serial.println("num: " + str);
      if(count == 0) {
        first = str.toInt();
      } if(count == 1) {
        second = str.toInt();
      }
      check = false;
      str = "";
      count++;
    }


    if(s.charAt(i) == '-') {
      check = true;
    }
  }


  for(int i = first; i <= second; i++) {
    switch(choice) {
      case 1:
        hour1[countIndex1] = i;
        countIndex1++;
        break;
      case 2:
        hour2[countIndex2] = i;
        countIndex2++;
        break;
      case 3:
        hour3[countIndex3] = i;
        countIndex3++;
        break;
    }
  }
}


bool arrayContainValue(int arr[], int iss) {
  for(int i = 0; i < 24; i++) {
    if(iss == arr[i]) {
      return true;
      break;
    }
  }
  return false;
}


void saveToEEPROM(String s, int begin) {
  for(int i = begin; i < s.length() + begin; i++) {
    EEPROM.write(i, s.charAt(i - begin));
    EEPROM.commit();
    Serial.println(String(i) + ": " + EEPROM.read(i));
    Serial.println(String(i) + ": " + s.charAt(i - begin));
    delay(50);
  }
}


String translateToString(int number) {
  Serial.println("number " + String(number));
  return String(number);
}
