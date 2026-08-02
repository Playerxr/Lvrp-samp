/*
	afrp_postepolice.inc - Poste de police routiere posable en plusieurs
	exemplaires (/a poste)

	POURQUOI CE MODULE N'EST PAS UNE MAP DE PLUS : les autres maps
	communautaires sont uniques et vivent a un endroit fixe. Celle-ci, le
	proprietaire veut la reutiliser telle quelle a plusieurs endroits du
	serveur, sur les axes routiers de son choix. Le decor est donc ecrit une
	seule fois, sous forme de fonction parametree par un point d'ancrage et un
	angle, et le module tient une petite liste des exemplaires poses.

	  /a poste                   aide + liste des exemplaires
	  /a poste poser             pose un exemplaire ici, dans l'angle du regard
	  /a poste liste             liste les exemplaires et leurs coordonnees
	  /a poste aller <n>         se teleporte a l'exemplaire n
	  /a poste supprimer <n>     retire l'exemplaire n
	(admin niveau 3, comme /a deplacer et les autres /a de creation)

	PLAFOND : 6 exemplaires simultanes. Chaque exemplaire compte 178 objets,
	soit 1068 objets au maximum. Le streamer encaisse largement, mais au-dela le
	decor cesse d'etre un point de repere et devient du bruit sur la carte : le
	plafond est la pour ca, pas pour des raisons techniques.

	CE QUE CONTIENT UN EXEMPLAIRE : la scene entiere telle que son auteur l'a
	construite, c'est-a-dire le poste ET la portion de route, les plots et les
	arbres qui l'entourent (l'ensemble s'etend sur environ 65 metres de large
	et 160 de long). Ce n'est pas un oubli : decouper la scene aurait voulu
	dire choisir a la place du proprietaire quels objets sont "le poste". Il
	faut donc poser les exemplaires le long d'une route, pas en plein centre
	ville. Si elle prefere une version reduite au seul batiment, il suffit de
	le dire, la liste des objets est ici.

	REBRANDING : la source est un poste de la police federale routiere
	bresilienne, avec des enseignes en portugais decoupees lettre par lettre
	sur plusieurs objets (un objet porte "RODO", le suivant "VIARIA"). Les
	remplacements gardent le meme nombre de caracteres a un pres, sinon les
	morceaux ne se rejoignent plus a l'ecran. Enseigne finale : "POLICE
	NATIONALE ROUTIERE".
	  "PRF"        -> "PNR"        (sigle, meme longueur)
	  "POLICIA"    -> "POLICE"
	  "RODOVIARIA" -> "NATIONALE"
	  "FEDERAL"    -> "ROUTIERE"
	  "RODO"+"VIARIA" -> "NATI"+"ONALE"
	  "F"+"EDERAL"    -> "R"+"OUTIERE"
	  "190"        -> "911"
	Le 190 est le numero d'urgence bresilien. Le serveur en a un seul pour la
	police, le 911 : c'est celui que /numero police renvoie, celui du dialogue
	d'urgence du telephone et celui de l'annuaire ("Police LSPD - /call 911").
	Le 15 sert a l'hopital, il n'avait rien a faire sur un poste de police.

	PERSISTANCE : meme principe que MapPack_Save/MapPack_Load et
	afrp_mapmove.inc - un fichier texte dans scriptfiles, relu au demarrage,
	une ligne par exemplaire pose.

	PREMIER DEMARRAGE : sans fichier de sauvegarde, un exemplaire est pose a la
	position d'origine de la map. Sinon le portage ne se verrait nulle part et
	il faudrait deviner ou aller le chercher. Il se supprime comme les autres.

	A VERIFIER EN JEU (non verifiable depuis ce sandbox, pas de client SA-MP
	ici) : que la scene se pose bien au niveau du sol la ou elle est deposee.
	La hauteur de reference du decor (PP_PIVOT_Z ci-dessous) est estimee a
	partir des donnees ; si tous les exemplaires flottent ou s'enfoncent de la
	meme distance, c'est ce seul nombre qu'il faut corriger.

	ATTENTION DOUBLON : le pack de mappings contient deja cette map ("Poste PRF
	(Bresil)"). Elle est eteinte, et /mappack refuse desormais de l'allumer
	(voir afrp_mappack.inc) : sinon le poste d'origine existerait en double.

	Map communautaire creditee NINJAx_MAPPER (export Texture Studio).
*/

