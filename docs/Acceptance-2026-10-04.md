# 当前验收台账

更新时间：2026-10-04。以下战斗结果来自 2026-10-03 保存的报告，不是本日重新实机通过。

| 项目 | 证据 | 结论 |
| --- | --- | --- |
| 敌人行为 | Saved/EnemyCombatRuntime.json | 12/12 通过；包括前后受击、技能眩晕恢复、三个方向闪避、死亡等；不是全游戏无 Bug 证明 |
| 快速五次点击 | Saved/PlayabilityCheck.json | 0.12 秒、0.35 秒间隔失败；0.55 秒通过 |
| 输入释放 | Saved/AttackReleaseCheck.json | 有界、顺序攻击及无新输入回到待机通过；不等价于五次点击必出五段 |
| 移动恢复 | Saved/MovementRecovery.json | 攻击后恢复移动、跑步释放、玩家急停通过 |
| 难度生成 | Saved/DifficultyCheck.json | 函数调用检查 1/4 敌人；真实按键及四敌人综合体验待验 |
| 坠落死亡 | Saved/VoidDeathCheck.json | 死亡、血量归零、保留角色通过 |
| 旧角色总回归 | Saved/KianaRuntimeValidation.json | 停留 running，不作为通过证据 |
| 实体手柄 | 无实机报告 | 待验 |

HUD、首页、难度、血条、伤害数字及胜负重试已有实现。旧 UI-VFX 设计中的未实装 UI、9/12 结论已过时。Niagara 新接入需独立编译及运行验证，不能沿用旧测试签收。

最终交付仍缺独立 exe、完整项目归档和带说明的最终视频。Submission-AG.md 为 A–G 说明，F 是视频录制方案，E 包含未完成来源核验项。

## 本轮新增 Niagara 验证
2026-10-04：KianaCombatDemoEditor Win64 Development 编译 Result: Succeeded。Saved/VFXCheck.json 为 passed：通过 PIE 调用技能、闪避与玩家受伤，观察到 NS_Skill、NS_Dodge、NS_Hit，峰值反馈演员 1，结束剩余 0。验证覆盖生成绑定和短生命周期，不覆盖实际粒子视觉、敌人受击方向、多敌人同屏帧率、空挥/无敌视觉和独立打包。测试结束退出 PIE，未修改连击缓存规则。

2026-10-04 连招更新：改用单轮计数缓存。ComboCountCheck.json 的 0.01 秒间隔 8 次、0.35 秒间隔 5 次、0.55 秒间隔 5 次均只按序执行五段并回待机。旧单槽快速点击失败记录保留为历史，不代表最新版本。测试通过 debug 调用进入同一攻击处理器，尚非实体鼠标实测。
ComboCancelCheck.json：单次输入仅第一段、闪避取消、技能取消、轻受击取消四项通过。
