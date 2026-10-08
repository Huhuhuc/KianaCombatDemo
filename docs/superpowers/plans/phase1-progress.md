# 阶段 1 进度：已自动搭建的资产与仍需手工连的节点

> 本文件记录 2026-10-02 通过 UE Python 远程执行实际写入项目的资产，以及 Python 无法代劳、必须在蓝图编辑器手连的部分。

## 一、已创建并验证的资产

### 输入层 `/Game/CombatDemo/Input`
- `IA_Move` AXIS2D、`IA_Look` AXIS2D
- `IA_Attack` / `IA_Branch` / `IA_Dodge` / `IA_Skill` Boolean
- `IMC_Combat`，9 条绑定：W/A/S/D → IA_Move；MouseXY2D → IA_Look；左键 → IA_Attack；右键 → IA_Branch；空格 → IA_Dodge；E → IA_Skill

### 关卡 `/Game/CombatDemo/Levels/L_CombatArena`
- ArenaFloor（Cube 缩放 40x40x1，z=-50）、四面墙 Wall_North/South/East/West
- ArenaSun（DirectionalLight）、ArenaSky（SkyAtmosphere）、ArenaSkyLight
- PlayerStart（0,0,100）、StrikeJaeger_01（600,0,100，yaw=180）
- WorldSettings.DefaultGameMode 已设为 BP_CombatGameMode（关卡级覆盖）

### 蓝图
- `BP_CombatGameMode`（父 GameModeBase），DefaultPawnClass = BP_KianaCharacter
- `BP_KianaCharacter`（父 Character）
  - 组件：CapsuleComponent / ArrowComponent / Mesh / CharacterMovement / SpringArm(400, UsePawnControlRotation) / Camera
  - Mesh 绑 `Avatar_Kiana_C2_Model_RootBone`，相对位置 (0,0,-90)，yaw=-90
  - AnimClass = ABP_Kiana
  - 移动：MaxWalkSpeed 600；Pawn 的 UseControllerRotationYaw = True
  - 变量：CombatState(byte)、bIsAttacking(bool)、CurrentHealth(real,默认 100)、MaxHealth(real,默认 100)、ComboIndex(int)
- `BP_StrikeJaeger`（父 Character）
  - Mesh 绑 `Monster_StrikeJaeger_Model_RootBone`，相对位置 (0,0,-92)，yaw=-90
  - AnimClass = ABP_StrikeJaeger
  - 变量：MaxHealth(300)、CurrentHealth(300)、EnemyState(byte)、AttackCooldown(real)、bIsDead(bool)、AttackIndex(int)
- `ABP_Kiana`（目标骨骼 Avatar_Kiana_C2_Model_RootBone_Skeleton）
- `ABP_StrikeJaeger`（目标骨骼 Monster_StrikeJaeger_Model_RootBone_Skeleton）

### 项目配置 `Config/DefaultEngine.ini`
```
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Game/CombatDemo/Levels/L_CombatArena
GlobalDefaultGameMode=/Game/CombatDemo/Blueprints/BP_CombatGameMode.BP_CombatGameMode_C
```

## 二、PIE 验证结果

`editor_request_begin_play` 后实测：
- 生成 `BP_KianaCharacter_C_0` 于 (0,0,90.15)，`BP_StrikeJaeger_C_0` 于 (600,0,90.15)
- 两者 Mesh 的 SkeletalMeshAsset 与 AnimClass 均正确绑定
- Kiana 有 1 个 SpringArm + 1 个 Camera，敌人 0 个（符合预期）
- 无 DefaultPawn 生成，说明 GameMode 生效

## 三、Python 的能力边界（重要）

以下操作在 UE 5.8.1 的 Python API 中不可行，必须手工完成：
1. **铺蓝图节点图**：`new_object(K2Node_CallFunction)` 能建出对象，但 `function_reference` 未暴露给 Python，节点永远不会产生 pin，编译后消失。
2. **铺动画蓝图图表**：`AnimGraphNode_SequencePlayer` 同样能建出并写入 `node.sequence`，但 `list_all_pins()` 恒为空，没有 `allocate_default_pins`，无法连线，编译后节点不保留。
3. **按动画单独设置 Root Motion 缩放**：`AnimRootMotionTranslationScale` 是 Character 上的属性，Python 只有 `get_anim_root_motion_translation_scale()`，没有 setter，且 `set_editor_property` 找不到该属性。

可行且已验证的：创建资产、加组件、设默认值、加成员变量、改动画的 `enable_root_motion`、生成关卡与 Actor、读写 INI。

## 四、Root Motion 已校准数据（cm）

| 动画 | 时长 s | 根位移 | 已启用 |
|---|---:|---:|---|
| Kiana_Walk | 1.067 | +157.2 (Y) | 是，PIE 实测约 144 cm/s |
| Kiana_Run | 1.200 | +655.7 (Y) | 是，PIE 实测约 550 cm/s |
| Kiana_EvadeFront | 1.867 | +364.4 | 是 |
| Kiana_EvadeBack | 1.333 | -201.1 | 是 |
| Kiana_Attack1 | 1.750 | +419.8 | 否 |
| Kiana_Attack2 | 2.500 | +507.8 | 否 |
| Kiana_Attack3 | 2.583 | +643.2 | 否 |
| Kiana_Attack3_fenzhi | 2.833 | +695.5 | 否 |
| Kiana_Attack4 | 3.667 | +748.6 | 否 |
| Kiana_Attack4_fenzhi | 3.817 | +757.9 | 否 |
| Kiana_Attack5 | 4.333 | +977.0 | 否 |
| Kiana_Skill | 1.417 | 0 | 否 |
| Kiana_Hit_H | 1.383 | -493.7 | 否 |
| Kiana_Hit_L | 1.050 | -73.1 | 否 |
| Jaeger_Attack01 | 1.400 | +169.3 | 否 |
| Jaeger_Attack02 | 4.200 | +2206.1 | 否（危险，需限制） |
| Jaeger_Attack03 | 5.200 | +961.1 | 否 |
| Jaeger_Walk_F | 1.400 | +123.6 | 否 |
| Jaeger_Run_Start/Loop/End | 0.700/0.467/1.167 | +504.8/+472.5/+534.6 | 否 |


