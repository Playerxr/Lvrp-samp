// =====================================================================================
// SAMPVOICE INC - AFRP edition
// =====================================================================================
// [REECRIT] La precedente version de ce fichier inventait une API qui ne
// correspond a AUCUNE fonction reellement exportee par plugins/sampvoice.so
// (verifie avec `strings` sur le binaire installe) : "SvCreateGStream",
// "SvAttachSpeakerToStream", "SvIsPlayerSpeaking", "SvAddKey"... n'existent
// pas dans le vrai plugin. Consequence : chaque appel plantait silencieusement
// (native non enregistree) et rien de tout le systeme radio/telephone ne
// pouvait fonctionner, quel que soit le code ecrit par-dessus.
//
// Les vraies declarations ci-dessous viennent de sv_control/sampvoice.chm
// (l'aide livree avec CE binaire exact, extraite avec extract_chmLib), donc
// garanties correspondre a la version installee - pas d'une doc generique
// trouvee en ligne qui pourrait viser une autre version du plugin.
//
// === MODELE REEL DU PLUGIN (different de l'ancienne version fabriquee) ===
// - Chaque JOUEUR a 32 "canaux" (0-31) et deux roles : Listener (il entend
//   ce qui arrive aux streams auxquels il est abonne) et Speaker (son micro
//   peut alimenter des streams via ses canaux).
// - Un STREAM est un noeud de transit : le trafic audio des canaux Speaker
//   qui lui sont attaches (SvAttachStream) est diffuse a ses Listeners
//   (SvAttachListener). SvCreateStream(0.0) = stream global (frequence radio,
//   telephone...), une distance > 0 cree un stream local positionnel (3D).
// - SvPlay(playerid, channel) demarre reellement le micro sur ce canal
//   (commentaire officiel de la doc : "active le microphone et demarre la
//   diffusion") ; SvStop l'arrete. Pas de notion de touche d'activation
//   cote serveur necessaire pour un mode permanent comme /radio.
// =====================================================================================

#if defined SAMPVOICE_INC
    #endinput
#endif
#define SAMPVOICE_INC

// =====================================================================================
// Constantes generales
// =====================================================================================
#define SV_NULL        (0)
#define SV_NONE        (-1)
#define SV_FALSE       (0)
#define SV_TRUE        (1)

// =====================================================================================
// Natives - Info plugin client / roles joueur
// =====================================================================================
native SvGetVersion(playerid);                    // 0 si le joueur n'a pas le plugin, sinon version
native SvHasMicro(playerid);                       // SV_TRUE/SV_FALSE

native SvEnableListener(playerid);                 // Autorise le joueur a entendre les streams abonnes
native SvDisableListener(playerid);
native SvCheckListener(playerid);

native SvEnableSpeaker(playerid, ...);              // channels en plus (aucun = tous les canaux actifs)
native SvDisableSpeaker(playerid, ...);
native SvCheckSpeaker(playerid, ...);

// =====================================================================================
// Natives - Micro (demarrer/arreter la diffusion reelle sur un canal)
// =====================================================================================
native SvPlay(playerid, ...);                       // channels en plus - demarre le micro sur ces canaux
native SvStop(playerid, ...);                       // arrete le micro sur ces canaux

// =====================================================================================
// Natives - Streams
// =====================================================================================
native SvCreateStream(Float:distance = 0.0);        // 0.0 = stream global (2D), >0 = stream local (3D)
native SvDeleteStream(streamid);

native SvAttachStream(playerid, streamid, ...);     // relie les canaux (Speaker) du joueur a ce stream
native SvDetachStream(playerid, streamid, ...);

// ATTENTION : d'apres la doc officielle (attachlistener.htm), SvAttachListener
// n'est valable QUE pour les streams GLOBAUX. Pour un stream LOCAL (distance
// > 0), on n'abonne personne : tout joueur ayant le role Listener actif et se
// trouvant dans le rayon entend automatiquement. C'est ce qui rend la voix de
// proximite possible sans tenir soi-meme la liste des joueurs a portee.
native SvAttachListener(streamid, playerid);        // abonne le joueur (Listener) a ce stream global
native SvDetachListener(streamid, playerid);

// =====================================================================================
// Natives - Streams LOCAUX (positionnels)
// =====================================================================================
// Un stream cree avec une distance > 0 est local : il a une position dans le
// monde, et seuls les joueurs a portee l'entendent, avec le volume qui baisse
// avec l'eloignement. Un stream ne peut PAS passer de global a local ni
// l'inverse (createstream.htm) : il faut en creer un de chaque type.
//
// Deux facons de positionner un stream local :
//   - SvSetPosition : on donne les coordonnees a la main (a rafraichir).
//   - SvSetTarget   : on l'accroche a un joueur/vehicule/objet et il suit tout
//                     seul. VOLONTAIREMENT NON DECLARE ICI : sa cible doit etre
//                     fabriquee par la macro SvMakePlayer/SvMakeVehicle/
//                     SvMakeObject, dont la doc livree avec ce binaire ne donne
//                     PAS le contenu (elle montre "#define SvMakePlayer(index)"
//                     sans le corps). Inventer cette valeur ferait echouer la
//                     liaison en silence - exactement le piege qui a fait que
//                     toute la voix de ce serveur n'a jamais marche. On utilise
//                     donc SvSetPosition, dont la signature est connue avec
//                     certitude, quitte a rafraichir la position nous-memes.
native SvSetPosition(streamid, Float:x, Float:y, Float:z);
native SvSetDistance(streamid, Float:distance);     // change le rayon d'un stream local
