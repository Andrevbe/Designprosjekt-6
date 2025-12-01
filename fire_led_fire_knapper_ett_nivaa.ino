// Bruker kun kjerne 1 for demonstrasjon
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define NUM_PAIRS 4
int level = 1;
int sucessFullPresses = 0;
usingnes long gameStartTime = 0;
int currentId = -1;
int lives = 6;


enum {BUF_SIZE = 10};                         // Størrelse på buffer

// Globals
static int buf[BUF_SIZE];             // Shared buffer
static int head = 0;                  // Writing index to buffer
static int tail = 0;                  // Reading index to buffer
static SemaphoreHandle_t mutex;       // Lock access to buffer and Serial
static SemaphoreHandle_t sem_empty;   // Counts number of empty slots in buf
static SemaphoreHandle_t sem_filled;  // Counts number of filled slots in buf


struct interactor {
const int id;
const int btnPin;
const int ledPin;
};

interactor interactors[NUM_PAIRS] {
  {0, 12, 13},
  {1, 14, 27},
  {2, 26, 25},
  {3, 33, 32}
};

/* 
* BinÃ¦r semafor for Ã¥ sÃ¸rge for at id-nummeret blir lageret i interactor-tasken. 
* Dette brukes for Ã¥ identifisere hvilken knapp som har blitt trykket inn.
*/


/* Identifikator for interactor-oppgaven */
static TaskHandle_t interactorTask[4];