---

# 追加：Montage / Notify / 变量 资产已生成（同日第二轮）

## 新增资产

### Montage（21 个，均由 AnimMontageFactory 从对应动画生成，Slot 为 DefaultSlot）
`/Game/CombatDemo/Characters/Kiana/Montages`（12）
`AM_Kiana_Attack1/2/3/4/5`、`AM_Kiana_Attack3_Branch`(Attack3_fenzhi)、`AM_Kiana_Attack4_Branch`(Attack4_fenzhi)、
`AM_Kiana_EvadeFront/Back`、`AM_Kiana_Skill`、`AM_Kiana_Hit_H/L`

`/Game/CombatDemo/Characters/StrikeJaeger/Montages`（9）
`AM_Jaeger_Attack01/02/03`、`AM_Jaeger_Hit_H/L`、`AM_Jaeger_Stun_Start/Loop/End`、`AM_Jaeger_Death`

### Notify State 蓝图（`/Game/CombatDemo/Blueprints/Components`）
- `ANS_ComboWindow`：连段输入窗口
- `ANS_AttackWindow`：攻击判定窗口
- `ANS_DodgeIFrame`：闪避无敌帧
- `ANS_SkillWindow`：技能判定窗口

这 4 个是 AnimNotifyState 的空白子类，Python 无法为它们铺事件图（`add_function_override` 返回 None、无 event 可挂），
**必须在编辑器里手工重写 Received_NotifyBegin / Received_NotifyEnd**，内容见下文。

### Notify 窗口已按时间写入 Montage
Kiana（AttackWindow 判定 / ComboWindow 连段）：

| Montage | 长度 | AttackWindow | ComboWindow |
|---|---:|---:|---:|
| Attack1 | 1.75 | 0.35→0.80 | 0.90→1.60 |
| Attack2 | 2.50 | 0.40→0.90 | 1.10→1.90 |
| Attack3 | 2.58 | 0.45→1.00 | 1.20→2.00 |
| Attack3_Branch | 2.83 | 0.45→1.00 | 1.20→2.10 |
| Attack4 | 3.67 | 0.50→1.10 | 1.40→2.30 |
| Attack4_Branch | 3.82 | 0.50→1.10 | 1.40→2.40 |
| Attack5 | 4.33 | 0.60→1.30 | 1.80→2.80 |
| EvadeFront / EvadeBack | 1.87 / 1.33 | — | IFrame 0.10→0.70 |
| Skill | 1.42 | SkillWindow 0.30→0.80 | — |

Jaeger：Attack01 0.50→0.90；Attack02 1.20→1.80；Attack03 1.50→2.20（全部 AttackWindow）

## Root Motion 启用状态（已更新）

Kiana 全部攻击/闪避/技能/受击动画均已 `set_root_motion_enabled(True)`。按 位移/时长 折算的峰值速度都在 200-360 cm/s，
低于 Run 的 550 cm/s，因此**不再需要按动画缩放**，之前的"Attack5 9.77 米瞬移"顾虑解除——它是 4.33 秒里的累计位移，不是瞬移。

Jaeger 同理启用了 Attack01(121 cm/s)、Attack03(185 cm/s)、Walk_F(88)、Run_Start(721)、Run_Loop(1013)、Run_End(458)。
**Attack02 刻意保持禁用**：22 米位移摊在 4.2 秒里是 525 cm/s，但那是一段长冲刺，开启会让敌人冲出场地。
用法：播 Attack02 时保持 Root Motion 关闭，改用蓝图手动位移或缩小场地尺度。

## 蓝图变量已补齐（含默认值）

### BP_KianaCharacter
Montage 引用：`AM_Attack1/2/3/3_Branch/4/4_Branch/5`、`AM_EvadeFront/Back`、`AM_Skill`、`AM_Hit_H/L`（已全部指向对应资产）
状态：`CombatState`(byte)、`bIsAttacking`、`bCanCombo`、`bBranchRequested`、`bDodgeInvuln`、`bIsDead`(bool)
数值：`MaxHealth`=100、`CurrentHealth`=100、`ComboIndex`(int)、`SkillCooldown`、`MoveSpeedInput`、`CachedMoveInput`(real)

### BP_StrikeJaeger
Montage 引用：`AM_Attack01/02/03`、`AM_Hit_H/L`、`AM_Stun_Start/Loop/End`、`AM_Death`（已全部指向对应资产）
数值：`MaxHealth`=300、`CurrentHealth`=300、`DetectRadius`=1200、`AttackRange`=250、`AttackInterval`=2.5
状态：`EnemyState`(byte)、`bIsDead`(bool)、`AttackIndex`(int)、`AttackCooldown`(real)

### BP_CombatGameMode
`DefaultPawnClass` = BP_KianaCharacter；`HUDWidgetClass` = WBP_CombatHUD（存的是 WidgetBlueprint 资产引用，不是 GeneratedClass）

### WBP_CombatHUD
变量：`PlayerHealthBar`/`EnemyHealthBar`(ProgressBar)、`StatusText`(TextBlock)。
**控件树需手工搭建**：Python 的 WidgetBlueprint 没有 widget_tree 接口，无法创建 Canvas/ProgressBar/TextBlock 控件。

