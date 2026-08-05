/*
	afrp_radiovoice.inc - Vrai canal vocal (SampVoice) derriere /radio et /local

	=== POURQUOI CE MODULE EXISTE ===
	Plainte recurrente du proprietaire : "la radio ne marche pas". /radio (sans
	texte) et /local se contentaient d'ecrire dans le PVar "talkStats" (3 ou 0).
	Ce PVar n'etait relu qu'UNE seule fois dans tout le gamemode (OnPlayerVoiceStart,
	juste pour choisir une anim de telephone) puis marque #pragma unused : rien ne
	changeait qui entend la voix de qui. C'etait purement decoratif - la seule radio
	qui marchait vraiment etait /radio <message>, un message texte via msg_Radio().

	=== [REECRITURE] LA PREMIERE VERSION DE CE MODULE NE POUVAIT PAS MARCHER ===
	Elle appelait des fonctions ("SvCreateGStream", "SvAttachSpeakerToStream",
	"SvIsPlayerSpeaking", ...) qui n'existent pas dans le vrai plugin installe
	(plugins/sampvoice.so) : voir sampvoice.inc, reecrit en meme temps que ce
	fichier a partir de la vraie doc (sv_control/sampvoice.chm, extraite avec
	extract_chmLib). Chaque appel echouait silencieusement (native introuvable),
	donc aucun son ne passait, quel que soit le code autour. Cette version
	utilise les vraies fonctions du plugin.

	=== MODELE REEL (channels/streams, pas "GStream/LStream") ===
	Chaque joueur a 32 canaux et un role Listener + un role Speaker. Un STREAM
	global (SvCreateStream(0.0)) recoit l'audio des canaux Speaker qui lui sont
	attaches (SvAttachStream) et le diffuse a ses Listeners (SvAttachListener).
	SvPlay(playerid, channel) demarre reellement le micro sur ce canal (c'est
	l'action, pas juste une autorisation) ; SvStop l'arrete. Comme /radio est un
	mode permanent (pas du push-to-talk), on appelle SvPlay/SvStop directement
	depuis /radio et /local plutot que de brancher une touche d'activation.

	=== POURQUOI UN STREAM PAR FACTION (PAS UN SEUL CANAL GLOBAL) ===
	Meme regroupement que msg_Radio() : chaque faction (PlayerInfo[playerid][pMember])
	a sa propre frequence texte, donc sa propre frequence vocale. Un flic en /radio
	ne doit pas entendre un pompier en /radio.

	=== CAPACITE CLIENT (JOUEURS SANS LE PLUGIN) ===
	SvGetVersion/SvHasMicro verifies a la connexion : si le joueur n'a pas le
	plugin SampVoice ou pas de micro, /radio continue de fonctionner en mode
	texte (talkStats + le widget existant), mais RadioVoice_Enable() renvoie un
	message clair au lieu d'echouer en silence, et n'attache jamais un joueur
	incapable a un canal.

	=== ICONE DU WIDGET ===
	Comme SvPlay/SvStop demarrent/arretent le micro de facon deterministe (pas
	de touche a surveiller, pas de polling necessaire), on appelle directement
	OnPlayerVoiceStart/Stop (afrp_talkie.inc, qui a remplace afrp_radiowidget.inc)
	au moment ou on appelle
	SvPlay/SvStop : l'icone s'allume des l'entree en mode radio et s'eteint des
	la sortie, sans dependre d'un evenement du plugin qui n'existe pas dans
	cette version (pas de callback "le joueur parle" cote serveur).

	=== CHANGEMENT DE FACTION PENDANT UN /radio ACTIF ===
	PlayerInfo[playerid][pMember] est reassigne a une quarantaine d'endroits
	differents dans LVRP.pwn (recrutement, licenciement, arrestation, admin...).
	Plutot que d'accrocher un appel de nettoyage a chacun, un timer de
	reconciliation toutes les 8 secondes verifie que le canal auquel un joueur
	est attache correspond toujours a sa faction actuelle, et le detache sinon
	(+ remet talkStats a 0 pour que l'icone du widget ne reste pas allumee).

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit    -> RadioVoice_Init()
	- OnPlayerConnect   -> RadioVoice_OnConnect(playerid)
	- OnPlayerDisconnect -> RadioVoice_OnDisconnect(playerid)
	- /radio (sans texte) -> RadioVoice_Enable(playerid) avant SetPVarInt("talkStats", 3)
	- /local              -> RadioVoice_Disable(playerid) avant SetPVarInt("talkStats", 0)
*/

#if defined AFRP_RADIOVOICE_INCLUDED
	#endinput
#endif
#define AFRP_RADIOVOICE_INCLUDED

#include <sampvoice>

// pMember va jusqu'a 55 (Marine) dans la regle de faction legale ci-dessous.
#define RADIOVOICE_MAX_FACTION   56
// Reconciliation faction/canal (rattrape un changement de faction sans /local).
#define RADIOVOICE_RECONCILE_MS  8000
// Canal SampVoice utilise pour la radio (sur les 32 disponibles par joueur).
// Reserve : 0 pour un eventuel futur chat de proximite, la radio prend le 1.
#define RADIOVOICE_CHANNEL       1

new radioVoiceStream[RADIOVOICE_MAX_FACTION] = {SV_NULL, ...};
new bool:radioVoiceCapable[MAX_PLAYERS];   // plugin SampVoice + micro detectes a la connexion
new radioVoiceAttachedTo[MAX_PLAYERS];     // pMember du canal actuellement attache, 0 = aucun

