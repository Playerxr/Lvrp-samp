/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			mobile_system.pwn
*/

/* 
Requirements:
	https://github.com/pawn-lang/YSI-Includes
	https://github.com/pBlueG/SA-MP-MySQL
	https://github.com/katursis/Pawn.CMD
*/

#include < a_samp >
#include < a_mysql >

// YSI replaced by stock SA-MP equivalents (hook -> public, timer/defer -> SetTimerEx)

#define FILTERSCRIPT
#include < Pawn.CMD >

// Main - config
#include "main/config.pwn"

// Translation
#include "translation/loader.pwn"

// MySQL
#include "mysql/loader.pwn"

// Textdraws
#include "textdraws/loader.pwn"

// Main
#include "main/loader.pwn"
