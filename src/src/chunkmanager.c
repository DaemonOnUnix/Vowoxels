#include "voxelengine/chunkmanager.h"
#include "pthread/locks.h"
#include "voxelengine/data.h"
#include "metaprog/utils.h"
#include "collections/string.h"
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

void updateChunks(Vec3 camera_pos){
    EngineData* data = getEngineData();
    // Check if camera change chunk
    if(
        (int32_t)floorf(camera_pos.x/CHUNK_DIMENSION) != data->chunkM->actual_chunk_x ||
        (int32_t)floorf(camera_pos.y/CHUNK_DIMENSION) != data->chunkM->actual_chunk_y ||
        (int32_t)floorf(camera_pos.z/CHUNK_DIMENSION) != data->chunkM->actual_chunk_z ){
        // ICI
        LOG_INFO("Chunks need update")
        data->chunkM->actual_chunk_x = (int32_t)floorf(camera_pos.x/CHUNK_DIMENSION);
        data->chunkM->actual_chunk_y = (int32_t)floorf(camera_pos.y/CHUNK_DIMENSION);
        data->chunkM->actual_chunk_z = (int32_t)floorf(camera_pos.z/CHUNK_DIMENSION);
        smartstr pogstr pikalul = string("", 40 );
        int64_t nbchunks = 0;

        // DEBUG
        pthread_rwlock_rdlock(&data->chunkM->chunkslock);
        for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL; chnk = chnk->next) {
            nbchunks++;
        }
        pthread_rwlock_unlock(&data->chunkM->chunkslock);

        sprintf(pikalul, "x: %li; y: %li; z: %li   Chunks: %li", data->chunkM->actual_chunk_x, data->chunkM->actual_chunk_y, data->chunkM->actual_chunk_z, nbchunks);
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

    srand( time( NULL ) );
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
                chnk = data->chunkM->chunks;
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
            chunk = generateChunk(x, y, z);
        }

        // Create mesh
        updateChunkVertex(chunk);

        // Add chunk to render
        insertChunkToChunklist(chunk);

        updateCoord(&dir, &x, &y, &z, camx, camy, camz);
    }

    pthread_exit(NULL);
}


void place_voxel_to_hit(RaycastHit hit, uint32_t voxel_id){
    Chunk *chunk = (Chunk *)hit.object;

    if (!chunk)
    {
        LOG_ERR("Can't place voxel x: %f y: %f z: %f", hit.point.x, hit.point.y, hit.point.z);
        return;
    }
    
    Vec3 coord = vec3_sub(hit.point, vec3_mul_val(vec3$(chunk->chunk_x, chunk->chunk_y, chunk->chunk_z), CHUNK_DIMENSION));
    chunk->voxel_list[ INDEX_TO_CHUNK((int32_t)coord.x, (int32_t)coord.y, (int32_t)coord.z)] = voxel_id;
    updateChunkVertex(chunk);
    updateChunk(chunk);
    Chunk *chunknext;
    if(coord.x == 0){
        chunknext = getChunk(chunk->chunk_x-1, chunk->chunk_y, chunk->chunk_z);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
    if(coord.x == CHUNK_DIMENSION-1){
        chunknext = getChunk(chunk->chunk_x+1, chunk->chunk_y, chunk->chunk_z);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
    if(coord.y == 0){
        chunknext = getChunk(chunk->chunk_x, chunk->chunk_y-1, chunk->chunk_z);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
    if(coord.y == CHUNK_DIMENSION-1){
        chunk = getChunk(chunk->chunk_x, chunk->chunk_y+1, chunk->chunk_z);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
    if(coord.z == 0){
        chunknext = getChunk(chunk->chunk_x, chunk->chunk_y, chunk->chunk_z-1);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
    if(coord.z == CHUNK_DIMENSION-1){
        chunknext = getChunk(chunk->chunk_x, chunk->chunk_y, chunk->chunk_z+1);
        if (chunknext){
            updateChunkVertex(chunknext);
            updateChunk(chunknext);
        }
    }
}
void place_voxel(int32_t voxel_x, int32_t voxel_y, int32_t voxel_z, uint32_t voxel_id){
    Chunk *chunk = getChunk(voxel_x/CHUNK_DIMENSION, voxel_y/CHUNK_DIMENSION, voxel_z/CHUNK_DIMENSION);

    if (!chunk)
    {
        LOG_ERR("Can't place voxel x: %i y: %i z: %i", voxel_x, voxel_y, voxel_z);
        return;
    }
    

    chunk->voxel_list[ INDEX_TO_CHUNK(voxel_x % CHUNK_DIMENSION, voxel_y % CHUNK_DIMENSION, voxel_z % CHUNK_DIMENSION)] = voxel_id;
    updateChunkVertex(chunk);
    updateChunk(chunk);
}