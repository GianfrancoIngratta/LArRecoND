#!/usr/bin/env python
"""
Macro per plottare le CaloHit di MuonParticles3D, MuonParticles3D_daughter
e DeltaRayParticles3D sovrapposte alla geometria (proiezioni ZX e ZY).

Uso:
    python plot_calohits.py <LArRecoND_file.root> <file_pfos_info.json> <path_to_geo.txt> [--xz-only] [--out output.pdf] [--n-events 50]

Ogni evento viene salvato come pagina separata di un unico file PDF.
"""

import re
import json
import argparse
import os

import pandas as pd
import uproot as upr
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.backends.backend_pdf import PdfPages
from tqdm import tqdm


# ----------------------------------------------------------------------------------------------
# Geometria
# ----------------------------------------------------------------------------------------------

def get_geo_2D(filename, plot2d=False, XZ_only=False, edgecolor="black"):

    with open(filename) as f:
        text = f.read()

    # Match both normal numbers and scientific notation, e.g.
    # -48.5587
    # -4.04121e-14
    # 1.23E+05
    number = r"[-+]?(?:\d*\.?\d+)(?:[eE][-+]?\d+)?"

    pattern = re.compile(
        rf"dimensions:\s*dx\s*=\s*({number}),\s*"
        rf"dy\s*=\s*({number}),\s*"
        rf"dz\s*=\s*({number})\s*"
        rf"global coordinates:\s*X\s*=\s*({number}),\s*"
        rf"Y\s*=\s*({number}),\s*"
        rf"Z\s*=\s*({number})",
        re.MULTILINE
    )

    starts = []
    stops = []

    for match in pattern.finditer(text):

        dx, dy, dz, X, Y, Z = map(float, match.groups())

        # Convert full dimensions to half-dimensions
        dx /= 2.
        dy /= 2.
        dz /= 2.

        # Global coordinates -> lower and upper corners
        start = np.array([X - dx, Y - dy, Z - dz])
        stop = np.array([X + dx, Y + dy, Z + dz])

        starts.append(start)
        stops.append(stop)

    # Initialize return values
    fig = None
    ax = None

    if plot2d:

        if XZ_only:

            fig, ax = plt.subplots(1, 1, figsize=(6, 10))

            for start, stop in zip(starts, stops):
                x0, y0, z0 = start
                x1, y1, z1 = stop

                # Z-X projection
                ax.add_patch(
                    patches.Rectangle(
                        (z0, x0), z1 - z0, x1 - x0,
                        fill=False, edgecolor=edgecolor
                    )
                )

            ax.set_xlabel("Z [cm]", fontsize=20)
            ax.set_ylabel("X [cm]", fontsize=20)
            ax.set_title("ZX projection", fontsize=20)

            ax.autoscale()
            ax.set_aspect("equal", adjustable="box")

            plt.tight_layout()

        else:

            fig, ax = plt.subplots(
                1, 2,
                figsize=(20, 10),
                gridspec_kw={'width_ratios': [1.7, 1]}
            )

            for start, stop in zip(starts, stops):
                x0, y0, z0 = start
                x1, y1, z1 = stop

                # Z-X projection
                ax[0].add_patch(
                    patches.Rectangle(
                        (z0, x0), z1 - z0, x1 - x0,
                        fill=False, edgecolor=edgecolor
                    )
                )

                # Z-Y projection
                ax[1].add_patch(
                    patches.Rectangle(
                        (z0, y0), z1 - z0, y1 - y0,
                        fill=False, edgecolor=edgecolor
                    )
                )

            ax[0].set_xlabel("Z [cm]", fontsize=20)
            ax[0].set_ylabel("X [cm]", fontsize=20)
            ax[0].set_title("ZX projection", fontsize=20)

            ax[1].set_xlabel("Z [cm]", fontsize=20)
            ax[1].set_ylabel("Y [cm]", fontsize=20)
            ax[1].set_title("ZY projection", fontsize=20)

            ax[0].autoscale()
            ax[1].autoscale()

            ax[0].set_aspect("equal", adjustable="box")
            ax[1].set_aspect("equal", adjustable="box")

            plt.tight_layout()

    return (starts, stops), fig, ax


