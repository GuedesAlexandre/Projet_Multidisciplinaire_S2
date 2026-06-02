"""
Rendu des cartes de couverture médicale en images PNG (matplotlib).

Chaque fonction renvoie un tuple (png_bytes, largeur_px, hauteur_px) prêt à être
inséré dans le PDF via FPDF.image(io.BytesIO(png_bytes), ...).

On ne dépend que de matplotlib / PIL : la silhouette de la France émerge du nuage
des ~34 000 communes, sans fond de carte ni fichier de frontières externe.
"""
import io
import json
import os
from math import cos, radians

import matplotlib
matplotlib.use("Agg")           # backend hors-écran (génération de fichiers)
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from PIL import Image

# Bornes France métropolitaine hors Corse (cohérent avec src/graphics/map/map.h)
LAT_MIN, LAT_MAX = 41.3, 51.1
LON_MIN, LON_MAX = -5.2, 9.6
LAT_CENTER       = 46.5
# Correction de longitude (~0.69) : 1° de longitude vaut cos(lat) × 1° de latitude.
# En multipliant la longitude par ce facteur on peut utiliser un repère "equal".
_K = cos(radians(LAT_CENTER))

# Palette
COL_COUVERTE = "#bcd6ef"   # commune couverte par un hôpital
COL_DESERT   = "#e0492f"   # commune en désert médical
COL_HOPITAL  = "#1f2430"   # hôpital (non CHRU)
COL_CHRU     = "#f5a623"   # CHRU
COL_TITRE    = "#1e3c78"
COL_CONTOUR  = "#33415c"   # contours administratifs

# Contours administratifs (GeoJSON, source : github.com/gregoiredavid/france-geojson).
# On ne dépend que du module json : les polygones sont tracés en lignes matplotlib,
# sans geopandas ni shapely.
_GEO_DIR        = os.path.join(os.path.dirname(__file__), os.pardir, "assets")
_METROPOLE_PATH = os.path.join(_GEO_DIR, "metropole.geojson")
_DEPTS_PATH     = os.path.join(_GEO_DIR, "departements.geojson")
_metropole_cache = None
_departements_cache = None


def _polygons_of(geometry):
    """Normalise une géométrie GeoJSON en liste de polygones (liste d'anneaux)."""
    t = geometry["type"]
    if t == "Polygon":
        return [geometry["coordinates"]]
    if t == "MultiPolygon":
        return geometry["coordinates"]
    return []


def _is_corsica(polygon):
    """Vrai si le polygone correspond à la Corse (jeu de données hors Corse)."""
    lons = [pt[0] for ring in polygon for pt in ring]
    lats = [pt[1] for ring in polygon for pt in ring]
    return min(lons) > 8.3 and max(lats) < 43.3


def _bbox(polygons):
    """Boîte englobante (lon_min, lon_max, lat_min, lat_max) d'une liste de polygones."""
    lons = [pt[0] for poly in polygons for ring in poly for pt in ring]
    lats = [pt[1] for poly in polygons for ring in poly for pt in ring]
    return min(lons), max(lons), min(lats), max(lats)


def _load_metropole():
    """Contour de la France métropolitaine, Corse exclue (mis en cache)."""
    global _metropole_cache
    if _metropole_cache is None:
        with open(_METROPOLE_PATH, encoding="utf-8") as fh:
            gj = json.load(fh)
        polys = _polygons_of(gj["geometry"])
        _metropole_cache = [p for p in polys if not _is_corsica(p)]
    return _metropole_cache


def _load_departements():
    """Dict {code (str) -> liste de polygones} des contours départementaux (mis en cache)."""
    global _departements_cache
    if _departements_cache is None:
        with open(_DEPTS_PATH, encoding="utf-8") as fh:
            gj = json.load(fh)
        _departements_cache = {
            feat["properties"]["code"]: _polygons_of(feat["geometry"])
            for feat in gj["features"]
        }
    return _departements_cache


def _draw_polygons(ax, polygons, lw, color, alpha=1.0):
    """Trace les anneaux des polygones (projection longitude × _K)."""
    for poly in polygons:
        for ring in poly:
            xs = [pt[0] * _K for pt in ring]
            ys = [pt[1] for pt in ring]
            ax.plot(xs, ys, color=color, linewidth=lw, alpha=alpha,
                    solid_capstyle="round", solid_joinstyle="round")


def _split(df):
    """Sépare un dataframe communes en (couvertes, déserts, hôpitaux, CHRU)."""
    est_hop  = df["a_hopital"] == 1
    est_chru = df["est_chru"] == 1
    desert   = df["hopital_desservant"].isna() & ~est_hop
    couverte = df["hopital_desservant"].notna() & ~est_hop
    return df[couverte], df[desert], df[est_hop & ~est_chru], df[est_chru]


