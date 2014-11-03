#include <LiquidCrystal.h>
#include <DHT22.h>

#include <Wire.h>
#include <Adafruit_BMP085.h>

#include <Button.h>

// LCD pins to Arduino:
// 1 <-> GND
// 2 <-> +5V
// 3 <-> 2Kohm <-> GND
// 4 <-> Dig 12
// 5 <-> GND
// 6 <-> Dig 11
// 11 <-> Dig 5
// 12 <-> Dig 4
// 13 <-> Dig 3
// 14 <-> Dig 2
// 15 <-> 2K7Ohm <-> +5V
// 16 <-> GND
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// DHT22 pins to Arduino:
// 1 <-> 2K7Ohm <-> 2, +5V
// 2 <-> Dig 7
// 3 <-> N/C
// 4 <-> GND
DHT22 myDHT22(7);

// Buttons to Arduino:
// GND <-> Button <-> Dig 8
// GND <-> Button <-> Dig 9
// GND <-> Button <-> Dig 10
Button btnUp = Button(8,BUTTON_PULLUP_INTERNAL);
Button btnDown = Button(9,BUTTON_PULLUP_INTERNAL);
Button btnSelect = Button(10,BUTTON_PULLUP_INTERNAL);

// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here
Adafruit_BMP085 bmp;

// === RUSSIAN FONT ===
byte rus_p_cap[8] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
};

byte rus_g[8] = {
  B00000,
  B00000,
  B11110,
  B10000,
  B10000,
  B10000,
  B10000,
};

byte rus_d[8] = {
  B00000,
  B00000,
  B00110,
  B01010,
  B01010,
  B11111,
  B10001,
};

byte rus_d_cap[8] = {
  B00110,
  B01010,
  B01010,
  B01010,
  B01010,
  B11111,
  B10001,
};

byte rus_m[8] = {
  B00000,
  B00000,
  B10001,
  B11011,
  B10101,
  B10001,
  B10001,
};

byte rus_l[8] = {
  B00000,
  B00000,
  B00111,
  B01001,
  B01001,
  B01001,
  B10001,
};

int RUS_P_CAP = 0;
#define RUS_G 1
#define RUS_D 2
#define RUS_D_CAP 3
#define RUS_M 4
#define RUS_L 5
// === RUSSIAN FONT ===

// Dialogs

class Dialog
{
  public:
    virtual void paint() = 0;
    virtual void handleInput() = 0;
};

class App
{
  public:
    void setCurrentDialog(Dialog* newDialog)
    {
      if (currentDialog != NULL)
      {
        delete currentDialog;
      }
      currentDialog = newDialog;
    }

    void handleInput()
    {
      currentDialog->handleInput();
    }
    
    void paint()
    {
      currentDialog->paint();
    }

  private:
    Dialog* currentDialog;
};
App app;

// Buttons
enum Buttons
{
  BTN_NONE,
  BTN_UP,
  BTN_DOWN,
  BTN_SELECT
};
int currentButtonPressed = BTN_NONE;

class SettingsMenu : public Dialog
{
  public:
    virtual void paint()
    {
      // check selected item index
      // draw 2 elements of menu with 1 selected
    }
    
    virtual void handleInput()
    {
    }
};

class MainScreen : public Dialog
{
  public:
    virtual void paint()
    {
      // check bmp temp
      // check bmp pressure
      // check bmp altitude
      // check bmp qnh
      // check display mode
      // update lcd.line1 if necessary
      // check dht status
      // check dht temperature
      // check dht humidity
      // update lcd.line2 if necessary
    }
    
    virtual void handleInput()
    {
      switch (currentButtonPressed)
      {
        case BTN_UP:
        case BTN_DOWN:
        case BTN_SELECT:
          app.setCurrentDialog(new SettingsMenu());
      }
    }
};

class PressureUnitsSelectionDialog : public Dialog
{
  public:
    virtual void paint()
    {
      // draw hPa and mmHg selection menu
    }
    
    virtual void handleInput()
    {
    }
};

class TemperatureUnitsSelectionDialog : public Dialog
{
  public:
    virtual void paint()
    {
      // draw C and F selection menu
    }
    
