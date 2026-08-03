/*
 * Bot Discord - validation des candidatures whitelist de La Vie RolePlay
 * ----------------------------------------------------------------------
 * Ce bot ne parle JAMAIS au serveur SA-MP. Les deux echangent uniquement par
 * la table MySQL `lvrp_whitelist` (SA-MP ne sait pas faire de HTTPS, et on
 * refuse d'installer un plugin natif Discord sur un serveur de production).
 *
 * Consequence : ce bot peut tourner absolument n'importe ou du moment qu'il
 * atteint la base MySQL du serveur. Il est aussi entierement remplacable :
 * tout ce qui respecte le contrat ci-dessous fera le meme travail.
 *
 * CONTRAT (identique cote gamemode, voir pawno/include/afrp_whitelist.inc)
 *   Statut = -1  brouillon, le joueur remplit encore le formulaire  -> IGNORER
 *   Statut =  0  dossier complet en attente de decision             -> A POSTER
 *   Statut =  1  accepte     |  Statut = 2  refuse (Motif rempli)
 *   Poste  =  1  deja publie sur Discord      (ecrit par CE bot)
 *   Traite =  1  decision deja appliquee en jeu (ecrit par le GAMEMODE)
 *
 * Ce bot n'ecrit QUE : Poste, Statut, Motif, DecidePar, DateDecision.
 * Il ne touche jamais a Traite (c'est au gamemode de dire qu'il a applique)
 * ni a aucune autre table.
 *
 * Configuration : copier config.example.json en config.json et le remplir.
 * Voir README.md pour la marche a suivre pas a pas.
 */

'use strict';

const fs = require('fs');
const path = require('path');
const mysql = require('mysql2/promise');
const {
  Client,
  GatewayIntentBits,
  EmbedBuilder,
  ActionRowBuilder,
  ButtonBuilder,
  ButtonStyle,
  ModalBuilder,
  TextInputBuilder,
  TextInputStyle,
  MessageFlags,
} = require('discord.js');

// ---------------------------------------------------------------- config

const CONFIG_PATH = path.join(__dirname, 'config.json');

if (!fs.existsSync(CONFIG_PATH)) {
  console.error('[ERREUR] config.json introuvable.');
  console.error('         Copie config.example.json en config.json et remplis-le.');
  process.exit(1);
}

let config;
try {
  config = JSON.parse(fs.readFileSync(CONFIG_PATH, 'utf8'));
} catch (e) {
  console.error('[ERREUR] config.json illisible (JSON invalide) :', e.message);
  process.exit(1);
}

const manquants = [];
if (!config.discord || !config.discord.token) manquants.push('discord.token');
if (!config.discord || !config.discord.salonCandidatures) manquants.push('discord.salonCandidatures');
if (!config.mysql || !config.mysql.host) manquants.push('mysql.host');
if (!config.mysql || !config.mysql.user) manquants.push('mysql.user');
if (!config.mysql || !config.mysql.database) manquants.push('mysql.database');
if (manquants.length) {
  console.error('[ERREUR] Champs manquants dans config.json : ' + manquants.join(', '));
  process.exit(1);
}

const options = config.options || {};
// Cadence de publication. Elle n'a pas besoin d'etre rapide : c'est le
// gamemode qui reagit vite (toutes les 12s) une fois la decision prise.
const INTERVALLE_MS = Math.max(5, Number(options.intervalleSecondes) || 15) * 1000;
const AGE_REEL_ALERTE = Number(options.alerteAgeReelMin) || 16;
const ROLE_STAFF = (config.discord.roleStaff || '').trim();

// ---------------------------------------------------------------- outils

