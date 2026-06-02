#include <SPI.h>
#include <LoRa.h>
#include <Arduino_FreeRTOS.h>

// Pin LoRa
#define LORA_SS    10
#define LORA_RST   9
#define LORA_DIO0  2

// Task Handle
TaskHandle_t TaskLoRaHandle;

// =========================
// Task Pengirim LoRa
// =========================
void TaskLoRaSend(void *pvParameters)
{
  (void) pvParameters;

  uint32_t counter = 0;

  while (1)
  {
    String msg = "Hello " + String(counter);

    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();

    Serial.print("Sent: ");
    Serial.println(msg);

    counter++;

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void setup()
{
  Serial.begin(9600);

  while (!Serial);

  Serial.println("LoRa TX RTOS Start");

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6))
  {
    Serial.println("LoRa init failed!");
    while (1);
  }

  Serial.println("LoRa init OK");

  xTaskCreate(
    TaskLoRaSend,
    "LoRaSend",
    256,
    NULL,
    1,
    &TaskLoRaHandle
  );

  vTaskStartScheduler();
}

void loop()
{
  // Tidak digunakan karena FreeRTOS
}
