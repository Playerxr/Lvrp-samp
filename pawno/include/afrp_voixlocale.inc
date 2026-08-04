/*
	afrp_voixlocale.inc - Voix de proximite (les joueurs proches s'entendent)

	=== POURQUOI CE MODULE EXISTE ===
	Un ancien filterscript (sampvoice.amx, dont le code source n'est nulle part
	dans ce depot) annoncait au demarrage "local 20m". En realite il appelait
	SvCreateGStream / SvAttachSpeakerToStream / SvAddKey..., des fonctions qui
	n'existent PAS dans le plugin installe (verifie sur le binaire) : chacun de
	ses appels echouait en silence. La voix de proximite n'a donc jamais
	fonctionne, elle a seulement rempli les logs d'erreurs.
	Ce module la refait, avec l'API reellement exportee par le plugin.

	=== COMMENT CA MARCHE (modele reel du plugin, voir sampvoice.inc) ===
	Un stream cree avec une distance > 0 est LOCAL : il a une position dans le
	monde et seuls les joueurs a portee l'entendent, plus ou moins fort selon
	l'eloignement. Point important tire de la doc officielle : on n'abonne
	PERSONNE a un stream local (SvAttachListener est reserve aux streams
	globaux). Tout joueur qui a le role Listener actif et qui est dans le rayon
	entend automatiquement. Il n'y a donc aucune liste de voisins a maintenir.

	Chaque joueur a donc SON stream local, qui le suit, et sur lequel son micro
	diffuse quand il est en mode local.

	=== POURQUOI UN TIMER DE POSITION PLUTOT QUE SvSetTarget ===
	Le plugin sait accrocher un stream a un joueur (SvSetTarget) pour qu'il
	suive tout seul. Mais la cible doit etre fabriquee par la macro
	SvMakePlayer, et la doc livree avec CE binaire donne sa signature sans son
	contenu. Inventer cette valeur ferait echouer la liaison silencieusement -
	precisement le piege qui a fait que toute la voix de ce serveur est restee
	muette pendant des mois. On rafraichit donc la position nous-memes avec
	SvSetPosition, dont la signature est certaine. C'est un peu moins elegant,
	mais c'est verifiable.
	Le cout est volontairement minimal : on ne met a jour QUE les joueurs
	reellement en train de diffuser en local, et seulement s'ils ont bouge.

	=== EXCLUSIF AVEC LA RADIO ===
	/radio (canal de faction) et /local (proximite) s'excluent : le joueur
	choisit sur quoi il parle, comme avant. C'est le modele mental deja en
	place sur ce serveur, et ca evite de diffuser deux flux par joueur.
	La radio occupe le canal 1 (afrp_radiovoice.inc), la proximite le canal 0
	- qui avait justement ete reserve pour ca.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> VoixLocale_Init()
	- OnPlayerConnect    -> VoixLocale_OnConnect(playerid)
	- OnPlayerDisconnect -> VoixLocale_OnDisconnect(playerid)
	- /local             -> VoixLocale_Toggle(playerid)
*/

#if defined AFRP_VOIXLOCALE_INCLUDED
	#endinput
#endif
#define AFRP_VOIXLOCALE_INCLUDED

// Rayon d'ecoute, en metres du jeu. Valeur demandee par la proprietaire.
// C'est le seul chiffre a changer pour rendre la zone plus large ou plus
// etroite ; il est applique a la creation du stream de chaque joueur.
#define VOIXLOCALE_DISTANCE      20.0

// Canal micro dedie a la proximite. Le 1 est pris par la radio de faction
// (RADIOVOICE_CHANNEL dans afrp_radiovoice.inc) : ne pas les confondre, un
// joueur peut avoir les deux attaches sans s'en rendre compte.
#define VOIXLOCALE_CHANNEL       0

// Rafraichissement de la position des streams. 700 ms : assez fin pour que la
// voix suive quelqu'un qui marche ou roule doucement, assez lache pour ne rien
// couter quand personne ne parle (on ne traite que les emetteurs actifs).
#define VOIXLOCALE_TICK_MS       700

// En dessous de cette distance, on ne renvoie pas la position au plugin : ca
// evite un appel natif par joueur immobile a chaque tick.
#define VOIXLOCALE_SEUIL_BOUGE   0.8

new voixStream[MAX_PLAYERS];             // stream local du joueur, SV_NULL si aucun
new bool:voixActive[MAX_PLAYERS];        // le joueur diffuse-t-il en proximite ?
new Float:voixDernierX[MAX_PLAYERS];
new Float:voixDernierY[MAX_PLAYERS];
new Float:voixDernierZ[MAX_PLAYERS];

stock VoixLocale_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		voixStream[i] = SV_NULL;
		voixActive[i] = false;
	}
	SetTimer("VoixLocale_Tick", VOIXLOCALE_TICK_MS, true);
	printf("[VOIX LOCALE] Voix de proximite active (%.0f m) sur le canal %d.", VOIXLOCALE_DISTANCE, VOIXLOCALE_CHANNEL);
	return 1;
}

stock VoixLocale_OnConnect(playerid)
{
	voixStream[playerid] = SV_NULL;
	voixActive[playerid] = false;
	voixDernierX[playerid] = 0.0;
	voixDernierY[playerid] = 0.0;
	voixDernierZ[playerid] = 0.0;
	return 1;
}

