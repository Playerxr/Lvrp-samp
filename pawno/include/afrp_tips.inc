/*
	afrp_tips.inc - Astuces et conseils RP qui pop automatiquement

	Timer toutes les TIPS_INTERVAL (5 min par defaut) : pick une astuce au
	hasard dans le pool et broadcast a tous les joueurs connectes.

	Format : "{FFAA00}[Astuce]{FFFFFF} ..."

	On evite la repetition immediate : on retient l'index du dernier tip
	envoye et on en pick un autre.

	Commande /astuce : declenche manuellement un tip random (pour les
	joueurs qui veulent un conseil instantane).
*/

#if defined AFRP_TIPS_INCLUDED
	#endinput
#endif
#define AFRP_TIPS_INCLUDED

#define TIPS_INTERVAL  300       // secondes entre 2 astuces auto (5 min)

new lastTipIndex = -1;

// Pool d'astuces — varie le ton (motivant, info, drole)
static const tipMessages[][] = {
	"Pas d'argent ? Tape {00FF00}/jobs{FFFFFF} et choisis un metier. Tu commences avec 25 000$ !",
	"Tu veux un telephone ? Tape {00FF00}/shop{FFFFFF} a un marche (icone briefcase sur la map).",
	"Pour ouvrir ton tel : {00FF00}/mobile{FFFFFF}. SMS, appels, banque, twitter dispos.",
	"Joue ton role : {00FF00}/me{FFFFFF} <action> pour decrire ce que ton perso fait.",
	"Tu peux decrire ton environnement avec {00FF00}/do{FFFFFF} <description>.",
	"Besoin d'aide ? Tape {00FF00}/tuto{FFFFFF} pour revoir le tutoriel complet.",
	"Tu veux changer ta posture ou animer ton perso ? Tape {00FF00}/emotes{FFFFFF}.",
	"Un event est en cours ? Tape {00FF00}/event{FFFFFF} pour avoir les coords + un checkpoint.",
	"Tu te fais menacer ? N'abandonne pas, joue le RP et utilise {00FF00}/report{FFFFFF} si abus.",
	"VIP = bonus de paie et acces special. Pense au site pour soutenir le serveur.",
	"En faction ? Tape {00FF00}/factions{FFFFFF} pour voir la liste et qui recrute.",
	"Tu peux te deplacer en {00FF00}/taxi{FFFFFF} si tu connais un chauffeur en service.",
	"Mort en RP ? Souviens-toi de la regle {FFFF00}PK{FFFFFF} : ton perso oublie son tueur.",
	"Le {00FF00}/me{FFFFFF} et le {00FF00}/do{FFFFFF} comptent dans ton classement de bon RPer.",
	"Tu peux verrouiller ta voiture avec {00FF00}Y{FFFFFF} pour eviter les vols.",
	"Tape {FFFF00}T{FFFFFF} pour parler IC (in-character), {FFFF00}/b{FFFFFF} pour parler OOC.",
	"Tu cherches un job rapide ? Pizza ou Eboueur sont les plus simples pour commencer.",
	"Le serveur fait des events random toutes les 30-60 min. Reste connecte !",
	"Tape {00FF00}/aide{FFFFFF} ou {00FF00}/regles{FFFFFF} pour le reglement complet.",
	"Tu peux acheter une maison via les panneaux {FFFF00}A VENDRE{FFFFFF} en ville.",
	"Au volant : {00FF00}H{FFFFFF} = klaxon, {00FF00}2/8{FFFFFF} = phares, {00FF00}N{FFFFFF} = action.",
	"Tu veux un truc fun ? Tape {00FF00}/emotes{FFFFFF} -> Militaire -> Garde a vous !"
};

forward Tips_Tick();
public Tips_Tick()
{
	// Trouve un tip different du precedent
	new total = sizeof(tipMessages);
	if(total <= 0) return 0;
	new tries = 0;
	new idx;
	do
	{
		idx = random(total);
		tries++;
	}
	while(idx == lastTipIndex && tries < 5);
	lastTipIndex = idx;

	// Broadcast a tous les joueurs connectes loggue
	new msg[200];
	format(msg, sizeof(msg), "{FFAA00}[Astuce]{FFFFFF} %s", tipMessages[idx]);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SendClientMessage(i, -1, msg);
	}
	return 1;
}

forward Tips_StartRepeating();
public Tips_StartRepeating()
{
	// Lance le tip + le timer repetitif des le 1er fire
	Tips_Tick();
	SetTimer("Tips_Tick", TIPS_INTERVAL * 1000, true);
	return 1;
}

stock Tips_Init()
{
	// Premier tip dans 2 min, ensuite toutes les TIPS_INTERVAL sec
	SetTimer("Tips_StartRepeating", 120 * 1000, false);
	return 1;
}

// /astuce : pick un tip random pour l'appelant uniquement
stock Tips_CmdAstuce(playerid)
{
	new idx = random(sizeof(tipMessages));
	new msg[200];
	format(msg, sizeof(msg), "{FFAA00}[Astuce]{FFFFFF} %s", tipMessages[idx]);
	SendClientMessage(playerid, -1, msg);
	return 1;
}
