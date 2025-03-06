#include "main.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
uint16_t buttonState = 0;

uint8_t brightness = 100;

void setup(void)
{
  //* start DEBUGSERIAL debugging
#ifdef DEBUG
  DEBUGSERIAL.begin(115200);
#endif
  PRINT("\t\tSetup");

  //* start SPIFFS
  while (!SPIFFS.begin())
  {
    PRINTLN("SPIFFS SETUP PROBLEM");
    vTaskDelay(100);
  }
  DUMPFS();

  //* setup button pad

  buttonStateInit();

  //* init usb as hid
  Mouse.begin();
  Keyboard.begin();
  USB.begin();

  //* initialize macroses
  macrosInit();

  //* setup backlight pwm

  pinMode(PIN_BACKLIGHT, OUTPUT);
  ledcSetup(0, 100000, 8);
  ledcAttachPin(PIN_BACKLIGHT, 0);

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
      4096,
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
      32768,
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
  for (;;)
  {
    // if (currTab)
    // {
    setBrightness(currMode == MODE_OFF ? 0 : brightness); // turn on/off the backlight
    switch (currMode)
    {
    case MODE_OFF:
      setBrightness(0);
      break;
    case MODE_ON:
      //* Button state
      for (size_t buttonRow = 0; buttonRow < 4; buttonRow++)
      {
        for (size_t buttonColumn = 0; buttonColumn < 4; buttonColumn++)
        {
          tft.drawPixel(121 + buttonColumn * 2, 0 + buttonRow * 2, buttonState & 1 << (buttonColumn + buttonRow * 4) ? TFT_GREEN : TFT_RED);
        }
      }

      //* tab slider
      if (currTab != 0)
      {
        tft.fillRect(0, 154, ((currTab) << 5) - 1, 5, TFT_DARKGREY);
      }
      tft.fillRect((currTab) << 5, 154, 32, 5, TFT_RED);
      if (currTab != 3)
      {
        tft.fillRect((currTab + 1) << 5, 154, (3 - currTab) << 5, 5, TFT_DARKGREY);
      }

      break;
    case MODE_SETTINGS:
      setBrightness(0);
      break;

    default:
      break;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void handleButtons(void *args)
{
  for (;;)
  {
    buttonStateUpdate(&buttonState);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void mainSystem(void *args)
{
  for (;;)
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
      else if (buttonState & 1 << BUTTON_STAR)
      {
        currMode = MODE_OFF;
        PRINTLN("TRY TO TURN OFF");
      }

      switch (currTab) // macros activation
      {
      case TAB_A:
        for (size_t i = 0; i < MACROS_COUNT_ON_TAB; i++)
        {
          if (macrosOnTabs[TAB_A][i] != nullptr)
          {
            macrosOnTabs[TAB_A][i]->pokeMacro(buttonState & 1 << macroNumToButtonNum(i));
            macrosOnTabs[TAB_A][i]->runMacro();
          }
        }
        break;

      case TAB_B:
        for (size_t i = 0; i < MACROS_COUNT_ON_TAB; i++)
        {
          if (macrosOnTabs[TAB_B][i] != nullptr)
          {
            macrosOnTabs[TAB_B][i]->pokeMacro(buttonState & 1 << macroNumToButtonNum(i));
            macrosOnTabs[TAB_B][i]->runMacro();
          }
        }
        break;

      case TAB_C:
        for (size_t i = 0; i < MACROS_COUNT_ON_TAB; i++)
        {
          if (macrosOnTabs[TAB_C][i] != nullptr)
          {
            macrosOnTabs[TAB_C][i]->pokeMacro(buttonState & 1 << macroNumToButtonNum(i));
            macrosOnTabs[TAB_C][i]->runMacro();
          }
        }
        break;

      case TAB_D:
        for (size_t i = 0; i < MACROS_COUNT_ON_TAB; i++)
        {
          if (macrosOnTabs[TAB_D][i] != nullptr)
          {
            macrosOnTabs[TAB_D][i]->pokeMacro(buttonState & 1 << macroNumToButtonNum(i));
            macrosOnTabs[TAB_D][i]->runMacro();
          }
        }
        break;
      default:
        break;
      }
      break;
    case MODE_SETTINGS:
      /* code */
      break;

    default:
      if (buttonState and not(buttonState & 1 << BUTTON_STAR))
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
  for (;;)
  {
    PRINT("ledcRead(0)");
    PRINTLN(ledcRead(0));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void macrosInit()
{
  macrosOnTabs[TAB_A][0] = macros_autoClickerLMB;
  macrosOnTabs[TAB_A][3] = macros_toggle_autoClickerLMB;
  macrosOnTabs[TAB_A][1] = macros_autoClickerRMB;
  macrosOnTabs[TAB_A][4] = macros_toggle_autoClickerRMB;
  macrosOnTabs[TAB_A][2] = macros_plusW;
  macrosOnTabs[TAB_A][5] = macros_toggle_plusW;
}

int_fast8_t macroNumToButtonNum(int_fast8_t num)
{
  assert(0 <= num and num < MACROS_COUNT_ON_TAB);
  switch (num)
  {
  case 9:
    num = BUTTON_0;
    break;
  default:
    num = num * 4 / 3;
    break;
  }
  return num;
}

void lmbSpam()
{
  Mouse.click(MOUSE_LEFT);
}

void rmbSpam()
{
  Mouse.click(MOUSE_RIGHT);
}

void plusW()
{
  Keyboard.press('w');
}

void minusW()
{
  Keyboard.press('w');
}
