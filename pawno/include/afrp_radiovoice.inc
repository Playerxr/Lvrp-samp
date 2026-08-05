/*
	afrp_radiovoice.inc - Radio de faction, derriere /radio (sans texte)

	=== [REECRITURE] LE GAMEMODE N'APPELLE PLUS SvXxx DIRECTEMENT ===
	Version precedente : ce module appelait SvCreateStream/SvAttachStream/
	SvAttachListener/SvPlay/SvStop directement, contre l'include v4.0.0
	"officielle" que la proprietaire avait transmise. Sauf que son
	filterscript REEL (sampvoice.pwn, fourni ensuite) n'utilise PAS cette
	API : il utilise SvCreateGStream / SvAttachSpeakerToStream / SvAddKey...,
	une autre version du plugin (le fork aszarth/sampvoice_phone_and_radio).
	Deux includes contradictoires pour un seul plugin reel : le gamemode ne
	peut pas deviner laquelle est vraie sans lire le binaire installe sur
	SON serveur - ce qui avait deja cause une casse une fois.
	La proprietaire a tranche elle-meme : "laisse le filterscript gerer,
	le gamemode appelle juste en dehors". Ce module ne touche donc plus
	AUCUNE fonction Sv*. Il ne fait que :
	  1. Lire le PVar "hasVoiceOnClient" (0/1/2), pose par sampvoice.pwn a
	     OnPlayerConnect - jamais recalcule ici.
	  2. Appeler CallRemoteFunction("JoinGroupVoiceChannel"/"LeaveGroup...")
	     exactement comme gm_code_example.pwn (fourni par la proprietaire)
	     le montre pour /frequency et /call, deja cables ainsi dans LVRP.pwn.
	C'est le filterscript, et lui seul, qui cree les streams et parle au
	plugin. Si un jour la voix ne marche toujours pas, le probleme est dans
	sampvoice.pwn/sampvoice.amx, plus jamais ici.

	=== FREQUENCE UTILISEE POUR LA RADIO DE FACTION ===
	JoinGroupVoiceChannel(playerid, frequency_id) prend le meme identifiant
	que /frequency (walkie-walkie manuel, deja cable dans LVRP.pwn) : c'est
	UN SEUL espace d'identifiants cote filterscript (voice_radiocall_radioid,
	un seul slot par joueur). La radio de faction utilise pMember comme id
	(1 a 55) pour que tous les membres d'une meme faction tombent sur le
	meme canal, sans dupliquer une table de correspondance.
	[A VERIFIER PAR LA PROPRIETAIRE] son fork declare
	`new SV_GSTREAM:radio_stream[SV_VOICE_MAX_RADIO]` : si SV_VOICE_MAX_RADIO
	(constante definie dans SON sampvoice.inc reel, pas celui de ce depot)
	est inferieur a 56, un pMember eleve (ex. 55, Marine) depasserait le
	tableau cote filterscript. A verifier une fois, cote fichiers reels du
	serveur - je ne peux pas le lire depuis ce depot.

	=== EXCLUSION AVEC /frequency (walkie manuel) ===
	Les deux passent par le meme JoinGroupVoiceChannel : rejoindre l'un doit
	quitter l'autre, sinon le second ecrase le premier cote filterscript
	sans que le gamemode s'en rende compte. isUsingRadioVoip[] (LVRP.pwn,
	deja utilise par /frequency et /call) sert desormais aussi de garde ici.

	Hooks a brancher dans LVRP.pwn (deja en place, signatures inchangees) :
	- OnGameModeInit     -> RadioVoice_Init()
	- OnPlayerConnect    -> RadioVoice_OnConnect(playerid)
	- OnPlayerDisconnect -> RadioVoice_OnDisconnect(playerid)
	- /radio (sans texte) -> RadioVoice_Enable(playerid) avant SetPVarInt("talkStats", 3)
	- /local              -> RadioVoice_Disable(playerid) avant SetPVarInt("talkStats", 0)
*/

#if defined AFRP_RADIOVOICE_INCLUDED
	#endinput
#endif
#define AFRP_RADIOVOICE_INCLUDED

// Reconciliation faction/canal (rattrape un changement de faction sans /local).
#define RADIOVOICE_RECONCILE_MS  8000

new radioVoiceAttachedTo[MAX_PLAYERS];     // pMember du canal actuellement rejoint, 0 = aucun

