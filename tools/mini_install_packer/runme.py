# Copyright (c) 2025 The Aloha-Xromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
@brief 用于打包 Aloha-Xromium 可执行文件的脚本
@author yeyun.anton
@email 1762282883@qq.com
"""

import argparse
from pathlib import Path
import constants
import json
import glob
import re
from string import Template
import shutil
import sys
import os

g_argparser = argparse.ArgumentParser(description="Aloha-Xromium Installation Packer")
g_argparser.add_argument(
    "--root_build_dir", type=str, help="The root directory of the output files"
)
g_argparser.add_argument("--deps_file", type=Path, help="The path to the deps file")
g_argparser.add_argument(
    "--pack_patterns_file", type=Path, help="The path to the pack patterns file(.json)"
)


class DependencyEntry:
    def __init__(self, src_file_path: Path, target_path: Path):
        self.src_file_path = src_file_path
        self.target_path = target_path


def GetLZMAExec(build_dir: Path) -> str:
    executable = "7za"
    if sys.platform == "win32":
        executable += ".exe"

    exe_path = (
        Path(build_dir)
        / ".."
        / ".."
        / "third_party"
        / "lzma_sdk"
        / "bin"
        / "host_platform"
        / executable
    )
    return str(exe_path)


def GetAlohaDependenciesList(
    root_build_dir: Path, intall_dir: Path, deps_file: Path, pack_patterns_file: Path
) -> list[DependencyEntry]:
    """
      deps_file 格式Demo:
    "runtime-dependencies-patterns": {
        ".*\\.dll$" : {
            "target-path": "${install_root}"
        }
    },
    "specific-targets": {
        "aloha.exe": {
            "target-path": "${install_root}"
        }
    }
    我们在 constants.py 中定义了对应 json 中的字段名称，方便后续的代码阅读和维护。
    """
    deps_content: str = None
    with open(deps_file, "r") as f:
        deps_content = f.read()
    pack_patterns_file_content: json = None
    with open(pack_patterns_file, "r") as f:
        pack_patterns_file_content = f.read()
        pack_patterns_file_content = json.loads(pack_patterns_file_content)

    deps_path_list: list[DependencyEntry] = []

    # 通过 pattern 匹配出 deps_file 中的路径
    for line in deps_content.splitlines():
        deps_patterns = pack_patterns_file_content[
            constants.kRuntimeDependenciesPatternName
        ].keys()
        deps_patterns = [re.compile(pattern) for pattern in deps_patterns]
        for pattern in deps_patterns:
            if pattern.match(line):
                deps_absolute_path = Path(root_build_dir).joinpath(line.strip())
                target_path_template = pack_patterns_file_content[
                    constants.kRuntimeDependenciesPatternName
                ][pattern.pattern][constants.kSpecificTargetEntryTargetPathName]
                # 替换 target_path_template 中的 ${install_root} 为 intall_dir
                target_path_template = Template(target_path_template)
                target_path = target_path_template.substitute(
                    {constants.kInstallRootPlaceholderName: str(intall_dir)}
                )

                deps_path_list.append(
                    DependencyEntry(deps_absolute_path, Path(target_path))
                )
                break
    # 基于 glob 库从 root_build_dir 中筛选出 deps_file 中的路径
    specific_targets_names = pack_patterns_file_content[
        constants.kSpecificTargetEntryName
    ].keys()
    for target_name in specific_targets_names:
        target_path = Path(root_build_dir) / target_name
        glob_files = glob.glob(str(target_path))
        target_path_template = pack_patterns_file_content[
            constants.kSpecificTargetEntryName
        ][target_name][constants.kSpecificTargetEntryTargetPathName]
        # 替换 target_path_template 中的 ${install_root} 为 intall_dir
        target_path_template = Template(target_path_template)
        target_path = target_path_template.substitute(
            {constants.kInstallRootPlaceholderName: str(intall_dir)}
        )
        glob_files = [
            DependencyEntry(Path(file), Path(target_path)) for file in glob_files
        ]
        deps_path_list.extend(glob_files)

    return deps_path_list


def CopyDependenciesFiles(deps_path_list: list[DependencyEntry]):
    for entry in deps_path_list:
        entry.target_path.mkdir(parents=True, exist_ok=True)
        if entry.src_file_path.is_dir():
            shutil.copytree(entry.src_file_path, entry.target_path, dirs_exist_ok=True)
            continue
        else:
            shutil.copy(entry.src_file_path, entry.target_path)


def CompressInstallation(root_build_dir: Path, install_dir: Path):
    lzma_exec = GetLZMAExec(root_build_dir)
    os.system(f"{lzma_exec} a {str(install_dir)}.7z {str(install_dir)}")


if __name__ == "__main__":
    args = g_argparser.parse_args()
    install_dir = Path(args.root_build_dir).joinpath(constants.kInstallDirName)
    deps_path_list = GetAlohaDependenciesList(
        args.root_build_dir, install_dir, args.deps_file, args.pack_patterns_file
    )
    CopyDependenciesFiles(deps_path_list)
    CompressInstallation(args.root_build_dir, install_dir)
