////////////////////////////////////////////////////////////////////////////////
//
// Wireless functions
//
// Wifi and Bluetooth
//


#define  INITIAL_BT_TO_CLI 1

void wireless_init(void);
void wireless_loop(void);
void wireless_taskloop(void);

extern int pending_tx;

#define CL_BT_BUFFER_SIZE  1000

extern int cl_bt_in;
extern int cl_bt_out;

extern BYTE cl_bt_buffer[CL_BT_BUFFER_SIZE];
extern int pending_tx;
