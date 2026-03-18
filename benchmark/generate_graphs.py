import argparse
import logging
import os
import re
from typing import cast

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

OUTPUT_DIR = "graphs"
FIGURE_SIZE = (16, 9)
DPI = 150

STRUCTURE_ORDER = [
    "None",
    "Uniform Grid",
    "Hierarchical Uniform Grid",
    "Octree",
    "BSP Tree",
    "KD Tree",
    "BVH",
]


def load_data() -> pd.DataFrame:
    data_frame = pd.read_csv("results/results.csv", keep_default_na=False)

    # Build a config label column for grouping
    data_frame["config_slug"] = (
        data_frame["width"].astype(str) + "x" + data_frame["height"].astype(str)
    )
    data_frame["config"] = (
        data_frame["width"].astype(str)
        + "x"
        + data_frame["height"].astype(str)
        + ", "
        + data_frame["samples_per_pixel"].astype(str)
        + " samples"
    )
    data_frame["total_pixels"] = (
        data_frame["width"] * data_frame["height"] * data_frame["samples_per_pixel"]
    )

    return data_frame


def assign_colours(
    structures: list[str],
) -> dict[str, tuple[float, float, float, float]]:

    ordered = [s for s in STRUCTURE_ORDER if s in structures]
    unordered = [s for s in structures if s not in STRUCTURE_ORDER]
    n = len(ordered)

    colours = {
        name: plt.colormaps["gist_rainbow"](i / max(n - 1, 1))
        for i, name in enumerate(ordered)
    }

    # Structures not in STRUCTURE_ORDER fall back to tab10
    fallback_colourmap = plt.colormaps["tab10"]
    for j, name in enumerate(unordered):
        colours[name] = fallback_colourmap(j % 10)

    return colours


def grouped_bar_chart(
    df: pd.DataFrame,
    mean_col: str,
    title: str,
    ylabel: str,
    output_path: str,
    colour_map: dict[str, tuple[float, float, float, float]],
    structures: list[str],
    log_scale: bool = False,
    min_col: str | None = None,
    max_col: str | None = None,
) -> None:
    scenes = df["scene"].unique()
    num_scenes = len(scenes)
    num_structures = len(structures)

    x = np.arange(num_scenes)
    bar_width = 0.8 / num_structures

    fig, ax = plt.subplots(figsize=FIGURE_SIZE)

    for i, structure in enumerate(structures):
        subset = cast(pd.DataFrame, df[df["acceleration_structure"] == structure])
        # Align rows to scene order
        values = []
        err_low = []
        err_high = []
        for scene in scenes:
            row = cast(pd.DataFrame, subset[subset["scene"] == scene])
            if len(row) == 0:
                values.append(0)
                err_low.append(0)
                err_high.append(0)
            else:
                mean_val = row[mean_col].iloc[0]
                values.append(mean_val)
                if min_col and max_col:
                    err_low.append(mean_val - row[min_col].iloc[0])
                    err_high.append(row[max_col].iloc[0] - mean_val)
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


def generate_per_config_charts(
    df: pd.DataFrame,
    colour_map: dict[str, tuple[float, float, float, float]],
    structures: list[str],
    fmt: str = "pdf",
) -> None:
    configs = df["config"].unique()

    for config in configs:
        config_df = cast(pd.DataFrame, df[df["config"] == config])
        config_label = config
        config_slug = config_df["config_slug"].iloc[0]

        # 1. Render time by scene
        grouped_bar_chart(
            config_df,
            "mean_render_time_ms",
            f"Mean Render Time by Scene ({config_label})",
            "Render Time (ms)",
            os.path.join(OUTPUT_DIR, f"render_time_by_scene_{config_slug}.{fmt}"),
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
            os.path.join(OUTPUT_DIR, f"total_time_by_scene_{config_slug}.{fmt}"),
            colour_map,
            structures,
            log_scale=True,
            min_col="min_total_time_ms",
            max_col="max_total_time_ms",
        )

        # 3. Construction time by scene (exclude None — always 0)
        accel_structures = [s for s in structures if s != "None"]
        grouped_bar_chart(
            cast(
                pd.DataFrame, config_df[config_df["acceleration_structure"] != "None"]
            ),
            "mean_construction_time_ms",
            f"Mean Construction Time by Scene ({config_label})",
            "Construction Time (ms)",
            os.path.join(OUTPUT_DIR, f"construction_time_by_scene_{config_slug}.{fmt}"),
            colour_map,
            accel_structures,
            log_scale=True,
            min_col="min_construction_time_ms",
            max_col="max_construction_time_ms",
        )

        # 4. Speedup over None
        generate_speedup_chart(
            config_df, config_label, config_slug, colour_map, accel_structures, fmt=fmt
        )

        # 5. Memory usage by scene
        grouped_bar_chart(
            cast(
                pd.DataFrame, config_df[config_df["acceleration_structure"] != "None"]
            ),
            "mean_memory_used_bytes",
            f"Memory Usage by Scene ({config_label})",
            "Memory (bytes)",
            os.path.join(OUTPUT_DIR, f"memory_usage_by_scene_{config_slug}.{fmt}"),
            colour_map,
            accel_structures,
            log_scale=True,
        )

        # 6. Avg intersection tests per ray
        grouped_bar_chart(
            cast(
                pd.DataFrame, config_df[config_df["acceleration_structure"] != "None"]
            ),
            "mean_avg_intersection_tests_per_ray",
            f"Avg Intersection Tests per Ray by Scene ({config_label})",
            "Intersection Tests / Ray",
            os.path.join(
                OUTPUT_DIR, f"intersection_tests_by_scene_{config_slug}.{fmt}"
            ),
            colour_map,
            accel_structures,
            log_scale=True,
        )

        # 7. Avg nodes traversed per ray
        grouped_bar_chart(
            cast(
                pd.DataFrame, config_df[config_df["acceleration_structure"] != "None"]
            ),
            "mean_avg_nodes_per_ray",
            f"Avg Nodes Traversed per Ray by Scene ({config_label})",
            "Nodes / Ray",
            os.path.join(OUTPUT_DIR, f"nodes_traversed_by_scene_{config_slug}.{fmt}"),
            colour_map,
            accel_structures,
        )


