#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"
#include "voxelengine/data.h"
#include "metaprog/utils.h"
#include "linear_algebra/perlinnoise.h"

CREATE_COMMON(chunk_manager_pos);

Chunk_manager* initChunkManager(){
    EngineData* data = getEngineData();
    data->chunkM = malloc(sizeof(Chunk_manager));
    data->chunkM->chunks = NULL;
    data->chunkM->actual_chunk_x = 0;
    data->chunkM->actual_chunk_y = 0;
    data->chunkM->actual_chunk_z = 0;
    data->chunkM->chunks_toFree = NULL;
    // Create newx thread
    pthread_create(&data->chunkM->working_th, NULL, thread_loading_chunks, NULL);
    pthread_detach(data->chunkM->working_th);

    return data->chunkM;
}

#define abs(x) (((x) >= 0) ? (x) : (x)*(-1))

void toFreeChunk(){
    if(data->chunkM->chunks_toFree){
        struct chunk_list* chnk = data->chunkM->chunks_toFree;
        while (chnk->next != NULL)
        {
            freeChunk(chnk->chunk);
            struct chunk_list* temp = chnk;
            chnk = chnk->next;
            free(temp);
        }
        data->chunkM->chunks_toFree = NULL;
    }
}

void removeChunks(){
    EngineData* data = getEngineData();
    struct chunk_list* prev = NULL;
    struct chunk_list* next = NULL;
    struct chunk_list* tofree = data->chunkM->chunks_toFree;
    for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL; chnk = next) {
        //Distance chnk<>player_chnk > dist_affichage ?
        get_lock_chunk_manager_pos();
        if (
            abs(chnk->chunk->chunk_x - data->chunkM->actual_chunk_x) > VIEW_DIST || 
            abs(chnk->chunk->chunk_y - data->chunkM->actual_chunk_y) > VIEW_DIST ||
            abs(chnk->chunk->chunk_z - data->chunkM->actual_chunk_z) > VIEW_DIST
            ) {
                set_lock_chunk_manager_pos();
                //B E  G O N E
                if(prev)
                    prev->next = chnk->next;
                else
                    data->chunkM->chunks = chnk->next;
                next = chnk->next;
                if(!tofree){
                    data->chunkM->chunks_toFree = chnk;
                    tofree = chnk;
                }else{
                    tofree->next = chnk;
                    tofree = tofree->next;
                }
                tofree->next = NULL;
        }
        else {
            set_lock_chunk_manager_pos();
            prev = chnk;
            next = chnk->next;
        }
    }
}

void updateChunks(Vec3 camera_pos){
    EngineData* data = getEngineData();
    // Check if camera change chunk
    if(
        ((int)camera_pos.x)/CHUNK_DIMENSION != data->chunkM->actual_chunk_x ||
        ((int)camera_pos.y)/CHUNK_DIMENSION != data->chunkM->actual_chunk_y ||
        ((int)camera_pos.z)/CHUNK_DIMENSION != data->chunkM->actual_chunk_z ){
        // ICI
        LOG_INFO("Chunks need update")
        data->chunkM->actual_chunk_x = ((int)camera_pos.x)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_y = ((int)camera_pos.y)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_z = ((int)camera_pos.z)/CHUNK_DIMENSION;
        toFreeChunk();
        data->chunkM->need_update = 1;
    }
}


void updateCoord(int* dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz){
    UNUSED(camy);
    if(*y<0){
        LOG_INFO("UNDER THE MAP")
        *dir = 5;
    }
    switch (*dir)
    {
    case 0:
        (*x)++;
        (*dir)++;
        break;
    case 1:
        (*z)++;
        if (*z - camz == *x - camx)
            (*dir)++;
        break;
    case 2:
        (*x)--;
        if (*x - camx == -(*z - camz))
            (*dir)++;
        break;
    case 3:
        (*z)--;
        if (*x - camx == *z - camz)
            (*dir)++;
        break;
    case 4:
        (*x)++;
        if (*x - camx == -(*z - camz)){
            if(VIEW_DIST > *x - camx)
                (*dir)=0;
            else
                (*dir)++;
        }
        break;
    case 5:
        LOG_INFO("Finish to load all CHUNKS")
        while(!__sync_bool_compare_and_swap(&(data->chunkM->need_update), 1, 2));
        __sync_synchronize();
        break;
    default:
        PANIC("How did you get there ?");
        break;
    }
    LOG_INFO("Updating Coord x: %i; y: %i; z: %i", *x, *y, *z)
}

void* thread_loading_chunks(void *args){
    EngineData* data = getEngineData();
    UNUSED(args);

    int dir = 0;
    int32_t x = data->chunkM->actual_chunk_x;
    int32_t y = data->chunkM->actual_chunk_y;
    int32_t z = data->chunkM->actual_chunk_z;
    
    get_lock_chunk_manager_pos();
    int32_t camx = data->chunkM->actual_chunk_x;
    int32_t camy = data->chunkM->actual_chunk_y;
    int32_t camz = data->chunkM->actual_chunk_z;
    set_lock_chunk_manager_pos();

    while (1)
    {
        // Check if update
        if(data->chunkM->need_update)
        {
            data->chunkM->need_update = 0;
            LOG_INFO("Chunks updating")
            // Remove far chunks
            removeChunks();

            // Reset var
            dir = 0;
            x = data->chunkM->actual_chunk_x;
            y = data->chunkM->actual_chunk_y;
            z = data->chunkM->actual_chunk_z;

            get_lock_chunk_manager_pos();    
            camx = data->chunkM->actual_chunk_x;
            camy = data->chunkM->actual_chunk_y;
            camz = data->chunkM->actual_chunk_z;
            set_lock_chunk_manager_pos();

        }
        // Shearch next chunk to load
        for (struct chunk_list* chnk = data->chunkM->chunks; !data->chunkM->need_update && chnk != NULL; chnk = chnk->next) {
            if(chnk->chunk->chunk_x == x && chnk->chunk->chunk_y == y && chnk->chunk->chunk_z == z) {
                updateCoord(&dir, &x, &y, &z, camx, camy, camz);
            }
        }

        if (data->chunkM->need_update){
            
            continue;
        }
        
        // Try to load the chunk
        Chunk* chunk = loadChunkFromFile("", x, y, z);
        
        // Failed to load existing chunk
        if(!chunk){
            // TODO: Generate Chunk
            // TEST CHUNK
            chunk = newChunk(x, y, z);
            for (size_t _x = 0; _x < CHUNK_DIMENSION; _x++)
            {
                for (size_t _z = 0; _z < CHUNK_DIMENSION; _z++)
                {
                    float p = perlin2d((float)x*CHUNK_DIMENSION + _x,(float)z*CHUNK_DIMENSION + _z, 0.01, 4);
                    
                    float h = mapfloat(p, 0.0f, 1.0f, 0.0f, MAX_HEIGHT);
                    for (size_t _y = 0; _y < CHUNK_DIMENSION && y*CHUNK_DIMENSION+_y < h; _y++)
                    {
                        chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 1;
                    }
                    
                }
                
            }
        }

        // Create mesh
        updateChunkVertex(chunk);

        // Adding the new chunk to the linked list
        struct chunk_list* chunktmp = data->chunkM->chunks;
        struct chunk_list* chunkst = malloc((sizeof(struct chunk_list)));
        chunkst->next = chunktmp;
        chunkst->chunk = chunk;
        data->chunkM->chunks = chunkst;
        updateCoord(&dir, &x, &y, &z, camx, camy, camz);
    }

    pthread_exit(NULL);
}

