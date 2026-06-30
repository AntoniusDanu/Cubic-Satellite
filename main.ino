#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <SPI.h>
#include <LoRa.h>

#include <DHT.h>

//=============================
// DHT
//=============================
#define DHTPIN      2
#define DHTTYPE     DHT22

DHT dht(DHTPIN, DHTTYPE);

//=============================
// LoRa Pin
//=============================
#define LORA_SS     10
#define LORA_RST    9
#define LORA_DIO0   3

//=============================
// LoRa Configuration
//=============================
#define LORA_FREQ          433E6

#define LORA_SYNC_WORD     0xF3
#define LORA_TX_POWER      17

#define LORA_SF            7
#define LORA_BW            125E3
#define LORA_CR            5
#define LORA_PREAMBLE      8

//=============================
// Global Variable
//=============================
float suhu = 0.0;
float hum  = 0.0;

//====================================================
// Inisialisasi LoRa
//====================================================
bool initLoRa()
{
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.println("ERROR: LoRa Initialization Failed!");
    return false;
  }

  // Konfigurasi Radio
  LoRa.setSyncWord(LORA_SYNC_WORD);
  LoRa.setTxPower(LORA_TX_POWER);

  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);

  LoRa.setPreambleLength(LORA_PREAMBLE);

  LoRa.enableCrc();

  Serial.println("--------------------------------");
  Serial.println("LoRa Initialized Successfully");
  Serial.print("Frequency : ");
  Serial.println("433 MHz");

  Serial.print("Sync Word : 0x");
  Serial.println(LORA_SYNC_WORD, HEX);

  Serial.print("SF         : ");
  Serial.println(LORA_SF);

  Serial.print("Bandwidth  : ");
  Serial.println("125 kHz");

  Serial.print("CodingRate : 4/");
  Serial.println(LORA_CR);

  Serial.println("--------------------------------");

  return true;
}

//====================================================
// Task Membaca DHT
//====================================================
void TaskReadDHT(void *pvParameters)
{
  (void) pvParameters;

  while (1)
  {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
      suhu = t;
      hum = h;

      Serial.print("Temperature : ");
      Serial.print(suhu);
      Serial.print(" °C");

      Serial.print("   Humidity : ");
      Serial.print(hum);
      Serial.println(" %");
    }
    else
    {
      Serial.println("DHT Read Failed");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

//====================================================
// Task Kirim LoRa
//====================================================
void TaskLoRa(void *pvParameters)
{
  (void) pvParameters;

  char payload[40];

  while (1)
  {
    snprintf(payload, sizeof(payload),
             "TEMP=%.1f,HUM=%.1f",
             suhu,
             hum);

    LoRa.beginPacket();
    LoRa.print(payload);
    LoRa.endPacket();

    Serial.print("LoRa TX : ");
    Serial.println(payload);

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

//====================================================
// Setup
//====================================================
void setup()
{
  Serial.begin(9600);

  while (!Serial);

  Serial.println();
  Serial.println("Starting System...");

  dht.begin();

  if (!initLoRa())
  {
    while (1);
  }

  xTaskCreate(
    TaskReadDHT,
    "ReadDHT",
    128,
    NULL,
    2,
    NULL);

  xTaskCreate(
    TaskLoRa,
    "LoRaSend",
    192,
    NULL,
    1,
    NULL);

  vTaskStartScheduler();
}

//====================================================
// Loop
//====================================================
void loop()
{
}
