#define DISPLAY_AVAILABLE

#define DEVICE_ADDR 87
#define CONNECTED_DEVICES 2
#define COMM_FREQ 2400

constexpr uint_fast8_t PinList[CONNECTED_DEVICES][2] = {
// { TX , RX }
        {7, 6},
        {4, 3},
//{2, 3},
};
