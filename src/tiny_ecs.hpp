#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <typeindex>
#include <assert.h>
#include <iostream>

#include "../ext/nlohmann/json.hpp"
#include "components.hpp"
#include "common.hpp"
#include "entity.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

// Common interface to refer to all containers in the ECS registry
struct ContainerInterface
{
	virtual void clear() = 0;
	virtual size_t size() = 0;
	virtual void remove(Entity e) = 0;
	virtual bool has(Entity entity) = 0;
	virtual std::string getName() const = 0;
	virtual nlohmann::json toJSON() const = 0;
	virtual void fromJSON(const nlohmann::json& j) = 0;
	//virtual void serialize(Entity e, nlohmann::json& j) const = 0;
	//virtual void deserialize(Entity e, const json& j) = 0;
};

// A container that stores components of type 'Component' and associated entities
template <typename Component> // A component can be any class
class ComponentContainer : public ContainerInterface
{
private:
	// The hash map from Entity -> array index.
	std::unordered_map<unsigned int, unsigned int> map_entity_componentID; // the entity is cast to uint to be hashable.
	bool registered = false;
public:
	// Container of all components of type 'Component'
	std::vector<Component> components;

	// The corresponding entities
	std::vector<Entity> entities;

	// name of the Component container, "Not Saved" if they won't be saved
	std::string name = "Not Saved";

	// Constructor that registers the type
	ComponentContainer()
	{
	}

	// Inserting a component c associated to entity e
	inline Component& insert(Entity e, Component c, bool check_for_duplicates = true)
	{
		// Usually, every entity should only have one instance of each component type
		assert(!(check_for_duplicates && has(e)) && "Entity already contained in ECS registry");

		map_entity_componentID[e] = (unsigned int)components.size();
		components.push_back(std::move(c)); // the move enforces move instead of copy constructor
		entities.push_back(e);
		return components.back();
	};

	// The emplace function takes the the provided arguments Args, creates a new object of type Component, and inserts it into the ECS system
	template<typename... Args>
	Component& emplace(Entity e, Args &&... args) {
		return insert(e, Component(std::forward<Args>(args)...));
	};
	template<typename... Args>
	Component& emplace_with_duplicates(Entity e, Args &&... args) {
		return insert(e, Component(std::forward<Args>(args)...), false);
	};

	// A wrapper to return the component of an entity
	Component& get(Entity e) {
		assert(has(e) && "Entity not contained in ECS registry");
		return components[map_entity_componentID[e]];
	}

	// Check if entity has a component of type 'Component'
	bool has(Entity entity) {
		return map_entity_componentID.count(entity) > 0;
	}
		
	// Remove an component and pack the container to re-use the empty space
	void remove(Entity e)
	{
		if (has(e))
		{
			// Get the current position
			int cID = map_entity_componentID[e];

			// Move the last element to position cID using the move operator
			// Note, components[cID] = components.back() would trigger the copy instead of move operator
			components[cID] = std::move(components.back());
			entities[cID] = entities.back(); // the entity is only a single index, copy it.
			map_entity_componentID[entities.back()] = cID;

			// Erase the old component and free its memory
			map_entity_componentID.erase(e);
			components.pop_back();
			entities.pop_back();
			// Note, one could mark the id for re-use
		}
	};


	// Remove all components of type 'Component'
	void clear()
	{
		map_entity_componentID.clear();
		components.clear();
		entities.clear();
	}

	// Report the number of components of type 'Component'
	size_t size()
	{
		return components.size();
	}

	// get the name of the Component container, "Not saved" if they won't be saved
	std::string getName() const {
		return name;
	}

	nlohmann::json toJSON() const {
		if (name == "Not Saved") {
			return {};
		}

		nlohmann::json component_json = nlohmann::json::array();
		for (const auto& entity : entities) {
			nlohmann::json entity_json;
			serialize(entity, entity_json);
			component_json.push_back({ {"entity", static_cast<unsigned int>(entity)}, {"component", entity_json} });
		}
		return { {name, component_json} };
	}

	void fromJSON(const nlohmann::json& j) {
		for (auto& element : j) {
			Entity entity;
			unsigned int e_id = element.at("entity");
			// if saved_id maps to a created entity
			if (map_saved_entity_new_entity.count(e_id) > 0) {
				entity = map_saved_entity_new_entity.at(e_id);
			}
			else {
				entity = Entity();
				// map saved_id to new entity
				map_saved_entity_new_entity[e_id] = entity;
			}
			
			nlohmann::json component_data = element.at("component");
			deserialize(entity, component_data);
		}
	}

	void serialize(Entity e, nlohmann::json& j) const {
		// if has e in this container
		if (map_entity_componentID.count(e) > 0) {
			// get(e)
			const Component& component = components[map_entity_componentID.at(e)];
			if constexpr (std::is_same<Component, Mesh*>::value) {
				if (component) { // Ensure pointer is not null
					std::cout << "Explicitly calling toJSON for Mesh* component" << std::endl;
					::toJSON(j, static_cast<const Mesh*>(component));  // Explicit cast to Mesh*
				}
			}
			else if constexpr (std::is_pointer<Component>::value) {
//if constexpr (std::is_pointer<Component>::value) {

				// For pointer types, dereference before calling toJSON
				if (component) { // Ensure pointer is not null
					std::cout << "Calling toJSON for pointer type component" << std::endl;
					::toJSON(j, *component);
				}
				else {
					std::cout << "Component pointer is null" << std::endl;
				}
			}
			else {
				// For non-pointer types, directly call toJSON
				//std::cout << "Calling toJSON for non-pointer component" << std::endl;
                ::toJSON(j, component);
			}
		}
		else {
			std::cout << "Entity not found in container" << std::endl;
		}
	}

	void deserialize(Entity e, const nlohmann::json& j) {
		if constexpr (std::is_same<Component, Collision>::value) {
			// skip collision container
		}
		else if constexpr (std::is_same<Component, Mesh*>::value) {
			// Allocate a new Mesh object and deserialize
			Mesh* component = new Mesh();
			::fromJSON(j, component);  // Populate Mesh data from JSON
			insert(e, component);      // Insert Mesh* into container
		}
		else {
			Component component;
            ::fromJSON(j, component);
			insert(e, component);
		}
	}

	// Sort the components and associated entity assignment structures by the comparisonFunction, see std::sort
	template <class Compare>
	void sort(Compare comparisonFunction)
	{
		// First sort the entity list as desired
		std::sort(entities.begin(), entities.end(), comparisonFunction);
		// Now re-arrange the components (Note, creates a new vector, which may be slow! Not sure if in-place could be faster: https://stackoverflow.com/questions/63703637/how-to-efficiently-permute-an-array-in-place-using-stdswap)
		std::vector<Component> components_new; components_new.reserve(components.size());
		std::transform(entities.begin(), entities.end(), std::back_inserter(components_new), [&](Entity e) { return std::move(get(e)); }); // note, the get still uses the old hash map (on purpose!)
		components = std::move(components_new); // note, we use move operations to not create unneccesary copies of objects, but memory is still allocated for the new vector
		// Fill the new hashmap
		for (unsigned int i = 0; i < entities.size(); i++)
			map_entity_componentID[entities[i]] = i;
	}
};
