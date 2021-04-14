/**
 * @file ncv7718.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef NCV7718_H
#define NCV7718_H
#ifdef __cplusplus
extern "C" {
#endif
#include "spibus/spibus.h"

typedef enum
{
    NCV7718_OC_SHUTOFF = -3,
    NCV7718_PSU_FAIL = -2,
    NCV7718_FAILURE = -1,
    NCV7718_SUCCESS = 1,
    NCV7718_THERMAL_WARNING = 2,
    NCV7718_UNDER_LOAD,
} NCV7718_RETCODE;

typedef struct 
{
    spibus bus[1];
    uint8_t out_en;
    uint8_t out_conf;
} ncv7718;
/**
 * @brief Initialize an NCV7718 device
 * 
 * @param dev ncv7718 struct
 * @param bus SPI Bus ID
 * @param cs SPI CS ID
 * @param gpiocs Positive for valid GPIO used as chipselect, negative for native SPI CS
 * @return int Positive on success, negative on failure
 */
int ncv7718_init(ncv7718 *dev, int bus, int cs, int gpiocs);
/**
 * @brief Set output of pairs of hex half bridges
 * 
 * @param dev ncv7718 struct
 * @param axis X, Y, Z pairs (1-2, 3-4, 5-6)
 * @param direction 0 for disabled, +1 for odd-on, even-off, -1 for even-on, odd-off
 */
int ncv7718_set_output(ncv7718 *dev, int axis, int direction);
/**
 * @brief Apply the outputs set using ncv7718_set_output
 * 
 * @param dev ncv7718 struct
 * @return int positive on success, negative on failure
 */
NCV7718_RETCODE ncv7718_exec_output(ncv7718 *dev);
/**
 * @brief 
 * 
 * @param dev 
 * @return NCV7718_RETCODE 
 */
NCV7718_RETCODE ncv7718_por(ncv7718 *dev);
/**
 * @brief Turn off outputs and close an ncv7718 device
 * 
 * @param dev ncv7718 struct
 */
void ncv7718_destroy(ncv7718 *dev);

#ifdef NCV7718_PRIVATE
typedef union
{
    struct __attribute__((packed))
    {
        unsigned char ovlo  : 1; ///< over voltage lockout
        unsigned char hbcnf : 6; ///< half bridge 1 configuration (1 -> LS1 off and HS1 on, 0 -> LS1 on and HS1 off)
        unsigned char hben  : 6; ///< half bridge 1 enable (1 -> bridge in use, 0 -> bridge not in use)
        unsigned char uldsc : 1; ///< under load detection shutdown
        unsigned char hbsel : 1; ///< half bridge selection (needs to be set to 0)
        unsigned char srr   : 1; ///< status reset register: 1 -> clear all faults and reset
    };
    unsigned short data;
} ncv7718_cmd;

typedef union
{
    struct
    {
        unsigned char tw   : 1; ///< Thermal warning
        unsigned char hbcr : 6; ///< half bridge 1 configuration reporting (1 -> LS1 off and HS1 on, 0 -> LS1 on and HS1 off)
        unsigned char hbst : 6; ///< half bridge 1 enable (1 -> bridge in use, 0 -> bridge not in use)
        unsigned char uld  : 1; ///< Under load detection
        unsigned char psf  : 1; ///< Power supply failure
        unsigned char ocs  : 1; ///< Overcurrent tripped
    } __attribute__((packed));
    unsigned short data;
} ncv7718_data;
#endif

#ifdef __cplusplus
}
#endif
#endif