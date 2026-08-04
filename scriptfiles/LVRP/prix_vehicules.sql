-- ============================================================================
--  LVRP  -  GRILLE DE PRIX DES VEHICULES  (toutes concessions)
--  Table concernee : `lvrp_server_vehicles_prices`
--  Fichier genere le 2026-08-04
-- ============================================================================
--
--  A QUOI SERT CE FICHIER ?
--  ------------------------
--  Il remet a plat le PRIX de chaque vehicule vendu dans vos concessions,
--  sur la base de l'economie que vous avez fixee :
--
--        * prix le plus bas du serveur ......      5 000 000 $  (5 M$)
--        * prix le plus haut (jet prive) ... 1 000 000 000 $  (1 milliard)
--
--  CE QUE CE FICHIER FAIT :
--      - il change UNIQUEMENT la colonne `Price` ;
--      - il agit par modele (`WHERE Model = ...`), donc il corrige le prix
--        du vehicule DANS TOUTES LES CONCESSIONS ou vous le vendez, y compris
--        la 4e concession "Occasion" de Las Venturas.
--
--  CE QUE CE FICHIER NE FAIT PAS (c'est volontaire et c'est important) :
--      - il ne SUPPRIME rien (aucun DELETE, aucun TRUNCATE, aucun DROP) ;
--      - il ne decide pas QUELS vehicules vous vendez ;
--      - il ne decide pas DANS QUELLE concession ils sont vendus.
--    Votre catalogue reste exactement tel que vous l'avez compose. Si un
--    modele ci-dessous n'est pas dans votre catalogue, la ligne ne touche
--    simplement aucune donnee (0 ligne modifiee) : c'est sans effet et sans
--    danger.
--
--  RELANCABLE SANS RISQUE
--  ----------------------
--  Vous pouvez importer ce fichier autant de fois que vous voulez : il
--  reecrit toujours les memes prix. Deux executions donnent le meme resultat
--  qu'une seule.
--
--  AUCUNE RECOMPILATION NECESSAIRE
--  -------------------------------
--  Le gamemode lit les prix directement dans la base a chaque ouverture du
--  menu de concession. Les nouveaux prix sont actifs immediatement, sans
--  redemarrer le serveur et sans recompiler le .pwn.
--
--  COMMENT L'IMPORTER
--  ------------------
--  phpMyAdmin -> selectionnez votre base -> onglet "Importer" -> choisissez
--  ce fichier -> "Executer".
--
--  LIMITE TECHNIQUE A CONNAITRE
--  ----------------------------
--  La colonne `Price` est un entier signe 32 bits : le maximum absolu est
--  2 147 483 647 $. Le plafond de 1 milliard passe donc largement, mais ne
--  montez JAMAIS un prix au-dela de 2 milliards : le nombre "deborderait" et
--  deviendrait negatif (vehicule gratuit, voire qui rapporte de l'argent).
--
-- ============================================================================


-- ==========================================================================
-- SECTION 1  -  PRIX DES VEHICULES EN VENTE
-- ==========================================================================
--
--  Les titres de rubriques ci-dessous (Sport, Auto, 4x4...) sont indicatifs :
--  ils vous aident a vous reperer. Techniquement chaque ligne agit sur le
--  modele PARTOUT ou il est vendu. Si vous vendez le meme modele dans deux
--  concessions differentes, il aura le meme prix dans les deux -- c'est
--  normal et c'est ce qui evite les incoherences entre concessions.
--
--  Chaque ligne se lit ainsi :
--      UPDATE ... SET `Price` = <prix> WHERE `Model` = <numero>;  -- <Nom>
--  Le nom en commentaire est le nom EXACT affiche par votre serveur.
--

-- ==========================================================================
--  SPORTIVES  (concession Sport - dealerShip = 1)
--  Le haut du panier. De 15 M$ (kart de loisir) a 250 M$ (Infernus).
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  250000000 WHERE `Model` = 411;  -- Infernus  (supercar - haut de gamme)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  240000000 WHERE `Model` = 541;  -- Bullet  (supercar)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  220000000 WHERE `Model` = 451;  -- Turismo  (supercar)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  200000000 WHERE `Model` = 415;  -- Cheetah  (supercar)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  180000000 WHERE `Model` = 506;  -- Super GT  (GT)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  150000000 WHERE `Model` = 429;  -- Banshee
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 494;  -- Hotring  (voiture de course)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 502;  -- Hotring Racer A  (voiture de course)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 503;  -- Hotring Racer B  (voiture de course)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  100000000 WHERE `Model` = 480;  -- Comet
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   95000000 WHERE `Model` = 402;  -- Buffalo  (muscle car)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   90000000 WHERE `Model` = 477;  -- ZR-350  (sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   85000000 WHERE `Model` = 603;  -- Phoenix  (muscle car)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   80000000 WHERE `Model` = 602;  -- Alpha  (coupe sportif)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   75000000 WHERE `Model` = 562;  -- Elegy  (sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   70000000 WHERE `Model` = 559;  -- Jester  (sportive japonaise)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   70000000 WHERE `Model` = 560;  -- Sultan  (berline sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   65000000 WHERE `Model` = 587;  -- Euros  (sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 533;  -- Feltzer  (cabriolet sportif)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 555;  -- Windsor  (cabriolet de luxe)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 565;  -- Flash  (sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 434;  -- Hotknife  (hot rod)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 589;  -- Club  (sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   15000000 WHERE `Model` = 571;  -- Kart  (kart de loisir - non homologue route)

-- ==========================================================================
--  BERLINES / CITADINES / LOWRIDERS  (concession Auto - dealerShip = 2)
--  Les voitures du quotidien et le luxe routier. De 11 M$ a 45 M$.
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   45000000 WHERE `Model` = 409;  -- Limousine  (limousine de luxe)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   40000000 WHERE `Model` = 580;  -- Stafford  (berline de luxe)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   30000000 WHERE `Model` = 558;  -- Uranus  (compacte sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   28000000 WHERE `Model` = 545;  -- Hustler  (hot rod classique)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   26000000 WHERE `Model` = 496;  -- Blista Compact  (coupe compact)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   24000000 WHERE `Model` = 475;  -- Sabre  (muscle car classique)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   23000000 WHERE `Model` = 534;  -- Remington  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 426;  -- Premier
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 535;  -- Slamvan  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 567;  -- Savanna  (lowrider cabriolet)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   21000000 WHERE `Model` = 507;  -- Elegant
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   21000000 WHERE `Model` = 536;  -- Blade  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 421;  -- Washington
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 566;  -- Tahoma  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   19000000 WHERE `Model` = 445;  -- Admiral
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   19000000 WHERE `Model` = 575;  -- Broadway  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 405;  -- Sentinel
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 439;  -- Stallion  (cabriolet classique)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 542;  -- Clover
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   17000000 WHERE `Model` = 526;  -- Fortune
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   17000000 WHERE `Model` = 551;  -- Merit
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   16000000 WHERE `Model` = 518;  -- Buccaneer
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   16000000 WHERE `Model` = 561;  -- Stratum  (break)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   16000000 WHERE `Model` = 576;  -- Tornado  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   15000000 WHERE `Model` = 517;  -- Majestic
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   14000000 WHERE `Model` = 540;  -- Vincent
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   14000000 WHERE `Model` = 585;  -- Emperor
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   13000000 WHERE `Model` = 458;  -- Solair  (break)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   13000000 WHERE `Model` = 516;  -- Nebula
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   13000000 WHERE `Model` = 546;  -- Intruder
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   13000000 WHERE `Model` = 550;  -- Sunrise
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 527;  -- Cadrona
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 547;  -- Primo
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   11000000 WHERE `Model` = 529;  -- Willard
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   11000000 WHERE `Model` = 549;  -- Tampa

-- ==========================================================================
--  4X4 & UTILITAIRES  (concession 4X4 & Utilitaires - dealerShip = 3)
--  Tout-terrain, camions, engins de chantier et vehicules de travail. De 5 M$ a 120 M$.
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 444;  -- Monster  (monster truck)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 556;  -- Monster  (monster truck)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  120000000 WHERE `Model` = 557;  -- Monster  (monster truck)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   90000000 WHERE `Model` = 495;  -- Sandking  (4x4 sureleve)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   80000000 WHERE `Model` = 406;  -- Dumper  (engin de chantier)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   70000000 WHERE `Model` = 486;  -- Dozer  (bulldozer)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   65000000 WHERE `Model` = 515;  -- Roadtrain  (semi-remorque)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 443;  -- Packer  (porte-voitures)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 514;  -- Tanker  (tracteur citerne)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 403;  -- Linerunner  (tracteur routier)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 455;  -- Flatbed  (camion plateau)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 579;  -- Huntley  (4x4 de luxe)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   50000000 WHERE `Model` = 437;  -- Coach  (autocar)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   45000000 WHERE `Model` = 431;  -- Bus  (bus de ville)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   45000000 WHERE `Model` = 524;  -- Cement Truck  (toupie a beton)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   45000000 WHERE `Model` = 573;  -- Dune  (buggy des dunes)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   40000000 WHERE `Model` = 532;  -- Combine  (moissonneuse)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   35000000 WHERE `Model` = 408;  -- Trashmaster  (benne a ordures)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   32000000 WHERE `Model` = 578;  -- DFT-30  (camion)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   30000000 WHERE `Model` = 400;  -- Landstalker  (4x4 familial)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   30000000 WHERE `Model` = 456;  -- Yankee  (camion)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   30000000 WHERE `Model` = 508;  -- Journey  (mobil-home)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   28000000 WHERE `Model` = 414;  -- Mule  (camion de livraison)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   28000000 WHERE `Model` = 424;  -- BF Injection  (buggy tout-terrain)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   28000000 WHERE `Model` = 525;  -- Depanneuse  (depanneuse)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   26000000 WHERE `Model` = 483;  -- Camper  (camping-car)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   26000000 WHERE `Model` = 499;  -- Benson  (camion)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   26000000 WHERE `Model` = 582;  -- San News Van  (fourgon de presse)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   25000000 WHERE `Model` = 489;  -- 4x4  (4x4 (Rancher))
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   25000000 WHERE `Model` = 505;  -- 4x4  (4x4 (variante))
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   25000000 WHERE `Model` = 568;  -- Bandito  (buggy)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   24000000 WHERE `Model` = 498;  -- Boxvillde  (fourgon de livraison)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   24000000 WHERE `Model` = 552;  -- Utility  (camion nacelle)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   24000000 WHERE `Model` = 554;  -- Jeep  (pick-up 4x4)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   24000000 WHERE `Model` = 609;  -- Boxville  (fourgon de livraison)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 442;  -- Romero  (corbillard)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 500;  -- Mesa  (4x4)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   22000000 WHERE `Model` = 588;  -- Hotdog  (camion snack)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 423;  -- Whoopee  (camion de glaces)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 482;  -- Burrito  (fourgon)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 413;  -- Pony  (fourgon)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 459;  -- Berkleys RC Van  (fourgon)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   17000000 WHERE `Model` = 440;  -- Rumpo  (fourgon)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   16000000 WHERE `Model` = 422;  -- Bobcat  (pick-up)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   15000000 WHERE `Model` = 420;  -- Taxi  (vehicule de service - voir note taxis)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   15000000 WHERE `Model` = 531;  -- Tracteur  (tracteur agricole)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 438;  -- Cabbie  (vehicule de service - voir note taxis)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 543;  -- Sadler  (pick-up)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 574;  -- Sweeper  (balayeuse de voirie)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 600;  -- Picador  (pick-up)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   10000000 WHERE `Model` = 530;  -- Forklift  (chariot elevateur)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    8000000 WHERE `Model` = 457;  -- Caddy  (voiturette de golf)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    8000000 WHERE `Model` = 583;  -- Tug  (tracteur de piste aeroport)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    6000000 WHERE `Model` = 485;  -- Baggage  (chariot a bagages)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 572;  -- Mower  (tondeuse autoportee - PRIX PLANCHER)

-- ==========================================================================
--  DEUX ROUES  (concession 2 roues - dealerShip = 4)
--  Velos, scooters, motos. De 5 M$ a 25 M$ : volontairement sous le prix des voitures.
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   25000000 WHERE `Model` = 522;  -- NRG-500  (moto la plus rapide)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 521;  -- FCR-900  (moto sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   19000000 WHERE `Model` = 581;  -- BF-400  (moto routiere)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   18000000 WHERE `Model` = 461;  -- PCJ-600  (moto sportive)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   17000000 WHERE `Model` = 586;  -- Wayfarer  (custom)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   16000000 WHERE `Model` = 468;  -- Sanchez  (moto tout-terrain)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   15000000 WHERE `Model` = 463;  -- Freeway  (custom)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   12000000 WHERE `Model` = 471;  -- Quad  (quad)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 448;  -- Pizzaboy  (scooter de livraison - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 462;  -- Scooteur  (scooter - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 481;  -- BMX  (velo - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 509;  -- Velo  (velo - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 510;  -- VTT  (VTT - PRIX PLANCHER)

-- ==========================================================================
--  BATEAUX  (concession Bateau - dealerShip = 5)
--  Du pneumatique au yacht. De 20 M$ a 100 M$.
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  100000000 WHERE `Model` = 493;  -- Jetmax  (vedette haut de gamme)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   90000000 WHERE `Model` = 595;  -- Launch  (yacht)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   80000000 WHERE `Model` = 484;  -- Marquis  (voilier)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   70000000 WHERE `Model` = 452;  -- Speeder  (vedette)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   60000000 WHERE `Model` = 446;  -- Squalo  (vedette rapide)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 454;  -- Tropic  (vedette de plaisance)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   55000000 WHERE `Model` = 539;  -- Vortex  (aeroglisseur (amphibie))
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   40000000 WHERE `Model` = 453;  -- Reefer  (bateau de peche)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =   20000000 WHERE `Model` = 473;  -- Dinghy  (petit pneumatique - entree de gamme bateau)

-- ==========================================================================
--  AVIONS & HELICOPTERES  (concession Avion - dealerShip = 6)
--  La bande la plus chere du serveur. De 150 M$ (Dodo) a 1 milliard (Shamal, le jet prive).
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` = 1000000000 WHERE `Model` = 519;  -- Shamal  (JET PRIVE - PRIX PLAFOND (1 milliard))
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  800000000 WHERE `Model` = 577;  -- AT-400  (avion de ligne gros porteur)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  450000000 WHERE `Model` = 548;  -- Cargobob  (helicoptere cargo lourd)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  400000000 WHERE `Model` = 417;  -- Leviathan  (helicoptere de levage lourd)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  350000000 WHERE `Model` = 553;  -- Nevada  (avion cargo)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  320000000 WHERE `Model` = 513;  -- Stunt  (avion de voltige)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  300000000 WHERE `Model` = 511;  -- Beagle  (bimoteur)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  260000000 WHERE `Model` = 512;  -- Cropduster  (avion agricole)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  260000000 WHERE `Model` = 563;  -- Raindance  (helicoptere utilitaire)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  240000000 WHERE `Model` = 488;  -- News Chopper  (helicoptere de presse)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  220000000 WHERE `Model` = 487;  -- Maverick  (helicoptere)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  200000000 WHERE `Model` = 460;  -- Skimmer  (hydravion)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  180000000 WHERE `Model` = 469;  -- Sparrow  (helicoptere leger)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =  150000000 WHERE `Model` = 593;  -- Dodo  (petit avion - entree de gamme aviation)

-- ==========================================================================
--  OCCASIONS  (concession Occasion - dealerShip = 7, Las Venturas)
--  Les vehicules d'entree de gamme. C'est ici que vit le prix plancher de 5 M$.
-- ==========================================================================
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    9000000 WHERE `Model` = 412;  -- Voodoo  (lowrider)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    7500000 WHERE `Model` = 436;  -- Previon
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    7500000 WHERE `Model` = 504;  -- Bloodring Banger  (voiture de derby)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    7000000 WHERE `Model` = 474;  -- Hermes
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    6500000 WHERE `Model` = 419;  -- Esperanto
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    6500000 WHERE `Model` = 491;  -- Virgo
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    6000000 WHERE `Model` = 401;  -- Bravura
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    6000000 WHERE `Model` = 492;  -- Greenwood
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5500000 WHERE `Model` = 404;  -- Perrenial
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5500000 WHERE `Model` = 418;  -- Moonbeam
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5500000 WHERE `Model` = 467;  -- Oceanic
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5500000 WHERE `Model` = 479;  -- Regina
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 410;  -- Manana  (PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 466;  -- Glendale  (PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 478;  -- Walton  (vieux pick-up - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 604;  -- Glendale  (epave - PRIX PLANCHER)
UPDATE `lvrp_server_vehicles_prices` SET `Price` =    5000000 WHERE `Model` = 605;  -- Sadler  (epave - PRIX PLANCHER)


-- ==========================================================================
--  FILET DE SECURITE  -  aucun vehicule ne peut rester sous le plancher
-- ==========================================================================
--
--  A LIRE : c'est la ligne qui repare la 4e concession "Occasion".
--
--  Des prix errones (de l'ordre de 12 000 $ a 30 000 $) ont ete inseres par
--  erreur dans la concession Occasion (dealerShip = 7). Les lignes de la
--  section 1 corrigent deja tous les modeles listes ci-dessus, dans toutes
--  les concessions, cette 4e comprise.
--
--  La requete ci-dessous est une securite supplementaire : elle remonte
--  automatiquement a 5 000 000 $ TOUT vehicule encore vendu sous le plancher,
--  meme un modele que je n'aurais pas liste, meme dans une concession creee
--  plus tard. Elle ne touche pas les lignes a 0 (0 = vehicule masque, voir
--  la section "Comment ajuster" plus bas).
--
UPDATE `lvrp_server_vehicles_prices`
   SET `Price` = 5000000
 WHERE `Price` > 0
   AND `Price` < 5000000;

--  Variante ciblee sur la seule concession Occasion (dealerShip = 7).
--  Elle est REDONDANTE avec la requete ci-dessus (qui couvre deja toutes les
--  concessions) : elle est fournie pour que vous puissiez, si besoin, ne
--  traiter que cette concession-la. Retirez les deux tirets pour l'activer.
--
-- UPDATE `lvrp_server_vehicles_prices`
--    SET `Price` = 5000000
--  WHERE `dealerShip` = 7
--    AND `Price` > 0
--    AND `Price` < 5000000;


-- ==========================================================================
-- SECTION 2  -  VEHICULES A NE PAS METTRE EN VENTE
-- ==========================================================================
--
--  Les modeles ci-dessous ne devraient jamais etre achetables par un joueur
--  dans une concession : vehicules de secours, de police, vehicules armes,
--  militaires, trains et remorques.
--
--  ILS NE SONT PAS DANS LA SECTION 1 : je ne leur ai donc attribue aucun
--  prix. Si vous ne les vendez pas (le cas normal), vous n'avez rien a faire,
--  cette section est purement informative.
--
--  SI VOUS EN VENDEZ UN PAR ERREUR, comment le retirer du catalogue ?
--  ------------------------------------------------------------------
--  Le gamemode masque automatiquement tout vehicule dont le prix vaut 0 :
--  il n'apparait plus dans le menu de la concession. Mettre `Price` = 0 est
--  donc la facon prevue de cacher un vehicule sans rien supprimer.
--
--  !!!  AVERTISSEMENT IMPORTANT  -  A LIRE AVANT D'ACTIVER CES LIGNES  !!!
--  ----------------------------------------------------------------------
--  Le prix 0 masque bien le vehicule dans les concessions, MAIS l'achat de
--  vehicules par les FACTIONS utilise la meme colonne `Price` et, lui, ne
--  ignore pas le 0 : une faction pourrait alors obtenir le vehicule
--  GRATUITEMENT. Si vos factions (police, pompiers...) achetent leurs
--  vehicules via la commande d'achat faction, N'ACTIVEZ PAS les lignes
--  "Price = 0" ci-dessous : utilisez plutot la METHODE RECOMMANDEE indiquee
--  juste apres, qui masque le vehicule des concessions tout en lui laissant
--  un vrai prix pour les factions.
--
--  METHODE RECOMMANDEE (masquer des concessions SANS casser l'achat faction)
--  ------------------------------------------------------------------------
--  Vos concessions portent un numero de 1 a 7. Une ligne rangee sur le
--  numero 0 n'appartient donc a aucune concession : elle disparait de tous
--  les menus, mais garde son prix pour les achats de faction.
--  Pour l'activer, retirez les deux tirets des 3 lignes suivantes :
--
-- UPDATE `lvrp_server_vehicles_prices`
--    SET `dealerShip` = 0
--  WHERE `Model` IN (
--        407,416,425,427,428,430,432,433,435,441,447,449,
--        450,464,465,470,472,476,490,497,501,520,523,528,
--        537,538,544,564,569,570,584,590,591,592,594,596,
--        597,598,599,601,606,607,608,610,611);
--
--  Et en dernier recours, la methode "prix 0" (relisez l'avertissement
--  ci-dessus avant de vous en servir) : retirez les deux tirets des lignes
--  qui vous interessent.
--

--  >> Vehicules de secours (pompiers, ambulance, garde-cotes)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 407;  -- Camion LSFD : camion de pompiers
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 416;  -- Ambulance : ambulance
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 472;  -- Coastguard : garde-cotes / sauvetage
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 544;  -- Camion LSFD : camion de pompiers (2e modele)

--  >> Vehicules de police / FBI / SWAT
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 427;  -- Enforcer : fourgon d'intervention / SWAT
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 490;  -- 4x4 : 4x4 du FBI - ATTENTION : porte le meme nom "4x4" que le 489
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 497;  -- Helico LSPD : helicoptere de police
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 523;  -- HPV1000 : moto de police
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 528;  -- Truck : fourgon du FBI - le nom "Truck" prete a confusion
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 596;  -- LSPD : voiture de police Los Santos
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 597;  -- SFPD : voiture de police San Fierro
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 598;  -- LVPD : voiture de police Las Venturas
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 599;  -- Police 4x4 : 4x4 de police
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 601;  -- S.W.A.T : fourgon S.W.A.T.

--  >> Vehicules ARMES sous SA-MP (mitrailleuses ou missiles fonctionnels)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 425;  -- Hunter : helicoptere de combat (mitrailleuses/roquettes actives sous SA-MP)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 430;  -- Predator : bateau de police (arme sous SA-MP)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 432;  -- Rhino : char d'assaut
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 447;  -- Seasparrow : helicoptere arme (mitrailleuses actives sous SA-MP)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 476;  -- Rustler : avion de chasse arme (mitrailleuses actives sous SA-MP)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 520;  -- Hydra : jet de combat arme (missiles actifs sous SA-MP)

--  >> Vehicules militaires
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 433;  -- Barracks : camion militaire
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 470;  -- Patriot : 4x4 militaire
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 592;  -- Andromada : avion cargo militaire (vehicule de mission, ingerable en ville)

--  >> Vehicules de mission / scenario
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 428;  -- Securicar : convoyeur de fonds - vehicule de braquage/mission

--  >> Vehicules radiocommandes (miniatures : un joueur ne peut pas les piloter normalement)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 441;  -- RC Bandit : voiture radiocommandee - non conduisible par un joueur
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 464;  -- RC Baron : avion radiocommande
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 465;  -- RC Raider : helicoptere radiocommande
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 501;  -- RC Goblin : helicoptere radiocommande
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 564;  -- RC Tiger : char radiocommande
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 594;  -- RC Cam : camera radiocommandee

--  >> Trains et tramways (circulent sur rails, ne se pilotent pas comme une voiture)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 449;  -- Tram : tramway - circule sur rails uniquement
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 537;  -- Freight : locomotive - circule sur rails uniquement
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 538;  -- Streak : locomotive - circule sur rails uniquement
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 569;  -- Freight Flat : wagon de fret
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 570;  -- Streak Carriage : wagon de passagers
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 590;  -- Freight Box : wagon de marchandises

--  >> Remorques (elles se tractent, elles ne se conduisent pas : les vendre n'a pas de sens)
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 435;  -- Trailer : remorque - se tracte, ne se conduit pas
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 450;  -- Trailer : remorque
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 584;  -- Trailer : remorque citerne
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 591;  -- Trailer : remorque
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 606;  -- Luggage : remorque a bagages
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 607;  -- Luggage : remorque a bagages
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 608;  -- Stairs : passerelle d'avion
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 610;  -- Tiller : remorque agricole
-- UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 611;  -- Utility Trailer : remorque utilitaire


--  Deux noms a signaler, parce qu'ils pretent a confusion dans votre liste :
--    - le modele 490 s'appelle "4x4" comme le 489, mais c'est le 4x4 du FBI ;
--      c'est le 489 (et le 505) qui sont les 4x4 civils, mis en vente.
--    - le modele 528 s'appelle simplement "Truck", mais c'est le fourgon du
--      FBI, pas un camion civil.


-- ==========================================================================
-- SECTION 3  -  (OPTIONNEL) AJOUTER DES VEHICULES A UNE CONCESSION
-- ==========================================================================
--
--  !!!  SECTION DESACTIVEE PAR DEFAUT  -  NE FAIT RIEN EN L'ETAT  !!!
--
--  Tout ce qui suit est en commentaire. Vous pouvez importer ce fichier sans
--  rien craindre : cette section ne s'executera pas.
--
--  A QUOI CA SERT ?
--  Les sections precedentes ne font que CORRIGER LE PRIX de ce que vous
--  vendez deja. Si vous souhaitez ELARGIR le catalogue d'une concession,
--  vous trouverez ci-dessous une ligne prete a l'emploi pour chaque vehicule,
--  rangee dans la concession qui lui correspond.
--
--  ATTENTION, TROIS POINTS DE VIGILANCE :
--
--   1. Ces lignes AJOUTENT une entree au catalogue. Contrairement au reste
--      du fichier, elles ne sont pas relancables : les executer deux fois
--      cree le vehicule EN DOUBLE dans la concession.
--
--   2. Verifiez d'abord ce que vous vendez deja, pour ne pas creer de
--      doublon. Cette requete (sans danger, elle ne fait que lire) vous
--      donne la liste complete, concession par concession :
--
--          SELECT `dealerShip`, `Model`, `Price`
--            FROM `lvrp_server_vehicles_prices`
--           ORDER BY `dealerShip`, `Price` DESC;
--
--      Et pour verifier un modele precis avant de l'ajouter (exemple ici
--      avec le modele 411) :
--
--          SELECT * FROM `lvrp_server_vehicles_prices` WHERE `Model` = 411;
--
--   3. Ne surchargez pas une concession. Le menu d'achat affiche la liste
--      dans une fenetre de taille limitee : au-dela d'une soixantaine de
--      vehicules dans une meme concession, la fin de la liste risque d'etre
--      coupee et invisible pour les joueurs. Mieux vaut une selection courte
--      et lisible.
--
--  Pour activer une ligne : retirez les deux tirets au debut de cette ligne.
--

--  ----------------------------------------------------------------------
--  SPORTIVES  (concession Sport - dealerShip = 1)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,411,250000000);             -- Infernus
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,541,240000000);             -- Bullet
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,451,220000000);             -- Turismo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,415,200000000);             -- Cheetah
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,506,180000000);             -- Super GT
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,429,150000000);             -- Banshee
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,494,120000000);             -- Hotring
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,502,120000000);             -- Hotring Racer A
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,503,120000000);             -- Hotring Racer B
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,480,100000000);             -- Comet
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,402,95000000);              -- Buffalo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,477,90000000);              -- ZR-350
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,603,85000000);              -- Phoenix
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,602,80000000);              -- Alpha
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,562,75000000);              -- Elegy
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,559,70000000);              -- Jester
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,560,70000000);              -- Sultan
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,587,65000000);              -- Euros
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,533,60000000);              -- Feltzer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,555,60000000);              -- Windsor
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,565,60000000);              -- Flash
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,434,55000000);              -- Hotknife
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,589,55000000);              -- Club
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (1,571,15000000);              -- Kart

--  ----------------------------------------------------------------------
--  BERLINES / CITADINES / LOWRIDERS  (concession Auto - dealerShip = 2)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,409,45000000);              -- Limousine
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,580,40000000);              -- Stafford
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,558,30000000);              -- Uranus
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,545,28000000);              -- Hustler
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,496,26000000);              -- Blista Compact
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,475,24000000);              -- Sabre
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,534,23000000);              -- Remington
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,426,22000000);              -- Premier
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,535,22000000);              -- Slamvan
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,567,22000000);              -- Savanna
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,507,21000000);              -- Elegant
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,536,21000000);              -- Blade
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,421,20000000);              -- Washington
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,566,20000000);              -- Tahoma
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,445,19000000);              -- Admiral
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,575,19000000);              -- Broadway
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,405,18000000);              -- Sentinel
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,439,18000000);              -- Stallion
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,542,18000000);              -- Clover
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,526,17000000);              -- Fortune
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,551,17000000);              -- Merit
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,518,16000000);              -- Buccaneer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,561,16000000);              -- Stratum
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,576,16000000);              -- Tornado
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,517,15000000);              -- Majestic
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,540,14000000);              -- Vincent
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,585,14000000);              -- Emperor
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,458,13000000);              -- Solair
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,516,13000000);              -- Nebula
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,546,13000000);              -- Intruder
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,550,13000000);              -- Sunrise
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,527,12000000);              -- Cadrona
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,547,12000000);              -- Primo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,529,11000000);              -- Willard
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (2,549,11000000);              -- Tampa

--  ----------------------------------------------------------------------
--  4X4 & UTILITAIRES  (concession 4X4 & Utilitaires - dealerShip = 3)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,444,120000000);             -- Monster
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,556,120000000);             -- Monster
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,557,120000000);             -- Monster
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,495,90000000);              -- Sandking
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,406,80000000);              -- Dumper
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,486,70000000);              -- Dozer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,515,65000000);              -- Roadtrain
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,443,60000000);              -- Packer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,514,60000000);              -- Tanker
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,403,55000000);              -- Linerunner
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,455,55000000);              -- Flatbed
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,579,55000000);              -- Huntley
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,437,50000000);              -- Coach
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,431,45000000);              -- Bus
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,524,45000000);              -- Cement Truck
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,573,45000000);              -- Dune
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,532,40000000);              -- Combine
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,408,35000000);              -- Trashmaster
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,578,32000000);              -- DFT-30
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,400,30000000);              -- Landstalker
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,456,30000000);              -- Yankee
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,508,30000000);              -- Journey
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,414,28000000);              -- Mule
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,424,28000000);              -- BF Injection
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,525,28000000);              -- Depanneuse
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,483,26000000);              -- Camper
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,499,26000000);              -- Benson
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,582,26000000);              -- San News Van
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,489,25000000);              -- 4x4
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,505,25000000);              -- 4x4
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,568,25000000);              -- Bandito
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,498,24000000);              -- Boxvillde
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,552,24000000);              -- Utility
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,554,24000000);              -- Jeep
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,609,24000000);              -- Boxville
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,442,22000000);              -- Romero
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,500,22000000);              -- Mesa
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,588,22000000);              -- Hotdog
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,423,20000000);              -- Whoopee
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,482,20000000);              -- Burrito
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,413,18000000);              -- Pony
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,459,18000000);              -- Berkleys RC Van
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,440,17000000);              -- Rumpo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,422,16000000);              -- Bobcat
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,420,15000000);              -- Taxi
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,531,15000000);              -- Tracteur
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,438,12000000);              -- Cabbie
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,543,12000000);              -- Sadler
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,574,12000000);              -- Sweeper
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,600,12000000);              -- Picador
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,530,10000000);              -- Forklift
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,457,8000000);               -- Caddy
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,583,8000000);               -- Tug
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,485,6000000);               -- Baggage
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (3,572,5000000);               -- Mower

--  ----------------------------------------------------------------------
--  DEUX ROUES  (concession 2 roues - dealerShip = 4)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,522,25000000);              -- NRG-500
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,521,20000000);              -- FCR-900
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,581,19000000);              -- BF-400
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,461,18000000);              -- PCJ-600
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,586,17000000);              -- Wayfarer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,468,16000000);              -- Sanchez
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,463,15000000);              -- Freeway
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,471,12000000);              -- Quad
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,448,5000000);               -- Pizzaboy
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,462,5000000);               -- Scooteur
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,481,5000000);               -- BMX
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,509,5000000);               -- Velo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (4,510,5000000);               -- VTT

--  ----------------------------------------------------------------------
--  BATEAUX  (concession Bateau - dealerShip = 5)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,493,100000000);             -- Jetmax
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,595,90000000);              -- Launch
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,484,80000000);              -- Marquis
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,452,70000000);              -- Speeder
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,446,60000000);              -- Squalo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,454,55000000);              -- Tropic
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,539,55000000);              -- Vortex
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,453,40000000);              -- Reefer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (5,473,20000000);              -- Dinghy

--  ----------------------------------------------------------------------
--  AVIONS & HELICOPTERES  (concession Avion - dealerShip = 6)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,519,1000000000);            -- Shamal
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,577,800000000);             -- AT-400
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,548,450000000);             -- Cargobob
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,417,400000000);             -- Leviathan
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,553,350000000);             -- Nevada
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,513,320000000);             -- Stunt
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,511,300000000);             -- Beagle
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,512,260000000);             -- Cropduster
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,563,260000000);             -- Raindance
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,488,240000000);             -- News Chopper
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,487,220000000);             -- Maverick
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,460,200000000);             -- Skimmer
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,469,180000000);             -- Sparrow
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (6,593,150000000);             -- Dodo

--  ----------------------------------------------------------------------
--  OCCASIONS  (concession Occasion - dealerShip = 7, Las Venturas)
--  ----------------------------------------------------------------------
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,412,9000000);               -- Voodoo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,436,7500000);               -- Previon
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,504,7500000);               -- Bloodring Banger
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,474,7000000);               -- Hermes
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,419,6500000);               -- Esperanto
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,491,6500000);               -- Virgo
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,401,6000000);               -- Bravura
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,492,6000000);               -- Greenwood
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,404,5500000);               -- Perrenial
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,418,5500000);               -- Moonbeam
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,467,5500000);               -- Oceanic
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,479,5500000);               -- Regina
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,410,5000000);               -- Manana
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,466,5000000);               -- Glendale
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,478,5000000);               -- Walton
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,604,5000000);               -- Glendale
-- INSERT INTO `lvrp_server_vehicles_prices` (`dealerShip`,`Model`,`Price`) VALUES (7,605,5000000);               -- Sadler

--  Note sur les taxis : les modeles 420 (Taxi) et 438 (Cabbie) figurent dans
--  la rubrique "4X4 & Utilitaires" ci-dessus. Leur prix est corrige par la
--  section 1 au cas ou vous les vendriez deja, mais reflechissez avant de les
--  AJOUTER a une concession : si votre serveur a un metier de chauffeur de
--  taxi, laisser les joueurs acheter un taxi personnel peut vider ce metier
--  de son interet.


-- ==========================================================================
-- SECTION 4  -  COMMENT AJUSTER  (mode d'emploi)
-- ==========================================================================
--
--  MODIFIER LE PRIX D'UN SEUL VEHICULE
--  -----------------------------------
--  Retrouvez la ligne du vehicule dans la section 1 (les noms sont en
--  commentaire au bout de chaque ligne), changez le nombre, et reimportez le
--  fichier. Exemple, pour passer l'Infernus a 300 millions :
--
--      UPDATE `lvrp_server_vehicles_prices` SET `Price` = 300000000
--       WHERE `Model` = 411;
--
--  Ecrivez le prix sans espaces ni points : 300000000, pas 300 000 000.
--
--  LE FAIRE DIRECTEMENT EN JEU (le plus simple au quotidien)
--  ---------------------------------------------------------
--  Un administrateur de niveau 3 ou plus peut changer un prix sans toucher a
--  la base, avec la commande :
--
--      /a veprice <numero du modele> <prix>
--
--  Exemple : /a veprice 411 300000000
--  L'effet est immediat et vaut pour toutes les concessions, exactement comme
--  les lignes de ce fichier.
--
--  MASQUER UN VEHICULE (le retirer de la vente sans rien supprimer)
--  ----------------------------------------------------------------
--  Mettez son prix a 0 : le gamemode saute automatiquement les vehicules a 0
--  et ne les affiche plus dans le menu de la concession.
--
--      UPDATE `lvrp_server_vehicles_prices` SET `Price` = 0 WHERE `Model` = 411;
--
--  Pour le remettre en vente, il suffit de lui redonner un prix.
--  ATTENTION : relisez l'avertissement de la section 2 sur l'achat par les
--  factions avant d'utiliser le prix 0 sur un vehicule de faction.
--
--  DEPLACER UN VEHICULE D'UNE CONCESSION A UNE AUTRE
--  -------------------------------------------------
--  Les numeros de concession sont : 1 = Sport, 2 = Auto,
--  3 = 4X4 & Utilitaires, 4 = 2 roues, 5 = Bateau, 6 = Avion, 7 = Occasion.
--
--      UPDATE `lvrp_server_vehicles_prices` SET `dealerShip` = 7
--       WHERE `Model` = 411;
--
--  REIMPORTER LE FICHIER
--  ---------------------
--  Sans risque, autant de fois que vous voulez : les sections 2 et 3 sont
--  desactivees, et les sections 1 et 4 se contentent de reecrire les memes
--  prix. Pensez tout de meme a faire une sauvegarde de votre base avant tout
--  import (phpMyAdmin -> onglet "Exporter"), c'est le bon reflexe.
--
--  VERIFIER QUE TOUT EST BON APRES L'IMPORT
--  ----------------------------------------
--  Cette requete ne modifie rien, elle affiche juste vos prix du moins cher
--  au plus cher. Le premier de la liste ne doit pas etre sous 5 000 000 $ :
--
--      SELECT `dealerShip`, `Model`, `Price`
--        FROM `lvrp_server_vehicles_prices`
--       WHERE `Price` > 0
--       ORDER BY `Price` ASC;
--
-- ==========================================================================
-- FIN DU FICHIER
-- ==========================================================================
