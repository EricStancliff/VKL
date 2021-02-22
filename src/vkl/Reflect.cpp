#include <Reflect.h>


namespace reflect
{
	std::unordered_map<std::type_index, size_t>& typed_indexer()
	{
		static std::unordered_map<std::type_index, size_t> typedIndexer;
		return typedIndexer;
	}
	bool NullType::reflection_initialized()
	{
		return true;
	}

	type_dictionary& _types_unsafe() {
		static type_dictionary dictionary;
		return dictionary;
	}

	const type_dictionary& typeDictionary() {
		return _types_unsafe();
	}

	const reflect::reflection& reflect(const object* obj)
	{
		return obj->reflect();
	}


	void type_dictionary::forAllTypesDerivedFrom(size_t index, const std::function<void(const reflection&)>& f) const
	{
		auto& reflection = _types[index];
		f(reflection);
		for (auto&& child : reflection.children)
			forAllTypesDerivedFrom(child, f);
	}
}

IMPL_REFLECTION(reflect::object)