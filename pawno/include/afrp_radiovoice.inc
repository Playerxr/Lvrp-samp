/*
	afrp_radiovoice.inc - Vrai canal vocal (SampVoice) derriere /radio et /local

	=== POURQUOI CE MODULE EXISTE ===
	Plainte recurrente du proprietaire : "la radio ne marche pas". /radio (sans
	texte) et /local se contentaient d'ecrire dans le PVar "talkStats" (3 ou 0).
	Ce PVar n'etait relu qu'UNE seule fois dans tout le gamemode (OnPlayerVoiceStart,
	juste pour choisir une anim de telephone) puis marque #pragma unused : rien ne
	changeait qui entend la voix de qui. C'etait purement decoratif - la seule radio
	qui marchait vraiment etait /radio <message>, un message texte via msg_Radio().

	Le plugin SampVoice (plugins/sampvoice.so, natives dans sampvoice.inc) est
	installe mais jamais utilise dans le gamemode. Ce module cree un vrai canal
	vocal global (GSTREAM) par faction legale et y attache/detache les joueurs
	quand ils font /radio ou /local, EN PLUS de la radio texte existante (qui
	n'est pas touchee).

	=== POURQUOI UN GSTREAM PAR FACTION (PAS UN SEUL CANAL GLOBAL) ===
	Meme regroupement que msg_Radio() : chaque faction (PlayerInfo[playerid][pMember])
	a sa propre frequence texte, donc sa propre frequence vocale. Un flic en /radio
	ne doit pas entendre un pompier en /radio.

	=== LISTENER vs SPEAKER SUR UN GSTREAM ===
	sampvoice.inc ne declare QUE des natives "Listener" pour SV_GSTREAM (section
	"Streams Global") et QUE des natives "Speaker" pour SV_LSTREAM (section
	"Streams Local") - mais les tags SV_LSTREAM:/SV_GSTREAM: sont definis vides
	(#define SV_GSTREAM: -> rien) donc n'importe quel native s'applique a n'importe
	quel handle de stream : ce n'est qu'un classement documentaire, pas une
	restriction du compilateur. gamemodes/sampvoice.pwn (FS de reference, PAS
	compile/charge - lu uniquement pour l'idiome) le confirme : JoinGroupVoiceChannel
	attache un GSTREAM radio comme Listener (pour qu'il entende le canal), et
	SvAttachSpeakerToStream est appele sur ce meme GSTREAM pour qu'il y emette.
	Les deux sont necessaires et independants. Ici, comme /radio n'est pas un
	push-to-talk mais un mode permanent (jusqu'a /local), on attache les deux
	tout de suite plutot que de brancher les callbacks de touche d'activation.

	=== CAPACITE CLIENT (JOUEURS SANS LE PLUGIN) ===
	SvGetVersion/SvHasMicro (verifies a la connexion, meme idiome que le FS de
	reference) : si le joueur n'a pas le plugin SampVoice ou pas de micro, /radio
	continue de fonctionner en mode texte (talkStats + le widget existant), mais
	RadioVoice_Enable() renvoie un message clair au lieu d'echouer en silence, et
	n'attache jamais un joueur incapable a un canal.

	=== POURQUOI L'ICONE DU WIDGET NE S'ALLUMAIT JAMAIS ===
	RadioWidget (afrp_radiowidget.inc) n'ecoute que OnPlayerVoiceStart/Stop.
	Mais ces deux publics ne sont JAMAIS appeles par le plugin SampVoice
	lui-meme : sampvoice.inc precise que ce sont des callbacks "emis par le FS
	sampvoice.pwn" (voir ce fichier, present dans gamemodes/ pour reference
	seulement, jamais compile/charge sur le serveur). Sans ce FS, personne
	n'appelle jamais OnPlayerVoiceStart/Stop : l'icone ne s'affiche donc
	jamais, meme quand la voix passe reellement dans le canal. Ici, pas de FS
	et pas de touche d'activation a brancher (le mode radio est permanent, cf.
	section precedente) : un timer interroge simplement SvIsPlayerSpeaking()
	pour chaque joueur attache a un canal, et appelle OnPlayerVoiceStart/Stop
	seulement quand son etat change. Ca reste les memes deux publics que le
	FS de reference aurait appeles - juste declenches autrement.

	=== CHANGEMENT DE FACTION PENDANT UN /radio ACTIF ===
	PlayerInfo[playerid][pMember] est reassigne a une quarantaine d'endroits
	differents dans LVRP.pwn (recrutement, licenciement, arrestation, admin...).
	Plutot que d'accrocher un appel de nettoyage a chacun (fragile, facile a
	oublier a la prochaine faction ajoutee), un timer de reconciliation toutes
	les 8 secondes verifie que le canal auquel un joueur est attache correspond
	toujours a sa faction actuelle, et le detache sinon (+ remet talkStats a 0
	pour que l'icone du widget ne reste pas allumee pour rien).

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
// Poll SvIsPlayerSpeaking() pour piloter le widget (cf. section plus haut).
#define RADIOVOICE_POLL_MS       250
#define RADIOVOICE_STREAM_COLOR  0x00FF80FF

new SV_GSTREAM:radioVoiceStream[RADIOVOICE_MAX_FACTION] = {SV_NULL, ...};
new bool:radioVoiceCapable[MAX_PLAYERS];   // plugin SampVoice + micro detectes a la connexion
new radioVoiceAttachedTo[MAX_PLAYERS];     // pMember du canal actuellement attache, 0 = aucun
new bool:radioVoiceSpeaking[MAX_PLAYERS];  // dernier etat connu de SvIsPlayerSpeaking()

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
	new name[32];
	for(new pm = 1; pm < RADIOVOICE_MAX_FACTION; pm++)
	{
		if(!RadioVoice_IsLegalFaction(pm)) continue;
		format(name, sizeof(name), "Radio-Faction-%d", pm);
		radioVoiceStream[pm] = SvCreateGStream(RADIOVOICE_STREAM_COLOR, name);
	}
	SetTimer("RadioVoice_ReconcileTick", RADIOVOICE_RECONCILE_MS, 1);
	SetTimer("RadioVoice_PollTick", RADIOVOICE_POLL_MS, 1);
	return 1;
}

stock RadioVoice_OnConnect(playerid)
{
	radioVoiceAttachedTo[playerid] = 0;
	radioVoiceSpeaking[playerid] = false;
	// Meme idiome que gamemodes/sampvoice.pwn::OnPlayerConnect (FS de reference) :
	// verification directe, sans delai - le plugin repond des le handshake client.
	radioVoiceCapable[playerid] = bool:(SvGetVersion(playerid) != SV_NULL && SvHasMicro(playerid) == SV_TRUE);
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

	// Icone eteinte tout de suite : sinon elle resterait allumee jusqu'au
	// prochain SvIsPlayerSpeaking()==FALSE, ce qui peut ne jamais arriver si
	// le joueur relache son micro pile au moment ou il quitte le canal.
	if(radioVoiceSpeaking[playerid])
	{
		radioVoiceSpeaking[playerid] = false;
		OnPlayerVoiceStop(playerid);
	}

	if(pm <= 0 || pm >= RADIOVOICE_MAX_FACTION) return 0;

	new SV_GSTREAM:chan = radioVoiceStream[pm];
	if(chan == SV_NULL) return 0;

	if(SvHasSpeakerInStream(chan, playerid)) SvDetachSpeakerFromStream(chan, playerid);
	if(SvHasListenerInStream(chan, playerid)) SvDetachListenerFromStream(chan, playerid);
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

	new SV_GSTREAM:chan = radioVoiceStream[pm];
	if(chan == SV_NULL) return 0; // ne devrait pas arriver (faction legale sans canal cree)

	if(!SvHasListenerInStream(chan, playerid)) SvAttachListenerToStream(chan, playerid);
	if(!SvHasSpeakerInStream(chan, playerid)) SvAttachSpeakerToStream(chan, playerid);
	radioVoiceAttachedTo[playerid] = pm;
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

// [WIDGET] Voir la section "POURQUOI L'ICONE DU WIDGET NE S'ALLUMAIT JAMAIS"
// en tete de fichier : personne n'appelle jamais OnPlayerVoiceStart/Stop tout
// seul, donc on le fait ici a chaque fois que l'etat de parole change reellement.
forward RadioVoice_PollTick();
public RadioVoice_PollTick()
{
	for(new rvI = 0; rvI < MAX_PLAYERS_CURRENT + 1; rvI++)
	{
		if(!IsPlayerConnected(rvI)) continue;
		if(radioVoiceAttachedTo[rvI] == 0) continue;

		new bool:speaking = bool:(SvIsPlayerSpeaking(rvI) == SV_TRUE);
		if(speaking == radioVoiceSpeaking[rvI]) continue;

		radioVoiceSpeaking[rvI] = speaking;
		if(speaking) OnPlayerVoiceStart(rvI);
		else         OnPlayerVoiceStop(rvI);
	}
	return 1;
}
