#include <SPI.h>
#include <MFRC522.h>

constexpr uint8_t RST_PIN = D3; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4; // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

String tag;

struct Product {
  String name;
  String tagId;
  float price;
  int quantity;
};

Product product_list[] = {
  {"Shirt", "19575128150", 25.0,0},
  {"Pants", "518969166", 35.0,0}
};

struct BillItem {
  String name;
  float price;
  int quantity;
};

BillItem bill[10];
int bill_count = 0;
int scan_count = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();        // Init SPI bus
  rfid.PCD_Init();    // Init MFRC522
}

void loop() {
  if (scan_count < 6) {
    if (!rfid.PICC_IsNewCardPresent()) return;

    if (rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < 4; i++) {
        tag += rfid.uid.uidByte[i];
      }

      for (int i = 0; i < 1; i++) {
        for (int j = 0; j < sizeof(product_list)/sizeof(product_list[0]); j++) {
          if (tag == product_list[j].tagId) {
            bool found = false;
            for (int k = 0; k < bill_count; k++) {
              if (product_list[j].name == bill[k].name) {
                bill[k].quantity++;
                found = true;
                break;
              }
            }
            if (!found) {
              bill[bill_count].name = product_list[j].name;
              bill[bill_count].price = product_list[j].price;
              bill[bill_count].quantity = 1;
              bill_count++;
            }
            Serial.println("Product: " + product_list[j].name + ", Price: " + String(product_list[j].price));
            break;
          }
        }
      }

      tag = "";
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

      scan_count++;
    }
  } else {
    Serial.println("Do you want to continue scanning? (0/1)");
    while (!Serial.available()) {}
    int input = Serial.parseInt();

    if (input == 0) {
      float total_price = 0.0;
      Serial.println("Product\t\tQuantity\tPrice");
      Serial.println("-------------------------------------------");
      for (int i = 0; i < bill_count; i++) {
        Serial.println(bill[i].name + "\t\t" + String(bill[i].quantity) + "\t\t" + String(bill[i].price));
        total_price += (bill[i].price * bill[i].quantity);
      }
      Serial.println("-------------------------------------------");
      Serial.println("Total price: " + String(total_price));

      bill_count = 0;
      scan_count = 0;
    } else {
      scan_count = 0;
    }
  }
}
