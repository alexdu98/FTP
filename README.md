Mini projet de réseaux : serveur de FTP en TCP


##### INSTRUCTIONS DE COMPILATION #####
---------------------------------------

Compilation : se rendre dans la partie voulue (unclient, multi-clients, ou bonus) et taper :
"make" : pour compiler le serveur (mainS) et le client (mainC)


##### INSTRUCTIONS DE DEMARRAGE #####
--------------------------------------

Pour démarrer le serveur :
./mainS <port>(0 = rand, > 1024) [repertoireDL]
Par exemple : 
./mainS 0					: pour choisir un port par défaut et le répértoire de téléchargement par défaut (../repDL/)
./mainS 10000 ../download 	: pour choisir le port 10000 et le répertoire download du répertoire parent

Pour démarrer le client :
./mainC <IPServeur|NDDServeur> <PortServeur>
Par exemple :
./mainC localhost 10000 	: pour se connecter sur le serveur en localhost sur le port 10000
./mainC 10.0.10.5 45120 	: pour se connecter sur le serveur d'IP 10.0.10.5 sur le port 45120


##### INSTRUCTIONS D'UTILISATION #####
--------------------------------------

Pour le serveur :
- unclient 			: rien
- multi-clients		: appuyez sur la touche entrée pour voir le nombre de téléchargement par fichier
- bonus :			: idem que multi-clients

Pour le client :
- unclient 			: tapez HELP pour voir les commandes disponibles et leurs descriptions
- multi-clients		: idem que unclient
- bonus :			: idem que unclient