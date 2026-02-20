import logging
import os
import re

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

OUTPUT_DIR = "graphs"
FIGURE_SIZE = (16, 9)
DPI = 300


def load_data():
    df = pd.read_csv("results/results.csv", keep_default_na=False)
    # Build a config label column for grouping
    df["config"] = df["width"].astype(str) + "x" + df["height"].astype(str)
    df["total_pixels"] = df["width"] * df["height"] * df["samples_per_pixel"]
    return df


STRUCTURE_ORDER = [
    "None",
    "Uniform Grid",
    "Hierarchical Uniform Grid",
    "Octree",
    "BSP Tree",
    "KD Tree",
    "BVH",
]


def assign_colours(structures):
    # Assign rainbow colours in structure order (red → violet).
    # Structures not in STRUCTURE_ORDER fall back to tab10.
    ordered = [s for s in STRUCTURE_ORDER if s in structures]
    unordered = [s for s in structures if s not in STRUCTURE_ORDER]
    n = len(ordered)
    # hsv hue: 0.0 = red, ~0.75 = violet; spread evenly across that range
    rainbow_colours = {
        name: plt.colormaps["turbo"](i / max(n - 1, 1))
        for i, name in enumerate(ordered)
    }
    fallback = plt.colormaps["tab10"]
    for j, name in enumerate(unordered):
        rainbow_colours[name] = fallback(j % 10)
    return rainbow_colours


def grouped_bar_chart(
    df,
    mean_col,
    title,
    ylabel,
    output_path,
    colour_map,
    structures,
    log_scale=False,
    min_col=None,
    max_col=None,
):
    scenes = df["scene"].unique()
    num_scenes = len(scenes)
    num_structures = len(structures)

    x = np.arange(num_scenes)
    bar_width = 0.8 / num_structures

    fig, ax = plt.subplots(figsize=FIGURE_SIZE)

    for i, structure in enumerate(structures):
        subset = df[df["acceleration_structure"] == structure]
        # Align rows to scene order
        values = []
        err_low = []
        err_high = []
        for scene in scenes:
            row = subset[subset["scene"] == scene]
            if len(row) == 0:
                values.append(0)
                err_low.append(0)
                err_high.append(0)
            else:
                mean_val = row[mean_col].values[0]
                values.append(mean_val)
                if min_col and max_col:
                    err_low.append(mean_val - row[min_col].values[0])
                    err_high.append(row[max_col].values[0] - mean_val)
                else:
                    err_low.append(0)
                    err_high.append(0)

        offset = (i - num_structures / 2 + 0.5) * bar_width
        error = [err_low, err_high] if min_col and max_col else None
        ax.bar(
            x + offset,
            values,
            bar_width,
            label=structure,
            color=colour_map[structure],
            yerr=error,
            capsize=2,
        )

    ax.set_xlabel("Scene")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.set_xticks(x)
    ax.set_xticklabels(scenes, rotation=25, ha="right")
    if log_scale:
        ax.set_yscale("log")
    ax.legend(
        loc="upper center",
        bbox_to_anchor=(0.5, -0.18),
        ncol=min(num_structures, 7),
    )
    fig.tight_layout()
    fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)
    logging.info(f"Saved {output_path}")


def generate_per_config_charts(df, colour_map, structures):
    configs = df["config"].unique()

    for config in configs:
        config_df = df[df["config"] == config]
        config_label = config

        # 1. Render time by scene
        grouped_bar_chart(
            config_df,
            "mean_render_time_ms",
            f"Mean Render Time by Scene ({config_label})",
            "Render Time (ms)",
            os.path.join(OUTPUT_DIR, f"render_time_by_scene_{config_label}.png"),
            colour_map,
            structures,
            log_scale=True,
            min_col="min_render_time_ms",
            max_col="max_render_time_ms",
        )

        # 2. Total time by scene
        grouped_bar_chart(
            config_df,
            "mean_total_time_ms",
            f"Mean Total Time by Scene ({config_label})",
            "Total Time (ms)",
            os.path.join(OUTPUT_DIR, f"total_time_by_scene_{config_label}.png"),
            colour_map,
            structures,
            log_scale=True,
            min_col="min_total_time_ms",
            max_col="max_total_time_ms",
        )

        # 3. Construction time by scene (exclude None — always 0)
        accel_structures = [s for s in structures if s != "None"]
        grouped_bar_chart(
            config_df[config_df["acceleration_structure"] != "None"],
            "mean_construction_time_ms",
            f"Mean Construction Time by Scene ({config_label})",
            "Construction Time (ms)",
            os.path.join(OUTPUT_DIR, f"construction_time_by_scene_{config_label}.png"),
            colour_map,
            accel_structures,
            log_scale=True,
            min_col="min_construction_time_ms",
            max_col="max_construction_time_ms",
        )

        # 4. Speedup over None
        generate_speedup_chart(config_df, config_label, colour_map, accel_structures)

        # 5. Memory usage by scene
        grouped_bar_chart(
            config_df[config_df["acceleration_structure"] != "None"],
            "mean_memory_used_bytes",
            f"Memory Usage by Scene ({config_label})",
            "Memory (bytes)",
            os.path.join(OUTPUT_DIR, f"memory_usage_by_scene_{config_label}.png"),
            colour_map,
            accel_structures,
            log_scale=True,
        )

        # 6. Avg intersection tests per ray
        grouped_bar_chart(
            config_df[config_df["acceleration_structure"] != "None"],
            "mean_avg_intersection_tests_per_ray",
            f"Avg Intersection Tests per Ray by Scene ({config_label})",
            "Intersection Tests / Ray",
            os.path.join(OUTPUT_DIR, f"intersection_tests_by_scene_{config_label}.png"),
            colour_map,
            accel_structures,
            log_scale=True,
        )

        # 7. Avg nodes traversed per ray
        grouped_bar_chart(
            config_df[config_df["acceleration_structure"] != "None"],
            "mean_avg_nodes_per_ray",
            f"Avg Nodes Traversed per Ray by Scene ({config_label})",
            "Nodes / Ray",
            os.path.join(OUTPUT_DIR, f"nodes_traversed_by_scene_{config_label}.png"),
            colour_map,
            accel_structures,
        )


