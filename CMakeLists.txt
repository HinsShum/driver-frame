# @file driver/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2021/01/22 11:13:07
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

set(INC_VPATH ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/device.c ${CMAKE_CURRENT_SOURCE_DIR}/driver.c)

if(DEFINED AT24CXX)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/at24cxx.c)
endif()

if(DEFINED BUZZER)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/buzzer.c)
endif()

if(DEFINED D100)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/d100.c)
endif()

if(DEFINED DS28EXX)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ds28exx.c)
endif()

if(DEFINED FLASH)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/flash.c)
endif()

if(DEFINED FM13DT160)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/fm13dt160.c)
endif()

if(DEFINED FM25QXX)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/fm25qxx.c)
endif()

if(DEFINED GPIO)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/gpio.c)
endif()

if(DEFINED I2C_BUS)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/i2c_bus.c)
endif()

if(DEFINED INFRARED)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/infrared.c)
endif()

if(DEFINED LED)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/led.c)
endif()

if(DEFINED YT8512X)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/motorcomm_yt8512x.c)
endif()

if(DEFINED MS523)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ms523.c)
endif()

if(DEFINED MS8837)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ms8837.c)
endif()

if(DEFINED RTC)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rtc.c)
endif()

if(DEFINED SEPS525)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/seps525.c)
endif()

if(DEFINED SERIAL)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/serial.c)
endif()

if(DEFINED SI446X)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/si446x.c)
endif()

if(DEFINED SILKID)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/silkid.c)
endif()

if(DEFINED TIMER)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/timer.c)
endif()

if(DEFINED W1_BUS)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/w1_bus.c)
endif()

if(DEFINED WDT)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/wdt.c)
endif()

if(DEFINED ZAZXXX)
    list(APPEND C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/zazxxx.c)
endif()

set(G_C_FILES ${G_C_FILES} ${C_FILES} PARENT_SCOPE)
set(G_INC_VPATH ${G_INC_VPATH} ${INC_VPATH} PARENT_SCOPE)