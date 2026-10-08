# 提亚娜动作 Demo 对比与改进记录

## 参考项目

- [Unreal-combo-moves](https://github.com/RaduLecca-codingStuff/Unreal-combo-moves)：基础连招、输入缓存与连招窗口。
- [ActionSwordCombat](https://github.com/mediie/ActionSwordCombat)：UE5 C++ 模块化战斗、锁定与连招结构。
- [FeatureWarehouse](https://github.com/Movesky98/FeatureWarehouse)：UE5 RPG 战斗功能拆分。
- [ComboGraph](https://github.com/apokrif6/ComboGraph)：将连招节点和分支数据化的思路。

## 对比结论

| 模块 | 公开项目常见做法 | 当前提亚娜 Demo | 下一步 |
| --- | --- | --- | --- |
| 输入 | Started 事件进入单槽位缓存 | 已实现 `bBufferedAttack` | 保持单槽位，不堆积输入 |
| 连招 | ComboIndex + 当前段锁定 | 已实现五段索引 | 迁移到 Montage Section |
| 窗口 | 动画通知分别控制命中和连招 | 已有 C++ 窗口计时器和 Notify 类 | 用实际 Notify 校准帧 |
| 旧回调 | 版本号或状态 token 失效 | 已加入 `ComboGeneration` | 延续到闪避、受击和技能 |
| 命中 | 每段一次，按武器 Socket 或 Trace | 角色前方 Sphere Trace，每段清空命中集合 | 有武器 Socket 后替换 Trace |
| 朝向 | 攻击前锁定目标 | 已补充攻击前自动朝向敌人 | 加锁定目标状态 |
| 移动 | 状态机/BlendSpace | 当前为单节点 Idle/Walk/Run | 第二阶段制作 AnimBP |
| 反馈 | 受击、硬直、死亡优先级明确 | 已有轻/重受击和死亡 | 完成 Stun Loop 和 HUD |

## 当前五段连招规则

`Attack1 -> Attack2 -> Attack3 -> Attack4 -> Attack5 -> Idle`

- 左键 Started 只写入一个下一段请求。
- 连招窗口消费请求，每段最多推进一次。
- 每段生成新的 `ComboGeneration`，旧计时器回调自动失效。
- 第五段结束清空索引和缓存。
- 分支攻击仍由第三、第四段的分支状态选择。

## 迁移到 Montage 的条件

当前单节点方案用于验证输入和动作资产。达到以下条件后迁移到 AnimMontage：

1. 五段连招在 PIE 中连续测试不再出现段数回退。
2. 每个动作的命中帧和连招帧已经确认。
3. 建立 `Attack1` 到 `Attack5` 的 Montage Section。
4. 用 `Montage_SetNextSection` 替代单节点重新播放。

迁移后可获得更稳定的段间混合、通知帧和根运动控制。