# ----------------------------------------------------------------------------------------------
# Lettura CaloHits
# ----------------------------------------------------------------------------------------------

def load_calohits_multi(filepath, predicates):
    """
    Read a JSON-per-line file (with optional trailing commas) and keep
    only the lines that satisfy ALL the given predicates (logical AND).

    Parameters
    ----------
    filepath : str
        Path to the input file.
    predicates : list of callables
        Each predicate takes the parsed dict (a single line/object) and
        returns True/False. A line is kept only if every predicate
        returns True for it.

    Returns
    -------
    pandas.DataFrame
        One row per line that matched all predicates.
    """
    records = []
    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            # remove trailing comma that makes the line invalid JSON
            # when parsed on its own
            if line.endswith(","):
                line = line[:-1]

            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                # skip malformed / broken lines
                continue

            # keep the line only if ALL predicates are satisfied
            if all(pred(obj) for pred in predicates):
                records.append(obj)

    return pd.DataFrame(records)


# ---- example predicates ----

def has_field(field_name):
    """Predicate factory: True if 'field_name' is present in the object."""
    return lambda obj: field_name in obj


def field_equals(field_name, value):
    """Predicate factory: True if obj[field_name] == value."""
    return lambda obj: obj.get(field_name) == value

# ----------------------------------------------------------------------------------------------
# Lettura RecoHits from LArRecoND
# ----------------------------------------------------------------------------------------------
def get_LArRecoND_dataframe(LArRecoND_file, event_number, load_hits=True):

    if not os.path.exists(LArRecoND_file) or os.path.getsize(LArRecoND_file) == 0:
        print(f"--> skipping {LArRecoND_file}, file do not exists or empty")
        return None

    with upr.open(LArRecoND_file) as f:

        n3DHits = f['LArRecoND'].arrays(['n3DHits'], library='np')['n3DHits'][event_number]
        sliceId = f['LArRecoND'].arrays(['sliceId'], library='np')['sliceId'][event_number]
        clusterId = f['LArRecoND'].arrays(['clusterId'], library='np')['clusterId'][event_number]
        completeness = f['LArRecoND'].arrays(['completeness'], library='np')['completeness'][event_number]
        purity = f['LArRecoND'].arrays(['purity'], library='np')['purity'][event_number]
        mcPDG = f['LArRecoND'].arrays(['mcPDG'], library='np')['mcPDG'][event_number]
        recoPDG = f['LArRecoND'].arrays(['recoPDG'], library='np')['recoPDG'][event_number]
        isClearRock = f['LArRecoND'].arrays(['isClearRockOrCosmic'], library='np')['isClearRockOrCosmic'][event_number]
        mcId = f['LArRecoND'].arrays(['mcId'], library='np')['mcId'][event_number]
        mcParentPDG = f['LArRecoND'].arrays(['mcParentPDG'], library='np')['mcParentPDG'][event_number]
        mcNuVtxX = f['LArRecoND'].arrays(['mcNuVtxX'], library='np')['mcNuVtxX'][event_number]
        mcNuVtxY = f['LArRecoND'].arrays(['mcNuVtxY'], library='np')['mcNuVtxY'][event_number]
        mcNuVtxZ = f['LArRecoND'].arrays(['mcNuVtxZ'], library='np')['mcNuVtxZ'][event_number]
        nuVtxZ = f['LArRecoND'].arrays(['nuVtxZ'], library='np')['nuVtxZ'][event_number]
        nuVtxY = f['LArRecoND'].arrays(['nuVtxY'], library='np')['nuVtxY'][event_number]
        nuVtxX = f['LArRecoND'].arrays(['nuVtxX'], library='np')['nuVtxX'][event_number]

        if load_hits:
            recoHitX = f['LArRecoND'].arrays(['recoHitX'], library='np')['recoHitX'][event_number]
            recoHitY = f['LArRecoND'].arrays(['recoHitY'], library='np')['recoHitY'][event_number]
            recoHitZ = f['LArRecoND'].arrays(['recoHitZ'], library='np')['recoHitZ'][event_number]
            data = {
                'recoHitX': recoHitX,
                'recoHitY': recoHitY,
                'recoHitZ': recoHitZ,
                'mcPDG' : np.repeat(mcPDG, n3DHits),
                'recoPDG' : np.repeat(recoPDG, n3DHits),
                'isClearRock': np.repeat(isClearRock, n3DHits),
                'sliceId': np.repeat(sliceId, n3DHits),
                'clusterId': np.repeat(clusterId, n3DHits),
                'completeness': np.repeat(completeness, n3DHits),
                'purity': np.repeat(purity, n3DHits),
                'mcId': np.repeat(mcId, n3DHits),
                'mcParentPDG': np.repeat(mcParentPDG, n3DHits),
                'mcNuVtxX': np.repeat(mcNuVtxX, n3DHits),
                'nuVtxX': np.repeat(nuVtxX, n3DHits),
                'mcNuVtxY': np.repeat(mcNuVtxY, n3DHits),
                'nuVtxY': np.repeat(nuVtxY, n3DHits),
                'mcNuVtxZ': np.repeat(mcNuVtxZ, n3DHits),
                'nuVtxZ': np.repeat(nuVtxZ, n3DHits),
                'n3DHits': np.repeat(n3DHits, n3DHits)
                }
        else:
            data = {
                'mcPDG' : mcPDG,
                'recoPDG' : recoPDG,
                'isClearRock': isClearRock,
                'sliceId': sliceId,
                'clusterId': clusterId,
                'completeness': completeness,
                'purity': purity,
                'mcId': mcId,
                'mcParentPDG': mcParentPDG,
                'mcNuVtxX': mcNuVtxX,
                'nuVtxX': nuVtxX,
                'mcNuVtxY': mcNuVtxY,
                'nuVtxY': nuVtxY,
                'mcNuVtxZ': mcNuVtxZ,
                'nuVtxZ': nuVtxZ,
                'n3DHits': n3DHits
                }

    data = pd.DataFrame(data)
    data["event"] = event_number

    return data
