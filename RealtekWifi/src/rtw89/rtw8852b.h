#ifndef RTW8852B_H
#define RTW8852B_H

struct rtw89_dev;
struct rtw89_chip_info;

extern struct rtw89_chip_info rtw8852b_chip_info;
int rtw8852b_pwr_on_func(struct rtw89_dev *rtwdev);
int rtw8852b_pwr_off_func(struct rtw89_dev *rtwdev);

#endif
