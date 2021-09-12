#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"
#include "voxelengine/data.h"
#include "metaprog/utils.h"

CREATE_COMMON(chunk_manager_pos);


Chunk_manager* initChunkManager(){
    EngineData* data = getEngineData();
    data->chunkM = malloc(sizeof(Chunk_manager));
    data->chunkM->chunks = NULL;
    data->chunkM->actual_chunk_x = 0;
    data->chunkM->actual_chunk_y = 0;
    data->chunkM->actual_chunk_z = 0;
    // Create newx thread
    pthread_create(&data->chunkM->working_th, NULL, thread_loading_chunks, NULL);
    pthread_detach(data->chunkM->working_th);
    return data->chunkM;
}

#define abs(x) (((x) >= 0) ? (x) : (x)*(-1))

void removeChunks(){
    EngineData* data = getEngineData();
    struct chunk_list* prev = NULL;
    struct chunk_list* next = NULL;
    for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL; chnk = next) {
        //Distance chnk<>player_chnk > dist_affichage ?
        get_lock_chunk_manager_pos();
        if (
            abs(chnk->chunk_x - data->chunkM->actual_chunk_x) > VIEW_DIST || 
            abs(chnk->chunk_y - data->chunkM->actual_chunk_y) > VIEW_DIST ||
            abs(chnk->chunk_z - data->chunkM->actual_chunk_z) > VIEW_DIST
            ) {
                set_lock_chunk_manager_pos();
                //B E  G O N E
                prev->next = chnk->next;
                next = chnk->next;
                free(chnk);
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
        data->chunkM->need_update = 1;
    }
}


void updateCoord(int* dir, int32_t* x, int32_t* y, int32_t* z, int32_t camx, int32_t camy, int32_t camz){
    switch (*dir)
    {
    case 0:
        (*x)++;
        if (*x >= camx+(VIEW_DIST/2))
            (*dir)++;
        break;
    case 1:
        (*y)++;
        if (*y >= camy+(VIEW_DIST/2))
            (*dir)++;
        break;
    case 2:
        (*z)++;
        if (*z >= camz+(VIEW_DIST/2))
            (*dir)++;
        break;
    case 3:
        (*x)--;
        if (*x <= camx-(VIEW_DIST/2))
            (*dir)++;
        break;
    case 4:
        (*y)--;
        if (*y <= camy-(VIEW_DIST/2))
            (*dir)++;
        break;
    case 5:
        (*z)--;
        if (*z <= camz-(VIEW_DIST/2))
            (*dir)++;
        break;
    case 6:
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
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    
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
            x = 0;
            y = 0;
            z = 0;

            get_lock_chunk_manager_pos();    
            camx = data->chunkM->actual_chunk_x;
            camy = data->chunkM->actual_chunk_y;
            camz = data->chunkM->actual_chunk_z;
            set_lock_chunk_manager_pos();

        }
        // Shearch next chunk to load
        unsigned char already_exist = 0;
        for (struct chunk_list* chnk = data->chunkM->chunks; !data->chunkM->need_update && chnk != NULL; chnk = chnk->next) {
            if(chnk->chunk_x == camx+x && chnk->chunk_y == camy+y && chnk->chunk_z == camz+z) {
                updateCoord(&dir, &x, &y, &z, camx, camy, camz);
                already_exist++;
            }
        }

        // If don't need to load chunk
        if (data->chunkM->need_update || already_exist){
            
            continue;
        }
        
        // Try to load the chunk
        Chunk* chunk = loadChunkFromFile("", camx+x, camy+y, camz+z);
        
        // Failed to load existing chunk
        if(!chunk){
            // TODO: Generate Chunk
            // TEST CHUNK
            chunk = newChunk();
            for (size_t x = 0; x < CHUNK_DIMENSION; x++)
            {
                for (size_t y = 0; y < CHUNK_DIMENSION; y++)
                {
                    for (size_t z = 0; z < CHUNK_DIMENSION; z++)
                    {
                        if (y <= 0){
                            chunk->voxel_list[INDEX_TO_CHUNK(x, y, z)] = 1;
                        }
                    }
                    
                }
                
            }
            LOG_OK("Create chunk %i %i %i", camx+x, camy+y, camz+z)
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
