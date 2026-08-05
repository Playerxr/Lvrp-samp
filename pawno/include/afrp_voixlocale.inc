/*
	afrp_voixlocale.inc - Voix de proximite (les joueurs proches s'entendent)

	=== [REECRITURE] LE GAMEMODE N'APPELLE PLUS SvXxx DIRECTEMENT ===
	Meme decision que afrp_radiovoice.inc, meme raison (voir son en-tete) :
	la voix de proximite n'est PAS pilotee par le gamemode, elle est geree
	en entier par le filterscript reel (sampvoice.pwn, fourni par la
	proprietaire). Il y cree, a OnPlayerConnect, UN stream local par joueur
	qui le suit automatiquement (SvCreateDLStreamAtPlayer), pour toute la
	duree de la connexion :

		if ((local_stream[playerid] = SvCreateDLStreamAtPlayer(SV_VOICE_LOCAL_RADIUS, ...)))
			SvAddKey(playerid, SV_TALK_KEY);

	Et quand le joueur appuie sur la touche talkie (B), le filterscript
	choisit lui-meme SUR QUEL stream parler (OnPlayerActivationKeyPress,
	sampvoice.pwn) : le local_stream par defaut, sauf s'il est en appel
	telephone ou en mode radio (talkStats). Il n'existe donc PAS de fonction
	distante "JoinLocalVoiceChannel" a appeler : le mode local est l'etat de
	repos, pas une action. Le gamemode n'a plus qu'UNE seule chose a garantir
	: talkStats = 0 (pas radio) et aucun appel telephone actif, exactement
	ce que fait CMD:local dans gm_code_example.pwn (fourni par la
	proprietaire) - juste un SetPVarInt, rien d'autre.

	=== RAYON D'ECOUTE : PAS UN REGLAGE COTE GAMEMODE ===
	Le rayon (20m demande par la proprietaire) est fixe par la constante
	SV_VOICE_LOCAL_RADIUS DANS sampvoice.pwn/sampvoice.inc (le filterscript),
	pas ici. VOIXLOCALE_DISTANCE ci-dessous ne sert plus qu'a l'afficher dans
	le message au joueur : la changer ici NE CHANGE PAS le vrai rayon. Pour
	changer le vrai rayon, il faut editer SV_VOICE_LOCAL_RADIUS cote
	filterscript et recompiler sampvoice.pwn.

	=== EXCLUSIF AVEC LA RADIO ===
	/radio (canal de faction/frequence) et /local (proximite) s'excluent :
	le joueur choisit sur quoi il parle. RadioVoice_Disable() quitte le
	canal de groupe cote filterscript (CallRemoteFunction LeaveGroupVoiceChannel)
	avant de repasser talkStats a 0, pour ne pas rester abonne aux deux.

	Hooks a brancher dans LVRP.pwn (deja en place, signatures inchangees) :
	- OnGameModeInit     -> VoixLocale_Init()
	- OnPlayerConnect    -> VoixLocale_OnConnect(playerid)
	- OnPlayerDisconnect -> VoixLocale_OnDisconnect(playerid)
	- /local             -> VoixLocale_Toggle(playerid)
*/

#if defined AFRP_VOIXLOCALE_INCLUDED
	#endinput
#endif
#define AFRP_VOIXLOCALE_INCLUDED

// Purement informatif (texte du message /local) : voir l'en-tete, le vrai
// rayon vit dans sampvoice.pwn (SV_VOICE_LOCAL_RADIUS), pas ici.
#define VOIXLOCALE_DISTANCE      20.0

new bool:voixActive[MAX_PLAYERS];        // pour le message ("active"/"coupee") seulement

stock VoixLocale_Init()
{
	printf("[VOIX LOCALE] Voix de proximite (%.0f m, geree par sampvoice.pwn) prete.", VOIXLOCALE_DISTANCE);
	return 1;
}

stock VoixLocale_OnConnect(playerid)
{
	voixActive[playerid] = false;
	return 1;
}

stock VoixLocale_OnDisconnect(playerid)
{
	voixActive[playerid] = false;
	return 1;
}

// Appele par /local. Bascule la proximite, et coupe la radio si elle tournait
// (les deux modes s'excluent, voir l'en-tete).
stock VoixLocale_Toggle(playerid)
{
	if(voixActive[playerid])
	{
		voixActive[playerid] = false;
		msg_Client(playerid, COLOR_WHITE, "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite coupee. Vous ne parlez plus a voix haute.");
		return 1;
	}

	// Meme controle que la radio : sans le plugin client ou sans micro, on le
	// dit clairement au lieu d'echouer sans rien afficher.
	if(!RadioVoice_IsCapable(playerid))
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb VOIP \xab{FFFFFF} Plugin SampVoice ou micro non detecte : la voix de proximite est indisponible.");
		return 0;
	}

	RadioVoice_Disable(playerid);       // quitte le canal radio/frequence de faction, si actif
	SetPVarInt(playerid, "talkStats", 0);
	voixActive[playerid] = true;

	new s[144];
	format(s, sizeof(s), "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite activee : les joueurs a moins de %.0f m vous entendent.", VOIXLOCALE_DISTANCE);
	msg_Client(playerid, COLOR_WHITE, s);
	return 1;
}

// Appele quand le joueur passe en radio : la proximite doit s'arreter cote
// affichage/etat gamemode (le filterscript, lui, bascule tout seul des que
// talkStats == mode radio).
stock VoixLocale_StopPourRadio(playerid)
{
	if(!voixActive[playerid]) return 0;
	voixActive[playerid] = false;
	msg_Client(playerid, COLOR_WHITE, "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite coupee (vous passez en radio).");
	return 1;
}
