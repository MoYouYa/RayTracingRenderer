# RayTracingRenderer
a simple renderer that supports ray tracing

- PS：英文内容为翻译软件翻译结果
- PS：The English content is the translation result of the translation software

## Currently implemented light tracing methods:
* Path Tracing
* Bidirectional Path Tracing
* Photon Mapping
## Review the process of ray tracing:
1. Starting from the camera, emit detection rays into the scene
2. Then intersect with the scene, detect whether it intersects with an object (using BVH acceleration), and obtain information about the intersection point (position, normal, etc.)
3. Determine whether the object touched by the intersection is a light source. If so, return the emission of the light source. If no light source is touched, proceed with the following operations
4. Sample the light source. If there are multiple light sources, it is necessary to first select one light source
5. Russian roulette, then sample a shooting direction and repeat operations 2, 3, 4, and 5 for the shooting direction
## Implemented parts:
- Realized ray tracing from the camera.
- In terms of materials: implemented three types of materials: diffuse, specular, and microfact
- Utilized multithreading to improve rendering speed (dividing the entire image into N groups, each occupying x rows of the image, eliminating the need to consider asynchronous issues)
- Add BDPT (Bidirectional Path Tracing) bidirectional path tracking algorithm. If needed, please modify the RenderType called in the Render.render (...) function: XXX -- * 2023/10/8 *.
- Added the PM (Photon Mapping) photon mapping algorithm. This update modifies the structure of the Render class to make it compatible with the newly added PM algorithm - * 2023/10/19 *.
## Some implementation issues with Path Tracing:
1. Accuracy issue, as the data is mostly floating-point, when comparing data, accuracy can also affect the final result. It depends on personal preference to debug it yourself
2. Random number generation problem. During random sampling, the generation of random numbers is very important. I initially wrote a random number myself, but found that the effect was not good. In the end, I still used the random number generation in Game 101 assignment 7
3. Randomly sample an exit direction from the material. Similarly, due to a problem with the random method I initially designed, the resulting exit direction was concentrated in a single area, resulting in some areas being too bright. But just make the final modifications.
4. When there is a specular reflection object in the scene, the light source area on the specular reflection object is black. Later, it was found that there is still an accuracy issue. When determining whether the incident and reflected light are strictly specular reflected, the accuracy of the judgment determines the probability of the light source being sampled.
## Some implementation issues with PDPT (Bidirectional Path Tracing):
1. When calculating the light source path and camera path, it was troublesome to adjust the PDF and emission of the light source path. It took half a day to adjust the kd of the camera path.
2. When there is only one camera path point (only the camera exit point), the result of the screen will be very dark. If you don't want to adjust the parameters, limit the number of camera path points to at least two.
3. When connecting the light source path and camera path, it is important to pay attention to special situations (such as when the camera path directly hits the light source).
4. When adjusting the scene, it was found that there was a problem with the microfact material ball. The result was a problem with the accuracy of the judgment. This ball had self occlusion, and the reflected light from the ball collided with the original position, resulting in the direction being a 0 vector. In the subsequent calculation, the distance was 0, and once divided, an error occurred.
5. MIS (Multiple Importance Sampling) method, after reading for a while, I misunderstood it and figured out why it was not right.
6. Recommended Reading https://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Bidirectional_Path_Tracing If English is good, it should be able to understand faster.
7. Solved the previous issue of white noise appearing at the corner of the wall when the number of SPPs was larger. I looked at it and found that when calculating the weight of MIS, the weight changed to "nan". I guess it should be because the reflected light sampled hit the spot or the length of the light was too small in the corner, causing overflow when dividing by the square of the distance. I feel quite annoyed, the float error operation is not interrupted, and I have to find out why I was wrong* October 17, 2023*
## Some implementation issues PM (Photo Mapping):
1. It doesn't feel very difficult to implement, it's just the acceleration structure. I see that the commonly used kd-tree is divided spatially. However, since both PT and BDPT were previously written using BVH, when I implemented this acceleration structure, I divided the photons by the number of photons. Each time, I recursively sorted the photons by a certain axis and divided them into two parts.
2. When writing this, I had a brain twitch. When dividing it, I made a processing error, causing it to be continuously divided according to the z-axis. In the end, I was tortured during the query.
3. When querying later, to find the nearest photon:
    - This node is a leaf node, which is compared with the priority queue of the results to determine whether to store it.
    - This node is not a leaf node. If the priority queue for the current result is not stored to the specified size, further traverse the two child nodes of this node.
    - This node is not a leaf node. If the priority queue of the current result is stored to the specified size, determine the shortest distance between the bounding box of this node and the target point. If it is longer than the longest distance in the priority queue of the result, then the nodes in this node are too far away from the target point and will not be further traversed.
    - This node is not a leaf node. If the priority queue of the current result is stored to the specified size, determine the shortest distance between the bounding box of this node and the target point. If it is a bit shorter than the longest distance in the priority queue of the result, then the distance between the node in this node and the target point may be shorter than that at the top of the heap. Further traverse the two child nodes of this node.

