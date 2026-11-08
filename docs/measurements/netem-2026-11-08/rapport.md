# Mesures Linux netem

Date UTC : 2026-11-08T14:51:31.821182+00:00

Deux clients sans interface graphique utilisent le ClientConnection C++ de production,
le codec et le transport réels. Un serveur neuf est lancé pour chaque scénario.
Création d'une salle, arrivée du second joueur, démarrage, puis tirs pendant la mesure.
Les commandes créer/rejoindre/démarrer sont envoyées une seule fois par le test :
les répétitions éventuelles sont assurées par le mécanisme fiable du projet.

netem est appliqué à lo dans un espace réseau jetable, dans les deux sens.
Un délai de 100 ms affecte chaque trajet, donc ajoute environ 200 ms au RTT.
Le débit configuré est partagé par les paquets passant sur cette interface.

| Scénario | Verdict | RTT médian A/B (ms) | Snapshots/s A/B | Réception A/B (KiB/s) | Intervalle max A/B (s) |
|---|---|---|---|---|---|
| baseline | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.04 / 2.04 | 0.05 / 0.05 |
| delay_100ms | PASS | 203.00 / 202.00 | 20.00 / 20.00 | 3.76 / 3.76 | 0.05 / 0.05 |
| loss_10pct | PASS | 2.00 / 2.00 | 18.15 / 18.20 | 1.84 / 1.84 | 0.15 / 0.20 |
| duplicate_10pct | PASS | 2.00 / 2.00 | 20.00 / 20.00 | 2.20 / 2.21 | 0.05 / 0.05 |
| reorder_25pct | PASS | 102.00 / 103.00 | 18.65 / 18.90 | 2.66 / 2.66 | 0.10 / 0.10 |
| combined | PASS | 163.00 / 155.00 | 17.15 / 17.25 | 2.66 / 2.69 | 0.19 / 0.15 |
| rate_128kbit | PASS | 88.00 / 86.00 | 20.00 / 20.00 | 2.72 / 2.74 | 0.06 / 0.06 |

## Critères et limites

PASS : les deux clients rejoignent et démarrent, chacun reçoit au moins 10 snapshots
pendant la mesure, aucun tick appliqué ne recule, aucun doublon de ROOM_JOINED ou
GAME_STARTED n'est livré à l'application, et aucun silence des snapshots n'atteint 5 s.
Une exception ou un dépassement du délai du processus fait échouer le scénario.

Le RTT est celui du heartbeat, échantillonné chaque seconde ; une valeur peut être
réutilisée si la réponse suivante est perdue. Il ne mesure pas le délai entre une touche
et son affichage. Le débit compte les octets applicatifs, en-tête du protocole inclus,
sans en-têtes UDP/IP/Ethernet. La cadence des snapshots compte les états effectivement
fournis par le client, après filtrage et reconstruction des deltas.

Un passage par scénario, deux clients, une salle : ces essais ne prouvent ni la tenue
sous charge maximale, ni la livraison de tous les événements de jeu. Les pourcentages
netem sont des paramètres aléatoires, pas des taux de pertes applicatifs mesurés.
Consulter les fichiers *.qdisc.txt, *.server.log, *.client.log et results.json pour les preuves.
