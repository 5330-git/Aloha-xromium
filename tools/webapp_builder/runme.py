"""
@brief WebApp 前端资源构建工具
@details
    Aloha-Xromium 是基于 Chromium 内核（C++）和 Web（node.js技术栈）构建的浏览器，
    为了更好地管理前端资源，需要将前端资源进行打包到指定目录下。
@author yeyun.anton
@email 1762282883@qq.com

TODO(yeyun.anton):
    1. 检查 pnpm/node.js 是否存在，如果不存在则自动安装到构建目录中。
    2. 检查 pnpm 版本是否符合要求，如果不符合则自动升级到最新版本。
"""

from pathlib import Path
import argparse
import shutil
import subprocess

arg_parser = argparse.ArgumentParser()
# 默认下面的参数将由 gn 构建系统传入
arg_parser.add_argument("--target_gen_dir", type=str, default="")
arg_parser.add_argument("--root_gen_dir", type=str, default="")
arg_parser.add_argument("--root_build_dir", type=str, default="")
args = arg_parser.parse_args()

# Constants
kAlohaDir = Path(__file__).parent.parent.parent
kAlohaResourcesDir = kAlohaDir / "resources"
kAlohaWebAppSourceDir = kAlohaResourcesDir / "browser"
kChromiumDir = kAlohaDir.parent

kAlohaWebAppResouceGenDir = "aloha-webapp"
kTargetGenDir = kChromiumDir / args.target_gen_dir.removeprefix("//")
kRootGenDir = kChromiumDir / args.root_gen_dir.removeprefix("//")
kRootBuildDir = kChromiumDir / args.root_build_dir.removeprefix("//")
kPnpmPath = shutil.which("pnpm")

kInstallPackagesCommand = f"{kPnpmPath} install"
kBuildCommand = f"{kPnpmPath} run build"


kAlohaWebAppSourceTargets = [
    "aloha-app-main",
    "ai-agent-dialog",
]

# Checks
assert (
    kAlohaWebAppSourceDir.exists()
), f"webapp source directory not found : {kAlohaWebAppSourceDir}"
assert shutil.which("node") is not None, "node.js not found"
assert shutil.which("pnpm") is not None, "pnpm not found"
assert kTargetGenDir.exists(), f"target gen directory not found : {kTargetGenDir}"
assert kRootGenDir.exists(), f"root gen directory not found : {kRootGenDir}"
assert kRootBuildDir.exists(), f"root build directory not found : {kRootBuildDir}"


def DownloadNodeDotJs():
    """
    未来可以考虑：depot_tools/fetch_configs/node.py 的实现
    """
    pass



def Build():
    for target in kAlohaWebAppSourceTargets:

        target_dir = Path(kAlohaWebAppSourceDir) / target
        # 切入目录，执行构建命令，并检查是否构建成功o
        subprocess.run(kInstallPackagesCommand, cwd=target_dir, check=True)
        subprocess.run(kBuildCommand, cwd=target_dir, check=True)
        CopyDist(target_dir, target)


def CopyDist(target_dir: Path, target_name: str):
    dist_dir = target_dir / "dist"
    assert dist_dir.exists(), "dist directory not found"
    # 拷贝到 kRootBuildDir 目录下
    shutil.copytree(
        dist_dir, Path(kRootBuildDir) / kAlohaWebAppResouceGenDir / target_name, dirs_exist_ok=True
    )

    pass


def Main():
    Build()
    print("Build success")
    pass


if __name__ == "__main__":
    Main()