## 目前实现的光追方式：
* Path Tracing
* Bidirectional Path Tracing
* Photon Mapping
## 回顾一下光线追踪的过程：
1. 从摄像机开始向场景发出探测射线
2. 然后与场景求交，检测是否与某个物体相交（使用了BVH加速），获取交点的信息（位置，法线等）
3. 判断交点所触碰的物体是否为光源，如果是则返回光源的emission，如果没有触碰到光源则进行后面的操作
4. 对光源进行采样，如果存在多个光源需要先挑选一个光源
5. 俄罗斯轮盘赌，然后采样一个出射方向，对出射方向重复2，3，4，5操作
## 已实现的部分：
- 实现了从摄像机出发的光线追踪。
- 材质方面：实现了diffuse，specular，microfact三种材质
- 利用了多线程提高渲染速度（将整个图像分成N组，每一组占据图像的x行，这样就不必考虑异步问题）
- 新增BDPT(Bidirectional Path Tracing) 双向路径追踪算法，需要使用的话请在Render.render(...)函数里修改调用的RenderType::XXX -- *2023/10/8*。
- 新增PM(Photon Mapping)光子映射算法，此次更新修改了Render类的结构，使其能够兼容新增的 PM算法 -- *2023/10/19*。
## 一些实现时的问题Path Tracing：
1. 精度问题，因为数据基本都是浮点型，所以在进行数据比较的时候，精度也会影响最终结果，这个看个人喜好自己调试
2. 随机数生成问题，在随机采样的时候，随机数的生成很重要，我最开始自己写了个随机数，但是发现效果不好，最后还是用了games101作业7里面的随机数生成
3. 材质里面随机采样一个出射方向，同理，由于我最开始设计的随机方法有问题，结果产生的出射方向集中在一片区域，导致某些区域过亮。不过最后修改了就好了。
4. 当场景中有镜面反射物体时，镜面反射物体上的光源区域是黑的，后面发现还是精度问题，在判断对入射光线和反射光线是否严格镜面反射的时候，判定精度决定了光源被采样到的概率。
## 一些实现时的问题PDPT(Bidirectional Path Tracing)：
1. 在计算光源路径和摄像机路径的时候，调pdf和光源路径的emission，摄像机路径的kd时很烦，调了半天。
2. 在单独只有一个摄像机路径点的时候（只有摄像机出射点），画面的结果会很暗，不想调参数了，就限制至少有两个摄像机路径点。
3. 在将光源路径和摄像机 路径 连接的时候，一定要注意特殊情况（摄像机路径直接打到光源之类的情况）。
4. 在调场景的时候，发现microfact材质的球出现了问题，结果是判断精度的 问题，这个球发生了自遮挡，从球上反射的光线撞在了原地，导致该方向为0向量，后面运算的时候，距离为0，一除就出错了。
5. MIS（Multiple Importance Sampling）方法，看了半天，理解错了，算出来怎么怎么不对劲。
6. 推荐阅读https://www.pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Bidirectional_Path_Tracing 如果英文比较好的话应该能更快理解。
7. 解决了之前遗留的 当spp数量更大时，在墙角位置会出现白色噪点的问题，我看了下，发现是在算MIS的权重时，权重weight变成了“nan”，我猜应该是在墙角的时候，采样到的反射光线撞到了原地或者光线长度极小造成在算除以距离平方的时候溢出了。我感觉还挺烦的，float 错误运算不中断，还得自己找为什么错了 。 -- *2023/10/17*
## 一些实现时的问题PM(Photon Mapping)：
1. 实现起来感觉不是很难，就是那个加速结构，我看大家普遍用的kd-tree，从空间上进行划分，但是因为之前写PT和BDPT都是用的BVH，所以我实现这个加速结构的时候是按光子数量进行的划分，每次递归将光子按某一个轴排序，平分成两份进行划分。
2. 写这个的时候脑抽了，里面的划分的时候，处理出错，导致一直按z轴划分，最后查询的时候折磨坏了。
3. 在后面查询的时候，要查找最近的光子：
    - 该节点为叶节点，与结果的优先队列进行比较，安排是否存入。
    - 该节点不为叶节点 ，如果当前结果的优先队列没有存储到指定大小，进一步遍历此节点的两个子节点。
    - 该节点不为叶节点 ，如果当前结果的优先队列存储到指定大小，判断此节点的包围盒和目标点的最短距离，如果比结果优先队列里的最长距离还长，则此节点里面的节点，距离目标点的距离都太远了，不进一步遍历。
    - 该节点不为叶节点 ，如果当前结果的优先队列存储到指定大小，判断此节点的包围盒和目标点的最短距离，如果比结果优先队列里的最长距离短一点，则此节点里面的节点，距离目标点的距离有可能比堆顶的更短，进一步遍历此节点的两个子节点。
