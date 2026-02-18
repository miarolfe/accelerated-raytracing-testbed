import csv
import logging
import os
import shutil
import sys
from dataclasses import dataclass
from typing import List

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

configurations = [
    {"width": "200", "height": "100", "samples_per_pixel": "25"},
    {"width": "400", "height": "200", "samples_per_pixel": "50"},
    {"width": "800", "height": "400", "samples_per_pixel": "75"},
    {"width": "1600", "height": "800", "samples_per_pixel": "100"},
    {"width": "3200", "height": "1600", "samples_per_pixel": "125"},
    {"width": "3840", "height": "2160", "samples_per_pixel": "500"},
]

scenes = [
    ["Three sphere clusters", 1],
    ["Uniform dense field", 2],
    ["Sparse clusters in void", 3],
    ["Extreme size variation", 4],
    ["Long corridor", 5],
    ["Centroid co-location", 6],
    ["Flat plane distribution", 7],
    ["Diagonal wall", 8],
    ["High object count", 9],
    ["Overlapping box city", 10],
]

NUM_SAMPLES = 10


@dataclass
class AccelerationStructureResults:
    construction_time_ms: float
    render_time_ms: float
    total_time_ms: float
    memory_used_bytes: int
    avg_nodes_per_ray: float
    avg_intersection_tests_per_ray: float


@dataclass
class RenderSampleResult:
    render_with_none_results: AccelerationStructureResults
    render_with_uniform_grid_results: AccelerationStructureResults
    render_with_hierarchical_grid_results: AccelerationStructureResults
    render_with_octree_results: AccelerationStructureResults
    render_with_bsp_tree_results: AccelerationStructureResults
    render_with_k_d_tree_results: AccelerationStructureResults
    render_with_bounding_volume_hierarchy_results: AccelerationStructureResults


@dataclass
class RenderTestOneStructureResult:
    lowest_construction_time_ms: float
    mean_construction_time_ms: float
    highest_construction_time_ms: float
    lowest_render_time_ms: float
    mean_render_time_ms: float
    highest_render_time_ms: float
    lowest_total_time_ms: float
    mean_total_time_ms: float
    highest_total_time_ms: float
    lowest_memory_used_bytes: int
    mean_memory_used_bytes: int
    highest_memory_used_bytes: int
    lowest_avg_nodes_per_ray: float
    mean_avg_nodes_per_ray: float
    highest_avg_nodes_per_ray: float
    lowest_avg_intersection_tests_per_ray: float
    mean_avg_intersection_tests_per_ray: float
    highest_avg_intersection_tests_per_ray: float


@dataclass
class RenderTestResults:
    render_with_none_results: RenderTestOneStructureResult
    uniform_grid_results: RenderTestOneStructureResult
    hierarchical_grid_results: RenderTestOneStructureResult
    octree_results: RenderTestOneStructureResult
    bsp_tree_results: RenderTestOneStructureResult
    k_d_tree_results: RenderTestOneStructureResult
    bounding_volume_hierarchy_results: RenderTestOneStructureResult


