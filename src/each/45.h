//#define DISPLAY_AVAILABLE

#define DEVICE_ADDR 45
#define CONNECTED_DEVICES 2
#define COMM_FREQ 2400

constexpr uint_fast8_t PinList[CONNECTED_DEVICES][2] = {
// { TX , RX }
        {13, 12},
        {9,  10},
//{2, 3},
};