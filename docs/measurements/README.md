# Tests Linux avec netem

Le banc utilise le serveur du projet et deux clients C++ sans fenêtre. Les clients
emploient `engine::network::ClientConnection` : la fiabilité et la reconstruction
des snapshots sont celles du jeu. Aucun paquet n'est supprimé artificiellement
dans les clients ; les perturbations sont appliquées par le noyau avec `tc netem`.

## Compiler

Avec les dépendances habituelles du projet installées, depuis la racine :

```sh
cmake -S . -B build/netem -DENGINE_BUILD_CLIENT=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/netem --target r-type_server --parallel 2
c++ -std=c++20 -O2 -pthread \
  -Iengine/network/include -Iengine/net/include \
  -Iengine/scene/include -Iengine/core/include \
  -Ibuild/netem/vcpkg_installed/x64-linux/include \
  scripts/netem_client.cpp \
  -Wl,--start-group build/netem/libengine_*.a -Wl,--end-group \
  -o build/netem/bin/netem_client
```

Adapter le chemin des en-têtes vcpkg si `VCPKG_INSTALLED_DIR` est personnalisé.

## Lancer

Installer `iproute2` si les commandes `tc` ou `ip` sont absentes. Le noyau doit
autoriser les espaces de noms utilisateur et réseau et fournir le module netem.

```sh
NETEM_HOST_NS=$(readlink /proc/self/ns/net)
unshare --user --map-root-user --net \
  python3 scripts/netem_test.py --host-netns "$NETEM_HOST_NS" \
  --server build/netem/bin/r-type_server \
  --client build/netem/bin/netem_client \
  --seconds 20 --output docs/measurements/mon-essai
```

Le script vérifie que son espace réseau diffère de celui passé en argument,
active uniquement sa boucle locale et ne modifie aucune interface du réseau hôte.
Il ferme chaque serveur et retire la qdisc à la fin. La destruction de l'espace
réseau supprime aussi les réglages si le processus est interrompu brutalement.

Pour une seule condition, ajouter par exemple `--case loss_10pct`.
Les sept conditions sont définies dans `scripts/netem_test.py` et leurs commandes
exactes sont conservées dans `results.json`. Chaque sortie est enregistrée avec
son scénario ; `rapport.md` présente les mesures et leurs limites.

## Mesure du 8 novembre 2026

Voir [le rapport](netem-2026-11-08/rapport.md) et les fichiers bruts voisins.
Les bibliothèques ont été compilées depuis les sources courantes avec CMake en
Release (`-O3 -DNDEBUG`), puis les exécutables serveur et sonde liés avec GCC 13,
C++20 et `-O2`. Les hashes des exécutables et de la configuration sont conservés
dans `results.json`.

Sur cette machine, le paquet CMake pybind11 manquait dans vcpkg. Pour cette
compilation isolée, une configuration CMake locale dans `build/netem/pybind11`
a exposé les en-têtes pybind11 déjà installés avec PyTorch et les a liés à
`Python3::Python` (Python système 3.12.3). Le serveur utilise la bibliothèque
standard Python du système, sans copie d'un runtime à côté du binaire.
Le code du jeu, du transport et du protocole n'a pas été modifié.