def _legend(ax):
    handles = [
        Line2D([], [], marker="o", linestyle="", markersize=5,
               markerfacecolor=COL_COUVERTE, markeredgecolor="none",
               label="Commune couverte"),
        Line2D([], [], marker="o", linestyle="", markersize=5,
               markerfacecolor=COL_DESERT, markeredgecolor="none",
               label="Commune en désert"),
        Line2D([], [], marker="o", linestyle="", markersize=6,
               markerfacecolor=COL_HOPITAL, markeredgecolor="none",
               label="Hôpital"),
        Line2D([], [], marker="*", linestyle="", markersize=11,
               markerfacecolor=COL_CHRU, markeredgecolor="#7a5200",
               label="CHRU"),
    ]
    # Légende placée SOUS la carte (hors zone de tracé) pour ne jamais
    # masquer les points, la carte restant centrée.
    ax.legend(handles=handles, loc="upper center", bbox_to_anchor=(0.5, -0.01),
              ncol=2, fontsize=8, frameon=True, framealpha=0.95, borderpad=0.6,
              columnspacing=1.6, handletextpad=0.4, labelspacing=0.5)


def _fig_to_png(fig):
    buf = io.BytesIO()
    fig.savefig(buf, format="png", dpi=150, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    data = buf.getvalue()
    w, h = Image.open(io.BytesIO(data)).size
    return data, w, h


def render_national_map(df):
    """Carte nationale de la couverture hospitalière."""
    couv, des, hop, chru = _split(df)
    fig, ax = plt.subplots(figsize=(7.0, 7.4))

    ax.scatter(couv["longitude"] * _K, couv["latitude"], s=0.4,
               c=COL_COUVERTE, alpha=0.5, linewidths=0)
    ax.scatter(des["longitude"] * _K, des["latitude"], s=1.2,
               c=COL_DESERT, alpha=0.7, linewidths=0)
    ax.scatter(hop["longitude"] * _K, hop["latitude"], s=7,
               c=COL_HOPITAL, linewidths=0)
    ax.scatter(chru["longitude"] * _K, chru["latitude"], s=55, marker="*",
               c=COL_CHRU, edgecolors="#7a5200", linewidths=0.4)

    # Contour de la France métropolitaine (par-dessus le nuage de points).
    metropole = _load_metropole()
    _draw_polygons(ax, metropole, lw=0.7, color=COL_CONTOUR)
    lon0, lon1, lat0, lat1 = _bbox(metropole)
    mx, my = (lon1 - lon0) * 0.02, (lat1 - lat0) * 0.02
    ax.set_xlim((lon0 - mx) * _K, (lon1 + mx) * _K)
    ax.set_ylim(lat0 - my, lat1 + my)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.set_title("Couverture hospitalière - France métropolitaine",
                 fontsize=12, fontweight="bold", color=COL_TITRE, pad=8)
    _legend(ax)
    return _fig_to_png(fig)


def render_department_map(geo, dept_code, dept_nom):
    """Mini-carte zoomée sur un département. Renvoie None s'il n'y a aucun point."""
    if geo is None or len(geo) == 0:
        return None
    couv, des, hop, chru = _split(geo)
    fig, ax = plt.subplots(figsize=(6.4, 5.2))

    ax.scatter(couv["longitude"] * _K, couv["latitude"], s=10,
               c=COL_COUVERTE, alpha=0.85, linewidths=0)
    ax.scatter(des["longitude"] * _K, des["latitude"], s=14,
               c=COL_DESERT, alpha=0.9, linewidths=0)
    ax.scatter(hop["longitude"] * _K, hop["latitude"], s=45,
               c=COL_HOPITAL, linewidths=0)
    ax.scatter(chru["longitude"] * _K, chru["latitude"], s=160, marker="*",
               c=COL_CHRU, edgecolors="#7a5200", linewidths=0.6)

    # Contour du département + cadrage sur sa géométrie (repli sur les points sinon).
    polys = _load_departements().get(f"{int(dept_code):02d}")
    if polys:
        _draw_polygons(ax, polys, lw=1.2, color=COL_CONTOUR)
        lon0, lon1, lat0, lat1 = _bbox(polys)
        mx, my = (lon1 - lon0) * 0.05 + 0.02, (lat1 - lat0) * 0.05 + 0.02
        ax.set_xlim((lon0 - mx) * _K, (lon1 + mx) * _K)
        ax.set_ylim(lat0 - my, lat1 + my)
    else:
        xs, ys = geo["longitude"] * _K, geo["latitude"]
        mx = (xs.max() - xs.min()) * 0.06 + 0.05
        my = (ys.max() - ys.min()) * 0.06 + 0.05
        ax.set_xlim(xs.min() - mx, xs.max() + mx)
        ax.set_ylim(ys.min() - my, ys.max() + my)
    ax.set_aspect("equal")
    ax.axis("off")
    ax.set_title(f"Couverture - {dept_nom} ({dept_code})",
                 fontsize=11, fontweight="bold", color=COL_TITRE, pad=6)
    _legend(ax)
    return _fig_to_png(fig)