def parse_sample_log(filepath: str) -> RenderSampleResult:
    def parse_acceleration_structure_run_line(
        line: str,
    ) -> AccelerationStructureResults:
        _, part2 = line.split("Construction time: ", 1)
        construction_time_ms = float(part2.split("ms", 1)[0])
        _, part2 = line.split("Render time: ", 1)
        render_time_ms = float(part2.split("ms", 1)[0])
        _, part2 = line.split("Total time: ", 1)
        total_time_ms = float(part2.split("ms", 1)[0])
        _, part2 = line.split("Memory used: ", 1)
        memory_used_bytes = int(part2.split(" B", 1)[0])
        _, part2 = line.split("Avg nodes/ray:")
        avg_nodes_per_ray = float(part2.split(",")[0])
        _, part2 = line.split("Avg intersection tests/ray:")
        avg_intersection_tests_per_ray = float(part2.strip())

        return AccelerationStructureResults(
            construction_time_ms,
            render_time_ms,
            total_time_ms,
            memory_used_bytes,
            avg_nodes_per_ray,
            avg_intersection_tests_per_ray,
        )

    render_with_none_results = None
    render_with_uniform_grid_results = None
    render_with_hierarchical_grid_results = None
    render_with_octree_results = None
    render_with_bsp_tree_results = None
    render_with_k_d_tree_results = None
    render_with_bounding_volume_hierarchy_results = None

    with open(filepath) as f:
        for line in f.readlines():
            if "[Acceleration structure: None]" in line:
                render_with_none_results = parse_acceleration_structure_run_line(line)
            elif "[Acceleration structure: Uniform grid]" in line:
                render_with_uniform_grid_results = (
                    parse_acceleration_structure_run_line(line)
                )
            elif "[Acceleration structure: Hierarchical uniform grid]" in line:
                render_with_hierarchical_grid_results = (
                    parse_acceleration_structure_run_line(line)
                )
            elif "[Acceleration structure: Octree]" in line:
                render_with_octree_results = parse_acceleration_structure_run_line(line)
            elif "[Acceleration structure: BSP tree]" in line:
                render_with_bsp_tree_results = parse_acceleration_structure_run_line(
                    line
                )
            elif "[Acceleration structure: k-d tree]" in line:
                render_with_k_d_tree_results = parse_acceleration_structure_run_line(
                    line
                )
            elif "[Acceleration structure: Bounding volume hierarchy]" in line:
                render_with_bounding_volume_hierarchy_results = (
                    parse_acceleration_structure_run_line(line)
                )

    assert render_with_none_results
    assert render_with_uniform_grid_results
    assert render_with_hierarchical_grid_results
    assert render_with_octree_results
    assert render_with_bsp_tree_results
    assert render_with_k_d_tree_results
    assert render_with_bounding_volume_hierarchy_results

    return RenderSampleResult(
        render_with_none_results,
        render_with_uniform_grid_results,
        render_with_hierarchical_grid_results,
        render_with_octree_results,
        render_with_bsp_tree_results,
        render_with_k_d_tree_results,
        render_with_bounding_volume_hierarchy_results,
    )


