#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <U8x8lib.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "";                     //Change to your WiFi SSID
const char *password = "";             //Change to your WiFi password
const char *url = ""; //Change to your server url
bool startFlag = true;

WiFiClient client;
HTTPClient http;
unsigned long int timestart = 0;
StaticJsonBuffer<2048> jsbf;
uint8_t imgHex[1024] = {};


U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_1_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);

void initWifi(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi does not connect, try again ...");
    delay(3000);
  }

  Serial.println("Wifi is connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int hex2byte(unsigned char bits[], char s[])
{
  int i, n = 0;
  for (i = 0; s[i]; i += 2)
  {
    if (s[i] >= 'A' && s[i] <= 'F')
      bits[n] = s[i] - 'A' + 10;
    else
      bits[n] = s[i] - '0';
    if (s[i + 1] >= 'A' && s[i + 1] <= 'F')
      bits[n] = (bits[n] << 4) | (s[i + 1] - 'A' + 10);
    else
      bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
    ++n;
  }
  //Serial.println("Convert done");
  return n;
}

void freshDisplay()
{
  Serial.println("FreshScreenImg");
  u8g2.firstPage();
  do
  {
    u8g2.drawXBM(21, 0, 86, 64, imgHex);
  } while (u8g2.nextPage());
}

void processRes(const char *resJson)
{
  //Serial.println("Start to parse");
  JsonObject &resObj = jsbf.parseObject(resJson);
  //Serial.println("Parse done");
  if (resObj.containsKey("displayHex"))
  {
    //Serial.println("PrepareToConvert");
    hex2byte(imgHex, (char *)resObj["displayHex"].as<char *>());
    //Serial.println("PrepareToFresh");
    freshDisplay();
    jsbf.clear();
  }
}

void setup()
{
  Serial.begin(115200);
  initWifi(ssid, password);
  u8g2.begin();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    http.begin(client, url); //HTTP
    http.addHeader("Content-Type", "application/json");
    char postData[64] = "";
    sprintf(postData, "%s%lu%s", "{\"millis\":", millis(), "}");
    if (startFlag)
    {
      sprintf(postData, "%s%lu%s", "{\"millis\":", millis(), ",\"start\":\"true\"}");
      startFlag = false;
    }
    int httpCode = http.POST(postData);

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK)
      {
        const String &payload = http.getString();
        //Serial.println("received payload:\n<<");
        //Serial.println(payload);
        //Serial.println(">>");
        processRes(payload.c_str());
        delay(10);
        return;
      }
    }
    else
    {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    delay(1000);
  }
}
