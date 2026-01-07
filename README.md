
# 我的游戏

## 设计要求

采用面向对象编程方式实现（20%）

- 模型管理类
- 摄像机类（摄像机可采用第一人称或者第三人称）
- 交互管理类

构建初步三维场景(30%)

-   天空盒和地形
-   静态模型和动态模型

互动（20%）
- 实现场景漫游，人机交互等功能
- 可添加其他交互
- 用Directlnput实现 5%

加分功能（10%）

- 光照效果
    - 光源类型
    - 光照模型
    - 渲染效果等

作品报告规范（20%）


## 开发备注

### 项目配置

C/C++

预处理器

预处理器定义 WIN32;_DEBUG;_WINDOWS;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)

链接器

输入 glu32.lib;glaux.lib;opengl32.lib;

命令行 /SAFESEH:NO 
