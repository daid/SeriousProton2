#ifndef SP2_CONTAINER_INFINIGRID_H
#define SP2_CONTAINER_INFINIGRID_H

#include <unordered_map>
#include <sp2/nonCopyable.h>
#include <sp2/math/vector2.h>


namespace sp {

template<typename T, size_t CHUNK_SIZE_SHIFT=6> class InfiniGrid : NonCopyable {
public:
    InfiniGrid(const T& _default_value)
    : default_value(_default_value) {}

    void set(Vector2i position, const T& value)
    {
        auto it = chunks.find({position.x >> CHUNK_SIZE_SHIFT, position.y >> CHUNK_SIZE_SHIFT});
        if (it == chunks.end())
        {
            auto& data = chunks[{position.x >> CHUNK_SIZE_SHIFT, position.y >> CHUNK_SIZE_SHIFT}].data;
            for(int x=0; x<chunk_size; x++)
                for(int y=0; y<chunk_size; y++)
                    data[x][y] = default_value;
            data[position.x & position_mask][position.y & position_mask] = value;
        } else {
            auto& data = it->second.data;
            data[position.x & position_mask][position.y & position_mask] = value;
        }
    }

    void clear()
    {
        chunks.clear();
    }

    void clear(Vector2i position)
    {
        auto it = chunks.find({position.x >> CHUNK_SIZE_SHIFT, position.y >> CHUNK_SIZE_SHIFT});
        if (it == chunks.end())
            return;
        auto& data = it->second.data;
        data[position.x & position_mask][position.y & position_mask] = default_value;
        for(int x=0; x<chunk_size; x++)
            for(int y=0; y<chunk_size; y++)
                if (data[x][y] != default_value)
                    return;
        chunks.erase(it);
    }

    const T& get(Vector2i position)
    {
        auto it = chunks.find({position.x >> CHUNK_SIZE_SHIFT, position.y >> CHUNK_SIZE_SHIFT});
        if (it == chunks.end())
            return default_value;
        return it->second.data[position.x & position_mask][position.y & position_mask];
    }

    size_t chunkCount() {
        return chunks.size();
    }

    static constexpr int chunk_size = 1 << CHUNK_SIZE_SHIFT;
    static constexpr int position_mask = chunk_size - 1;
    struct Chunk {
        T data[chunk_size][chunk_size];
    };

    class Iterator {
    public:
        bool operator!=(const Iterator& other) { return chunk_it != other.chunk_it; }
        void operator++()
        {
            index.x++;
            if (index.x == chunk_size) {
                index.x = 0;
                index.y++;
                if (index.y == chunk_size) {
                    index.y = 0;
                    chunk_it++;
                }
            }
        }

        struct Data {
            Vector2i position;
            const T& data;
        };
        Data operator*()
        {
            return {
                {(chunk_it->first.x << CHUNK_SIZE_SHIFT) + index.x, (chunk_it->first.y << CHUNK_SIZE_SHIFT) + index.y},
                chunk_it->second.data[index.x][index.y]
            };
        }
    private:
        Iterator(typename std::unordered_map<Vector2i, Chunk>::iterator chunk_it)
        : index(0, 0), chunk_it(chunk_it) {}
        sp::Vector2i index;
        typename std::unordered_map<Vector2i, Chunk>::iterator chunk_it;

        friend class InfiniGrid;
    };

    Iterator begin() {
        return Iterator(chunks.begin());
    }

    Iterator end() {
        return Iterator(chunks.end());
    }

private:
    T default_value;
    std::unordered_map<Vector2i, Chunk> chunks;
};

}

#endif//SP2_CONTAINER_INFINIGRID_H
