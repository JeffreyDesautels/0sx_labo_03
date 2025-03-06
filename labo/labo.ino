// code = 2255309
#include <LCD_I2C.h>

#define PHOTO_PIN A0
#define LED_PIN 8
#define BTN_PIN 2

LCD_I2C lcd(0x27, 16, 2);

uint8_t number[8] = {
  0b11100,
  0b10100,
  0b10100,
  0b11111,
  0b00101,
  0b00111,
  0b00001,
  0b00111,
};

unsigned long current_time = 0;

bool start_done = false;

int luminosity = 0;
int last_luminosity = -1;


void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();

  lcd.createChar(0, number);

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  current_time = millis();
  luminosity = analogRead(PHOTO_PIN);
  int lum_percent = map(luminosity, 0, 1024, 0, 100);

  start_task(current_time);
  led_task(current_time, lum_percent);
}

// en lancant les 2 fonctions ensemble, le texte de la premiere fonction ne reste pas affiche les 3 secondes necessaires
void start_task(unsigned long ct) {
  static unsigned long previous_time = 0;
  const long interval = 3000;
  static bool lcd_cleared = false;

  if (!start_done) {
    lcd.setCursor(0, 0);
    lcd.print("Desautels");
    lcd.setCursor(0, 1);
    lcd.write(0);
    lcd.print("09");

    start_done = true;
    previous_time = ct;
  }

  if (!lcd_cleared && start_done && (ct - previous_time >= interval)) {
    lcd.clear();
    lcd_cleared = true;
  }
}

void led_task(unsigned long ct, int lp) {
  static unsigned long previous_time = 0;
  const long interval = 5000;

  if (lp != last_luminosity) {
    lcd.setCursor(0, 0);
    lcd.print("Pct lum: ");
    lcd.print(lp);
    lcd.print("%     ");
    lcd.setCursor(0, 1);
    lcd.print("Phare: ");

    last_luminosity = lp;
  }

  // etat led alterne chaque 5sec selon horloge systeme (changements < 5s = possible)
  // ajouter timer avec boolean pour changement etat a chaque 5 sec reelles (changements < 5s = impossible)
  if (ct - previous_time >= interval) {
    lcd.setCursor(8, 1);
    if (lp < 50) {
      digitalWrite(LED_PIN, HIGH);
      lcd.print("ON       ");
    } else {
      digitalWrite(LED_PIN, LOW);
      lcd.print("OFF      ");
    }
    previous_time = ct;
  }
}

void joystick_task() {
}

void button_task() {
}
