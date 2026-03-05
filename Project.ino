#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const int LED_PIN = 5;
const int BUZZER_PIN = 8;
const int BUTTON_PIN = 7;

// WIFI
char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

bool alarmActive = false;
char currentSection;

unsigned long lastBeep = 0;

// medication schedule
String schedule[8] = {
  "08:00",
  "10:00",
  "12:00",
  "14:00",
  "16:00",
  "18:00",
  "20:00",
  "22:00"
};

char sections[8] = {'A','B','C','D','E','F','G','H'};

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN,OUTPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.print("Connecting WiFi");

  WiFi.begin(ssid,pass);

  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
  }

  lcd.clear();
  lcd.print("WiFi Connected");

  timeClient.begin();

  delay(2000);
  lcd.clear();
}

void loop()
{
  timeClient.update();

  String timeNow = timeClient.getFormattedTime();
  String currentTime = timeNow.substring(0,5);

  lcd.setCursor(0,0);
  lcd.print(currentTime);

  checkSchedule(currentTime);

  if(alarmActive)
  handleAlarm();

  checkButton();
}

void checkSchedule(String currentTime)
{
  for(int i=0;i<8;i++)
  {
    if(currentTime == schedule[i] && !alarmActive)
    {
      startAlarm(sections[i]);
      break;
    }
  }
}

void startAlarm(char section)
{
  alarmActive = true;
  currentSection = section;

  digitalWrite(LED_PIN,HIGH);

  lcd.clear();
  lcd.print("Take Section:");
  lcd.setCursor(0,1);
  lcd.print(section);

  Serial.print("Phone notification: ");
  Serial.println(section);
}

void handleAlarm()
{
  unsigned long now = millis();

  if(now - lastBeep > 60000 || lastBeep == 0)
  {
    lastBeep = now;

    tone(BUZZER_PIN,1000);
    delay(3000);
    noTone(BUZZER_PIN);
  }
}
void checkButton()
{
  if(digitalRead(BUTTON_PIN)==LOW && alarmActive)
  {
    stopAlarm();
  }
}

void stopAlarm()
{
  alarmActive = false;

  digitalWrite(LED_PIN,LOW);
  noTone(BUZZER_PIN);

  lcd.clear();
  lcd.print("Medication");
  lcd.setCursor(0,1);
  lcd.print("Taken");

  delay(2000);
  lcd.clear();
}
