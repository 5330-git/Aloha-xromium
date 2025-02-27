"""
@brief: 补丁工具
@details: aloha-xromium 是在 Chromium 浏览器基础上进行二次开发的项目，
  有时无法避免对 Chromium 原始代码（aloha目录以外的内容）做出修改，为了减少依赖，
  需要开发一套工具自动分析对 Chromium 原始代码的修改，并生成补丁文件。同时支持将
  最新的补丁文件应用到 Chromium 原始代码上。
@author: anton
@email: 1762282883@qq.com
"""

import os
from pathlib import Path
import argparse
import subprocess

kAlohaPath = Path(__file__).parent.parent.parent.resolve()
kChromiumSrcPath = Path(kAlohaPath).parent.resolve()
kPatchName = "aloha.patch"
kApplyPatchCommand = f"git apply {Path(kAlohaPath).joinpath(kPatchName)}"
kProducePatchCommand = "git diff HEAD --binary :!aloha"
kCleanPatchCommand = "git checkout HEAD :!aloha"

g_argparser = argparse.ArgumentParser(description="Patch tool for Aloha-Xromium")
g_argparser.add_argument("-a", "--apply", action="store_true", help="apply patch")
g_argparser.add_argument("-p", "--produce", action="store_true", help="produce patch")
g_argparser.add_argument("-c", "--clean", action="store_true", help="clean patch from Chromium source")


def ApplyPatch():
    patch_file = Path(kAlohaPath).joinpath(kPatchName)
    assert patch_file.exists(), "Patch file not found, please check."
    os.chdir(kChromiumSrcPath)
    try:
        result = subprocess.run(
            kApplyPatchCommand, shell=True, capture_output=True, text=True, check=True
        )
        print("Apply patch success.")
        print("Command output:", result.stdout)
    except subprocess.CalledProcessError as e:
        print("Apply patch failed.")
        print("Error output:", e.stderr)


def ProducePatch():
    os.chdir(kChromiumSrcPath)
    try:
        result = subprocess.run(
            kProducePatchCommand, shell=True, capture_output=True, text=True, check=True
        )
        patch_content = result.stdout

        # 将 Windows 换行符 (\r\n) 替换为 Unix 换行符 (\n)
        patch_content = patch_content.replace("\r\n", "\n")

        # 将补丁内容写入文件，强制使用 Unix 换行符
        patch_file = Path(kAlohaPath).joinpath(kPatchName)
        with patch_file.open("w", newline="\n") as f:
            f.write(patch_content)

        print("Produce patch success.")
    except subprocess.CalledProcessError as e:
        print("Produce patch failed.")
        print("Error output:", e.stderr)


def CleanPatch():
    os.chdir(kChromiumSrcPath)
    try:
        result = subprocess.run(
            kCleanPatchCommand, shell=True, capture_output=True, text=True, check=True
        )
        print("Clean patch success.")
        print("Command output:", result.stdout)
    except subprocess.CalledProcessError as e:
        print("Clean patch failed.")


def Main():
    args = g_argparser.parse_args()
    if args.produce:
        print("Produce patch")
        ProducePatch()
        pass
    elif args.apply:
        print("Apply patch")
        ApplyPatch()
        pass
    elif args.clean:
        print("Clean patch")
        CleanPatch()
        pass
    else:
        print("In default. apply patch to Chromium source.")
        ApplyPatch()


if __name__ == "__main__":
    Main()
