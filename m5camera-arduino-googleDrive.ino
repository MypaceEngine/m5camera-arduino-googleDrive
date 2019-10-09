/*
  M5CameraでGoogle DriveにJPEGファイルをアップロードするテスト
*/

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
//#define M5CAM_MODEL_A
#define M5CAM_MODEL_B

#if defined(M5CAM_MODEL_A)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21
#elif defined(M5CAM_MODEL_B)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     22
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21
#else
  #error "Not supported"
#endif

WiFiClientSecure client;
#define WIFI_CONN_TIMEOUT 30000 //Wi-Fi接続のタイムアウト
String ssid = "SSID";
String pass = "PASSWORD";
boolean flag=false;
/*  事前にgoogle.comのCAを取得しておく */
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIESjCCAzKgAwIBAgINAeO0mqGNiqmBJWlQuDANBgkqhkiG9w0BAQsFADBMMSAw\n" \
"HgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFs\n" \
"U2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjAeFw0xNzA2MTUwMDAwNDJaFw0yMTEy\n" \
"MTUwMDAwNDJaMEIxCzAJBgNVBAYTAlVTMR4wHAYDVQQKExVHb29nbGUgVHJ1c3Qg\n" \
"U2VydmljZXMxEzARBgNVBAMTCkdUUyBDQSAxTzEwggEiMA0GCSqGSIb3DQEBAQUA\n" \
"A4IBDwAwggEKAoIBAQDQGM9F1IvN05zkQO9+tN1pIRvJzzyOTHW5DzEZhD2ePCnv\n" \
"UA0Qk28FgICfKqC9EksC4T2fWBYk/jCfC3R3VZMdS/dN4ZKCEPZRrAzDsiKUDzRr\n" \
"mBBJ5wudgzndIMYcLe/RGGFl5yODIKgjEv/SJH/UL+dEaltN11BmsK+eQmMF++Ac\n" \
"xGNhr59qM/9il71I2dN8FGfcddwuaej4bXhp0LcQBbjxMcI7JP0aM3T4I+DsaxmK\n" \
"FsbjzaTNC9uzpFlgOIg7rR25xoynUxv8vNmkq7zdPGHXkxWY7oG9j+JkRyBABk7X\n" \
"rJfoucBZEqFJJSPk7XA0LKW0Y3z5oz2D0c1tJKwHAgMBAAGjggEzMIIBLzAOBgNV\n" \
"HQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1Ud\n" \
"EwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYEFJjR+G4Q68+b7GCfGJAboOt9Cf0rMB8G\n" \
"A1UdIwQYMBaAFJviB1dnHB7AagbeWbSaLd/cGYYuMDUGCCsGAQUFBwEBBCkwJzAl\n" \
"BggrBgEFBQcwAYYZaHR0cDovL29jc3AucGtpLmdvb2cvZ3NyMjAyBgNVHR8EKzAp\n" \
"MCegJaAjhiFodHRwOi8vY3JsLnBraS5nb29nL2dzcjIvZ3NyMi5jcmwwPwYDVR0g\n" \
"BDgwNjA0BgZngQwBAgIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly9wa2kuZ29vZy9y\n" \
"ZXBvc2l0b3J5LzANBgkqhkiG9w0BAQsFAAOCAQEAGoA+Nnn78y6pRjd9XlQWNa7H\n" \
"TgiZ/r3RNGkmUmYHPQq6Scti9PEajvwRT2iWTHQr02fesqOqBY2ETUwgZQ+lltoN\n" \
"FvhsO9tvBCOIazpswWC9aJ9xju4tWDQH8NVU6YZZ/XteDSGU9YzJqPjY8q3MDxrz\n" \
"mqepBCf5o8mw/wJ4a2G6xzUr6Fb6T8McDO22PLRL6u3M4Tzs3A2M1j6bykJYi8wW\n" \
"IRdAvKLWZu/axBVbzYmqmwkm5zLSDW5nIAJbELCQCZwMH56t2Dvqofxs6BBcCFIZ\n" \
"USpxu6x6td0V7SvJCCosirSmIatj/9dSSVDQibet8q/7UK4v4ZUN80atnZz1yg==\n" \
"-----END CERTIFICATE-----\n";

