#pragma once
#include <Reflect.inl>
namespace reflect
{
	VKL_EXPORT const type_dictionary& typeDictionary();


	class VKL_EXPORT object : public NullType
	{
		REFLECTED_TYPE(object, NullType)

	public:
		static void populateReflection(reflection_data& reflection) {}
	};


	template <typename T>
	const reflect::reflection& reflect() {
		return reflect::typeDictionary().getType<T>();
	}

	template <typename T>
	const typename T::reflection_data& reflectionInfo() {
		return *static_cast<const typename T::reflection_data*>(reflect<T>().data);
	}

	template <typename T>
	void forAllTypesDerivedFrom(const std::function<void(const reflection&)>& f)
	{
		typeDictionary().forAllTypesDerivedFrom(typeDictionary().indexOf<T>(), f);
	}
}