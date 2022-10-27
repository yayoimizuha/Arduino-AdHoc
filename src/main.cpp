#include <Arduino.h>
#include <MultiUART.h>


#define DELAY 100
#define MACHINE_NUMBER 1
#define CONNECTED_DEVICES 3

MultiUART *devices[CONNECTED_DEVICES];

static_assert(CONNECTED_DEVICES <= 4, "Devices are too much.");
//static_assert(MACHINE_NUMBER > 0 && MACHINE_NUMBER < 10, "MACHINE_NUMBER must be 1 digit.");

constexpr uint_fast8_t PinList[CONNECTED_DEVICES][2] = {
        {2, 3},
        {4, 5},
        {6, 7},
};
static volatile char buffer[CONNECTED_DEVICES][256];
int addresses[CONNECTED_DEVICES];

String receive_text(MultiUART port);

String send_text(MultiUART port);

void sleep(double time);

void setup() {
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("Start Program!");
    Serial.print("This device address is:");
    Serial.println(MACHINE_NUMBER);

    for (int i = 0; i < CONNECTED_DEVICES; ++i) {
        devices[i] = new MultiUART(PinList[i][0], PinList[i][1]);
        devices[i]->begin(800);
        devices[i]->setRxBuffer(buffer[i], 256);
    }

    sleep(1e+5);

    for (auto &i: devices) {
        i->write(("machine_address:" + String(MACHINE_NUMBER) + '\0').c_str());
    }

    uint_fast8_t got_address = 0;
    while (true) {
        for (int i = 0; i < CONNECTED_DEVICES; ++i) {
            if (devices[i]->last() == '\0') {
                int len = devices[i]->available();
                char *receive_txt = (char *) malloc(sizeof(char) * len);
                for (int j = 0; j < len; ++j) {
                    receive_txt[j] = static_cast<char>(devices[i]->read());
                }
                devices[i]->read();
                if (!String(receive_txt).startsWith("machine_address:")) {
                    //error
                    continue;
                } else {
                    addresses[i] = String(receive_txt).substring(16).toInt();
                    got_address++;
                }
            }
        }
        if (got_address == CONNECTED_DEVICES)break;
    }
}

void loop() {
    for (int i = 0; i < CONNECTED_DEVICES; ++i) {
        if (devices[i]->last() == '\0') {
            int len = devices[i]->available();
            char *receive_txt = (char *) malloc(sizeof(char) * len);
            for (int j = 0; j < len; ++j) {
                receive_txt[j] = static_cast<char>(devices[i]->read());
            }
            devices[i]->read();
            int num_order;
            for (int j = 0;; ++j) {
                if (receive_txt[j] == ',') {
                    num_order = j;
                    break;
                }
            }
            auto dest_device_id = String(receive_txt).substring(0, num_order - 1).toInt();
            if (dest_device_id == MACHINE_NUMBER) {
                Serial.print("Text received!: ");
                Serial.println(String(receive_txt).substring(num_order));
                //Print text
            } else {
                bool call_forward = false;
                for (int j = 0; j < CONNECTED_DEVICES; ++j) {
                    if (addresses[j] == dest_device_id) {
                        devices[j]->write((String(receive_txt) + '\0').c_str());
                        call_forward = true;
                    }
                }
                auto determinate = false;
                uint_fast8_t call_forward_addr = 0;
                while (!determinate) {
                    call_forward = random(0, CONNECTED_DEVICES - 1);
                    if (call_forward == i) {
                        continue;
                    } else {
                        determinate = true;
                    }
                }
                devices[call_forward_addr]->write((String(receive_txt) + '\0').c_str());
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