// [SOURCE DE VERITE UNIQUE] Meme regle que le "_pm" de /radio (ligne ~55635 de
// LVRP.pwn) : qui a le droit a la radio de faction. Ne pas dupliquer ailleurs -
// si la regle change un jour, /radio et les canaux vocaux doivent rester d'accord.
stock bool:RadioVoice_IsLegalFaction(pm)
{
	if(pm == 0 || pm == 9 || pm == 10 || (pm > 13 && pm != FACTION_HOPITAL && (pm < 53 || pm > 55)))
		return false;
	return true;
}

stock RadioVoice_Init()
{
	for(new pm = 1; pm < RADIOVOICE_MAX_FACTION; pm++)
	{
		if(!RadioVoice_IsLegalFaction(pm)) continue;
		radioVoiceStream[pm] = SvCreateStream(); // 0.0 par defaut = stream global (2D), pas positionnel
	}
	SetTimer("RadioVoice_ReconcileTick", RADIOVOICE_RECONCILE_MS, 1);
	return 1;
}

stock RadioVoice_OnConnect(playerid)
{
	radioVoiceAttachedTo[playerid] = 0;
	radioVoiceCapable[playerid] = bool:(SvGetVersion(playerid) != SV_NULL && SvHasMicro(playerid) == SV_TRUE);
	if(radioVoiceCapable[playerid])
	{
		SvEnableListener(playerid);
		// [FIX VOIX MUETTE] Le plugin separe DEUX roles par joueur : Listener
		// (il entend les streams auxquels il est abonne) et Speaker (son micro
		// peut alimenter un stream). On n'activait que le premier. Resultat :
		// tout le monde pouvait entendre, PERSONNE ne pouvait parler - et
		// comme aucune de ces fonctions ne renvoie d'erreur visible, rien dans
		// les logs ne le disait. C'est la cause de "la voix ne passe pas"
		// malgre un plugin bien charge et des natives toutes presentes.
		// Le canal est active explicitement plutot que "tous" (0), pour ne pas
		// ouvrir des canaux que le gamemode n'utilise pas.
		SvEnableSpeaker(playerid, RADIOVOICE_CHANNEL);
	}
	return 1;
}

stock RadioVoice_OnDisconnect(playerid)
{
	RadioVoice_DetachCurrent(playerid);
	radioVoiceCapable[playerid] = false;
	return 1;
}

// Detache proprement du canal actuellement attache (si il y en a un). Utilise
// par /local, par la deconnexion, et par la reconciliation faction/canal.
stock RadioVoice_DetachCurrent(playerid)
{
	new pm = radioVoiceAttachedTo[playerid];
	radioVoiceAttachedTo[playerid] = 0;
	if(pm <= 0 || pm >= RADIOVOICE_MAX_FACTION) return 0;

	SvStop(playerid, RADIOVOICE_CHANNEL);
	OnPlayerVoiceStop(playerid); // ferme le talkie a l'ecran (afrp_talkie.inc)

	new voiceStream = radioVoiceStream[pm];
	if(voiceStream == SV_NULL) return 0;

	SvDetachStream(playerid, voiceStream, RADIOVOICE_CHANNEL);
	SvDetachListener(voiceStream, playerid);
	return 1;
}

// Appele par /radio (sans texte), avant de basculer talkStats a 3.
// Retourne 0 si aucun canal vocal n'a pu etre rejoint (message deja envoye au
// joueur dans ce cas) - /radio continue quand meme en mode texte via talkStats.
stock RadioVoice_Enable(playerid)
{
	new pm = PlayerInfo[playerid][pMember];
	if(!RadioVoice_IsLegalFaction(pm)) return 0; // deja filtre par /radio, garde-fou seulement

	if(!radioVoiceCapable[playerid])
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb VOIP \xab{FFFFFF} Plugin SampVoice/micro non d\xe9tect\xe9 : la voix radio est indisponible (le texte via /radio <message> fonctionne toujours).");
		return 0;
	}

	if(radioVoiceAttachedTo[playerid] == pm) return 1; // deja sur le bon canal

	RadioVoice_DetachCurrent(playerid); // au cas ou la faction a change depuis le dernier /radio

	new voiceStream = radioVoiceStream[pm];
	if(voiceStream == SV_NULL) return 0; // ne devrait pas arriver (faction legale sans stream cree)

	SvAttachListener(voiceStream, playerid);
	SvAttachStream(playerid, voiceStream, RADIOVOICE_CHANNEL);
	SvPlay(playerid, RADIOVOICE_CHANNEL); // demarre reellement le micro sur ce canal
	radioVoiceAttachedTo[playerid] = pm;
	OnPlayerVoiceStart(playerid); // ouvre le talkie a l'ecran (afrp_talkie.inc)
	return 1;
}

// Appele par /local, avant de basculer talkStats a 0.
stock RadioVoice_Disable(playerid)
{
	return RadioVoice_DetachCurrent(playerid);
}

// [RECONCILIATION] pMember est reassigne a une quarantaine d'endroits dans
// LVRP.pwn (recrutement/licenciement/arrestation/admin...) : plutot que
// d'accrocher un nettoyage a chacun, on verifie periodiquement que le canal
// attache correspond toujours a la faction actuelle du joueur.
forward RadioVoice_ReconcileTick();
public RadioVoice_ReconcileTick()
{
	for(new rvI = 0; rvI < MAX_PLAYERS_CURRENT + 1; rvI++)
	{
		if(!IsPlayerConnected(rvI)) continue;
		if(radioVoiceAttachedTo[rvI] == 0) continue;
		if(radioVoiceAttachedTo[rvI] != PlayerInfo[rvI][pMember])
		{
			RadioVoice_DetachCurrent(rvI);
			// Evite une icone widget allumee pour un canal qu'il a quitte.
			if(GetPVarInt(rvI, "talkStats") == 3) SetPVarInt(rvI, "talkStats", 0);
		}
	}
	return 1;
}