    virtual void handleInput()
    {
    }
};

class ModeSelectionDialog : public Dialog
{
  public:
    virtual void paint()
    {
      // draw QNH and Altitude selection menu
    }
    
    virtual void handleInput()
    {
    }
};

class NumericInputDialog : public Dialog
{
  public:
    virtual void paint()
    {
      // draw label, numeric field, cance and ok buttons
    }
    
    virtual void handleInput()
    {
    }
};

// Globals

unsigned long previousMillis = 0;        // will store last time
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long interval = 5000;           // interval at which to cycle (milliseconds)

DHT22_ERROR_t errorCode;

float qnh = 101325;
char newQnhBuf[5];

char line1[17];
char line2[17];
bool isLine1Changed = false;
bool isLine2Changed = false;

int temp_bmp085 = 0;
long pres_bmp085 = 0;
float alti_bmp085 = 0;

// Functions

void showSplash()
{
  lcd.setCursor(0, 0);
  lcd.write(RUS_P_CAP);
  lcd.print("o");
  lcd.write(RUS_G);
  lcd.print("o");
  lcd.write(RUS_D);
  lcd.print("a ");
  lcd.write(RUS_D_CAP);
  lcd.print("o");
  lcd.write(RUS_M);
  lcd.print("a");
}

void clearLine1()
{
  memset(line1, 0, 17);
}

void clearLine2()
{
  memset(line2, 0, 17);
}
  
void setup() {
  // start serial port
  Serial.begin(9600);
  Serial.println("Home Weather");
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  delay(200);
  lcd.createChar(RUS_P_CAP, rus_p_cap);
  delay(100);
  lcd.createChar(RUS_G, rus_g);
  delay(100);
  lcd.createChar(RUS_D, rus_d);
  delay(100);
  lcd.createChar(RUS_D_CAP, rus_d_cap);
  delay(100);
  lcd.createChar(RUS_M, rus_m);
  delay(100);
  lcd.createChar(RUS_L, rus_l);
  delay(100);
  
  showSplash();

  bmp.begin();  

  clearLine1();
  clearLine2();
}

void checkSerial()
{
  // check if new QNH is received
  int bytesReceived = Serial.available();
  if (bytesReceived >= 7) {
    Serial.print("Got ");
    Serial.print(bytesReceived);
    Serial.println(" bytes from serial");
    int incomingByte = Serial.read();
    char incomingChar = (char)incomingByte;
    Serial.print(incomingChar);
    if (incomingChar == 'Q' || incomingChar == 'q') {
      incomingByte = Serial.read();
      Serial.print((char)incomingByte);
      incomingByte = Serial.read();
      Serial.print((char)incomingByte);
      newQnhBuf[3] = '\0';
      newQnhBuf[4] = '\0';
      incomingByte = Serial.read();
      Serial.print((char)incomingByte);
      newQnhBuf[0] = (char)incomingByte;
      incomingByte = Serial.read();
      Serial.print((char)incomingByte);
      newQnhBuf[1] = (char)incomingByte;
      incomingByte = Serial.read();
      Serial.print((char)incomingByte);
      newQnhBuf[2] = (char)incomingByte;
      incomingByte = Serial.read();
      if (incomingByte != -1) {
        Serial.print((char)incomingByte);
        newQnhBuf[3] = (char)incomingByte;
      }
      else
      {
        Serial.print("-1");
      }
      Serial.println();
      Serial.print("Got serial data: ");
      Serial.println(newQnhBuf);
      int tempQnh = atoi(newQnhBuf);
      qnh = (float)tempQnh * 100;
      while(Serial.available()) {
         incomingByte = Serial.read();
      }
      Serial.print("Got new QNH: ");
      Serial.println(qnh);
    }
    else
    {
      while (incomingByte != -1)
      {
        incomingByte = Serial.read();
        if (incomingByte != -1)
        {
          Serial.print((char)incomingByte);
        }
      }
      Serial.println();
    }
  }
}

