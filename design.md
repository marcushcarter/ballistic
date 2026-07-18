## Visibility Cell

This is a engine resource where any children are not rendered unless inside of the cell (includes meshes, lights and particle emitters).

```bash
Scene
├── Visibility Cell 0
│   ├── Meshes
│   ├── Lights
│   ├── Particle Emitters
│   └── Visibility Cell 4
│       ├── Meshes
│       ├── Lights
│       ├── Particle Emitters
├── Visibility Cell 1
├── Visibility Cell 2
└── Outside
```

## APP DATA

```bash
Local
├── shader_cache
│   ├── 092wanlsmoc97g3js.spv
│   ├── asubca9neio2nbc92.spv
│   └── ...
└── ...

Roaming Root
├── app_userdata
│   ├── <project_name>
│   │   ├── logs
│   │   │   ├── ballistic2026-02-05.log
│   │   │   └── ballistic2026-03-04.log
│   │   └── pipeline_cache
│   │       └── pipeline_cache.bin
│   └── ...
├── themes
│   ├── <theme_name>.theme
│   ├── dark.theme
│   └── ...
├── editor_layout.cfg
├── editor_settings.cfg
├── projects.cfg
├── recent_dirs.cfg
└── ...
```

# PIPELINE

frustum cull
main z pass
hi z build
occlusion + frustum cull

geometry pass (dt = equal)

hbao + ao attachment + bilateral blur
clustered light culling

deferred lighting

screen space sss (combine with hdr)
sky + atmosphere (where depth  = far)
hair pass
raymarching pass (lava lamp etc)
transparent sort pass (back to front)
transparent pass

ssr
volumetric fog

post processing

ui pass

# FEATURES

Visibility:
frustum cull -> main depth pass -> hi-z pyramid -> frustum + occlusion cull

Geometry:
gbuffer

AO:
Hbao -> bilateral blur

LightCulling:
