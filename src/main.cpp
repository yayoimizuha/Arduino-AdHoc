#include<Arduino.h>
#include <MultiUART.h>
#include <vector>
#include <ArduinoSTL.h>
#include <string>


#define DEVICE_ADDR 87
#define CONNECTED_DEVICES 2
#define COMM_FREQ 3200

constexpr uint_fast8_t PinList[CONNECTED_DEVICES][2] = {
        // { TX , RX }
        {7, 6},
        {4, 3},
        //{2, 3},
};


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
    Serial.begin(COMM_FREQ);
    sleep(5e+3);
    Serial.println("実行開始");
    Serial.println("[送信先アドレス,送信したい内容]の形式で入力してください。");
    Serial.print("このデバイスのアドレスは: ");
    Serial.println(DEVICE_ADDR);

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
    Serial.println("MultiUARTを開始");

    sleep(1e+3 * 10);
    Serial.println("ネゴシエーションを開始します。");

    for (auto &i: devices) {
        i->write(("machine_address:" + String(DEVICE_ADDR) + '\0').c_str());
    }
    Serial.println("アドレスの通知を完了");

    uint_fast8_t got_address = 0;
    while (true) {
        for (int i = 0; i < CONNECTED_DEVICES; ++i) {
            if (devices[i]->available() != 0) {
                digitalWrite(13, HIGH);
                char *txt = (char *) malloc(sizeof(char));
                uint_fast8_t txt_len = 0;
                while (devices[i]->available() != 0) {
                    txt[txt_len] = devices[i]->read();
                    Serial.print(txt[txt_len]);
                    sleep(1);
                    txt_len++;
                    realloc(txt, sizeof(char) * (txt_len + 1));
                }
                Serial.println("");
                txt[txt_len] = '\0';
                if (!String(txt).startsWith("machine_address:")) {
                    //error
                    continue;
                } else {
                    addresses[i] = static_cast<int>(String(txt).substring(16).toInt());
                    Serial.println("record address: " + String(addresses[i]));
                    got_address++;
                }
                free(txt);
            }
        }
        if (got_address == CONNECTED_DEVICES)break;
    }
    Serial.println("ネゴシエーションを完了しました。");
}


__attribute__((unused)) void loop() {
    randomSeed(micros());
    if (Serial.available()) {
        string input = Serial.readString().c_str();
        string comma = ",";
        auto comma_place = search(input.begin(), input.end(), comma.begin(), comma.end());
        if (comma_place == input.end()) {
            cout << "正しいフォーマットで入力してください。" << endl;
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
            cout << address << "番に" << endl;
        }
    }
    for (auto &device: devices) {
        if (device->available()) {
            string input = device->readString().c_str();
            string comma = ",";
            auto comma_place = search(input.begin(), input.end(), comma.begin(), comma.end());
            if (comma_place == input.end()) {
                cout << "正しいフォーマットで入力してください。" << endl;
            } else {
                flag exist_next = false;
                auto address_str = string(input.begin(), comma_place);
                auto address = String(address_str.c_str()).toInt();
                if (address == DEVICE_ADDR) {
                    cout << input << endl;
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
