# Ship

Final project of the course Advanced Topics in Programing Tel Aviv University.

In this exercise you would be required to manage a “Ship” class with “Containers”, based on the following requirements:
A “Container” is an unknown type (modeled as a Template parameter)
The “Ship” has dimensions X * Y * Height for holding containers
All containers are of same size and dimensions
Certain points on the ship may have specific restrictions stating smaller height than the max Height. In case there are no such restrictions the total containers capacity of the ship is X * Y * Height 
You are required to implement the exact API described below


Ship’s API
All classes and types would be inside the namespace: shipping
Entire implementation shall be inside Ship.h -- there is NO Ship.cpp as this is a template

Special Global Types:
X, Y, Height
each one of the above three types is constructed explicitly by int and has a casting to int

Ship’s template parameter:
Ship would be a templated class, with a template parameter: typename Container

type:
template<typename Container>
using Grouping = std::unordered_map<string, std::function<string(const Container&)>>;

above type defines a groping map, with:
name of the grouping: std::string, as the key
a grouping function that gets a const Container& and returns its group name as a string, for this grouping function, as the value
Groupings can be according to: destination port, container’s owner etc.

Ship’s Constructor (1):
Ship(X x, Y y, Height max_height,
		std::vector<std::tuple<X, Y, Height>> restrictions,
Grouping<Container> groupingFunctions) noexcept(false);
Ship’s Constructor (2):
Ship(X x, Y y, Height max_height,
std::vector<std::tuple<X, Y, Height>> restrictions) noexcept(false);
This constructor is useful if there are no groupings.
both methods above may throw BadShipOperationException
(see details of this Exception class below).

Ship’s Constructor (3):
Ship(X x, Y y, Height max_height) noexcept;
This constructor is useful if there are no restrictions and no groupings.

So to create a ship, one can do for example:
	Ship<int> myShip{X{5}, Y{12}, Height{8}};
Above creates a ship with containers of type int.

loading a container:
void load(X x, Y y, Container c) noexcept(false);
the method may throw BadShipOperationException.

unloading a container:
Container unload(X x, Y y) noexcept(false);
the method may throw BadShipOperationException.

moving a container from one location to another on the ship:
void move(X from_x, Y from_y, X to_x, Y to_y) noexcept(false);
the method may throw BadShipOperationException.

iterators begin and end:
The ship would only have a const version begin and end iterators for iterating over all containers on the ship. There is no defined order. Iteration shall not create a copy of the containers but rather run on the original containers on ship.

getContainersView:
The ship would have the following methods to obtain a “view” of the containers.
The return value of those functions is explained below.
getContainersViewByPosition(X x, Y y) const;
getContainersViewByGroup(const string& groupingName, const string& groupName) const;

functions would not throw an exception, but may return an empty view.
The view functions would return something of your choice which has iterators begin and end to allow traversal on the view.
The view would never be a copy of the containers. If the user calls one of these functions and holds the result, then loads, unloads or moves a container, then runs on the view - the run on the view would be on the new data. On the other hand, the view doesn’t have to support traversing on the view, stopping, then loading, unloading or moving a container, then continuing the traversal - such operation is not defined, i.e. load/unload/move operations may invalidate the iterators of a view.
After a full cycle over the view you cannot traverse over it again, but you can retrieve the same view again with the proper getContainersView function.
The order for running on the view:
getContainersViewByPosition - from the highest container and downwards
getContainersViewByGroup - order is not important
*iterator provided by the view would be:
getContainersViewByPosition - const Container&
getContainersViewByGroup - std::pair<tuple {X, Y, Height}, const Container&>

BadShipOperationException 
has the following ctor: BadShipOperationException(string msg);
the message is yours, we will not check it, use it as you find suitable

