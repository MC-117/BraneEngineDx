# BraneEngineDx
>by Luo Yizhou

![BraneEngine_Temple](https://user-images.githubusercontent.com/28572690/234598251-b7085760-c252-4eba-947b-098ae16915b8.jpg)

## Introduction
BraneEngine is a 3D game engine, which build with ImGui, PhysX and OpenAL. Engine has supported multiply rendering features, such as SSR, GI probe system, volumtric fog, GTAO, DOF and so on. For game play, engine provides python scipt and graph system. Also, engine provides animation graph based on graph system.

## Execution Root Folder Layout
```
├─Content
│  └─...
├─Engine
│  ├─Banner
│  ├─Fonts
│  ├─Icons
│  ├─LUTS
│  ├─Shaders
│  │  ├─Default
│  │  ├─lib
│  │  └─PostProcess
│  ├─Shapes
│  ├─SkySphere
│  └─Textures
├─BraneEngineDx.exe
├─Config.ini
├─NvCloth_x64.dll
├─OpenAL32.dll
├─PhysX_64.dll
├─PhysXCommon_64.dll
├─PhysXCooking_64.dll
├─PhysXDevice64.dll
├─PhysXFoundation_64.dll
└─PhysXGpu_64.dll
```
* Content Folder: place user assets. Demo content folder is at 'BraneEngineDx/Context'

* Engine Folder: Engine default resource. The default Engine folder can be found at 'BraneEngineDx/Engine'

* Config.ini: default configure file. PS: user can assgin custom config file by execution argument.

* *.dll: all the dll file can be found in 'BraneEngineDx/ThirdParty' folder.

## Multithread Rendering
![MultithreadRendering](https://user-images.githubusercontent.com/28572690/186325609-956793c5-996f-49b0-8de7-928651dfcd4a.svg)

Engine use extra rendering thread to process rendering task, which alse support parallel disptch mesh draw. DeferredRenderGraph and ForwardRenderGraph both are embedded in engine.

## Showcase
>Base Editor
>![BraneEngine_Suzu](https://user-images.githubusercontent.com/28572690/234598489-88e382f6-6212-4677-88e6-608a348f231a.jpg)

>GraphSystem&AnimationGraph
>![BraneEngine_AnimationGraph1](https://user-images.githubusercontent.com/28572690/234598796-2cf0ed80-e15b-4588-9d12-a52c4702418f.png)
>![BraneEngine_AnimationGraph2](https://user-images.githubusercontent.com/28572690/234598831-75aeb978-713a-4a14-86d3-34df9253eb37.png)
>![BraneEngine_AnimationGraph3](https://user-images.githubusercontent.com/28572690/234598864-361845a8-910e-4ed3-9ff2-74cabf93027d.png)
>![BraneEngine_AnimationGraph4](https://user-images.githubusercontent.com/28572690/234598913-52bd5578-2e56-442a-af07-7fed4dd2c85f.png)

>Captures
>![Capture_Suzume](https://user-images.githubusercontent.com/28572690/234599055-4a3a321b-9fe7-436c-9a18-37fa1c83da01.png)
>![Capture_Park](https://user-images.githubusercontent.com/28572690/234599087-3c6dbbed-0667-4c53-aa7b-2dcbba619d8d.png)
