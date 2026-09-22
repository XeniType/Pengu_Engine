/**
 * @file TagComponent.hpp
 * @brief Component for tagging entities with a name.
 */

#ifndef TAGCOMPONENT_HPP
#define TAGCOMPONENT_HPP 1
#include <string>

/**
 * @struct TagComponent
 * @brief Component that provides a string tag to an entity.
 */
struct TagComponent {

	/** @brief The tag string. */
	std::string tag;
	/**
	 * @brief Default constructor.
	 */
	TagComponent() = default;
	/**
	 * @brief Constructor with tag string.
	 * @param t The tag string.
	 */
	TagComponent(const std::string& t) : tag(t) {};
};

#endif // !TAGCOMPONENT_HPP
