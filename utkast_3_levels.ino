#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define NUM_PAIRS 4

int lives = 6;
int level = 1;
int successfulPresses = 0;
unsigned long gameStartTime = 0;
bool gameWon = false;

enum {BUF_SIZE = 10};

// Buffer
static int buf[BUF_SIZE];
static int head = 0;
static int tail = 0;
static SemaphoreHandle_t mutex;
static SemaphoreHandle_t sem_empty;
static SemaphoreHandle_t sem_filled;

// Debounce system
volatile unsigned long lastPressTime[NUM_PAIRS] = {0};
const unsigned long debounceDelay = 150; // ms

struct interactor {
  const int id;
  const int btnPin;
  const int ledPin;
};

interactor interactors[NUM_PAIRS] = {
  {0, 12, 13},
  {1, 14, 27},
  {2, 26, 25},
  {3, 33, 32}
};

// Interrupt handler with debounce
void handleButtonPress(int id) {
  BaseType_t task_woken = pdFALSE;
  unsigned long now = millis();

  if (now - lastPressTime[id] > debounceDelay) {
    if (xSemaphoreTakeFromISR(sem_empty, &task_woken)) {
      xSemaphoreTakeFromISR(mutex, &task_woken);
      buf[head] = id;
      head = (head + 1) % BUF_SIZE;
      xSemaphoreGiveFromISR(mutex, &task_woken);
      xSemaphoreGiveFromISR(sem_filled, &task_woken);
    }
    lastPressTime[id] = now;
  }
  if (task_woken)
    portYIELD_FROM_ISR();
}

void IRAM_ATTR ISR_BTN_0() { handleButtonPress(0); }
void IRAM_ATTR ISR_BTN_1() { handleButtonPress(1); }
void IRAM_ATTR ISR_BTN_2() { handleButtonPress(2); }
void IRAM_ATTR ISR_BTN_3() { handleButtonPress(3); }

void clearInputBuffer() {
  while (uxSemaphoreGetCount(sem_filled) > 0) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    tail = (tail + 1) % BUF_SIZE;
    xSemaphoreGive(mutex);
    xSemaphoreGive(sem_empty);
  }
}

void resetGame() {
    lives = 6;
    level = 1;
    successfulPresses = 0;
    gameStartTime = millis();
    gameWon = false;
    clearInputBuffer();
    Serial.println("\n▶ Spillet restartes!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void gameTask(void *parameters) {

  gameStartTime = millis();

  while (1) {

    if (gameWon) {
      // Vent på restart med S1
      if (xSemaphoreTake(sem_filled, 50 / portTICK_PERIOD_MS) == pdTRUE) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        int id = buf[tail];
        tail = (tail + 1) % BUF_SIZE;
        xSemaphoreGive(mutex);
        xSemaphoreGive(sem_empty);

        if (id == 0) { // S1 trykket
          resetGame();
        }
      }
      continue;
    }

    if (lives > 0) {

      clearInputBuffer();
      Serial.println("\n👉 Ny runde!");
      vTaskDelay(600 / portTICK_PERIOD_MS);

      Serial.print("Level: ");
      Serial.println(level);

      // Tenn 1 tilfeldig LED
      int target = esp_random() % NUM_PAIRS;
      digitalWrite(interactors[target].ledPin, HIGH);

      int maxReaction = 3000 - (level - 1) * 1000;
      if (maxReaction < 1000) maxReaction = 1000;

      bool correct = false;
      bool wrongPress = false;
      unsigned long startTime = millis();

      while (millis() - startTime < maxReaction) {
        if (xSemaphoreTake(sem_filled, 50 / portTICK_PERIOD_MS) == pdTRUE) {
          xSemaphoreTake(mutex, portMAX_DELAY);
          int id = buf[tail];
          tail = (tail + 1) % BUF_SIZE;
          xSemaphoreGive(mutex);
          xSemaphoreGive(sem_empty);

          if (id == target) correct = true;
          else wrongPress = true;
          break;
        }
      }

      digitalWrite(interactors[target].ledPin, LOW);

      if (correct) {
        successfulPresses++;
        Serial.println("✨ Riktig!");

        if (level == 3) {
          Serial.println("\n🎉 GRATULERER — DU VANT! 🎉");
          Serial.println("Trykk S1 for å starte på nytt!");
          gameWon = true;
          continue;
        }

        level++;
        continue; // START NESTE RUNDE DIREKTE
      }

      // Feil
      lives--;
      Serial.println("❌ FEIL eller for treg!");
      Serial.print("Lives: ");
      Serial.println(lives);
      vTaskDelay(600 / portTICK_PERIOD_MS);

      if (lives <= 0) {
          unsigned long survived = (millis() - gameStartTime) / 1000;
          Serial.println("\n--- GAME OVER ---");
          Serial.print("Tid overlevd: ");
          Serial.print(survived);
          Serial.println(" sek");
          Serial.print("Antall riktige: ");
          Serial.println(successfulPresses);
          Serial.println("Trykk S1 for restart!");
          gameWon = true;
          continue;
      }
    }
  }
}

#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define NUM_PAIRS 4

int lives = 6;
int level = 1;
int successfulPresses = 0;
unsigned long gameStartTime = 0;
bool gameWon = false;

enum {BUF_SIZE = 10};

// Buffer
static int buf[BUF_SIZE];
static int head = 0;
static int tail = 0;
static SemaphoreHandle_t mutex;
static SemaphoreHandle_t sem_empty;
static SemaphoreHandle_t sem_filled;

// Debounce system
volatile unsigned long lastPressTime[NUM_PAIRS] = {0};
const unsigned long debounceDelay = 150; // ms

struct interactor {
  const int id;
  const int btnPin;
  const int ledPin;
};

