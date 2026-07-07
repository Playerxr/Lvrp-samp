/*
	afrp_jobicons.inc - Icones map pour chaque job

	Cree une icone briefcase (56) sur la map a la position de chaque
	employeur, visible pour TOUS les joueurs. Permet aux nouveaux
	de trouver direct un job sans avoir a faire /jobs.

	A appeler depuis OnGameModeInit -> JobIcons_Init();
*/

#if defined AFRP_JOBICONS_INCLUDED
	#endinput
#endif
#define AFRP_JOBICONS_INCLUDED

stock JobIcons_Init()
{
	// Memes coords que dans le dialog /jobs (30318)
	static const Float:jobCoords[13][3] = {
		{2117.32, -1790.55, 13.55}, // 0  Pizza
		{-382.69, -1426.28, 26.24}, // 1  Fermier
		{2276.65, -2226.90, 13.54}, // 2  Eboueur
		{1969.42, -2189.07, 13.54}, // 3  Pilote
		{1743.12, -1582.02, 13.54}, // 4  Facteur
		{ 906.10, -1904.29,  1.12}, // 5  Pecheur
		{ 321.88, -1519.53, 36.03}, // 6  Voiturier
		{2422.93, -2075.87, 13.55}, // 7  Camionneur
		{1906.70, -2207.90, 13.54}, // 8  Bagagiste
		{1174.00,  -485.00, 26.54}, // 9  Bucheron
		{1008.49, -1358.39, 13.39}, // 10 Mecanicien
		{1059.80, -1350.60, 13.54}, // 11 Taxi
		{1516.96, -1022.16, 23.83}  // 12 Convoyeur fonds
	};
	// Icone par job : 56=briefcase, 22=truck/transport, 25=food, 27=pay'n'spray,
	// 33=fishing, 35=plane, 52=shop
	static const jobIconIDs[13] = {
		25, // Pizza
		56, // Fermier
		56, // Eboueur
		35, // Pilote
		22, // Facteur
		33, // Pecheur
		56, // Voiturier
		22, // Camionneur
		56, // Bagagiste
		56, // Bucheron
		27, // Mecanicien
		22, // Taxi
		52  // Convoyeur
	};
	new count = 0;
	for(new i = 0; i < 13; i++)
	{
		CreateDynamicMapIcon(
			jobCoords[i][0], jobCoords[i][1], jobCoords[i][2],
			jobIconIDs[i],
			0,                  // color (0 = defaut)
			-1, -1, -1,
			300.0,              // streamdist 300m
			MAPICON_GLOBAL      // visible pour tous
		);
		count++;
	}
	printf("[JOB ICONS] %d icones jobs creees sur la map.", count);
	return count;
}
