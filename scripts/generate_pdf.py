import io

import pandas as pd
from fpdf import FPDF
from datetime import date

from map_render import render_national_map, render_department_map

COMMUNES_CSV = "assets/communes-france-metrople-2025.csv"
SOLUTION_CSV = "assets/best_solution_mock.csv"
OUTPUT_PDF   = "rapport_meilleure_generation.pdf"

# D'après le sujet : Dimensionner les hoptiaux en nombre de lits pour garantir une moyenne de 5.40 lits d'hopitaux partout sur le territoire + https://fr.tradingeconomics.com/france/hospital-beds.
LITS_PER_1000 = 5.40
SEUIL_CHRU    = 80000   # d'après le sujet : hôpital dans une ville de plus de 80 000 hab. = CHRU

# Préciser ;car pas d'en-tête dans le csv
COMMUNES_COLS = [
    "insee_code", "nom", "region_code", "region_nom", "dept_code", "dept_nom", "code_postal", "population", "latitude", "longitude"
]

# Ne pas laisser panda deviner les types pour éviter les erreurs
COMMUNES_DTYPE = {"insee_code": int, "dept_code": int, "population": int,
                  "latitude": float, "longitude": float}
SOLUTION_DTYPE = {"insee_code": int, "a_hopital": int, "est_chru": int}

# Retourne un tableau pandas de toutes les datas
def load_data():
    communes = pd.read_csv(
        COMMUNES_CSV,
        header=None,
        names=COMMUNES_COLS,
        # Gain de temps et mémoire car on sait ce dont on a besoin
        # latitude/longitude servent au tracé des cartes de couverture.
        usecols=["insee_code", "nom", "dept_code", "dept_nom", "population",
                 "latitude", "longitude"],
        dtype=COMMUNES_DTYPE,
    )
    solution = pd.read_csv(SOLUTION_CSV, dtype=SOLUTION_DTYPE)

    # Comme une jointure SQL (how="left" garde toutes les lignes de solution)
    df = solution.merge(communes, on="insee_code", how="left")
    return df