# ----------------------------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Plotta le CaloHit sovrapposte alla geometria (proiezioni ZX/ZY)."
    )
    parser.add_argument("LArRecoND_file", help="PandoraInterace root file")
    parser.add_argument("path_to_geo_txt", help="File di testo con la geometria del detector")
    parser.add_argument("--xz-only", action="store_true", help="Mostra solo la proiezione ZX")
    parser.add_argument("--out", default="event_displays.pdf",
                         help="Nome del file PDF multipagina in cui salvare tutti gli event display")
    parser.add_argument("--n-events", type=int, default=50,
                         help="Numero di eventi da processare (default: 50)")
    args = parser.parse_args()

    # parser.add_argument("file_pfos_info", help="File JSON-per-linea con le info dei PFO/CaloHit")
    # df = load_calohits_multi(
    #     args.file_pfos_info,
    #     predicates=[
    #         has_field("pfoListName"),
    #         field_equals("STAGE", "LArTrackParticleBuilding"),
    #     ]
    # )
    #
    # MuonParticles3D = df[df.pfoListName == "MuonParticles3D"]
    # MuonParticles3D_daughter = df[df.pfoListName == "MuonParticles3D_daughter"]
    # DeltaRayParticles3D = df[df.pfoListName == "DeltaRayParticles3D"]

    # NOTE: plotting -------------------------------------------------------------------------

    with PdfPages(args.out) as pdf:

        for event in tqdm(range(args.n_events), desc="Plotting event displays", unit="evt"):

            df_LArRecoND = get_LArRecoND_dataframe(args.LArRecoND_file, event, load_hits=True)

            if df_LArRecoND is None:
                # file mancante/vuoto: salta l'evento ma continua con gli altri
                continue

            (starts, stops), fig, ax = get_geo_2D(
                args.path_to_geo_txt,
                plot2d=True,
                XZ_only=args.xz_only
            )

            if args.xz_only:
                # con XZ_only=True, get_geo_2D ritorna un singolo Axes, non una lista
                # ax.scatter(MuonParticles3D["CaloHitZ"], MuonParticles3D["CaloHitX"],
                #            color='blue', marker='s', label="MuonParticles3D")
                # ax.scatter(MuonParticles3D_daughter["CaloHitZ"], MuonParticles3D_daughter["CaloHitX"],
                #            color='red', marker='s', label="MuonParticles3D_daughter")
                # ax.scatter(DeltaRayParticles3D["CaloHitZ"], DeltaRayParticles3D["CaloHitX"],
                #            color='black', marker='s', label="DeltaRayParticles3D")

                ax.scatter(df_LArRecoND["recoHitZ"], df_LArRecoND["recoHitX"],
                           color='blue', marker='s', s=4, label="LArRecoND hits")
                ax.scatter(df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitZ"],
                           df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitX"],
                           color='red', marker='s', s=4, label="isClearRock")

                ax.set_title(f"Event {event} - ZX projection", fontsize=20)
                ax.legend()

            else:

                # ax[0].scatter(MuonParticles3D_daughter["CaloHitZ"], MuonParticles3D_daughter["CaloHitX"],
                #               color='red', marker='s', s=1, label="MuonParticles3D_daughter")
                # ax[1].scatter(MuonParticles3D_daughter["CaloHitZ"], MuonParticles3D_daughter["CaloHitY"],
                #               color='red', marker='s', s=1, label="MuonParticles3D_daughter")
                #
                # ax[0].scatter(MuonParticles3D["CaloHitZ"], MuonParticles3D["CaloHitX"],
                #               color='blue', marker='s', s=1, label="MuonParticles3D")
                # ax[1].scatter(MuonParticles3D["CaloHitZ"], MuonParticles3D["CaloHitY"],
                #               color='blue', marker='s', s=1, label="MuonParticles3D")

                # ax[0].scatter(DeltaRayParticles3D["CaloHitZ"], DeltaRayParticles3D["CaloHitX"],
                #               color='green', marker='s', s=1, label="DeltaRayParticles3D")
                # ax[1].scatter(DeltaRayParticles3D["CaloHitZ"], DeltaRayParticles3D["CaloHitY"],
                #               color='green', marker='s', s=1, label="DeltaRayParticles3D")

                ax[0].scatter(df_LArRecoND["recoHitZ"], df_LArRecoND["recoHitX"],
                              color='blue', marker='s', s=4, label="LArRecoND hits")
                ax[1].scatter(df_LArRecoND["recoHitZ"], df_LArRecoND["recoHitY"],
                              color='blue', marker='s', s=4, label="LArRecoND hits")

                ax[0].scatter(df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitZ"],
                              df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitX"],
                              color='red', marker='s', s=4, label="isClearRock")
                ax[1].scatter(df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitZ"],
                              df_LArRecoND[df_LArRecoND["isClearRock"] == 1]["recoHitY"],
                              color='red', marker='s', s=4, label="isClearRock")

                # plot reconstructed neutrino vertex
                ax[0].scatter(df_LArRecoND[df_LArRecoND["isClearRock"] == 0]["nuVtxZ"],
                              df_LArRecoND[df_LArRecoND["isClearRock"] == 0]["nuVtxX"],
                              color='orange', marker='x', s=80, label="reco nu vertex")
                ax[1].scatter(df_LArRecoND[df_LArRecoND["isClearRock"] == 0]["nuVtxZ"],
                              df_LArRecoND[df_LArRecoND["isClearRock"] == 0]["nuVtxY"],
                              color='orange', marker='x', s=80, label="reco nu vertex")

                fig.suptitle(f"Event {event}", fontsize=20)
                ax[0].legend()
                ax[1].legend()

            # Salva questa figura come pagina del PDF, poi la chiude per liberare memoria
            pdf.savefig(fig, dpi=200, bbox_inches="tight")
            plt.close(fig)

    print(f"Plot salvati in {args.out}")


if __name__ == "__main__":
    main()
