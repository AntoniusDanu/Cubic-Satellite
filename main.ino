#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <RS-FEC.h>

//=========================
// DHT22
//=========================
#define DHTPIN   2
#define DHTTYPE  DHT22
DHT dht(DHTPIN, DHTTYPE);

//=========================
// LoRa
//=========================
#define LORA_SS        10
#define LORA_RST       9
#define LORA_DIO0      3

#define LORA_FREQ      433E6
#define LORA_TX_POWER  14
#define LORA_SF        7
#define LORA_BW        125E3
#define LORA_CR        5
#define LORA_SYNC      0x12

//=========================
// Reed Solomon
//=========================
const uint8_t MSG_LEN   = 32;
const uint8_t ECC_LEN   = 8;
const uint8_t TOTAL_LEN = 40;

RS::ReedSolomon<MSG_LEN,ECC_LEN> rs;

//=========================
// Buffer
//=========================
char message[MSG_LEN];
uint8_t encoded[TOTAL_LEN];

//=========================
void setup()
{
    Serial.begin(115200);

    dht.begin();

    LoRa.setPins(LORA_SS,LORA_RST,LORA_DIO0);

    if(!LoRa.begin(LORA_FREQ))
    {
        while(1);
    }

    LoRa.setTxPower(LORA_TX_POWER);
    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setCodingRate4(LORA_CR);
    LoRa.setSyncWord(LORA_SYNC);
    LoRa.enableCrc();
}

void loop()
{
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h))
    {
        Serial.println(F("DHT22 Error"));
        delay(2000);
        return;
    }

    memset(message, 0, MSG_LEN);

    char tStr[8];
    char hStr[8];

    dtostrf(t, 4, 1, tStr);
    dtostrf(h, 4, 1, hStr);

    snprintf(message,
             MSG_LEN,
             "{\"id\":3,\"t\":%s,\"h\":%s}",
             tStr,
             hStr);

    // Encode Reed-Solomon
    rs.Encode(message, encoded);

    // Kirim LoRa
    LoRa.beginPacket();
    LoRa.write(encoded, TOTAL_LEN);
    bool status = LoRa.endPacket();

    // ==========================
    // Serial Monitor
    // ==========================
    Serial.println(F("--------------------------------"));

    Serial.print(F("Temperature : "));
    Serial.print(t, 1);
    Serial.println(F(" °C"));

    Serial.print(F("Humidity    : "));
    Serial.print(h, 1);
    Serial.println(F(" %"));

    Serial.print(F("JSON        : "));
    Serial.println(message);

    Serial.print(F("Packet Size : "));
    Serial.print(TOTAL_LEN);
    Serial.println(F(" Byte"));

    Serial.print(F("LoRa Status : "));
    if (status)
        Serial.println(F("SEND OK"));
    else
        Serial.println(F("SEND FAILED"));

    Serial.println(F("--------------------------------\n"));

    delay(5000);
}
