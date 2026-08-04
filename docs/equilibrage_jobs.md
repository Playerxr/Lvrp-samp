# Equilibrage des metiers - ce qu'il faut savoir avant de changer les gains

Ce document explique **d'ou vient l'argent d'un joueur qui travaille**, **quel
reglage controle quoi**, et **quelles informations sont necessaires** pour fixer
des montants coherents avec l'economie du serveur.

**Aucun montant n'a ete modifie.** Ce document est une preparation : il decrit
l'existant et donne la methode de calcul. Les chiffres finaux restent a decider.

---

## 1. Le revenu d'un joueur qui travaille : deux morceaux

Quand un joueur a un metier, son argent vient de **deux sources differentes**,
versees **en meme temps**, au moment de la paye.

### Morceau 1 - LE SALAIRE (le gros morceau)

- Verse **une fois par heure de jeu** (60 minutes connecte et non AFK).
- Son montant **n'est pas dans le code du jeu** : il est stocke dans la **base
  de donnees** (voir section 3).
- Il y a **un salaire different par metier** : le Livreur de Pizza et le
  Camionneur peuvent avoir des salaires differents.
- **Regle de temps de travail** : pour toucher le salaire **entier**, il faut
  avoir ete **en service au moins 30 minutes** dans l'heure. En dessous, le
  salaire est verse au prorata (15 minutes de service = la moitie du salaire).
- **Attention** : un joueur qui est **a la fois** dans un metier **et** dans une
  faction ne touche **pas** le salaire de metier. Le code ne verse le salaire de
  metier que si le joueur n'a ni faction ni role de chef.

### Morceau 2 - LE BONUS DE TACHE (le petit morceau)

- Gagne **a chaque action de metier reussie** : une pizza livree, un rondin
  depose, un camion decharge...
- Il **s'accumule** dans un compteur, et il est **ajoute au salaire** au moment
  de la paye. Le compteur est ensuite remis a zero.
- Son montant **est dans le code**, et il est maintenant **regroupe au meme
  endroit** (voir section 2).

---

## 2. Le bonus de tache : ou est le bouton

Tous les reglages sont desormais rassembles **en haut du fichier
`gamemodes/LVRP.pwn`**, juste apres la ligne `#define JOB_TIME`. Ils sont
identifies par le commentaire `[EQUILIBRAGE JOBS]`.

### La formule

Pour une tache accomplie :

```
bonus = PARLVL x niveau_du_metier  +  FIXE  +  tirage au sort entre 0 et ALEA-1
```

Le `niveau_du_metier` va de 0 a 10 : plus le joueur a fait de taches, plus il
monte, plus il gagne.

### Les trois bandes

Chaque metier appartient a une bande, selon l'effort que demande une tache :

| Bande | Pour quels metiers | PARLVL | FIXE | ALEA |
|---|---|---|---|---|
| **BASSE** | action tres rapide et repetable : Pizza, Facteur, Mecanicien, Convoyeur, Taxi | 2 | 3 | 15 |
| **MOYENNE** | travail manuel a pied : Fermier, Mineur, Eboueur, Ouvrier, Voiturier, Bucheron, Jardinier | 2 | 0 | 20 |
| **HAUTE** | metier en vehicule, une action = un long trajet : Pilote, Pecheur, Camionneur, Bagagiste | 3 | 0 | 40 |

Les noms exacts des reglages sont `JOB_BONUS_BASSE_PARLVL`,
`JOB_BONUS_BASSE_FIXE`, `JOB_BONUS_BASSE_ALEA`, et pareil pour `MOYENNE` et
`HAUTE`. **Changer une bande change d'un coup tous les metiers de cette bande**
- c'est justement le but de ce regroupement.

### Deux multiplicateurs viennent ensuite s'ajouter

Ils sont dans `pawno/include/afrp_jobs.inc` :

- **Bonus debutant** : un joueur de niveau de compte 3 ou moins gagne le
  **double** sur chaque tache.
- **Prime de serie** : enchainer les taches sans pause ajoute **+10% par tache
  consecutive**, jusqu'a **+100%** maximum. La serie se perd apres 3 minutes
  sans action.

Cumules, un joueur debutant qui enchaine peut donc toucher jusqu'a **3 fois** le
bonus de base.

### La vitesse de progression

Le nombre de taches necessaires pour monter d'un niveau de metier est controle
par `JOB_XP_APIED_PARLVL` (20) et `JOB_XP_VEHICULE_PARLVL` (8). Plus le chiffre
est grand, plus la progression est lente, donc plus le bonus met de temps a
grossir.

---

## 3. Le salaire : ou il vit exactement

Le salaire **n'est pas modifiable dans le code**. Il est lu dans la base de
donnees MySQL du serveur :

- **Table** : `lvrp_factions_governements`
- **Ligne** : celle dont l'`id` vaut **4**
- **Colonnes** : `job1` a `job20`

La correspondance est directe : `job1` = metier n°1 (Livreur de Pizza),
`job2` = Fermier, `job3` = Mineur, ... `job20` = Chauffeur de taxi.

> Remarque : le metier n°21 (Avocat) **n'a pas de colonne** - il n'y a que 20
> cases. Ce metier n'a de toute facon aucune mission jouable aujourd'hui.

### Pour voir les valeurs actuelles

