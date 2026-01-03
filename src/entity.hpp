
#ifndef OUTLAWS_ASCENT_ENTITY_HPP
#define OUTLAWS_ASCENT_ENTITY_HPP

#include "unordered_map"


class Entity
{
    unsigned int id;
    static unsigned int id_count; // starts from 1, entit 0 is the default initialization
public:
    Entity()
    {
        id = id_count++;
        // Note, indices of already deleted entities arent re-used in this simple implementation.
    }
    // changed this to const for save-load
    operator unsigned int() const { return id; } // this enables automatic casting to int
};


#endif //OUTLAWS_ASCENT_ENTITY_HPP
