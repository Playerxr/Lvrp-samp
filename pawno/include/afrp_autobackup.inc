/*
	afrp_autobackup.inc - Sauvegarde automatique des fichiers de donnees .cfg

	Copie parkings.cfg et casinos.cfg dans scriptfiles/backups/ toutes les
	AUTOBACKUP_INTERVAL_MS millisecondes, sur 7 emplacements qui tournent
	avec le jour du mois (pas de croissance infinie du dossier, pas besoin
	de lister le contenu d'un dossier - fonction non disponible en Pawn).

	Cree suite a la perte des >70 parkings du systeme /parking lors d'une
	mise a jour du gamemode (fichier ecrase sans aucune sauvegarde).
*/

#if defined AFRP_AUTOBACKUP_INCLUDED
	#endinput
#endif
#define AFRP_AUTOBACKUP_INCLUDED

#define AUTOBACKUP_INTERVAL_MS  (4*60*60*1000)  // toutes les 4h
#define AUTOBACKUP_DIR          "backups/"
#define AUTOBACKUP_FILE_COUNT   15

// entreprises.cfg vaut cher : 150M la creation, plus la caisse, le stock, les
// employes et les grades. On sauvegarde aussi les contrats, les camionnettes
// et les meubles exterieurs, tous stockes en fichier plat comme les parkings
// qui avaient deja ete perdus une fois.
new const AutoBackup_Files[AUTOBACKUP_FILE_COUNT][24] = {
	"parkings.cfg",
	"casinos.cfg",
	"entreprises.cfg",
	"contrats.cfg",
	"entreprise_camions.cfg",
	"objectifs.cfg",
	"events_parcours.cfg",
	"topjour.cfg",
	"assurances.cfg",
	"saison.cfg",
	"braquages.cfg",
	"nouveautes.cfg",
	"mapping.cfg",
	"mappack.cfg",
	"basket_paniers.cfg"
};

forward AutoBackup_Tick();

stock AutoBackup_Init()
{
	AutoBackup_Run();
	SetTimer("AutoBackup_Tick", AUTOBACKUP_INTERVAL_MS, true);
	return 1;
}

public AutoBackup_Tick()
{
	return AutoBackup_Run();
}

stock AutoBackup_CopyFile(const srcName[], slot)
{
	if(!fexist(srcName)) return 0;

	new File:src = fopen(srcName, io_read);
	if(!src) return 0;

	new dstName[64];
	format(dstName, sizeof(dstName), "%s%s.slot%d", AUTOBACKUP_DIR, srcName, slot);

	if(fexist(dstName)) fremove(dstName);
	new File:dst = fopen(dstName, io_write);
	if(!dst)
	{
		fclose(src);
		return 0;
	}

	new line[256];
	while(fread(src, line))
	{
		fwrite(dst, line);
	}
	fclose(src);
	fclose(dst);
	return 1;
}

stock AutoBackup_Run()
{
	new year, month, day;
	getdate(year, month, day);
	new slot = day % 7;

	for(new i = 0; i < AUTOBACKUP_FILE_COUNT; i++)
	{
		AutoBackup_CopyFile(AutoBackup_Files[i], slot);
	}
	printf("[AUTOBACKUP] Sauvegarde de %d fichier(s) effectuee (emplacement %d)", AUTOBACKUP_FILE_COUNT, slot);
	return 1;
}
