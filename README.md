# ![Aloha logo](resources/app_icons/aloha.ico)**Aloha - Xromium**

## 简介

基于 Chromium 二次定制开发的浏览器

## 编译/运行方法

1. 在 chromium 源代码目录下（`src` 目录）拉取 `aloha` 仓库：
   1. ```bash
      # github:
      git clone https://github.com/5330-git/Aloha-xromium.git aloha
      # 国内可以拉取 gitee 的地址：
      git clone https://gitee.com/ordinaryAnton/aloha-xromium.git aloha
      ```
2. 运行补丁脚本将必要更改打入 chromium 代码中
   1. ```bash
      python aloha/tools/patcher/runme.py -a
      ```
   2. 此外还可以选择手动补丁
      1. ```bash
         # 切换到 chromium 根目录下
         git apply aloha/aloha.patch
         ```
3. 编译前端资源
   1. 前端资源主要是 `aloha\resources\browser` 目录下的两个子目录，都是使用 vue + vite 开发的，可以切换到这两个目录下分别执行构建命令：
      ```bash
      pnpm install
      pnpm run build
      ```
4. 编译浏览器客户端
   1. 确保切换到 chromium 代码的 src 根目录下，执行下面的命令

      ```bash
      gn gen out/Default
      autoninja -C out/Default aloha
      ```

## 博客链接

[【一】从0定制浏览器：引言-CSDN博客](https://blog.csdn.net/yyy11280335/article/details/145891351?spm=1001.2014.3001.5502)

[【二】从0定制浏览器：修复HTML5播放器问题并引入 Native Views-CSDN博客](https://blog.csdn.net/yyy11280335/article/details/145899706?spm=1001.2014.3001.5502)

[【间章一】从0定制浏览器：实现脚本工具管理耦合Chromium的代码更改-CSDN博客](https://blog.csdn.net/yyy11280335/article/details/145921616?spm=1001.2014.3001.5501)

[【三】从0定制浏览器：Tab UI 交互优化以及添加程序图标-CSDN博客](https://blog.csdn.net/yyy11280335/article/details/146159997?spm=1001.2014.3001.5501)
