---
title: 'TP1 Installation, "Hello World" et led-bp user'

---

# TP1 Installation, "Hello World" et led-bp user

## Auteurs
- Das
- Carbou

## Réponses aux questions du TME

- Pourquoi passer par la redirection des ports ?
\- On passe par une redirection de port afin de sélectionner la carte Raspberry-pi sur laquelle on veut se connecter.

- Pourquoi nos fichiers doivent-ils être dans un répertoire propre à notre groupe ?
\- On doit créer notre répertoire de groupe car la raspberry-pi est partagée entre plusieurs groupes, et qu'il n'y a qu'un seul compte utilisateur sur les cartes RapsberryPi.

- Expliquez pourquoi, il pourrait être dangereux de se tromper de broche pour la configuration des GPIO.
\- Cela pourrait être dangereux de se tromper de broche. Si on mettait du courant dans une broche dans laquelle on ne s'attendait pas à en avoir risquerait de griller un composant ou de faire un court circuit.

- A quoi correspond l'adresse BCM2835_GPIO_BASE ?
\- Cette adresse correspond à la premère adresse à laquelle on peut accéder aux registres de configuration.

- Que représente la structure struct gpio_s ? 
\- La structure correspond aux registres de configurations qui correspondent aux entrées/sorties.

- Dans quel espace d'adressage est l'adresse gpio_regs_virt ? 
\- gpio_regs_virt est une adresse virtuelle vers les registres de configuration.

- Dans la fonction gpio_fsel(), que contient la variable reg ? 
\- Elle contient l'indice du tableau gpfsel correspondant à la ligne qui contient la broche correspondant au pin.

- Dans la fonction gpio_write(), pourquoi écrire à deux adresses différentes en fonction de la valeur val ?
\-Si val est à 1 alors on set le bit donc on accède à gpset. Si val = 0 alors on clear donc on accède à gpclr.

- Dans la fonction gpio_mmap(), à quoi correspondent les flags de open()?
\- Le flag O_RDWR signifie qu'on veut l'accès d'écriture et de lecture. Le flag O_SYNC spécifie que chaque modification dans le fichier sera enregistrée en mémoire avant le retour de la fonction de modification. L'écriture sera donc synchrone.

- Dans la fonction gpio_mmap(), commentez les arguments de mmap().
\- NULL : on confie au kernel le choix du placement de l'adresse (alignée) du mapping
\-  RPI_BLOCK_SIZE : Taille à mapper (ici 4096, donc exactement 1 page)
\-  PROT_READ | PROT_WRITE : on veut les droits de lecture et d'écriture
\-  MAP_SHARED : d'autres processus peuvent intérégir avec la zone mémoire mappée, les modifications sont donc partagées entre les processus ayant un accès à la map. De plus on propage les modifications sur le fichier sous jacent.
\-  mmap_fd : file descriptor de la zone à mapper
\-  BCM2835_GPIO_BASE (= 0x20000000 + 0x200000) : offset physique à partir duquel on map la mémoire (on map les registres gpio, qui se trouvent à l'adresse BCM2835_GPIO_BASE dans /dev/mem)

- Que fait la fonction delay() ?
\- Elle permet de mettre en pause le programme pendant un temps donné.

- Pourquoi doit-on utiliser sudo ?
\- On a besoin des droits de super utilisateurs pour faire un mapping de /dev/mem. (on accède à des données sensibles)

## Expériences réalisées

### SSH et Génération de clés

### Notions apprises
which -> permet de vérifier le répertoire de l'exécutable d'une commande

bashrc -> code exécuté au démarrage d'un shell intéractif (et non login, donc pas au démarrage de la machine) (permet d'ajouter des alias par exemple)

-static -> permet d'écrire le code des fichiers inclus, dans le fichier binaire exécutable (par exemple, inclure la fonction printf, qui existe dans la glibc, donc une machine n'ayant pas installé la glibc ne pourra pas utiliser printf sans une compilation static)

mmap -> renvoie un pointeur (virtuel) du mapping d'un fichier (ouvert dans un file descriptor) accessible comme un pointeur classique, et permet de modifier le contenu du fichier mappé par ce pointeur.
On utilise mmap pour faire un mapping du dossier "dev/mem" qui correspond à la mémoire du GPIO.
On peut après ça utiliser le mapping pour accéder à ces adresses et les modifier.