# Retourne un tableau pandas (insee_code, pop_desservie, nb_lits) pour chaque hôpital.
def compute_lits(df):
    pop_par_hop = (
        # Garde que les communes couvertes par un hôpital.
        df[df["hopital_desservant"].notna()]
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
        desert = groupe[groupe["hopital_desservant"].isna()]
        hop_du_dept = groupe[groupe["a_hopital"] == 1][["insee_code", "nom", "est_chru", "population"]].merge(
            lits_df[["insee_code", "pop_desservie", "nb_lits"]], on="insee_code", how="left"
        )

        pop_totale  = groupe["population"].sum()
        total_lits  = hop_du_dept["nb_lits"].sum()

        # Commune en désert la plus peuplée
        commune_desert_max = None
        if len(desert) > 0:
            idx = desert["population"].idxmax()
            commune_desert_max = {
                "nom":        desert.loc[idx, "nom"],
                "population": int(desert.loc[idx, "population"]),
            }

        # Hôpital le plus chargé (population desservie la plus grande)
        hopital_max_charge = None
        if len(hop_du_dept) > 0 and hop_du_dept["pop_desservie"].notna().any():
            idx = hop_du_dept["pop_desservie"].idxmax()
            hopital_max_charge = {
                "nom":          str(hop_du_dept.loc[idx, "nom"]),
                "pop_desservie": int(hop_du_dept.loc[idx, "pop_desservie"]),
            }

        # Infos de stats par département
        stats.append({
            "dept_code":           dept_code,
            "dept_nom":            dept_nom,
            "nb_hop":              int(groupe["a_hopital"].sum()),
            "nb_chru":             int(groupe["est_chru"].sum()),
            "nb_communes_total":   len(groupe),
            "nb_communes_desert":  len(desert),
            "habitants_desert":    int(desert["population"].sum()),
            "pop_totale":          int(pop_totale),
            "total_lits":          int(total_lits),
            "taux_lits":           (total_lits / pop_totale * 1000) if pop_totale > 0 else 0,
            "hopitaux":            hop_du_dept,
            "commune_desert_max":  commune_desert_max,
            "hopital_max_charge":  hopital_max_charge,
            # Points géographiques du département pour la mini-carte de couverture.
            "geo":                 groupe[["latitude", "longitude", "a_hopital",
                                           "est_chru", "hopital_desservant"]],
        })
    return stats

# Stats globales en France
def build_global_stats(df, stats):
    def pct_desert(d):
        return d["habitants_desert"] / d["pop_totale"] * 100 if d["pop_totale"] > 0 else 0

    triés = sorted(stats, key=pct_desert)
    meilleur = triés[0]  if triés else None
    pire     = triés[-1] if triés else None

    return {
        "nb_depts":           len(stats),
        "nb_hop_total":       sum(d["nb_hop"] for d in stats),
        "nb_chru_total":      sum(d["nb_chru"] for d in stats),
        "nb_communes_desert": sum(d["nb_communes_desert"] for d in stats),
        "pop_desert_total":   sum(d["habitants_desert"] for d in stats),
        "pop_totale":         int(df["population"].sum()),
        "dept_meilleur":      f"{meilleur['dept_nom']} ({pct_desert(meilleur):.1f} %)" if meilleur else "—",
        "dept_pire":          f"{pire['dept_nom']} ({pct_desert(pire):.1f} %)"     if pire     else "—",
    }

def _fmt(n):
    return f"{n:,}".replace(",", " ")

class RapportPDF(FPDF):
    # Pas d'en-tête sur la page 1
    def header(self):
        if self.page_no() == 1:
            return
        self.set_font("Helvetica", "I", 8)
        self.set_text_color(150, 150, 150)
        self.cell(0, 6, "Rapport - Meilleure solution de couverture médicale", align="R")
        self.ln(2)
        self.set_draw_color(210, 210, 210)
        self.line(10, self.get_y(), 200, self.get_y())
        self.ln(4)

    def footer(self):
        if self.page_no() == 1:
            return
        self.set_y(-12)
        self.set_font("Helvetica", "I", 8)
        self.set_text_color(150, 150, 150)
        self.cell(0, 6, f"Page {self.page_no()}", align="C")

    # --- pages ---

    def page_titre(self):
        self.add_page()
        self.ln(60)
        self.set_font("Helvetica", "B", 26)
        self.set_text_color(30, 60, 120)
        self.cell(0, 14, "Rapport de couverture médicale", align="C")
        self.ln(16)
        self.set_font("Helvetica", "B", 16)
        self.set_text_color(70, 70, 70)
        self.cell(0, 10, "Meilleure solution de la génération", align="C")
        self.ln(24)
        self.set_draw_color(30, 60, 120)
        self.line(40, self.get_y(), 170, self.get_y())
        self.ln(12)
        self.set_font("Helvetica", "", 11)
        self.set_text_color(110, 110, 110)
        self.cell(0, 7, f"Généré le {date.today().strftime('%d/%m/%Y')}", align="C")
        self.ln(7)
        self.cell(0, 7, "Projet multidisciplinaire S2 - ESIEE Paris", align="C")

    def page_resume_global(self, gs):
        self.add_page()
        self._titre_section("Résumé global")

        pct_desert = gs["pop_desert_total"] / gs["pop_totale"] * 100 if gs["pop_totale"] > 0 else 0
        items = [
            ("Départements analysés",            str(gs["nb_depts"])),
            ("Hôpitaux",                          str(gs["nb_hop_total"])),
            ("dont CHRU",                         str(gs["nb_chru_total"])),
            ("Communes en désert médical",        str(gs["nb_communes_desert"])),
            ("Population totale du territoire",   _fmt(gs["pop_totale"])),
            ("Population en désert médical",      _fmt(gs["pop_desert_total"])),
            ("Part de la population en désert",   f"{pct_desert:.1f} %"),
            ("Département le mieux couvert",      gs["dept_meilleur"]),
            ("Département le moins bien couvert", gs["dept_pire"]),
        ]
        for label, val in items:
            self._ligne_stat(label, val)

    def page_carte_et_classement(self, stats, nat_map):
        self.add_page()
        self._titre_section("Carte de couverture")

        if nat_map:
            self._placer_image(*nat_map, max_w=150, max_h=130)
        else:
            # Repli si la carte n'a pas pu être générée.
            y_carte = self.get_y()
            self.set_fill_color(240, 242, 248)
            self.set_draw_color(180, 190, 210)
            self.rect(10, y_carte, 190, 95, style="FD")
            self.set_font("Helvetica", "I", 11)
            self.set_text_color(160, 160, 180)
            self.set_xy(10, y_carte + 42)
            self.cell(190, 10, "[ Carte de couverture indisponible ]", align="C")
            self.set_y(y_carte + 100)

        self.ln(4)
        self._titre_section("Classement des départements par couverture")
        self._tableau_classement(stats)

    def page_departement(self, dept):
        self.add_page()
        self._titre_section(f"Département {dept['dept_code']} - {dept['dept_nom']}")

        pct      = dept["habitants_desert"] / dept["pop_totale"] * 100 if dept["pop_totale"] > 0 else 0
        pct_com  = dept["nb_communes_desert"] / dept["nb_communes_total"] * 100 if dept["nb_communes_total"] > 0 else 0
        couvertes = dept["nb_communes_total"] - dept["nb_communes_desert"]
        items = [
            ("Population totale",           _fmt(dept["pop_totale"])),
            ("Hôpitaux",                    str(dept["nb_hop"])),
            ("dont CHRU",                   str(dept["nb_chru"])),
            ("Communes couvertes",          f"{couvertes} / {dept['nb_communes_total']} ({100 - pct_com:.1f} %)"),
            ("Communes en désert",          str(dept["nb_communes_desert"])),
            ("Habitants en désert",         _fmt(dept["habitants_desert"])),
            ("Part en désert",              f"{pct:.1f} %"),
            ("Total lits",                  str(dept["total_lits"])),
            ("Taux lits / 1 000 hab.",     f"{dept['taux_lits']:.2f}"),
        ]
        for label, val in items:
            self._ligne_stat(label, val)

        self.ln(3)
        self._titre_sous_section("Carte du département")
        dept_map = render_department_map(dept.get("geo"), dept["dept_code"],
                                         dept["dept_nom"])
        if dept_map:
            self._placer_image(*dept_map, max_w=120, max_h=95)

        self.ln(3)
        self._titre_sous_section("Hôpitaux du département")
        self._tableau_hopitaux(dept["hopitaux"])
        self.ln(3)
        self._analyse_couverture(dept)

    # --- blocs réutilisables ---

    def _tableau_classement(self, stats):
        # Trie du département le mieux couvert (moins de désert) au moins bien couvert
        tries = sorted(
            stats,
            key=lambda d: d["habitants_desert"] / d["pop_totale"] if d["pop_totale"] > 0 else 0,
        )
        cols = [
            ("Rang", 14), ("Dép.", 14), ("Département", 58),
            ("Hôpitaux", 24), ("Lits/1 000 hab.", 36), ("% pop. en désert", 44),
        ]
        self._entete_tableau(cols)
        for rang, dept in enumerate(tries, 1):
            pct = dept["habitants_desert"] / dept["pop_totale"] * 100 if dept["pop_totale"] > 0 else 0
            self._ligne_tableau(cols, [
                str(rang),
                str(dept["dept_code"]),
                dept["dept_nom"],
                str(dept["nb_hop"]),
                f"{dept['taux_lits']:.2f}",
                f"{pct:.1f} %",
            ], rang)

    def _tableau_hopitaux(self, hopitaux_df):
        cols = [("INSEE", 22), ("Commune", 68), ("CHRU", 16), ("Pop. desservie", 38), ("Nb lits", 26)]
        self._entete_tableau(cols)
        for i, (_, row) in enumerate(hopitaux_df.iterrows()):
            chru = "Oui" if row.get("est_chru", 0) == 1 else "Non"
            pop  = _fmt(int(row["pop_desservie"])) if pd.notna(row.get("pop_desservie")) else "—"
            lits = str(int(row["nb_lits"])) if pd.notna(row.get("nb_lits")) else "—"
            self._ligne_tableau(cols, [
                str(int(row["insee_code"])),
                str(row["nom"]),
                chru,
                pop,
                lits,
            ], i)

    def _analyse_couverture(self, dept):
        hopitaux_df = dept["hopitaux"]

        # --- CHRU ---
        self._titre_sous_section("Analyse de la couverture CHRU")
        self._analyse_chru(hopitaux_df)
        self.ln(2)

        # --- Commune en désert la plus peuplée ---
        self._titre_sous_section("Commune en désert la plus peuplée")
        self.set_font("Helvetica", "", 10)
        if dept["commune_desert_max"]:
            c = dept["commune_desert_max"]
            self.set_text_color(180, 60, 0)
            self.multi_cell(0, 6,
                f"{c['nom']} : {_fmt(c['population'])} habitants sans couverture hospitalière.")
        else:
            self.set_text_color(0, 130, 0)
            self.multi_cell(0, 6, "Aucune commune en désert médical dans ce département.")
        self.set_text_color(30, 30, 30)
        self.ln(2)

        # --- Hôpital le plus sollicité ---
        self._titre_sous_section("Hôpital le plus sollicité")
        self.set_font("Helvetica", "", 10)
        if dept["hopital_max_charge"]:
            h = dept["hopital_max_charge"]
            self.set_text_color(30, 30, 30)
            self.multi_cell(0, 6,
                f"{h['nom']} dessert {_fmt(h['pop_desservie'])} habitants "
                f"(charge la plus élevée du département).")
        else:
            self.set_text_color(150, 150, 150)
            self.multi_cell(0, 6, "Aucun hôpital dans ce département.")
        self.set_text_color(30, 30, 30)
        self.ln(2)

        # --- Taux de lits vs norme nationale ---
        self._titre_sous_section(f"Taux de lits vs norme nationale ({LITS_PER_1000} / 1 000 hab.)")
        self.set_font("Helvetica", "", 10)
        if dept["pop_totale"] > 0 and dept["total_lits"] > 0:
            ecart = dept["taux_lits"] - LITS_PER_1000
            if ecart >= 0:
                self.set_text_color(0, 130, 0)
                self.multi_cell(0, 6,
                    f"Taux de {dept['taux_lits']:.2f} lits/1 000 hab. "
                    f"(+{ecart:.2f} vs norme) : couverture satisfaisante.")
            else:
                self.set_text_color(180, 60, 0)
                self.multi_cell(0, 6,
                    f"Taux de {dept['taux_lits']:.2f} lits/1 000 hab. "
                    f"({ecart:.2f} vs norme) : sous la norme nationale.")
        else:
            self.set_text_color(180, 60, 0)
            self.multi_cell(0, 6, "Aucun lit recensé dans ce département.")
        self.set_text_color(30, 30, 30)

    def _analyse_chru(self, hopitaux_df):
        # hôpital dans une ville de plus de 80 000 hab. = CHRU obligatoire
        manquants = [
            (row["nom"], int(row["population"]))
            for _, row in hopitaux_df.iterrows()
            if row.get("est_chru", 0) == 0 and row["population"] > SEUIL_CHRU
        ]
        self.set_font("Helvetica", "", 10)
        if not manquants:
            self.set_text_color(0, 130, 0)
            self.multi_cell(0, 6, "Couverture CHRU satisfaisante pour ce département.")
        else:
            self.set_text_color(180, 60, 0)
            for nom, pop in manquants:
                self.multi_cell(0, 6, f"- {nom} ({_fmt(pop)} hab.) devrait avoir le statut CHRU.")
        self.set_text_color(30, 30, 30)

    # --- helpers mise en page ---

    def _placer_image(self, png, iw, ih, max_w, max_h):
        # Insère une image PNG (depuis ses octets) centrée, en respectant son
        # ratio et en tenant dans la boîte max_w x max_h (mm).
        ratio = ih / iw
        w = max_w
        h = w * ratio
        if h > max_h:
            h = max_h
            w = h / ratio
        # Saut de page si l'image ne tient pas dans l'espace restant.
        if self.get_y() + h > self.h - self.b_margin:
            self.add_page()
        x = (self.w - w) / 2          # centré sur la largeur de la page
        y = self.get_y()
        self.image(io.BytesIO(png), x=x, y=y, w=w, h=h)
        self.set_y(y + h)

    def _titre_section(self, texte):
        self.set_font("Helvetica", "B", 13)
        self.set_text_color(30, 60, 120)
        self.cell(0, 8, texte)
        self.ln(9)
        self.set_draw_color(30, 60, 120)
        self.line(10, self.get_y() - 1, 200, self.get_y() - 1)
        self.ln(5)
        self.set_text_color(30, 30, 30)

    def _titre_sous_section(self, texte):
        self.set_font("Helvetica", "B", 11)
        self.set_text_color(60, 60, 60)
        self.cell(0, 7, texte)
        self.ln(9)

    def _ligne_stat(self, label, valeur):
        self.set_font("Helvetica", "", 10)
        self.set_text_color(90, 90, 90)
        self.cell(85, 7, label + " :")
        self.set_font("Helvetica", "B", 10)
        self.set_text_color(20, 20, 20)
        self.cell(0, 7, valeur)
        self.ln(7)

    def _entete_tableau(self, cols):
        self.set_fill_color(30, 60, 120)
        self.set_text_color(255, 255, 255)
        self.set_font("Helvetica", "B", 9)
        for label, width in cols:
            self.cell(width, 7, label, border=1, fill=True)
        self.ln()

    def _ligne_tableau(self, cols, values, idx):
        self.set_fill_color(235, 240, 250) if idx % 2 == 0 else self.set_fill_color(255, 255, 255)
        self.set_text_color(30, 30, 30)
        self.set_font("Helvetica", "", 9)
        for (_, width), val in zip(cols, values):
            self.cell(width, 6, val, border=1, fill=True)
        self.ln()


def generate_pdf(stats, global_stats, df):
    pdf = RapportPDF(orientation="P", unit="mm", format="A4")
    pdf.set_auto_page_break(auto=True, margin=15)
    pdf.set_margins(10, 15, 10)

    nat_map = render_national_map(df)

    pdf.page_titre()
    pdf.page_resume_global(global_stats)
    pdf.page_carte_et_classement(stats, nat_map)
    for dept in stats:
        pdf.page_departement(dept)

    pdf.output(OUTPUT_PDF)
    print(f"PDF généré : {OUTPUT_PDF}")


def main():
    df           = load_data()
    lits_df      = compute_lits(df)
    stats        = build_dept_stats(df, lits_df)
    global_stats = build_global_stats(df, stats)
    generate_pdf(stats, global_stats, df)


if __name__ == "__main__":
    main()
