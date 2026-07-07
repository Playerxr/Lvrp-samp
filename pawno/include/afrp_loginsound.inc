/*
	afrp_loginsound.inc - Son d'ambiance pendant l'ecran de login

	Demarre quand le dialog mot de passe s'affiche (joueur avec compte
	existant), se repete toutes les 3s (PlayerPlaySound n'est pas un son
	en boucle nativement), et s'arrete net une fois la connexion validee
	(gPlayerLogged[playerid] = 1).

	Hooks :
	- LoginSound_Start(playerid) : a appeler juste avant le 1er
	  ShowPlayerDialog(playerid,12346,...) (OnPlayerRequestClass)
	- LoginSound_Stop(playerid)  : a appeler juste apres
	  gPlayerLogged[playerid] = 1 (login reussi)
	- LoginSound_OnDisconnect(playerid) : cleanup si deco pendant le login
*/

#if defined AFRP_LOGINSOUND_INCLUDED
	#endinput
#endif
#define AFRP_LOGINSOUND_INCLUDED

#define LOGINSOUND_ID        1058   // son d'ambiance (boucle manuelle)
#define LOGINSOUND_INTERVAL  3000   // ms entre 2 declenchements

new loginSoundActive[MAX_PLAYERS];

forward LoginSound_Tick(playerid);
public LoginSound_Tick(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(!loginSoundActive[playerid]) return 0;
	PlayerPlaySound(playerid, LOGINSOUND_ID, 0.0, 0.0, 0.0);
	SetTimerEx("LoginSound_Tick", LOGINSOUND_INTERVAL, false, "i", playerid);
	return 1;
}

stock LoginSound_Start(playerid)
{
	if(loginSoundActive[playerid]) return 0; // deja en cours
	loginSoundActive[playerid] = 1;
	LoginSound_Tick(playerid);
	return 1;
}

stock LoginSound_Stop(playerid)
{
	loginSoundActive[playerid] = 0;
	return 1;
}

stock LoginSound_OnDisconnect(playerid)
{
	loginSoundActive[playerid] = 0;
	return 1;
}
