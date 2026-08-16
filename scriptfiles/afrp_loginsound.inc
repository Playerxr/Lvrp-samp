/*
	afrp_loginsound.inc - Musique d'ambiance pendant l'ecran de login

	Joue une vraie piste MP3 en streaming (PlayAudioStreamForPlayer) pendant
	que le joueur est sur le dialogue mot de passe, et l'arrete net des que
	la connexion est validee (gPlayerLogged[playerid] = 1).

	=== HISTORIQUE : POURQUOI CE N'ETAIT PAS DEJA FAIT COMME CA ===
	Une premiere tentative (voir gamemodes/LVRP.pwn, commentaire "[FIX
	CONNEXION] URL audio retiree") appelait PlayAudioStreamForPlayer EN
	DIRECT au moment de OnPlayerConnect, avec une URL dont l'hebergeur est
	tombe hors ligne. Un client SA-MP qui tente de charger un flux MP3 sur
	un serveur injoignable peut rester bloque plusieurs secondes avant
	d'abandonner - en plein milieu de la sequence de connexion, ca donnait
	l'impression que le serveur freeze/plante pour le joueur.

	Deux precautions ici pour eviter de reproduire le meme probleme :
	1) L'appel est retarde de quelques centaines de ms via timer, APRES que
	   le dialogue mot de passe soit deja affiche - donc meme si le flux met
	   du temps a charger ou echoue, le joueur voit deja l'ecran de login et
	   peut deja taper son mot de passe, rien n'est bloque.
	2) LOGINSOUND_URL doit pointer vers un hebergement fiable et STABLE
	   (ton propre espace web LemeHost par exemple, pas un lien perso qui
	   peut disparaitre). Lien direct vers un .mp3, en HTTPS de preference,
	   sans redirection. Teste l'URL dans un navigateur avant de la mettre
	   ici : si elle ne se telecharge/joue pas directement, ne l'utilise pas.

	Hooks (deja en place dans LVRP.pwn, rien a changer de ce cote) :
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

// [A REMPLACER/COMPLETER] Une piste est tiree au hasard parmi celles-ci a
// chaque connexion. Chaque lien doit finir par .mp3 et etre accessible
// publiquement (teste-le dans un navigateur : il doit se lire/telecharger
// direct, sans page intermediaire ni redirection).
stock const loginSoundUrls[][128] = {
	"http://d.top4top.io/m_3634664230.mp3",
	"http://l.top4top.io/m_3622xnpmo0.mp3",
	"http://e.top4top.io/m_3626xyrmv0.mp3",
	"http://i.top4top.io/m_36256dp8j0.mp3",
	"http://j.top4top.io/m_36256nqpz0.mp3"
};
#define LOGINSOUND_TOTAL      5
#define LOGINSOUND_DELAY_MS   400   // delai avant de lancer le flux, voir en-tete

new bool:loginSoundActive[MAX_PLAYERS];

forward LoginSound_Play(playerid);
public LoginSound_Play(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(!loginSoundActive[playerid]) return 0; // le joueur s'est deja connecte/deconnecte entre-temps
	PlayAudioStreamForPlayer(playerid, loginSoundUrls[random(LOGINSOUND_TOTAL)]);
	return 1;
}

stock LoginSound_Start(playerid)
{
	if(loginSoundActive[playerid]) return 0; // deja en cours
	loginSoundActive[playerid] = true;
	SetTimerEx("LoginSound_Play", LOGINSOUND_DELAY_MS, false, "i", playerid);
	return 1;
}

stock LoginSound_Stop(playerid)
{
	if(loginSoundActive[playerid])
		{StopAudioStreamForPlayer(playerid);}
	loginSoundActive[playerid] = false;
	return 1;
}

stock LoginSound_OnDisconnect(playerid)
{
	loginSoundActive[playerid] = false;
	return 1;
}
