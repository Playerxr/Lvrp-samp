# Bot Discord — validation des candidatures whitelist

Ce petit programme affiche les candidatures des nouveaux joueurs dans un salon
Discord, avec deux boutons **Accepter** et **Refuser**. Quand un membre du staff
clique, le joueur est mis en jeu (ou refusé, avec le motif) **automatiquement**,
sans que personne ait besoin d'être connecté au serveur de jeu.

Ce document est écrit pour être suivi pas à pas, sans connaissances techniques.
Compte environ 30 minutes la première fois.

---

## Comment ça marche (en une phrase)

Le serveur de jeu écrit les candidatures dans la base de données ; ce bot les
lit, les affiche sur Discord, et réécrit la décision dans la même base ; le
serveur de jeu relit cette décision toutes les 12 secondes et l'applique.

**Le serveur de jeu et le bot ne se parlent jamais directement.** C'est
volontaire : le serveur SA-MP est techniquement incapable de contacter Discord,
et ajouter un module supplémentaire dans le serveur est risqué (une erreur et le
serveur ne démarre plus). En passant par la base de données, le bot peut tourner
**n'importe où** — et si un jour tu veux le remplacer, le serveur de jeu n'aura
pas une ligne à changer.

---

## Étape 1 — Créer le bot sur Discord

1. Va sur <https://discord.com/developers/applications> et connecte-toi.
2. Clique **New Application** en haut à droite, donne un nom
   (par exemple `LVRP Whitelist`), accepte, puis **Create**.
