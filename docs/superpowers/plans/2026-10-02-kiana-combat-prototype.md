# Kiana 战斗原型实施计划

> 基于《../specs/2026-10-02-kiana-combat-prototype-design.md》。当前尚未创建正式战斗蓝图、战斗场地或输入资产；已创建独立 Root Motion 校准关卡，并仅将 Kiana_Walk 作为校准探针启用。

## 当前进度

- 已仅对 Kiana_Walk 启用并保存 Root Motion，并在 UE 中打开该序列。
- 已创建 L_RootMotionCalibration，包含 200 米测试地面、定向光和 KianaRootMotionTester 原生 Character；该地图不会替换默认地图。
- 单节点动画测试中，AnimSingleNodeInstance 使用 Root Motion from Everything，CharacterMovement 为 Walking；Kiana_Walk 沿 +X 轴驱动胶囊，实测速度约 144 cm/s，竖直位置稳定。
- 原始根轨迹首末差值约 157.16 cm，序列长 1.067 秒。这个测试验证了资产 Root Motion 与 CharacterMovement 能协同工作；正式 AnimBP 中的状态机移动仍待复测。
- Kiana_Run、闪避、攻击及敌人动作还未校准；除 Kiana_Walk 外，其余动画仍未启用 Root Motion。

## 阶段 0：校准 Root Motion

1. 在 Animation Sequence Editor 预览 Kiana 的 Walk、Run、EvadeFront、EvadeBack、Attack1、Attack5 和技能，确认根轨迹方向、动作姿势及前后位移。
2. 在临时角色/测试地面上启用单个动作的 Root Motion，确认角色朝向与根运动实际位移。避免同时批量启用所有动作，以便识别异常序列。
3. 为 Walk/Run 和攻击动作确定可接受的 Translation Scale；重点处理 Kiana 攻击长位移及 Jaeger Attack02。若 UE 的统一缩放无法分别满足行走与攻击，优先保证攻击距离可控，并在计划评审后决定按 Montage 或动画序列分开处理。
4. 确认 IK/Root Lock 选择、碰撞与循环接缝。输出每个已验证动作的目标位移和启用设置，作为后续蓝图配置依据。

**验收：** Kiana 能沿预期方向移动；攻击与闪避不越界；Run 循环无明显跳变；Idle 和原地 Skill 不漂移。

## 阶段 1：灰盒关卡与角色骨架

1. 创建 L_CombatArena，配置平地、边界和基本灯光。
2. 创建 BP_CombatGameMode 与 BP_KianaCharacter，指定 Kiana 网格、胶囊碰撞、Player Start 和默认地图。
3. 添加 Spring Arm 与 Follow Camera，调第三人称视距和镜头旋转范围。
4. 创建 BP_StrikeJaeger，先手动放置在场地对面，确认两套网格、Skeleton 和缩放正常。

**验收：** PIE 进入自建关卡，玩家与敌人显示正常，摄像机无穿地，玩家碰撞正常。

## 阶段 2：输入与 Root Motion 移动

1. 在 CombatDemo/Input 创建 Enhanced Input 的移动、镜头、普通攻击、分支攻击、闪避和技能 Input Action，并创建 IMC_Combat。
2. 在 BP_KianaCharacter 中读取相机相对输入，按输入方向转向，按输入强度选择 Walk/Run。
3. 创建 ABP_Kiana：绑定 Kiana Skeleton，建立 Idle/Walk/Run 状态机，设置 Root Motion Mode 为 Root Motion from Everything，并将确认过的动作启用 Root Motion。
4. 处理输入释放、Idle 转换和 Root Motion 方向，确保不输入时不残留移动。

**验收：** WASD、镜头与速度切换稳定；角色朝控制方向移动；松开输入后停住；没有额外漂移或穿过边界。

## 阶段 3：敌人行为与生命值

1. 创建 BP_HealthComponent，支持最大/当前生命值、受伤、死亡事件。
2. 在 BP_StrikeJaeger 创建简单状态枚举：Idle、Approach、Telegraph、Attack、Recover、Stunned、Dead。
3. 首先接通 Idle 与单一可控距离的 Attack01；测试通过后接入 Run Start/Loop/End 与其余攻击动作。
4. 使用目标距离和计时器控制接近、预警、攻击间隔和恢复；目标死亡后停止攻击。
5. 把 Root Motion 位移校准结果应用到敌人攻击，尤其限制 Attack02 的长距离。

**验收：** 敌人可接近玩家、播出预警和攻击，受伤/硬直/死亡状态不会重复触发攻击。

## 阶段 4：玩家连段、分支与命中

1. 创建 BP_CombatComponent，定义 Idle、Attacking、Evading、Skill、Hit、Dead 等状态。
2. 将 Attack1-5 与两个分支动作制作成 Montage，按阶段约定设置 Slot、播放速率和 Root Motion。
3. 为每段攻击增加 Combo Window 与 Attack Window Notify State；实现一次输入缓存、窗口关闭时清空及第三/第四段分支选择。
4. 在攻击有效帧做前方 Capsule/Sphere Trace，记录本次攻击已命中的目标，避免同一段重复扣血。
5. 根据伤害强度播放敌人的轻/重受击动作；生命值归零时切到 Death_Stay。
6. 在攻击开始前对准唯一有效敌人；Root Motion 动作开始后保持朝向稳定。

**验收：** 普通五段连段可达；右键分支只改变第三/第四段；窗口外输入不破坏状态；每段对目标只造成一次伤害。

## 阶段 5：闪避、技能与 HUD

1. 将空格映射为前闪/后闪，按移动方向选择动作；使用 Notify State 开关闪避无敌帧。
2. 将 E 映射为 Kiana_Skill Montage，在技能有效帧做一次范围检测，并添加简单冷却。
3. 创建 WBP_CombatHUD，显示双方生命值；绑定 HealthComponent 事件更新。
4. 完成胜负状态：敌人死亡时结束战斗；玩家生命值归零时显示失败并停止控制。

**验收：** 闪避移动距离可控、有效帧能避开敌人攻击；技能范围与冷却有效；生命条与战斗结果一致。

## 阶段 6：PIE 回归与调优

1. 在 CombatArena 连续完成普通攻击、分支、闪避、技能、受伤、硬直和击杀流程。
2. 检查不同距离、场地边界、敌人死亡后输入、攻击中重复输入和快速方向切换。
3. 修正根运动比例、Notify 时间、碰撞范围、相机距离、移动速度与攻击间隔。
4. 在文档中记录已验证的动作设置与遗留问题；只有当前原型稳定后再决定是否添加受击 UI、特效、音效或更多敌人。

**验收：** PIE 可以从开场完整打完一场战斗，无软锁状态、无重复伤害、无明显 Root Motion 穿墙/越界，输出与设计稿一致。

## 计划约束

- 全程先用蓝图，不加 C++、GAS、完整 Behavior Tree 或联机架构。
- 不重新导入全部 FBX；先用现有 29 个动作序列，只有可见问题或骨骼缺失影响姿势时才单独返工。
- 首版空格只绑定闪避；Jump、JumpLocal、FreeFall 和 FreeFallToRun 暂留在 SourceAssets，不导入、不实现跳跃，待地面战斗验证后再评估。
- 每阶段先在 PIE 验收后再进入下一阶段，Root Motion 轨迹没校准前不批量设置动画资产。
- 当前项目不是 Git 仓库，因此本计划和设计稿无法提交为 Git commit；未初始化仓库。
