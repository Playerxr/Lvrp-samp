/*
	afrp_meublesdehors.inc - Meubles places EN DEHORS des maisons

	Le systeme de mobilier d'origine ne fonctionne qu'a l'interieur d'une
	maison (stockage dans house[]). Ce module ajoute la possibilite de
	poser des meubles dans le monde exterieur, limites a 10 par joueur.

	/ma mobilier ouvre desormais un choix "Dans la maison" / "Dehors" :
	  - Dans la maison : flux d'origine (verifie qu'on est bien dans une
	    maison - l'interieur des maisons a un Z > 0).
	  - Dehors : ce module. Pose un objet a la position du joueur, editable
	    (deplacement libre), supprimable, limite a 10 objets par joueur.

	Persistance : scriptfiles/meubles_dehors.cfg (une ligne par meuble).
	Format : owner_sqlid|model|x|y|z|rx|ry|rz|vw|interior
*/

#if defined AFRP_MEUBLESDEHORS_INCLUDED
	#endinput
#endif
#define AFRP_MEUBLESDEHORS_INCLUDED

#define MD_MAX_TOTAL       600      // total de meubles exterieurs serveur
#define MD_MAX_PER_PLAYER  10       // limite par joueur
#define MD_FILE            "meubles_dehors.cfg"
#define MD_DLG_MENU        9970     // menu dehors (poser/liste)
#define MD_DLG_CAT         9971     // choix categorie
#define MD_DLG_LIST        9972     // liste de mes meubles
#define MD_DLG_ITEM        9973     // action sur un meuble (editer/supprimer)

enum e_MD
{
	mdUsed,
	mdOwnerSqlid,
	mdModel,
	Float:mdX, Float:mdY, Float:mdZ,
	Float:mdRX, Float:mdRY, Float:mdRZ,
	mdVW, mdInt,
	mdObject
};
new MDfurn[MD_MAX_TOTAL][e_MD];
new md_EditSlot[MAX_PLAYERS];      // slot en cours d'edition (-1 si aucun)
new md_ListSlot[MAX_PLAYERS][MD_MAX_PER_PLAYER]; // mapping listitem -> slot reel
new md_Count = 0;

// Les categories reutilisent les menus deja charges dans house_FurnituresList
// (voir house_LoadModels dans LVRP.pwn) : bath, beds, chairs, divers, floors,
// kitchen, lights, pictures, plants, shelfs, sounds, tables, tables&chairs,
// tvs, gym, walls, gates (index 0 a 16).

stock MD_Init()
{
	for(new i = 0; i < MD_MAX_TOTAL; i++)
	{
		MDfurn[i][mdUsed] = 0;
		MDfurn[i][mdObject] = -1;
	}
	for(new p = 0; p < MAX_PLAYERS; p++) md_EditSlot[p] = -1;
	MD_Load();
	return 1;
}

stock MD_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock MD_CreateObject(slot)
{
	if(MDfurn[slot][mdUsed] != 1) return 0;
	if(MDfurn[slot][mdObject] != -1) { DestroyDynamicObject(MDfurn[slot][mdObject]); MDfurn[slot][mdObject] = -1; }
	MDfurn[slot][mdObject] = CreateDynamicObject(MDfurn[slot][mdModel],
		MDfurn[slot][mdX], MDfurn[slot][mdY], MDfurn[slot][mdZ],
		MDfurn[slot][mdRX], MDfurn[slot][mdRY], MDfurn[slot][mdRZ],
		MDfurn[slot][mdVW], MDfurn[slot][mdInt], -1, 200.0);
	return 1;
}

