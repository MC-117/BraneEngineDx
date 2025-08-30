#pragma once
#include "../Serialization.h"
#include "../Asset.h"
#include "../Importer/Importer.h"
#include "Parallel.h"

template<class T>
class AssetRef
{
public:
    AssetRef() = default;
    AssetRef(const string& path) : path(path)
    {
    }
    AssetRef(T* assetObject)
    {
        setObject(assetObject);
    }

    ~AssetRef() = default;
    
    AssetRef(const AssetRef& other)
    {
        copyFrom(other);
    }
    
    AssetRef(AssetRef&& other)
    {
        moveFrom(other);
    }

    bool isValid() const
    {
        return resolvedObject;
    }
    
    bool isPending() const
    {
        return !path.empty();
    }

    bool isRegistered() const
    {
        return resolvedAsset;
    }

    void reset()
    {
        path.clear();
        resolvedAsset = NULL;
        resolvedObject = NULL;
    }

    void resolve()
    {
        if (resolvedObject) {
            path = AssetInfo::getPath(resolvedObject);
        }
        if (!path.empty()) {
            resolvedAsset = AssetInfo::getAssetByPath(path);
        }
    }

    void setObject(T* assetObject)
    {
        resolvedObject = assetObject;
        resolvedAsset = NULL;
        path.clear();
        resolve();
    }

    void copyFrom(const AssetRef& other)
    {
        if (this == &other) {
            return;
        }
        path = other.path;
        resolvedAsset = other.resolvedAsset;
        resolvedObject = other.resolvedObject;
        resolve();
    }

    void moveFrom(AssetRef& other)
    {
        if (this == &other) {
            return;
        }
        path = std::move(other.path);
        resolvedAsset = other.resolvedAsset;
        resolvedObject = other.resolvedObject;
        resolve();
        other.reset();
    }

    T* load()
    {
        if (!resolvedObject && !resolvedAsset) {
            ImportResult result;
            if (IImporter::load(path, result)) {
                resolvedAsset = result.asset;
                if (resolvedAsset) {
                    resolvedObject = dynamic_cast<T*>(result.asset->getActualAsset());
                }
                if (resolvedObject) {
                    path = resolvedAsset->path;
                }
                else {
                    resolvedAsset = NULL;
                    path.clear();
                }
            }
        }
        return resolvedObject;
    }
    
    TaskEventHandle loadAsync()
    {
        if (resolvedObject || path.empty()) {
            taskHandle = TaskEventHandle();
            return taskHandle;
        }
        resolvedObject = getAssetByPath<T>(path);
        if (resolvedObject) {
            taskHandle = TaskEventHandle();
        }
        else if (!taskHandle.isValid()) {
            taskHandle = IImporter::loadAsync(path);
        }
        return taskHandle;
    }

    void deserialize(const SerializationInfo& from)
    {
        string type;
        if (from.get("type", type)) {
            
        }
        from.get("path", path);
    }
    
    void serialize(SerializationInfo& to)
    {
        to.type = "AssetRef";
        resolve();
        if (resolvedAsset) {
            to.set("type", resolvedAsset->assetInfo.type);
        }
        to.set("path", path);
    }
    
    T* get()
    {
        return load();
    }
    T* get() const
    {
        return resolvedObject;
    }
    T* operator->()
    {
        return load();
    }
    T* operator->() const
    {
        return resolvedObject;
    }
    operator T* ()
    {
        return load();
    }
    operator T* () const
    {
        return resolvedObject;
    }
    
    AssetRef& operator=(T* assetObject)
    {
        setObject(assetObject);
        return *this;
    }
    AssetRef& operator=(const AssetRef& other)
    {
        copyFrom(other);
        return *this;
    }
    AssetRef& operator=(AssetRef&& other)
    {
        moveFrom(other);
        return *this;
    }
protected:
    string path;
    Asset* resolvedAsset = NULL;
    T* resolvedObject = NULL;
    TaskEventHandle taskHandle;
};
