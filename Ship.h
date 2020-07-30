//=====================================
// @author: Asaf Rozin 
//=====================================

//-------------------------------------
// Ship.h
//-----

#pragma once
#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <string>
#include <optional>
#include <iostream>
#include <deque>

namespace shipping {
	template<typename T> class NamedType {
		T t;
	public:
		explicit NamedType(T t) : t(t) {}
		operator T() const {
			return t;
		}
	};

	struct X : NamedType<int> {
		using NamedType<int>::NamedType;
	};

	struct Y : NamedType<int> {
		using NamedType<int>::NamedType;
	};

	struct Height : NamedType<int> {
		using NamedType<int>::NamedType;
	};

	using Position = std::tuple<shipping::X, shipping::Y, shipping::Height>;
}

// pause from shipping to define hash for Position
namespace std
{
	template<> struct hash<shipping::Position>
	{
		std::size_t operator()(const shipping::Position& pos) const noexcept
		{
			return std::get<0>(pos) ^ ((std::get<1>(pos) << 1) ^ (std::get<2>(pos) << 2));
		}
	};
}
//back to shipping

namespace shipping {
	struct BadShipOperationException {
		BadShipOperationException([[maybe_unused]] const std::string& msg) {}
	};

	template<typename Container>
	using Grouping = std::unordered_map<std::string, std::function<std::string(const Container&)>>;

	template<typename Container>
	class Ship {
		class GroupView {
			const std::unordered_map<Position, const Container&>* p_group = nullptr;
			using iterator_type = typename std::unordered_map<Position, const Container&>::const_iterator;
			int beginFlag = 0;
		public:
			GroupView(const std::unordered_map<Position, const Container&>& group) : p_group(&group) {}
			GroupView(int) {}
			auto begin() {
				if (p_group && !beginFlag) {
					beginFlag = 1;
					return p_group->cbegin();
				}
				else return iterator_type{};
			}
			auto end() const {
				return p_group ? p_group->cend() : iterator_type{};
			}
		};
		class PosView {
			const std::deque< Container>* p_pos = nullptr;
			using iterator_type = typename std::deque<Container>::const_iterator;
			int beginFlag = 0;
		public:
			PosView(const std::deque<Container>& conts) : p_pos(&conts) {}
			PosView(int) {}
			auto begin() {
				if (!beginFlag) {
					beginFlag = 1;
					return p_pos ? p_pos->cbegin() : iterator_type{};
				}
				else
					return  p_pos->cend();

			}
			auto end() const {
				return p_pos ? p_pos->cend() : iterator_type{};
			}
		};
		class iterator {
			friend class Ship;
			typename std::vector<std::deque<Container>>::const_iterator positionsIteratorEnd;
			typename std::vector<std::deque<Container>>::const_iterator  positionsIterator;
			typename std::deque<Container>::const_iterator containersIterator;
			iterator(Ship& ship)
				: positionsIteratorEnd(ship.containers.end()), positionsIterator(ship.containers.begin()), containersIterator(positionsIterator->cbegin()) {
				moveToNonEmptyBucket();
			}
			iterator(Ship& s, int)
				: positionsIteratorEnd(s.containers.end()), positionsIterator(s.containers.end()), containersIterator{} {}
			void moveToNonEmptyBucket() {
				while (positionsIterator != positionsIteratorEnd && positionsIterator->empty()) {
					++positionsIterator;
				}
				if (positionsIterator != positionsIteratorEnd) {
					containersIterator = positionsIterator->cbegin();
				}
				else {
					containersIterator = {};
				}
			}
		public:
			bool operator!=(iterator other) const {
				return positionsIterator != other.positionsIterator || containersIterator != other.containersIterator;
			}
			iterator& operator++() {
				++containersIterator;
				if (containersIterator == positionsIterator->cend()) {
					++positionsIterator;
					moveToNonEmptyBucket();
				}
				return *this;
			}
			const Container& operator*() {
				return *containersIterator;
			}
		};
		/*
		 * Ship is a 2D vector (flat) - which has a deque of containers at each posIndex(x,y)
		 */
		std::vector<std::deque<Container>> containers;
		// maximum hight of every index. 
		// before setting restrictions all are the same (maximum hight of ship)
		std::vector<Height> maxHight;
		int xSize, ySize;
		std::vector<std::tuple<X, Y, Height>> restrictions;
		// grouping:
		Grouping<Container> groupingFunctions;
		using Pos2Container = std::unordered_map<Position, const Container&>;
		using Group = std::unordered_map<std::string, Pos2Container>;
		mutable std::unordered_map<std::string, Group> groups;

		// private:
		int posIndex(X x, Y y) const {
			if (validIndex(x, y)) {
				return y * xSize + x;
			}
			throw BadShipOperationException("index out of range");
		}

		int empty(X x, Y y) const {
			auto& space = containers[posIndex(x, y)];
			return ((int)space.size() == 0);
		}

		int full(X x, Y y) const {
			auto& space = containers[posIndex(x, y)];
			return ((int)space.size() >= maxHight[posIndex(x, y)]);
		}

