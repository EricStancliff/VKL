#include <vector>
#include <typeindex>
#include <string>
#include <iostream>
#include <any>
#include <functional>
#include <unordered_map>
#include <memory>
#include <vkl/Common.h>

namespace reflect
{
	template <typename T>
	struct GetMemberTypes;

	template <typename T, typename M>
	struct GetMemberTypes<M T::*>
	{
		using member_type = M;
		using class_type = T;
	};

	template <typename T>
	using GetMemberType = typename GetMemberTypes<T>::member_type;

	template <typename T>
	using GetClassType = typename GetMemberTypes<T>::class_type;

	class object;

	class VKL_EXPORT member
	{
	public:
		member() = default;
		virtual ~member() = default;
		member(const member&) = delete;
		member(member&&) noexcept = default;
		member& operator=(member&&) noexcept = default;
		member& operator=(const member&) = delete;
		virtual std::any get(object* obj) const = 0;
		virtual void set(object* obj, const std::any& value) const = 0;
		virtual std::string name() const = 0;

	};

	template <typename T, typename M>
	class typed_member : public member
	{
	public:

		typed_member() = delete;
		typed_member(M T::* member, const char* name) : _member(member), _name(name) {}

		virtual std::any get(object* obj) const override
		{
			return static_cast<T*>(obj)->*_member;
		}
		virtual void set(object* obj, const std::any& value) const override
		{
			static_cast<T*>(obj)->*_member = std::any_cast<M>(value);
		}
		virtual std::string name() const override {
			return _name;
		}


	private:
		M T::* _member;
		std::string _name;
	};

	class VKL_EXPORT reflection_data_base
	{

		std::vector<std::unique_ptr<member>> members;

	public:

		reflection_data_base() = default;
		virtual ~reflection_data_base() = default;
		reflection_data_base(const reflection_data_base&) = delete;
		reflection_data_base(reflection_data_base&&) noexcept = default;
		reflection_data_base& operator=(reflection_data_base&&) noexcept = default;
		reflection_data_base& operator=(const reflection_data_base&) = delete;

		template <auto _member>
		void declareMember(const char* name) {
			using member_pointer_type = decltype(_member);
			using class_type = GetClassType<member_pointer_type>;
			using member_type = GetMemberType<member_pointer_type>;
			members.push_back(std::make_unique<typed_member<class_type, member_type>>(_member, name));
		}

		template <typename T>
		void setMember(object* obj, const std::string& name, const T& value) const
		{
			auto member = findMember(name);
			if (member)
				member->set(obj, value);
		}

		template <typename T>
		T getValue(object* obj, const std::string& name) const
		{
			auto member = findMember(name);
			if (member)
				return std::any_cast<T>(member->get(obj));
			return {};
		}

	private:
		const member* findMember(const std::string& name) const
		{
			auto find = std::find_if(members.begin(), members.end(), [&name](const std::unique_ptr<member>& _member) {
				return _member->name() == name;
				});
			if (find == members.end())
				return nullptr;
			return find->get();
		}
	};

	class VKL_EXPORT reflection
	{
	public:
		reflection() = default;
		size_t index{ 0 };
		std::string className;
		reflection_data_base* data;
		size_t parent = 0;
		std::vector<size_t> children;
	};

	class type_dictionary;

	VKL_EXPORT std::unordered_map<std::type_index, size_t>& typed_indexer();

	class VKL_EXPORT type_dictionary
	{
	public:
		type_dictionary() {
		};
		template <typename T>
		bool addType(const char* name) {
			if (!T::reflection_parent::reflection_initialized())
				return false;

			//index
			size_t index = _types.size();
			typed_indexer()[typeid(T)] = index;

			//establish type
			_types.push_back({});
			reflection& info = _types[index];
			info.index = index;
			info.className = name;
			info.data = new typename T::reflection_data();

			//take in reflection data
			T::populateReflection(*static_cast<typename T::reflection_data*>(info.data));

			//child/parent
			size_t parentIndex = typed_indexer()[typeid(typename T::reflection_parent)];
			if (parentIndex != index)
			{
				info.parent = parentIndex;
				_types[parentIndex].children.push_back(index);
			}

			return true;
		}

		template <typename T>
		const reflection& getType() const
		{
			return _types[typed_indexer()[typeid(T)]];
		}

		template <typename T>
		size_t indexOf() const
		{
			return typed_indexer()[typeid(T)];
		}

		void forAllTypesDerivedFrom(size_t index, const std::function<void(const reflection&)>& f) const;

	private:
		std::vector<reflection> _types;
	};

	VKL_EXPORT type_dictionary& _types_unsafe();


	class VKL_EXPORT NullType
	{
	public:
		virtual const reflection& reflect() const = 0;
		virtual const reflection& reflectParent() const = 0;
		virtual const reflection_data_base& reflectionInfo() const = 0;
	private:
		friend class type_dictionary;
		static bool reflection_initialized();
	};

}

#define REFLECTED_TYPE_CUSTOM(type, parent, reflection_type)\
public:\
using reflection_parent = parent;\
virtual const reflect::reflection& reflect() const override{\
return reflect::typeDictionary().getType<type>();\
}\
virtual const reflect::reflection& reflectParent() const override{\
return reflect::typeDictionary().getType<parent>();\
}\
using reflection_data = reflection_type;\
virtual const reflect::reflection_data_base& reflectionInfo() const override {\
return *reflect::typeDictionary().getType<type>().data;\
}\
private:\
friend class reflect::type_dictionary;\
static bool reflection_initialized();\
static bool _reflection_initialized;

#define REFLECTED_TYPE(type, parent)\
REFLECTED_TYPE_CUSTOM(type, parent, parent::reflection_data)

#define IMPL_REFLECTION(type)\
bool type::reflection_initialized(){\
static bool initialized = reflect::_types_unsafe().addType<type>(#type);\
return initialized;\
}\
bool type::_reflection_initialized = type::reflection_initialized();