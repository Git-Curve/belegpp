#pragma once
#include <string>
#include <random>
#include <cctype>
#include <vector>
#include <optional>
#include <algorithm>
#include <functional>

namespace beleg
{
	namespace sfinae
	{
		template <typename T>
		struct has_const_iterator
		{
		private:
			template <typename C> static std::true_type & test(typename C::const_iterator*);
			template <typename C> static std::false_type & test(...);
		public:
			static const bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
		};

		template <typename T>
		struct is_map_like
		{
		private:
			template <typename C> static std::true_type & test(typename C::const_iterator::value_type::second_type*);
			template <typename C> static std::false_type & test(...);
		public:
			static const bool value = sizeof(test<T>(nullptr)) == sizeof(std::true_type);
		};

		template<typename T, typename D, typename = void>
		struct is_equality_comparable : std::false_type { };
		template<typename T, typename D>
		struct is_equality_comparable<T, D,
			typename std::enable_if<
			true,
			decltype(std::declval<T&>() == std::declval<D&>(), (void)0)
			>::type
		> : std::true_type {};
	}
	namespace helpers
	{
		namespace strings
		{
			inline std::string toLower(std::string str)
			{
				std::transform(str.begin(), str.end(), str.begin(), ::tolower);
				return str;
			}
			inline std::string toUpper(std::string str)
			{
				std::transform(str.begin(), str.end(), str.begin(), ::toupper);
				return str;
			}
			inline std::string mul(std::string str, unsigned int& n)
			{
				std::string rtn;
				for (int i = 0; n > i; i++) rtn += str;
				return rtn;
			}
			inline std::string replace(std::string str, std::string& from, std::string& to)
			{
				if (!from.empty())
					for (size_t pos = 0; (pos = str.find(from, pos)) != std::string::npos; pos += to.size())
						str.replace(pos, from.size(), to);
				return str;
			}
			inline bool contains(std::string str, std::string what)
			{
				return str.find(what) != std::string::npos;
			}
			inline std::vector<std::string> split(std::string str, std::string& delim)
			{
				std::vector<std::string> rtn;
				size_t found = str.find(delim);
				size_t startIndex = 0;
				while (found != std::string::npos)
				{
					std::string temp(str.begin() + startIndex, str.begin() + found);
					rtn.push_back(temp);
					startIndex = found + delim.size();
					found = str.find(delim, startIndex);
				}
				if (startIndex != str.size())
					rtn.push_back(std::string(str.begin() + startIndex, str.end()));
				return rtn;
			}
			inline std::string trim(std::string str)
			{
				str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), [](char c) { return std::isspace(c); }));
				str.erase(std::find_if_not(str.rbegin(), str.rend(), [](char c) { return std::isspace(c); }).base(), str.end());
				return str;
			}
			inline bool startsWith(std::string str, std::string& what)
			{
				return str.rfind(what, 0) == 0;
			}
			inline bool endsWith(std::string str, std::string& what)
			{
				return str.size() >= what.size() && str.compare(str.size() - what.size(), what.size(), what) == 0;
			}
			inline bool equalsIgnoreCase(std::string str, std::string& what)
			{
				return toLower(str) == toLower(what);
			}
		}
		namespace containers
		{
			template <typename T, typename W,
				typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value &&
				!std::is_same<T, std::string>::value
				>* = nullptr
			>
				bool contains(T& container, W& what)
			{
				auto found = std::find_if(container.begin(), container.end(), [&](auto item)
				{
					return item == what;
				});
				return found != container.end();
			}

			template <typename T, typename W,
				typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_map_like<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type::second_type, W>::value &&
				!std::is_same<T, std::string>::value
				>* = nullptr
			>
				bool containsItem(T& container, W& what)
			{
				auto found = std::find_if(container.begin(), container.end(), [&](auto item)
				{
					return item.second == what;
				});
				return found != container.end();
			}

			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_map_like<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type::first_type, W>::value &&
				!std::is_same<T, std::string>::value
			>* = nullptr>
				bool containsKey(T& container, W& what)
			{
				return container.find(what) != container.end();
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T map(T& container, std::function<typename T::const_iterator::value_type(typename T::const_iterator::value_type&)> func)
			{
				std::for_each(container.begin(), container.end(), [&](auto& item)
				{
					item = func(item);
				});
				return container;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T filter(T& container, std::function<bool(typename T::const_iterator::value_type&)> func)
			{
				T newContainer;
				std::for_each(container.begin(), container.end(), [&](auto& item)
				{
					if (func(item))
					{
						newContainer.insert(newContainer.end(), item);
					}
				});
				return newContainer;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				void forEach(T& container, std::function<void(typename T::const_iterator::value_type&)> func)
			{
				std::for_each(container.begin(), container.end(), [&](auto& item)
				{
					func(item);
				});
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				std::optional<typename T::const_iterator> findIf(T& container, std::function<bool(typename T::const_iterator::value_type&)> func)
			{
				auto item = std::find_if(container.begin(), container.end(), [&](auto it)
				{
					return func(it);
				});
				if (item != container.end())
					return item;
				return std::nullopt;
			}
			
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value
			>* = nullptr>
				std::optional<typename T::const_iterator> find(T& container, W& what)
			{
				auto item = std::find_if(container.begin(), container.end(), [&](auto it)
				{
					return it == what;
				});
				if (item != container.end())
					return item;
				return std::nullopt;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T& removeIf(T& container, std::function<bool(typename T::const_iterator::value_type&)> func)
			{
				container.erase(std::remove_if(container.begin(), container.end(), [&](auto it)
				{
					return func(it);
				}), container.end());
				return container;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T reverse(T& container)
			{
				std::reverse(container.begin(), container.end());
				return container;
			}

			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value
			>* = nullptr>
				T& remove(T& container, W& what)
			{
				container.erase(std::remove(container.begin(), container.end(), what), container.end());
				return container;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T sort(T& container, std::function<bool(typename T::const_iterator::value_type& first, typename T::const_iterator::value_type& second)> func)
			{
				std::sort(container.begin(), container.end(), func);
				return container;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				bool some(T& container, std::function<bool(typename T::const_iterator::value_type& first)> func)
			{
				bool some = false;
				std::all_of(container.begin(), container.end(), [&](auto& item)
				{
					if (func(item))
					{
						some = true;
						return false;
					}
					return true;
				});
				return some;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				bool every(T& container, std::function<bool(typename T::const_iterator::value_type& first)> func)
			{
				bool every = true;
				std::all_of(container.begin(), container.end(), [&](auto& item)
				{
					if (!func(item))
					{
						every = false;
						return false;
					}
					return true;
				});
				return every;
			}

			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
			T slice(T& input, int start, int end)
			{
				if (end == 0) end = input.size();
				if (end < 0)
					end = input.size() + end;
				if (end < 0) end = 0;
				if (end > input.size()) end = input.size();

				if (start < 0) start = input.size() + start;
				if (start < 0) start = 0;

				if (start > end || start == end)
					return {};

				T sliced;
				sliced.assign(input.begin() + start, input.begin() + end);
				return sliced;
			}

			inline std::random_device rd;
			inline std::mt19937 mt(rd());
			template <typename T, class random = std::mt19937, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T shuffle(T& input, random rand = mt)
			{
				std::shuffle(input.begin(), input.end(), rand);
				return input;
			}
		}
	}
	namespace extensions
	{
		namespace strings
		{
			struct toLower {};
			inline std::string operator|(std::string str, toLower)
			{
				return helpers::strings::toLower(str);
			}
			inline std::string operator|(const char* str, toLower)
			{
				return helpers::strings::toLower(std::string(str));
			}
			struct toUpper {};
			inline std::string operator|(std::string str, toUpper)
			{
				return helpers::strings::toUpper(str);
			}
			inline std::string operator|(const char* str, toUpper)
			{
				return helpers::strings::toUpper(str);
			}
			inline std::string operator*(std::string str, unsigned int amount)
			{
				return helpers::strings::mul(str, amount);
			}
			struct mul { unsigned int n; mul(unsigned int n) : n(n) {} };
			inline std::string operator|(const char* str, mul mul)
			{
				return helpers::strings::mul(str, mul.n);
			}
			struct replace { std::string from; std::string to; replace(std::string from, std::string to) : from(from), to(to) {} };
			inline std::string operator|(std::string str, replace what)
			{
				return helpers::strings::replace(str, what.from, what.to);
			}
			inline std::string operator|(const char* str, replace what)
			{
				return helpers::strings::replace(str, what.from, what.to);
			}
			struct split { std::string delim; split(std::string delim) : delim(delim) {} };
			inline std::vector<std::string> operator|(std::string str, split what)
			{
				return helpers::strings::split(str, what.delim);
			}
			inline std::vector<std::string> operator|(const char* str, split what)
			{
				return helpers::strings::split(str, what.delim);
			}
			struct startsWith { std::string what; startsWith(std::string what) : what(what) {} };
			inline bool operator|(std::string str, startsWith what)
			{
				return helpers::strings::startsWith(str, what.what);
			}
			inline bool operator|(const char* str, startsWith what)
			{
				return helpers::strings::startsWith(str, what.what);
			}
			struct endsWith { std::string what; endsWith(std::string what) : what(what) {} };
			inline bool operator|(std::string str, endsWith what)
			{
				return helpers::strings::endsWith(str, what.what);
			}
			inline bool operator|(const char* str, endsWith what)
			{
				return helpers::strings::endsWith(str, what.what);
			}
			struct equalsIgnoreCase { std::string what; equalsIgnoreCase(std::string what) : what(what) {} };
			inline bool operator|(std::string str, equalsIgnoreCase what)
			{
				return helpers::strings::equalsIgnoreCase(str, what.what);
			}
			inline bool operator|(const char* str, equalsIgnoreCase what)
			{
				return helpers::strings::equalsIgnoreCase(str, what.what);
			}
			struct trim {};
			inline std::string operator|(std::string str, trim)
			{
				return helpers::strings::trim(str);
			}
			inline std::string operator|(const char* str, trim)
			{
				return helpers::strings::trim(str);
			}
		}
		namespace containers
		{
			template <typename T> struct contains { T what; contains(T what) : what(what) {} };
			template <typename T, typename W, std::enable_if_t<std::is_same<T, std::string>::value && (std::is_same<W, std::string>::value || std::is_same<const char*, W>::value)>* = nullptr>
			bool operator|(T str, contains<W> what)
			{
				return helpers::strings::contains(str, std::is_same<W, std::string>::value ? what.what : std::string(what.what));
			}
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value &&
				!std::is_same<T, std::string>::value
			>* = nullptr>
				bool operator|(T& container, contains<W> what)
			{
				return helpers::containers::contains(container, what.what);
			}

			template <typename T> struct containsItem { T what; containsItem(T what) : what(what) {} };
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type::second_type, W>::value &&
				!std::is_same<T, std::string>::value
			>* = nullptr>
				bool operator|(T& container, containsItem<W> what)
			{
				return helpers::containers::containsItem(container, what.what);
			}

			template <typename T> struct containsKey { T what; containsKey(T what) : what(what) {} };
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type::first_type, W>::value &&
				!std::is_same<T, std::string>::value
			>* = nullptr>
				bool operator|(T& container, containsKey<W> what)
			{
				return helpers::containers::containsKey(container, what.what);
			}

			template <typename T> struct map { T func; map(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, map<F> transfrm)
			{
				return helpers::containers::map(container, transfrm.func);
			}

			template <typename T> struct filter { T func; filter(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, filter<F> transfrm)
			{
				return helpers::containers::filter(container, transfrm.func);
			}

			template <typename T> struct forEach { T func; forEach(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				void operator|(T container, forEach<F> transfrm)
			{
				return helpers::containers::forEach(container, transfrm.func);
			}

			template <typename T> struct find { T what; find(T what) : what(what) {} };
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value
			>* = nullptr>
				std::optional<typename T::const_iterator> operator|(T& container, find<W> what)
			{
				return helpers::containers::find(container, what.what);
			}

			template <typename T> struct findIf { T func; findIf(T func) : func(func) {} };
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				std::optional<typename T::const_iterator> operator|(T& container, findIf<W> what)
			{
				return helpers::containers::findIf(container, what.func);
			}

			template <typename T> struct removeIf { T func; removeIf(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T& operator|(T& container, removeIf<F> what)
			{
				return helpers::containers::removeIf(container, what.func);
			}

			struct reverse {};
			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, reverse)
			{
				return helpers::containers::reverse(container);
			}

			template <typename T> struct remove { T what; remove(T what) : what(what) {} };
			template <typename T, typename W, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value &&
				sfinae::is_equality_comparable<typename T::const_iterator::value_type, W>::value
			>* = nullptr>
				T& operator|(T& container, remove<W> what)
			{
				return helpers::containers::remove(container, what.what);
			}

			template <typename T> struct sort { T func; sort(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, sort<F> what)
			{
				return helpers::containers::sort(container, what.func);
			}

			template <typename T> struct some { T func; some(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				bool operator|(T& container, some<F> what)
			{
				return helpers::containers::some(container, what.func);
			}

			template <typename T> struct every { T func; every(T func) : func(func) {} };
			template <typename T, typename F, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				bool operator|(T& container, every<F> what)
			{
				return helpers::containers::every(container, what.func);
			}

			struct slice { int start; int end; slice(int start, int end = 0) : start(start), end(end) {} };
			template <typename T, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, slice what)
			{
				return helpers::containers::slice(container, what.start, what.end);
			}

			template <class random = std::mt19937> struct shuffle { random rand; shuffle(random rand = helpers::containers::mt) : rand(rand) {} };
			template <typename T, class random = std::mt19937, typename = std::decay_t<decltype(*begin(std::declval<T>()))>,
				typename = std::decay_t<decltype(*end(std::declval<T>()))>,
				std::enable_if_t<
				sfinae::has_const_iterator<T>::value
			>* = nullptr>
				T operator|(T container, shuffle<random> what)
			{
				return helpers::containers::shuffle(container);
			}
		}
	}
}