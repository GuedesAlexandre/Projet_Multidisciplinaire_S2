# Parseur CSV - Explications

## Conception du parseur CSV

Le parseur est implémenté dans `parser_csv.c` et expose une API simple via `parser_csv.h` :

- `CsvParserError` : un enum pour représenter les différents codes d’erreur (OK, erreur d’I/O, erreur mémoire, format invalide) ce qui facilite la compréhension de ces dernières.
***cleanup: est un label de fin de fonction. Il permet de regrouper la fermeture du fichier et la libération de la mémoire au même endroit, afin d’éviter de dupliquer ce code dans plusieurs branches d’erreur.***
- `csv_parser_parse_file(path, out)` : lit un fichier CSV et remplit une `MunicipalityTable`.
- `csv_parser_strerror(err)` : permet de convertir un code d’erreur en message lisible.
- Le `MunicipalityTable` généré par le parseur sert ensuite de base de données fixe pour l’algorithme génétique, qui ne manipule plus que des indices de communes plutôt que du CSV pur.

### Stratégie de parsing d’une ligne

Le parsing de chaque ligne se fait en plusieurs étapes :

- Lecture d’une ligne dans un buffer fixe (`LINE_BUF`), puis suppression des caractères de fin de ligne (`\n`, `\r`). ***LINE_BUF c'est la taille du buffer de lecture d’une ligne CSV. Comme le parsing se fait ligne par ligne avec fgets, on a besoin d’un espace temporaire pour stocker chaque ligne avant son découpage.***
- Découpage de la ligne sur le caractère `','` via la fonction `split_line`, qui remplace les virgules par des `'\0'` et remplit un tableau de pointeurs vers chaque champ.
- Vérification du nombre de champs : si la ligne ne contient pas au moins `NUM_FIELDS` champs, la ligne est considérée comme invalide.
- Conversion des champs en leurs types corespondants dans `parse_row` :
  - Utilisation de `atoi` / `atof` pour les entiers et les flottants.
  - Utilisation de `safe_copy` pour copier les chaînes dans les buffers `name` et `dept_name` en garantissant la terminaison par `'\0'` et en évitant les dépassements de tampon (`strncpy` + écriture manuelle du `'\0'`).

Les lignes mal formatées ne provoquent pas l’arrêt du parsing : elles sont simplement données sur `stderr` et ignorées, ce qui rend l’import plus robuste en présence de données imparfaites.

### Gestion de la mémoire et tableau dynamique

La table de communes est construite de manière dynamique au fur et à mesure de la lecture du fichier :

- Au début du parsing, `items` est `NULL`, `count` et `capacity` sont initialisés à 0.
- Avant d’ajouter une nouvelle commune, on vérifie si `count >= capacity`. Si oui, on appelle la fonction interne `table_grow`.
- `table_grow` applique une stratégie d’agrandissement exponentielle. Si la capacité est nulle, elle commence à `INITIAL_CAP` (512), sinon elle double la capacité actuelle. Cela limite le nombre d’appels à `realloc` et offre de meilleures performances qu’une croissance linéaire. ***Elle est suffisante pour notre cas où l’on charge une table de 35 000 communes.***
- En cas d’échec de `realloc`, la fonction retourne une erreur mémoire, un message d’erreur est affiché, et on nettoie proprement la table via `municipality_table_free`.

Ce design sépare les responsabilités : le parseur gère l’allocation et la croissance du tableau, tandis que le reste du programme consomme simplement une `MunicipalityTable` déjà remplie.

### Gestion des erreurs

La fonction `csv_parser_parse_file` renvoie un `CsvParserError` qui permet à l’appelant de savoir si le parsing a réussi ou non et pourquoi :

- Erreur d’ouverture/lecture de fichier (I/O).
- Erreur d’allocation mémoire.
- Format invalide pour une ligne (gérée de façon non bloquante en loggant la ligne et en continuant).

La fonction `csv_error_mapper` est une fonction qui est utilisée seulement pour le debogage.
