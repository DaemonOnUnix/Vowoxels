#ifndef VOXELENGINE_VOXEL_H
#define VOXELENGINE_VOXEL_H

typedef char* voxel_side;
typedef unsigned voxel_coord;

enum voxel_mask {
    Zp = 1,
    Zm = 2,
    Xm = 4,
    Xp = 8,
    Ym = 16,
    Yp = 32
};

typedef struct {
    voxel_mask enabled_dirs;
    voxel_side textures[6];
    voxel_coord x;
    voxel_coord y;
    voxel_coord z;
} voxel;

#endif