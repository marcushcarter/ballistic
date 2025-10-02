#pragma once

namespace BE::Default {
    
inline const std::string defaults_dsl = R"(

@vs vertex_shader

#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTex;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    TexCoord = aTex;
    Normal = mat3(transpose(inverse(uModel))) * aNormal;
    FragPos = vec3(uModel * vec4(aPos, 1.0));
}

@end

@fs scene_fragment

#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint outID;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform vec4 diffuseColor;

uniform uint objectID;

uniform bool enableLights;
uniform float ambientLight;
uniform int numLights;
struct Light {
    vec4 position;
    vec4 color;
    vec4 direction;
    mat4 shadowMatrices[2];
};

layout(std430, binding = 0) buffer LightBlock {
    Light lights[];
};

vec3 calcLight(int index) {
    vec3 lightColor = lights[index].color.rgb * lights[index].color.a;
    float diff = 0.0;

    if (lights[index].position.w == 0.0) {
        vec3 lightDir = normalize(-lights[index].direction.xyz);
        diff = max(dot(normalize(Normal), lightDir), 0.0);
    } 
    else if (lights[index].position.w == 1.0) {
        vec3 lightDir = normalize(lights[index].position.xyz - FragPos);
        float distance = length(lights[index].position.xyz - FragPos);
        float attenuation = 1.0 / (distance * distance);
        diff = max(dot(normalize(Normal), lightDir), 0.0) * attenuation;

    } 
    else if (lights[index].position.w == 2.0) {
        vec3 lightDir = normalize(lights[index].position.xyz - FragPos);
        float theta = dot(lightDir, normalize(-lights[index].direction.xyz));
        if (theta > lights[index].direction.w) {
            float distance = length(lights[index].position.xyz - FragPos);
            float attenuation = 1.0 / (distance * distance);
            diff = max(dot(normalize(Normal), lightDir), 0.0) * attenuation;
        }
    }

    return lightColor * diff;
}

void main() {
    outID = objectID + 1;

    vec4 textureColor = vec4(texture(diffuseMap, TexCoord).rgb, 1.0);

    vec4 lightColor = vec4(1);
    if (enableLights) {
        vec3 finalColor = vec3(ambientLight);
        for (int i = 0; i < numLights; i++) {
            finalColor += calcLight(i);
        }
        lightColor = vec4(finalColor, 1);
    }

    vec4 baseColor = textureColor * lightColor * diffuseColor;

    FragColor = baseColor;
}

@end

@fs uv_fragment

#version 460 core
in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint outID;
uniform uint objectID;
void main() {
    outID = objectID + 1;
    float colormix = 0.5 * TexCoord.x + 0.5 * TexCoord.y;
    FragColor = vec4(TexCoord, 1, 1.0);
}

@end

@fs color_fragment
#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint outID;
uniform vec4 diffuseColor;
uniform uint objectID;
void main() {
    outID = objectID + 1;
    FragColor = vec4(diffuseColor.xyz, 1.0);
}

@end

@program default_basic vertex_shader scene_fragment
@program default_uv vertex_shader uv_fragment
@program default_color vertex_shader color_fragment

)";

inline const std::string FallbackTexture =
    std::string(
        "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF"
        "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF" "\xFF\xFF\xFF\xFF" "\xFF\x00\xFF\xFF",
        64
    );

inline const std::string cube_obj = R"(

# Blender 4.4.0
# www.blender.org
o Cube
v 1.000000 1.000000 -1.000000
v 1.000000 -1.000000 -1.000000
v 1.000000 1.000000 1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 1.000000 -1.000000
v -1.000000 -1.000000 -1.000000
v -1.000000 1.000000 1.000000
v -1.000000 -1.000000 1.000000
vn -0.0000 1.0000 -0.0000
vn -0.0000 -0.0000 1.0000
vn -1.0000 -0.0000 -0.0000
vn -0.0000 -1.0000 -0.0000
vn 1.0000 -0.0000 -0.0000
vn -0.0000 -0.0000 -1.0000
vt 0.625000 0.500000
vt 0.875000 0.500000
vt 0.875000 0.750000
vt 0.625000 0.750000
vt 0.375000 0.750000
vt 0.625000 1.000000
vt 0.375000 1.000000
vt 0.375000 0.000000
vt 0.625000 0.000000
vt 0.625000 0.250000
vt 0.375000 0.250000
vt 0.125000 0.500000
vt 0.375000 0.500000
vt 0.125000 0.750000
f 1/1/1 5/2/1 7/3/1 3/4/1
f 4/5/2 3/4/2 7/6/2 8/7/2
f 8/8/3 7/9/3 5/10/3 6/11/3
f 6/12/4 2/13/4 4/5/4 8/14/4
f 2/13/5 1/1/5 3/4/5 4/5/5
f 6/11/6 5/10/6 1/1/6 2/13/6

)";