stock MD_Load()
{
	if(!fexist(MD_FILE)) return 0;
	new File:f = fopen(MD_FILE, io_read);
	if(!f) return 0;
	new line[256], tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 5) continue;
		if(md_Count >= MD_MAX_TOTAL) break;
		new slot = md_Count;
		new i = 0;
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdOwnerSqlid] = strval(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdModel]      = strval(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdX]  = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdY]  = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdZ]  = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdRX] = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdRY] = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdRZ] = floatstr(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdVW]  = strval(tmpBuf);
		MD_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); MDfurn[slot][mdInt] = strval(tmpBuf);
		if(MDfurn[slot][mdModel] <= 0) continue;
		MDfurn[slot][mdUsed] = 1;
		MD_CreateObject(slot);
		md_Count++;
	}
	fclose(f);
	printf("[MeublesDehors] %d meuble(s) exterieur(s) charge(s)", md_Count);
	return md_Count;
}

stock MD_Save()
{
	if(fexist(MD_FILE)) fremove(MD_FILE);
	new File:f = fopen(MD_FILE, io_write);
	if(!f) return 0;
	new line[256];
	for(new i = 0; i < MD_MAX_TOTAL; i++)
	{
		if(MDfurn[i][mdUsed] != 1) continue;
		format(line, sizeof(line), "%d|%d|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%d|%d\n",
			MDfurn[i][mdOwnerSqlid], MDfurn[i][mdModel],
			MDfurn[i][mdX], MDfurn[i][mdY], MDfurn[i][mdZ],
			MDfurn[i][mdRX], MDfurn[i][mdRY], MDfurn[i][mdRZ],
			MDfurn[i][mdVW], MDfurn[i][mdInt]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock MD_CountForOwner(ownerSqlid)
{
	new c = 0;
	for(new i = 0; i < MD_MAX_TOTAL; i++)
	{
		if(MDfurn[i][mdUsed] != 1) continue;
		if(MDfurn[i][mdOwnerSqlid] == ownerSqlid) c++;
	}
	return c;
}

stock MD_AllocSlot()
{
	for(new i = 0; i < MD_MAX_TOTAL; i++)
		if(MDfurn[i][mdUsed] == 0) return i;
	return -1;
}

stock MD_SlotByObject(objectid)
{
	if(objectid == -1) return -1;
	for(new i = 0; i < MD_MAX_TOTAL; i++)
	{
		if(MDfurn[i][mdUsed] != 1) continue;
		if(MDfurn[i][mdObject] == objectid) return i;
	}
	return -1;
}

// Pose un meuble exterieur a la position du joueur. Retourne le slot ou -1.
stock MD_Place(playerid, furModel)
{
	if(MD_CountForOwner(PlayerInfo[playerid][pSQLID]) >= MD_MAX_PER_PLAYER)
	{
		new msg[128];
		format(msg, sizeof(msg), "{CF9756} Mobilier {FFFFFF} Limite de {FFFFB2}%d{FFFFFF} meubles exterieurs atteinte. Supprime-en un d'abord.", MD_MAX_PER_PLAYER);
		msg_Client(playerid, COLOR_INFO, msg);
		return -1;
	}
	new slot = MD_AllocSlot();
	if(slot == -1) { msg_Client(playerid, COLOR_INFO, "{CF9756} Mobilier {FFFFFF} Limite serveur atteinte."); return -1; }

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	// Pose un peu devant le joueur
	new Float:a; GetPlayerFacingAngle(playerid, a);
	x -= floatsin(a, degrees) * 1.5;
	y += floatcos(a, degrees) * 1.5;

	MDfurn[slot][mdUsed]       = 1;
	MDfurn[slot][mdOwnerSqlid] = PlayerInfo[playerid][pSQLID];
	MDfurn[slot][mdModel]      = furModel;
	MDfurn[slot][mdX] = x; MDfurn[slot][mdY] = y; MDfurn[slot][mdZ] = z;
	MDfurn[slot][mdRX] = 0.0; MDfurn[slot][mdRY] = 0.0; MDfurn[slot][mdRZ] = 0.0;
	MDfurn[slot][mdVW]  = GetPlayerVirtualWorld(playerid);
	MDfurn[slot][mdInt] = GetPlayerInterior(playerid);
	md_Count++;
	MD_CreateObject(slot);
	MD_Save();

	// Lance directement l'edition pour un placement precis
	md_EditSlot[playerid] = slot;
	EditDynamicObject(playerid, MDfurn[slot][mdObject]);
	msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Meuble pose. Deplace-le puis valide (clic droit = annuler l'edition).");
	return slot;
}

stock MD_ShowCategories(playerid)
{
	ShowPlayerDialog(playerid, MD_DLG_CAT, DIALOG_STYLE_LIST, "{42AAFF} Mobilier exterieur {FFFFFF} Categorie",
		"Salle de bain\nLits\nChaises\nDivers\nTapis\nCuisine\nLampes\nTableaux\nPlantes\nArmoires\nStereo\nTables\nTables & chaises\nTeles\nGym\nMurs\nPortes",
		"Choisir", "Retour");
	return 1;
}

stock MD_ShowMenu(playerid)
{
	new body[256];
	new nb = MD_CountForOwner(PlayerInfo[playerid][pSQLID]);
	format(body, sizeof(body), "{FFFFFF}Tu as {FFFFB2}%d/%d{FFFFFF} meubles exterieurs.\n\nPoser un nouveau meuble\nGerer mes meubles (deplacer/supprimer)", nb, MD_MAX_PER_PLAYER);
	ShowPlayerDialog(playerid, MD_DLG_MENU, DIALOG_STYLE_LIST, "{42AAFF} Mobilier exterieur", body, "Choisir", "Fermer");
	return 1;
}

stock MD_ShowList(playerid)
{
	new body[1024], tmp[96], c = 0;
	strcat(body, "{FFFFFF}Tes meubles exterieurs :\n\n");
	for(new i = 0; i < MD_MAX_TOTAL && c < MD_MAX_PER_PLAYER; i++)
	{
		if(MDfurn[i][mdUsed] != 1) continue;
		if(MDfurn[i][mdOwnerSqlid] != PlayerInfo[playerid][pSQLID]) continue;
		md_ListSlot[playerid][c] = i;
		format(tmp, sizeof(tmp), "Meuble %d (modele %d)\n", c + 1, MDfurn[i][mdModel]);
		strcat(body, tmp);
		c++;
	}
	if(c == 0) { msg_Client(playerid, COLOR_INFO, "{CF9756} Mobilier {FFFFFF} Aucun meuble exterieur pose."); return 1; }
	ShowPlayerDialog(playerid, MD_DLG_LIST, DIALOG_STYLE_LIST, "{42AAFF} Mes meubles exterieurs", body, "Gerer", "Retour");
	return 1;
}

stock MD_Remove(slot)
{
	if(slot < 0 || slot >= MD_MAX_TOTAL) return 0;
	if(MDfurn[slot][mdUsed] != 1) return 0;
	if(MDfurn[slot][mdObject] != -1) { DestroyDynamicObject(MDfurn[slot][mdObject]); MDfurn[slot][mdObject] = -1; }
	MDfurn[slot][mdUsed] = 0;
	md_Count--; if(md_Count < 0) md_Count = 0;
	MD_Save();
	return 1;
}

// Retourne 1 si le model select a ete traite ici (mode dehors actif).
stock MD_OnModelSelection(playerid, response, furModel)
{
	if(GetPVarInt(playerid, "md_placing") != 1) return 0;
	DeletePVar(playerid, "md_placing");
	if(!response) return 1;
	MD_Place(playerid, furModel);
	return 1;
}

// Retourne 1 si l'edition d'objet concernait un meuble exterieur.
stock MD_OnEditObject(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
	new slot = md_EditSlot[playerid];
	if(slot < 0 || slot >= MD_MAX_TOTAL) return 0;
	if(MDfurn[slot][mdUsed] != 1) { md_EditSlot[playerid] = -1; return 0; }
	if(MDfurn[slot][mdObject] != objectid) return 0;

	if(response == EDIT_RESPONSE_CANCEL)
	{
		// Repositionne l'objet a sa derniere position sauvegardee
		SetDynamicObjectPos(MDfurn[slot][mdObject], MDfurn[slot][mdX], MDfurn[slot][mdY], MDfurn[slot][mdZ]);
		SetDynamicObjectRot(MDfurn[slot][mdObject], MDfurn[slot][mdRX], MDfurn[slot][mdRY], MDfurn[slot][mdRZ]);
		md_EditSlot[playerid] = -1;
		msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Edition annulee.");
		return 1;
	}
	if(response == EDIT_RESPONSE_FINAL)
	{
		MDfurn[slot][mdX] = x; MDfurn[slot][mdY] = y; MDfurn[slot][mdZ] = z;
		MDfurn[slot][mdRX] = rx; MDfurn[slot][mdRY] = ry; MDfurn[slot][mdRZ] = rz;
		SetDynamicObjectPos(MDfurn[slot][mdObject], x, y, z);
		SetDynamicObjectRot(MDfurn[slot][mdObject], rx, ry, rz);
		md_EditSlot[playerid] = -1;
		MD_Save();
		msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Position sauvegardee.");
		return 1;
	}
	return 1;
}

// Dispatcher dialog (dialogid 9970-9973). Retourne 1 si traite.
stock MD_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == MD_DLG_MENU)
	{
		if(!response) return 1;
		if(listitem == 0) return MD_ShowCategories(playerid);
		if(listitem == 1) return MD_ShowList(playerid);
		return 1;
	}
	if(dialogid == MD_DLG_CAT)
	{
		if(!response) { MD_ShowMenu(playerid); return 1; }
		if(listitem < 0 || listitem > 16) return 1;
		SetPVarInt(playerid, "md_placing", 1);
		ShowModelSelectionMenu(playerid, house_FurnituresList[listitem], "Choisis un meuble");
		return 1;
	}
	if(dialogid == MD_DLG_LIST)
	{
		if(!response) { MD_ShowMenu(playerid); return 1; }
		if(listitem < 0 || listitem >= MD_MAX_PER_PLAYER) return 1;
		new slot = md_ListSlot[playerid][listitem];
		if(slot < 0 || slot >= MD_MAX_TOTAL || MDfurn[slot][mdUsed] != 1) return 1;
		SetPVarInt(playerid, "md_manageslot", slot);
		ShowPlayerDialog(playerid, MD_DLG_ITEM, DIALOG_STYLE_LIST, "{42AAFF} Meuble exterieur",
			"Deplacer ce meuble\nSupprimer ce meuble\nTe teleporter dessus", "Choisir", "Retour");
		return 1;
	}
	if(dialogid == MD_DLG_ITEM)
	{
		if(!response) { MD_ShowList(playerid); return 1; }
		new slot = GetPVarInt(playerid, "md_manageslot");
		if(slot < 0 || slot >= MD_MAX_TOTAL || MDfurn[slot][mdUsed] != 1) return 1;
		if(listitem == 0)
		{
			md_EditSlot[playerid] = slot;
			EditDynamicObject(playerid, MDfurn[slot][mdObject]);
			msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Deplace le meuble puis valide.");
		}
		else if(listitem == 1)
		{
			MD_Remove(slot);
			msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Meuble supprime.");
		}
		else if(listitem == 2)
		{
			SetPlayerInterior(playerid, MDfurn[slot][mdInt]);
			SetPlayerVirtualWorld(playerid, MDfurn[slot][mdVW]);
			SafeSetPlayerPos(playerid, MDfurn[slot][mdX], MDfurn[slot][mdY], MDfurn[slot][mdZ] + 1.0);
			msg_Client(playerid, COLOR_WHITE, "{42AAFF} Mobilier {FFFFFF} Teleporte sur ton meuble.");
		}
		return 1;
	}
	return 0;
}
