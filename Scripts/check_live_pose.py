import unreal, json, traceback, time
from pathlib import Path
out = Path('E:/mihoyou/KianaCombatDemo/Saved/LivePoseCheck.json')
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
rows = []
world = None
started = time.monotonic()
def finish(error=None):
    unreal.unregister_slate_post_tick_callback(handle)
    out.write_text(json.dumps(dict(samples=rows, error=error), indent=2))
    level.editor_request_end_play()
def tick(dt):
    global world, player, enemy, mesh, start, last
    try:
        if world is None:
            world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
            if world is None:
                if time.monotonic()-started > 30: finish('PIE start timeout')
                return
            player = unreal.GameplayStatics.get_player_character(world, 0)
            player.disable_input(unreal.GameplayStatics.get_player_controller(world,0))
            enemy = unreal.GameplayStatics.get_all_actors_of_class(world,unreal.StrikeJaeger)[0]
            enemy.set_editor_property('attack_damage',0.1)
            mesh = enemy.get_component_by_class(unreal.SkeletalMeshComponent)
            start = unreal.GameplayStatics.get_time_seconds(world)
            last = -1
        t = unreal.GameplayStatics.get_time_seconds(world)-start
        if t-last >= 0.1:
            last=t
            bones={}
            for i in range(mesh.get_num_bones()):
                name=mesh.get_bone_name(i)
                tr=mesh.get_socket_transform(name,unreal.RelativeTransformSpace.RTS_COMPONENT)
                bones[str(name)]=[tr.translation.x,tr.translation.y,tr.translation.z]
            a=enemy.get_active_sequence()
            rows.append(dict(t=t,dt=dt,clip=a.get_name() if a else '',speed=enemy.get_velocity().length(),bones=bones))
        if t>=12: finish()
    except Exception: finish(traceback.format_exc())
handle=unreal.register_slate_post_tick_callback(tick)
level.editor_request_begin_play()