		int validIndex(X x, Y y) const {
			return (x >= 0 && x < xSize&& y >= 0 && y < ySize);
		}

		Container& getContainer(X x, Y y, Height h) {
			return containers[posIndex(x, y)][(int)h];
		}

		void addContainerToGroups(X x, Y y, Height h, Container& e) {
			for (auto& groupPair : groupingFunctions) {
				groups[groupPair.first][groupPair.second(e)].insert({ std::tuple{x, y, h}, e });
			}
		}

		void removeContainerFromGroups(X x, Y y, Height h, Container& e) {
			for (auto& groupPair : groupingFunctions) {
				groups[groupPair.first][groupPair.second(e)].erase(std::tuple{ x, y, h });
			}
		}

		void setContainers(X x, Y y, Height h) {
			containers.resize(x * y);
			for (int i = 0; i < (int)containers.size(); i++)
				// before restrictions - every index has same max hight
				// essential for check duplicate restriction
				maxHight.push_back(h);
		}

		void setRestrictions(Height h) {
			int x,y,r;
			for (int i = 0; i < (int)restrictions.size(); i++) {
				x = std::get<0>(restrictions.at(i));
				y = std::get<1>(restrictions.at(i));
				r = std::get<2>(restrictions.at(i)); // restriction hight
				// check restriction hight is legal 
				// if it is - change maximum hight at the restriction's index
				if (Height{r} >= h)
					throw BadShipOperationException("restriction is greate ar equal then ship's height");
				if (maxHight[posIndex(X{x},Y{y})] != h) 
				// hight already changed at this index
					throw BadShipOperationException("duplicate restriction");
				maxHight[posIndex(X{x},Y{y})] = Height{r};
			}
		}

	public:
		Ship(X x, Y y, Height h, std::vector<std::tuple<X, Y, Height>> restrictions, Grouping<Container> groupingFunctions) noexcept (false)
			: xSize(x), ySize(y), restrictions(restrictions), groupingFunctions(groupingFunctions) {
			setContainers(x, y, h);
			setRestrictions(h);
		}
		Ship(X x, Y y, Height h, std::vector<std::tuple<X, Y, Height>> restrictions) noexcept (false) :Ship(x, y, h, restrictions, {}) {}
		Ship(X x, Y y, Height h) noexcept (false) : Ship(x, y, h, {}, {}) {}

		void load(X x, Y y, Container c) noexcept (false) {
			auto& space = containers[posIndex(x, y)];
			if (!validIndex(x, y) || full(x, y)) {
				throw BadShipOperationException("occupied or illegal space");
			}
			space.push_front(c);
			addContainerToGroups(x, y, Height{ (int)space.size() - 1 }, space.front());
		}

		Container unload(X x, Y y) noexcept(false) {
			auto& space = containers[posIndex(x, y)];
			if (!validIndex(x, y) || empty(x, y)) {
				throw BadShipOperationException("no container to unload");
			}
			auto container = space.front();
			removeContainerFromGroups(x, y, Height{ (int)space.size() - 1 }, container);
			space.pop_front();
			return container;
		}

		void move(X from_x, Y from_y, X to_x, Y to_y) noexcept(false) {
			//if source index = destenaion index - nothing to do unless position is empty
			if (!(to_x == from_x && to_y == from_y)) {
				if (!validIndex(to_x, to_y) || !validIndex(from_x, from_y)  || full(to_x, to_y) || empty(from_x, from_y)) {
					//if destenation/source index is full/empty or not exists - throw exception
					throw BadShipOperationException("occupied or illegal space");
				}
				load(to_x, to_y, unload(from_x, from_y));
			}
			else if (empty(from_x, from_y))
				throw BadShipOperationException("no container to unload");
			// else - nothing to do
		}

		PosView getContainersViewByPosition(X x, Y y) const {
			if (validIndex(x, y)) {
				return { containers[posIndex(x, y)] };
			}
			return { {} };
		}

		GroupView getContainersViewByGroup(const std::string& groupingName, const std::string& groupName) const {
			auto itr = groups.find(groupingName);
			if (itr == groups.end() && groupingFunctions.find(groupingName) != groupingFunctions.end()) {
				auto [insert_itr, _] = groups.insert({ groupingName, Group{} });
				itr = insert_itr;
			}
			if (itr != groups.end()) {
				const auto& grouping = itr->second;
				auto itr2 = grouping.find(groupName);
				if (itr2 == grouping.end()) {
					auto [insert_itr, _] = itr->second.insert({ groupName, Pos2Container{} });
					itr2 = insert_itr;
				}
				return GroupView{ itr2->second };
			}
			return GroupView{ 0 };
		}

		iterator begin() {
			return iterator{ *this };
		}
		iterator end() {
			return iterator{ *this, -1 };
		}

		Ship(const Ship&) = delete;
		Ship& operator=(const Ship&) = delete;
		Ship(Ship&&) = default;
		Ship& operator=(Ship&&) = default;
	};
}

