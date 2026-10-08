import unreal
def begin_kiana_test(_dt):
    if unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world():
        unreal.unregister_slate_post_tick_callback(kiana_boot_callback)
        exec(open('E:/mihoyou/KianaCombatDemo/Scripts/test_kiana_runtime.py',encoding='utf-8').read(),globals())
kiana_boot_callback=unreal.register_slate_post_tick_callback(begin_kiana_test)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