// Le contenu des dossiers est de la saisie joueur : on neutralise le Markdown
// pour qu'un candidat ne puisse pas deformer l'affichage du salon staff.
function echapper(texte) {
  return String(texte == null ? '' : texte)
    .replace(/[\\`*_~|>[\]]/g, (c) => '\\' + c)
    .slice(0, 1000);
}

function couper(texte, max) {
  const s = String(texte == null ? '' : texte);
  return s.length > max ? s.slice(0, max - 1) + '…' : s;
}

// Aucune mention ne doit pouvoir partir d'un contenu ecrit par un joueur.
const SANS_MENTION = { parse: [] };

function estStaff(interaction) {
  if (!ROLE_STAFF) return true; // pas de role configure : on fait confiance aux permissions du salon
  const membre = interaction.member;
  if (!membre || !membre.roles) return false;
  if (typeof membre.roles.cache?.has === 'function') return membre.roles.cache.has(ROLE_STAFF);
  if (Array.isArray(membre.roles)) return membre.roles.includes(ROLE_STAFF);
  return false;
}

function nomStaff(interaction) {
  const u = interaction.user;
  return couper(u ? (u.tag || u.username || u.id) : 'inconnu', 60);
}

// ---------------------------------------------------------------- MySQL

const pool = mysql.createPool({
  host: config.mysql.host,
  port: Number(config.mysql.port) || 3306,
  user: config.mysql.user,
  password: config.mysql.password || '',
  database: config.mysql.database,
  waitForConnections: true,
  connectionLimit: 4,
  charset: 'latin1', // la base du gamemode est en latin1
});

// ---------------------------------------------------------------- Discord

const client = new Client({ intents: [GatewayIntentBits.Guilds] });

// Ou se trouve le message publie pour chaque candidature. Sert uniquement a
// re-editer le message apres la saisie du motif de refus (modal). Si le bot
// redemarre entre les deux, on perd juste l'edition cosmetique, jamais la
// decision : celle-ci vit en base.
const messagesPublies = new Map(); // idCandidature -> { channelId, messageId }

function construireEmbed(ligne) {
  const alerteAge = Number(ligne.AgeReel) > 0 && Number(ligne.AgeReel) < AGE_REEL_ALERTE;
  const embed = new EmbedBuilder()
    .setColor(alerteAge ? 0xe67e22 : 0xffaa00)
    .setTitle('Nouvelle candidature whitelist #' + ligne.id)
    .setDescription('Compte en jeu : **' + echapper(ligne.Name) + '**')
    .addFields(
      { name: 'Nom RP', value: echapper(ligne.NomRP) || '-', inline: true },
      { name: 'Age du personnage', value: String(ligne.AgeRP || '-'), inline: true },
      {
        name: 'Age reel du joueur',
        value: String(ligne.AgeReel || '-') + (alerteAge ? '  :warning: sous ' + AGE_REEL_ALERTE + ' ans' : ''),
        inline: true,
      },
      { name: 'Background du personnage', value: echapper(ligne.Background) || '-' },
      { name: 'Comment il a connu le serveur', value: echapper(ligne.Source) || '-' },
    )
    .setFooter({ text: 'Candidature #' + ligne.id + ' - decide ci-dessous' })
    .setTimestamp(ligne.DateSoumission ? new Date(Number(ligne.DateSoumission) * 1000) : new Date());
  return embed;
}

function construireBoutons(id) {
  return new ActionRowBuilder().addComponents(
    new ButtonBuilder()
      .setCustomId('wl_ok_' + id)
      .setLabel('Accepter')
      .setStyle(ButtonStyle.Success),
    new ButtonBuilder()
      .setCustomId('wl_no_' + id)
      .setLabel('Refuser')
      .setStyle(ButtonStyle.Danger),
  );
}

function embedDecide(ligne, statut, motif, par) {
  const accepte = statut === 1;
  return new EmbedBuilder()
    .setColor(accepte ? 0x2ecc71 : 0xe74c3c)
    .setTitle((accepte ? 'ACCEPTEE' : 'REFUSEE') + ' - candidature #' + ligne.id)
    .setDescription(
      'Compte en jeu : **' + echapper(ligne.Name) + '**\n' +
      'Nom RP : **' + (echapper(ligne.NomRP) || '-') + '**\n' +
      'Decision par : **' + echapper(par) + '**' +
      (accepte ? '' : '\nMotif : ' + (echapper(motif) || '-')),
    )
    .setTimestamp(new Date());
}

// --------------------------------------------------- publication des dossiers

async function publierDossiers() {
  let salon;
  try {
    salon = await client.channels.fetch(config.discord.salonCandidatures);
  } catch (e) {
    console.error('[Discord] Salon des candidatures introuvable (' + config.discord.salonCandidatures + ') :', e.message);
    return;
  }
  if (!salon || !salon.isTextBased()) {
    console.error('[Discord] Le salon configure n\'est pas un salon textuel.');
    return;
  }

  let lignes;
  try {
    [lignes] = await pool.query(
      'SELECT id, SQLid, Name, NomRP, AgeRP, Background, AgeReel, Source, DateSoumission ' +
      'FROM lvrp_whitelist WHERE Statut = 0 AND Poste = 0 ORDER BY id ASC LIMIT 10',
    );
  } catch (e) {
    console.error('[MySQL] Lecture des candidatures impossible :', e.message);
    return;
  }

  for (const ligne of lignes) {
    // On "reserve" le dossier AVANT de l'envoyer : si deux instances du bot
    // tournaient par erreur, une seule verrait affectedRows = 1 et il n'y
    // aurait pas de double publication.
    let reserve;
    try {
      const [res] = await pool.execute(
        'UPDATE lvrp_whitelist SET Poste = 1 WHERE id = ? AND Poste = 0 AND Statut = 0',
        [ligne.id],
      );
      reserve = res.affectedRows === 1;
    } catch (e) {
      console.error('[MySQL] Reservation du dossier #' + ligne.id + ' impossible :', e.message);
      continue;
    }
    if (!reserve) continue;

    try {
      const message = await salon.send({
        embeds: [construireEmbed(ligne)],
        components: [construireBoutons(ligne.id)],
        allowedMentions: SANS_MENTION,
      });
      messagesPublies.set(String(ligne.id), { channelId: message.channelId, messageId: message.id });
      console.log('[OK] Candidature #' + ligne.id + ' publiee (' + ligne.Name + ').');
    } catch (e) {
      // Publication ratee : on rend le dossier au prochain tour, sinon il
      // resterait invisible pour toujours alors que le joueur attend.
      console.error('[Discord] Publication du dossier #' + ligne.id + ' impossible :', e.message);
      try {
        await pool.execute('UPDATE lvrp_whitelist SET Poste = 0 WHERE id = ? AND Statut = 0', [ligne.id]);
      } catch (e2) {
        console.error('[MySQL] Impossible de remettre le dossier #' + ligne.id + ' en file :', e2.message);
      }
    }
  }
}

// --------------------------------------------------- application d'une decision

async function lireDossier(id) {
  const [lignes] = await pool.execute('SELECT * FROM lvrp_whitelist WHERE id = ? LIMIT 1', [id]);
  return lignes.length ? lignes[0] : null;
}

// Renvoie { ok, ligne, raison }. Le WHERE Statut = 0 est ce qui rend
// l'operation sure : si le staff a deja tranche en jeu (/approuver, /refuser)
// ou depuis un autre clic, affectedRows vaut 0 et on ne rien ecrase.
async function enregistrerDecision(id, statut, motif, par) {
  const ligne = await lireDossier(id);
  if (!ligne) return { ok: false, raison: 'Cette candidature n\'existe plus en base.' };
  if (Number(ligne.Statut) !== 0) {
    return {
      ok: false,
      ligne,
      raison:
        'Cette candidature a deja ete traitee' +
        (ligne.DecidePar ? ' par ' + echapper(ligne.DecidePar) : '') +
        ' (' + (Number(ligne.Statut) === 1 ? 'acceptee' : 'refusee') + ').',
    };
  }

  const [res] = await pool.execute(
    'UPDATE lvrp_whitelist SET Statut = ?, Motif = ?, DecidePar = ?, DateDecision = UNIX_TIMESTAMP() ' +
    'WHERE id = ? AND Statut = 0',
    [statut, couper(motif || '', 128), couper(par, 64), id],
  );
  if (res.affectedRows !== 1) {
    return { ok: false, ligne, raison: 'Quelqu\'un vient de traiter cette candidature avant toi.' };
  }
  return { ok: true, ligne };
}

// Remplace le message d'origine par le resultat, pour que le salon reste lisible.
async function marquerMessage(interaction, id, ligne, statut, motif, par) {
  const contenu = {
    embeds: [embedDecide(ligne, statut, motif, par)],
    components: [],
    allowedMentions: SANS_MENTION,
  };
  try {
    if (interaction.message && typeof interaction.message.edit === 'function') {
      await interaction.message.edit(contenu);
      messagesPublies.delete(String(id));
      return;
    }
    const ref = messagesPublies.get(String(id));
    if (!ref) return;
    const salon = await client.channels.fetch(ref.channelId);
    const message = await salon.messages.fetch(ref.messageId);
    await message.edit(contenu);
    messagesPublies.delete(String(id));
  } catch (e) {
    // Purement cosmetique : la decision est deja en base, le joueur entrera
    // (ou sera refuse) quoi qu'il arrive.
    console.error('[Discord] Edition du message de la candidature #' + id + ' impossible :', e.message);
  }
}

// ---------------------------------------------------------------- evenements

// discord.js a renomme l'evenement 'ready' en 'clientReady' au fil des
// versions 14.x : on ecoute les deux, avec un garde-fou pour ne demarrer la
// boucle qu'une seule fois quelle que soit la version installee.
let demarre = false;
function auDemarrage() {
  if (demarre) return;
  demarre = true;
  console.log('[OK] Connecte a Discord en tant que ' + client.user.tag + '.');
  console.log('[OK] Publication des nouvelles candidatures toutes les ' + INTERVALLE_MS / 1000 + ' s.');
  publierDossiers().catch((e) => console.error(e));
  setInterval(() => publierDossiers().catch((e) => console.error(e)), INTERVALLE_MS);
}
client.once('ready', auDemarrage);
client.once('clientReady', auDemarrage);

client.on('interactionCreate', async (interaction) => {
  try {
    // ---- clic sur Accepter / Refuser -------------------------------------
    if (interaction.isButton()) {
      const m = /^wl_(ok|no)_(\d+)$/.exec(interaction.customId);
      if (!m) return;
      const action = m[1];
      const id = m[2];

      if (!estStaff(interaction)) {
        await interaction.reply({
          content: 'Seul le staff peut traiter les candidatures.',
          flags: MessageFlags.Ephemeral,
        });
        return;
      }

      if (action === 'no') {
        // Le refus demande un motif : il est renvoye au joueur en jeu.
        messagesPublies.set(String(id), {
          channelId: interaction.channelId,
          messageId: interaction.message ? interaction.message.id : null,
        });
        const modal = new ModalBuilder()
          .setCustomId('wl_modal_' + id)
          .setTitle('Refus de la candidature #' + id)
          .addComponents(
            new ActionRowBuilder().addComponents(
              new TextInputBuilder()
                .setCustomId('motif')
                .setLabel('Motif (lu par le joueur en jeu)')
                .setStyle(TextInputStyle.Paragraph)
                .setMaxLength(128)
                .setRequired(true)
                .setPlaceholder('Background trop court, hors-RP, age non conforme...'),
            ),
          );
        await interaction.showModal(modal);
        return;
      }

      await interaction.deferReply({ flags: MessageFlags.Ephemeral });
      const res = await enregistrerDecision(id, 1, '', nomStaff(interaction));
      if (!res.ok) {
        await interaction.editReply({ content: res.raison });
        if (res.ligne) {
          await marquerMessage(
            interaction, id, res.ligne, Number(res.ligne.Statut),
            res.ligne.Motif, res.ligne.DecidePar || 'le staff',
          );
        }
        return;
      }
      await marquerMessage(interaction, id, res.ligne, 1, '', nomStaff(interaction));
      await interaction.editReply({
        content: 'Candidature #' + id + ' acceptee. Le joueur est mis en jeu automatiquement dans les secondes qui viennent.',
      });
      console.log('[OK] Candidature #' + id + ' acceptee par ' + nomStaff(interaction) + '.');
      return;
    }

    // ---- validation du motif de refus ------------------------------------
    if (interaction.isModalSubmit()) {
      const m = /^wl_modal_(\d+)$/.exec(interaction.customId);
      if (!m) return;
      const id = m[1];

      await interaction.deferReply({ flags: MessageFlags.Ephemeral });
      const motif = interaction.fields.getTextInputValue('motif');
      const res = await enregistrerDecision(id, 2, motif, nomStaff(interaction));
      if (!res.ok) {
        await interaction.editReply({ content: res.raison });
        return;
      }
      await marquerMessage(interaction, id, res.ligne, 2, motif, nomStaff(interaction));
      await interaction.editReply({
        content: 'Candidature #' + id + ' refusee. Le joueur recevra le motif et son compte sera supprime.',
      });
      console.log('[OK] Candidature #' + id + ' refusee par ' + nomStaff(interaction) + '.');
    }
  } catch (e) {
    console.error('[Bot] Erreur pendant le traitement d\'une interaction :', e);
    try {
      if (interaction.deferred || interaction.replied) {
        await interaction.editReply({ content: 'Une erreur est survenue, rien n\'a ete modifie. Reessaie.' });
      } else if (interaction.isRepliable()) {
        await interaction.reply({
          content: 'Une erreur est survenue, rien n\'a ete modifie. Reessaie.',
          flags: MessageFlags.Ephemeral,
        });
      }
    } catch (_) { /* interaction perdue, rien a faire */ }
  }
});

client.on('error', (e) => console.error('[Discord] ', e));
process.on('unhandledRejection', (e) => console.error('[Bot] Rejet non gere :', e));

client.login(config.discord.token).catch((e) => {
  console.error('[ERREUR] Connexion a Discord impossible. Verifie discord.token dans config.json.');
  console.error('         Detail :', e.message);
  process.exit(1);
});
