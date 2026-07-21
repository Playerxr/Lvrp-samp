/*
	afrp_emotes.inc - Systeme d'emotes/animations RP avance

	Commande : /emotes (alias /e)
	NB: "/anim" n'est PAS un alias valide, deja pris par /anim(ateur) (staff).
	Dialog : 9500 (menu categories) + 9501 (sous-liste)

	~40 anims reparties en 6 categories :
	- Vie courante (manger, boire, fumer, telephone, etc.)
	- Posture (s'asseoir, s'allonger, accroupir, etc.)
	- Salutations (bras leve, poignee, salut militaire, etc.)
	- Reactions (rire, pleurer, applaudir, etc.)
	- Combat (provocation, victoire, defaite, etc.)
	- Stop : annule l'anim en cours
*/

#if defined AFRP_EMOTES_INCLUDED
	#endinput
#endif
#define AFRP_EMOTES_INCLUDED

#define DIALOG_EMOTES_CATS  9500
#define DIALOG_EMOTES_LIST  9501

enum eEmote {
	em_label[32],
	em_lib[24],
	em_anim[32],
	Float:em_speed,
	em_loop,
	em_lockx,
	em_locky,
	em_freeze,
	em_time
};

// CATEGORIE 0 : VIE COURANTE
static const emotes_VieCourante[][eEmote] = {
	{"Manger un sandwich",     "FOOD",     "EAT_Burger",         4.0, 0, 0, 0, 0, 3500},
	{"Boire (canette)",        "VENDING",  "VEND_Drink2_P",      4.0, 0, 0, 0, 0, 3200},
	{"Fumer une cigarette",    "SMOKING",  "M_smk_in",           4.0, 0, 0, 0, 0, 4000},
	{"Tirer sur la cigarette", "SMOKING",  "M_smk_drag",         4.0, 0, 0, 0, 0, 2000},
	{"Telephone (parler)",     "PED",      "phone_in",           4.0, 1, 0, 0, 0, 0},
	{"Telephone (raccrocher)", "PED",      "phone_out",          4.0, 0, 0, 0, 0, 1500},
	{"Verifier l'heure",       "COP_AMBIENT","Coplook_loop",     4.0, 0, 0, 0, 0, 2200},
	{"Crachat",                "PED",      "spitoon_in",         4.0, 0, 0, 0, 0, 2000}
};

// CATEGORIE 1 : POSTURE
static const emotes_Posture[][eEmote] = {
	{"S'asseoir (sol)",        "BEACH",    "ParkSit_M_loop",     4.0, 1, 0, 0, 0, 0},
	{"S'asseoir (chaise)",     "BAR",      "Barcustom_loop",     4.0, 1, 1, 1, 0, 0},
	{"S'allonger (dos)",       "BEACH",    "bather",             4.0, 1, 1, 1, 0, 0},
	{"S'allonger (ventre)",    "BEACH",    "Lay_Bac_in",         4.0, 1, 1, 1, 0, 0},
	{"S'accroupir",            "PED",      "crouch_roll_l",      4.0, 1, 1, 1, 0, 0},
	{"S'appuyer (mur)",        "GANGS",    "leanIDLE",           4.0, 1, 1, 1, 0, 0},
	{"Mains derriere la tete", "PED",      "endchat_03",         4.0, 1, 0, 0, 0, 0},
	{"Bras croises",           "DEALER",   "DEALER_IDLE_01",     4.0, 1, 0, 0, 0, 0}
};

// CATEGORIE 2 : SALUTATIONS
static const emotes_Salutations[][eEmote] = {
	{"Bras leve (salut)",      "PED",      "endchat_01",         4.0, 0, 0, 0, 0, 2000},
	{"Pouce leve",             "PED",      "endchat_02",         4.0, 0, 0, 0, 0, 2000},
	{"Faire signe (taxi)",     "ON_LOOKERS","wave_loop",         4.0, 1, 0, 0, 0, 0},
	{"Hochement de tete",      "PED",      "gang_gunstand",      4.0, 0, 0, 0, 0, 2000},
	{"Salut militaire",        "PED",      "facsurpr",           4.0, 0, 0, 0, 0, 2000},
	{"Hausser les epaules",    "ON_LOOKERS","shrug_in",          4.0, 0, 0, 0, 0, 1500},
	{"Pointer du doigt",       "PED",      "Ariesgun_in",        4.0, 0, 0, 0, 0, 2000}
};