def generate_speedup_chart(
    config_df: pd.DataFrame,
    config_label: str,
    config_slug: str,
    colour_map: dict[str, tuple[float, float, float, float]],
    accel_structures: list[str],
    fmt: str = "pdf",
) -> None:
    scenes = config_df["scene"].unique()
    none_times = {}
    for scene in scenes:
        none_row = cast(
            pd.DataFrame,
            config_df[
                (config_df["scene"] == scene)
                & (config_df["acceleration_structure"] == "None")
            ],
        )
        if len(none_row) > 0:
            none_times[scene] = none_row["mean_render_time_ms"].iloc[0]

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
            row = cast(
                pd.DataFrame,
                config_df[
                    (config_df["scene"] == scene)
                    & (config_df["acceleration_structure"] == structure)
                ],
            )
            if len(row) > 0 and scene in none_times and none_times[scene] > 0:
                speedups.append(none_times[scene] / row["mean_render_time_ms"].iloc[0])
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
    output_path = os.path.join(OUTPUT_DIR, f"speedup_over_none_{config_slug}.{fmt}")
    fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
    plt.close(fig)
    logging.info(f"Saved {output_path}")


def generate_scaling_charts(
    df: pd.DataFrame,
    colour_map: dict[str, tuple[float, float, float, float]],
    structures: list[str],
    fmt: str = "pdf",
) -> None:
    scenes = df["scene"].unique()

    for scene in scenes:
        scene_df = cast(pd.DataFrame, df[df["scene"] == scene])
        configs = scene_df["config"].unique()
        if len(configs) <= 1:
            continue

        fig, ax = plt.subplots(figsize=FIGURE_SIZE)

        for structure in structures:
            subset = cast(
                pd.DataFrame, scene_df[scene_df["acceleration_structure"] == structure]
            ).sort_values("total_pixels")
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
        output_path = os.path.join(OUTPUT_DIR, f"scaling_{scene_slug}.{fmt}")
        fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
        plt.close(fig)
        logging.info(f"Saved {output_path}")