def calculate_render_test_result(
    render_sample_results: List[RenderSampleResult],
) -> RenderTestResults:
    # Close enough for my purposes
    MIN_FLOAT = -1e30
    MAX_FLOAT = 1e30
    MIN_INT = -sys.maxsize
    MAX_INT = sys.maxsize

    def calculate_render_test_one_structure_result(
        results: List[AccelerationStructureResults],
    ) -> RenderTestOneStructureResult:
        lowest_construction_time_ms = MAX_FLOAT
        mean_construction_time_ms = 0.0
        highest_construction_time_ms = MIN_FLOAT
        lowest_render_time_ms = MAX_FLOAT
        mean_render_time_ms = 0.0
        highest_render_time_ms = MIN_FLOAT
        lowest_total_time_ms = MAX_FLOAT
        mean_total_time_ms = 0.0
        highest_total_time_ms = MIN_FLOAT
        lowest_memory_used_bytes = MAX_INT
        mean_memory_used_bytes = 0
        highest_memory_used_bytes = MIN_INT
        lowest_avg_nodes_per_ray = MAX_FLOAT
        mean_avg_nodes_per_ray = 0.0
        highest_avg_nodes_per_ray = MIN_FLOAT
        lowest_avg_intersection_tests_per_ray = MAX_FLOAT
        mean_avg_intersection_tests_per_ray = 0.0
        highest_avg_intersection_tests_per_ray = MIN_FLOAT

        num_results = len(results)
        for result in results:
            lowest_construction_time_ms = min(
                lowest_construction_time_ms, result.construction_time_ms
            )
            mean_construction_time_ms += result.construction_time_ms
            highest_construction_time_ms = max(
                highest_construction_time_ms, result.construction_time_ms
            )
            lowest_render_time_ms = min(lowest_render_time_ms, result.render_time_ms)
            mean_render_time_ms += result.render_time_ms
            highest_render_time_ms = max(highest_render_time_ms, result.render_time_ms)
            lowest_total_time_ms = min(lowest_total_time_ms, result.total_time_ms)
            mean_total_time_ms += result.total_time_ms
            highest_total_time_ms = max(highest_total_time_ms, result.total_time_ms)
            lowest_memory_used_bytes = min(
                lowest_memory_used_bytes, result.memory_used_bytes
            )
            mean_memory_used_bytes += result.memory_used_bytes
            highest_memory_used_bytes = max(
                highest_memory_used_bytes, result.memory_used_bytes
            )
            lowest_avg_nodes_per_ray = min(
                lowest_avg_nodes_per_ray, result.avg_nodes_per_ray
            )
            mean_avg_nodes_per_ray += result.avg_nodes_per_ray
            highest_avg_nodes_per_ray = max(
                highest_avg_nodes_per_ray, result.avg_nodes_per_ray
            )
            lowest_avg_intersection_tests_per_ray = min(
                lowest_avg_intersection_tests_per_ray,
                result.avg_intersection_tests_per_ray,
            )
            mean_avg_intersection_tests_per_ray += result.avg_intersection_tests_per_ray
            highest_avg_intersection_tests_per_ray = max(
                highest_avg_intersection_tests_per_ray,
                result.avg_intersection_tests_per_ray,
            )

        mean_construction_time_ms /= num_results
        mean_render_time_ms /= num_results
        mean_total_time_ms /= num_results
        mean_memory_used_bytes = int(mean_memory_used_bytes / num_results)
        mean_avg_nodes_per_ray /= num_results
        mean_avg_intersection_tests_per_ray /= num_results

        return RenderTestOneStructureResult(
            lowest_construction_time_ms,
            mean_construction_time_ms,
            highest_construction_time_ms,
            lowest_render_time_ms,
            mean_render_time_ms,
            highest_render_time_ms,
            lowest_total_time_ms,
            mean_total_time_ms,
            highest_total_time_ms,
            lowest_memory_used_bytes,
            mean_memory_used_bytes,
            highest_memory_used_bytes,
            lowest_avg_nodes_per_ray,
            mean_avg_nodes_per_ray,
            highest_avg_nodes_per_ray,
            lowest_avg_intersection_tests_per_ray,
            mean_avg_intersection_tests_per_ray,
            highest_avg_intersection_tests_per_ray,
        )

    num_samples = len(render_sample_results)
    none_results = []
    uniform_grid_results = []
    hierarchical_grid_results = []
    octree_results = []
    bsp_tree_results = []
    k_d_tree_results = []
    bounding_volume_hierarchy_results = []
    for sample_index in range(0, num_samples):
        sample = render_sample_results[sample_index]

        none_results.append(sample.render_with_none_results)
        uniform_grid_results.append(sample.render_with_uniform_grid_results)
        hierarchical_grid_results.append(sample.render_with_hierarchical_grid_results)
        octree_results.append(sample.render_with_octree_results)
        bsp_tree_results.append(sample.render_with_bsp_tree_results)
        k_d_tree_results.append(sample.render_with_k_d_tree_results)
        bounding_volume_hierarchy_results.append(
            sample.render_with_bounding_volume_hierarchy_results
        )

    return RenderTestResults(
        calculate_render_test_one_structure_result(none_results),
        calculate_render_test_one_structure_result(uniform_grid_results),
        calculate_render_test_one_structure_result(hierarchical_grid_results),
        calculate_render_test_one_structure_result(octree_results),
        calculate_render_test_one_structure_result(bsp_tree_results),
        calculate_render_test_one_structure_result(k_d_tree_results),
        calculate_render_test_one_structure_result(bounding_volume_hierarchy_results),
    )


def setup_benchmark_environment():
    build_return_code = os.system("cd .. && ./build.sh release headless")
    if build_return_code != 0:
        logging.error("Failed to build ART")
        sys.exit(1)
    else:
        logging.info("Built ART")

    if os.path.exists("results"):
        shutil.rmtree("results")

    for scene_index in range(len(scenes)):
        for configuration_index in range(len(configurations)):
            os.makedirs(
                f"results/scene_{scene_index + 1}/config_{configuration_index + 1}/renders"
            )


