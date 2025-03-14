#include <LCD_I2C.h>

#define PHOTO_PIN A0
#define X_PIN A1
#define Y_PIN A2
#define BTN_PIN 2
#define LED_PIN 8

LCD_I2C lcd(0x27, 16, 2);

uint8_t da_number[8] = {
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

bool timer_started = false;
bool previous_state = LOW;
int luminosity = 0;
int last_luminosity = -1;
bool led_state = LOW;

int x_val = 0;
int y_val = 0;

bool display_mode = true;

void setup() {
  Serial.begin(115200);

  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, da_number);

  pinMode(LED_PIN, OUTPUT);
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  start_task();
}

void loop() {
  current_time = millis();

  luminosity = analogRead(PHOTO_PIN);
  int lum_percent = map(luminosity, 0, 1023, 0, 100);

  x_val = analogRead(X_PIN);
  y_val = analogRead(Y_PIN);

  is_clicked(current_time);

  if (display_mode) {
    led_task(lum_percent);
  } else {
    joystick_task(x_val, y_val);
  }

  console_output(current_time);
}

void start_task() {
  static unsigned long previous_time = 0;
  const long interval = 3000;

  lcd.setCursor(0, 0);
  lcd.print("Desautels");
  lcd.setCursor(0, 1);
  lcd.write(0);
  lcd.setCursor(14, 1);
  lcd.print("09");
  delay(3000);
  lcd.clear();
}

void led_task(int lp) {
  static unsigned long start_timer = 0;
  static int lp_change_point = 50;
  static bool first_time = true;
  bool current_state = (lp < lp_change_point);
  const long interval = 5000;

  if (lp != last_luminosity) {
    lcd.setCursor(0, 0);
    lcd.print("Pct lum: ");
    lcd.setCursor(9, 0);
    lcd.print(lp);
    lcd.print("%     ");
    lcd.setCursor(0, 1);
    lcd.print("Phare: ");

    last_luminosity = lp;
  }

  if (first_time) {
    if (current_state) {
      digitalWrite(LED_PIN, HIGH);
      led_state = HIGH;
      lcd.print("ON       ");
    } else {
      digitalWrite(LED_PIN, LOW);
      led_state = LOW;
      lcd.print("OFF      ");
    }
    first_time = false;
  }

  if (current_state != previous_state) {
    if (!timer_started) {
      start_timer = millis();
      timer_started = true;
    } else {
      timer_started = false;
    }
  }

  if (timer_started) {
    if (current_state && millis() - start_timer >= interval) {
      digitalWrite(LED_PIN, HIGH);
      led_state = HIGH;
      lcd.print("ON       ");
      timer_started = false;
    } else if (!current_state && millis() - start_timer >= interval) {
      digitalWrite(LED_PIN, LOW);
      led_state = LOW;
      lcd.print("OFF      ");
      timer_started = false;
    }
  }

  previous_state = current_state;
}

void joystick_task(int x_val, int y_val) {
  int dead_point_x = 506;
  int angle;
  int dead_point_y = 498;
  int speed;

  lcd.setCursor(0, 0);
  lcd.print("Vitesse: ");
  lcd.setCursor(9, 0);
  if (y_val >= (dead_point_y - 2) && y_val <= (dead_point_y + 2)) {
    speed = 0;
  } else if (y_val > dead_point_y) {
    speed = map(y_val, dead_point_y, 1023, 0, 120);
  } else {
    speed = map(y_val, 0, dead_point_y, -25, 0);
  }
  lcd.print(speed);
  lcd.print("KM/H  ");

  lcd.setCursor(0, 1);
  lcd.print("Direction: ");
  lcd.setCursor(11, 1);
  if (x_val >= (dead_point_x - 2) && x_val <= (dead_point_x + 2)) {
    angle = 0;
    lcd.print(angle);
    lcd.print(" C    ");
  } else if (x_val > dead_point_x) {
    angle = map(x_val, dead_point_x, 1023, 0, 90);
    lcd.print(angle);
    lcd.print(" D    ");
  } else {
    angle = map(x_val, 0, dead_point_x, -90, 0);
    lcd.print(angle);
    lcd.print(" G    ");
  }
}

int is_clicked(unsigned long ct) {
  static unsigned long last_time = 0;
  static int last_state = HIGH;
  const int rate = 50;
  int clic = 0;

  if (ct - last_time < rate) {
    return clic;
  }

  last_time = ct;

  int state = digitalRead(BTN_PIN);

  if (state == LOW) {
    if (last_state == HIGH) {
      clic = 1;
      display_mode = !display_mode;
      lcd.clear();
    }
  }

  last_state = state;

  return clic;
}

void console_output(unsigned long ct) {
  static unsigned long previous_time = 0;
  const long interval = 100;

  if (ct - previous_time >= interval) {
    Serial.print("etd : 2255309 | ");
    Serial.print("x : ");
    Serial.print(x_val);
    Serial.print(" | y : ");
    Serial.print(y_val);
    Serial.print(" sys : ");
    Serial.println(led_state);

    previous_time = ct;
  }
}