/*
	mobile.inc - integration mobile_system inline dans LVRP.pwn

	Pre-requis dans LVRP.pwn AVANT le #include <mobile> :
	  - #include <a_samp>, <a_mysql>
	  - new MySQL:MYSQL;            (deja present ligne ~1424)
	  - new hasPhone[MAX_PLAYERS];  (deja present ligne ~4761)
	  - new playerNumber[MAX_PLAYERS];
	  - new playerCredit[MAX_PLAYERS];

	Hooks a brancher dans LVRP.pwn :
	  - OnGameModeInit          -> Mobile_Init();
	  - OnGameModeExit          -> Mobile_Exit();
	  - OnPlayerConnect         -> Mobile_OnPlayerConnect(playerid);
	  - OnPlayerText            -> if(!Mobile_OnPlayerText(playerid, text)) return 0;
	  - OnPlayerClickPlayerTextDraw -> Mobile_OnPlayerClickPlayerTextDraw(playerid, playertextid);
	  - OnDialogResponse        -> Mobile_OnDialogResponse(playerid, dialogid, response, listitem, inputtext);
	  - OnPlayerCommandText     -> dispatch strcmp /mobile, /mobitel, /shop, /market, /mobilesettings
*/

// Config + variables (sans hasPhone/playerNumber/playerCredit deja dans LVRP)
#include "mobile_main/config.pwn"

// Strings de traduction
#include "mobile_translation/loader.pwn"

// MySQL : db_handle = MYSQL (define), pas de connexion redondante
#include "mobile_mysql/connection.pwn"

// Textdraws de toutes les apps
#include "mobile_textdraws/noapps_td.pwn"
#include "mobile_textdraws/home_td.pwn"
#include "mobile_textdraws/notes_td.pwn"
#include "mobile_textdraws/notification_td.pwn"
#include "mobile_textdraws/bank_td.pwn"
#include "mobile_textdraws/call_td.pwn"
#include "mobile_textdraws/sms_td.pwn"
#include "mobile_textdraws/time_td.pwn"
#include "mobile_textdraws/twitter_td.pwn"

// Logique : timers, fonctions, callbacks, commandes
#include "mobile_main/timers.pwn"
#include "mobile_main/functions.pwn"
#include "mobile_main/callbacks.pwn"
#include "mobile_main/commands.pwn"
