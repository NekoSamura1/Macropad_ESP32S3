#include "main.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
uint16_t buttonState = 0;

uint8_t brightness = 100;
void setup(void)
{
  //* start DEBUGSERIAL debugging

#ifdef DEBUG
  DEBUGSERIAL.begin(115200);
  PRINT("\t\tSetup");
#endif

  //* start SPIFFS

  while (!SPIFFS.begin())
  {
    PRINTLN("SPIFFS SETUP PROBLEM");
    vTaskDelay(100);
  }

#ifdef DEBUG
  DUMPFS();
#endif

  //* setup backlight pwm

  pinMode(PIN_BACKLIGHT, OUTPUT);
  ledcSetup(0, 100000, 8);
  ledcAttachPin(PIN_BACKLIGHT, 0);

  //* setup button pad

  buttonStateInit();

  //* setup display

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(0);
  tft.loadFont("DaysOne16");
  tft.setCursor(8, 5);
  tft.print("ButtonState");
  setBrightness(brightness);

  //* task create

  xTaskCreate(
      handleScreen,
      "handleScreen",
      2048,
      NULL,
      1,
      NULL);

  xTaskCreate(
      handleButtons,
      "handleButtons",
      1024,
      NULL,
      1,
      NULL);

  xTaskCreate(
      mainSystem,
      "mainSystem",
      2048,
      NULL,
      1,
      NULL);

#ifdef DEBUG
  xTaskCreate(
      debug,
      "debug",
      2048,
      NULL,
      1,
      NULL);
#endif
}

void loop()
{
  vTaskDelete(NULL);
}

void setBrightness(uint8_t brightness)
{
  ledcWrite(0, brightness);
}

void handleScreen(void *args)
{
  while (1)
  {
    // if (currTab)
    // {
    setBrightness(currMode == MODE_OFF ? 0 : brightness); // turn on/off the backlight

    { //* Button state
      for (size_t buttonRow = 0; buttonRow < 4; buttonRow++)
      {
        for (size_t buttonColumn = 0; buttonColumn < 4; buttonColumn++)
        {
          tft.drawPixel(121 + buttonColumn * 2, 0 + buttonRow * 2, buttonState & 1 << (buttonColumn + buttonRow * 4) ? TFT_GREEN : TFT_RED);
          tft.drawCircle(22 + 28 * buttonColumn, 50 + 28 * buttonRow, 11, buttonState & 1 << (buttonColumn + buttonRow * 4) ? TFT_GREEN : TFT_RED);
          tft.drawCircle(22 + 28 * buttonColumn, 50 + 28 * buttonRow, 10, buttonState & 1 << (buttonColumn + buttonRow * 4) ? TFT_GREEN : TFT_RED);
        }
      }
    }

    { //* tab slider
      if (currTab != 0)
      {
        tft.fillRect(0, 154, ((currTab) << 5) - 1, 5, TFT_DARKGREY);
      }

      tft.fillRect((currTab) << 5, 154, 32, 5, TFT_RED);

      if (currTab != 3)
      {
        tft.fillRect((currTab + 1) << 5, 154, (3 - currTab) << 5, 5, TFT_DARKGREY);
      }
    }
    // }
    // else
    // {
    //   if (ledcRead(0))
    //   {
    //     PRINTLN("power off");
    //     setBrightness(0);
    //   }
    // }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void handleButtons(void *args)
{
  while (1)
  {
    buttonStateUpdate(&buttonState);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void mainSystem(void *args)
{
  while (1)
  {

    switch (currMode)
    {
    case MODE_ON:
      if (buttonState == 1 << BUTTON_A)
      {
        currTab = TAB_A;
      }
      else if (buttonState == 1 << BUTTON_B)
      {
        currTab = TAB_B;
      }
      else if (buttonState == 1 << BUTTON_C)
      {
        currTab = TAB_C;
      }
      else if (buttonState == 1 << BUTTON_D)
      {
        currTab = TAB_D;
      }
      else if (buttonState & 1 << BUTTON_POWER)
      {
        currMode = MODE_OFF;
        PRINTLN("TRY TO TURN OFF");
      }
      break;
    case MODE_SETTINGS:
      /* code */
      break;

    default:
      if (buttonState and not(buttonState & 1 << BUTTON_POWER))
      {
        PRINTLN("TRY TO TURN ON");
        currMode = MODE_ON;
      }
      break;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void debug(void *args)
{
  while (1)
  {
    PRINT("ledcRead(0)");
    PRINTLN(ledcRead(0));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}