/*
* Avbruddsrutine for knappetrykk. Vekker den pÃ¥koblete interactorTask-trÃ¥den. 
*/
void IRAM_ATTR ISR_BTN_0() {
  BaseType_t task_woken = pdFALSE;    // Forteller om oppgaven har blitt "vekket" inne i ISR
  int id = 0;                         // Setter id til knappen som tilhører denne ISR. 0 betyr 

  if (xSemaphoreTakeFromISR(sem_empty, &task_woken) == pdTRUE){ 
  // Hvis vi får pdTRue så er det en plass for å lagre nytt knappetrykk
  xSemaphoreTakeFromISR(mutex, &task_woken); // Låser buffer
  buf[head] = id;                            // Legger knappens id inn på posisjon head
  head = (head + 1) % BUF_SIZE;
  xSemaphoreGiveFromISR(mutex, &task_woken); // Gir tilbake buffer
  xSemaphoreGiveFromISR(sem_filled, &task_woken);
  }
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR ISR_BTN_1() {
  BaseType_t task_woken = pdFALSE;

  int id = 1;

  if (xSemaphoreTakeFromISR(sem_empty, &task_woken) == pdTRUE) {
    xSemaphoreTakeFromISR(mutex, &task_woken);
    buf[head] = id;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGiveFromISR(mutex, &task_woken); // Gir tilbake buffer
    xSemaphoreGiveFromISR(sem_filled, &task_woken);
  }

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR ISR_BTN_2() {
  BaseType_t task_woken = pdFALSE;

  int id = 2;

  if (xSemaphoreTakeFromISR(sem_empty, &task_woken) == pdTRUE) {
    xSemaphoreTakeFromISR(mutex, &task_woken);
    buf[head] = id;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGiveFromISR(mutex, &task_woken); // Gir tilbake buffer
    xSemaphoreGiveFromISR(sem_filled, &task_woken);
  }

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR ISR_BTN_3() {
  BaseType_t task_woken = pdFALSE;

  int id = 3;

  if (xSemaphoreTakeFromISR(sem_empty, &task_woken) == pdTRUE) {
    xSemaphoreTakeFromISR(mutex, &task_woken);
    buf[head] = id;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGiveFromISR(mutex, &task_woken); // Gir tilbake buffer
    xSemaphoreGiveFromISR(sem_filled, &task_woken);
  }

  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

/*
* Interactor-oppgaven, som reagerer pÃ¥ knappetrykk og styrer en LED basert pÃ¥ dette. Laget for Ã¥ kunne brukes for
* flere knappe-LED-par ved Ã¥ ha en parameteriserbar ID.  En binÃ¦r semafor brukes for Ã¥ signalisere at IDen er lagret lokalt.
*/
void gameTask(void *parameters) {


  while(1) {
    
    if (lives > 0) {
      Serial.println("Gjør deg klar!");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      Serial.println("3!");
      for (int i = 0; i < NUM_PAIRS; i++) {
        digitalWrite(interactors[i].ledPin, HIGH);
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      for (int i = 0; i < NUM_PAIRS; i++){
      digitalWrite(interactors[i].ledPin, LOW);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      Serial.println("2!");
      for (int i = 0; i < NUM_PAIRS; i++) {
        digitalWrite(interactors[i].ledPin, HIGH);
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      for (int i = 0; i < NUM_PAIRS; i++){
      digitalWrite(interactors[i].ledPin, LOW);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      Serial.println("1!");
      for (int i = 0; i < NUM_PAIRS; i++) {
        digitalWrite(interactors[i].ledPin, HIGH);
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);

      // Vent en tilfelig periode med tid, her satt til mellom 1-3 sekunder
      for (int i = 0; i < NUM_PAIRS; i++){
      digitalWrite(interactors[i].ledPin, LOW);
      }
      int wait_ms = 1000 + (esp_random() % 2000);
      vTaskDelay(wait_ms / portTICK_PERIOD_MS);

      // Ten LED som signaliserer start på runden
      currentId = esp_random() % NUM_PAIRS;
      int ledPin = interactors[currentId].ledPin;
      Serial.println("Kjør");
      Serial.println(currentId);

      digitalWrite(ledPin, HIGH);

      // Vent på et knappetrykk
      if (xSemaphoreTake(sem_filled, 3000 / portTICK_PERIOD_MS) == pdTRUE) {
        // Les ID-en til knappen fra byffer
        int pressedId;
        xSemaphoreTake(mutex, portMAX_DELAY);
        pressedId = buf[tail];
        tail = (tail + 1) % BUF_SIZE;
        xSemaphoreGive(mutex);

        // Gi plass tilbake i bufferen
        xSemaphoreGive(sem_empty);

        if (pressedId == currentId) {
          Serial.println("Bra! du rakk det");
        } else {
          Serial.println("Feil knapp! Du trykket ");
          Serial.println(pressedId);
          --lives;
        } 

      digitalWrite(ledPin, LOW);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      Serial.println("Nåværende liv: ");
      Serial.println(lives);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      } else {
        Serial.println("---GAME OVER---");
        Serial.println("Du er tom for liv");
        Serial.println("Spillet avsluttes");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        vTaskDelete(NULL);
      }
    }
  }
}

void setup() {
    Serial.begin(115200);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    Serial.println("---Reaksjonsspill---");
    

    for (int i = 0; i < NUM_PAIRS; i++) {
      pinMode(interactors[i].btnPin, INPUT_PULLUP);
      pinMode(interactors[i].ledPin, OUTPUT);
    }

    // Lag semaphores mutex før taskene begynner
    sem_empty = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE);
    sem_filled = xSemaphoreCreateCounting(BUF_SIZE, 0);
    mutex = xSemaphoreCreateMutex();

    // Fest interrupts
    attachInterrupt(interactors[0].btnPin, ISR_BTN_0, FALLING);
    attachInterrupt(interactors[1].btnPin, ISR_BTN_1, FALLING);
    attachInterrupt(interactors[2].btnPin, ISR_BTN_2, FALLING);
    attachInterrupt(interactors[3].btnPin, ISR_BTN_3, FALLING);

    xTaskCreatePinnedToCore(gameTask,
                            "Game Task",
                            4096,
                            NULL,
                            1,
                            NULL,
                            app_cpu);
  
}

void loop() {


}