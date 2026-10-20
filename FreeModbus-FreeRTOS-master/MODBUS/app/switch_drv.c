#include "switch_drv.h"

#include "mb.h"
#include "mb_m.h"
#include "mbfunc.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbconfig.h"

int modbus_relay_open_light(void)
{
 return eMBMasterReqWriteCoil(0x01, 0x00, 0xff00, 100);
}

int modbus_relay_close_light(void)
{
 return eMBMasterReqWriteCoil(0x01, 0x00, 0x0000, 100);
}

int modbus_relay_open_fan(void)
{
 return eMBMasterReqWriteCoil(0x01, 0x01, 0xff00, 100);
}

int modbus_relay_close_fan(void)
{
 return eMBMasterReqWriteCoil(0x01, 0x01, 0x0000, 100);
}