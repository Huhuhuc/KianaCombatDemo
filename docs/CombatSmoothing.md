# Combat smoothing — 2026-10-03

The approved scope is smooth no-VFX combat with the existing five-click combo,
Shift run, Space dodge and manual Tab targeting.

Both characters now use CombatAnimInstance, a single-node-compatible animation
instance. Its proxy caches the last evaluated local bone pose and blends from it
into each new sequence over 0.12 seconds using smoothstep. The incoming clip runs
immediately; damage timers and click queues are unchanged. Interrupted blends
start from the latest displayed pose. The blend does not carry outgoing velocity
or blend animation curves; a full locomotion graph remains a future improvement.

Enemy run-to-walk no longer inserts Run_End while the capsule keeps moving.
Walking and running loop rates follow actual velocity, clamped to 0.65–1.35.
Reference speeds are 150 and 300 cm/s; these are tuning values, not measured
source-animation stride lengths.

Validation: compile, repeat three five-click timing trials, inspect live enemy
bone motion and combat recovery. Automated checks do not establish subjective
visual quality. Editor background frame throttling is separate from animation
evaluation and is not fixed by forcing skeletal mesh ticks.
