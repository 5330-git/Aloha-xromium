该目录实现了 Aloha-Xromium 的安装程序（WIP），目录分析：

- core：安装程序核心，基于 base 库实现分析 Windows PE 文件导入表，从而收集 `aloha.exe` 所依赖的dll。此外提供一个命令行工具
- gui：GUI 实现，封装 core 的内容。