inline const std::string quad_obj = R"(

# Blender 4.4.0
# www.blender.org
o Quad
v -1.0 -1.0 0.0
v 1.0 -1.0 0.0
v 1.0  1.0 0.0
v -1.0  1.0 0.0
vt 0.0 0.0
vt 1.0 0.0
vt 1.0 1.0
vt 0.0 1.0
f 1/1 2/2 3/3
f 3/3 4/4 1/1

)";

inline const std::string ini = R"(

[Window][Debug##Default]
Pos=261,261
Size=400,400
Collapsed=0

[Window][Hello, ImGui!]
Pos=168,27
Size=926,865
Collapsed=0
DockId=0x00000005,0

[Window][Dear ImGui Demo]
Pos=8,46
Size=379,846
Collapsed=0
DockId=0x00000002,0

[Window][Dear ImGui Style Editor]
Pos=829,-21
Size=353,894
Collapsed=0

[Window][Example: Assets Browser]
Pos=1052,291
Size=380,601
Collapsed=0
DockId=0x00000004,0

[Window][Example: Console]
Pos=370,493
Size=680,399
Collapsed=0
DockId=0x00000006,0

[Window][Dear ImGui Debug Log]
Pos=138,165
Size=365,308
Collapsed=0

[Window][Example: Custom rendering]
Pos=144,48
Size=623,414
Collapsed=0

[Window][Dockspace Root]
Size=1440,900
Collapsed=0

[Window][Scene View]
Pos=294,92
Size=1103,721
Collapsed=0
DockId=0x00000008,2

[Window][Console]
Pos=294,92
Size=1103,721
Collapsed=0
DockId=0x00000008,1

[Window][Dockspace]
Pos=0,19
Size=1440,881
Collapsed=0

[Window][Example: Auto-resizing window]
Pos=-103,-103
Size=514,352
Collapsed=0

[Window][Same title as another window##1]
Pos=-131,33
Size=451,61
Collapsed=0

[Window][Same title as another window##2]
Pos=-121,106
Size=451,61
Collapsed=0

[Window][###AnimatedTitle]
Pos=-64,197
Size=247,48
Collapsed=0

[Window][Example: Long text display]
ViewportPos=1225,309
ViewportId=0x24F375AC
Size=520,600
Collapsed=0

[Window][Example: Log]
Pos=-103,-103
Size=500,400
Collapsed=0

[Window][Example: Simple layout]
Pos=8,46
Size=360,846
Collapsed=0
DockId=0x00000007,0

[Window][Example: Simple layout/left pane_244A9CDE]
IsChild=1
Size=108,792

[Window][console]
Pos=8,46
Size=709,846
Collapsed=0
DockId=0x00000009,0

[Window][Inspector]
Pos=1096,27
Size=336,865
Collapsed=0
DockId=0x0000000C,0

[Window][Heirarchy]
Pos=8,27
Size=158,407
Collapsed=0
DockId=0x00000013,0

[Window][Hier]
Pos=8,46
Size=218,846
Collapsed=0
DockId=0x0000000F,0

[Window][Plot WIndow]
Pos=356,185
Size=381,464
Collapsed=0

[Window][ImPlot Demo]
Pos=771,29
Size=199,846
Collapsed=1

[Window][Heirea]
Pos=8,46
Size=231,846
Collapsed=0
DockId=0x00000011,0

[Window][Resources]
Pos=8,436
Size=158,456
Collapsed=0
DockId=0x00000014,0

[Window][Ainvapi]
Pos=80,127
Size=569,294
Collapsed=0

[Window][TestWindow]
Pos=435,307
Size=468,400
Collapsed=0

[Window][Edit Material]
Pos=347,292
Size=278,374
Collapsed=0

[Window][Choose a File##ChooseFileDlg]
Pos=258,225
Size=580,478
Collapsed=0

[Table][0xB6880529,2]
RefScale=13
Column 1  Sort=0v

[Table][0x0F51256F,4]
RefScale=13
Column 0  Sort=0v

[Table][0x77FF49E1,4]
RefScale=13
Column 0  Sort=0v

[Table][0x9250B595,4]
RefScale=13
Column 0  Sort=0v

[Table][0xAC638DBB,4]
RefScale=13
Column 0  Sort=0v

[Table][0x4BC6A05F,4]
RefScale=13
Column 0  Sort=0v

[Docking][Data]
DockSpace                   ID=0x382E4429 Window=0x260A4489 Pos=188,134 Size=1424,865 Split=X
  DockNode                  ID=0x0000000D Parent=0x382E4429 SizeRef=158,846 Split=Y Selected=0x0B397EC5
    DockNode                ID=0x00000013 Parent=0x0000000D SizeRef=177,398 Selected=0x0B397EC5
    DockNode                ID=0x00000014 Parent=0x0000000D SizeRef=177,446 Selected=0x30401527
  DockNode                  ID=0x0000000E Parent=0x382E4429 SizeRef=1264,846 Split=X
    DockNode                ID=0x0000000B Parent=0x0000000E SizeRef=926,846 Split=X
      DockNode              ID=0x00000011 Parent=0x0000000B SizeRef=231,846 Selected=0x8CAC0286
      DockNode              ID=0x00000012 Parent=0x0000000B SizeRef=1191,846 Split=X
        DockNode            ID=0x0000000F Parent=0x00000012 SizeRef=218,846 Selected=0x60F6AF87
        DockNode            ID=0x00000010 Parent=0x00000012 SizeRef=1204,846 Split=X
          DockNode          ID=0x00000009 Parent=0x00000010 SizeRef=709,846 Selected=0x51A2ACB4
          DockNode          ID=0x0000000A Parent=0x00000010 SizeRef=713,846 Split=X
            DockNode        ID=0x00000002 Parent=0x0000000A SizeRef=379,846 Selected=0x5E5F7166
            DockNode        ID=0x00000003 Parent=0x0000000A SizeRef=1043,846 Split=X
              DockNode      ID=0x00000007 Parent=0x00000003 SizeRef=360,846 Selected=0x734921F1
              DockNode      ID=0x00000008 Parent=0x00000003 SizeRef=1062,846 Split=X
                DockNode    ID=0x00000001 Parent=0x00000008 SizeRef=680,846 Split=Y
                  DockNode  ID=0x00000005 Parent=0x00000001 SizeRef=680,445 CentralNode=1 Selected=0xF1729921
                  DockNode  ID=0x00000006 Parent=0x00000001 SizeRef=680,399 Selected=0x1BCA3180
                DockNode    ID=0x00000004 Parent=0x00000008 SizeRef=380,846 Selected=0x5E5F7166
    DockNode                ID=0x0000000C Parent=0x0000000E SizeRef=336,846 Selected=0x36DC96AB

)";

};

namespace BE::Default {
    
inline const std::string baseSceneVertex = R"(

    #version 460 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec3 aColor;
    layout(location = 3) in vec2 aTex;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProjection;

    out vec2 TexCoord;
    out vec3 Normal;
    out vec3 FragPos;

    void main() {
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        TexCoord = aTex;
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
        FragPos = vec3(uModel * vec4(aPos, 1.0));
    }

)";

inline const std::string baseSceneFragmentGlobal = R"(

    #version 460 core

    layout(location = 0) out vec4 FragColor;
    layout(location = 1) out uint outID;

    in vec2 TexCoord;
    in vec3 Normal;
    in vec3 FragPos;

    uniform sampler2D diffuseMap;
    uniform sampler2D normalMap;
    uniform sampler2D roughnessMap;
    uniform vec4 diffuseColor;

    uniform uint objectID;

    uniform bool enableLights;
    uniform float ambientLight;
    uniform int numLights;
    struct Light {
        vec4 position;
        vec4 color;
        vec4 direction;
        mat4 shadowMatrices[2];
    };

    layout(std430, binding = 0) buffer LightBlock {
        Light lights[];
    };

    vec3 calcLight(int index) {
        vec3 lightColor = lights[index].color.rgb * lights[index].color.a;
        float diff = 0.0;

        if (lights[index].position.w == 0.0) {
            vec3 lightDir = normalize(-lights[index].direction.xyz);
            diff = max(dot(normalize(Normal), lightDir), 0.0);
        } 
        else if (lights[index].position.w == 1.0) {
            vec3 lightDir = normalize(lights[index].position.xyz - FragPos);
            float distance = length(lights[index].position.xyz - FragPos);
            float attenuation = 1.0 / (distance * distance);
            diff = max(dot(normalize(Normal), lightDir), 0.0) * attenuation;

        } 
        else if (lights[index].position.w == 2.0) {
            vec3 lightDir = normalize(lights[index].position.xyz - FragPos);
            float theta = dot(lightDir, normalize(-lights[index].direction.xyz));
            if (theta > lights[index].direction.w) {
                float distance = length(lights[index].position.xyz - FragPos);
                float attenuation = 1.0 / (distance * distance);
                diff = max(dot(normalize(Normal), lightDir), 0.0) * attenuation;
            }
        }

        return lightColor * diff;
    }

)";

inline const std::string baseSceneFragmentMain = R"(

    void main() {
        outID = objectID + 1;

        vec4 textureColor = vec4(texture(diffuseMap, TexCoord).rgb, 1.0);

        vec4 lightColor = vec4(1);
        if (enableLights) {
            vec3 finalColor = vec3(ambientLight);
            for (int i = 0; i < numLights; i++) {
                finalColor += calcLight(i);
            }
            lightColor = vec4(finalColor, 1);
        }

        vec4 baseColor = textureColor * lightColor * diffuseColor;

)";

// inline const std::string baseSceneVertex = R"()";

};
