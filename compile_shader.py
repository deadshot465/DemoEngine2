import io
import os
import shutil

os.chdir('./GLVK/VK/Shaders')
os.system('glslangValidator -V basicShader.vert')
os.system('glslangValidator -V basicShader_mesh.vert -o basicShader_mesh.spv')
os.system('glslangValidator -V basicShader.frag')
os.chdir('../../../')
shutil.copyfile('./GLVK/VK/Shaders/vert.spv', 'x64/Debug/GLVK/VK/Shaders/vert.spv')
shutil.copyfile('./GLVK/VK/Shaders/basicShader_mesh.spv', 'x64/Debug/GLVK/VK/Shaders/basicShader_mesh.spv')
shutil.copyfile('./GLVK/VK/Shaders/frag.spv', 'x64/Debug/GLVK/VK/Shaders/frag.spv')

if os.path.isdir('cmake-build-debug'):
    shutil.copyfile('./GLVK/VK/Shaders/vert.spv', 'cmake-build-debug/GLVK/VK/Shaders/vert.spv')
    shutil.copyfile('./GLVK/VK/Shaders/basicShader_mesh.spv', 'cmake-build-debug/GLVK/VK/Shaders/basicShader_mesh.spv')
    shutil.copyfile('./GLVK/VK/Shaders/frag.spv', 'cmake-build-debug/GLVK/VK/Shaders/frag.spv')