stock VoixLocale_OnDisconnect(playerid)
{
	VoixLocale_Couper(playerid);
	// Le stream appartient au joueur : sans destruction ici, chaque connexion
	// en creerait un de plus et le plugin finirait par les refuser.
	if(voixStream[playerid] != SV_NULL)
	{
		SvDeleteStream(voixStream[playerid]);
		voixStream[playerid] = SV_NULL;
	}
	return 1;
}

// Coupe la diffusion en proximite sans detruire le stream (le joueur peut
// reprendre juste apres).
stock VoixLocale_Couper(playerid)
{
	if(!voixActive[playerid]) return 0;
	voixActive[playerid] = false;
	SvStop(playerid, VOIXLOCALE_CHANNEL);
	if(voixStream[playerid] != SV_NULL)
		SvDetachStream(playerid, voixStream[playerid], VOIXLOCALE_CHANNEL);
	return 1;
}

// Retourne 0 (avec un message deja envoye) si la voix n'est pas possible.
stock VoixLocale_Activer(playerid)
{
	if(voixActive[playerid]) return 1;

	// Meme controle que la radio : sans le plugin client ou sans micro, on le
	// dit clairement au lieu d'echouer sans rien afficher.
	if(SvGetVersion(playerid) == SV_NULL || SvHasMicro(playerid) != SV_TRUE)
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb VOIP \xab{FFFFFF} Plugin SampVoice ou micro non detecte : la voix de proximite est indisponible.");
		return 0;
	}

	// Un stream par joueur, cree a la premiere utilisation seulement : inutile
	// d'en allouer un pour les joueurs qui ne parlent jamais en local.
	if(voixStream[playerid] == SV_NULL)
	{
		voixStream[playerid] = SvCreateStream(VOIXLOCALE_DISTANCE);
		if(voixStream[playerid] == SV_NULL || voixStream[playerid] == SV_NONE)
		{
			voixStream[playerid] = SV_NULL;
			msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb VOIP \xab{FFFFFF} Impossible de creer le canal de proximite.");
			return 0;
		}
	}

	// Position initiale tout de suite : sinon le stream reste a 0,0,0 jusqu'au
	// premier tick et les voisins n'entendraient rien pendant ce court instant.
	new Float:vx, Float:vy, Float:vz;
	GetPlayerPos(playerid, vx, vy, vz);
	SvSetPosition(voixStream[playerid], vx, vy, vz);
	voixDernierX[playerid] = vx;
	voixDernierY[playerid] = vy;
	voixDernierZ[playerid] = vz;

	SvAttachStream(playerid, voixStream[playerid], VOIXLOCALE_CHANNEL);
	SvPlay(playerid, VOIXLOCALE_CHANNEL);
	voixActive[playerid] = true;
	return 1;
}

// Appele par /local. Bascule la proximite, et coupe la radio si elle tournait
// (les deux modes s'excluent, voir l'en-tete).
stock VoixLocale_Toggle(playerid)
{
	if(voixActive[playerid])
	{
		VoixLocale_Couper(playerid);
		msg_Client(playerid, COLOR_WHITE, "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite coupee. Vous ne parlez plus a voix haute.");
		return 1;
	}

	RadioVoice_Disable(playerid);       // quitte le canal radio de faction
	SetPVarInt(playerid, "talkStats", 0);

	if(!VoixLocale_Activer(playerid)) return 0;

	new s[144];
	format(s, sizeof(s), "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite activee : les joueurs a moins de %.0f m vous entendent.", VOIXLOCALE_DISTANCE);
	msg_Client(playerid, COLOR_WHITE, s);
	return 1;
}

// Appele quand le joueur passe en radio : la proximite doit s'arreter, sinon
// il diffuserait sur les deux canaux en meme temps sans le savoir.
stock VoixLocale_StopPourRadio(playerid)
{
	if(!voixActive[playerid]) return 0;
	VoixLocale_Couper(playerid);
	msg_Client(playerid, COLOR_WHITE, "{8B8B00}\xbb VOIP \xab{FFFFFF} Voix de proximite coupee (vous passez en radio).");
	return 1;
}

// Deplace les streams pour qu'ils suivent leur proprietaire. On ne traite que
// les joueurs qui diffusent reellement : un serveur plein ou personne ne parle
// en local ne coute rien du tout.
forward VoixLocale_Tick();
public VoixLocale_Tick()
{
	new Float:vx, Float:vy, Float:vz;
	for(new vi = 0; vi < MAX_PLAYERS_CURRENT + 1; vi++)
	{
		if(!voixActive[vi]) continue;
		if(!IsPlayerConnected(vi)) { VoixLocale_Couper(vi); continue; }
		if(voixStream[vi] == SV_NULL) continue;

		GetPlayerPos(vi, vx, vy, vz);

		// Immobile : rien a renvoyer au plugin.
		if(floatabs(vx - voixDernierX[vi]) < VOIXLOCALE_SEUIL_BOUGE &&
		   floatabs(vy - voixDernierY[vi]) < VOIXLOCALE_SEUIL_BOUGE &&
		   floatabs(vz - voixDernierZ[vi]) < VOIXLOCALE_SEUIL_BOUGE)
			continue;

		SvSetPosition(voixStream[vi], vx, vy, vz);
		voixDernierX[vi] = vx;
		voixDernierY[vi] = vy;
		voixDernierZ[vi] = vz;
	}
	return 1;
}