3. Dans le menu de gauche, clique **Bot**.
4. Clique **Reset Token**, confirme, puis **Copy**.
   → C'est le **token**. Colle-le tout de suite quelque part, il ne s'affichera
   plus jamais. **Ce token est un mot de passe : ne le montre à personne, ne le
   mets jamais sur Discord, GitHub ou dans une capture d'écran.**
   (Si tu le perds ou s'il fuite : reviens ici et clique à nouveau
   **Reset Token**, l'ancien devient aussitôt inutilisable.)
5. Toujours dans la page **Bot**, plus bas, tu peux désactiver
   **Public Bot** (ça évite que d'autres personnes puissent l'inviter).
   Tu n'as besoin d'activer **aucun** « Privileged Gateway Intent ».

## Étape 2 — Inviter le bot sur ton serveur Discord

1. Menu de gauche → **OAuth2** → **URL Generator**.
2. Dans **Scopes**, coche uniquement : `bot`.
3. Dans **Bot Permissions**, coche :
   - **View Channels**
   - **Send Messages**
   - **Embed Links**
   - **Read Message History**
4. Copie le lien généré tout en bas, colle-le dans ton navigateur, choisis ton
   serveur Discord, et valide.

## Étape 3 — Récupérer l'identifiant du salon

1. Dans Discord : **Paramètres utilisateur** → **Avancés** → active
   **Mode développeur**.
2. Crée (ou choisis) un salon réservé au staff, par exemple `#candidatures`.
3. Clic droit sur ce salon → **Copier l'identifiant du salon**.
   → C'est le **salonCandidatures**. C'est une longue suite de chiffres.
4. Vérifie que le bot voit bien ce salon (il doit apparaître dans la liste des
   membres du salon). Vérifie aussi que **les joueurs normaux n'y ont pas accès** :
   les candidatures contiennent l'âge réel des gens.
5. *(Facultatif mais recommandé)* Clic droit sur ton rôle staff dans la liste des
   membres → **Copier l'identifiant du rôle**. → C'est le **roleStaff**. Si tu le
   remplis, seules les personnes ayant ce rôle pourront cliquer sur les boutons.

## Étape 4 — Récupérer les accès à la base de données

Ce sont **exactement les mêmes** que ceux du serveur de jeu. Tu les trouves dans
le fichier `server.cfg` de ton serveur SA-MP (lignes commençant par `mysql_`),
ou dans le panneau de ton hébergeur :

- **host** : l'adresse du serveur MySQL
- **port** : `3306` dans l'immense majorité des cas
- **user** : le nom d'utilisateur
- **password** : le mot de passe
- **database** : le nom de la base (souvent quelque chose comme `lvrp`)

> ⚠️ Si le bot ne tourne **pas** sur la même machine que le serveur de jeu, il
> faut demander à ton hébergeur d'**autoriser les connexions MySQL depuis
> l'extérieur** pour l'adresse IP où tourne le bot. C'est la seule question
> technique à leur poser, et c'est une demande banale.

## Étape 5 — Créer la table dans la base

Le serveur de jeu crée la table tout seul à son prochain redémarrage, donc en
principe tu n'as rien à faire. Si jamais ça ne marche pas, ouvre phpMyAdmin (ou
l'outil de base de données de ton hébergeur), onglet **SQL**, et colle ceci :

```sql
CREATE TABLE IF NOT EXISTS `lvrp_whitelist` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `SQLid` int(11) NOT NULL DEFAULT '0',
  `Name` varchar(32) NOT NULL DEFAULT '',
  `NomRP` varchar(32) NOT NULL DEFAULT '',
  `AgeRP` smallint(6) NOT NULL DEFAULT '0',
  `Background` varchar(255) NOT NULL DEFAULT '',
  `AgeReel` smallint(6) NOT NULL DEFAULT '0',
  `Source` varchar(80) NOT NULL DEFAULT '',
  `Etape` smallint(6) NOT NULL DEFAULT '1',
  `Statut` smallint(6) NOT NULL DEFAULT '-1',
  `Motif` varchar(128) NOT NULL DEFAULT '',
  `DecidePar` varchar(64) NOT NULL DEFAULT '',
  `Poste` smallint(6) NOT NULL DEFAULT '0',
  `Traite` smallint(6) NOT NULL DEFAULT '0',
  `DateSoumission` int(11) NOT NULL DEFAULT '0',
  `DateDecision` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `SQLid` (`SQLid`),
  KEY `Statut` (`Statut`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1;
```

## Étape 6 — Installer et lancer le bot

Il faut une machine avec **Node.js version 18 ou plus**
(<https://nodejs.org> — prends la version « LTS »). Ça peut être un VPS, un
serveur dédié, ou même ton PC pour tester.

1. Copie le dossier `discord_bot/` sur cette machine.
2. Dans ce dossier, duplique `config.example.json` et renomme la copie
   `config.json`.
3. Ouvre `config.json` avec un éditeur de texte et remplis les cases vides :

```json
{
  "discord": {
    "token": "le token copié à l'étape 1",
    "salonCandidatures": "l'identifiant du salon copié à l'étape 3",
    "roleStaff": "l'identifiant du rôle staff, ou laisse vide"
  },
  "mysql": {
    "host": "adresse de la base",
    "port": 3306,
    "user": "utilisateur",
    "password": "mot de passe",
    "database": "nom de la base"
  },
  "options": {
    "intervalleSecondes": 15,
    "alerteAgeReelMin": 16
  }
}
```

   - `intervalleSecondes` : à quelle fréquence le bot va chercher les nouvelles
     candidatures. 15 secondes est un bon réglage.
   - `alerteAgeReelMin` : si un candidat déclare un âge réel inférieur à ce
     nombre, sa fiche est affichée en orange avec un ⚠️. **Le bot ne refuse
     jamais tout seul** : c'est toi qui décides, il te le signale simplement.

4. Ouvre un terminal dans ce dossier et tape :

```bash
npm install
npm start
```

Si tout va bien, tu vois s'afficher `[OK] Connecte a Discord en tant que ...`.

> `config.json` contient le mot de passe de ta base et le token du bot :
> ne l'envoie à personne et ne le mets jamais sur GitHub.

## Étape 7 — Le faire tourner en permanence

`npm start` s'arrête dès que tu fermes le terminal. Pour qu'il tourne 24h/24 :

**Solution simple — pm2** (fonctionne sur Windows et Linux) :

```bash
npm install -g pm2
pm2 start index.js --name whitelist
pm2 save
pm2 startup      # affiche une commande à recopier pour un démarrage automatique
```

Ensuite `pm2 logs whitelist` montre ce que fait le bot, et
`pm2 restart whitelist` le relance.

**Solution Linux « propre » — systemd** : demande à ton hébergeur de créer un
service qui lance `node /chemin/vers/discord_bot/index.js` au démarrage. C'est
la même chose que pm2, géré par le système.

---

## Utilisation au quotidien

- Un joueur se connecte, remplit les 5 questions en jeu, et attend.
- Quelques secondes plus tard, sa fiche apparaît dans le salon `#candidatures`.
- Un membre du staff clique **Accepter** → le joueur apparaît en jeu tout seul,
  sans se reconnecter.
- Ou **Refuser** → une petite fenêtre demande le motif. Le joueur le reçoit,
  son compte est supprimé, et il peut retenter sa chance plus tard avec un
  meilleur dossier.
- Le message dans Discord est réécrit pour afficher la décision et le nom de la
  personne qui l'a prise : le salon reste lisible.

**Si le bot est en panne, tu n'es pas bloquée.** Les commandes en jeu
`/attente`, `/approuver [id]` et `/refuser [id] [raison]` fonctionnent toujours
exactement comme avant, et le bot en tient compte (il ne reposte pas un dossier
déjà traité en jeu).

---

## En cas de problème

| Symptôme | Cause la plus probable |
|---|---|
| `config.json introuvable` | Tu as oublié de renommer `config.example.json`. |
| `Connexion a Discord impossible` | Le token est faux, ou il a été régénéré. |
| `Salon des candidatures introuvable` | Mauvais identifiant de salon, ou le bot n'a pas accès au salon. |
| `Lecture des candidatures impossible` | Accès MySQL faux, ou connexions extérieures non autorisées (voir étape 4). |
| Le bot démarre mais rien ne s'affiche | Normal s'il n'y a aucune candidature en attente. Vérifie dans la base : il faut une ligne avec `Statut = 0`. |
| Le joueur reste bloqué après « Accepter » | Vérifie que le serveur de jeu tourne bien avec la dernière version du gamemode : c'est lui qui relit la décision. |

---

## Le contrat entre le serveur de jeu et le bot

*(section technique — utile seulement si quelqu'un doit un jour remplacer ce bot)*

Tout passe par la table `lvrp_whitelist`. **Rien d'autre.** Aucun port réseau,
aucune API, aucun fichier partagé.

| Colonne | Écrite par | Signification |
|---|---|---|
| `SQLid`, `Name`, `NomRP`, `AgeRP`, `Background`, `AgeReel`, `Source`, `Etape` | serveur de jeu | Le dossier, rempli question par question. |
| `Statut` | les deux | `-1` brouillon (formulaire en cours, **à ignorer**) · `0` en attente · `1` accepté · `2` refusé. |
| `Poste` | le bot | `1` = déjà publié sur Discord. |
| `Motif`, `DecidePar`, `DateDecision` | le bot (ou le staff en jeu) | La décision. |
| `Traite` | serveur de jeu **uniquement** | `1` = décision déjà appliquée en jeu. Un bot ne doit **jamais** y toucher. |

Ce que doit faire un bot, quel qu'il soit :

1. `SELECT ... FROM lvrp_whitelist WHERE Statut = 0 AND Poste = 0` — les
   dossiers à publier.
2. Réserver la ligne avant de publier :
   `UPDATE lvrp_whitelist SET Poste = 1 WHERE id = ? AND Poste = 0 AND Statut = 0`
   (si 0 ligne modifiée, quelqu'un d'autre l'a prise : ne pas publier).
3. Enregistrer la décision :
   `UPDATE lvrp_whitelist SET Statut = ?, Motif = ?, DecidePar = ?, DateDecision = UNIX_TIMESTAMP() WHERE id = ? AND Statut = 0`
   (le `AND Statut = 0` empêche d'écraser une décision déjà prise en jeu).

C'est tout. Le serveur de jeu fait le reste.

### Et si je ne peux pas héberger du Node.js ?

Le choix de l'hébergement n'étant pas encore arrêté, sache que **ce bot est
remplaçable sans toucher au serveur de jeu**, parce que le contrat ci-dessus ne
dépend d'aucun langage.

Si ton hébergement ne propose que **PHP + cron** (le cas de la plupart des
hébergements mutualisés, celui-là même qui héberge déjà `discord_relay.php`), la
version PHP consiste en un script exécuté toutes les minutes par une tâche cron,
qui fait exactement les points 1 et 2 ci-dessus et envoie le dossier au webhook
Discord.

La seule vraie différence est le point 3 : **PHP en cron ne peut pas recevoir de
clic sur un bouton** (Discord doit pouvoir appeler ton serveur en HTTPS et
recevoir une réponse en moins de 3 secondes). Deux contournements possibles :

- **Le plus simple** : pas de boutons. Le dossier est publié avec son numéro, et
  le staff tranche en jeu avec `/approuver` / `/refuser`. Discord ne sert alors
  qu'à *voir* les candidatures — ce qui règle déjà le problème principal (savoir
  qu'un joueur attend).
- **Avec boutons** : il faut un fichier PHP accessible en **HTTPS** publiquement,
  déclaré comme « Interactions Endpoint URL » dans le portail développeur
  Discord, et qui vérifie la signature Ed25519 envoyée par Discord. C'est
  faisable, mais nettement plus délicat que la version Node.js.

Dans les deux cas, **le serveur de jeu ne change pas d'une ligne** : il continue
à lire `Statut` dans la même table, toutes les 12 secondes.
