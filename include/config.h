/*
 * config.h
 *
 *  Created on: 18 Apr 2017
 *      Author: kopermir
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define GL_NR_OF_O2CELLS (2)

#define GL_HIGH_PPO2_WARNING 140
#define GL_LOW_PPO2_WARNING   40

// battery monitor levels (1024 = 2.56V)
#define GL_BATTERY_100  1180 // 100% 1180 = 2,95V / 0,0025
#define GL_BATTERY_75   1160 //  75%        2,90V
#define GL_BATTERY_50   1140 //  50%        2,85V
#define GL_BATTERY_25   1120 //  25%        2,80V
#define GL_BATTERY_0    1100 //   0%        2,75V

#define GL_BATTERY_FACTOR_MAX 1320

// head up levels
// above 1.6 : red only
// 1.3<->1.6 : red and green
// 1.0<->1.3 : green only
// 0.7<->1.0 : green short
// below 0.7 : gren short and red long


#define GL_PPO2_HIGH_WARNING    130   // red and green
#define GL_PPO2_TOOHIGH_WARNING 160   // red only
#define GL_PPO2_LOW_WARNING     100
#define GL_PPO2_TOOLOW_WARNING   70

#endif /* CONFIG_H_ */
