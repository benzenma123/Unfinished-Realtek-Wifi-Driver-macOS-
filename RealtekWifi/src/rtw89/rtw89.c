#include "rtw89.h"
#include "rtw8852b.h"

/* Globals exposed outside rtw89 */
/* (These are declared in rtw8852be.c for PCIe-specific stuff) */

/* Initialize the rtw89 core device */
int rtw89_core_init(struct rtw89_dev *rtwdev)
{
    if (!rtwdev) return -1;
    if (!rtwdev->chip) {
        IOLog("rtw89: No chip info registered\n");
        return -1;
    }
    IOLog("rtw89: Core init complete (chip_id=%d)\n", rtwdev->chip->chip_id);
    return 0;
}

/* Deinitialize */
void rtw89_core_deinit(struct rtw89_dev *rtwdev)
{
    if (!rtwdev) return;
    rtwdev->flags = 0;
}

/* Check if rtw89 is RTL8852B series */
int rtw89_is_rtl885xb(struct rtw89_dev *rtwdev)
{
    return rtwdev->chip && rtwdev->chip->chip_id == RTL8852B;
}
