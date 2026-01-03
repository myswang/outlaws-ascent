// internal
#include "tiny_ecs.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

// All we need to store besides the containers is the id of every entity and callbacks to be able to remove entities across containers
unsigned int Entity::id_count = 1;