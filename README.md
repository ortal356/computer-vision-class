# computer-vision-class

机器视觉课程中的 OpenCV（C++）小项目。每个子文件夹都是独立、可直接在 VS Code 中按 `F5` 运行的工程。

| 项目 | 说明 | 示例结果 |
| --- | --- | --- |
| [CoinCounter](CoinCounter) | 二值化 + 形态学 + 轮廓检测，统计硬币数量 | 10 枚硬币 |
| [PaperclipCounter](PaperclipCounter) | Otsu 反向二值化 + 闭运算 + 轮廓检测，统计回形针数量 | 13 个回形针 |

## 运行方式

用 VS Code 单独打开某个子文件夹（例如 `CoinCounter`），按 `F5` 自动编译并运行。
需要本机已安装 VS2022 的 MSVC 编译器和 OpenCV 4.x，具体见各子文件夹内的 `README.md`。