def generate_per_scene_charts(
    df: pd.DataFrame,
    colour_map: dict[str, tuple[float, float, float, float]],
    structures: list[str],
    fmt: str = "pdf",
) -> None:
    acceleration_structures = [s for s in structures if s != "None"]
    scenes = df["scene"].unique()

    for scene in scenes:
        scene_df = cast(pd.DataFrame, df[df["scene"] == scene])
        configs = list(scene_df["config"].unique())
        x = np.arange(len(configs))

        fig, axes = plt.subplots(3, 2, figsize=(16, 14))
        fig.suptitle(scene, fontsize=16, y=1.01)

        panels: list[
            tuple[
                plt.Axes,
                str,  # title
                str,  # ylabel
                list[str],  # which structures to include
                str,  # mean column
                bool,  # log scale
                str | None,  # min column
                str | None,  # max column
                bool,  # draw y=1 reference line
            ]
        ] = [
            (
                axes[0, 0],
                "Speedup over no acceleration",
                "Speedup (x)",
                acceleration_structures,
                "",
                False,
                None,
                None,
                True,
            ),
            (
                axes[0, 1],
                "Render time by configuration (width, height, samples)",
                "Render time (ms)",
                structures,
                "mean_render_time_ms",
                True,
                "min_render_time_ms",
                "max_render_time_ms",
                False,
            ),
            (
                axes[1, 0],
                "Construction time by configuration (width, height, samples)",
                "Construction time (ms)",
                acceleration_structures,
                "mean_construction_time_ms",
                True,
                "min_construction_time_ms",
                "max_construction_time_ms",
                False,
            ),
            (
                axes[1, 1],
                "Memory usage by configuration (width, height, samples)",
                "Memory (bytes)",
                acceleration_structures,
                "mean_memory_used_bytes",
                True,
                None,
                None,
                False,
            ),
            (
                axes[2, 0],
                "Intersection tests / ray by configuration (width, height, samples)",
                "Intersection tests / ray",
                acceleration_structures,
                "mean_avg_intersection_tests_per_ray",
                True,
                None,
                None,
                False,
            ),
            (
                axes[2, 1],
                "Nodes traversed / ray by configuration (width, height, samples)",
                "Nodes / ray",
                acceleration_structures,
                "mean_avg_nodes_per_ray",
                False,
                None,
                None,
                False,
            ),
        ]

        # Pre-compute None render times per config for speedup panel
        none_times: dict[str, float] = {}
        for config in configs:
            none_row = cast(
                pd.DataFrame,
                scene_df[
                    (scene_df["config"] == config)
                    & (scene_df["acceleration_structure"] == "None")
                ],
            )
            if len(none_row) > 0:
                none_times[config] = none_row["mean_render_time_ms"].iloc[0]

        if none_times:
            logging.info(f"Speedups for scene: {scene}")
            for config in configs:
                if config not in none_times:
                    continue
                logging.info(f"  {config}:")
                for structure in acceleration_structures:
                    row = cast(
                        pd.DataFrame,
                        scene_df[
                            (scene_df["config"] == config)
                            & (scene_df["acceleration_structure"] == structure)
                        ],
                    )
                    if len(row) > 0 and none_times[config] > 0:
                        speedup = (
                            none_times[config] / row["mean_render_time_ms"].iloc[0]
                        )
                        logging.info(f"    {structure}: {speedup:.2f}x")

        for (
            ax,
            title,
            ylabel,
            panel_structures,
            mean_col,
            log_scale,
            min_col,
            max_col,
            hline,
        ) in panels:
            num_structures = len(panel_structures)
            bar_width = 0.8 / num_structures

            for i, structure in enumerate(panel_structures):
                values = []
                err_low = []
                err_high = []
                for config in configs:
                    row = cast(
                        pd.DataFrame,
                        scene_df[
                            (scene_df["config"] == config)
                            & (scene_df["acceleration_structure"] == structure)
                        ],
                    )
                    if len(row) == 0:
                        values.append(0)
                        err_low.append(0)
                        err_high.append(0)
                    elif mean_col == "":
                        # Speedup panel
                        if config in none_times and none_times[config] > 0:
                            values.append(
                                none_times[config] / row["mean_render_time_ms"].iloc[0]
                            )
                        else:
                            values.append(0)
                        err_low.append(0)
                        err_high.append(0)
                    else:
                        mean_val = row[mean_col].iloc[0]
                        values.append(mean_val)
                        if min_col and max_col:
                            err_low.append(mean_val - row[min_col].iloc[0])
                            err_high.append(row[max_col].iloc[0] - mean_val)
                        else:
                            err_low.append(0)
                            err_high.append(0)

                offset = (i - num_structures / 2 + 0.5) * bar_width
                error = [err_low, err_high] if (min_col and max_col) else None
                ax.bar(
                    x + offset,
                    values,
                    bar_width,
                    label=structure,
                    color=colour_map[structure],
                    yerr=error,
                    capsize=2,
                )

            if hline:
                ax.axhline(y=1.0, color="grey", linestyle="--", linewidth=0.8)
            if log_scale:
                ax.set_yscale("log")
            ax.set_title(title)
            ax.set_ylabel(ylabel)
            ax.set_xlabel("Configuration")
            ax.set_xticks(x)
            ax.set_xticklabels(configs, rotation=15, ha="right")
            ax.legend(fontsize=7, ncol=max(1, num_structures // 3))

        fig.tight_layout()
        scene_slug = re.sub(r"[^a-z0-9]+", "_", scene.lower()).strip("_")
        output_path = os.path.join(OUTPUT_DIR, f"scene_{scene_slug}.{fmt}")
        fig.savefig(output_path, dpi=DPI, bbox_inches="tight")
        plt.close(fig)
        logging.info(f"Saved {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--format",
        choices=["pdf", "png"],
        default="pdf",
        dest="fmt",
        help="Output format for graphs (default: pdf)",
    )
    args = parser.parse_args()

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    df = load_data()
    structures = list(df["acceleration_structure"].unique())
    colour_map = assign_colours(structures)

    generate_per_config_charts(df, colour_map, structures, fmt=args.fmt)
    generate_scaling_charts(df, colour_map, structures, fmt=args.fmt)
    generate_per_scene_charts(df, colour_map, structures, fmt=args.fmt)

    logging.info("Done")


if __name__ == "__main__":
    main()