// [SOURCE DE VERITE UNIQUE] Meme regle que le "_pm" de /radio (ligne ~55970 de
// LVRP.pwn) : qui a le droit a la radio de faction. Ne pas dupliquer ailleurs -
// si la regle change un jour, /radio et les canaux vocaux doivent rester d'accord.
stock bool:RadioVoice_IsLegalFaction(pm)
{
	if(pm == 0 || pm == 9 || pm == 10 || (pm > 13 && pm != FACTION_HOPITAL && (pm < 53 || pm > 55)))
		return false;
	return true;
}

// [SOURCE DE VERITE UNIQUE] "le joueur a-t-il vraiment le plugin + un micro
// utilisable ?" - jamais recalcule ici, seulement lu. C'est sampvoice.pwn qui
// pose ce PVar a OnPlayerConnect (1 = plugin+micro, 2 = plugin sans micro,
// 0 = pas de plugin). Utilise aussi par afrp_talkie.inc pour l'ecran du talkie.
stock bool:RadioVoice_IsCapable(playerid)
{
	return bool:(GetPVarInt(playerid, "hasVoiceOnClient") == 1);
}

stock RadioVoice_Init()
{
	SetTimer("RadioVoice_ReconcileTick", RADIOVOICE_RECONCILE_MS, 1);
	printf("[RADIO VOICE] Radio de faction prete (via CallRemoteFunction, filterscript sampvoice.pwn requis).");
	return 1;
}

stock RadioVoice_OnConnect(playerid)
{
	radioVoiceAttachedTo[playerid] = 0;
	return 1;
}

stock RadioVoice_OnDisconnect(playerid)
{
	RadioVoice_DetachCurrent(playerid);
	return 1;
}

// [SORTIE UNIQUE] Quitte n'importe quel canal de groupe en cours - radio de
// faction (/radio) OU frequence manuelle (/frequency) : les deux partagent le
// MEME canal cote filterscript (un seul slot par joueur, voice_radiocall_radioid
// dans sampvoice.pwn), donc une seule fonction de sortie evite que les deux
// bookkeeping (radioVoiceAttachedTo ici, isUsingRadioVoip dans LVRP.pwn) se
// desynchronisent. Utilisee par /frequency 0, /local, la deconnexion, la
// reconciliation faction/canal, et le sous-menu talkie du menu Y.
stock RadioVoice_LeaveAny(playerid)
{
	if(!isUsingRadioVoip[playerid] && radioVoiceAttachedTo[playerid] == 0) return 0;

	CallRemoteFunction("LeaveGroupVoiceChannel", "i", playerid);
	isUsingRadioVoip[playerid] = false;
	radioVoiceAttachedTo[playerid] = 0;
	if(GetPVarInt(playerid, "talkStats") == 3) SetPVarInt(playerid, "talkStats", 0);
	OnPlayerVoiceStop(playerid); // ferme le talkie a l'ecran (afrp_talkie.inc)
	return 1;
}

// Quitte le canal radio actuellement rejoint (si il y en a un). Utilise par
// /local et par la reconciliation faction/canal.
stock RadioVoice_DetachCurrent(playerid)
{
	return RadioVoice_LeaveAny(playerid);
}

// Appele par /radio (sans texte), avant de basculer talkStats a 3.
// Retourne 0 si aucun canal vocal n'a pu etre rejoint (message deja envoye au
// joueur dans ce cas) - /radio continue quand meme en mode texte via talkStats.
stock RadioVoice_Enable(playerid)
{
	new pm = PlayerInfo[playerid][pMember];
	if(!RadioVoice_IsLegalFaction(pm)) return 0; // deja filtre par /radio, garde-fou seulement

	if(!RadioVoice_IsCapable(playerid))
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb VOIP \xab{FFFFFF} Plugin SampVoice/micro non d\xe9tect\xe9 : la voix radio est indisponible (le texte via /radio <message> fonctionne toujours).");
		return 0;
	}

	if(radioVoiceAttachedTo[playerid] == pm) return 1; // deja sur le bon canal

	// Quitte proprement tout canal en cours - faction precedente (changement
	// depuis le dernier /radio) OU /frequency manuel : meme canal cote
	// filterscript, un seul actif a la fois (voir RadioVoice_LeaveAny).
	RadioVoice_LeaveAny(playerid);

	CallRemoteFunction("JoinGroupVoiceChannel", "ii", playerid, pm);
	isUsingRadioVoip[playerid] = true;
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
// rejoint correspond toujours a la faction actuelle du joueur.
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