#if defined AFRP_POSTEPOLICE_INCLUDED
	#endinput
#endif
#define AFRP_POSTEPOLICE_INCLUDED

#define PP_ADMIN_LEVEL  3
#define PP_FILE         "postepolice.cfg"
#define PP_MAX_COPIES   6
#define PP_MAX_OBJ      183

// Point de reference du decor d'origine : centre du poste lui-meme (et non du
// rectangle englobant, qui tomberait au milieu de la route a cause des arbres
// et des plaques de bitume alignes le long de l'axe). C'est ce point-la qui
// vient se poser sous les pieds de l'admin.
#define PP_PIVOT_X      -160.0000
#define PP_PIVOT_Y      -1333.0000
#define PP_PIVOT_Z      0.5000

new ppPose[PP_MAX_COPIES];
new Float:ppX[PP_MAX_COPIES];
new Float:ppY[PP_MAX_COPIES];
new Float:ppZ[PP_MAX_COPIES];
new Float:ppRot[PP_MAX_COPIES];
new ppObj[PP_MAX_COPIES][PP_MAX_OBJ];
new ppNb[PP_MAX_COPIES];

// ============================================================================
// OUTILS
// ============================================================================
stock Pp_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// Pose un objet du decor pour un exemplaire donne. Les coordonnees passees sont
// TOUJOURS celles de l'auteur : c'est ici, et seulement ici, qu'on les ramene
// autour du point d'ancrage de l'exemplaire. Meme convention de rotation que
// MM_RotateOffset / Prison_RotateOffset.
stock Pp_Obj(copie, model, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, wid = -1, iid = -1, pid = -1, Float:sd = 300.0, Float:dd = 300.0)
{
	new Float:dx, Float:dy;
	MM_RotateOffset(x - PP_PIVOT_X, y - PP_PIVOT_Y, ppRot[copie], dx, dy);
	new Float:nrz = rz + ppRot[copie];
	while(nrz >= 360.0) nrz -= 360.0;
	while(nrz < 0.0) nrz += 360.0;

	new objectid = CreateDynamicObject(model, ppX[copie] + dx, ppY[copie] + dy, ppZ[copie] + (z - PP_PIVOT_Z), rx, ry, nrz, wid, iid, pid, sd, dd);
	if(ppNb[copie] < PP_MAX_OBJ) ppObj[copie][ppNb[copie]++] = objectid;
	return objectid;
}

stock Pp_Detruire(copie)
{
	if(copie < 0 || copie >= PP_MAX_COPIES) return 0;
	new n = 0;
	for(new k = 0; k < ppNb[copie]; k++)
	{
		if(ppObj[copie][k] > 0 && IsValidDynamicObject(ppObj[copie][k]))
		{
			DestroyDynamicObject(ppObj[copie][k]);
			n++;
		}
		ppObj[copie][k] = 0;
	}
	ppNb[copie] = 0;
	return n;
}

