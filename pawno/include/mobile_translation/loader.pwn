/*
	translation/loader.pwn - chemins depuis pawno/include/
*/

#if LANG == 1
	#include "mobile_translation/ba.pwn"
#elseif LANG == 2
	#include "mobile_translation/en.pwn"
#elseif LANG == 3
	#include "mobile_translation/de.pwn"
#elseif LANG == 4
	#include "mobile_translation/fr.pwn"
#else
	#include "mobile_translation/fr.pwn"
#endif
