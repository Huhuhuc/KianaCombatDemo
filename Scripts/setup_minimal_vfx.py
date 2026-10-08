import unreal
print('PIE',unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).is_in_play_in_editor())
for name,template in [('NS_Hit','DirectionalBurst'),('NS_Skill','RadialBurst'),('NS_Dodge','DirectionalBurst')]:
 path='/Game/CombatDemo/VFX/'+name
 a=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.EditorAssetLibrary.duplicate_asset('/Niagara/DefaultAssets/Templates/Systems/'+template,path)
 assert a, path
 unreal.EditorAssetLibrary.save_loaded_asset(a)
 print(name,a.get_class().get_name())