```sql
SELECT job1, job2, job3, job4, job5, job6, job7, job8, job9, job10,
       job11, job12, job13, job14, job15, job16, job17, job18, job19, job20
FROM lvrp_factions_governements
WHERE id = 4;
```

### Deux limites importantes a connaitre

**a) Le menu en jeu est plafonne a 10 000 $.**
L'ecran de gestion des salaires (`/gouv gestion`) refuse tout montant en dehors
de **100 $ - 10 000 $**. Autrement dit, **il est aujourd'hui impossible de
mettre un salaire superieur a 10 000 $ par heure depuis le jeu**. Pour aller
au-dela il faut soit passer par une requete SQL, soit faire modifier ce plafond
dans le code.

**b) Le menu en jeu ne modifie pas forcement la bonne ligne.**
La ligne utilisee par la paye est la ligne `id = 4`. Dans le menu en jeu, cette
ligne correspond a la faction n°9 (C.I.A). Un membre de la Mairie qui ouvre le
menu modifiera une **autre** ligne, sans effet sur les salaires reellement
verses. **Le plus sur est de passer par SQL.**

### Un bug corrige au passage

La sauvegarde des salaires ecrivait, dans la colonne `job4` (Eboueur), la
**taxe d'electricite** au lieu du salaire de l'Eboueur. Autrement dit, chaque
fois que quelqu'un modifiait un salaire depuis le jeu, le salaire de l'Eboueur
etait ecrase par une valeur qui n'avait rien a voir. C'est corrige.
**Il faut donc verifier la valeur actuelle de `job4` en base : elle est
probablement fausse.**

---

## 4. Ou en est-on aujourd'hui, en chiffres

En prenant les reglages actuels, **par tache** :

| Bande | Joueur niveau 1 | Joueur niveau 10 (maximum) |
|---|---|---|
| BASSE | 5 a 19 $ | 23 a 37 $ |
| MOYENNE | 0 a 19 $ | 20 a 39 $ |
| HAUTE | 3 a 42 $ | 30 a 69 $ |

Et attention : le bonus n'est pas verse a **chaque** tache, mais **toutes les 3
a 5 taches** selon le metier.

Meme en supposant un joueur tres actif et le bonus debutant, on reste dans un
ordre de grandeur de **quelques centaines de dollars par heure** de bonus.

**Comparaison avec l'economie du serveur :**

- vehicule le moins cher : **5 000 000 $**
- vehicule le plus cher : **1 000 000 000 $**
- salaire maximum reglable depuis le jeu : **10 000 $ / heure**

Donc, dans l'etat actuel, meme au salaire maximum autorise par le menu en jeu,
il faut environ **500 heures de jeu pour s'offrir le vehicule le moins cher**.
C'est cet ecart qu'il faut trancher.

---

## 5. Exemple de calcul (methode, PAS une recommandation)

La question a se poser n'est pas "combien doit gagner un metier ?" mais
**"en combien d'heures de jeu un joueur regulier doit-il pouvoir s'offrir le
vehicule le moins cher ?"**. Tout le reste se deduit.

La formule est simple :

```
revenu vise par heure  =  prix du vehicule le moins cher  /  nombre d'heures voulu
```

Trois exemples de calcul, **a titre d'illustration uniquement** :

| Si on veut que le 1er vehicule (5 000 000 $) s'obtienne en... | ...il faut viser par heure |
|---|---|
| 500 heures | 10 000 $ |
| 100 heures | 50 000 $ |
| 50 heures | 100 000 $ |

Ensuite, comme le bonus de tache est aujourd'hui negligeable devant ces
montants, on peut poser en premiere approche :

```
salaire a mettre dans job1..job20  ~=  revenu vise par heure
```

Puis, si on veut **recompenser l'activite** plutot que la simple presence, on
baisse le salaire et on remonte le bonus de tache. Par exemple, pour un objectif
de 50 000 $ / heure : 40 000 $ de salaire + 10 000 $ de bonus de tache gagnes en
travaillant vraiment. Cela suppose de multiplier les reglages de la section 2
par un facteur important.

**Encore une fois : les chiffres de ce tableau sont des exemples de calcul, pas
des propositions.** Le choix du nombre d'heures appartient a la direction du
serveur.

---

## 6. Ce qu'il faut decider pour aller plus loin

Pour pouvoir proposer des montants precis, il faut ces reponses :

1. **En combien d'heures de jeu** un joueur regulier doit-il pouvoir s'offrir le
   vehicule le moins cher (5 000 000 $) ? C'est la question centrale.
2. **Tous les metiers doivent-ils payer pareil**, ou certains doivent-ils etre
   nettement mieux payes que d'autres ? (par exemple : Camionneur mieux paye que
   Livreur de Pizza)
3. **Faut-il recompenser la presence ou l'activite ?** Un gros salaire recompense
   le fait d'etre connecte ; un gros bonus de tache recompense le fait de
   travailler vraiment.
4. **Garde-t-on le bonus debutant (x2) et la prime de serie (jusqu'a +100%) ?**
5. **Faut-il relever le plafond de 10 000 $ du menu en jeu ?** Sans cela, la
   gestion des salaires devra se faire en SQL.
6. **Quelles sont les valeurs actuellement en base ?** (requete SQL de la
   section 3) - sans elles, impossible de savoir de quel point on part.
