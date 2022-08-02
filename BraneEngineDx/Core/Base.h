#pragma once

#include "Serialization.h"
#include <type_traits>
#include "Brane.h"

class Base : public Serializable
{
	template<class T>
	friend struct Ref;
private:
	int _magic_start_ = '$BAZ';
public:
	Serialize(Base,);

	Base();
	virtual ~Base();

	static bool CheckValid(Base* base);

	InstanceID getInstanceID() const;
	bool isAsset() const;

	bool setInstanceParent(Base& base);

	InstanceAssetFile getAssetFile() const;
	InstanceAssetHandle getAssetHandle() const;
	const list<InstanceID>& getLinkedInstances() const;

	static Serializable* instantiate(const SerializationInfo& from);
	virtual bool deserialize(const SerializationInfo& from);
	virtual bool serialize(SerializationInfo& to);
protected:
	Base(InstanceID insID, const InstanceAssetHandle& handle);
	InstanceID instanceID = 0;
	InstanceAssetHandle assetHandle;

	InstanceID parentInstanceID = 0;
	list<InstanceID> linkedInstances;
	bool dirty = false;

	Guid getOrNewGuid();
	bool createAssetFromPath(const string& path, bool allowMove);
	bool createAssetFromFileID(FileID fileID, bool allowMove);
private:
	int _magic_end_ = 'ZAB$';
};

template<class T>
struct Ref
{
	Ref();
	Ref(Base* base);
	InstanceID instanceID = 0;
	Guid guid;
	T* pointer;

	void deserialize(const SerializationInfo& from);
	void serialize(SerializationInfo& to);

	T* get();
	T* get() const;
	T* operator->();
	T* operator->() const;
	operator T* ();
	operator T* () const;

	Ref& operator=(T* base);
};

template<class T>
inline Ref<T>::Ref()
{
	static_assert(std::is_base_of<Base, T>::value,
		"Only support classes based on Base");
}

template<class T>
inline Ref<T>::Ref(Base* base)
{
	static_assert(std::is_base_of<Base, T>::value,
		"Only support classes based on Base");
	T* ptr = dynamic_cast<T*>(base);
	*this = ptr;
}

template<class T>
inline void Ref<T>::deserialize(const SerializationInfo& from)
{
	string guidStr;
	if (from.get("guid", guidStr)) {
		instanceID = 0;
		pointer = NULL;
		guid = Guid::fromString(guidStr);
	}
}

template<class T>
inline void Ref<T>::serialize(SerializationInfo& to)
{
	to.type = "Ref";
	if (guid.isDefault()) {
		Base* base = dynamic_cast<Base*>(get());
		if (base)
			guid = base->getOrNewGuid();
	}
	to.set("guid", guid.toString());
}

template<class T>
inline T* Ref<T>::get()
{
	pointer = dynamic_cast<T*>((Base*)Brane::getPtrByInsID(instanceID));
	if (pointer != NULL)
		return pointer;
	instanceID = Brane::getInsIDByGuid(guid);
	pointer = dynamic_cast<T*>((Base*)Brane::getPtrByInsID(instanceID));
	return pointer;
}

template<class T>
inline T* Ref<T>::get() const
{
	T* pointer = dynamic_cast<T*>((Base*)Brane::getPtrByInsID(instanceID));
	if (pointer != NULL)
		return pointer;
	InstanceID instanceID = Brane::getInsIDByGuid(guid);
	pointer = dynamic_cast<T*>((Base*)Brane::getPtrByInsID(instanceID));
	return pointer;
}

template<class T>
inline T* Ref<T>::operator->()
{
	return get();
}

template<class T>
inline T* Ref<T>::operator->() const
{
	return get();
}

template<class T>
inline Ref<T>::operator T* ()
{
	return get();
}

template<class T>
inline Ref<T>::operator T* () const
{
	return get();
}

template<class T>
inline Ref<T>& Ref<T>::operator=(T* ptr)
{
	if (ptr == NULL) {
		pointer = NULL;
		instanceID = 0;
		guid = Guid();
	}
	else {
		pointer = ptr;
		instanceID = pointer->getInstanceID();
		guid = pointer->getAssetHandle().guid;
	}
	return *this;
}