void checkSensors()
{
  // check to see if it's time to make things; that is, if the
  // difference between the current time and last time thing were done
  // is bigger than the interval at which you want to do things
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you have done the things
    previousMillis = currentMillis;
    if (interval != 60000)
    {
      interval = 60000;
    }

//=== BMP085 ===
    bool isTempBMP085Changed = false;
    bool isPresBMP085Changed = false;
    bool isAltiBMP085Changed = false;

    int new_temp_bmp085 = (int)bmp.readTemperature();
    if (new_temp_bmp085 != temp_bmp085)
    {
      isTempBMP085Changed = true;
      temp_bmp085 = new_temp_bmp085;
    }

    long new_pres_bmp085 = bmp.readPressure();
    if (new_pres_bmp085 != pres_bmp085)
    {
      isPresBMP085Changed = true;
      pres_bmp085 = new_pres_bmp085;
    }
    
    float new_alti_bmp085 = bmp.readAltitude(qnh);
    if (new_alti_bmp085 != alti_bmp085)
    {
      isAltiBMP085Changed = true;
      alti_bmp085 = new_alti_bmp085;
    }

    if (isTempBMP085Changed || isPresBMP085Changed || isAltiBMP085Changed)
    {
      isLine1Changed = true;
      clearLine1();
      sprintf(line1, "%2dC Q%4d %dm", temp_bmp085, (int)(pres_bmp085/100), (int)alti_bmp085);
    }
//=== END OF BMP085 ===

//=== DHT22 ===
    errorCode = myDHT22.readData();
//    lcd.setCursor(0, 1);
  
    switch(errorCode) {
      case DHT_ERROR_NONE:
        clearLine2();
        sprintf(line2, "%2d%cC B%c:%2d%%", (int)myDHT22.getTemperatureC(), B11011111, RUS_L, (int)myDHT22.getHumidity());
        isLine2Changed = true;
        break;
      case DHT_ERROR_CHECKSUM:
        break;
      case DHT_BUS_HUNG:
        clearLine2();
        sprintf(line2, "BUS hung");
        isLine2Changed = true;
        break;
      case DHT_ERROR_NOT_PRESENT:
        clearLine2();
        sprintf(line2, "No sensor");
        isLine2Changed = true;
        break;
      case DHT_ERROR_ACK_TOO_LONG:
        clearLine2();
        sprintf(line2, "Sensor timeout");
        isLine2Changed = true;
        break;
      case DHT_ERROR_SYNC_TIMEOUT:
        clearLine2();
        sprintf(line2, "Sensor not sync");
        isLine2Changed = true;
        break;
      case DHT_ERROR_DATA_TIMEOUT:
        clearLine2();
        sprintf(line2, "Sensor timeout");
        isLine2Changed = true;
        break;
      case DHT_ERROR_TOOQUICK:
        clearLine2();
        sprintf(line2, "Polled too quick");
        isLine2Changed = true;
        break;
    }
//=== END OF DHT22 ===
  }
}

void updateLCD()
{
  if (isLine1Changed)
  {
    isLine1Changed = false;
    lcd.setCursor(0, 0);
    lcd.print(line1);
  }
  if (isLine2Changed)
  {
    isLine2Changed = false;
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void checkButtons()
{
  if (btnUp.isPressed() || btnDown.isPressed() || btnSelect.isPressed())
  {
    digitalWrite(13, HIGH);
  }
  else
  {
    digitalWrite(13, LOW);
  }

  if (btnUp.isPressed() && btnUp.stateChanged())
  {
    currentButtonPressed = BTN_UP;
    return;
  }
  
  if (btnDown.isPressed() && btnDown.stateChanged())
  {
    currentButtonPressed = BTN_DOWN;
    return;
  }
  
  if (btnSelect.isPressed() && btnSelect.stateChanged())
  {
    currentButtonPressed = BTN_SELECT;
    return;
  }

  currentButtonPressed = BTN_NONE;
}

void handleInput()
{
  if (currentButtonPressed != BTN_NONE)
  {
    app.handleInput();
  }
}

void loop() {
  checkSerial();

  checkSensors();

  checkButtons();
  
  handleInput();
  
  app.paint();

  updateLCD();
}

