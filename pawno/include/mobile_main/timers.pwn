/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			main/timers.pwn
*/

forward HideNotification(playerid);
public HideNotification(playerid)
{
    // Le joueur peut s'etre deconnecte pendant les 2 secondes du timer : ses
    // TextDraws sont alors deja detruites.
    if(!IsPlayerConnected(playerid)) return;

    for(new i = 0; i < 4; i++)
    {
        if(TEXTDRAW_NOTIFICATION[playerid][i] == PlayerText:INVALID_TEXT_DRAW) continue;
    	PlayerTextDrawHide(playerid, TEXTDRAW_NOTIFICATION[playerid][i]);
    }
}