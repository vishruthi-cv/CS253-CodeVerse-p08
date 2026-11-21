#!/usr/bin/env python3
"""scripts/plot.py -- make the Project 8 figures from the benchmark CSV.

Usage:  python scripts/plot.py [results/raw/bench.csv] [results/figures]

Reads the CSV written by bench/bench.cpp. Rep 0 of every configuration is the
warm-up and is used only for memory. Reps 1.. are the timed runs; the plots show
their mean with a +/- 1 standard deviation error bar.

"Predicted" curves come from the cost model in REPORT.md and are calibrated at ONE
point (the smallest w), so only their shape is a prediction, not their height.
"""
import os
import sys
import textwrap

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

CSV = sys.argv[1] if len(sys.argv) > 1 else "results/raw/bench.csv"
OUT = sys.argv[2] if len(sys.argv) > 2 else "results/figures"
os.makedirs(OUT, exist_ok=True)

# Colours: first two slots of the validated categorical palette (blue, orange).
COLOR = {"A": "#2a78d6", "B": "#eb6834"}
LABEL = {"A": "Structure A (two heaps)", "B": "Structure B (Fenwick tree)"}
INK, INK2, GRID, SURFACE = "#0b0b0b", "#52514e", "#e6e5e1", "#fcfcfb"

df = pd.read_csv(CSV)
timed = df[df.rep >= 1]
mem = df[(df.rep == 0) & (df.mem_kb >= 0)]
SOURCE = f"Source: scripts/plot.py, {CSV}"


def style(ax, xlabel, ylabel, title):
    ax.set_facecolor(SURFACE)
    ax.set_title(title, loc="left", fontsize=12, color=INK)
    ax.set_xlabel(xlabel, color=INK2)
    ax.set_ylabel(ylabel, color=INK2)
    ax.grid(True, color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    for s in ("top", "right"):
        ax.spines[s].set_visible(False)
    for s in ("left", "bottom"):
        ax.spines[s].set_color(INK2)
    ax.tick_params(colors=INK2)


def finish(fig, ax, name, note):
    ax.legend(frameon=False, labelcolor=INK2, fontsize=9)
    note = "\n".join(textwrap.wrap(note, 120))
    fig.text(0.01, 0.01, f"{note}\n{SOURCE}", fontsize=7, color=INK2, va="bottom")
    fig.tight_layout(rect=(0, 0.12, 1, 1))
    fig.savefig(os.path.join(OUT, name), dpi=150, facecolor=SURFACE)
    plt.close(fig)
    print("wrote", os.path.join(OUT, name))


def stats(sub, x, y):
    g = sub.groupby(["structure", x])[y].agg(["mean", "std", "count"]).reset_index()
    g["std"] = g["std"].fillna(0.0)
    return g


def draw(ax, g, x):
    for s in ("A", "B"):
        d = g[g.structure == s].sort_values(x)
        if len(d):
            ax.errorbar(d[x], d["mean"], yerr=d["std"], color=COLOR[s], label=LABEL[s],
                        linewidth=2, marker="o", markersize=8, capsize=3)


def make_time_vs_w():
    e = timed[timed.exp == "E1_w"]
    if e.empty:
        return
    g = stats(e, "w", "ns_per_step")
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "w")
    w0 = g.w.min()
    ws = np.array(sorted(g.w.unique()), dtype=float)
    a0 = g[(g.structure == "A") & (g.w == w0)]["mean"].iloc[0]
    b0 = g[(g.structure == "B") & (g.w == w0)]["mean"].iloc[0]
    ax.plot(ws, a0 * np.log2(ws) / np.log2(w0), "--", color=COLOR["A"], linewidth=2,
            label="A predicted: grows like log2 w")
    ax.plot(ws, np.full_like(ws, b0), "--", color=COLOR["B"], linewidth=2,
            label="B predicted: flat in w")
    ax.set_xscale("log")
    ax.set_ylim(bottom=0)
    style(ax, "Window size w (readings)", "Time per step (ns)",
          "Time per step against window size")
    finish(fig, ax, "fig1_time_vs_w.png",
           "Mean of the timed runs, error bars = 1 standard deviation. Predicted curves are "
           "calibrated at the smallest w.")


def make_time_vs_r():
    e = timed[timed.exp == "E2_r"]
    if e.empty:
        return
    g = stats(e, "r_pct", "ns_per_step")
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "r_pct")
    ax.set_ylim(bottom=0)
    style(ax, "Retraction rate r (% of steps that retract a reading)", "Time per step (ns)",
          "Time per step against retraction rate")
    finish(fig, ax, "fig2_time_vs_r.png",
           "Mean of the timed runs, error bars = 1 standard deviation.")