interactor interactors[NUM_PAIRS] = {
  {0, 12, 13},
  {1, 14, 27},
  {2, 26, 25},
  {3, 33, 32}
};

// Interrupt handler with debounce
void handleButtonPress(int id) {
  BaseType_t task_woken = pdFALSE;
  unsigned long now = millis();

  if (now - lastPressTime[id] > debounceDelay) {
    if (xSemaphoreTakeFromISR(sem_empty, &task_woken)) {
      xSemaphoreTakeFromISR(mutex, &task_woken);
      buf[head] = id;
      head = (head + 1) % BUF_SIZE;
      xSemaphoreGiveFromISR(mutex, &task_woken);
      xSemaphoreGiveFromISR(sem_filled, &task_woken);
    }
    lastPressTime[id] = now;
  }
  if (task_woken)
    portYIELD_FROM_ISR();
}

void IRAM_ATTR ISR_BTN_0() { handleButtonPress(0); }
void IRAM_ATTR ISR_BTN_1() { handleButtonPress(1); }
void IRAM_ATTR ISR_BTN_2() { handleButtonPress(2); }
void IRAM_ATTR ISR_BTN_3() { handleButtonPress(3); }

void clearInputBuffer() {
  while (uxSemaphoreGetCount(sem_filled) > 0) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    tail = (tail + 1) % BUF_SIZE;
    xSemaphoreGive(mutex);
    xSemaphoreGive(sem_empty);
  }
}

void resetGame() {
    lives = 6;
    level = 1;
    successfulPresses = 0;
    gameStartTime = millis();
    gameWon = false;
    clearInputBuffer();
    Serial.println("\n▶ Spillet restartes!");
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

void gameTask(void *parameters) {

  gameStartTime = millis();

  while (1) {

    if (gameWon) {
      // Vent på restart med S1
      if (xSemaphoreTake(sem_filled, 50 / portTICK_PERIOD_MS) == pdTRUE) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        int id = buf[tail];
        tail = (tail + 1) % BUF_SIZE;
        xSemaphoreGive(mutex);
        xSemaphoreGive(sem_empty);

        if (id == 0) { // S1 trykket
          resetGame();
        }
      }
      continue;
    }

    if (lives > 0) {

      clearInputBuffer();
      Serial.println("\n👉 Ny runde!");
      vTaskDelay(600 / portTICK_PERIOD_MS);

      Serial.print("Level: ");
      Serial.println(level);

      // Tenn 1 tilfeldig LED
      int target = esp_random() % NUM_PAIRS;
      digitalWrite(interactors[target].ledPin, HIGH);

      int maxReaction = 3000 - (level - 1) * 1000;
      if (maxReaction < 1000) maxReaction = 1000;

      bool correct = false;
      bool wrongPress = false;
      unsigned long startTime = millis();

      while (millis() - startTime < maxReaction) {
        if (xSemaphoreTake(sem_filled, 50 / portTICK_PERIOD_MS) == pdTRUE) {
          xSemaphoreTake(mutex, portMAX_DELAY);
          int id = buf[tail];
          tail = (tail + 1) % BUF_SIZE;
          xSemaphoreGive(mutex);
          xSemaphoreGive(sem_empty);

          if (id == target) correct = true;
          else wrongPress = true;
          break;
        }
      }

      digitalWrite(interactors[target].ledPin, LOW);

      if (correct) {
        successfulPresses++;
        Serial.println("✨ Riktig!");

        if (level == 3) {
          Serial.println("\n🎉 GRATULERER — DU VANT! 🎉");
          Serial.println("Trykk S1 for å starte på nytt!");
          gameWon = true;
          continue;
        }

        level++;
        continue; // START NESTE RUNDE DIREKTE
      }

      // Feil
      lives--;
      Serial.println("❌ FEIL eller for treg!");
      Serial.print("Lives: ");
      Serial.println(lives);
      vTaskDelay(600 / portTICK_PERIOD_MS);

      if (lives <= 0) {
          unsigned long survived = (millis() - gameStartTime) / 1000;
          Serial.println("\n--- GAME OVER ---");
          Serial.print("Tid overlevd: ");
          Serial.print(survived);
          Serial.println(" sek");
          Serial.print("Antall riktige: ");
          Serial.println(successfulPresses);
          Serial.println("Trykk S1 for restart!");
          gameWon = true;
          continue;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("--- Reaksjonsspill ---");

  
  for (int i = 0; i < NUM_PAIRS; i++) {
    pinMode(interactors[i].btnPin, INPUT_PULLUP);
    pinMode(interactors[i].ledPin, OUTPUT);
  }
  
  // Startnedtelling - kjører kun ved første oppstart
  Serial.println("Gjør deg klar!");
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  Serial.println("3!");
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, HIGH);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, LOW);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  Serial.println("2!");
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, HIGH);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, LOW);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  Serial.println("1!");
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, HIGH);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  for (int i = 0; i < NUM_PAIRS; i++)
  digitalWrite(interactors[i].ledPin, LOW);



  sem_empty = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE);
  sem_filled = xSemaphoreCreateCounting(BUF_SIZE, 0);
  mutex = xSemaphoreCreateMutex();

  attachInterrupt(interactors[0].btnPin, ISR_BTN_0, FALLING);
  attachInterrupt(interactors[1].btnPin, ISR_BTN_1, FALLING);
  attachInterrupt(interactors[2].btnPin, ISR_BTN_2, FALLING);
  attachInterrupt(interactors[3].btnPin, ISR_BTN_3, FALLING);

  xTaskCreatePinnedToCore(gameTask, "GameTask", 4096, NULL, 1, NULL, app_cpu);
}

void loop() {}


void loop() {}
