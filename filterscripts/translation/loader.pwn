/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			translation/loader.pwn
*/

#if LANG == 1
	#include "translation/ba.pwn"
#elseif LANG == 2
	#include "translation/en.pwn"
#elseif LANG == 3
	#include "translation/de.pwn"
#else
	#include "translation/en.pwn"
#endif