def make_mem_vs_w():
    e = mem[mem.exp == "E1_w"]
    if e.empty:
        return
    g = e.groupby(["structure", "w"])["mem_kb"].mean().reset_index().rename(
        columns={"mem_kb": "mean"})
    g["std"] = 0.0
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "w")
    wmax = g.w.max()
    ws = np.array(sorted(g.w.unique()), dtype=float)
    a1 = g[(g.structure == "A") & (g.w == wmax)]["mean"].iloc[0]
    b0 = g[(g.structure == "B") & (g.w == g.w.min())]["mean"].iloc[0]
    ax.plot(ws, a1 * ws / wmax, "--", color=COLOR["A"], linewidth=2,
            label="A predicted: proportional to w")
    ax.plot(ws, np.full_like(ws, b0), "--", color=COLOR["B"], linewidth=2,
            label="B predicted: flat (set by U)")
    ax.set_xscale("log")
    ax.set_yscale("log")
    style(ax, "Window size w (readings)", "Extra resident memory (KB)",
          "Memory against window size")
    finish(fig, ax, "fig3_memory_vs_w.png",
           "Resident memory added by the structure (one run per point). A's prediction is "
           "calibrated at the largest w, B's at the smallest.")


def make_mem_vs_r():
    e = mem[mem.exp == "E2_r"]
    if e.empty:
        return
    g = e.groupby(["structure", "r_pct"])["mem_kb"].mean().reset_index().rename(
        columns={"mem_kb": "mean"})
    g["std"] = 0.0
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "r_pct")
    ax.set_ylim(bottom=0)
    style(ax, "Retraction rate r (% of steps that retract a reading)",
          "Extra resident memory (KB)", "Memory against retraction rate")
    finish(fig, ax, "fig4_memory_vs_r.png",
           "Resident memory added by the structure (one run per point).")


def make_purges():
    e = timed[(timed.exp == "E2_r") & (timed.structure == "A")]
    if e.empty:
        return
    g = e.groupby("r_pct").agg(purges=("purges", "mean"), phys=("max_physical", "mean"),
                               w=("w", "first")).reset_index()
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    ax.plot(g.r_pct, g.purges, color=COLOR["A"], linewidth=2, marker="o", markersize=8,
            label="Purges per run")
    ax.set_ylim(bottom=0)
    style(ax, "Retraction rate r (% of steps that retract a reading)", "Number of purges",
          "Structure A: purges against retraction rate")
    finish(fig, ax, "fig5_A_purges_vs_r.png",
           "Mean over the timed runs. Largest number of entries seen in A's heaps, divided by w: "
           + ", ".join(f"r={int(r)}%: {p / w:.2f}" for r, p, w in zip(g.r_pct, g.phys, g.w))
           + "  (prediction: below about 2).")


def make_time_vs_U():
    e = timed[timed.exp == "E3_U"]
    if e.empty:
        return
    g = stats(e, "U", "ns_per_step")
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "U")
    u0 = g.U.min()
    us = np.array(sorted(g.U.unique()), dtype=float)
    b0 = g[(g.structure == "B") & (g.U == u0)]["mean"].iloc[0]
    a0 = g[(g.structure == "A") & (g.U == u0)]["mean"].iloc[0]
    ax.plot(us, b0 * np.log2(us) / np.log2(u0), "--", color=COLOR["B"], linewidth=2,
            label="B predicted: grows like log2 U")
    ax.plot(us, np.full_like(us, a0), "--", color=COLOR["A"], linewidth=2,
            label="A predicted: flat in U")
    ax.set_xscale("log")
    ax.set_ylim(bottom=0)
    style(ax, "Distinct values U", "Time per step (ns)", "Time per step against number of distinct values")
    finish(fig, ax, "fig6_time_vs_U.png",
           "Mean of the timed runs, error bars = 1 standard deviation; log x axis. Predicted curves "
           "are calibrated at the smallest U.")


def make_mem_vs_U():
    e = mem[mem.exp == "E3_U"]
    if e.empty:
        return
    g = e.groupby(["structure", "U"])["mem_kb"].mean().reset_index().rename(
        columns={"mem_kb": "mean"})
    g["std"] = 0.0
    fig, ax = plt.subplots(figsize=(7, 4.6), facecolor=SURFACE)
    draw(ax, g, "U")
    umax = g.U.max()
    us = np.array(sorted(g.U.unique()), dtype=float)
    b1 = g[(g.structure == "B") & (g.U == umax)]["mean"].iloc[0]
    a0 = g[(g.structure == "A") & (g.U == g.U.min())]["mean"].iloc[0]
    ax.plot(us, b1 * us / umax, "--", color=COLOR["B"], linewidth=2,
            label="B predicted: proportional to U")
    ax.plot(us, np.full_like(us, a0), "--", color=COLOR["A"], linewidth=2,
            label="A predicted: flat in U")
    ax.set_xscale("log")
    ax.set_yscale("log")
    style(ax, "Distinct values U", "Extra resident memory (KB)", "Memory against number of distinct values")
    finish(fig, ax, "fig7_memory_vs_U.png",
           "Resident memory added by the structure (one run per point); log axes. B's prediction is "
           "calibrated at the largest U, A's at the smallest.")


make_time_vs_w()
make_time_vs_r()
make_mem_vs_w()
make_mem_vs_r()
make_purges()
make_time_vs_U()
make_mem_vs_U()