// CATEGORIE 3 : REACTIONS
static const emotes_Reactions[][eEmote] = {
	{"Applaudir",              "RAPPING",  "Laugh_01",           4.0, 1, 0, 0, 0, 0},
	{"Rire",                   "RAPPING",  "Laugh_01",           4.0, 0, 0, 0, 0, 3000},
	{"Pleurer",                "GRAVEYARD","mrnF_loop",          4.0, 0, 1, 1, 0, 3500},
	{"Fascination",            "RAPPING",  "RAP_C_Loop",         4.0, 0, 0, 0, 0, 3000},
	{"Frustration",            "GANGS",    "prtial_gngtlkD",     4.0, 0, 0, 0, 0, 2500},
	{"Choque (recul)",         "PED",      "facsurp",            4.0, 0, 0, 0, 0, 1800},
	{"Vomir",                  "FOOD",     "EAT_Vomit_P",        4.0, 0, 0, 0, 0, 3500}
};

// CATEGORIE 4 : COMBAT/GANG
static const emotes_Combat[][eEmote] = {
	{"Provocation gang 1",     "GANGS",    "hndshkfa_swt",       4.0, 0, 0, 0, 0, 2500},
	{"Provocation gang 2",     "GANGS",    "hndshkda",           4.0, 0, 0, 0, 0, 2500},
	{"Frapper du pied",        "GHANDS",   "gsign1LH",           4.0, 0, 0, 0, 0, 2000},
	{"Croiser doigts",         "GHANDS",   "gsign4",             4.0, 0, 0, 0, 0, 2000},
	{"Cracher au sol",         "FOOD",     "EAT_Vomit_P",        4.0, 0, 0, 0, 0, 2500},
	{"Geste menacant",         "GHANDS",   "gsign2",             4.0, 0, 0, 0, 0, 2000},
	{"Coup de poing (air)",    "PED",      "shake_cara",         4.0, 0, 0, 0, 0, 1500}
};

// CATEGORIE 5 : POLICE RP
static const emotes_Police[][eEmote] = {
	{"Mains en l'air",         "PED",      "handsup",            4.0, 1, 1, 1, 0, 0},
	{"Reddition (a genoux)",   "PED",      "SURRENDER",          4.0, 1, 1, 1, 0, 0},
	{"Menottage (gestes)",     "PED",      "ARRESTgun",          4.0, 0, 0, 0, 0, 3500},
	{"Diriger circulation",    "PED",      "CopTraf2",           4.0, 1, 0, 0, 0, 0},
	{"Surveiller (jumelles)",  "BINOCS",   "BINOCS_IN",          4.0, 1, 1, 1, 0, 0},
	{"Inspection (regarde)",   "COP_AMBIENT","Coplook_loop",     4.0, 1, 0, 0, 0, 0},
	{"Donner ordres",          "ON_LOOKERS","wave_loop",         4.0, 1, 0, 0, 0, 0},
	{"Tap-frisk (fouille)",    "GANGS",    "Invite_Yes",         4.0, 0, 0, 0, 0, 2500},
	{"Pointer suspect",        "PED",      "Ariesgun_in",        4.0, 0, 0, 0, 0, 2000}
};

// CATEGORIE 6 : ARMES RP
static const emotes_Armes[][eEmote] = {
	{"Recharger (pistolet)",   "COLT45",   "colt45_reload",      4.0, 0, 0, 0, 0, 1500},
	{"Recharger (Uzi)",        "UZI",      "UZI_reload",         4.0, 0, 0, 0, 0, 1500},
	{"Recharger (AK/M4)",      "RIFLE",    "RIFLE_load",         4.0, 0, 0, 0, 0, 1500},
	{"Recharger (Sniper)",     "RIFLE",    "RIFLE_load",         4.0, 0, 0, 0, 0, 1500},
	{"Verifier arme",          "PED",      "Idle_Gang1",         4.0, 1, 0, 0, 0, 0},
	{"Holster (ranger)",       "PED",      "PUTAWAY",            4.0, 0, 0, 0, 0, 2000},
	{"Sortir arme (lent)",     "PED",      "Gunaim_idle",        4.0, 0, 0, 0, 0, 1500},
	{"Tir en l'air",           "RIFLE",    "RIFLE_fire",         4.0, 0, 0, 0, 0, 1500},
	{"Lancer (cocktail)",      "GRENADE",  "WEAPON_throw",       4.0, 0, 0, 0, 0, 2200}
};

