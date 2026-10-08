# Kiana 单敌人战斗原型设计

日期：2026-10-02  
状态：蓝图方向与 Root Motion 方案已获用户批准；本文供实现前复核。

## 目标

在 UE 5.8.1 蓝图项目中做出一个可玩的第三人称单场战斗：玩家操控 Kiana 面对一名 Strike Jaeger，能移动、自动朝向敌人、打出普通连段和分支、闪避、释放技能；敌人会接近、攻击、受击、硬直并死亡。首版用于验证动作、距离、受击反馈和战斗节奏，不扩展成完整游戏框架。

## 已确定的体验与输入

| 输入 | 首版行为 |
| --- | --- |
| WASD | 控制相机相对方向的 Root Motion 行走/跑动；角色朝输入方向转向 |
| 鼠标移动 | 控制第三人称跟随镜头 |
| 鼠标左键 | 普通攻击连段：Attack1、Attack2、Attack3、Attack4、Attack5 |
| 鼠标右键 | 在第三、第四段分支窗口选择对应的分支攻击 |
| 空格 | 按输入方向播放前闪或后闪，并在指定帧提供短暂无敌时间 |
| E | 播放 Kiana_Skill，在技能有效帧做范围伤害 |

镜头使用 Spring Arm 与 Follow Camera。战斗只有一个有效敌人时，Kiana 在攻击开始前自动转向敌人；攻击 Root Motion 已开始后不持续追踪目标旋转，避免角色一边移动一边被旋转打断。没有有效目标时，攻击朝角色当前朝向执行。

首版不实现跳跃，空格固定用于闪避。原始素材中有 Jump、JumpLocal、FreeFall 和 FreeFallToRun FBX，但这些动作尚未导入；待地面战斗流程稳定后再评估空中移动。

## 方案选择

1. **Root Motion 全位移，首版采用。** Walk、Run、闪避和战斗动作由动画根轨迹驱动；蓝图读取输入来选择移动状态并确定角色朝向，Character Movement 仍负责碰撞、地面检测和应用根运动。优点是动作位移与画面一致；风险是轨迹幅度需要逐动作校准，且现有基础移动素材主要是前向动作。
2. **混合方案。** 普通移动由 Character Movement 输入驱动，攻击、闪避和技能使用 Root Motion。操控较直接，适合只有单向行走动画的情况，但与当前选择的全 Root Motion 移动不一致；仅当第一轮实测无法得到可控的全向移动时，才回到此备选。
3. **全部由 Character Movement 驱动。** 动画只负责姿态，移动速度和攻击突进由蓝图参数控制。实现简单但会产生动作与位移不一致，因此不用于首版。

## 蓝图结构

- **BP_KianaCharacter**：玩家角色、摄像机、输入入口、目标引用、移动朝向和角色状态。通过移动输入强度选择 Idle、Walk、Run；使用相机相对输入方向决定角色朝向。
- **ABP_Kiana**：使用 Kiana Skeleton，负责 Idle/Walk/Run 状态机和 Montage Slot。Root Motion Mode 设为 Root Motion from Everything，使状态机动作与战斗 Montage 都可驱动角色位移。
- **BP_CombatComponent**：管理攻击状态、连段索引、分支选择、输入缓存、闪避无敌帧、技能冷却和攻击窗口。角色蓝图调用组件，不把所有战斗状态堆进单个 Event Graph。
- **BP_HealthComponent**：供玩家和敌人共用，保存生命值并广播受伤、死亡事件。
- **BP_StrikeJaeger**：敌人角色与轻量状态流程。使用计时器和状态枚举完成待机、接近、攻击、恢复、受击、硬直和死亡，不使用 C++、GAS 或完整 Behavior Tree。
- **L_CombatArena / BP_CombatGameMode**：灰盒测试场地与玩家/敌人生成规则。首版只有一个场地和一个敌人。
- **WBP_CombatHUD**：仅显示玩家与敌人的生命值；首版不制作完整菜单或复杂技能 UI。

## 动作与命中规则

