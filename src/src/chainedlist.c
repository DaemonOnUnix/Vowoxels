#include "collections/string.h"
#include "voxelengine/chunk.h"
#include "voxelengine/data.h"

void insertChunkToChunklist(Chunk *chunk){
    EngineData* data = getEngineData();

    // Malloc new node
    struct chunk_list* new_chunk_list = malloc(sizeof(struct chunk_list));
    new_chunk_list->chunk = chunk;
    new_chunk_list->next = NULL;

    // Add to the start of the list
    pthread_rwlock_wrlock(&data->chunkM->chunkslock);
    if(data->chunkM->chunks){
        new_chunk_list->next = data->chunkM->chunks;
    }
    data->chunkM->chunks = new_chunk_list;
    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    return;
}

Chunk* deleteNextChunklist(struct chunk_list* ch_list_prev){
    EngineData* data = getEngineData();
    struct chunk_list* deleted_chunk_list = NULL;
    Chunk* chunk_to_return = NULL;
    pthread_rwlock_wrlock(&data->chunkM->chunkslock);

    // Take deleted chunk list
    if(!ch_list_prev){
        if(!data->chunkM->chunks)
            return NULL;
        // Delete the first one
        deleted_chunk_list = data->chunkM->chunks;
        // Skip deleted value
        data->chunkM->chunks = data->chunkM->chunks->next;
    }else
    {
        // Delete the next to ch_list_prev
        deleted_chunk_list = ch_list_prev->next;
        // Skip deleted value
        ch_list_prev->next = ch_list_prev->next->next;
    }

    // Keep his chunk
    chunk_to_return = deleted_chunk_list->chunk;   

    // Destroy link to the list
    deleted_chunk_list->next = NULL;
    free(deleted_chunk_list);

    pthread_rwlock_unlock(&data->chunkM->chunkslock);
    return chunk_to_return;
}

Chunk* getChunk(int32_t x, int32_t y, int32_t z){
    EngineData* data = getEngineData();
    Chunk* chunk_to_return = NULL;


    pthread_rwlock_rdlock(&data->chunkM->chunkslock);
    for (struct chunk_list* chnk = data->chunkM->chunks; chnk != NULL && chunk_to_return == NULL; chnk = chnk->next) {
        if (x == chnk->chunk->chunk_x && y == chnk->chunk->chunk_y && z == chnk->chunk->chunk_z)
        {
            chunk_to_return = chnk->chunk;
        }
        
    }
    pthread_rwlock_unlock(&data->chunkM->chunkslock);

    return chunk_to_return;
}