// ============================================================================
// LE DECOR (donnees portees telles quelles depuis l'export fourni)
// ============================================================================
stock Pp_CreerDecor(copie)
{
	if(copie < 0 || copie >= PP_MAX_COPIES) return 0;
	ppNb[copie] = 0;
	new tmpobjid;
	tmpobjid = Pp_Obj(copie, 18766, -156.826507, -1341.470703, 0.723100, -67.000000, 0.000000, 87.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18800, "mroadhelix1", "road1-3", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -159.862396, -1326.427490, 0.428200, -180.000000, -90.000000, 87.798103, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -164.406936, -1335.645874, 0.428250, -180.000000, -90.000000, 357.303649, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19435, -159.248855, -1330.170532, 2.737799, 0.000000, -90.000000, 268.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-80-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19435, -159.097427, -1327.009521, 1.786900, 0.000000, -55.000000, 89.649360, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-80-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19435, -159.370590, -1333.662231, 2.737799, 0.000000, -90.000000, 268.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-80-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -158.384521, -1328.418334, 1.821200, 0.000000, 0.000000, 357.728698, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -158.627685, -1334.802734, 1.818130, 0.000000, 0.000000, 358.147003, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -158.512573, -1331.612182, 1.822170, 0.000000, 0.000000, 357.728668, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18980, -164.684112, -1346.567626, -3.211250, 0.000000, 0.000000, 358.953765, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -166.078475, -1340.987426, 1.659649, -90.000000, 0.000000, 86.982063, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18800, "mroadhelix1", "road1-3", 0x00000000);
	tmpobjid = Pp_Obj(copie, 1499, -159.788253, -1330.438110, 2.824399, 0.000000, 0.000000, 87.834877, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-80-percent", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 18646, "matcolours", "grey", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -165.166625, -1331.252685, 5.822999, -90.000000, 0.000000, 87.691101, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18981, -155.682418, -1337.324218, 8.985349, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18980, -144.890502, -1325.772338, -3.211299, 0.000000, 0.000000, 267.047424, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -161.429702, -1341.230712, 1.659649, -90.000000, 0.000000, 86.982063, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18800, "mroadhelix1", "road1-3", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18980, -158.593124, -1346.925415, -3.211250, 0.000000, 0.000000, 358.953765, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18980, -147.003875, -1347.301879, -3.211250, 0.000000, 0.000000, 358.953765, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -167.057754, -1327.729003, 2.915600, -180.000000, 0.000000, 88.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -162.060195, -1331.378173, 5.822999, -90.000000, 0.000000, 87.691101, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -161.751052, -1333.771484, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -165.104003, -1333.727539, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -161.752975, -1330.597778, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -161.770401, -1327.616455, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -161.783142, -1325.364013, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -165.122879, -1325.405639, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -165.133834, -1328.619140, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19362, -165.096099, -1331.771118, 2.737200, 0.000000, -90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-10-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -164.302749, -1323.344970, 0.428250, -180.000000, -90.000000, 357.303649, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -162.249008, -1335.746582, 0.428250, -180.000000, -90.000000, 357.303649, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -161.747497, -1323.462646, 0.428250, -180.000000, -90.000000, 357.303649, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -160.131820, -1332.882568, 0.428200, -180.000000, -90.000000, 87.798103, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -167.171707, -1331.009643, 2.915600, -180.000000, 0.000000, 88.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -161.920410, -1327.947143, 5.822999, -90.000000, 0.000000, 87.691101, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 18766, -165.024215, -1327.797485, 5.822999, -90.000000, 0.000000, 87.691101, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "blue", 0x00000000);
	tmpobjid = Pp_Obj(copie, 19435, -159.490249, -1336.794799, 1.750859, 0.000000, -55.000000, 267.552795, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-80-percent", 0x00000000);
	tmpobjid = Pp_Obj(copie, 2789, -159.438415, -1326.069580, 4.400802, 0.000000, 0.000000, 87.870788, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{0000FF}PNR", 140, "Ariel", 199, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -157.646850, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}POLICE", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -159.663269, -1334.028198, 5.603600, 0.000000, 0.000000, 87.658996, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}POLICE", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 2789, -163.405151, -1336.161010, 4.110759, 0.000000, 0.000000, 357.047424, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}PNR", 130, "Ariel", 199, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -159.486785, -1329.711303, 5.603600, 0.000000, 0.000000, 87.658996, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}NATIONALE", 140, "Ariel", 90, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -159.318466, -1325.594360, 5.603600, 0.000000, 0.000000, 87.658996, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}ROUTIERE", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -154.696853, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}NATI", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -152.796813, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}ONALE", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -149.346923, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}OUTIERE", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -150.746902, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}R", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -146.606918, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}911", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -157.740600, -1324.790161, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}ROUTIERE", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -149.730606, -1324.809326, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}POLICE", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -152.640655, -1324.802490, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}NATI", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -154.240615, -1324.798461, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}ONALE", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -145.610656, -1324.819458, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}911", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -167.200515, -1324.766235, 9.183603, 0.000000, 0.000000, 179.859024, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}911", 140, "Ariel", 130, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 19327, -167.046920, -1349.864135, 9.018630, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFF00}911", 140, "Ariel", 150, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Pp_Obj(copie, 2611, -166.512802, -1327.944824, 4.384940, 0.000000, 0.000000, 88.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2198, -165.320663, -1329.398559, 2.822799, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1806, -166.490280, -1330.089477, 2.824599, 0.000000, 0.000000, -84.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2161, -161.993576, -1335.270507, 2.823499, 0.000000, 0.000000, 177.436035, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2387, -161.102416, -1324.636474, 2.824939, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2104, -166.717514, -1334.690063, 2.824500, 0.000000, 0.000000, 135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2208, -166.307556, -1324.215454, 2.824899, 0.000000, 0.000000, -91.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1694, -140.912857, -1412.860473, 10.567199, 0.000000, 0.000000, 346.179962, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1688, -157.520095, -1300.457885, 2.949300, 0.000000, 0.000000, 351.935211, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 3881, -158.231597, -1304.215332, 3.805700, 0.000000, 0.000000, 171.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1618, -163.729293, -1322.867675, 5.193600, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 11729, -164.577468, -1324.218627, 2.823240, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 11729, -162.246459, -1324.183715, 2.823240, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -144.334686, -1310.319824, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 356, -166.123413, -1326.209106, 3.848000, -5.000000, -41.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 356, -166.122222, -1326.810913, 3.848000, -5.000000, -41.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 356, -166.122619, -1326.609985, 3.848000, -5.000000, -41.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 356, -166.123016, -1326.409057, 3.848000, -5.000000, -41.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 348, -166.351867, -1325.643920, 3.724900, -90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 348, -166.371047, -1326.067749, 3.724900, -90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 348, -166.371444, -1325.866821, 3.724900, -90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2358, -166.233398, -1324.953613, 3.995800, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2041, -166.233520, -1324.266235, 3.872600, 0.000000, 0.000000, 40.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2041, -166.051635, -1324.136230, 3.872600, 0.000000, 0.000000, 40.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19942, -166.044692, -1325.390747, 3.754899, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19942, -165.935577, -1325.312744, 3.754899, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19942, -166.187149, -1325.360595, 3.754899, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2358, -166.233398, -1324.953613, 3.771800, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 11729, -163.052307, -1324.199829, 2.823240, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 11729, -163.769470, -1324.208740, 2.823240, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2010, -166.345642, -1328.847412, 2.823620, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2198, -162.112579, -1333.016357, 2.822819, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2010, -166.467498, -1333.800292, 2.823620, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2167, -163.447402, -1335.279785, 2.823100, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2163, -164.483123, -1335.192382, 2.822900, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 2010, -160.995193, -1335.143066, 2.823620, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1209, -166.520401, -1332.737670, 2.823999, 0.000000, 0.000000, 89.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -152.496063, -1346.957519, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -150.244079, -1309.793334, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -147.027160, -1310.072387, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1237, -159.076843, -1349.318725, 1.682189, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1278, -141.825393, -1308.685180, -2.064300, 0.000000, 0.000000, 295.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -153.356323, -1309.508300, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -149.205017, -1347.252807, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -150.204071, -1409.237182, 1.690099, 0.000000, 0.000000, 190.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -157.044342, -1346.538452, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -146.820571, -1295.210571, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -145.059310, -1327.648071, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -145.307403, -1330.770141, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -145.565658, -1333.913208, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -145.827804, -1337.304199, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -146.070098, -1340.887939, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1282, -151.919082, -1393.427246, 2.324300, 0.000000, 0.000000, 279.868988, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -151.150772, -1393.831176, 2.068599, 0.000000, 0.000000, 76.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -146.807846, -1412.713500, 2.068599, 0.000000, 0.000000, -45.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -144.693435, -1403.804077, 2.068599, 0.000000, 0.000000, 5.443799, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -142.561752, -1413.327880, 2.068599, 0.000000, 0.000000, 5.443799, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -148.598861, -1407.800292, 2.068599, 0.000000, 0.000000, -78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -147.094131, -1404.038208, 2.068599, 0.000000, 0.000000, 5.443799, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -144.656097, -1413.532104, 2.068599, 0.000000, 0.000000, 5.443799, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19953, -149.807998, -1330.644897, 1.681820, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1278, -141.748992, -1412.829589, 1.684200, 0.000000, 0.000000, 25.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1278, -143.659103, -1404.457885, 1.684200, 0.000000, 0.000000, -137.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1315, -149.420104, -1409.659179, 4.991940, 0.000000, 0.000000, 9.192609, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -155.799636, -1346.652465, 1.690089, 0.000000, 0.000000, 354.755798, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19425, -152.923660, -1409.714233, 1.690099, 0.000000, 0.000000, 190.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -148.001907, -1410.484497, 2.068599, 0.000000, 0.000000, -78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -158.882858, -1419.037841, -1.556399, 0.000000, 0.000000, -273.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -128.482101, -1327.354370, -1.556399, 0.000000, 0.000000, -127.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -137.070343, -1357.283447, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -102.547302, -1308.698852, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -136.328353, -1283.322387, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -156.925384, -1286.839721, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -154.930282, -1410.259765, 2.168600, 0.000000, 0.000000, -78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -155.618667, -1407.322265, 2.168600, 0.000000, 0.000000, -78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -156.263595, -1404.278930, 2.168600, 0.000000, 0.000000, -78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -152.603698, -1375.588867, 2.168600, 0.000000, 0.000000, 271.263519, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -156.836547, -1401.161743, 2.168600, 0.000000, 0.000000, 278.250549, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19954, -151.375305, -1393.200561, 1.684980, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -148.565292, -1405.289062, 2.068599, 0.000000, 0.000000, 76.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1282, -147.207443, -1413.716064, 2.324300, 0.000000, 0.000000, 279.868988, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19982, -144.783996, -1393.717773, 1.687800, 0.000000, 0.000000, -40.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19984, -150.189193, -1274.500488, 1.687800, 0.000000, 0.000000, 158.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19982, -147.297897, -1348.243652, 1.687800, 0.000000, 0.000000, -55.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19983, -157.713989, -1395.749389, 1.687800, 0.000000, 0.000000, 158.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1282, -140.586822, -1413.769409, 2.324300, 0.000000, 0.000000, 279.868988, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19983, -132.778594, -1436.915283, 1.687800, 0.000000, 0.000000, -25.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -140.937026, -1385.132568, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1694, -160.725143, -1314.688720, 10.667200, 0.000000, 0.000000, 351.068603, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -157.292633, -1397.650268, 2.168600, 0.000000, 0.000000, 278.250549, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -152.085952, -1383.937866, 2.168600, 0.000000, 0.000000, 274.772033, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -152.397003, -1379.473999, 2.168600, 0.000000, 0.000000, 274.772033, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 705, -162.554351, -1376.068725, -1.556399, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -152.061569, -1345.122314, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -140.614242, -1296.972900, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -147.407272, -1299.758178, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -154.511306, -1297.935791, 2.245899, 0.000000, 0.000000, 129.067596, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1315, -155.017593, -1313.028198, 5.018400, 0.000000, 0.000000, 173.390289, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -148.494689, -1308.071899, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -154.783248, -1307.138916, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -154.174224, -1301.906372, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -147.983016, -1304.171264, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -141.651840, -1304.991333, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -141.131332, -1300.999145, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19984, -132.195907, -1327.730957, 1.052000, 0.000000, 0.000000, -47.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19983, -155.049865, -1309.316406, 1.687800, 0.000000, 0.000000, 158.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -149.043670, -1312.349365, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -142.463806, -1312.863159, 2.038899, 0.000000, 0.000000, 262.724884, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -142.588043, -1316.937866, 1.837900, 0.000000, 0.000000, 246.090789, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -144.438964, -1323.701538, 2.038899, 0.000000, 0.000000, 255.763046, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -143.505508, -1320.330566, 2.038899, 0.000000, 0.000000, 255.763046, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 0, -142.288803, -1310.540893, 1.687800, 0.000000, 0.000000, -55.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -146.357681, -1344.560791, 2.038899, 0.000000, 0.000000, 265.549224, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -146.536499, -1326.938354, 2.038899, 0.000000, 0.000000, 229.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1422, -148.760696, -1329.584960, 2.038899, 0.000000, 0.000000, 229.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1237, -158.649658, -1348.074218, 1.682189, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 1237, -159.646606, -1348.054565, 1.682189, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 19953, -146.363052, -1413.968627, 1.681820, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -155.988204, -1317.680786, 1.684999, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -145.033813, -1391.523559, 1.684999, 0.000000, 0.000000, 164.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -142.936874, -1315.163818, 1.684999, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -132.056793, -1326.490356, 0.975899, 0.000000, 0.000000, 149.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -146.865112, -1348.231933, 1.684999, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Pp_Obj(copie, 18880, -157.968994, -1396.427734, 1.684999, 0.000000, 1.000000, -7.000000, -1, -1, -1, 300.00, 300.00);
	#pragma unused tmpobjid
	return ppNb[copie];
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Pp_Save()
{
	if(fexist(PP_FILE)) fremove(PP_FILE);
	new File:f = fopen(PP_FILE, io_write);
	if(!f) return 0;
	new line[128];
	for(new c = 0; c < PP_MAX_COPIES; c++)
	{
		if(ppPose[c] != 1) continue;
		format(line, sizeof(line), "%d|%f|%f|%f|%f\n", c, ppX[c], ppY[c], ppZ[c], ppRot[c]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Pp_Load()
{
	for(new c = 0; c < PP_MAX_COPIES; c++) { ppPose[c] = 0; ppNb[c] = 0; }
	if(!fexist(PP_FILE)) return -1;   // -1 = jamais configure, voir Pp_Init
	new File:f = fopen(PP_FILE, io_read);
	if(!f) return -1;

	new line[160], buf[32], n = 0;
	while(fread(f, line))
	{
		// MM_Champ (afrp_mapmove.inc) decoupe deja ce format "a|b|c" : le
		// reecrire ici ne ferait qu'un deuxieme decoupeur a maintenir.
		MM_Champ(line, 0, buf, sizeof(buf));
		new c = strval(buf);
		if(c < 0 || c >= PP_MAX_COPIES) continue;
		MM_Champ(line, 1, buf, sizeof(buf)); ppX[c] = floatstr(buf);
		MM_Champ(line, 2, buf, sizeof(buf)); ppY[c] = floatstr(buf);
		MM_Champ(line, 3, buf, sizeof(buf)); ppZ[c] = floatstr(buf);
		MM_Champ(line, 4, buf, sizeof(buf)); ppRot[c] = floatstr(buf);
		ppPose[c] = 1;
		n++;
	}
	fclose(f);
	return n;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Pp_Liste(playerid)
{
	new s[190], total = 0;
	Pp_Msg(playerid, "{FF6347}\xbb Poste de police routiere \xab{FFFFFF} exemplaires poses :");
	for(new c = 0; c < PP_MAX_COPIES; c++)
	{
		if(ppPose[c] != 1) continue;
		total++;
		format(s, sizeof(s), "  {FFFF00}%d{FFFFFF} - %.1f / %.1f / %.1f (angle %.0f) - %d objets", c, ppX[c], ppY[c], ppZ[c], ppRot[c], ppNb[c]);
		Pp_Msg(playerid, s);
	}
	if(total == 0) Pp_Msg(playerid, "  {888888}aucun exemplaire pose.");
	format(s, sizeof(s), "{AAAAAA}%d / %d exemplaires. /a poste poser - /a poste aller <n> - /a poste supprimer <n>", total, PP_MAX_COPIES);
	Pp_Msg(playerid, s);
	return 1;
}

stock Pp_Poser(playerid)
{
	new slot = -1;
	for(new c = 0; c < PP_MAX_COPIES; c++) if(ppPose[c] != 1) { slot = c; break; }
	if(slot == -1)
	{
		new s[160];
		format(s, sizeof(s), "{FF4444}Plafond atteint : %d exemplaires poses. Supprime-en un d'abord (/a poste supprimer <n>).", PP_MAX_COPIES);
		return Pp_Msg(playerid, s);
	}

	new Float:px, Float:py, Float:pz, Float:pa;
	GetPlayerPos(playerid, px, py, pz);
	GetPlayerFacingAngle(playerid, pa);
	ppX[slot] = px;
	ppY[slot] = py;
	// GetPlayerPos rend le centre du joueur, environ un metre au-dessus du sol :
	// sans cette correction chaque exemplaire serait pose un metre trop haut.
	ppZ[slot] = pz - 1.0;
	ppRot[slot] = pa;
	ppPose[slot] = 1;

	new n = Pp_CreerDecor(slot);
	Pp_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}Poste de police routiere pose ici (exemplaire %d, %d objets, angle %.0f).", slot, n, ppRot[slot]);
	Pp_Msg(playerid, s);
	Pp_Msg(playerid, "{AAAAAA}La scene comprend la route et les arbres autour du poste : a poser le long d'un axe routier.");
	return 1;
}

stock Pp_Supprimer(playerid, copie)
{
	if(copie < 0 || copie >= PP_MAX_COPIES || ppPose[copie] != 1)
		return Pp_Msg(playerid, "{FF4444}Numero d'exemplaire inconnu. /a poste liste");
	new n = Pp_Detruire(copie);
	ppPose[copie] = 0;
	Pp_Save();
	new s[160];
	format(s, sizeof(s), "{FFFF00}Exemplaire %d retire (%d objets).", copie, n);
	return Pp_Msg(playerid, s);
}

stock Pp_Aller(playerid, copie)
{
	if(copie < 0 || copie >= PP_MAX_COPIES || ppPose[copie] != 1)
		return Pp_Msg(playerid, "{FF4444}Numero d'exemplaire inconnu. /a poste liste");
	SafeTeleportPlayer(playerid, ppX[copie], ppY[copie], ppZ[copie] + 1.0, 0, 0);
	new s[160];
	format(s, sizeof(s), "{00FF00}Sur place : exemplaire %d du poste de police routiere.", copie);
	return Pp_Msg(playerid, s);
}

// arg1 = action, arg2 = numero d'exemplaire quand l'action en attend un.
stock Pp_Cmd(playerid, const arg1[], const arg2[])
{
	if(PlayerInfo[playerid][pAdmin] < PP_ADMIN_LEVEL)
	{
		new err[160];
		format(err, sizeof(err), "{FF0069}\xbb Erreur \xab{FFFFFF} Reserve aux admins niveau %d (tu es niveau %d).", PP_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return Pp_Msg(playerid, err);
	}
	if(!strlen(arg1) || strcmp(arg1, "liste", true) == 0) return Pp_Liste(playerid);
	if(strcmp(arg1, "poser", true) == 0 || strcmp(arg1, "creer", true) == 0) return Pp_Poser(playerid);
	// Sans numero, strval() rendrait 0 et on supprimerait le premier
	// exemplaire sans que personne ne l'ait demande.
	if(strcmp(arg1, "supprimer", true) == 0 || strcmp(arg1, "sup", true) == 0)
	{
		if(!strlen(arg2)) return Pp_Msg(playerid, "{FF4444}Precise le numero : /a poste supprimer <n> (voir /a poste liste).");
		return Pp_Supprimer(playerid, strval(arg2));
	}
	if(strcmp(arg1, "aller", true) == 0)
	{
		if(!strlen(arg2)) return Pp_Msg(playerid, "{FF4444}Precise le numero : /a poste aller <n> (voir /a poste liste).");
		return Pp_Aller(playerid, strval(arg2));
	}
	Pp_Msg(playerid, "{FF4444}Action inconnue.");
	return Pp_Liste(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Pp_Init()
{
	new n = Pp_Load();
	if(n == -1)
	{
		// Aucun fichier de sauvegarde : premier demarrage apres l'ajout du
		// module. On pose un exemplaire a la position d'origine de la map pour
		// que le decor porte soit visible quelque part, et on l'enregistre.
		ppPose[0] = 1;
		ppX[0] = PP_PIVOT_X;
		ppY[0] = PP_PIVOT_Y;
		ppZ[0] = PP_PIVOT_Z;
		ppRot[0] = 0.0;
		n = 1;
		Pp_Save();
	}
	new total = 0;
	for(new c = 0; c < PP_MAX_COPIES; c++)
	{
		if(ppPose[c] != 1) continue;
		total += Pp_CreerDecor(c);
	}
	printf("[POSTE POLICE] %d exemplaire(s), %d objet(s).", n, total);
	return n;
}