普通连段按 Attack1 → Attack2 → Attack3 → Attack4 → Attack5 推进。第三段可以由右键改为 Attack3_fenzhi，第四段可以改为 Attack4_fenzhi。只在动画定义的输入窗口接收下一击；窗口外输入丢弃，避免连点造成动画队列失控。各攻击有效帧由 Anim Notify State 打开和关闭，单次攻击对同一目标最多结算一次。

当前项目没有独立武器网格。首轮用角色前方的 Capsule/Sphere Trace 做可调的命中区域；若后续在 Skeleton 上确认了武器 Socket，再将 Trace 起止点改为 Socket 驱动。Kiana_Skill 的根轨迹没有明显位移，首版将技能做成原地范围攻击。敌人首轮可以先接通 Attack01，再逐个加入 Attack02、Attack03，确保长距离轨迹校准后才开放。

敌人攻击在接近玩家后先给出动作预警，再播放攻击 Montage；命中由攻击有效帧结算。玩家闪避无敌窗口与闪避动画的时间范围绑定。敌人受击按轻/重攻击切换 Hit 动作，可进入 Stun_Start → Stun_Loop → Stun_End；生命值归零后播放 Death_Stay 并停止攻击。

## Root Motion 数据与风险

UE MCP 已连接当前项目 KianaCombatDemo（UE 5.8.1）。首次检查时全部 AnimSequence 的 Enable Root Motion 均为关闭，Root Lock 为 Ref Pose。作为校准探针，现已单独启用并保存 Kiana_Walk；其余动画开关仍保持关闭。校准关卡的单节点动画测试确认 Kiana_Walk 通过 CharacterMovement 沿 +X 轴驱动角色，速度约 144 cm/s，胶囊保持在地面上。原始轨迹首末差值约 157.16 cm / 1.067 秒；最终移动方向和速度还需用正式 AnimBP 再验。其他动作的位移差异显著：

| 动作 | 原始根轨迹位移 |
| --- | ---: |
| Kiana_Walk | 约 1.57 米/段 |
| Kiana_Run | 约 6.56 米/段 |
| Kiana_EvadeFront / Back | 约 +3.64 / -2.01 米 |
| Kiana_Attack1 / Attack5 | 约 +4.20 / +9.77 米 |
| Kiana_Attack3_fenzhi / Attack4_fenzhi | 约 +6.96 / +7.58 米 |
| Jaeger Attack01 / Attack02 / Attack03 | 约 +1.69 / +22.06 / +9.61 米 |

这些是 AnimationLibrary 从导入根轨迹首末帧读出的原始差值，方向轴受 FBX 与骨架朝向影响，不能直接当作最终游戏世界位移。进入战斗实现前须在角色预览和 PIE 中确认朝向、实际距离、碰撞及循环动作接缝，再调 Root Motion Translation Scale 或处理超长动作。不能只对全部动画打开开关后就认为移动合格。敌人 Attack02 的距离尤其需要先校准，避免越过整个场地。

普通骨骼缺失提示（带 mirrored 的骨骼）与 Root Motion 是不同问题；不作为逐个重导动画的前置工作。只有预览发现动作变形或缺失骨骼确实影响可见姿势时才另行处理。

## 完成标准

- 从 CombatArena 运行 PIE 后可控制角色、转动镜头、停止移动时角色不漂移。
- 移动、闪避与战斗动作按已校准的根轨迹推动角色，空闲状态不会持续移动。
- 左键可完成普通五段连段，第三与第四段分支可由右键选择；攻击输入只在窗口内接收。
- 每个攻击窗口只命中一次；敌人受伤、受击、硬直、死亡与生命条同步。
- 敌人能接近、预警、攻击，闪避无敌窗口能避开攻击。
- 长距离动作不会把角色无控制地送出测试场地；碰撞不会被 Root Motion 穿透。
- 技能原地播放并在有效帧对范围内敌人造成一次伤害。

## 当前项目边界

已导入 Kiana 与 Strike Jaeger 的角色网格、Skeleton、贴图和 29 个动作序列。现有 L_RootMotionCalibration 是仅供校准的测试关卡，内含原生 Character 测试演员；正式战斗蓝图、战斗场地和输入资产尚未创建。DefaultGame.ini 仍将模板 OpenWorld 指定为默认地图。本文定义的是战斗蓝图原型，不代表正式战斗内容已实现。
