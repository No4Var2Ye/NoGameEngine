
# CG 学习记录

---

## 坐标系转换

### 局部坐标

局部坐标（Local Coordinates），也称为模型坐标（Model Coordinates），是指在三维图形学中，物体自身的坐标系。每个物体都有自己的局部坐标系，用于定义其顶点的位置和形状。
局部坐标系的原点通常位于物体的几何中心或某个特定的参考点，坐标轴的方向和单位长度可以根据物体的设计进行定义。

### M 模型矩阵

模型矩阵（Model Matrix）是用于将局部坐标系中的顶点变换到世界坐标系中的矩阵。
它包含了物体的平移、旋转和缩放等变换信息。
通过模型矩阵，可以将物体从其局部坐标系转换到整个场景的统一世界坐标系中。

### 世界坐标

世界坐标（World Coordinates）是指在三维图形学中，整个场景的统一坐标系。它用于描述物体在整个三维空间中的位置和方向。
世界坐标系的原点通常位于场景的中心，坐标轴的方向和单位长度是全局统一的。

### 观察矩阵

视图矩阵（View Matrix）是用于将世界坐标系中的顶点变换到观察坐标系中的矩阵。它包含了摄像机的位置和方向等信息。通过视图矩阵，可以将场景从世界坐标系转换到摄像机的视角下。

### 观察坐标

观察坐标（View Coordinates），也称为摄像机坐标（Camera Coordinates）或视图坐标系（Eye Coordinates），是指在三维图形学中，基于摄像机位置和方向的坐标系。它用于描述物体相对于摄像机的位置和方向。
在观察坐标系中，摄像机通常位于原点，朝向负z轴方向，x轴指向右侧，y轴指向上方。

### P 投影矩阵

通过投影矩阵，可以将三维场景映射到二维平面上。

定义了一个视景体（Viewing Frustum），用于确定哪些物体在摄像机的视野范围内。

投影矩阵（Projection Matrix）是用于将观察坐标系中的顶点变换到裁剪空间中的矩阵。它定义了摄像机的视野范围和投影方式（透视投影或正交投影）。

### 裁剪坐标

裁剪坐标（Clip Coordinates）是指在三维图形学中，经过投影变换后的坐标系。它用于描述物体在裁剪空间中的位置和方向。
裁剪坐标系的范围通常是一个标准化的立方体，x轴和y轴的范围是[-w, w]，z轴的范围是[-w, w]，其中w是齐次坐标的第四个分量。

### 齐次裁剪坐标
齐次裁剪坐标（Homogeneous Clip Coordinates）是指在三维图形学中，使用齐次坐标表示的裁剪坐标系。齐次坐标引入了一个额外的分量w，用于表示点在三维空间中的位置。

### 透视除法

透视除法（Perspective Division）是将齐次裁剪坐标转换为归一化设备坐标（NDC）的过程。通过将x、y、z坐标分别除以w分量，可以得到NDC坐标。

降维：将四维齐次坐标转换为三维坐标。

### NDC坐标

NDC（Normalized Device Coordinates，归一化设备坐标）是指在裁剪空间（Clip Space）经过透视除法（Perspective Division）后得到的坐标系。NDC的范围是固定的，通常是[-1, 1]，表示在三维空间中一个标准化的立方体内的位置。

### 视口变换

视口变换（Viewport Transformation）是将NDC坐标转换为窗口坐标的过程。视口变换根据视口的大小和位置，将NDC坐标映射到屏幕上的实际像素位置。

真正变成2D坐标。

### 窗口坐标

窗口坐标（Window Coordinates）是指在渲染管线的最后阶段，经过视口变换（Viewport Transformation）后得到的坐标系。它们表示图形在屏幕上的实际位置，通常以像素为单位。
窗口坐标的范围取决于视口的大小，例如，如果视口的宽度为800像素，高度为600像素，那么窗口坐标的范围就是从(0,0)到(800,600)。

## 渲染使用流

绘制 3D 实体: 开启深度测试，设置不透明混合。

绘制透明物体: 调用 SetBlending(TRUE) $\rightarrow$ SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) $\rightarrow$ 关闭深度写入。

调试模式: 若按下特定键，调用 SetWireframeMode(TRUE) 查看网格。

状态保护: 在绘制临时对象（如 HUD）前使用 PushState()，绘制结束后 PopState()，确保状态不污染主管线。

[简单模型地址](https://github.com/KhronosGroup/glTF-Sample-Models/)
```sh
git clone --filter=blob:none --sparse https://github.com/KhronosGroup/glTF-Sample-Models.git
cd glTF-Sample-Models
git sparse-checkout set 2.0/Duck
```