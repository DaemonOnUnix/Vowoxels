#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"
#include "voxelengine/data.h"
#include "metaprog/utils.h"
#include "linear_algebra/perlinnoise.h"

CREATE_COMMON(chunk_manager_pos);

Chunk_manager* initChunkManager(){
    EngineData* data = getEngineData();
    data->chunkM = malloc(sizeof(Chunk_manager));
    data->chunkM->actual_chunk_x = 0;
    data->chunkM->actual_chunk_y = 0;
    data->chunkM->actual_chunk_z = 0;
    data->chunkM->chunks = NULL;
    data->chunkM->need_update = 0;
    pthread_rwlock_init(&data->chunkM->chunkslock, NULL);
    // Create newx thread
    pthread_create(&data->chunkM->working_th, NULL, thread_loading_chunks, NULL);
    pthread_detach(data->chunkM->working_th);

    return data->chunkM;
}

#define abs(x) (((x) >= 0) ? (x) : (x)*(-1))

void removeChunks(){
    EngineData* data = getEngineData();
    int howmany = 0;
    int howmany2 = 0;
    
    // Remove chunks to far away from Camera
    struct chunk_list* prev = NULL;
    pthread_rwlock_rdlock(&data->chunkM->chunkslock);
    for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL;) {
        pthread_rwlock_unlock(&data->chunkM->chunkslock);
        //Distance chnk<>player_chnk > dist_affichage ?
        get_lock_chunk_manager_pos();
        if (
            abs(chnk->chunk->chunk_x - data->chunkM->actual_chunk_x) > VIEW_DIST+1 ||
            abs(chnk->chunk->chunk_z - data->chunkM->actual_chunk_z) > VIEW_DIST+1
            ) {
                set_lock_chunk_manager_pos();
                howmany++;
                
                // Remove from chunks list
                Chunk* chunk_to_free = deleteNextChunklist(prev);
                freeChunk(chunk_to_free);

                // Set chnk
                chnk = data->chunkM->chunks;
                if (prev)
                    chnk = prev->next;
                
        }else
        {
            set_lock_chunk_manager_pos();
            prev = chnk;
            chnk = chnk->next;
        }
        howmany2++;
        pthread_rwlock_rdlock(&data->chunkM->chunkslock);
    }
    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    LOG_INFO("%u freed on %u!", howmany, howmany2)
}
#include "collections/string.h"

void updateChunks(Vec3 camera_pos){
    EngineData* data = getEngineData();
    // Check if camera change chunk
    if(
        ((int64_t)camera_pos.x)/CHUNK_DIMENSION != data->chunkM->actual_chunk_x ||
        ((int64_t)camera_pos.y)/CHUNK_DIMENSION != data->chunkM->actual_chunk_y ||
        ((int64_t)camera_pos.z)/CHUNK_DIMENSION != data->chunkM->actual_chunk_z ){
        // ICI
        LOG_INFO("Chunks need update")
        data->chunkM->actual_chunk_x = ((int64_t)camera_pos.x)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_y = ((int64_t)camera_pos.y)/CHUNK_DIMENSION;
        data->chunkM->actual_chunk_z = ((int64_t)camera_pos.z)/CHUNK_DIMENSION;
        smartstr pogstr pikalul = string("", 40 );
        sprintf(pikalul, "x: %li; y: %li; z: %li", data->chunkM->actual_chunk_x, data->chunkM->actual_chunk_y, data->chunkM->actual_chunk_z);
        glfwSetWindowTitle(data->window, pikalul);
        if (!data->chunkM->need_update)
            removeChunks();
        data->chunkM->need_update = 1;
    }
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

    setSEED(rand());

    while (1)
    {
        // Check if update
        if(data->chunkM->need_update)
        {
            data->chunkM->need_update = 0;
            LOG_INFO("Chunks updating")
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
        pthread_rwlock_rdlock(&data->chunkM->chunkslock);
        for (struct chunk_list* chnk = data->chunkM->chunks; !data->chunkM->need_update && chnk != NULL; chnk = chnk->next) {
            if(chnk->chunk->chunk_x == x && chnk->chunk->chunk_y == y && chnk->chunk->chunk_z == z) {
                pthread_rwlock_unlock(&data->chunkM->chunkslock);
                updateCoord(&dir, &x, &y, &z, camx, camy, camz);
                pthread_rwlock_rdlock(&data->chunkM->chunkslock);
            }
        }
        pthread_rwlock_unlock(&data->chunkM->chunkslock);

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
                    int lx = _x;
                    int lz = _z;
                    float p = perlin2d((float)x*CHUNK_DIMENSION + lx,(float)z*CHUNK_DIMENSION + lz, 0.01, 3);
                    if(p>1.0f)
                        p = 1.0f;
                    float h = floorf(mapfloat(p, 0.0f, 1.0f, 0.0f, MAX_HEIGHT));
                    for (size_t _y = 0; _y < CHUNK_DIMENSION && ((y*CHUNK_DIMENSION)+_y < h); _y++)
                    {
                        chunk->voxel_list[INDEX_TO_CHUNK(_x, _y, _z)] = 1;
                    }
                    
                }
                
            }
        }

        // Create mesh
        updateChunkVertex(chunk);

        // Add chunk to render
        insertChunkToChunklist(chunk);

        updateCoord(&dir, &x, &y, &z, camx, camy, camz);
    }

    pthread_exit(NULL);
}

