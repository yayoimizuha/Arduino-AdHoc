#include<Arduino.h>
#include <MultiUART.h>
#include <vector>
#include <string>

//#include "each/16.h"
//#include "each/23.h"
//#include "each/45.h"
#include "each/87.h"

using namespace std;
typedef bool flag;

MultiUART *devices[CONNECTED_DEVICES];

static_assert(CONNECTED_DEVICES <= 4, "接続デバイス数が多すぎます。4以下にしてください。");
static_assert(DEVICE_ADDR != 0, "デバイスアドレスに0を指定することはできません。");


//static volatile char buffer[CONNECTED_DEVICES][256];
int addresses[CONNECTED_DEVICES];


void sleep(double time);

__attribute__((unused)) void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef DISPLAY_AVAILABLE
    Serial.begin(COMM_FREQ);
#endif
    sleep(5e+3);
#ifdef DISPLAY_AVAILABLE
    Serial.println("実行開始");
    Serial.println("[送信先アドレス,送信したい内容]の形式で入力してください。");
    Serial.print("このデバイスのアドレスは: ");
    Serial.println(DEVICE_ADDR);
#endif
    for (int i = 0; i < CONNECTED_DEVICES; ++i) {
        devices[i] = new MultiUART(PinList[i][0], PinList[i][1]);
        devices[i]->begin(COMM_FREQ);
        //devices[i]->setRxBuffer(buffer[i], 256);
    }
    for (int i = 0; i < CONNECTED_DEVICES; ++i) {
        digitalWrite(LED_BUILTIN, HIGH);
        sleep(500);
        digitalWrite(LED_BUILTIN, LOW);
        sleep(200);
    }
#ifdef DISPLAY_AVAILABLE
    Serial.println("MultiUARTを開始");
#endif
    sleep(1e+3 * 10);
#ifdef DISPLAY_AVAILABLE
    Serial.println("ネゴシエーションを開始します。");
#endif
    for (auto &i: devices) {
        i->write(("machine_address:" + String(DEVICE_ADDR) + '\0').c_str());
    }
#ifdef DISPLAY_AVAILABLE
    Serial.println("アドレスの通知を完了");
#endif
    uint_fast8_t got_address = 0;
    while (true) {
        for (int i = 0; i < CONNECTED_DEVICES; ++i) {
            if (devices[i]->available() != 0) {
                digitalWrite(13, HIGH);
                char *txt = (char *) malloc(sizeof(char));
                uint_fast8_t txt_len = 0;
                while (devices[i]->available() != 0) {
                    txt[txt_len] = devices[i]->read();
#ifdef DISPLAY_AVAILABLE
                    Serial.print(txt[txt_len]);
#endif
                    sleep(1);
                    txt_len++;
                    realloc(txt, sizeof(char) * (txt_len + 1));
                }
#ifdef DISPLAY_AVAILABLE
                Serial.println("");
#endif
                txt[txt_len] = '\0';
                if (!String(txt).startsWith("machine_address:")) {
                    //error
                    continue;
                } else {
                    addresses[i] = static_cast<int>(String(txt).substring(16).toInt());
#ifdef DISPLAY_AVAILABLE
                    Serial.println("record address: " + String(addresses[i]));
#endif
                    got_address++;
                }
                free(txt);
            }
        }
        if (got_address == CONNECTED_DEVICES)break;
    }
#ifdef DISPLAY_AVAILABLE
    Serial.println("ネゴシエーションを完了しました。");
#endif
}


__attribute__((unused)) void loop() {
    randomSeed(micros());
#ifdef DISPLAY_AVAILABLE
    if (Serial.available()) {
        string input = Serial.readString().c_str();
        string comma = ",";
        auto comma_place = search(input.begin(), input.end(), comma.begin(), comma.end());
        if (comma_place == input.end()) {
            Serial.println("正しいフォーマットで入力してください。");
        } else {
            flag exist_next = false;
            auto address_str = string(input.begin(), comma_place);
            auto address = String(address_str.c_str()).toInt();
            for (int i = 0; i < CONNECTED_DEVICES; ++i) {
                if (addresses[i] == address) {
                    devices[i]->write(input.c_str());
                    exist_next = true;
                    continue;
                }
            }
            if (!exist_next) {
                devices[random(CONNECTED_DEVICES)]->write(input.c_str());
            }
            Serial.print(String(address) + "番に: " + String(input.c_str()));
        }
    }
#endif
    for (auto &device: devices) {
        if (device->available()) {
            string input = device->readString().c_str();
            string comma = ",";
            auto comma_place = search(input.begin(), input.end(), comma.begin(), comma.end());
            if (comma_place == input.end()) {
#ifdef DISPLAY_AVAILABLE
                Serial.println("正しいフォーマットで入力してください。");
#endif
            } else {
                flag exist_next = false;
                auto address_str = string(input.begin(), comma_place);
                auto address = String(address_str.c_str()).toInt();
                if (address == DEVICE_ADDR) {
#ifdef DISPLAY_AVAILABLE
                    Serial.println(input.c_str());
#endif
                    continue;
                }
                for (int j = 0; j < CONNECTED_DEVICES; ++j) {
                    if (addresses[j] == address) {
                        devices[j]->write(input.c_str());
                        exist_next = true;
                        continue;
                    }
                }
                if (!exist_next) {
                    devices[random(CONNECTED_DEVICES)]->write(input.c_str());
                }
            }
        }
    }

}

void sleep(double time) {
    auto start_time = millis();
    while (true) {
        if (millis() > start_time + static_cast<unsigned long>(time))break;
    }
}