def run_render():
    for scene_index in range(len(scenes)):
        scene_name = scenes[scene_index][0]
        scene_number = scenes[scene_index][1]
        for configuration_index in range(len(configurations)):
            configuration = configurations[configuration_index]
            logging.info(
                f"Testing scene ({scene_number}) '{scene_name}' with configuration {configurations[configuration_index]}"
            )
            for sample in range(NUM_SAMPLES):
                logging.info(f"Running render sample {sample + 1}")

                # Run render for this sample
                os.system(
                    f"../bin/Release_Headless/ART --width {configuration['width']} --height {configuration['height']} --samples {configuration['samples_per_pixel']} --scene {scene_number}"
                )

                # Move log file and rename
                os.system(
                    f"mv log.txt results/scene_{scene_index + 1}/config_{configuration_index + 1}/log_sample_{sample + 1}.txt"
                )

                # Move all renders
                os.system(
                    command=f"mv *.png results/scene_{scene_index + 1}/config_{configuration_index + 1}/renders"
                )


def write_results_as_csv(filepath: str = "results/results.csv"):
    structures = [
        ("None", "render_with_none_results"),
        ("Uniform Grid", "uniform_grid_results"),
        ("Hierarchical Uniform Grid", "hierarchical_grid_results"),
        ("Octree", "octree_results"),
        ("BSP Tree", "bsp_tree_results"),
        ("k-d Tree", "k_d_tree_results"),
        ("BVH", "bounding_volume_hierarchy_results"),
    ]

    # (scene_name, config, struct_name, result)
    all_results = []

    for scene_index in range(len(scenes)):
        for configuration_index in range(len(configurations)):
            render_sample_results = []
            for sample in range(NUM_SAMPLES):
                render_sample_result = parse_sample_log(
                    f"results/scene_{scene_index + 1}/config_{configuration_index + 1}/log_sample_{sample + 1}.txt"
                )
                render_sample_results.append(render_sample_result)

            results = calculate_render_test_result(render_sample_results)
            scene_name = scenes[scene_index][0]
            config = configurations[configuration_index]
            for struct_name, attr in structures:
                all_results.append(
                    (scene_name, config, struct_name, getattr(results, attr))
                )

    csv_path = "results/results.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "scene",
                "width",
                "height",
                "samples_per_pixel",
                "acceleration_structure",
                "min_construction_time_ms",
                "mean_construction_time_ms",
                "max_construction_time_ms",
                "min_render_time_ms",
                "mean_render_time_ms",
                "max_render_time_ms",
                "min_total_time_ms",
                "mean_total_time_ms",
                "max_total_time_ms",
                "min_memory_used_bytes",
                "mean_memory_used_bytes",
                "max_memory_used_bytes",
                "min_avg_nodes_per_ray",
                "mean_avg_nodes_per_ray",
                "max_avg_nodes_per_ray",
                "min_avg_intersection_tests_per_ray",
                "mean_avg_intersection_tests_per_ray",
                "max_avg_intersection_tests_per_ray",
            ]
        )
        for scene_name, config, struct_name, r in all_results:
            writer.writerow(
                [
                    scene_name,
                    config["width"],
                    config["height"],
                    config["samples_per_pixel"],
                    struct_name,
                    r.lowest_construction_time_ms,
                    r.mean_construction_time_ms,
                    r.highest_construction_time_ms,
                    r.lowest_render_time_ms,
                    r.mean_render_time_ms,
                    r.highest_render_time_ms,
                    r.lowest_total_time_ms,
                    r.mean_total_time_ms,
                    r.highest_total_time_ms,
                    r.lowest_memory_used_bytes,
                    r.mean_memory_used_bytes,
                    r.highest_memory_used_bytes,
                    r.lowest_avg_nodes_per_ray,
                    r.mean_avg_nodes_per_ray,
                    r.highest_avg_nodes_per_ray,
                    r.lowest_avg_intersection_tests_per_ray,
                    r.mean_avg_intersection_tests_per_ray,
                    r.highest_avg_intersection_tests_per_ray,
                ]
            )
    logging.info(f"Results written to {csv_path}")


def main():
    setup_benchmark_environment()
    run_render()
    write_results_as_csv()
    # TODO: Graphs


if __name__ == "__main__":
    main()
