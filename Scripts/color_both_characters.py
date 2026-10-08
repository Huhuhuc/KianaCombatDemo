import unreal
from pathlib import Path
exec(Path('E:/mihoyou/KianaCombatDemo/Scripts/color_kiana.py').read_text())
player_materials=[mats[p] for p in parts]
base='/Game/CombatDemo/Characters/StrikeJaeger'
textures={}
for suffix in ['D','N']:
    path=base+'/Textures/StrikeJaeger_'+suffix
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        t=unreal.AssetImportTask()
        t.filename='E:/mihoyou/KianaCombatDemo/SourceAssets/StrikeJaeger/Texture/StrikeJaeger_'+suffix+'.tga'
        t.destination_path=base+'/Textures';t.automated=True;t.save=True
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    tex=unreal.load_asset(path)
    tex.set_editor_property('srgb',suffix=='D')
    if suffix=='N':tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
    unreal.EditorAssetLibrary.save_loaded_asset(tex,False)
    textures[suffix]=tex
path=base+'/Materials/M_StrikeJaeger_Color'
mat=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_StrikeJaeger_Color',base+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(mat)
for suffix,prop in [('D',unreal.MaterialProperty.MP_BASE_COLOR),('N',unreal.MaterialProperty.MP_NORMAL)]:
    sample=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample,-400,0 if suffix=='D' else 200)
    sample.texture=textures[suffix]
    if suffix=='N':sample.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    assert lib.connect_material_property(sample,'RGB',prop)
rough=lib.create_material_expression(mat,unreal.MaterialExpressionConstant,-400,400);rough.r=0.65
lib.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
mat.set_editor_property('used_with_skeletal_mesh',True)
lib.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat,False)
for meshpath,bppath,materials in [
('/Game/CombatDemo/Characters/Kiana/Meshes/Avatar_Kiana_C2_Model_RootBone','/Game/CombatDemo/Blueprints/Player/BP_KianaCharacter_Cpp',player_materials),
(base+'/Meshes/Monster_StrikeJaeger_Model_RootBone','/Game/CombatDemo/Blueprints/Enemy/BP_StrikeJaeger_Cpp',[mat])]:
    mesh=unreal.load_asset(meshpath);slots=mesh.get_editor_property('materials')
    for i,material in enumerate(materials):
        slot=slots[i];slot.set_editor_property('material_interface',material);slots[i]=slot
    mesh.set_editor_property('materials',slots)
    assert unreal.EditorAssetLibrary.save_loaded_asset(mesh,False)
    bp=unreal.load_asset(bppath);cdo=unreal.get_default_object(bp.generated_class())
    component=cdo.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_editor_property('override_materials',materials)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
    print('SAVED',bppath,[str(component.get_material(i)) for i in range(len(materials))])
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
