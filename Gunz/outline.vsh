vs_1_1
dcl_position v0
dcl_blendweight v1
dcl_blendindices v2
dcl_normal v3


mov a0.x, v2.x
dp4 r1.x, v0, c[a0.x + 0]
dp4 r1.y, v0, c[a0.x + 1]
dp4 r1.z, v0, c[a0.x + 2]
mul r5.xyz, r1.xyz, v1.xxx


dp3 r3.x, v3, c[a0.x + 0]
dp3 r3.y, v3, c[a0.x + 1]
dp3 r3.z, v3, c[a0.x + 2]
mul r4.xyz, r3.xyz, v1.xxx


mov a0.x, v2.y
dp4 r1.x, v0, c[a0.x + 0]
dp4 r1.y, v0, c[a0.x + 1]
dp4 r1.z, v0, c[a0.x + 2]
mad r5.xyz, r1.xyz, v1.yyy, r5.xyz

dp3 r3.x, v3, c[a0.x + 0]
dp3 r3.y, v3, c[a0.x + 1]
dp3 r3.z, v3, c[a0.x + 2]
mad r4.xyz, r3.xyz, v1.yyy, r4.xyz


add r2.x, v1.x, v1.y
sub r2.x, c[0].x, r2.x
mov a0.x, v2.z
dp4 r1.x, v0, c[a0.x + 0]
dp4 r1.y, v0, c[a0.x + 1]
dp4 r1.z, v0, c[a0.x + 2]
mad r5.xyz, r1.xyz, r2.xxx, r5.xyz

dp3 r3.x, v3, c[a0.x + 0]
dp3 r3.y, v3, c[a0.x + 1]
dp3 r3.z, v3, c[a0.x + 2]
mad r4.xyz, r3.xyz, r2.xxx, r4.xyz


mul r0.xyz, r4.xyz, c[81].x
add r5.xyz, r5.xyz, r0.xyz
mov r5.w, c[0].x


dp4 oPos.x, r5, c[92]
dp4 oPos.y, r5, c[93]
dp4 oPos.z, r5, c[94]
dp4 oPos.w, r5, c[95]


mov oD0, c[82]