def generate_speedup_chart(config_df, config_label, colour_map, accel_structures):
    scenes = config_df["scene"].unique()
    none_times = {}
    for scene in scenes:
        none_row = config_df[
            (config_df["scene"] == scene)
            & (config_df["acceleration_structure"] == "None")
        ]
        if len(none_row) > 0:
            none_times[scene] = none_row["mean_render_time_ms"].values[0]

    if not none_times:
        return

    num_scenes = len(scenes)
    num_structures = len(accel_structures)
    x = np.arange(num_scenes)
    bar_width = 0.8 / num_structures

    fig, ax = plt.subplots(figsize=FIGURE_SIZE)

    for i, structure in enumerate(accel_structures):
        speedups = []
        for scene in scenes:
            row = config_df[
                (config_df["scene"] == scene)
                & (config_df["acceleration_structure"] == structure)
            ]
            if len(row) > 0 and scene in none_times and none_times[scene] > 0:
                speedups.append(
                    none_times[scene] / row["mean_render_time_ms"].values[0]
                )
            else:
                speedups.append(0)

        offset = (i - num_structures / 2 + 0.5) * bar_width
        ax.bar(
            x + offset,
            speedups,
            bar_width,
            label=structure,
            color=colour_map[structure],
        )

    ax.set_xlabel("Scene")
    ax.set_ylabel("Speedup (x)")
    ax.set_title(f"Render Speedup over No Acceleration ({config_label})")
    ax.set_xticks(x)
    ax.set_xticklabels(scenes, rotation=25, ha="right")
    ax.axhline(y=1.0, color="grey", linestyle="--", linewidth=0.8)
    ax.legend(
        loc="upper center",
        bbox_to_anchor=(0.5, -0.18),
        ncol=min(num_structures, 7),
    )
    fig.tight_layout()
    output_path = os.path.join(OUTPUT_DIR, f"speedup_over_none_{config_label}.png")
    fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)
    logging.info(f"Saved {output_path}")


def generate_scaling_charts(df, colour_map, structures):
    scenes = df["scene"].unique()

    for scene in scenes:
        scene_df = df[df["scene"] == scene]
        configs = scene_df["config"].unique()
        if len(configs) <= 1:
            continue

        fig, ax = plt.subplots(figsize=FIGURE_SIZE)

        for structure in structures:
            subset = scene_df[
                scene_df["acceleration_structure"] == structure
            ].sort_values("total_pixels")
            if len(subset) == 0:
                continue
            ax.plot(
                subset["total_pixels"],
                subset["mean_render_time_ms"],
                marker="o",
                label=structure,
                color=colour_map[structure],
            )

        ax.set_xlabel("Total Work (width * height * samples)")
        ax.set_ylabel("Mean Render Time (ms)")
        ax.set_title(f"Render Time Scaling: {scene}")
        ax.set_yscale("log")
        ax.legend(
            loc="upper center",
            bbox_to_anchor=(0.5, -0.12),
            ncol=min(len(structures), 7),
        )
        fig.tight_layout()
        scene_slug = re.sub(r"[^a-z0-9]+", "_", scene.lower()).strip("_")
        output_path = os.path.join(OUTPUT_DIR, f"scaling_{scene_slug}.png")
        fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
        plt.close(fig)
        logging.info(f"Saved {output_path}")


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    df = load_data()
    structures = list(df["acceleration_structure"].unique())
    colour_map = assign_colours(structures)

    generate_per_config_charts(df, colour_map, structures)
    generate_scaling_charts(df, colour_map, structures)

    logging.info("Done")


if __name__ == "__main__":
    main()
