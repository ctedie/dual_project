/*
 * txt_lib.h
 *
 *  Created on: 27 janv. 2017
 *      Author: Dev
 */

#ifndef DRIVER_LIB_TXT_LIB_H_
#define DRIVER_LIB_TXT_LIB_H_

#define COMMAND					"\033"

#define	FOREGROUND_BLACK        "[31m"
#define FOREGROUND_RED          "[32m"
#define FOREGROUND_GREEN        "[33m"
#define FOREGROUND_YELLOW       "[34m"
#define FOREGROUND_BLUE         "[35m"
#define FOREGROUND_MAGENTA      "[36m"
#define FOREGROUND_CYAN         "[37m"
#define FOREGROUND_WHITE        "[38m"
#define FOREGROUND_DEFAULT      "[39m"

#define BACKGROUND_BLACK        "[41m"
#define BACKGROUND_RED          "[42m"
#define BACKGROUND_GREEN        "[43m"
#define BACKGROUND_YELLOW       "[44m"
#define BACKGROUND_BLUE         "[45m"
#define BACKGROUND_MAGENTA      "[46m"
#define BACKGROUND_CYAN         "[47m"
#define BACKGROUND_WHITE        "[48m"
#define BACKGROUND_DEFAULT      "[49m"

#define SAVE_SCREEN		"\033[?47h"
#define RESTORE_SCREEN	"\033[?47l"

#define MOVE_CURSOR_UP          "\033[A"
#define MOVE_CURSOR_DOWN        "\033[B"
#define MOVE_CURSOR_RIGHT       "\033[C"
#define MOVE_CURSOR_LEFT		"\033[D"

#endif /* DRIVER_LIB_TXT_LIB_H_ */