// CATEGORIE 7 : MILITAIRE / PROTOCOLE
static const emotes_Militaire[][eEmote] = {
	{"Garde a vous",           "PED",      "gang_gunstand",      4.0, 1, 1, 1, 0, 0},
	{"Repos (mains derriere)", "PED",      "endchat_03",         4.0, 1, 1, 1, 0, 0},
	{"Salut militaire",        "PED",      "facsurpr",           4.0, 0, 0, 0, 0, 2000},
	{"Presenter armes",        "RIFLE",    "RIFLE_load",         4.0, 0, 0, 0, 0, 2000},
	{"Marche militaire",       "PED",      "WALK_armed",         4.0, 1, 0, 0, 0, 0},
	{"Faire rapport",          "PED",      "Idle_chat",          4.0, 1, 1, 1, 0, 0},
	{"S'incliner (respect)",   "KISSING",  "gfwd_b_o",           4.0, 0, 1, 1, 0, 2500},
	{"Porter drapeau (lever)", "CARRY",    "liftup",             4.0, 0, 0, 0, 0, 2500},
	{"Decoration (recevoir)",  "PED",      "endchat_02",         4.0, 0, 0, 0, 0, 2500},
	{"Tomber au sol (KO)",     "PED",      "FLOOR_hit",          4.0, 0, 1, 1, 0, 3000},
	{"Mort (faire le mort)",   "PED",      "KO_skid_back",       4.0, 0, 1, 1, 1, 0}
};

// CATEGORIE 8 : VIP EXCLUSIF (reserve pDonateRank >= 1, voir Emotes_ShowList)
static const emotes_Vip[][eEmote] = {
	{"Danse disco",            "DANCING",  "dnce_M_a",           4.0, 1, 1, 1, 0, 0},
	{"Danse robot",            "DANCING",  "dnce_M_b",           4.0, 1, 1, 1, 0, 0},
	{"Danse funky",            "DANCING",  "dnce_M_c",           4.0, 1, 1, 1, 0, 0},
	{"Danse energique",        "DANCING",  "dnce_M_d",           4.0, 1, 1, 1, 0, 0},
	{"Pompes (musculation)",   "GYMNASIUM","gym_pushups",        4.0, 1, 1, 1, 0, 0},
	{"Abdos (sit-ups)",        "GYMNASIUM","gym_situps",         4.0, 1, 1, 1, 0, 0}
};

// Header table avec nombre d'emotes par categorie
static const emoteCategoryCount[9] = {
	8, // 0 VieCourante
	8, // 1 Posture
	7, // 2 Salutations
	7, // 3 Reactions
	7, // 4 Combat
	9, // 5 Police RP
	9, // 6 Armes RP
	11, // 7 Militaire / Protocole
	6  // 8 VIP Exclusif
};

stock Emotes_ShowCategories(playerid)
{
	new list[800];
	format(list, sizeof(list),
		"{FFFFFF}1. Vie courante {8C8C8C}(manger, boire, fumer...)\n{FFFFFF}2. Posture {8C8C8C}(s'asseoir, s'allonger...)\n{FFFFFF}3. Salutations {8C8C8C}(saluer, hocher, pointer...)\n{FFFFFF}4. Reactions {8C8C8C}(rire, pleurer, applaudir...)\n{FFFFFF}5. Combat/Gang {8C8C8C}(provoc, menaces...)\n{2B6AFF}6. Police RP {8C8C8C}(mains en l'air, menottage, fouille...)\n{FF8C00}7. Armes RP {8C8C8C}(recharger, holster, lancer...)\n{5B7A3A}8. Militaire / Protocole {8C8C8C}(garde a vous, salut, drapeau...)\n{FFD700}9. VIP Exclusif {8C8C8C}(danses, sport...)\n{FF6666}10. Arreter l'animation en cours");
	ShowPlayerDialog(playerid, DIALOG_EMOTES_CATS, DIALOG_STYLE_LIST,
		"{FFAA00}>> Emotes RP <<{FFFFFF}", list, "Choisir", "Fermer");
	return 1;
}

