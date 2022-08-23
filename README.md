# BraneEngineDx
DirectX version of BraneEngine.

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
├─ALUT.dll
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
<div><div class="mxgraph" style="max-width:100%;border:1px solid transparent;" data-mxgraph="{&quot;highlight&quot;:&quot;#0000ff&quot;,&quot;target&quot;:&quot;blank&quot;,&quot;lightbox&quot;:false,&quot;nav&quot;:true,&quot;resize&quot;:true,&quot;toolbar&quot;:&quot;zoom&quot;,&quot;edit&quot;:&quot;_blank&quot;,&quot;xml&quot;:&quot;&lt;mxfile host=\&quot;Electron\&quot; modified=\&quot;2022-08-23T12:09:28.108Z\&quot; agent=\&quot;5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) draw.io/14.4.3 Chrome/87.0.4280.141 Electron/11.3.0 Safari/537.36\&quot; etag=\&quot;V5tj4cGnnB-LpmRoJxxW\&quot; version=\&quot;14.4.3\&quot; type=\&quot;device\&quot;&gt;&lt;diagram id=\&quot;3H7gglzRRAbl8Wb1lq3Z\&quot; name=\&quot;第 1 页\&quot;&gt;5Zpfc6IwEMA/DY/OEFCqj2qr7c20ndbaTu/lJiUR6ADhYjj0Pv0FCComd+dNkT/XJ8MmhPDb3exuRDOnwWZOYeTeEoR9zdDRRjMvNcMAxlDnP6lkKyT60MglDvWQkO0FC+8nLgYKaewhvC4NZIT4zIvKQpuEIbZZSQYpJUl52Ir45adG0MGSYGFDX5a+eIi5uXRoXOzl19hz3OLJwBrlPQEsBos3WbsQkeRAZF5p5pQSwvJWsJliP6VXcMnvm/2md7cwikN2yg367bO5Zi+zIH69e3DeKXry73tilh/Qj8ULz2GAxYLZtqBASRwinE6ka+YkcT2GFxG0096EK57LXBb4/Arw5oqETCgSWPxaXmjxVEwZ3hyIxMLnmASY0S0fInrNAqIwo51dJQc6ESL3QB19IYPCCpzdzHtQvCFY/QO3vsRtGfkEotaR6/dbRm4gkXvEnBBtHbnBkc3tbPCA3A5TLeisbjjrMbjGTe6iK846bJuzjpQWN8OhXbHZIbh2s3tBNSStE0gaep0oi4RG2vg6QPM4/Cq3wnppygFY/xjDErM6Upj+iQyNszGUQzFoN0MpsigY9utlKMfkjtmhqdgZd5zrYSjH55bb4XGcVjE8mx3eoWDgPY0ve3R7s7SR8fDgE0Udt7ypNqZUEZT1vwflcyXUSmpGF6j1G6R28zYIv946z7Pr2H68+vY+ASTpypnBqMEyRMmtE9YG9LaZW2eOWoDRYPmmRCcnJy09azn21VrPWpTo5JykQGf5/NmTt7TlpK0O1G9SvqLAW6thDpUhpAMkgd7kwYKSpXxGY3yM4bmzZykw11nFqTMa+WxmkXAsbdsmL6yW+TGQc8FWggOgQXJOtOotnXn8ZTqNFzpaofG9KoluNr5ILBXET898Gscr59rn3xQrRnhi8ljFpqhEaEoIC9tMX7+E0voek6Kjt87AjPkAYEWbfWdhz1mkp1nFmE/HV5fPmPdLeuJQWVkZ0PeckLdtDpe7jDlJ0Xs29MeiI/AQSm+fUMyXA9+yqVItR8QLWUZqMNEGl+lcMSNroctqVLn7duEP8Q2AOlUpl1DVqFL8gfGZlKlyy3qVKZ/aV+eXTy7FaWX9OXRpjJrWpVygV+mY/7k29UHZM02FNiv6I4Nf7j/UyvoOvnczr34B&lt;/diagram&gt;&lt;/mxfile&gt;&quot;}"></div><script type="text/javascript" src="https://viewer.diagrams.net/js/viewer-static.min.js"></script></div>