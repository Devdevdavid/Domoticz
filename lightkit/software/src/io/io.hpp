/**
 * @file io.hpp
 * @brief Handle defines for input and output modules
 * @author David DEVANT
 * @date 19/07/2021
 */

#ifndef IO_IO_HPP
#define IO_IO_HPP

// clang-format off

#include "global.hpp"

#ifdef HAS_IOI2C_BOARD
#include "drivers/ioi2c/ioi2c_group.h"
#endif

/** Special IO board - Special pins are indentified by bits [9:8] */
#define IO_SPECIAL            8
#define IO_SPECIAL_MASK       (0x3 << IO_SPECIAL)
#define io_special_get_pin(i) (i & ~IO_SPECIAL_MASK)

/** MACROS */
#define IO_SPECIAL_NONE       0
#define io_special_none(i)    (i)
#define is_io_special_none(i) ((i & IO_SPECIAL_MASK) == IO_SPECIAL_NONE)

#define IO_SPECIAL_IOI2C       (1 << IO_SPECIAL)
#define io_special_ioi2c(i)    (i | IO_SPECIAL_IOI2C)
#define is_io_special_ioi2c(i) ((i & IO_SPECIAL_MASK) == IO_SPECIAL_IOI2C)

#ifdef HAS_IOI2C_BOARD
    #ifdef IO_OUTPUTS_CPP
        const uint8_t ioi2cAddrArray[] = IOI2C_ADDRESSES;
        IOI2CGroup    ioi2cGroup(IOI2C_SDA, IOI2C_SCL, ioi2cAddrArray, sizeof(ioi2cAddrArray));
    #else
        extern const uint8_t ioi2cAddrArray[];
        extern IOI2CGroup    ioi2cGroup;
    #endif
#endif

#endif /* IO_IO_HPP */