stock Emotes_ShowList(playerid, cat)
{
	if(cat < 0 || cat >= 9) return Emotes_ShowCategories(playerid);
	// [VIP] Categorie 8 reservee aux membres VIP (tous paliers)
	if(cat == 8 && PlayerInfo[playerid][pDonateRank] < 1)
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Emotes]{FFFFFF} Categorie reservee aux membres V.I.P.");
		return Emotes_ShowCategories(playerid);
	}
	new list[1024];
	list[0] = 0;
	new count = emoteCategoryCount[cat];
	new tmpLabel[32]; // doit matcher exactement em_label[32] pour l'assignation directe
	for(new i = 0; i < count; i++)
	{
		// [FIX] em_label est une string normale (non packee). strunpack() attendait
		// une string packee (4 chars/cell) et produisait des labels vides/corrompus
		// (1er octet decode = 0x00 -> chaine coupee immediatement). Copie directe.
		switch(cat)
		{
			case 0: tmpLabel = emotes_VieCourante[i][em_label];
			case 1: tmpLabel = emotes_Posture[i][em_label];
			case 2: tmpLabel = emotes_Salutations[i][em_label];
			case 3: tmpLabel = emotes_Reactions[i][em_label];
			case 4: tmpLabel = emotes_Combat[i][em_label];
			case 5: tmpLabel = emotes_Police[i][em_label];
			case 6: tmpLabel = emotes_Armes[i][em_label];
			case 7: tmpLabel = emotes_Militaire[i][em_label];
			case 8: tmpLabel = emotes_Vip[i][em_label];
		}
		strcat(list, tmpLabel, sizeof(list));
		strcat(list, "\n", sizeof(list));
	}
	SetPVarInt(playerid, "emoteCat", cat);
	new title[40];
	format(title, sizeof(title), "{FFAA00}Categorie %d{FFFFFF}", cat+1);
	ShowPlayerDialog(playerid, DIALOG_EMOTES_LIST, DIALOG_STYLE_LIST, title, list, "Jouer", "Retour");
	return 1;
}