/*  HTTPSでPOSTする  */
String httpsPost(String url, String data, int port)
{
  String host = url.substring(8, url.indexOf("/",8));
  const char* charHost = host.c_str();
  String response = "";
  char rcvBuf[2048];
  int rcvCount = 0;
  client.setCACert(root_ca);
  while(!client.connect(charHost, port))
  {
    delay(10);
  }
  if (client.connected())
  {
    client.print(data);
    long timeout = millis();
    while (rcvCount == 0)
    {
      while (client.available())
      {
        rcvBuf[rcvCount]=client.read();
        rcvCount ++;
      }
    }
  }
  response = rcvBuf;
  client.stop();
  return (response);
}


/*  画像をPOSTする */
String postPic(String url, String token, uint8_t* addr, int picLen, int port)
{ 
  String host = url.substring(8, url.indexOf("/",8));
  const char* charHost = host.c_str();
  String response = "";
  char rcvBuf[2048];
  int rcvCount = 0;
  Serial.print("Connecting ");
  Serial.println(charHost);
  
  //時計からファイル名生成
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char filename[128];
  sprintf(filename,"M5Camera-%02d%02d%02d %02d%02d%02d.jpg", timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday,timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
 

  String boundary = "------WebKitFormBoundary6n1BXCrYS8DFaBeb\r\n";
  String body="\r\n";
  body += boundary;
  body += "Content-Disposition: form-data; name=\"metadata\"; filename=\"blob\"\r\n";
  body += "Content-Type: application/json\r\n";
  body += "\r\n";
  //idにてGoogleDriveのフォルダIDを指定することにより、特定のフォルダにアップできます。
  body += "{ \"title\": \""+((String)filename)+"\", \"mimeType\": \"image/jpeg\", \"description\": \"Uploaded From ESP32\", \"parents\": [ { \"kind\": \"drive#parentReference\", \"id\": \"   folderid  \" } ] }\r\n";
  body += boundary;
  body += "Content-Disposition: form-data; name=\"file\"; filename=\"upload.jpg\"\r\n";
  body += "Content-Type: image/jpeg\r\n";
  body += "\r\n";

  String header =
  "POST /upload/drive/v2/files?uploadType=multipart HTTP/1.1\r\n"
  "HOST: www.googleapis.com\r\n"
  "authorization: Bearer ";
  header += token;
  header += "\r\n";
  header += "content-type: multipart/form-data; boundary=----WebKitFormBoundary6n1BXCrYS8DFaBeb\r\n";
  header += "content-length: ";
  int len = body.length();
  len += picLen;
  len += boundary.length();
  len += ((String)"\r\n").length();
  header += (String)len;
  header += "\r\n";
  while(!client.connect(charHost, port))
  {
    delay(10);
    Serial.print(".");
  }
  if (client.connected())
  {
    client.print(header);
    client.print(body);
    int sizeDivT = ((int)(picLen / 1000));
    int remSize = ((int)picLen - (int)(picLen / 1000)*1000);

    /*  1000bytesに区切ってPOST  */
    for(int i = 0; i < sizeDivT ;i++)
    {
        client.write((addr + i * 1000), 1000);
    }
    if(remSize > 0)
    {
        client.write((addr + (sizeDivT * 1000)), remSize);
    }

    client.print("\r\n------WebKitFormBoundary6n1BXCrYS8DFaBeb--\r\n");
    long timeout = millis();
    while (rcvCount == 0)
    {
      while (client.available())
      {
        rcvBuf[rcvCount]=client.read();
        Serial.write(rcvBuf[rcvCount]);
        rcvCount ++;
        if(rcvCount>2047)break;
      }
    }
  }
  else Serial.println("Connection failed");
  response = rcvBuf;
  client.stop();
  return (response);
}


/*  コードからリフレッシュトークンを取得  */
String getRefreshToken(String code, String client_id, String client_secret)
{
  String body =  
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"code\"\r\n"
  "\r\n"
  + code +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"client_id\"\r\n"
  "\r\n"
  + client_id +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"client_secret\"\r\n"
  "\r\n"
  + client_secret +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"redirect_uri\"\r\n"
  "\r\n"
  "urn:ietf:wg:oauth:2.0:oob\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"grant_type\"\r\n"
  "\r\n"
  "authorization_code\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr--\r\n";

  String header=
  "POST /o/oauth2/token HTTP/1.1\r\n"
  "HOST: accounts.google.com\r\n"
  "content-type: multipart/form-data; boundary=----WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "content-length: "
  + (String)body.length() +
  "\r\n"
  "\r\n";

  Serial.println(header);
  Serial.println(body);
  String ret = httpsPost("https://accounts.google.com/o/oauth2/token", header + body, 443);

  Serial.println("RAW response:");
  Serial.println(ret);
  Serial.println();
  
  if(ret.indexOf("200 OK") > -1) Serial.println("Got the refresh token!");
  else Serial.println("NG");

  int refresh_tokenStartPos = ret.indexOf("refresh_token");
  refresh_tokenStartPos = ret.indexOf("\"",refresh_tokenStartPos) + 1;
  refresh_tokenStartPos = ret.indexOf("\"",refresh_tokenStartPos) + 1;
  int refresh_tokenEndPos   = ret.indexOf("\"",refresh_tokenStartPos);
  String refresh_token = ret.substring(refresh_tokenStartPos,refresh_tokenEndPos); 
  Serial.println("refresh_token:"+refresh_token);
  return refresh_token;
}


/*  リフレッシュトークンからアクセストークンを取得 */
String getAccessToken(String refresh_token, String client_id, String client_secret)
{
  String body =
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"refresh_token\"\r\n"
  "\r\n"
  + refresh_token +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"client_id\"\r\n"
  "\r\n"
  + client_id +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"client_secret\"\r\n"
  "\r\n"
  + client_secret +
  "\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "Content-Disposition: form-data; name=\"grant_type\"\r\n"
  "\r\n"
  "refresh_token\r\n"
  "------WebKitFormBoundarytjGcuAGS9sMRWKcr--\r\n";
  
  String header=
  "POST /o/oauth2/token HTTP/1.1\r\n"
  "HOST: accounts.google.com\r\n"
  "content-type: multipart/form-data; boundary=----WebKitFormBoundarytjGcuAGS9sMRWKcr\r\n"
  "content-length: "
  + (String)body.length() +
  "\r\n"
  "\r\n";

  Serial.println("Header: "+header);
  Serial.println("Body: "+body);
  String ret = httpsPost("https://accounts.google.com/o/oauth2/token", header + body, 443); 
  
  Serial.println("RAW response:");
  Serial.println(ret);
  Serial.println();
  
  if(ret.indexOf("200 OK") > -1) Serial.println("Got the token!");
  else Serial.println("NG");

  int access_tokenStartPos = ret.indexOf("access_token");
  access_tokenStartPos = ret.indexOf("\"",access_tokenStartPos) + 1;
  access_tokenStartPos = ret.indexOf("\"",access_tokenStartPos) + 1;
  int access_tokenEndPos   = ret.indexOf("\"",access_tokenStartPos);
  String access_token = ret.substring(access_tokenStartPos,access_tokenEndPos); 
  Serial.println("Access_token:"+access_token);
  return access_token;
}


/*  画像取得  */
camera_fb_t * getJPEG()
{
  camera_fb_t * fb = NULL;
  /*  数回撮影してAE(自動露出)をあわせる */
  esp_camera_fb_get();
  esp_camera_fb_get();
  esp_camera_fb_get();
  esp_camera_fb_get();
  fb = esp_camera_fb_get();  //JPEG取得
  /*
  fb->buf JPEGバッファ
  fb->len JPEGサイズ
  */
  
  if (!fb) 
  {
    Serial.printf("Camera capture failed");
  }
  esp_camera_fb_return(fb); //後始末
  Serial.printf("JPG: %uB ", (uint32_t)(fb->len));
  return fb;  
}


/*  APに接続 */
bool initSTA() 
{
  WiFi.begin(ssid.c_str(), pass.c_str());
  int n;
  long timeout = millis();
  while ((n = WiFi.status()) != WL_CONNECTED && (millis()-timeout) < WIFI_CONN_TIMEOUT)
  {
    Serial.print(".");
    delay(500);
    if (n == WL_NO_SSID_AVAIL || n == WL_CONNECT_FAILED)
    {
      delay(1000);
      WiFi.reconnect();
    }
  }
  if (n == WL_CONNECTED)
  {
    return true;
  }
  else
  {
    return false;
  }
}



void setup() 
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  initSTA();  //APに接続
  /*
    SPIフラッシュがフォーマットされてなかったらフォーマット
    設定データがなければリフレッシュトークンを取得して保存する。
    設定データは行ごとに分けて
    ・client_id
    ・client_secret
    ・refresh_token
    と保存
  */
  if(!SPIFFS.begin())
  {
    Serial.println("SPIFFS formatting");
    SPIFFS.format();
  }
  String code = "";
  String client_id = "";
  String client_secret = "";
  File fd = SPIFFS.open("/access_token.txt", "r");
  fd.readStringUntil('\n');
  fd.readStringUntil('\n');
  String line = fd.readStringUntil('\n');
  fd.close();
  if(line == "")
  {
    /*  APIのクライアントIDを入力  */
    Serial.println("Client ID?");
    while(!Serial.available()){}
    client_id = Serial.readStringUntil('\n');
    client_id.trim();
    fd = SPIFFS.open("/access_token.txt", "w");
    if (!fd) 
    {
      Serial.println("Config open error");
    }
    fd.println(client_id);

    /*  コードを取得してくれと表示 */
    Serial.println("Please access to");
    Serial.println("https://accounts.google.com/o/oauth2/auth?client_id=" + client_id +"&response_type=code&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope=https://www.googleapis.com/auth/drive");

    /*  コードを入力  */
    Serial.println("Code?");
    while(!Serial.available()){}
    code = Serial.readStringUntil('\n');
    code.trim();

    /*  APIのクライアントシークレットを入力  */
    Serial.println("Client secret?");
    while(!Serial.available()){}
    client_secret = Serial.readStringUntil('\n');
    client_secret.trim();
    fd.println(client_secret);
    Serial.println("OK");
    delay(1000);
    Serial.println("Refresh token:");
    fd.println(getRefreshToken(code, client_id, client_secret));
    fd.close();
  }
  



  /*  カメラ初期化設定*/
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  else 
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  //カメラ初期化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
  }
  else
  {
    flag=true;
  }
  /*
  あとでカメラ設定変更するときは
  sensor_t * s = esp_camera_sensor_get(); //カメラプロパティのポインタ取得
  s->set_framesize(s, FRAMESIZE_UXGA);  //解像度設定
  とかやればおｋ
  */
  //現在時刻をNTPサーバから取得
  configTime(9*3600L,0,"ntp.nict.jp","time.google.com","ntp.jst.mfeed.ad.jp");
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.printf("%02d-%02d-%02dT%02d:%02d:%02d+09:00\n", timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday,timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

}

/*  15秒に一回撮影 */
void loop() 
{
  if(flag){
    /*  設定読み込み  */
	String code = "";
	String client_id = "";
	String client_secret = "";
	File fd = SPIFFS.open("/access_token.txt", "r");
	client_id = fd.readStringUntil('\r');
	Serial.print("Client ID");
	client_id.trim();
	Serial.print(client_id);
	Serial.println();
	client_secret = fd.readStringUntil('\r');
	client_secret.trim();
	Serial.print("Client Secret:");
	Serial.print(client_secret);
	Serial.println();
	String refresh_token = fd.readStringUntil('\r');
	refresh_token.trim();  
	Serial.print("Refresh token:");
	Serial.print(refresh_token);
	Serial.println();
	fd.close();
  
	Serial.println("Upload Start");
	client.setCACert(root_ca);  
	String access_token = getAccessToken(refresh_token, client_id, client_secret);
	Serial.println("Access token: "+access_token);
	camera_fb_t * fb = getJPEG();
	String ret = postPic("https://www.googleapis.com/upload/drive/v2/files?uploadType=multipart", access_token, fb->buf, fb->len, 443); 
	Serial.println("Return: "+ret);
  }else{
	Serial.println("Failure");
  }
  delay(15000);
}
