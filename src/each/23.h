//#define DISPLAY_AVAILABLE

#define DEVICE_ADDR 23
#define CONNECTED_DEVICES 3

constexpr uint_fast8_t PinList[CONNECTED_DEVICES][2] = {
// { TX , RX }
{7, 6},
{5, 4},
{2, 3},
};