stock Emotes_Play(playerid, cat, idx)
{
	new lib[24], anim[32], Float:speed;
	new loop, lockx, locky, freeze, etime;
	switch(cat)
	{
		case 0:
		{
			if(idx >= emoteCategoryCount[0]) return 0;
			lib = emotes_VieCourante[idx][em_lib];
			anim = emotes_VieCourante[idx][em_anim];
			speed = emotes_VieCourante[idx][em_speed];
			loop = emotes_VieCourante[idx][em_loop];
			lockx = emotes_VieCourante[idx][em_lockx];
			locky = emotes_VieCourante[idx][em_locky];
			freeze = emotes_VieCourante[idx][em_freeze];
			etime = emotes_VieCourante[idx][em_time];
		}
		case 1:
		{
			if(idx >= emoteCategoryCount[1]) return 0;
			lib = emotes_Posture[idx][em_lib];
			anim = emotes_Posture[idx][em_anim];
			speed = emotes_Posture[idx][em_speed];
			loop = emotes_Posture[idx][em_loop];
			lockx = emotes_Posture[idx][em_lockx];
			locky = emotes_Posture[idx][em_locky];
			freeze = emotes_Posture[idx][em_freeze];
			etime = emotes_Posture[idx][em_time];
		}
		case 2:
		{
			if(idx >= emoteCategoryCount[2]) return 0;
			lib = emotes_Salutations[idx][em_lib];
			anim = emotes_Salutations[idx][em_anim];
			speed = emotes_Salutations[idx][em_speed];
			loop = emotes_Salutations[idx][em_loop];
			lockx = emotes_Salutations[idx][em_lockx];
			locky = emotes_Salutations[idx][em_locky];
			freeze = emotes_Salutations[idx][em_freeze];
			etime = emotes_Salutations[idx][em_time];
		}
		case 3:
		{
			if(idx >= emoteCategoryCount[3]) return 0;
			lib = emotes_Reactions[idx][em_lib];
			anim = emotes_Reactions[idx][em_anim];
			speed = emotes_Reactions[idx][em_speed];
			loop = emotes_Reactions[idx][em_loop];
			lockx = emotes_Reactions[idx][em_lockx];
			locky = emotes_Reactions[idx][em_locky];
			freeze = emotes_Reactions[idx][em_freeze];
			etime = emotes_Reactions[idx][em_time];
		}
		case 4:
		{
			if(idx >= emoteCategoryCount[4]) return 0;
			lib = emotes_Combat[idx][em_lib];
			anim = emotes_Combat[idx][em_anim];
			speed = emotes_Combat[idx][em_speed];
			loop = emotes_Combat[idx][em_loop];
			lockx = emotes_Combat[idx][em_lockx];
			locky = emotes_Combat[idx][em_locky];
			freeze = emotes_Combat[idx][em_freeze];
			etime = emotes_Combat[idx][em_time];
		}
		case 5:
		{
			if(idx >= emoteCategoryCount[5]) return 0;
			lib = emotes_Police[idx][em_lib];
			anim = emotes_Police[idx][em_anim];
			speed = emotes_Police[idx][em_speed];
			loop = emotes_Police[idx][em_loop];
			lockx = emotes_Police[idx][em_lockx];
			locky = emotes_Police[idx][em_locky];
			freeze = emotes_Police[idx][em_freeze];
			etime = emotes_Police[idx][em_time];
		}
		case 6:
		{
			if(idx >= emoteCategoryCount[6]) return 0;
			lib = emotes_Armes[idx][em_lib];
			anim = emotes_Armes[idx][em_anim];
			speed = emotes_Armes[idx][em_speed];
			loop = emotes_Armes[idx][em_loop];
			lockx = emotes_Armes[idx][em_lockx];
			locky = emotes_Armes[idx][em_locky];
			freeze = emotes_Armes[idx][em_freeze];
			etime = emotes_Armes[idx][em_time];
		}
		case 7:
		{
			if(idx >= emoteCategoryCount[7]) return 0;
			lib = emotes_Militaire[idx][em_lib];
			anim = emotes_Militaire[idx][em_anim];
			speed = emotes_Militaire[idx][em_speed];
			loop = emotes_Militaire[idx][em_loop];
			lockx = emotes_Militaire[idx][em_lockx];
			locky = emotes_Militaire[idx][em_locky];
			freeze = emotes_Militaire[idx][em_freeze];
			etime = emotes_Militaire[idx][em_time];
		}
		case 8:
		{
			// [VIP] Double-check ici au cas ou Emotes_Play serait appele hors dialog
			if(PlayerInfo[playerid][pDonateRank] < 1) return 0;
			if(idx >= emoteCategoryCount[8]) return 0;
			lib = emotes_Vip[idx][em_lib];
			anim = emotes_Vip[idx][em_anim];
			speed = emotes_Vip[idx][em_speed];
			loop = emotes_Vip[idx][em_loop];
			lockx = emotes_Vip[idx][em_lockx];
			locky = emotes_Vip[idx][em_locky];
			freeze = emotes_Vip[idx][em_freeze];
			etime = emotes_Vip[idx][em_time];
		}
		default: return 0;
	}
	// [FIX EMOTES] forcesync=1 obligatoire : sans ce parametre (defaut 0),
	// beaucoup d'animations "idle"/RP ne s'affichent PAS pour le joueur qui
	// les declenche lui-meme (piege classique SA-MP). C'etait la cause
	// probable du "aucune emote ne se joue" - le code appelait bien
	// ApplyAnimation, mais sans forcesync l'anim ne se voyait jamais.
	ApplyAnimation(playerid, lib, anim, speed, loop, lockx, locky, freeze, etime, 1);
	return 1;
}

stock Emotes_Stop(playerid)
{
	ClearAnimations(playerid);
	SendClientMessage(playerid, 0x00FF00FF, "{00FF00}[Emotes]{FFFFFF} Animation arretee.");
	return 1;
}

// Dispatcher dialog (a appeler depuis OnDialogResponse)
stock Emotes_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == DIALOG_EMOTES_CATS)
	{
		if(!response) return 1;
		// 10e option (index 9) = "Arreter l'animation"
		if(listitem == 9) { Emotes_Stop(playerid); return 1; }
		Emotes_ShowList(playerid, listitem);
		return 1;
	}
	if(dialogid == DIALOG_EMOTES_LIST)
	{
		if(!response) { Emotes_ShowCategories(playerid); return 1; }
		new cat = GetPVarInt(playerid, "emoteCat");
		Emotes_Play(playerid, cat, listitem);
		return 1;
	}
	return 0;
}
