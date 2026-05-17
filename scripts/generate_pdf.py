import pandas as pd

COMMUNES_CSV = "assets/communes-france-metrople-2025.csv"
SOLUTION_CSV = "assets/best_solution_mock.csv"
OUTPUT_PDF   = "rapport_meilleure_generation.pdf"

# D'après le sujet : Dimensionner les hoptiaux en nombre de lits pour garantir une moyenne de 5.40 lits d’hopitaux partout sur le territoire.
LITS_PER_1000 = 5.40

# Préciser ;car pas d'en-tête dans le csv
COMMUNES_COLS = [
    "insee_code", "nom", "region_code", "region_nom", "dept_code", "dept_nom", "code_postal", "population", "latitude", "longitude"
]

# Ne pas laisser panda deviner les types pour éviter les erreurs
COMMUNES_DTYPE = {"insee_code": int, "dept_code": int, "population": int}
SOLUTION_DTYPE = {"insee_code": int, "a_hopital": int, "est_chru": int, "est_couverte": int}

# Retourne un tableau pandas de toutes les datas
def load_data():
    communes = pd.read_csv(
        COMMUNES_CSV,
        header=None,
        names=COMMUNES_COLS,
        # Gain de temps et mémoire car on sait ce dont on a besoin
        usecols=["insee_code", "nom", "dept_code", "dept_nom", "population"],
        dtype=COMMUNES_DTYPE,
    )
    solution = pd.read_csv(SOLUTION_CSV, dtype=SOLUTION_DTYPE)

# Comme une jointure SQL (how="left" garde toutes les lignes de solution)
    df = solution.merge(communes, on="insee_code", how="left")
    return df

# Retourne un tableau pandas (insee_code, nb_lits) pour chaque hôpital.
def compute_lits(df):
    pop_par_hop = (
        # Garde que les communes couvertes par un hôpital.
        df[df["est_couverte"] == 1]
        # Regroupe par hôpital et somme la population de toutes les communes qu'il dessert.
        .groupby("hopital_desservant")["population"].sum()
        .reset_index()
        .rename(columns={"hopital_desservant": "insee_code", "population": "pop_desservie"})
    )
    hopitaux = df[df["a_hopital"] == 1][["insee_code"]].merge(
        pop_par_hop, on="insee_code", how="left"
    )
    # Calcule le nombre de lits (d'après le sujet)
    hopitaux["nb_lits"] = (hopitaux["pop_desservie"] * LITS_PER_1000 / 1000).round().astype(int)
    return hopitaux

# Construit une liste de dictionnaires, un par département.
def build_dept_stats(df, lits_df):
    stats = []
    for (dept_code, dept_nom), groupe in df.groupby(["dept_code", "dept_nom"]):
        desert = groupe[groupe["est_couverte"] == 0]
        hop_du_dept = groupe[groupe["a_hopital"] == 1][["insee_code", "nom"]].merge(
            lits_df[["insee_code", "nb_lits"]], on="insee_code", how="left"
        )

        pop_totale  = groupe["population"].sum()
        total_lits  = hop_du_dept["nb_lits"].sum()

        # Dictionnaire de stats par département
        stats.append({
            "dept_code":           dept_code,
            "dept_nom":            dept_nom,
            "nb_hop":              int(groupe["a_hopital"].sum()),
            "nb_chru":             int(groupe["est_chru"].sum()),
            "nb_communes_desert":  len(desert),
            "habitants_desert":    int(desert["population"].sum()),
            "pop_totale":          int(pop_totale),
            "total_lits":          int(total_lits),
            "taux_lits":           (total_lits / pop_totale * 1000) if pop_totale > 0 else 0,
            "hopitaux":            hop_du_dept,
        })
    return stats


def main():
    df      = load_data()
    lits_df = compute_lits(df)
    stats   = build_dept_stats(df, lits_df)
    print(f"{len(stats)} départements traités.")
    for dept in stats:
        print(f"  Dept {dept['dept_code']} {dept['dept_nom']} : {dept['nb_hop']} hôpitaux, {dept['habitants_desert']} habitants en désert.")


if __name__ == "__main__":
    main()
